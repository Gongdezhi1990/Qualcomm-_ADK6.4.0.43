/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Return a block of private memory to the pool
 *
 */

#include "pmalloc/pmalloc_private.h"

#if defined(MEMORY_PROFILING)
#include "debug/debug.h"
#endif /* MEMORY_PROFILING */

#define IS_REMOTE_DM_RAM(ptr) MEMORY_MAP_IS_P0_DM_RAM(ptr)

/**
 * Return a block of private memory to the pool
 */
void pfree(void *ptr)
{
#if defined(PFREE_CHECK_ALREADY_FREED)
    void *free_block_pointer;
#endif

#ifdef PFREE_BURY_BLOCK
    void **free_ptr;
    int shovelfuls;
#endif

#ifdef PMALLOC_RECORD_LENGTHS
    size_t n;
    pmalloc_pool *pool2;
#endif

    pmalloc_pool *pool;

#ifdef IPC_MODULE_PRESENT
    /* For convenience we allow the processors not to care about whether a
     * pmalloc block was allocated locally or remotely. */
    if (IS_REMOTE_DM_RAM(ptr))
    {
        ipc_send_pfree(ptr);
        return;
    }
#endif

    PMALLOC_DEBUG_VALIDATE_POOL_CONTROL();

#if defined(MEMORY_PROFILING)
    {
        static uint16 counter=0;
        ++counter;
        DBG_MSG2("%08d MEM pfree.c:0000 F %d", counter, (uint16) ptr);
    }
#endif /* MEMORY_PROFILING */
    /* Determine the pool that contains the specified pointer. Since the pool
       boundaries never change, this search can be peformed with interrupts
       enabled. */
    if (pmalloc_blocks <= ptr)
    {
        /* This unconventional use of a do-while block allows "continue" to be
           used like a "goto" without using the dreaded keyword. Blame sms. */
        do
        {
            /* Initialising here helps the compiler to generate better code */
            pool = pmalloc_pools;

            /* Unroll the first few iterations of the search loop for speed */
            /*lint -e{960} continue statement used */
            if (6 <= pmalloc_num_pools)
            {
                if (pool[0].pool_end <= ptr)
                {
                    if (pool[1].pool_end <= ptr)
                    {
                        if (pool[2].pool_end <= ptr)
                        {
                            if (pool[3].pool_end <= ptr)
                            {
                                if (pool[4].pool_end <= ptr)
                                {
                                    if (pool[5].pool_end <= ptr)
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

            /* Check whether the pointer lies within any pool to avoid
               performing two comparisons per pool in the following loop */
            if (pmalloc_pools[pmalloc_num_pools - 1].pool_end <= ptr)
            {
                panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
            }
            /* Find which of the remaining pools contains the pointer. It is
               not necessary to check for the end of the pool control block
               array because the preceding check ensures that the pointer
               lies within one of the pools. */
            while (pool->pool_end <= ptr)
            {
                ++pool;
            }
            /* CONSTANTCONDITION */
        } while(0);
    }
    else if (ptr == NULL)
    {
        /* No action required for NULL pointers */
        return;
    }
    else
    {
        /* Pointer to location before start of the pools */
        panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
    }

    /* The supplied pointer may be within the middle of a block, e.g. if a
       pointer had been walked up a PDU held within the block, and the
       modified pointer was used to free the block. Hence, the pointer needs
       to be rounded down to the base of the block. This is a waste of time
       if the this functionality is not required...
    */
    ptr = (void *) ((char *) ptr
                    - ((size_t) ((char *) ptr
                                 - (char *) (pool == pmalloc_pools
                                             ? pmalloc_blocks
                                             : pool[-1].pool_end))
                       % pool->size));

#ifdef PMALLOC_RECORD_LENGTHS
    /* Convert the pointer into an absolute block number */
    n = 0;
    for (pool2 = pmalloc_pools; pool2 < pool; ++pool2)
    {
        n += pool2->blocks;
    }
    n += (size_t)(((char *)ptr - (char *)(pool == pmalloc_pools
                                 ? pmalloc_blocks
                                 : pool[-1].pool_end)))
        / pool->size;
#endif /*PMALLOC_RECORD_LENGTHS*/

    PMALLOC_BLOCK_INTERRUPTS();

#ifdef PMALLOC_BLOCK_OVERRUN_CHECK
    if(n < PMALLOC_MAX_DEBUG_BLOCKS)
    {
        size_t requested_size = PMALLOC_GET_LENGTH(n);

        if(requested_size != 0) /* if it's zero it hasn't been initialised yet */
        {
            uint8* location_being_checked = (uint8*)ptr + requested_size;
            
#ifdef PMALLOC_BLOCK_OVERRUN_CHECK_SHORT
            size_t extra_size = PMALLOC_OVERRUN_SIZE;
#else
            size_t extra_size = requested_size;            
#endif            
            uint8* end_location = location_being_checked + extra_size;

            while(location_being_checked < end_location)
            {
                if(*location_being_checked != PMALLOC_GUARD_VALUE)
                {
                    /*lint --e{123} Ignore the ugly clash between pmalloc the
                    * _MODULE_NAME_ and pmalloc the function-like macro.
                    */
                    L1_DBG_MSG1("Warning! free'd memory block starting at 0x%08x has been overrun.",
                                (uint32)ptr);
                    L1_DBG_MSG2("Error at 0x%08x. Length requested = 0x%x",
                                (uint32)location_being_checked,
                                requested_size);
                    break;
                }
                location_being_checked++;
            }
        }
    }
#endif /*PMALLOC_BLOCK_OVERRUN_CHECK*/

#ifdef PMALLOC_RECORD_LENGTHS
    /* Clear the length stored for this block */
    if (n < PMALLOC_MAX_DEBUG_BLOCKS)
    {
        PMALLOC_SET_LENGTH(n, 0);
    }
#endif

    PFREE_DEBUG_VALIDATE_FREE_LIST(pool, ptr);

#if defined(PFREE_CHECK_ALREADY_FREED)
    /** Check if the block to be released is already included in the pool free list.
     * When a block is added more then once to the free list, once the first instance
     * is used for malloc it breaks the liked list of free blocks.
     */
    free_block_pointer =  PMALLOC_UNSCRAMBLE_ADDRESS(&pool->free);

    while (free_block_pointer)
    {
        if (ptr == free_block_pointer)
        {
            panic(PANIC_HYDRA_FREE_ALREADY_DEALLOCATED_MEMORY);
        }

        free_block_pointer = PMALLOC_UNSCRAMBLE_ADDRESS(*((void**)free_block_pointer));
    }
#endif /* defined(PFREE_CHECK_ALREADY_FREED) */

    /* Add the block to the free list for this pool */
#ifdef PFREE_BURY_BLOCK
    /* Walk along the free list to bury this block to the required depth */
    free_ptr = &pool->free;
    for (shovelfuls = PFREE_BURIAL_DEPTH; shovelfuls; --shovelfuls)
    {
        void *next = PMALLOC_UNSCRAMBLE_ADDRESS(*free_ptr);
        if (!next)
        {
            break;
        }
        free_ptr = next;
    }

    /* Insert this block at the selected position in the free list */
    PFREE_SET_FREE_LIST_PTR(ptr, *free_ptr, pool->size);
    if (free_ptr != &pool->free)
    {
        PMALLOC_DEBUG_CHECK_BLOCK((void *) free_ptr, pool->size);
        PFREE_SET_FREE_LIST_PTR((void *) free_ptr,
                                (void *) PMALLOC_SCRAMBLE_ADDRESS(ptr),
                                pool->size);
    }
    else
        pool->free = PMALLOC_SCRAMBLE_ADDRESS(ptr);
#else
    /* Add the block to the head of the pool's free list.
       The PFREE_SET_FREE_LIST_PTR macro is roughly "*ptr = pool->free"
       (with appropriate casts), but may do other things when debugging. */
    PFREE_SET_FREE_LIST_PTR(ptr, pool->free, pool->size);
    pool->free = PMALLOC_SCRAMBLE_ADDRESS(ptr);
#endif

    /* Update the count of allocated blocks in this pool */
#ifdef PMALLOC_POOL_CONTROL_CHECK
    if (!pool->allocated)
    {
        panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
    }
#endif
    --(pool->allocated);

#ifdef PMALLOC_RECORD_USAGE_LEVEL
    /* Take off the memory used from the count */
    pmalloc_current_bytes_out -= pool->size;
#endif

#ifdef PMALLOC_MONITOR_POOLS
    /* Update the count of free blocks in the monitored range of pools */
    if ((pmalloc_monitored_pool_low <= pool)
        && (pool <= pmalloc_monitored_pool_high))
    {
        ++pmalloc_monitored_pools;
    }
#endif

#ifdef RESOURCE_MODULE_PRESENT
    /* Publish an updated resource status if block is large enough */
    if (PMALLOC_RESOURCE_SIZE <= pool->size)
    {
        ++pmalloc_resource_free;
        if (PMALLOC_RESOURCE_LOW < pmalloc_resource_free)
        {
            resource_set_pmalloc_status(resource_normal);
        }
        else if (PMALLOC_RESOURCE_CRITICAL < pmalloc_resource_free)
        {
            resource_set_pmalloc_status(resource_low);
        }
        /*lint --e{961} no action required if level still critical */
    }
#endif

    PMALLOC_UNBLOCK_INTERRUPTS();
}

void pdestroy_array(void **ptr, uint16 num)
{
    while (num--)
    {
        pfree(*ptr);
        *ptr = NULL;
        ptr++;
    }
}


