/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Allocate a block of private memory
 *
 */

#include "pmalloc/pmalloc_private.h"

#if defined(MEMORY_PROFILING)
#include "debug/debug.h"
#endif /* MEMORY_PROFILING */

/* pmalloc length is currently stored in half of a uint16
so limit checks to 0xff*/
#define MAX_PMALLOC_LENGTH_CHECK 0xff
/**
 * Allocate a block of private memory
 */
void *PMALLOC_TRACE_FN(xpmalloc, size_t size, pmalloc_owner_ref owner)
{
    void *ptr;
    const pmalloc_pool *pools_end = pmalloc_pools + pmalloc_num_pools;
    pmalloc_pool *pool;
    
#ifdef PMALLOC_RECORD_LENGTHS
    size_t requested_size = size;
    
#ifdef PMALLOC_BLOCK_OVERRUN_CHECK    
    size_t extra_size = requested_size; /*Doubling in case of PMALLOC_BLOCK_OVERRUN_CHECK*/


    if(size <= MAX_PMALLOC_LENGTH_CHECK) /* Can't store lengths > 255 */
    {       
#ifdef PMALLOC_BLOCK_OVERRUN_CHECK_SHORT
        extra_size = PMALLOC_OVERRUN_SIZE; /*Adding PMALLOC_OVERRUN_SIZE octets extra in case of PMALLOC_BLOCK_OVERRUN_CHECK_SHORT*/
#endif
        size = requested_size + extra_size;
    }
#endif
#endif

    /* Update the count of blocks requested of this size  */
#ifdef PMALLOC_RECORD_REQUESTS
    if (size <= PMALLOC_MAX_DEBUG_SIZE)
    {
        ++pmalloc_requests[size];
    }
#endif

    PMALLOC_DEBUG_VALIDATE_POOL_CONTROL();

    /* Determine the pool that contains the smallest blocks that can service
       this request. This relies on the fact that the pools are stored in
       order of increasing size. Since the block size in the pools never
       change, this search can be peformed with interrupts enabled.

       This unconventional use of a do-while block allows "continue" to be
       used like a "goto" without using the dreaded keyword. Blame sms. */
    do
    {
        /* Initialising here helps the compiler to generate better code */
        pool = pmalloc_pools;

        /* Unroll the first few iterations of the search loop for speed */
        /*lint -e{960} continue statement used */
        if (6 <= pmalloc_num_pools)
        {
            if (pool[0].size < size)
            {
                if (pool[1].size < size)
                {
                    if (pool[2].size < size)
                    {
                        if (pool[3].size < size)
                        {
                            if (pool[4].size < size)
                            {
                                if (pool[5].size < size)
                                {
                                    pool += 6; /* Continue search below */
                                }
                                else
                                {
                                    pool += 5;
                                    continue;
                                }
                            }
                            else
                            {
                                pool += 4;
                                continue;
                            }
                        }
                        else
                        {
                            pool += 3;
                            continue;
                        }
                    }
                    else
                    {
                        pool += 2;
                        continue;
                    }
                }
                else
                {
                    pool += 1;
                    continue;
                }
            }
            else
            {
                continue;
            }
        }

        /* Check whether any pool contains large enough blocks to avoid
           performing two comparisons per pool in the following loop */
        if (pools_end[-1].size < size)
        {
            return NULL;
        }
        /* Find the first of the remaining pools that contains sufficiently
           large blocks. It is not necessary to check for the end of the pool
           control block array because the preceding check ensures that there
           is a pool with large enough blocks. */
        while (pool->size < size)
        {
            ++pool;
        }
        /* CONSTANTCONDITION */
    } while (0);

    /* Find the first pool that has free blocks */
    do {
        PMALLOC_BLOCK_INTERRUPTS();

        /* Check whether there are any blocks on the free list (scrambling the
           NULL value rather than unscrambling the free list pointer enables
           the scrambling/unscrambling to be performed at compile time) */
        if (pool->free != PMALLOC_SCRAMBLE_ADDRESS(NULL))
        {
            /* Unlink this block from the pool's free list */
            ptr = PMALLOC_UNSCRAMBLE_ADDRESS(pool->free);
            pool->free = * (void **) ptr;

#ifdef RESOURCE_MODULE_PRESENT
            /* Publish an updated resource status if block is large enough */
            if (PMALLOC_RESOURCE_SIZE <= pool->size)
            {
                --pmalloc_resource_free;
                if (pmalloc_resource_free <= PMALLOC_RESOURCE_CRITICAL)
                {
                    resource_set_pmalloc_status(resource_critical);
                }
                else if (pmalloc_resource_free <= PMALLOC_RESOURCE_LOW)
                {
                    resource_set_pmalloc_status(resource_low);
                }
                /*lint --e{961} no action required if level still normal */
            }
#endif

#ifdef PMALLOC_MONITOR_POOLS
            /* Update count of free blocks in the monitored range of pools */
            if ((pmalloc_monitored_pool_low <= pool)
                && (pool <= pmalloc_monitored_pool_high))
            {
                --pmalloc_monitored_pools;
            }
#endif

            /* Update the count of allocated blocks in this pool */
            ++(pool->allocated);
#ifdef PMALLOC_STATS
            if (pool->max_allocated < pool->allocated)
            {
                pool->max_allocated = pool->allocated;
            }
#endif

#ifdef PMALLOC_RECORD_USAGE_LEVEL
            pmalloc_current_bytes_out += pool->size;
            if (pmalloc_highest_bytes_out < pmalloc_current_bytes_out)
            {
                pmalloc_highest_bytes_out = pmalloc_current_bytes_out;
            }
#endif
            PMALLOC_DEBUG_VALIDATE_FREE_LIST(pool, ptr);

            PMALLOC_UNBLOCK_INTERRUPTS();

            /* Check that the block pointer lies within this pool */
#ifdef PMALLOC_ADDRESS_CHECK
            if ((ptr < (pool == pmalloc_pools
                        ? pmalloc_blocks
                        : pool[-1].pool_end))
                || (pool->pool_end <= ptr))
            {
                panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
            }
#endif

            PMALLOC_DEBUG_CHECK_BLOCK(ptr, pool->size);

#if defined(PMALLOC_TRACE_OWNER_ANY) || defined(PMALLOC_RECORD_LENGTHS)
            {
                size_t n;
                pmalloc_pool *pool2;

                /* Convert the pointer into an absolute block number */
                n = 0;
                for (pool2 = pmalloc_pools; pool2 < pool; ++pool2)
                {
                    n += pool2->blocks;
                }
                n += (size_t)(((char *) ptr - (char *) (pool == pmalloc_pools
                                               ? pmalloc_blocks
                                               : pool[-1].pool_end)))
                    / pool->size;

                if (n < PMALLOC_MAX_DEBUG_BLOCKS)
                {
#ifdef PMALLOC_TRACE_OWNER_ANY
                    /* Required to keep the enum definition in the KCC DWARF */
                    enum PC_TRACE_DETAILS shift_amount = PC_TRACE_SHIFT;
                    /* Store the owner for this block */
                    pmalloc_owner[n] =
                        (pmalloc_owner_internal_ref)(owner >> shift_amount);
#endif /* PMALLOC_TRACE_OWNER_ANY */
#ifdef PMALLOC_RECORD_LENGTHS
                    /* Store the requested length for this block */
                    if(requested_size <= MAX_PMALLOC_LENGTH_CHECK)
                    {
                        PMALLOC_BLOCK_INTERRUPTS();
                        PMALLOC_SET_LENGTH(n, requested_size);
                        PMALLOC_UNBLOCK_INTERRUPTS();
#ifdef PMALLOC_BLOCK_OVERRUN_CHECK
                        /* put in our guard values to check during free */
                        memset((uint8*)ptr + requested_size, 
                                PMALLOC_GUARD_VALUE,
                                extra_size);
#endif /* PMALLOC_BLOCK_OVERRUN_CHECK*/

                    }
#endif
                }
            }
#endif


#if defined(MEMORY_PROFILING)
            {
                static uint16 counter=0;
                ++counter;
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
                DBG_MSG4("%08d MEM pmalloc.c:%d A %d, %d", counter, owner, (uint16) ptr, size);
#else
                DBG_MSG3("%08d MEM pmalloc.c:0000 A %d, %d", counter, (uint16) ptr, size);
#endif /* PMALLOC_TRACE_OWNER_PC_ONLY */
            }
#endif /* MEMORY_PROFILING */
            /* Block successfully allocated */
            return ptr;
        }

        PMALLOC_UNBLOCK_INTERRUPTS();

        /* Update the count of times that the allocation request overflows to
           the next pool */
#ifdef  PMALLOC_STATS
        ++(pool->overflows);
#endif

    } while (++pool < pools_end);

    /* No free blocks if this point reached */
    return NULL;
}
