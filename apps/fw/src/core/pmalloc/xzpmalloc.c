/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Allocate a block of private memory
 */

#include "pmalloc/pmalloc_private.h"


/**
 * Allocate a block of private memory
 * This function does not call zpmalloc() (or vice versa). Remember to
 * propagate any changes to zpmalloc.c.
 */
void *PMALLOC_TRACE_FN(xzpmalloc, size_t size, pmalloc_owner_ref owner)
{
    /* Attempt to allocate the memory */
    void *ptr = PMALLOC_TRACE_FN(xpmalloc, size, owner);

    /* Initialise the block if successfully allocated */
    if (ptr)
        (void) memset(ptr, 0, size);

    /* Return a pointer to the newly allocated block */
    return ptr;
}
