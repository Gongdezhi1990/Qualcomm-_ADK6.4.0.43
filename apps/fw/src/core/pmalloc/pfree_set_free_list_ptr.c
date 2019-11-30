/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Set pointer to next block on free list
 *
 */

#include "pmalloc/pmalloc_private.h"


/**
 * Set pointer to next block on free list
 */
void pfree_set_free_list_ptr(void *ptr, void *next, size_t size)
{
    void **p;

    /* Fill the block with the next free pointer value */
    for (p = (void **) ptr; sizeof(void *) <= size; size -= sizeof(void *))
        *p++ = next;
}
