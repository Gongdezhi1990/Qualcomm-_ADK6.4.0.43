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
 */
void *pcopy(const void *ptr, size_t size)
{
    /* Since pmalloc() never returns NULL (it calls panic() if allocation
       fails) it is possible to avoid use of a temporary variable. This
       takes advantage of memcpy() returning its first argument. */
    return memcpy(pmalloc(size), ptr, size);
}
