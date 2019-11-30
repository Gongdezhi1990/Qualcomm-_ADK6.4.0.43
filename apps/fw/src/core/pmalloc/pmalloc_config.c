/* Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Calculate pmalloc pool configuration
 */

#include "pmalloc/pmalloc_private.h"
#include "pmalloc/pmalloc_config_P1.h"
#include "hydra_log/hydra_log.h"

/* After the explicit pools (from the config header) have been allocated (and
 * potentially combined) there is a second pass to use any 'spare' memory. The
 * definition places an upper limit on the pool sizes to be considered in this
 * pass.
 */
#define PM_MAX_SIZE_EXTRA 127

/* Pools with slightly different sizes will be combined if the sizes differ by
 * ratio < (1/POOL_COMBINE_FRAC_ERROR)
 * Keep this a power of two for efficiency, as it is used as a divisor.
 */
#define POOL_COMBINE_FRAC_ERROR 32

static unsigned int add_pmalloc_config(pmalloc_pool_config *pool_config,
                                       unsigned int pool_max,
                                       unsigned int pool_used,
                                       const pmalloc_pool_config *source_config,
                                       unsigned int source_entries);
static void sort_pmalloc_config(pmalloc_pool_config *pool_config, unsigned int config_len);
#ifndef PMALLOC_DONT_COMBINE_POOLS  
static void combine_pmalloc_sizes(pmalloc_pool_config *pool_config, unsigned *config_len);
#endif

/**
 * Construct fixed pool memory allocation configuration, based on summing the
 * contributions from known sources.
 *
 * Combine them if required, then allocate any remaining heap memory to the
 * pool configuration too.
 *
 * Returns true if successful, otherwise false.
 * pool_config_len updated with number of entries in pool_config.
 */
bool get_pmalloc_config(pmalloc_pool_config *pool_config, unsigned *pool_config_len)
{
    pmalloc_pool_config* pool_config_ptr;
    unsigned int pool_len;
    unsigned int i;
    size_t config_reqd;
    size_t heap_size;
#ifndef PMALLOC_FIXED_CONFIG_ONLY
    size_t config_avail;
    size_t tot_block_size = 0;
    size_t pcb_size;
#endif /*PMALLOC_FIXED_CONFIG_ONLY*/
    /* Initialise pool_config array to zero */
    memset(pool_config, 0, MAX_NUM_POOLS * sizeof(pmalloc_pool_config));
 

    /* Add the pools request from the header to the returned array */
    pool_len = add_pmalloc_config(pool_config, MAX_NUM_POOLS, 0, pools_reqd,
                                  sizeof(pools_reqd)/sizeof(pools_reqd[0]));

    /* See if there's a customer pools request too, and add that */
    if(&pmalloc_app_end != &pmalloc_app_begin)
    {
        pool_len = add_pmalloc_config(pool_config, MAX_NUM_POOLS, pool_len,
                                      (pmalloc_pool_config *)(&pmalloc_app_begin),
                                      ((unsigned int)&pmalloc_app_size)
                                        /sizeof(pmalloc_pool_config));
    }
   
    /* Sort the combined requests into ascending order */
    sort_pmalloc_config(pool_config, pool_len);

#ifndef PMALLOC_DONT_COMBINE_POOLS  
    /* Combine pools whose sizes differ by only a small amount */                       
    combine_pmalloc_sizes(pool_config, &pool_len);
#endif    
   
    /* Calculate the space used by the config request */
    config_reqd = 0;
    pool_config_ptr = pool_config;
    for (i = 0; i < pool_len; ++i)
    {  
        config_reqd += pool_config_ptr->size * pool_config_ptr->blocks;
#ifndef PMALLOC_FIXED_CONFIG_ONLY
        if ((pool_config_ptr->size) <= PM_MAX_SIZE_EXTRA)
        {
            tot_block_size += pool_config_ptr->size;
        }
#endif
        ++pool_config_ptr;        
    }   
    
    /* Calculates the un-allocated remainder of the heap 
     * that can be given additional blocks spread across the defined pools */
#ifdef NO_PMALLOC_MEMMAP_SYMBOLS
    heap_size = PMALLOC_RAM_SIZE; 
#else    
    heap_size = (size_t)pmalloc_end - (size_t)pmalloc_begin;
#endif
    /*lint --e{123} Ignore the ugly clash between pmalloc the
     * _MODULE_NAME_ and pmalloc the function-like macro.
     */ 
#ifndef PMALLOC_FIXED_CONFIG_ONLY
    /* Space required for pool-control blocks */
    pcb_size = (sizeof(pmalloc_pool) + PMALLOC_ALIGN_BOUNDARY)*pool_len;
    if (config_reqd >= (heap_size - pcb_size))
    {
        return FALSE;
    }
   
    config_avail = heap_size - (pcb_size + config_reqd);

    /* Only consider adding blocks if there are some non-zero pools */
    if (tot_block_size > 0)
    {
        size_t num_extra_blocks;

        /* Calculate number of blocks per pool, assumed added over the available pools */
        num_extra_blocks = config_avail / tot_block_size;
        
        /* Spread blocks evenly over the available pools */
        if (num_extra_blocks)
        {
            pool_config_ptr = pool_config;
            for (i = 0; i < pool_len; i++)
            {
                if (pool_config_ptr->size <= PM_MAX_SIZE_EXTRA)
                {
                    pool_config_ptr->blocks += num_extra_blocks;
                    config_avail -= num_extra_blocks * pool_config_ptr->size;
                }
                pool_config_ptr++;
            }
        }
    
        /* Any spare space, add one block per pool until we run out */
        pool_config_ptr = pool_config;
        for (i = 0; i < pool_len; i++)
        {
            if (pool_config_ptr->size < config_avail)
            {
                pool_config_ptr->blocks++;
                config_avail -=  pool_config_ptr->size;             
            }
            pool_config_ptr++;
        }
        
        /* Finally, fill up the remaining space with blocks in the smallest pool-size */
        num_extra_blocks = config_avail / pool_config->size;
        pool_config->blocks += num_extra_blocks;
    }
#endif    
     
    /* Re-calculate the space used by the config */
    config_reqd = 0;
    pool_config_ptr = pool_config;  
    for (i = 0; i < pool_len; ++i,++pool_config_ptr)
    {  
        config_reqd += pool_config_ptr->size * pool_config_ptr->blocks;        
    } 
    config_reqd += (sizeof(pmalloc_pool) + PMALLOC_ALIGN_BOUNDARY)*pool_len;

    *pool_config_len = pool_len;

    return(config_reqd <= heap_size); 
    
}


