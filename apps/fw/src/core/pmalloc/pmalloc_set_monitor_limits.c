/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Set free block monitoring limits
 *
 */

#include "pmalloc/pmalloc_private.h"


#ifdef PMALLOC_MONITOR_POOLS
/**
 * Set free block monitoring limits
 */
void pmalloc_set_monitor_limits(size_t size_low, size_t size_high)
{
    const pmalloc_pool *pools_end = pmalloc_pools + pmalloc_num_pools;
    const pmalloc_pool *pool;
    size_t blocks = 0;

    /* Find the first pool to monitor */
    for (pool = pmalloc_pools;
         (pool < pools_end) && (pool->size < size_low);
         ++pool)
        /* LINTED empty loop body */
        {}
    pmalloc_monitored_pool_low = pool;

    PMALLOC_BLOCK_INTERRUPTS();

    /* Find the last pool to monitor and count the number of free blocks */
    for (; (pool < pools_end) && (pool->size < size_high); ++pool)
        blocks += pool->blocks - pool->allocated;
    pmalloc_monitored_pool_high = pool - 1;

    /* Store the count of currently free blocks in the monitored pools */
    pmalloc_monitored_pools = blocks;

    PMALLOC_UNBLOCK_INTERRUPTS();
}
#else
int suppress_no_external_declaration_in_translation_unit_whinges;
#endif /* PMALLOC_MONITOR_POOLS */
