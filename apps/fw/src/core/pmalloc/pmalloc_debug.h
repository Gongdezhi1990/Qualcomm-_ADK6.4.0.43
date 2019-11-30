/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Internal header file for the private memory allocator
 */

#ifndef PMALLOC_DEBUG_H
#define PMALLOC_DEBUG_H

#include "pmalloc/pmalloc.h"


/**
 * Some debugging options use fixed size arrays indexed by either the block
 * size or block number. The following constants set the size of these
 * arrays. Note that the actual pools may exceed these limits, but debugging
 * data will only be maintained for operations within them. */
#ifdef PMALLOC_BLOCK_OVERRUN_CHECK

#ifndef PMALLOC_RECORD_LENGTHS
#define PMALLOC_RECORD_LENGTHS
#endif

#define PMALLOC_MAX_DEBUG_BLOCKS (800) /* need to store all sizes when
                                        checking for overrun */
#define PMALLOC_GUARD_VALUE     0xaa
#else

#define PMALLOC_MAX_DEBUG_BLOCKS (450)

#endif /* PMALLOC_BLOCK_OVERRUN_CHECK */

#ifdef PMALLOC_BLOCK_OVERRUN_CHECK_SHORT
#define PMALLOC_OVERRUN_SIZE 4
#endif /* PMALLOC_BLOCK_OVERRUN_CHECK_SHORT */

#define PMALLOC_MAX_DEBUG_SIZE (100)


/** Enable groups of checks */

#ifdef PMALLOC_EXTREME_CHECKS
#ifndef PMALLOC_SLOW_CHECKS
#define PMALLOC_SLOW_CHECKS
#endif
#ifndef PFREE_BURY_BLOCK
#define PFREE_BURY_BLOCK
#endif
#endif

#ifdef PMALLOC_SLOW_CHECKS
#ifndef PMALLOC_FAST_CHECKS
#define PMALLOC_FAST_CHECKS
#endif
#ifndef PMALLOC_FREE_LIST_CHECK
#define PMALLOC_FREE_LIST_CHECK
#endif
#ifndef PFREE_FREE_LIST_CHECK
#define PFREE_FREE_LIST_CHECK
#endif
#ifndef PMALLOC_POOL_BLOCK_CHECK
#define PMALLOC_POOL_BLOCK_CHECK
#endif
#ifndef PMALLOC_SCRAMBLE_FREE_LIST_POINTERS
#define PMALLOC_SCRAMBLE_FREE_LIST_POINTERS
#endif
#endif

#ifdef PMALLOC_FAST_CHECKS
#ifndef PMALLOC_ADDRESS_CHECK
#define PMALLOC_ADDRESS_CHECK
#endif
#ifndef PMALLOC_POOL_CONTROL_CHECK
#define PMALLOC_POOL_CONTROL_CHECK
#endif
#endif


/** Avoid placing recently freed blocks at the start of the free list */
#ifdef PFREE_BURY_BLOCK
#ifndef PFREE_BURIAL_DEPTH
#define PFREE_BURIAL_DEPTH (10)
#endif
#endif

/** Sanity check the free list for a single pool in xpmalloc() */
#ifdef PMALLOC_FREE_LIST_CHECK
#define PMALLOC_DEBUG_VALIDATE_FREE_LIST(pool, ptr) \
    pmalloc_debug_validate_free_list((pool), (ptr))
#else
#define PMALLOC_DEBUG_VALIDATE_FREE_LIST(pool, ptr) ((void) 0)
#endif

/** Sanity check the free list for a single pool in pfree() */
#ifdef PFREE_FREE_LIST_CHECK
#define PFREE_DEBUG_VALIDATE_FREE_LIST(pool, ptr) \
    pmalloc_debug_validate_free_list((pool), (ptr))
#else
#define PFREE_DEBUG_VALIDATE_FREE_LIST(pool, ptr) ((void) 0)
#endif

/** Check to writes to blocks that have been freed */
#ifdef PMALLOC_POOL_BLOCK_CHECK
#define PMALLOC_DEBUG_CHECK_BLOCK(ptr,size) \
    pmalloc_debug_check_block((ptr), (size))
#ifdef XAP
#define PFREE_SET_FREE_LIST_PTR(ptr,next,size) \
    memset((ptr), (int) (char *) (next), (size))
#else
#define PFREE_SET_FREE_LIST_PTR(ptr,next,size) \
    pfree_set_free_list_ptr((ptr), (next), (size))
#endif
#else
#define PMALLOC_DEBUG_CHECK_BLOCK(ptr,size) ((void) 0)
#define PFREE_SET_FREE_LIST_PTR(ptr,next,size) \
    (* (void **) (ptr) = next)
#endif

/** Store free list pointers in a scrambled form  */
#ifdef PMALLOC_SCRAMBLE_FREE_LIST_POINTERS
#ifndef PMALLOC_FREE_LIST_SCRAMBLER
#define PMALLOC_FREE_LIST_SCRAMBLER (0x9876)
#endif
#define PMALLOC_SCRAMBLE_ADDRESS(p) \
    ((void *) (((size_t) (char *) (p)) ^ PMALLOC_FREE_LIST_SCRAMBLER))
