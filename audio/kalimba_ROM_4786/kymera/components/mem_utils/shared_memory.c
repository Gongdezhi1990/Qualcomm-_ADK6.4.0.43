/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file shared_memory.c
 * \ingroup shared_mem
 *
 * This file contains the functions that handles the creation of
 * shared memory.
 *
 */

 /****************************************************************************
Include Files
*/
#include "pmalloc/pl_malloc.h"
#include "platform/pl_intrinsics.h"
#include "panic/panic.h"
#include "fault/fault.h"
#include "shared_memory.h"
#include "patch/patch.h"
#ifdef SHARED_MEMORY_TEST
#include "platform/pl_assert.h"
#endif

/****************************************************************************
Private Constant Declarations
*/
/** The number of entries each shared memory table should contain. */
#define NO_TABLE_ENTRIES 15
/** The number of bits to allocate for recording the number of users for a
 * memory allocation.
 */
#define NO_BITS_FOR_USERS 8
/** The maximum number of users that can share a memory allocation. */
#define MAX_NO_USERS ((1 << NO_BITS_FOR_USERS) - 1)

/****************************************************************************
Private Type Declarations
*/
typedef struct
{
    /**
     * This field represents the number of users this shared memory has.
     * Restricted to MAX_NO_USERS.
     */
    unsigned int nr_of_users:NO_BITS_FOR_USERS;
    /**
     * Id of the shared memory. Restricted to 16 bits
     */
    unsigned int id:16;
    /**
     * Size of the shared memory. Restricted to an allocation of 16,383.
     */
    unsigned int size:15;
    /**
     * Preference for the shared memory.
     */
    unsigned int preference:9;
    /**
     * Pointer to the shared memory.
     */
    void *pmem;
} SHARED_MEMORY_ENTRY;

/** Table structure that shared memory entries are stored in. This is a list of tables */
typedef struct SHARED_MEMORY_TABLE
{
    /** A table of 10 shared memory entries. */
    SHARED_MEMORY_ENTRY table[NO_TABLE_ENTRIES];

    /** The next table if this one became full */
    struct SHARED_MEMORY_TABLE *next;
} SHARED_MEMORY_TABLE;


/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Private Variable Definitions
*/
/** The beginning of the shared memory allocation tables */
static SHARED_MEMORY_TABLE* shared_memory_list=NULL;


/****************************************************************************
Private Function Definitions
*/

/**
 * \brief  Gets the next free entry in the shared memory table. This will malloc
 * a new table entry if all the existing tables are full.
 *
 * NOTE: This function expects to be called with Interupts blocked.
 *
 * \return Pointer to a free entry to populate. NULL if no space could be found.
 */
static SHARED_MEMORY_ENTRY *get_next_free_entry(void)
{
    SHARED_MEMORY_ENTRY *shmem;
    SHARED_MEMORY_TABLE *cur_table = shared_memory_list;
    unsigned i;

    if (NULL == cur_table)
    {
        cur_table = shared_memory_list = xzpnew(SHARED_MEMORY_TABLE);
        if (NULL == cur_table)
        {
            /* Insufficient memory for the action. */
            return NULL;
        }
    }
    /* Search each table for an empty entry and return it. If there isn't a free
     * entry malloc a new table. If this fails return NULL */
    do
    {
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
            shmem = &(cur_table->table[i]);
            if(NULL == shmem->pmem)
            {
                return shmem;
            }
        }
        if (NULL == cur_table->next)
        {
            cur_table->next = xzpnew(SHARED_MEMORY_TABLE);
        }
        cur_table = cur_table->next;
    } while (cur_table != NULL);

    /* Insufficient memory for the action. */
    return NULL;
}

/**
 * \brief  Function for releasing shared memory tables when they are no longer
 *      in use.
 *
 * \param  released pointer to the entry that has just been released.
 */
static void notify_entry_release(SHARED_MEMORY_ENTRY *released)
{
    SHARED_MEMORY_TABLE **cur_table, *temp;
    unsigned i;

    LOCK_INTERRUPTS;
    for(cur_table = &shared_memory_list; *cur_table != NULL; cur_table = &((*cur_table)->next))
    {
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
            if(&((*cur_table)->table[i]) == released)
            {
                /* Found the table containing the memory being released if
                 * none of the entries are allocated then we can free it. */
                for (i = 0; i < NO_TABLE_ENTRIES; i++)
                {
                    if (NULL != (*cur_table)->table[i].pmem)
                    {
                        UNLOCK_INTERRUPTS;
                        return;
                    }
                }
                temp = *cur_table;
                *cur_table = (*cur_table)->next;
                pdelete(temp);
                UNLOCK_INTERRUPTS;
                return;
            }
        }
    }
    /* It's possible that some other free beat us to freeing the table. So we
     * can just continue. */
    UNLOCK_INTERRUPTS;
}

