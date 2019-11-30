/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Allocate a memory buffer
 *
 */

#include "pmalloc/pmalloc_private.h"

/**
 * Allocate a memory buffer
 */
void *xpmalloc_buffer(size_t maxsize, size_t *bufsiz)
{
    pmalloc_pool *pool;
    void *ret;

    if (!maxsize)
    {
        return (void*)(NULL);
    }
    /* Try to honour the request directly.  malloc()'s size promotion
       mechanism may cause a buffer larger than "maxsize" to be allocated,
       but we can just lie about its size to the caller.  He can use psizeof()
       if he really wants to know the returned block's size. */

    ret = xpmalloc(maxsize);
    if (ret)
    {
        if (bufsiz)
        {
            *bufsiz = maxsize;
        }
        return ret;
    }

    /* We know there are no blocks of size "maxsize" or greater, but we
       may still be able to allocate a smaller block.  Walk the list of pools
       in reverse size order, and try to allocate from each in turn.

       (Well, strictly, interrupt code could just have released a big block,
       so we may be lucky.)

       This loop code presumes we have at least one pool; it's just not
       worth worrying about the alternative. */

    for (pool = &(pmalloc_pools[pmalloc_num_pools - 1]); pool >= pmalloc_pools;
         pool--)
    {
        ret = xpmalloc(pool->size);
        if (ret)
        {
            if (bufsiz)
            {
                *bufsiz = MIN(pool->size, maxsize);
            }
            return ret;
        }
    }
    return (void*)(NULL);
}