/** Sort a pool_config[] into order of block_size, lowest block_size first
 *  
 * \param pool_config The pool configuration to be sorted
 * \param config_len The number of entries in pool_config
 */
static void sort_pmalloc_config(pmalloc_pool_config *pool_config, unsigned int config_len)
{
    unsigned i;
    unsigned j;
    pmalloc_pool_config tmp_pool;
    pmalloc_pool_config *pool_config_ptr_i;
    pmalloc_pool_config *pool_config_ptr_j;
    size_t min_size;
    unsigned min_pos;          

    /* Trivial bubble sort, O(n^2), but OK because config_len is small */
    pool_config_ptr_i = pool_config;
    for (i = 0; i < (config_len - 1); i++)
    {
        min_pos = i;
        min_size = pool_config_ptr_i->size;
        pool_config_ptr_j = pool_config_ptr_i;
        
        /* Find the minimum size at and above the i'th
         * position in the list */
        for (j = i; j < config_len; j++)
        {
            if ((pool_config_ptr_j->size) < min_size)
            {
                min_size =  pool_config_ptr_j->size;
                min_pos = j;             
            }
            pool_config_ptr_j++;
        }

        /* Swap the i'th element with the minimum */
        tmp_pool = *pool_config_ptr_i;
        (*pool_config_ptr_i) = pool_config[min_pos];
        pool_config[min_pos] = tmp_pool;
         
        ++pool_config_ptr_i;
    }
}


/**
 * Given a non-ordered, non-unique array of pmalloc_pool_config elements,
 * add them to the pool_config, ending up with a non-ordered but unique
 * array. Clips poolsize to [PMALLOC_MIN_BLOCK, PMALLOC_MAX_BLOCK].
 *
 * Panics if the pool_config overflows.
 * 
 * \param pool_config the configuration we are accumulating into
 * \param pool_max the maximum number of entries in pool_config
 * \param pool_used the number of entries already present
 * \param source_config the configuration we are reading from
 * \param source_entries the number of entries in the source config array
 *
 * \return The number of entries in pool_config
 */
