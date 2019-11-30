/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Internal header file for the private memory allocator
 *
 */

#ifndef PMALLOC_PRIVATE_H
#define PMALLOC_PRIVATE_H

#include "pmalloc/pmalloc.h"
#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"
#include "hydra_log/hydra_log.h"
#include "int/int.h"
#include "panic/panic.h"
#ifdef RESOURCE_MODULE_PRESENT
#include "resource/resource.h"
#endif
#ifdef IPC_MODULE_PRESENT
#include "ipc/ipc.h"
#endif
#include "memory_map.h"
#ifdef PMALLOC_BLOCK_OVERRUN_CHECK
#include "hydra_log/hydra_log.h"
#include "hydra/hydra_macros.h"
#endif /* PMALLOC_BLOCK_OVERRUN_CHECK */

/** Resource management thresholds and status */
/** (based on ability to allocate doubly linked list of MA-UNITDATA.request) */
#define PMALLOC_RESOURCE_SIZE (16 * sizeof(uint16) + 2 * sizeof(void *))
#define PMALLOC_RESOURCE_LOW (20)
#define PMALLOC_RESOURCE_CRITICAL (10)
#ifdef RESOURCE_MODULE_PRESENT
extern size_t pmalloc_resource_free;
#endif


/**
 * Required alignment in multiples of location size (this is 1 for the Xap2,
 * and 4 for most other modern processors that support byte addressing but
 * require 32bit values to be quad-byte aligned) */
typedef struct pmalloc_align_test
{
    char d;
    uint32 a;
} pmalloc_align_test;
#define PMALLOC_ALIGN_BOUNDARY (offsetof(pmalloc_align_test, a))

/** Round a pointer to the next alignment boundary */
#define PMALLOC_ALIGN(p) \
    ((void *) (char *) ((((size_t) (char *) (p)) \
                         + (PMALLOC_ALIGN_BOUNDARY - 1)) \
                        & (size_t) ~((ptrdiff_t) \
                                     (PMALLOC_ALIGN_BOUNDARY - 1))))

/**
 * Range of allowed block sizes. Blocks smaller than the minimum size are
 * enlarged, and blocks larger than the maximum size are rejected. Both of
 * these tests are performed when the pools are initialised. */
#define PMALLOC_MIN_BLOCK MIN(PMALLOC_ALIGN_BOUNDARY, sizeof(void *))
#define PMALLOC_MAX_BLOCK (2048)


#ifndef NO_PMALLOC_MEMMAP_SYMBOLS
/**
 * Address range to be used for the private memory pool, including all
 * dynamically allocated control and tracing structures (the implementation
 * may change for different targets, so always use the PMALLOC_ versions) */
extern char *pmalloc_app_begin;
extern char *pmalloc_app_end;
extern char *pmalloc_app_size;
#define PMALLOC_BEGIN ((void *) pmalloc_begin)
#define PMALLOC_END ((void *) pmalloc_end)
#else
/**
 * In case pmalloc_begin() and pmalloc_end() are not externally defined
 * by linker, when unit-testing, statically and locally allocate a buffer.
 * The actual value of this isn't particularly relevant, if we've really
 * used more RAM than we have then the firmware build will fail, this value
 * is only used by desktop builds.
 */
#define PMALLOC_RAM_SIZE (48 * 1024)
#endif

/** Mechanism used to block interrupts when atomic access is required to
 * pool control blocks */
#define PMALLOC_BLOCK_INTERRUPTS() block_interrupts();
#define PMALLOC_UNBLOCK_INTERRUPTS() unblock_interrupts();


/** Control block for a pool of blocks of the same size */
typedef struct pmalloc_pool
{
    /** Size of blocks in this pool */
    size_t size;

    /** Total blocks in pool (free + allocated) */
    size_t blocks;

    /** First location after end of pool */
    void *pool_end;

    /** Head of free list for this pool */
    void *free;

    /** Number of blocks currently allocated */
    size_t allocated;

#ifdef PMALLOC_STATS
    /** Highest value of "allocated" */
    size_t max_allocated;

    /** Number of times demand exceeded capacity */
    unsigned overflows;
#endif

} pmalloc_pool;


/**
 * Array of pool control blocks. This is not statically allocated because the
 * number of pools may be configured at run-time. The control blocks must be
 * held in order of increasing size. */
extern size_t pmalloc_num_pools;
extern pmalloc_pool *pmalloc_pools;

/** Pointer to the first block in the first pool */
extern void *pmalloc_blocks;

#ifdef PMALLOC_MONITOR_POOLS
/** Monitoring of number of free blocks in a range of pools */
extern const pmalloc_pool *pmalloc_monitored_pool_low;
extern const pmalloc_pool *pmalloc_monitored_pool_high;
#endif


/** Hide the myriad of debugging options in a separate file
 * (must be included after the data structures have been declared) */
#include "pmalloc/pmalloc_debug.h"


/**
 * Is a block just the right size
 *
 * Determines whether the block containing "ptr" is the preferred size for
 * holding a block of data of size "size".
 *
 * For the memory block to be the preferred size it must be at least of
 * size "size" and a block of memory taken from the next smaller pool would
 * be smaller than "size".
 *
 * This does not check whether any blocks of the ideal size are available.
 *
 * Returns
 *
 * Zero if the block is the preferred size, otherwise the size of the block.
 */
extern size_t prightsize(const void *ptr, size_t size);

/**
 * Construct fixed pool memory allocation configuration,
 * based on summing the contributions from known sources
 * defined in pmalloc_config_P0 or pmalloc_config_P1.h depending on
 * compile-flags
 * Partition the memory into pools based on "configuration".
 * Then allocate the remaining heap to the given pools 
 * Doesn't set up the pool control blocks at this stage
 *
 * \param pool_config list of block-size (0 = unused), block-num pairs
 * \param pool_config_len length of non-zero entries in pool_config
 * \return TRUE if there was sufficient space in the heap to fit the 
 *              configured pools, otherwise FALSE
 */ 
bool get_pmalloc_config(pmalloc_pool_config *pool_config, unsigned *pool_config_len);

/**
 * Partition the memory into pools based on "configuration". This sets the
 * main characteristics of the pool control blocks ("size", "blocks" and
 * "pool_end"), but does not initialise the free list or statistics
 * collection.
 * Acts directly on pool control blocks pointed to by static pmalloc_begin_ind
 * \param pool_config the pool-configuration to be used
 *                    to initialise the pool-control blocks
 * \param pool_config_len of pool_config
 */
void pmalloc_configure(pmalloc_pool_config *pool_config, unsigned pool_config_len);


#endif /* PMALLOC_PRIVATE_H */
