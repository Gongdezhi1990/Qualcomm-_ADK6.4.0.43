/* Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Initialse the private memory allocator
 *
 */

#include "pmalloc/pmalloc_private.h"
#include "hydra_log/hydra_log.h"

/** For production builds, pmalloc_begin and pmalloc_end
 * are extern-defined, by the linker.
 * For unit-testing builds, this is not available and 
 * we statically allocate here, of size PMALLOC_RAM_SIZE
 * Note: if NO_PMALLOC_MEMMAP_SYMBOLS is not def'd in the
 * build, you MUST allocate unused memory from the linker,
 * between pmalloc_begin and pmalloc_end */
#ifdef NO_PMALLOC_MEMMAP_SYMBOLS
static uint16 pmalloc_begin[PMALLOC_RAM_SIZE];
static char *pmalloc_end = (char *)pmalloc_begin + (PMALLOC_RAM_SIZE*sizeof(uint16));
#endif

#ifdef RESOURCE_MODULE_PRESENT
/** Resource management status */
size_t pmalloc_resource_free;
#endif

/** Array of pool control blocks */
size_t pmalloc_num_pools;
pmalloc_pool *pmalloc_pools;

/** Pointer to the first block in the first pool */
void *pmalloc_blocks;

#ifdef PMALLOC_MONITOR_POOLS
/** Monitoring of number of free blocks in a range of pools */
const pmalloc_pool *pmalloc_monitored_pool_low;
const pmalloc_pool *pmalloc_monitored_pool_high;
size_t pmalloc_monitored_pools;
#endif

/** Record the requested size for all allocated blocks */
#ifdef PMALLOC_RECORD_LENGTHS
uint16 pmalloc_length[(PMALLOC_MAX_DEBUG_BLOCKS + 1) / 2];
#endif

/** Record the total number of requests for blocks of different size */
#ifdef PMALLOC_RECORD_REQUESTS
uint16 pmalloc_requests[PMALLOC_MAX_DEBUG_SIZE + 1];
#endif

/** Record ownership information for each of the blocks if tracing enabled */
#ifdef PMALLOC_TRACE_OWNER_ANY
pmalloc_owner_internal_ref pmalloc_owner[PMALLOC_MAX_DEBUG_BLOCKS];
#endif

/** Record current and high watermark of actual memory usage */
#ifdef PMALLOC_RECORD_USAGE_LEVEL
uint16 pmalloc_current_bytes_out;
uint16 pmalloc_highest_bytes_out;
#endif

 /**
 * Partition the memory into pools based on "configuration". This sets the
 * main characteristics of the pool control blocks ("size", "blocks" and
 * "pool_end"), but does not initialise the free list or statistics
 * collection.
 * Acts directly on pool control blocks pointed to by pmalloc_begin
 */
void pmalloc_configure(pmalloc_pool_config *pool_config, unsigned pool_config_len)
{
    pmalloc_pool *pools_end;
    pmalloc_pool *pool;
    char *hwm;
    char *new_hwm;
    unsigned i;
    size_t size;
    size_t blocks;
    pmalloc_pool_config* pool_config_ptr;
     
    /* The pool control blocks are placed at the start of the memory to be
       allocated (with safe alignment enforced) */
    pmalloc_pools = (pmalloc_pool *) PMALLOC_ALIGN(pmalloc_begin);
    pools_end = pmalloc_pools;
    hwm = (char *) pmalloc_pools;
    
    /* Create pool control blocks for as much of the configuration as fits
       (but not necessarily in the final order). */
    pmalloc_num_pools = 0;
    pool_config_ptr = pool_config;    
    for (i = 0; i < pool_config_len; ++i, ++pool_config_ptr)
    {
        /* Check whether a new pool needs to be created */
        size = pool_config_ptr->size;
        blocks = pool_config_ptr->blocks;
        
        if (size)
        {
             new_hwm = ((char *) PMALLOC_ALIGN(hwm + sizeof(pmalloc_pool))) 
                       + size * blocks;
             /* Check whether there is space for the new pool
              * to be created. If not, panic because there should
              * be, as this was validated in get_pmalloc_config() */               
             if ((void *) new_hwm <= (void *) pmalloc_end)
             {
               /* Create a control block for this pool */
               pools_end->size = size;
               pools_end->blocks = blocks;
               ++pools_end;
               ++pmalloc_num_pools;
               hwm = new_hwm;                               
             }
             else
             {
                panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_CONFIG);
             }
        }      
    }

    /* Set end address of each pool
     * Done separately, because the first iteration of hwm,
     * interleaved control-blocks with data
     * Start at the next alignment boundary after the
     *  end of the pool control blocks */
    pmalloc_blocks = PMALLOC_ALIGN(pools_end);
    hwm = (char *)pmalloc_blocks;

    for (pool = pmalloc_pools; pool < pools_end; ++pool)
    {
        /* Set the end address for the current pool */
        hwm += pool->size * pool->blocks;
        hwm = PMALLOC_ALIGN(hwm);
        pool->pool_end = hwm;
    }
}

/**
  NAME
  init_pmalloc  -  initialse the private memory allocator
*/
void init_pmalloc(void)
{
    const pmalloc_pool *pools_end;
    pmalloc_pool *pool;
    char *hwm;
    pmalloc_pool_config config_pools[MAX_NUM_POOLS];
    unsigned pool_config_len;

    /* Get the pool configuration list (collated from several sources) */
    if (!get_pmalloc_config(config_pools, &pool_config_len))
    {
        panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_CONFIG);
    }

    pmalloc_configure(config_pools,pool_config_len);
    
    /* Cache a pointer to the end of the pools
       (have to do this after the pools have been configured) */
    pools_end = pmalloc_pools + pmalloc_num_pools;

    /* Start by pretending that all of the blocks are allocated */
    for (pool = pmalloc_pools; pool < pools_end; ++pool)
    {
        pool->free = PMALLOC_SCRAMBLE_ADDRESS(NULL);
        pool->allocated = pool->blocks;
#ifdef PMALLOC_STATS
        pool->max_allocated = 0;
        pool->overflows = 0;
#endif
    }

    /* Populate the free lists by calling pfree() on each block */
    hwm = (char *)pmalloc_blocks;
    for (pool = pmalloc_pools; pool < pools_end; ++pool)
    {
        while ((void *)hwm < pool->pool_end)
        {
#ifndef HAVE_32BIT_DATA_WIDTH
            /* Initial magic num 0xDEAD to track unused blocks is
             * written to memory after "next block" pointer.
             * XAP has 16 bit chars so pmalloc "next block" is hwm[0] */
            hwm[1] = (char)0xDEAD;
#else
            /* We have octet access, so we need to put our magic number
             * at hwm[4] and [5] because our "next block" pointer is at [0:3]*/
            if(pool->size > 4)
            {
                hwm[4] = (char)0xDE;
                hwm[5] = (char)0xAD;
            }
#endif
            pfree(hwm);
            hwm += pool->size; /*lint !e449 not really pfree'd above */
        }
    }

#ifdef PMALLOC_RECORD_USAGE_LEVEL
    pmalloc_current_bytes_out = 0;
    pmalloc_highest_bytes_out = 0;
#endif
    /*lint -save -e774 we *expect* all the Booleans to be false! */

    /*lint -restore */
}
