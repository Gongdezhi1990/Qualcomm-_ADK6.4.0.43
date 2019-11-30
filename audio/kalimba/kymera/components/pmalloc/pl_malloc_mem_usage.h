/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file pl_malloc_mem_usage.h
 * Interface for reading the memory usage of the system.
 *
 * \ingroup platform
 *
 ****************************************************************************/

#if !defined(PL_MALLOC_USAGE_H)
#define PL_MALLOC_USAGE_H

/****************************************************************************
Include Files
*/
#include "platform/pl_intrinsics.h"

/****************************************************************************
Public Macro Declarations
*/

#ifdef HEAP_DEBUG
/**
 * Currently available heap memory.
 */
#define heap_cur() ((heap_debug_free) >> LOG2_ADDR_PER_WORD)

/**
 * Minimum available heap. The minimum available memory is also called memory watermarks.
 */
#define heap_min() ((heap_debug_min_free) >> LOG2_ADDR_PER_WORD)

/**
 * Clear the minimum available heap memory. In other words, the minimum available heap
 * memory will be equal to the currently available heap memory.
 */
#define heap_clear_watermarks() \
    do                                                                  \
    {                                                                   \
        LOCK_INTERRUPTS;                                                \
        heap_debug_min_free = heap_debug_free;                          \
        UNLOCK_INTERRUPTS;                                              \
    }                                                                   \
    while(0)

#else

/**
 * Dummy macros in case the heap is not enabled for the build.
 */
#define heap_cur()              (-1)
#define heap_min()              (-1)
#define heap_clear_watermarks() ((void)0)

#endif

/* Pool size is defined in words. So just report it */
#define pool_size() total_pool_size

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/
/*
 * Expose memory usage variables used in the macros.
 */
#ifdef HEAP_DEBUG
extern unsigned heap_debug_free, heap_debug_min_free;
#endif
extern const unsigned total_pool_size;

/****************************************************************************
Public Function Prototypes
*/
/*
 * Expose memory usage interface functions from the pool memory module
 */
extern unsigned pool_cur(void);
extern unsigned pool_min(void);
extern void pool_clear_watermarks(void);

extern unsigned heap_size(void);



#endif /* PL_MALLOC_USAGE_H */
