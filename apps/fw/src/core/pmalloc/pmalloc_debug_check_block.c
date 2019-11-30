/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Check a block has not been written to
 *
 */

#include "pmalloc/pmalloc_private.h"
#include "assert.h"

/**
 * Check a block has not been written to
 */
void pmalloc_debug_check_block(const void *ptr, size_t size)
{
#ifdef XAP

    /* On a Xap memcmp() can check 16bit values */
    if ((1 < size) && memcmp(ptr, ((const uint16 *) ptr) + 1, size - 1))
        panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
#else
    /* Simple implementation for other platforms */
    /*
     * What we're trying to do is check that every entry
     * in the block is the same as the first
     */
    if (sizeof(void *) < size)
    {
        /* Pointers to void aren't much use, so we use int * instead */
        const unsigned int *next = ptr, *p;
        assert(sizeof(void *) == sizeof(unsigned int));
        for (p = next + 1, size -= sizeof(void *);
             sizeof(void *) <= size;
             size -= sizeof(void *))
        {
            if (*p++ != *next)
            {
                panic(PANIC_HYDRA_INVALID_PRIVATE_MEMORY_POINTER);
            }
        }
    }
#endif
}
