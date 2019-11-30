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
 * This function does not call xzpmalloc() (or vice versa). Remember to
 * propagate any changes to xzpmalloc.c.
 */
void *PMALLOC_TRACE_FN(zpmalloc, size_t size, pmalloc_owner_ref owner)
{
    /* Since pmalloc() never returns NULL (it calls panic() if allocation
       fails) it is possible to avoid use of a temporary variable. This
       takes advantage of memset() returning its first argument. */
    return memset(PMALLOC_TRACE_FN(pmalloc, size, owner), 0, size);
}
