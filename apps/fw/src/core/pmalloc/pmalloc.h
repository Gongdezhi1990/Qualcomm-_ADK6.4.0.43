/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Private memory allocator
*/


#ifndef PMALLOC_H
#define PMALLOC_H

/** For completeness. */
/* Needed for both cases */
#define xpfree(d)               (pfree(d))

#include "hydra/hydra_types.h"

#ifdef DESKTOP_TEST_BUILD
/*
 * For desktop builds, monitor pool sizes
 */
#define PMALLOC_MONITOR_POOLS
#endif

#ifdef PMALLOC_MONITOR_POOLS
/** Monitoring of number of free blocks in a range of pools */
extern size_t pmalloc_monitored_pools;
#endif

/** Configuration of a pmalloc pool */
typedef struct
{
    /** Size of blocks in this pool */
    size_t size;
    
    /** Number of blocks in pool */
    size_t blocks;
} pmalloc_pool_config;
#define MAX_NUM_POOLS 21

/**
 * The indirection for the start of pmalloc memory
 *
 * Can be used by the patch system to move up the memory
 * start to allow patches to use some of the space
 */

extern char *pmalloc_begin_ind;

/**
 * Initialise the private memory allocator
 *
 * Initialise the private memory allocator.
 *
 * This must be called before any use is made of the other functions in
 * this library.
 */
extern void init_pmalloc(void);

/**
 * Allocate a block of private memory
 *
 * Allocate a block of private memory of at least "size" locations.
 *
 * If "size" is zero then a zero-length block is allocated, and a valid
 * pointer returned. Note that this still uses resources.
 *
 * The z...() forms of the function initialise the block to zero, otherwise
 * the contents are undefined.
 *
 * Returns
 *
 * A pointer to the allocated memory block. The x...() forms of the function
 * returns NULL if the requested memory is not available.
 */
extern void *pmalloc(size_t size);
extern void *zpmalloc(size_t size);
extern void *xpmalloc(size_t size);
extern void *xzpmalloc(size_t size);

/**
 * Return a block of private memory to the pool
 *
 * Deallocate a block of memory previously allocated by pmalloc() or
 * prealloc(). If "ptr" is NULL then no action is taken.
 */
extern void pfree(void *ptr);

/**
 * free an array of memory blocks and zero the pointers
 * @param ptr The array of pointers
 * @param num Number of elements in the array
 */
extern void pdestroy_array(void **ptr, uint16 num);

/**
 * Change the size of a block of private memory
 *
 * Change the size of the block pointed to by "ptr" to at least "size"
 * locations. The contents of the block will be unchanged up to the lesser
 * of the new and old sizes.
 *
 * If "ptr" is NULL then this behaves like pmalloc() for the specified size.
 *
 * A pointer to the allocated memory block. The x...() form of the function
 * returns NULL if the requested memory is not available, and the block
 * pointed to by "ptr" is left intact.
 */
extern void *prealloc(void *ptr, size_t size);
extern void *xprealloc(void *ptr, size_t size);

/**
 * Allocate and initialise a block of private memory
 *
 * Allocate a block of private memory of at least "size" locations, and
 * copy the data from "ptr".
 *
 * If "size" is zero then a zero-length block is allocated, and a valid
 * pointer returned. Note that this still uses resources.
 *
 * A pointer to the allocated memory block. The x...() forms of the function
 * returns NULL if the requested memory is not available.
 */
extern void *pcopy(const void *ptr, size_t size);
extern void *xpcopy(const void *ptr, size_t size);

/**
 * Determine the size of a block of private memory
 *
 * The size of the block containing "ptr". If "ptr" does not lie within a
 * block of private memory then zero is returned.
 */
extern size_t psizeof(const void *ptr);

/**
 * How many blocks can be allocated
 *
 * The number of blocks that can currently be allocated of at least "size"
 * locations.
 */
extern size_t pmalloc_available(size_t size);

/**
 * Allocate a memory buffer
 *
 * Obtains an xpmalloc()ed buffer that is as big as is currently possible,
 * but of maximum size "maxsize".
 *
 * Returns
 *
 * An xpmalloc()ed buffer, or NULL if no memory is available.
 *
 * If the function doesn't return NULL, then "*bufsiz" is set to the
 * buffer's size.
 *
 * If "maxsize" is zero then the function returns NULL.
 */
extern void *xpmalloc_buffer(size_t maxsize, size_t *bufsiz);

#ifdef PMALLOC_TRACE_OWNER_PC_ONLY
/**
 * Reallocates a buffer with tracing
 * This function should only be called explicitly if it is necessary to
 * provide a custom trace parameter, which is rare. In normal usage you
 * should just ca ll xprealloc and the pmalloc trace ifdefs will select
 * the appropriate tracing, if any.
 */
extern void *xprealloc_trace(void *ptr, size_t size, size_t caller);
#else
/**
 * Reallocates a buffer without tracing
 * This function should not be called by user directly ever. It is used
 * internally as part of the memory allocation tracing system.
 */
extern void *xprealloc_no_trace(void *ptr, size_t size);
#endif /* PMALLOC_TRACE_OWNER_PC_ONLY */

#ifdef PMALLOC_MONITOR_POOLS
/**
 * Set free block monitoring limits
 *
 * Set the size limits for the pools to be monitored. Monitoring consists of
 * keeping track of the number of free blocks between "size_low" and
 * "size_high" locations (inclusive). This number is available in the
 * external variable "pmalloc_monitored_pools".
 */
extern void pmalloc_set_monitor_limits(size_t size_low, size_t size_high);
#endif

/**
 * Carl, whatever you do, don't add brackets around the t in (t *) in the
 * following macros - they will break.
 * Steven, very well.  I won't let it happen again. */

/** Allocate memory of a given type (t) from the pool. */
#define pnew(t)                 ((t *)(pmalloc(sizeof(t))))
#define xpnew(t)                ((t *)(xpmalloc(sizeof(t))))

/** Allocate zeroed memory of a given type (t) from the pool. */
#define zpnew(t)                ((t *)(zpmalloc(sizeof(t))))
#define xzpnew(t)               ((t *)(xzpmalloc(sizeof(t))))

/** Return the [x]pmalloc()ed memory block (d) to the pool. */
#define pdelete(d)              (pfree((void *)(d)))
#define xpdelete(d)             (pfree((void *)(d)))

/** Hide the complexities of tracing memory ownership in a separate file
 * (must not be included before the function prototypes) */

#include "pmalloc/pmalloc_trace.h"

#ifdef __KALIMBA__
/* Some temporary stuff to support kalbuffer/mmu_buffer.c until we can get rid of it */
#define DM1_ADDRESS(x) (x)
#endif

#ifdef DESKTOP_TEST_BUILD
/**
 * Deinitialise anything initialised for pmalloc.
 *
 * This is only needed by the unit test system, and not for fw
 */
extern void deinit_pmalloc(void);

#ifdef PMALLOC_MODULE_PRESENT
/* Currently no action here when pmalloc used */
#define deinit_pmalloc()
#endif /* PMALLOC_TEST_BUILD */

#endif /* DESKTOP_TEST_BUILD */

#ifndef NO_PMALLOC_MEMMAP_SYMBOLS
/* Export the addresses of the start/end of the pmalloc memory block */
extern char pmalloc_begin[];
extern char pmalloc_end[];
#endif /* NO_PMALLOC_MEMMAP_SYMBOLS */

#endif /* PMALLOC_H */