static unsigned int add_pmalloc_config(pmalloc_pool_config *pool_config, unsigned int pool_max, unsigned int pool_used, const pmalloc_pool_config *source_config, unsigned int source_entries)
{
    unsigned int i;
    unsigned int j;
    unsigned int pool_config_entries;
    size_t new_size;
    size_t new_blocks;
    bool inserted;

    pool_config_entries = pool_used;

    for (i = 0; i < source_entries; i++)
    {   
        /* Attempt to insert a new pool config element */
        inserted = FALSE;

        new_size = source_config[i].size;
        new_blocks = source_config[i].blocks;

        /* As we're now customer-facing, check the pool config element at this point
         * rather than panicking later.
         */
        if ((new_size < PMALLOC_MIN_BLOCK) ||
            (new_size > PMALLOC_MAX_BLOCK) ||
            (new_size & (PMALLOC_ALIGN_BOUNDARY-1)))
        {
            panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_CONFIG_ENTRY);
        }
        
        /* Ignore the pool entry if the block size equals zero */  
        if (new_blocks)
        {
            /* iterate over the locations in pool_config where we
             * might be able to add/accumulate the new pool */
            for (j = 0;j < pool_config_entries; j++)
            {
                /* If the pool-size at this position is the same, accumulate it in */                
                if (pool_config[j].size == new_size)
                {
                    pool_config[j].blocks += new_blocks;
                    inserted = TRUE;
                    break;             
                }              
            }

            if (!inserted)
            {
                /* Add the configuration to the end of the list */      
                if (pool_config_entries < pool_max)
                {
                    pool_config[pool_config_entries].size = new_size;
                    pool_config[pool_config_entries].blocks = new_blocks;
                    pool_config_entries++;
                }
                else
                {
                    /* If there is no room to insert more configurations (more pools than 
                     * pool_max then PANIC - this is an invalid configuration */
                    panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_CONFIG);   
                }            
            }
        }
    }
    if (pool_config_entries == 0)
    {
        /* No non-zero configuration was found */ 
        panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_CONFIG);   
    }               
    return(pool_config_entries);
}

/** Combine pools with close sizes, such that combining the pools
 * makes a "small" difference to the memory required, but decreases
 * the number of pools. Decreasing the number of pools will speed up
 * pmalloc(). So this is a speed vs size tradeoff, determined by
 * POOL_COMBINE_FRAC_ERROR: larger val => faster, but more memory
 * \param pool_config The pool-configuration to be sorted
 * \param config_len The length of non-zero entries in pool_config[]
 */
#ifndef PMALLOC_DONT_COMBINE_POOLS  
static void combine_pmalloc_sizes(pmalloc_pool_config *pool_config, unsigned *config_len)
{
    unsigned i;
    unsigned j;
    unsigned config_end_index;
    size_t current_reqd_mem;
    size_t reqd_mem_increase;
    pmalloc_pool_config *lower_pool;
    pmalloc_pool_config *higher_pool;   
    
    config_end_index = (*config_len) - 1;
    /* Starting from the largest sizes, to avoid combining multiple 
     * times iteratively, making large overall size change by several 
     * "small" pool-combinations */
    for (i = config_end_index; i > 0;i--)
    {
        higher_pool = &pool_config[i];
        lower_pool = higher_pool - 1;
        current_reqd_mem = (higher_pool->blocks * higher_pool->size) 
                           + (lower_pool->blocks * lower_pool->size);
        reqd_mem_increase = lower_pool->blocks * (higher_pool->size - lower_pool->size);
                    
        /* If the increase in total size required by combining is less than a 
         * small fraction, combine the pools into one with the larger block-size
         * Express the fractional error as a divisor rather than multiplier
         * to prevent overflow issues */
        if (reqd_mem_increase < (current_reqd_mem / POOL_COMBINE_FRAC_ERROR))
        {
            lower_pool->blocks += higher_pool->blocks;
            lower_pool->size = higher_pool->size;
            (*config_len)--;
            /* shuffle-down the larger pools to fill the gap in the list */
            lower_pool = higher_pool;
            higher_pool = lower_pool + 1;
            for (j= i; j < config_end_index; j++)
            {
                lower_pool->blocks = higher_pool->blocks;
                lower_pool->size = higher_pool->size;
                lower_pool++;
                higher_pool++;
            }
        }
    }
}
#endif

