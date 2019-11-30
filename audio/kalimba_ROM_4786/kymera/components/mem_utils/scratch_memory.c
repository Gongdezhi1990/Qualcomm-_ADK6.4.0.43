/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file scratch_memory.c
 * \ingroup scratch_mem
 *
 * This file contains the functions that handle the
 * reservation and use of scratch memory.
 *
 */

 /****************************************************************************
Include Files
*/
#include "scratch_memory.h"
#include "pmalloc/pl_malloc.h"
#include "sched_oxygen/sched_oxygen.h"
#include "platform/pl_intrinsics.h"
#include "panic/panic.h"
#include "patch/patch.h"

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Type Declarations
*/

typedef struct mem_alloc_info
{
    void *base;
    unsigned int max_reserved;
    unsigned int total_committed;
    unsigned int alloc_size;
} mem_alloc_info;

typedef struct scratch_per_task_data
{
    taskid task_id;
    unsigned int task_priority;
    unsigned int total_reserved_dm1;
    unsigned int total_reserved_dm2;
    unsigned int total_reserved_none;
    struct scratch_per_task_data *next;
} scratch_per_task_data;

typedef struct scratch_per_prio_data
{
    int refcount;
    mem_alloc_info alloc_info_dm1;
    mem_alloc_info alloc_info_dm2;
    mem_alloc_info alloc_info_none;
    taskid last_task;
} scratch_per_prio_data;

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/
scratch_per_task_data *first_scratch_mem = NULL;

scratch_per_prio_data per_prio_data[NUM_PRIORITIES];

/****************************************************************************
Private Function Definitions
*/

/*
 * lookup_per_task_data
 *
 * Search the linked list of registered tasks
 * return the per-task data with matching ID, or NULL if not found
 * Do this with interrupts blocked in case another task at higher priority
 * tries to remove an entry while we're iterating through the list.
 */
static scratch_per_task_data *lookup_per_task_data(taskid task)
{
    scratch_per_task_data *ptd;
    LOCK_INTERRUPTS;
    ptd = first_scratch_mem;
    while ((ptd != NULL) && (ptd->task_id != task))
    {
        ptd = ptd->next;
    }
    UNLOCK_INTERRUPTS;

    return ptd;
}

/*
 * free_base_alloc
 *
 * Free and zero the provided allocation data
 */
static void free_base_alloc(mem_alloc_info *alloc_info)
{
    pfree(alloc_info->base);
    alloc_info->base = NULL;
    alloc_info->max_reserved = 0;
    alloc_info->alloc_size = 0;
    alloc_info->total_committed = 0;
}

/*
 * realloc_base
 *
 * Free the old base allocation and allocate a new one.
 * This should only be called if the new allocation is no larger
 * than the old one. Otherwise the call to pmalloc could panic.
 */
static void realloc_base(mem_alloc_info *alloc_info, unsigned int size, unsigned preference)
{
    /* Free the old block and reallocate a new (smaller) one
     * Do this with interrupts blocked so it will always succeed
     */
    LOCK_INTERRUPTS;
    pfree(alloc_info->base);
    if (size > 0)
    {
        alloc_info->base = ppmalloc(size, preference);
        UNLOCK_INTERRUPTS;
        alloc_info->alloc_size = psizeof(alloc_info->base);
    }
    else
    {
        UNLOCK_INTERRUPTS;
        alloc_info->base = NULL;
        alloc_info->alloc_size = 0;
    }
    alloc_info->max_reserved = size;
}


/*
 * deregister_update
 *
 * Called when a task has deregistered
 * Work out the new allocation sizes for each preference.
 */
static void deregister_update(unsigned int priority)
{
    unsigned int max_dm1 = 0, max_dm2 = 0, max_none = 0;
    scratch_per_task_data *ptd;

    /* Find the maximum reserved for each type
     * at this priority level
     * Do this with interrupts blocked in case another task at higher priority
     * tries to remove an entry while we're iterating through the list.
     */
    LOCK_INTERRUPTS;
    ptd = first_scratch_mem;

    while (ptd != NULL)
    {
        if (ptd->task_priority == priority)
        {
            if (ptd->total_reserved_dm1 > max_dm1)
            {
                max_dm1 = ptd->total_reserved_dm1;
            }
            if (ptd->total_reserved_dm2 > max_dm2)
            {
                max_dm2 = ptd->total_reserved_dm2;
            }
            if (ptd->total_reserved_none > max_none)
            {
                max_none = ptd->total_reserved_none;
            }
        }
        ptd = ptd->next;
    }

    UNLOCK_INTERRUPTS;

    /* Reallocate any base blocks that could be smaller */
    if (max_dm1 < per_prio_data[priority].alloc_info_dm1.max_reserved)
    {
        realloc_base(&per_prio_data[priority].alloc_info_dm1, max_dm1, MALLOC_PREFERENCE_DM1);
    }
    if (max_dm2 < per_prio_data[priority].alloc_info_dm2.max_reserved)
    {
        realloc_base(&per_prio_data[priority].alloc_info_dm2, max_dm2, MALLOC_PREFERENCE_DM2);
    }
    if (max_none < per_prio_data[priority].alloc_info_none.max_reserved)
    {
        realloc_base(&per_prio_data[priority].alloc_info_none, max_none, MALLOC_PREFERENCE_FAST);
    }
}

