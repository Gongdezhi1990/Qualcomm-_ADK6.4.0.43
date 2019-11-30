/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file  memory_table.c
 * \ingroup memory_table
 *
 *  Utility functions for bulk memory allocations based on a table. Routines
 *  are provided for both shared and non shared allocations.
 *
 */

/****************************************************************************
Include Files
*/
#include "memory_table.h"
#include "shared_memory.h"
#include "scratch_memory.h"
#include "patch/patch.h"

/****************************************************************************
Private Macro Declarations
*/
#ifdef PMALLOC_DEBUG
#define XPMALLOC xppmalloc_debug
#define XZPMALLOC xzppmalloc_debug
#else /* !PMALLOC_DEBUG */
#define XPMALLOC xppmalloc
#define XZPMALLOC xzppmalloc
#endif /* PMALLOC_DEBUG */

/****************************************************************************
Private Type Declerations
*/
/** malloc function prototype */
#ifdef PMALLOC_DEBUG
typedef void *(* malloc_func)(unsigned int, unsigned int, const char *file, unsigned int line);
#else
typedef void *(* malloc_func)(unsigned int, unsigned int);
#endif /* PMALLOC_DEBUG */
/** shared malloc function prototype */
typedef void *(* shared_malloc_func)(unsigned int, unsigned int, unsigned int, bool *);

/****************************************************************************
Private Function Definitions
*/

/**
 * \brief Allocate the memory requested in the table. If all the memory can't be
 * allocated it is all released. The caller specifies the malloc function to use.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the allocated memory.
 * \param malloc_table Table of memory allocations to make.
 * \param length The length of the table.
 * \param alloc The malloc function to use for making the allocations.
 *
 * \return Whether the allocations were successful or not.
 */
static bool mem_table_common_alloc(void* struc, const malloc_t_entry* malloc_table,
                                                unsigned length, malloc_func alloc)
{
    unsigned i;
    void *entry_addr;

    patch_fn_shared(mem_utils);

    for (i = 0; i < length; i++)
    {
        /* Store the pointer to the allocated buffer in the position of the
         * data structure as specified by the alloc table fields */
        entry_addr = (void *)((uintptr_t)struc +
                                            (unsigned)(malloc_table[i].offset));
        *((void **)entry_addr) = alloc(sizeof(unsigned) * malloc_table[i].size,
                malloc_table[i].preference
#ifdef PMALLOC_DEBUG
                ,__FILE__, __LINE__
#endif
                );

        if(NULL == *((void **)entry_addr))
        {
            /* Free all the memory previously allocated.*/
            mem_table_free(struc, malloc_table, i);
            return FALSE;
        }
    }
    return TRUE;
}

/**
 * \brief Allocate and zero the memory requested in the table. If
 * all the memory can't be allocated it is all released. The caller specifies
 * the shared memory malloc function to use for the allocations.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the allocated memory.
 * \param shared_malloc_table Table of memory allocations to make.
 * \param length The length of the table.
 * \param new_allocation pointer to a boolean value. This will be populated with
 *        the logical value of the block being newly allocated.
 * \param s_malloc The shared memory malloc function to use for the allocations
 *
 * \return Whether the allocations were successful or not.
 */
static bool mem_table_common_alloc_shared(void *struc,
        const share_malloc_t_entry *shared_malloc_table, unsigned length,
        bool *new_allocation, shared_malloc_func s_malloc)
{
    unsigned i;
    bool new_alloc;
    void *entry_addr;

    patch_fn_shared(mem_utils);

    for (i = 0, *new_allocation = FALSE; i < length; i++)
    {
        /* Store the pointer to the allocated buffer in the position of the
         * data structure as specified by the alloc table fields */
        entry_addr = (void *)((uintptr_t)struc +
                                        (unsigned)shared_malloc_table[i].offset);
        *((void **)entry_addr) = s_malloc(
                                sizeof(unsigned) * shared_malloc_table[i].size,
                                shared_malloc_table[i].preference,
                                shared_malloc_table[i].id,
                                &new_alloc);

        if(NULL == *((void **)entry_addr))
        {
            /* Free all the shared memory previously allocated.*/
            mem_table_free_shared(struc, shared_malloc_table, i);
            return FALSE;
        }

        /* Update the new allocation incase this is the first new alloc in the
         * table */
        *new_allocation = *new_allocation || new_alloc;
    }

    return TRUE;
}


/****************************************************************************
Public Function Definitions
*/
/*
 * mem_table_alloc_shared
 */
bool mem_table_alloc_shared(void *struc,
        const share_malloc_t_entry *shared_malloc_table, unsigned int length,
        bool *new_allocation)
{
    return mem_table_common_alloc_shared(struc, shared_malloc_table, length,
            new_allocation, shared_malloc);
}
/*
 * mem_table_zalloc_shared
 */
bool mem_table_zalloc_shared(void *struc,
        const share_malloc_t_entry *shared_malloc_table, unsigned int length,
        bool *new_allocation)
{
    return mem_table_common_alloc_shared(struc, shared_malloc_table, length,
            new_allocation, shared_zmalloc);
}

/*
 * mem_table_free_shared
 */
void mem_table_free_shared(void* struc,
                const share_malloc_t_entry* shared_malloc_table, unsigned length)
{
    unsigned i;
    void *entry_addr;

    for (i = 0; i < length; i++)
    {
        entry_addr = (void *)((uintptr_t)struc +
                                        (unsigned)shared_malloc_table[i].offset);
        if (*((void **)entry_addr) != NULL)
        {
            shared_free(*((void **)entry_addr));
            *((void **)entry_addr) = NULL;
        }
    }
}

/*
 * mem_table_alloc
 */
