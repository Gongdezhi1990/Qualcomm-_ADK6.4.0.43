/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Change the size of a block of private memory
 *
 */

#include "pmalloc/pmalloc_private.h"


/**
 * Change the size of a block of private memory
 */
void *prealloc(void *ptr, size_t size)
{
    /* Attempt to change the size of the block using the version of this
       function that returns NULL if the request cannot be serviced */
    void *new_ptr = xprealloc(ptr, size);

    /* Panic if the allocation failed (does not return) */
    if (!new_ptr)
    {
        panic_diatribe(PANIC_HYDRA_PRIVATE_MEMORY_EXHAUSTION, size);
    }

    /* Return a pointer to the newly allocated (or possibly same) block */
    return new_ptr;
}
