/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Change the size of a block of private memory
 *
 */

#include "pmalloc/pmalloc_private.h"

#ifdef PMALLOC_TRACE_OWNER_PC_ONLY
/* !!! portability issue
 * inline assembler function to return the caller address
 * This in-line function can only be used early on in a
 * function, certainly before any other function is called.
 */
/*lint -e{*}*/
asm size_t getReturnAddress(void)
{
    /* using @ { } as return register (instead of r0) allows the
     * compiler to optimize which register is actually used.
     * In all probability, it will be r0, but I don't need to care.
     */
    @{} = rlink;
}
#endif

/**
 * Change the size of a block of private memory
 *
 * This stub is left to implement a traceable xprealloc function
 */
void *xprealloc(void *ptr, size_t size)
{
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    size_t caller = getReturnAddress();
    return xprealloc_trace(ptr, size, caller);
#else
    return xprealloc_no_trace(ptr, size);
#endif
}


#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)

/* xprealloc implementation with additional caller parameter
 * to allow CALLER of CALLER to be traced.
 * Note this is called from xprealloc AND realloc
 */
void *xprealloc_trace(void *ptr, size_t size, size_t caller)

#else

/* xprealloc implementation without tracing
 * Note this is called from xprealloc
 * I'd like to make this xprealloc, but I can't!
 * xprealloc itself needs to be traceable.
 * Here, we are in the else part of the preprocessor
 * where we aren't able to implement traceable functions
 */
void *xprealloc_no_trace(void *ptr, size_t size)

#endif
{
    size_t current_size = size;
    void *new_ptr;

/* If we have enabled the block overrun check don't try to resize as this 
   will involve moving the guard values. Just malloc a new block
   as this will set up a new block with the correct data for overrun checking
   and free the previous block causing an overrun check */
#ifndef PMALLOC_BLOCK_OVERRUN_CHECK

    /* Determine the current size of the block, and keep the existing block
       if it is already the right size */
    if (ptr)
    {
        current_size = prightsize(ptr, size);
        if (current_size == 0)
        {
            return ptr;
        }
    }
    else
    {
        current_size = 0;
    }
#endif /* PMALLOC_BLOCK_OVERRUN_CHECK*/
    /* Attempt to allocate a new block of the requested size */
    /* use a traceable lower function if there is a traceable address supplied */
#if defined(PMALLOC_TRACE_OWNER_PC_ONLY)
    new_ptr = xpmalloc_trace(size, caller);
#else
    new_ptr = xpmalloc(size);
#endif
    if (!new_ptr)
    {
        return size <= current_size ? ptr : NULL;
    }
    /* Copy the data between the two blocks
       (may be zero length if "ptr" is NULL or "size" is 0) */
    if (ptr)
    {
        (void) memcpy(new_ptr, ptr, MIN(size, current_size));
    }
    /* Return the old block to the pool */
    pfree(ptr);

    /* Return a pointer to the new block */
    return new_ptr;
}