bool mem_table_alloc(void* struc, const malloc_t_entry* malloc_table,
                                                                unsigned length)
{
    return mem_table_common_alloc(struc, malloc_table, length, XPMALLOC);
}

/*
 * mem_table_zalloc
 */
bool mem_table_zalloc(void* struc, const malloc_t_entry* malloc_table,
                                                                unsigned length)
{
    return mem_table_common_alloc(struc, malloc_table, length, XZPMALLOC);
}

/*
 * mem_table_free
 */
void mem_table_free(void* struc, const malloc_t_entry* malloc_table,
                                                                unsigned length)
{
    unsigned i;
    void *entry_addr;

    for (i = 0; i < length; i++)
    {
        entry_addr = (void *)((uintptr_t)struc +
                                            (unsigned)(malloc_table[i].offset));
        pfree(*((void **)entry_addr));
        *((void **)entry_addr) = NULL;
    }
}

/*
 * mem_table_scratch_reserve
 */
bool mem_table_scratch_reserve(const scratch_malloc_t_entry *scratch_alloc_table, unsigned length, unsigned preference)
{
    unsigned i;

    for (i = 0; i < length; i++)
    {
        /* Store the pointer to the allocated buffer in the position of the
         * data structure as specified by the alloc table fields */

        if (!scratch_reserve(sizeof(unsigned) * scratch_alloc_table[i].size, preference))
        {
            /* Free all the memory previously allocated.*/
            mem_table_scratch_release(scratch_alloc_table, i, preference);
            return FALSE;
        }
    }
    return TRUE;
}

bool mem_table_scratch_tbl_reserve(const scratch_table* sc_table)
{
    patch_fn_shared(mem_utils);

    if (sc_table->dm1_length != 0)
    {
        if (!mem_table_scratch_reserve(sc_table->dm1_scratch_table, sc_table->dm1_length, MALLOC_PREFERENCE_DM1))
        {
            return FALSE;
        }
    }
    if (sc_table->dm2_length != 0)
    {
        if (!mem_table_scratch_reserve(sc_table->dm2_scratch_table, sc_table->dm2_length, MALLOC_PREFERENCE_DM2))
        {
            scratch_table free_table = *sc_table;
            free_table.dm2_length = 0;
            free_table.no_pref_length = 0;
            mem_table_scratch_tbl_release(&free_table);
            return FALSE;
        }
    }
    if (sc_table->no_pref_length != 0)
    {
        if(!mem_table_scratch_reserve(sc_table->no_pref_scratch_table, sc_table->no_pref_length, MALLOC_PREFERENCE_FAST))
        {
            scratch_table free_table = *sc_table;
            free_table.no_pref_length = 0;
            mem_table_scratch_tbl_release(&free_table);
            return FALSE;
        }
    }
    return TRUE;
}

/*
 * mem_table_scratch_release
 */
void mem_table_scratch_release(const scratch_malloc_t_entry* scratch_alloc_table, unsigned length, unsigned preference)
{
    unsigned i;

    for (i = 0; i < length; i++)
    {
        scratch_release(sizeof(unsigned) * scratch_alloc_table[i].size, preference);
    }
}

void mem_table_scratch_tbl_release(const scratch_table* scratch_table)
{
    if (scratch_table->dm1_length != 0)
    {
        mem_table_scratch_release(scratch_table->dm1_scratch_table, scratch_table->dm1_length, MALLOC_PREFERENCE_DM1);
    }
    if (scratch_table->dm2_length != 0)
    {
        mem_table_scratch_release(scratch_table->dm2_scratch_table, scratch_table->dm2_length, MALLOC_PREFERENCE_DM2);
    }
    if (scratch_table->no_pref_length != 0)
    {
        mem_table_scratch_release(scratch_table->no_pref_scratch_table, scratch_table->no_pref_length, MALLOC_PREFERENCE_FAST);
    }
}

/*
 * mem_table_scratch_commit
 */

void mem_table_scratch_commit(void* struc, const scratch_malloc_t_entry* scratch_alloc_table, unsigned length, unsigned preference)
{
    unsigned i, tot_commit_size;
    void * mem;
    void **entry_addr;

    for (tot_commit_size = scratch_alloc_table[0].size, i = 1; i < length; i++)
    {
        tot_commit_size += scratch_alloc_table[i].size;
    }

    tot_commit_size *= sizeof(unsigned);

    mem = scratch_commit(tot_commit_size, preference);

    entry_addr = (void **)((uintptr_t)struc + (unsigned)(scratch_alloc_table[0].offset));

    if (*entry_addr != mem)
    {
        *entry_addr = mem;
        for (i = 1; i < length; i++)
        {
            mem = (unsigned *)(mem) + (unsigned)(scratch_alloc_table[i-1].size);
            /* Store the pointer to the allocated buffer in the position of the
             * data structure as specified by the alloc table fields */
            entry_addr = (void **)((uintptr_t)struc +
                                                (unsigned)(scratch_alloc_table[i].offset));
            *entry_addr = mem;

        }
    }

}

void mem_table_scratch_tbl_commit(void* struc, const scratch_table* scratch_table)
{
    patch_fn_shared(mem_utils);

    if (scratch_table->dm1_length != 0)
    {
        mem_table_scratch_commit(struc, scratch_table->dm1_scratch_table, scratch_table->dm1_length, MALLOC_PREFERENCE_DM1);
    }
    if (scratch_table->dm2_length != 0)
    {
        mem_table_scratch_commit(struc, scratch_table->dm2_scratch_table, scratch_table->dm2_length, MALLOC_PREFERENCE_DM2);
    }
    if (scratch_table->no_pref_length != 0)
    {
        mem_table_scratch_commit(struc, scratch_table->no_pref_scratch_table, scratch_table->no_pref_length, MALLOC_PREFERENCE_FAST);
    }
}

