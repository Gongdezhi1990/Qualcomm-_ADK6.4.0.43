/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \defgroup pl_malloc Memory allocation and free functionality
 *
 * \file pl_malloc.h
 * \ingroup pl_malloc
 *
 *
 * Interface for memory allocation/free functions
 *
 ****************************************************************************/

#if !defined(PL_MALLOC_H)
#define PL_MALLOC_H

/****************************************************************************
Include Files
*/
#include "types.h"
#ifdef PL_DEBUG_MEM_ON_HOST
/**
 * PL_DEBUG_MEM_ON_HOST gives a printf for each malloc/free showing what pointer
 * was allocated or freed, what size an alloc was for, and where in the code
 * the call came from.
 *
 * Useful for finding memory leaks, or profiling memory use
 *
 * NOTE currently works only on host based tests, as uses printf to print
 *      filename (trace mechanism doesnt support %s)
 */
#include <stdio.h>
#endif

#include "pl_malloc_preference.h"

/****************************************************************************
Public Macro Declarations
*/

/* Macro for getting the equivalent DM1 address of an address that could be in DM1 or DM2 
 * Used for anything that needs a physical DM address e.g. MMU buffers 
 */
#if !defined(__GNUC__)
#define DM1_ADDRESS(x) (((uintptr_t)(x) >= DM2_RAM_BASE)?((uintptr_t)(x) - DM2_RAM_BASE):(uintptr_t)(x))
#else
#define DM1_ADDRESS(x) (x)
#endif

/**
 * Use Memory pool based malloc and free functions, defined in pl_malloc.c
 *
 * xpmalloc: a version of malloc similar to standard C library, that does not panic if it has not memory but instead returns a Null pointer.
 * pmalloc: a version of malloc that panics if it has no memory. .
 * xzpmalloc: a version of malloc that initialises the allocated memory to zero or returns a Null pointer if it has no memory.
 * zpmalloc: a version of malloc that initialises the allocated memory to zero or panics if it has no memory.
 * 
 * The '[x][z]ppmalloc variants take an additional 'preference' parameter to specify DM1/DM2/either.
 *
 * pfree: a version of free (does not panic if passed a Null pointer).
 *
 * Note init_pmalloc must be called before any version of malloc or free is called
 */

#ifdef PMALLOC_DEBUG
extern void *xppmalloc_debug(unsigned int numBytes, unsigned int preference, const char *file, unsigned int line);
extern void *ppmalloc_debug(unsigned int numBytes, unsigned int preference, const char *file, unsigned int line);
extern void *xzppmalloc_debug(unsigned int numBytes, unsigned int preference, const char *file, unsigned int line);
extern void *zppmalloc_debug(unsigned int numBytes, unsigned int preference, const char *file, unsigned int line);

#define xppmalloc(numBytes, pref) xppmalloc_debug(numBytes, pref, __FILE__, __LINE__)
#define ppmalloc(numBytes, pref) ppmalloc_debug(numBytes, pref, __FILE__, __LINE__)
#define xzppmalloc(numBytes, pref) xzppmalloc_debug(numBytes, pref, __FILE__, __LINE__)
#define zppmalloc(numBytes, pref) zppmalloc_debug(numBytes, pref, __FILE__, __LINE__)

extern void pvalidate(void *pMemory);

#else
extern void *xppmalloc(unsigned int numBytes, unsigned int preference);
extern void *ppmalloc(unsigned int numBytes, unsigned int preference);
extern void *xzppmalloc(unsigned int numBytes, unsigned int preference);
extern void *zppmalloc(unsigned int numBytes, unsigned int preference);
#endif

#define xpmalloc(numBytes) xppmalloc(numBytes, MALLOC_PREFERENCE_NONE)
#define pmalloc(numBytes) ppmalloc(numBytes, MALLOC_PREFERENCE_NONE)
#define xzpmalloc(numBytes) xzppmalloc(numBytes, MALLOC_PREFERENCE_NONE)
#define zpmalloc(numBytes) zppmalloc(numBytes, MALLOC_PREFERENCE_NONE)

extern void pfree(void *pMemory);

extern int psizeof(void *pMemory);

extern void init_pmalloc(void);

extern void config_pmalloc(void);

/**
 * Macros for type specific memory allocation
 */

/* Allocate memory of a given type (t) from the pool. */
#define pnew(t)             ((t *)( pmalloc( sizeof(t) )))
#define xpnew(t)            ((t *)( xpmalloc( sizeof(t) )))

/* Allocate memory of a given type (t) with preference (p). */
#define ppnew(t, p)         ((t *)( ppmalloc( sizeof(t), (p) )))
#define xppnew(t, p)        ((t *)( xppmalloc( sizeof(t), (p) )))

/* Allocate zeroed memory of a given type (t) from the pool. */
#define zpnew(t)            ((t *)( zpmalloc( sizeof(t) )))
#define xzpnew(t)           ((t *)( xzpmalloc( sizeof(t) )))

/* Allocate zeroed memory of a given type (t) with preference (p). */
#define zppnew(t, p)        ((t *)( zppmalloc( sizeof(t), (p) )))
#define xzppnew(t, p)       ((t *)( xzppmalloc( sizeof(t), (p) )))

/* Similar macros which allocate arrays of objects. */
#define pnewn(n, t)         ((t *)( pmalloc ( sizeof(t) * (n) )))
#define xpnewn(n, t)        ((t *)( xpmalloc ( sizeof(t) * (n) )))
#define zpnewn(n, t)        ((t *)( zpmalloc ( sizeof(t) * (n) )))
#define xzpnewn(n, t)       ((t *)( xzpmalloc ( sizeof(t) * (n) )))

/* As above with preference (p). */
#define ppnewn(n, t, p)     ((t *)( ppmalloc ( sizeof(t) * (n), (p) )))
#define xppnewn(n, t, p)    ((t *)( xppmalloc ( sizeof(t) * (n), (p) )))
#define zppnewn(n, t, p)    ((t *)( zppmalloc ( sizeof(t) * (n), (p) )))
#define xzppnewn(n, t, p)   ((t *)( xzppmalloc ( sizeof(t) * (n), (p) )))

/* Return the [x]pmalloc()ed memory block (d) to the pool. */
#define pdelete(d)          (pfree((void *)(d)))



/****************************************************************************
Public Type Declarations
*/

typedef unsigned (*pmalloc_cached_report_handler)(void);

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/
#ifdef PL_MEM_POOL_TEST
/* Used for module testing only */
extern void PlMemPoolTest(void);
#endif

/**
 * NAME
 *   pmalloc_cached_report
 *
 * \brief Function to allow pmalloc clients to report size of any cached data items
 * 
 * \param[in] new_handler Function called when calculating free pools
 *
 * \return  Previously-installed handler function
 *
 * \note Report handlers are statically chained - the caller of this function should 
 * store the old handler function returned, and (if non-NULL) call it from its handler, 
 * adding the result to the size returned.
 */
extern pmalloc_cached_report_handler pmalloc_cached_report(pmalloc_cached_report_handler new_handler);

#endif /* PL_MALLOC_H */
