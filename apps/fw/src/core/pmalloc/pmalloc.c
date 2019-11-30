/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Allocate a block of private memory
 *
 */

#include "pmalloc/pmalloc_private.h"

/**
 * Allocate a block of private memory
 */
void *PMALLOC_TRACE_FN(pmalloc, size_t size, pmalloc_owner_ref owner)
{
    /* Attempt to allocate the memory using the version of this function that
       returns NULL if the request cannot be serviced */
    void *ptr = PMALLOC_TRACE_FN(xpmalloc, size, owner);

    /* Panic if the allocation failed (does not return) */
    if (!ptr)
    {
        panic_diatribe(PANIC_HYDRA_PRIVATE_MEMORY_EXHAUSTION, size);
    }

    /* Return a pointer to the newly allocated block */
    return ptr;
}
