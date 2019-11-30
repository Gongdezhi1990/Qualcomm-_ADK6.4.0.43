/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup memory_table Memory table
 * \ingroup mem_utils
 *
 * \file  memory_table.h
 * \ingroup memory_table
 *
 * \brief
 * Utility functionality for bulk memory allocations based on a table. Routines
 * are provided for both shared and non shared allocations. Table structure
 * Declerations are provided for simplicity. <br>
 */

#ifndef MEMORY_TABLES_H
#define MEMORY_TABLES_H

/****************************************************************************
Include Files
*/
#include "types.h"
#include "pmalloc/pl_malloc.h"

/****************************************************************************
Public Type Declerations
*/
/** structure of a malloc table entry */
typedef struct
{
    /** The size of the allocation */
    uint16 size;
    /** The memory bank preference of the allocation */
    uint16 preference;
    /** The offset in the structure of the field that points to the
     * allocation */
    uint16 offset;
} malloc_t_entry;

/** structure of a malloc table entry for shared memory allocations */
typedef struct
{
    /** The size of the allocation */
    uint16 size;
    /** The memory bank preference of the allocation */
    uint16 preference;
    /** The shared memory id of this allocation */
    uint16 id;
    /** The offset in the structure of the field that points to the
     * allocation */
    uint16 offset;
} share_malloc_t_entry;

/** structure of a scratch allocation table entry. The table must be for a single malloc preference */
typedef struct
{
    /** The size of the allocation */
    uint16 size;
    /** The offset in the structure of the field that points to the
     * allocation */
    uint16 offset;
} scratch_malloc_t_entry;


typedef struct
{
    uint16 dm1_length;
    uint16 dm2_length;
    uint16 no_pref_length;

    const scratch_malloc_t_entry *dm1_scratch_table;

    const scratch_malloc_t_entry *dm2_scratch_table;

    const scratch_malloc_t_entry *no_pref_scratch_table;
}scratch_table;

/****************************************************************************
Public Function Declerations
*/

/**
 * \brief Allocate the memory requested in the table. If all the memory can't be
 * allocated it is all released.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the allocated memory.
 * \param malloc_table Table of memory allocations to make.
 * \param length The length of the table.
 *
 * \return Whether the allocations were successful or not.
 */
extern bool mem_table_alloc(void *struc, const malloc_t_entry *malloc_table,
                            unsigned length);

/**
 * \brief Allocate and zero the memory requested in the table. If
 * all the memory can't be allocated it is all released.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the allocated memory.
 * \param malloc_table Table of memory allocations to make.
 * \param length The length of the table.
 *
 * \return Whether the allocations were successful or not.
 */
extern bool mem_table_zalloc(void *struc, const malloc_t_entry *malloc_table,
                            unsigned length);

/**
 * \brief Free memory used by a codec object based upon the provided
 * malloc table.
 *
 * \param struc A pointer to the data structure that uses the allocations.
 * \param malloc_table Table of memory allocations that were made.
 * \param length The length of the table.
 */
extern void mem_table_free(void *struc,
                        const malloc_t_entry *malloc_table, unsigned length);

/**
 * \brief Share memory between two objects via a shared malloc table. If
 * all the memory can't be allocated it is all released.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the allocated memory.
 * \param shared_malloc_table Table of shared memory allocations to make.
 * \param length The length of the table.
 * \param new_allocation return value indicating if one or more of the
 * allocations was a new allocation.
 *
 * \return Whether the allocations were successful or not.
 */
extern bool mem_table_alloc_shared(void *struc,
            const share_malloc_t_entry *shared_malloc_table, unsigned length,
            bool *new_allocation);

/**
 * \brief Share memory between two  objects via a shared malloc table. If
 * all the memory can't be allocated it is all released. This function zeroes
 * the allocated memory if the allocation is new.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the allocated memory.
 * \param shared_malloc_table Table of shared memory allocations to make.
 * \param length The length of the table.
 * \param new_allocation return value indicating if one or more of the
 * allocations was a new allocation.
 *
 * \return Whether the allocations were successful or not.
 */
extern bool mem_table_zalloc_shared(void *struc,
            const share_malloc_t_entry *shared_malloc_table, unsigned length,
            bool *new_allocation);
/**
 * \brief Free shared memory used by a codec object based upon the provided
 * malloc table.
 *
 * \param struc A pointer to the data structure that uses the allocations.
 * \param shared_malloc_table Table of memory allocations that were made.
 * \param length The length of the table.
 */
extern void mem_table_free_shared(void *struc,
            const share_malloc_t_entry *shared_malloc_table, unsigned length);


/**
 * \brief Reserve the scratch memory requested in the table. If all the memory
 * can't be reserved it is all released.
 *
 * \param malloc_table Table of scratch memory reservations to make.
 * \param length The length of the table.
 * \param preference malloc preference of the allocations
 *
 * \return Whether the reservations were successful or not.
 */
extern bool mem_table_scratch_reserve(const scratch_malloc_t_entry *scratch_alloc_table,
                            unsigned length, unsigned preference);

extern bool mem_table_scratch_tbl_reserve(const scratch_table* scratch_table);

/**
 * \brief Release scratch memory reserved by an object based upon the provided
 * malloc table.
 *
 * \param shared_malloc_table Table of memory allocations that were made.
 * \param length The length of the table.
 * \param preference malloc preference of the allocations
 */
extern void mem_table_scratch_release(const scratch_malloc_t_entry* scratch_alloc_table,
                            unsigned length, unsigned preference);

extern void mem_table_scratch_tbl_release(const scratch_table* scratch_table);

/**
 * \brief Get the scratch pointer for the memory requested in the table.
 *
 * \param struc A pointer to the data structure to populate with
 * pointers to the scratch memory.
 * \param malloc_table Table of memory commits to make.
 * \param length The length of the table.
 * \param preference malloc preference of the allocations
 *
 */
extern void mem_table_scratch_commit(void* struc, const scratch_malloc_t_entry* scratch_alloc_table,
                                    unsigned length, unsigned preference);

extern void mem_table_scratch_tbl_commit(void* struc, const scratch_table* scratch_table);

#endif /* MEMORY_TABLES_H */
