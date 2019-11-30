/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * How many blocks can be allocated
 *
 */

#include "pmalloc/pmalloc_private.h"


/**
 * How many blocks can be allocated
 *
 * IMPLEMENTATION NOTE
 *
 * Interrupts are not blocked by this function. This is safe providing the
 * "allocated" field can be read atomically, which is likely to be the case
 * on all target platforms.
 */
size_t pmalloc_available(size_t size)
{
    const pmalloc_pool *pools_end = pmalloc_pools + pmalloc_num_pools;
    const pmalloc_pool *pool;
    size_t n = 0;

    /* Count the number of blocks that can service a request for the
       specified size */
    for (pool = pmalloc_pools; pool < pools_end; ++pool)
        if (size <= pool->size)
            n += pool->blocks - pool->allocated;

    /* Return the number of blocks */
    return n;
}