/**
 * \brief  Function for allocating shared memory object.
 *         Returns a pointer to the shared memory object.
 *
 *         NOTE: This function expects to be called with Interupts blocked.
 *
 * \param  size the size of the shared memory.
 * \param  preference the preferred location of the shared memory. (for ex. DM1).
 * \param  id the id of the user.
 * \param  set_to_zero parameter used to determine if the newly allocated shared memory will be zeroed.
 *
 * \return pointer to the shared memory or NULL if the allocation failed.
 */
static void *create_new_shared_mem(unsigned int size, unsigned int preference, unsigned int id, bool set_to_zero)
{
    SHARED_MEMORY_ENTRY *shmem;
    /* Create the new shared memory object */
    shmem = get_next_free_entry();
    if (shmem == NULL)
    {
        /* Not enough memory, return null. */
        return NULL;
    }

    /* Allocate the memory */
    if (set_to_zero)
    {
        shmem->pmem = xzppmalloc(size, preference);
    }
    else
    {
        shmem->pmem = xppmalloc(size, preference);
    }
    if (shmem->pmem == NULL)
    {
        /* Release this memory object allocated previously and return null. */
        notify_entry_release(shmem);
        return NULL;
    }

    /* initialize rest of the object*/
    shmem->id = id;
    shmem->size = size;
    shmem->preference = preference;
    shmem->nr_of_users = 1;

    return shmem;
}

/**
 * \brief looks up a shared memory entry for a given shared memory id.
 *
 * \param id the Identifier of the shared memory to search for.
 *
 * \return Pointer to the shared memory entry associated with the id.
 * Null if not found.
 */
static SHARED_MEMORY_ENTRY *find_shared_memory_from_id(unsigned int id)
{
    SHARED_MEMORY_ENTRY *shmem;
    SHARED_MEMORY_TABLE *cur_table;
    unsigned i;

    for (cur_table = shared_memory_list; cur_table != NULL; cur_table = cur_table->next)
    {
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
            shmem = &(cur_table->table[i]);
            /* The entry is only valid if pmem is not NULL so if the IDs match
             * we check that also as 0 is a valid ID and the table is zero
             * initialised! */
            if (shmem->id == id)
            {
                if (NULL != shmem->pmem)
                {
                    return shmem;
                }
            }
        }
    }
    return NULL;
}

/**
 * shmem_decrement_users
 */
static unsigned int shmem_decrement_users(SHARED_MEMORY_ENTRY *shmem)
{
    return --shmem->nr_of_users;
}

/**
 * \brief This function implements the shared memory allocation for both (set and not set to zero) cases.
 *
 * \param  size the size of the shared memory.
 * \param  preference the preferred location of the shared memory. (for ex. DM1).
 * \param  id the id of the user.
 * \param  new_allocation pointer to the boolean value which
 *         let the caller know whether the memory may need initialising
 * \param set_to_zero True if the newly allocated memory should be zeroed. False otherwise
 *
 * \return pointer to the shared memory or NULL if the allocation failed.
 */
static void *shared_memory_malloc(unsigned int size, unsigned int preference, unsigned int id, bool *new_allocation, bool set_to_zero)
{
    SHARED_MEMORY_ENTRY *shmem;

    patch_fn_shared(mem_utils);

    /* Sanity check before we do anything. */
    if (size == 0)
    {
        panic_diatribe(PANIC_AUDIO_REQ_ZERO_MEMORY, id);
    }

    LOCK_INTERRUPTS;

    if ((shmem = find_shared_memory_from_id(id)) != NULL)
    {
        if ((shmem->size == size) && (shmem->preference == preference))
        {
            /* Increment the number of users and return the memory pointer.
             * Check that we haven't hit the limit as only NO_BITS_FOR_USERS are
             * available to store the value. */
            if (shmem->nr_of_users == MAX_NO_USERS)
            {
                UNLOCK_INTERRUPTS;
                fault_diatribe(FAULT_AUDIO_SHARED_MEM_USER_LIMIT, shmem->id);
                return NULL;
            }
            ++shmem->nr_of_users;
            UNLOCK_INTERRUPTS;
            *new_allocation = FALSE;
            return shmem->pmem;
        }
        else
        {
            UNLOCK_INTERRUPTS;
            /* Same shared memory id but incompatible size or memory location*/
            panic_diatribe(PANIC_AUDIO_SHARED_MEM_INVALID_MALLOC, id);
#ifdef SHARED_MEMORY_TEST
            /* under the test panic does not block */
            return shmem->pmem;
#endif
        }

    }
    /* There is no shared memory with this id
     * create new one
     */
    shmem = create_new_shared_mem(size, preference, id, set_to_zero);
    UNLOCK_INTERRUPTS;
    if (shmem != NULL){
        *new_allocation = TRUE;
        return shmem->pmem;
    }
    else
    {
        /* Not enough memory, return NULL. */
        return NULL;
    }
}
/****************************************************************************
Public Function Definitions
*/
/*
 * shared_malloc
 */