/****************************************************************************
Public Function Definitions
*/

/*
 * scratch_register
 */
bool scratch_register(void)
{
    taskid task = get_current_task();
    unsigned int priority = GET_TASK_PRIORITY(task);

    scratch_per_task_data *ptd = lookup_per_task_data(task);

    patch_fn_shared(mem_utils);

    if (ptd == NULL)
    {
        /* No existing scratch record for this task, so make a new one */
        ptd = xzpnew(scratch_per_task_data);
        if (ptd != NULL)
        {
            ptd->task_id = task;
            ptd->task_priority = priority;
            /* Make sure the list update can't be pre-empted
             * by a higher priority task */
            LOCK_INTERRUPTS;
            ptd->next = first_scratch_mem;
            first_scratch_mem = ptd;
            UNLOCK_INTERRUPTS;
            per_prio_data[priority].refcount++;
            return TRUE;
        }
    }
    else
    {
        /* Can only register once per task */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
    }

    return FALSE;
}

/*
 * scratch_register
 */
bool scratch_deregister(void)
{
    taskid task = get_current_task();
    scratch_per_task_data **pptd, *temp;
    unsigned int priority = GET_TASK_PRIORITY(task);

    patch_fn_shared(mem_utils);

    if (per_prio_data[priority].refcount <= 0)
    {
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
    }

    /* Search the list for the current task
     * Do this with interrupts blocked to avoid concurrency issues */

    LOCK_INTERRUPTS;
    pptd = &first_scratch_mem;
    while ((*pptd != NULL) && ((*pptd)->task_id != task))
    {
        pptd = &(*pptd)->next;
    }
    if (*pptd != NULL)
    {
        temp = *pptd;
        *pptd = (*pptd)->next;
        /* Finished updating the list
         * per-priority data (for this priority) can only be accessed
         * by one task at a time, so doesn't need protecting
         */
        UNLOCK_INTERRUPTS;

        pdelete(temp);

        /* Check if this was the last one at this priority */
        if (--per_prio_data[priority].refcount == 0)
        {
            /* Free the scratch block if it was */
            free_base_alloc(&per_prio_data[priority].alloc_info_dm1);
            free_base_alloc(&per_prio_data[priority].alloc_info_dm2);
            free_base_alloc(&per_prio_data[priority].alloc_info_none);
        }
        else
        {
            deregister_update(priority);
        }
        per_prio_data[priority].last_task = NO_TASK;

        return TRUE;
    }
    else
    {
        UNLOCK_INTERRUPTS;
        /* Task was not registered */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
#ifdef DESKTOP_TEST_BUILD
        return FALSE;
#endif
    }
}

/*
 * scratch_reserve
 */
