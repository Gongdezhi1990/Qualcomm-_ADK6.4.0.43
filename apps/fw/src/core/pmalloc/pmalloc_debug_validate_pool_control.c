/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Check sanity of all pools
 *
 */

#include "pmalloc/pmalloc_private.h"

/**
 * Check sanity of all pools
 */
void pmalloc_debug_validate_pool_control(void)
{
    const pmalloc_pool *pools_end = pmalloc_pools + pmalloc_num_pools;
    const pmalloc_pool *pool;
    const void *blocks_start = pmalloc_blocks;

    PMALLOC_BLOCK_INTERRUPTS();

    /* Iterate over all of the pool control blocks */
    for (pool = pmalloc_pools; pool < pools_end; ++pool)
    {
        const void *blocks_end = pool->pool_end;
        const void *free_ptr = PMALLOC_UNSCRAMBLE_ADDRESS(pool->free);

        /* Apply sanity checks to the pool control block's fields */
        if (((free_ptr != NULL) &&
             ((free_ptr < blocks_start) || (blocks_end <= free_ptr)))
            || (pool->blocks < pool->allocated)
#ifdef PMALLOC_STATS
            || (pool->blocks < pool->max_allocated)
#endif
            )
        {
            panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
        }
        /* The end of this pool is the start of the next */
        blocks_start = blocks_end;
    }

    PMALLOC_UNBLOCK_INTERRUPTS();
}
