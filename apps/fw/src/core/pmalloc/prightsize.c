/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Is a block just the right size
 */

#include "pmalloc/pmalloc_private.h"

/**
 * Is a block just the right size
 */
size_t prightsize(const void *ptr, size_t size)
{
    const pmalloc_pool *pools_end = pmalloc_pools + pmalloc_num_pools;
    const pmalloc_pool *pool;

    /** Not a pool block if before the start of the first pool */
    if (ptr < pmalloc_blocks)
    {
        panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
    }
    /** Find which pool contains the pointer */
    for (pool = pmalloc_pools; pool < pools_end; ++pool)
    {
        if (ptr < pool->pool_end)
        {
            if ((size <= pool->size)
                && ((pool == pmalloc_pools) || (pool[-1].size < size)))
            {
                return 0;
            }
            else
            {
                return pool->size;
            }
        }
    }

    /** Not a pool block if after the end of the last pool */
    panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
}
