/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup mem_utils Memory utils
 *
 * \defgroup shared_mem Shared memory
 * \ingroup mem_utils
 *
 * \file shared_memory.h
 * \ingroup shared_mem
 *
 */

#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include "types.h"

/**
 * \brief  Function for allocating shared memory.
 *         Returns a pointer to a block of the specified size,
 *         allocating it if no existing allocation with the same id value exists.
 *
 * \param  size of the shared memory (in memory native addressing units).
 * \param  preference the preferred location of the shared memory. (for ex. DM1).
 * \param  id the id of the shared memory.
 * \param  new_allocation pointer to the boolean value which
 *         let the caller know whether the memory may need initialising
 *
 * \return pointer to the shared memory or NULL if the allocation failed.
 */
void *shared_malloc(unsigned int size, unsigned int preference, unsigned int id, bool *new_allocation);

/**
 * \brief  Function for allocating shared memory.
 *         Returns a pointer to a block of the specified size,
 *         allocating it if no existing allocation with the same id value exists.
 *         In case of a new allocation sets the allocated memory to zero.
 *
 * \param  size of the shared memory (in memory native addressing units).
 * \param  preference the preferred location of the shared memory. (for ex. DM1).
 * \param  id the id of the shared memory.
 * \param  new_allocation pointer to the boolean value which
 *         let the caller know whether the memory may need initialising
 *
 * \return pointer to the shared memory or NULL if the allocation failed.
 */
void *shared_zmalloc(unsigned int size, unsigned int preference, unsigned int id, bool *new_allocation);

/**
 * \brief  Releases one use of the shared memory pointed to by pmem.
 *
 * \param  pmem pointer to the shared memory.
 *
 * \return void.
 */
void shared_free(void *pmem);

/**
 * \brief  Releases one use of the shared memory with the given id value.
 *
 * \param  id the id of the shared memory.
 *
 * \return void
 */
void shared_free_by_id(unsigned int id);
int shared_id(void *pmem);

#endif /* SHARED_MEMORY_H */
