/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file
 * header for the internals of the PM memory allocation system
 *
 * This file contains bits of the PM memory system that we don't want
 * users to see.
 */

#ifndef MALLOC_PM_PRIVATE_H
#define MALLOC_PM_PRIVATE_H

/****************************************************************************
Include Files
*/
#include "malloc_pm.h"
#include "types.h"
#include "platform/pl_trace.h"
#include "platform/pl_intrinsics.h"
#include "panic/panic.h"
#include "fault/fault.h"
#include "io_map.h"

/****************************************************************************
Public Macro Declarations
*/

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/
/* Linker provides the calculated heapsize after
 * calculating the available free RAM
 */
extern unsigned _pm_heap_size;
#if defined(INSTALL_DUAL_CORE_SUPPORT) && defined(AUDIO_SECOND_CORE)
extern unsigned _pm_p1_heap_size;
#endif
/****************************************************************************
Public Function Prototypes
*/

/**
 * NAME
 *   init_heap_pm
 *
 * \brief Initialise memory heap
 *
 * \param[in] reservedSize number of bytes (see notes for what that means) reserved for
 * other systems (e.g. patchpoints) to be subtracted from P0's heap
 *
 *
 * \note
 *   The reservedSize is specified in units of the smallest addressable storage unit
 *   in PM on the processor used. This is 8 bits on KAL_ARCH4 and KAL_ARCH5 or 32 bits
 *   on KAL_ARCH3.
 *
 */
void init_heap_pm(unsigned int reservedSize);

/**
 * NAME
 *   heap_alloc_pm
 *
 * \brief Memory allocation using heap
 *
 * FUNCTION
 *   Allocate a chunk of memory from the heap.
 *   Returns a Null pointer if no suitable block is available.
 *   The memory is not initialised.
 *
 * \param[in] size number of addressable units required.
 * \param[in] preference_core core in which we want to allocate the memory
 *
 * \return pointer to the block of memory allocated
 *
 * \note
 *   The size is specified in units of the smallest addressable storage unit
 *   in PM on the processor used. This is 8 bits on KAL_ARCH4 and KAL_ARCH5 or 32 bits
 *   on KAL_ARCH3.
 *
 */
void_func_ptr heap_alloc_pm(unsigned size_byte, unsigned preference_core);


/**
 * NAME
 *   heap_free_pm
 *
 * \brief Free memory allocated from the PM heap
 *
 * \param[in] ptr pointer to the memory to be freed
 *
 * \return None
 *
 */
void heap_free_pm(void_func_ptr ptr);

/**
 * NAME
 *   heap_sizeof_pm
 *
 * \brief Get size of memory block allocated from the heap
 *
 * \param[in] ptr pointer to the memory
 *
 * \return Actual available size
 *
 */
unsigned heap_sizeof_pm(void_func_ptr ptr);

/**
 * NAME
 *   is_in_pm_heap
 *
 * \brief Returns whether provided address is within the boundaries
 * of the PM heap
 *
 * \param[in] ptr pointer to the memory
 *
 * \return True if it is. False otherwise.
 *
 */
bool is_in_pm_heap(void_func_ptr);

#endif /* MALLOC_PM_PRIVATE_H */