bool scratch_reserve(unsigned int size, unsigned int preference)
{
    unsigned int priority;
    unsigned int new_size;
    unsigned int *reserved;
    taskid task = get_current_task();
    scratch_per_task_data *ptd = lookup_per_task_data(task);
    mem_alloc_info *alloc_info;

    patch_fn_shared(mem_utils);

    if (ptd == NULL)
    {
        /* Must be registered first */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
    }

    priority = GET_TASK_PRIORITY(ptd->task_id);

    /* Scratch memory always requests FAST RAM in the don't care situation, so
     * modify the preference if the caller asked for SLOW RAM. */
    if (preference == MALLOC_PREFERENCE_NONE)
    {
        preference = MALLOC_PREFERENCE_FAST;
    }

    switch (preference)
    {
    case MALLOC_PREFERENCE_DM1:
        alloc_info = &per_prio_data[priority].alloc_info_dm1;
        reserved = &ptd->total_reserved_dm1;
        break;
    case MALLOC_PREFERENCE_DM2:
        alloc_info = &per_prio_data[priority].alloc_info_dm2;
        reserved = &ptd->total_reserved_dm2;
        break;
    case MALLOC_PREFERENCE_FAST:
    default:
        alloc_info = &per_prio_data[priority].alloc_info_none;
        reserved = &ptd->total_reserved_none;
        break;
    }

    if ((task == per_prio_data[priority].last_task) &&
        (alloc_info->total_committed != 0))
    {
        /* This task has committed scratch, it can't reserve any more */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
    }

    new_size = *reserved + size;

    /* Check if we can fit the new reservation in the current block,
     * and allocate a new one if not
     */
    if (new_size > alloc_info->max_reserved)
    {
        if (new_size > alloc_info->alloc_size)
        {
            void *temp = xppmalloc(new_size, preference);
            if (temp == NULL)
            {
                return FALSE;
            }
            pfree(alloc_info->base);
            alloc_info->base = temp;
            alloc_info->alloc_size = psizeof(temp);
        }
        alloc_info->max_reserved = new_size;
    }
    *reserved = new_size;

    return TRUE;
}

/*
 * scratch_release
 */
bool scratch_release(unsigned int size, unsigned int preference)
{
    unsigned int priority;
    unsigned int *reserved;
    int new_size;
    taskid task = get_current_task();
    mem_alloc_info *alloc_info;
    scratch_per_task_data *ptd = lookup_per_task_data(task);

    patch_fn_shared(mem_utils);

    if (ptd == NULL)
    {
        /* Must be registered first */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
    }

    priority = GET_TASK_PRIORITY(ptd->task_id);

    switch (preference)
    {
    case MALLOC_PREFERENCE_DM1:
        alloc_info = &per_prio_data[priority].alloc_info_dm1;
        reserved = &ptd->total_reserved_dm1;
        break;
    case MALLOC_PREFERENCE_DM2:
        alloc_info = &per_prio_data[priority].alloc_info_dm2;
        reserved = &ptd->total_reserved_dm2;
        break;
    case MALLOC_PREFERENCE_NONE:
    case MALLOC_PREFERENCE_FAST:
    default:
        alloc_info = &per_prio_data[priority].alloc_info_none;
        reserved = &ptd->total_reserved_none;
        break;
    }

    if ((task == per_prio_data[priority].last_task) &&
        (alloc_info->total_committed != 0))
    {
        /* This task has committed scratch, it can't release it yet */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
    }

    new_size = (int)(*reserved) - size;

    if (new_size >= 0)
    {
        *reserved = new_size;
        return TRUE;
    }
    else
    {
        /* Attempted to release more than was reserved */
        panic_diatribe(PANIC_AUDIO_SCRATCH_MEMORY_BAD_REQUEST, task);
#ifdef DESKTOP_TEST_BUILD
        return FALSE;
#endif
    }
}

/*
 * scratch_commit
 */
void *scratch_commit(unsigned int size, unsigned int preference)
{
    taskid task = get_current_task();
    mem_alloc_info *alloc_info;
    unsigned int priority = GET_TASK_PRIORITY(task);
    unsigned int new_commit;
    void *result = NULL;

    patch_fn_shared(mem_utils);

    switch (preference)
    {
    case MALLOC_PREFERENCE_DM1:
        alloc_info = &per_prio_data[priority].alloc_info_dm1;
        break;
    case MALLOC_PREFERENCE_DM2:
        alloc_info = &per_prio_data[priority].alloc_info_dm2;
        break;
    case MALLOC_PREFERENCE_NONE:
    case MALLOC_PREFERENCE_FAST:
    default:
        alloc_info = &per_prio_data[priority].alloc_info_none;
        break;
    }

    new_commit = alloc_info->total_committed + size;

    if (new_commit > alloc_info->alloc_size)
    {
        panic(PANIC_AUDIO_SCRATCH_MEMORY_OVERCOMMITTED);
    }
    else
    {
        result = (char *)alloc_info->base + alloc_info->total_committed;
        alloc_info->total_committed = new_commit;
    }

    return result;
}

/*
 * scratch_free
 */
void scratch_free(void)
{
    taskid task = get_current_task();
    unsigned int priority = GET_TASK_PRIORITY(task);
    per_prio_data[priority].alloc_info_dm1.total_committed = 0;
    per_prio_data[priority].alloc_info_dm2.total_committed = 0;
    per_prio_data[priority].alloc_info_none.total_committed = 0;
    per_prio_data[priority].last_task = NO_TASK;
}

