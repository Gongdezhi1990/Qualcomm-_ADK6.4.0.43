/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

/**
 * Return a 16-bit pointer to read data from a buffer.
 */
const uint16 *buf_raw_read_map_16bit(const BUFFER *buf)
{
    mmu_read_port_open(buf->handle);
    return mmu_read_port_map_16bit_le(buf->handle, buf->outdex);
}