void *shared_malloc(unsigned int size, unsigned int preference, unsigned int id, bool *new_allocation){
    return shared_memory_malloc(size, preference, id, new_allocation, FALSE);
}

/*
 * shared_zmalloc
 */
void *shared_zmalloc(unsigned int size, unsigned int preference, unsigned int id, bool *new_allocation){
    return shared_memory_malloc(size, preference, id, new_allocation, TRUE);
}

/*
 * shared_free
 */
void shared_free(void *pmem)
{
    SHARED_MEMORY_ENTRY *shmem;
    SHARED_MEMORY_TABLE *cur_table;
    unsigned i;

    /* The API is to silently do nothing if a NULL pointer is freed, the same
     * as the malloc library */
    if (pmem == NULL)
    {
        return;
    }

    for(cur_table = shared_memory_list; cur_table != NULL; cur_table = cur_table->next)
    {
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
            shmem = &(cur_table->table[i]);
            if(shmem->pmem == pmem)
            {
                /* found it */
                if (shmem_decrement_users(shmem) == 0)
                {
                    /* releasing the allocated memory */
                    pfree(shmem->pmem);
                    /* release the shared memory entry.*/
                    shmem->id = 0;
                    shmem->preference = 0;
                    shmem->size = 0;
                    shmem->pmem = NULL; /* Do this last as the Table won't be freed until all pmems are NULL */
                    notify_entry_release(shmem);
                }
                return;
            }
        }
    }

    if (NULL == cur_table)
    {
        /* No shared memory was found with this pointer. */
        panic_diatribe(PANIC_AUDIO_SHARED_MEM_FREE_INVALID_POINTER, (DIATRIBE_TYPE)((uintptr_t)pmem));
    }

}

/*
 * shared_free_by_id
 */
void shared_free_by_id(unsigned int id)
{
    SHARED_MEMORY_ENTRY *shmem;
    shmem = find_shared_memory_from_id(id);

    if (shmem != NULL)
    {
        shared_free(shmem->pmem);
    }
    else
    {
        /* No shared memory was found with this ID. */
        panic_diatribe(PANIC_AUDIO_SHARED_MEM_FREE_INVALID_ID, id);
    }
}

/**
 * Functions for the dynamic memory loader
 */
int shared_id(void *pmem)
{
    SHARED_MEMORY_ENTRY *shmem;
    SHARED_MEMORY_TABLE *cur_table;
    unsigned i;

    for(cur_table = shared_memory_list; cur_table != NULL; cur_table = cur_table->next)
    {
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
            shmem = &(cur_table->table[i]);
            if(shmem->pmem == pmem)
            {
               return shmem->id;
            }
        }
    }

    return -1;
}

/**
 * Functions for the unit test.
 */
#ifdef SHARED_MEMORY_TEST

#include "shared_memory_test.h"

/**
 * Function return the number of shared memory users.
 * If there is no shared memory with the Id given returns -1
 */
int shared_memory_users_from_id(unsigned int id)
{
    SHARED_MEMORY_ENTRY *shmem = find_shared_memory_from_id(id);
    if (shmem != NULL)
    {
        return shmem->nr_of_users;
    }
    return -1;
}

unsigned int no_shared_memory_records(void)
{
    SHARED_MEMORY_TABLE *cur_table;
    unsigned i, cnt = 0;

    for(cur_table = shared_memory_list; cur_table != NULL; cur_table = cur_table->next)
    {
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
                cnt++;
        };
    }
    /*PL_PRINT_P1(TR_PL_TEST_TRACE, "\nShared memory records in existent: %d\n", cnt);*/
    return cnt;
}
/**
 * Displays all the shared memory objects.
 */
void show_shared_memory(void)
{
    SHARED_MEMORY_TABLE *cur_table;
    SHARED_MEMORY_ENTRY *shmem;
    unsigned i;
    PL_PRINT_P1(TR_PL_TEST_TRACE, "\n \tshared_memory_list=%x\n", (unsigned int)(uintptr_t) shared_memory_list);
    for(cur_table = shared_memory_list; cur_table != NULL; cur_table = cur_table->next)
    {
        PL_PRINT_P0(TR_PL_TEST_TRACE, "\n\t\tStart of table\n");
        for (i = 0; i < NO_TABLE_ENTRIES; i++)
        {
            shmem = &(cur_table->table[i]);
            PL_PRINT_P5(TR_PL_TEST_TRACE, "\t\telement id=%d, size=%3d, mem=%x nr_of_users=%d, preference=%d\n",
                            shmem->id,shmem->size, (unsigned int)(uintptr_t) shmem->pmem, shmem->nr_of_users, shmem->preference);
        }
        PL_PRINT_P1(TR_PL_TEST_TRACE, "\n\t\tEnd of table. Next table %x\n",
                                    (unsigned int)(uintptr_t) cur_table->next);
    }
}

#endif
