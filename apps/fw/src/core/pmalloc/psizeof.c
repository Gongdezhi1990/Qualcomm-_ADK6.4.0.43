/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Determine the size of a block of private memory
 */

#include "pmalloc/pmalloc_private.h"

/**
 * Determine the size of a block of private memory
 */
size_t psizeof(const void *ptr)
{
    const pmalloc_pool *pools_end = pmalloc_pools + pmalloc_num_pools;
    const pmalloc_pool *pool;

    /* Not a pool block if before the start of the first pool */
    if (ptr < pmalloc_blocks)
    {
        return 0;
    }
    /* Find the pool containing the supplied pointer */
    for (pool = pmalloc_pools; pool < pools_end; ++pool)
    {
        if (ptr < pool->pool_end)
        {
            return pool->size;
        }
    }
    /* Not a pool block if after the end of the last pool */
    return 0;
}
