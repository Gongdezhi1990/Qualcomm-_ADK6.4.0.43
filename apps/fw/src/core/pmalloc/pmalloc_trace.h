/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Tracing support for the private memory allocator
 *
 * The pmalloc library supports tracing of block ownership, with several
 * options for how the ownership is recorded (such as source file reference
 * or program counter location). This file defines macros to forward the
 * standard allocation functions to versions that record the required
 * information.
 */

#ifndef PMALLOC_TRACE_H
#define PMALLOC_TRACE_H


/** No special support required unless one of the tracing options is enabled */
#if defined(PMALLOC_TRACE_OWNER) \
    || defined(PMALLOC_TRACE_OWNER_LINES_ONLY) \
    || defined(PMALLOC_TRACE_OWNER_PC_ONLY)


/** Define a single macro to indicate that tracing is enabled */
#define PMALLOC_TRACE_OWNER_ANY


/** Define the appropriate tracing behaviour for the selected option */
#if defined(PMALLOC_TRACE_OWNER)

/** Record the filename and line number as a text string */
typedef const char *pmalloc_owner_ref;
typedef const char *pmalloc_owner_internal_ref;
enum PC_TRACE_DETAILS {PC_TRACE_SHIFT = 0};
/** Two macro levels, one for expansion of __LINE__, one for stringification */
#define PMALLOC_LOCATION1(str, line) (str #line)
#define PMALLOC_LOCATION(file,line) PMALLOC_LOCATION1(file ":", line)
#define PMALLOC_TRACE(fn,size) \
    fn ## _trace((size), PMALLOC_LOCATION(__FILE__, __LINE__))

#elif defined(PMALLOC_TRACE_OWNER_LINES_ONLY)

/** Record the line number in numeric form */
typedef uint16 pmalloc_owner_ref;
typedef uint16 pmalloc_owner_internal_ref;
enum PC_TRACE_DETAILS {PC_TRACE_SHIFT = 0};
#define PMALLOC_TRACE(fn,size) \
    fn ## _trace((size), __LINE__)

#elif defined(PMALLOC_TRACE_OWNER_PC_ONLY)

/** Record the program counter */
typedef uint32 pmalloc_owner_ref;

/* To keep the RAM cost of PC tracing small we only record bits
 * [18:3] of the PC. 
 * Losing bits 19 and above costs us nothing because our code isn't big enough
 * Losing bits 2:0 means our Pylib PC trace decode has to be careful about
 * which function it claims called malloc */
typedef uint16 pmalloc_owner_internal_ref;
enum PC_TRACE_DETAILS {PC_TRACE_SHIFT = 3};

#define PMALLOC_TRACE(fn,size) \
    fn ## _trace_pc(size)

#endif


/** Remap all of the allocation functions to the version with tracing */
#define pmalloc(size) PMALLOC_TRACE(pmalloc, (size))
#define zpmalloc(size) PMALLOC_TRACE(zpmalloc, (size))
#define xpmalloc(size) PMALLOC_TRACE(xpmalloc, (size))
#define xzpmalloc(size) PMALLOC_TRACE(xzpmalloc, (size))


/**
 * Allocate a block of memory and record owner information
 *
 * Allocate a block of private memory of at least "size" locations,
 * recording the owner as "owner".
 */
extern void *pmalloc_trace(size_t size, pmalloc_owner_ref owner);
extern void *zpmalloc_trace(size_t size, pmalloc_owner_ref owner);
extern void *xpmalloc_trace(size_t size, pmalloc_owner_ref owner);
extern void *xzpmalloc_trace(size_t size, pmalloc_owner_ref owner);


/**
 * Allocate memory and record program counter of owner
 *
 * Allocate a block of private memory of at least "size" locations,
 * recording the owner as the least significant part of the return address.
 * These are all thin assembler veneers that forward to the corresponding
 * pmalloc_trace() function.
 */
#ifdef PMALLOC_TRACE_OWNER_PC_ONLY
extern void *pmalloc_trace_pc(size_t size);
extern void *zpmalloc_trace_pc(size_t size);
extern void *xpmalloc_trace_pc(size_t size);
extern void *xzpmalloc_trace_pc(size_t size);
#endif


#endif


/* Simplify the definition of allocation functions that forward tracing
 * information to other functions. The PMALLOC_TRACE_FN macro can be used
 * for both the function prototype and the forwarding call. */
#ifdef PMALLOC_TRACE_OWNER_ANY
#define PMALLOC_TRACE_FN(fn,size,owner) \
    fn ## _trace(size, owner)
#else
#define PMALLOC_TRACE_FN(fn,size,owner) \
    fn(size)
#endif


#endif /* PMALLOC_TRACE_H */