#else
#define PMALLOC_SCRAMBLE_ADDRESS(p) ((void *) (p))
#endif
#define PMALLOC_UNSCRAMBLE_ADDRESS(p) PMALLOC_SCRAMBLE_ADDRESS(p)

/** Sanity check all pool control blocks */
#ifdef PMALLOC_POOL_CONTROL_CHECK
#define PMALLOC_DEBUG_VALIDATE_POOL_CONTROL() \
    pmalloc_debug_validate_pool_control()
#else
#define PMALLOC_DEBUG_VALIDATE_POOL_CONTROL() ((void) 0)
#endif

/** Record the requested size for all allocated blocks
 * (maintained as an array of structures to allow efficient storage on
 * platforms without byte addressing) */
#ifdef PMALLOC_RECORD_LENGTHS
extern uint16 pmalloc_length[(PMALLOC_MAX_DEBUG_BLOCKS + 1) / 2];
#define PMALLOC_SET_LENGTH(n,s) \
    do                                                     \
    {                                                      \
        if((n) & 1)                                        \
        {                                                  \
            pmalloc_length[(n) >> 1] &= (uint16)(0xFF00);  \
            pmalloc_length[(n) >> 1] |= (uint16)(s & 0xFF);\
        }                                                  \
        else                                               \
        {                                                  \
            pmalloc_length[(n) >> 1] &= (uint16)(0x00FF);  \
            pmalloc_length[(n) >> 1] |= (uint16)((s & 0xFF) << 8);     \
        }                                                  \
    }while(0)

#define PMALLOC_GET_LENGTH(n) \
    (0xff & (((n) & 1) ? pmalloc_length[(n) >> 1] \
                       : (pmalloc_length[(n) >> 1] >> 8)))

#endif

/** Record the total number of requests for blocks of different size
 * (array indexed by requested block size) */
#ifdef PMALLOC_RECORD_REQUESTS
extern uint16 pmalloc_requests[PMALLOC_MAX_DEBUG_SIZE + 1];
#endif

/** Record ownership information for each of the blocks if tracing enabled */
#ifdef PMALLOC_TRACE_OWNER_ANY
extern pmalloc_owner_internal_ref pmalloc_owner[PMALLOC_MAX_DEBUG_BLOCKS];
#endif

/** Record current and high watermark of actual memory usage */
#ifdef PMALLOC_RECORD_USAGE_LEVEL
extern uint16 pmalloc_current_bytes_out;
extern uint16 pmalloc_highest_bytes_out;
#endif

/**
 * Check sanity of all pools
 *
 * Iterate over each pool control block making sure that there is nothing
 * obviously wrong.
 *
 * Currently the tests are:
 *
 *    - That the head of the free list for the pool control block is either
 *      NULL or is in the right pool.
 *
 *    - That the number of blocks taken from the pool does not exceed the
 *      total number of blocks in the pool.
 *
 *    - If statistics are available: That the maximum number of blocks taken
 *      from the pool does not exceed the number of blocks in the pool.
 */
extern void pmalloc_debug_validate_pool_control(void);


/**
 * Check sanity of a pool's free list
 *
 * Walk down the entire free list for the pool control block "pool",
 * checking that each entry is sane. In this context, sane means that the
 * free list entry is in this pool.
 *
 * The pointer "ptr" contains a block either just removed from the pool
 * (in xpmalloc) or just about to be added (in pfree). This function checks
 * that this pointer is not on the free list (to trap the same block being
 * freed twice).
 *
 * The function also checks that the free list is of the expected length
 * by comparing the position of the NULL with the value of "allocated".
 */
extern void pmalloc_debug_validate_free_list(const pmalloc_pool *pool,
                                             const void *ptr);


/**
 * Set pointer to next block on free list
 *
 * When a block is added to a pool's free list, a pointer to the next
 * entry of the free list is written to the start of the block. This
 * function extends this behaviour by duplicating the "next" pointer value
 * throughout the rest of the block.
 *
 * There are two reasons for doing this:
 *
 *    - It trashes the data in the block so attempts to read data from the
 *      block after freeing it are more likely to give an error.
 *
 *    - It allows the entire contents of the block to be checked before the
 *      block is reallocated, trapping writes that occurred after the free.
 *
 *  Note that if the block size is not an exact multiple of the size of a
 *  pointer then the left over space is not modified.
 */
extern void pfree_set_free_list_ptr(void *ptr, void *next, size_t size);


/**
 * Check a block has not been written to
 *
 * Check that the block pointed to by "ptr" of size "size" is filled with
 * the same pointer value. This should be used on a block that has been
 * previously filled by pfree_set_free_list_ptr().
 */
extern void pmalloc_debug_check_block(const void *ptr, size_t size);


#endif /* PMALLOC_DEBUG_H */
