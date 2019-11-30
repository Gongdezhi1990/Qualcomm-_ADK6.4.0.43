/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 *
 * mempack - mempack C functions in \c optim module.
 */

#include "optim/optim_private.h"

void mempack_le(uint16 *dest, const uint8 *source, uint32 len_bytes)
{
    while (len_bytes--)
    {
        uint16 lo = *source++ & 0xFF;
        uint16 hi = *source++;
        *dest++ = (uint16)((hi << 8) | lo);
    }
}
