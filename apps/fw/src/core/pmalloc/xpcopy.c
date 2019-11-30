/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Allocate and initialise a block of private memory
 *
 */

#include "pmalloc/pmalloc_private.h"


/**
 * Allocate and initialise a block of private memory
 *
 * This function does not call pcopy() (or vice versa). Remember to
 * propagate any changes to pcopy.c.
 */
void *xpcopy(const void *ptr, size_t size)
{
    /* Attempt to allocate the memory */
    void *new_ptr = xpmalloc(size);

    /* Initialise the block if successfully allocated */
    if (new_ptr)
        (void) memcpy(new_ptr, ptr, size);

    /* Return a pointer to the newly allocated block */
    return new_ptr;
}
