/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup scratch_mem Scratch memory
 * \ingroup mem_utils
 *
 * \file scratch_memory.h
 * \ingroup scratch_mem
 *
 */

#ifndef SCRATCH_MEMORY_H
#define SCRATCH_MEMORY_H

#include "types.h"
#include "pmalloc/pl_malloc_preference.h"

/**
 * \brief  Register for use of scratch memory
 *
 * \note Call this function to inform the system that
 *       the current task wants to use scratch memory.
 *       This must be called before the task calls any of
 *       the other functions below.
 *
 * \return TRUE if the registration was successful, FALSE otherwise.
 */
extern bool scratch_register(void);

/**
 * \brief  De-register for use of scratch memory
 *
 * \note Call this function to inform the system that
 *       the current task no longer wants to use scratch memory.
 *       This must be called before the task is destroyed
 *       (or earlier if appropriate).
 *
 * \return TRUE if the deregistration was successful, FALSE otherwise.
 */
extern bool scratch_deregister(void);

/**
 * \brief  Reserve a scratch memory block.
 *
 * \param  size of the memory block requested (in memory native addressing units).
 * \param  preference (DM1/DM2 etc)
 *
 * \return TRUE if the reservation was successful, FALSE otherwise.
 */
extern bool scratch_reserve(unsigned int size, unsigned int preference);

/**
 * \brief  Release some scratch memory.
 *
 * \param  size of the memory block (in memory native addressing units).
 * \param  preference (DM1/DM2 etc)
 *
 * \return TRUE if successful, FALSE otherwise.
 */
extern bool scratch_release(unsigned int size, unsigned int preference);

/**
 * \brief  Commit a scratch memory block.
 *
 * \param  size of the memory block requested (in memory native addressing units).
 * \param  preference (DM1/DM2 etc)
 *
 * \return pointer to the committed memory.
 *         This is only valid until the task yields to the scheduler,
 *         or until the next call to scratch_reserve / scratch_release
 *         from the same task.
 */
extern void *scratch_commit(unsigned int size, unsigned int preference);

/**
 * \brief  Free committed scratch memory.
 *
 */
extern void scratch_free(void);

#endif /* SCRATCH_MEMORY_H */
