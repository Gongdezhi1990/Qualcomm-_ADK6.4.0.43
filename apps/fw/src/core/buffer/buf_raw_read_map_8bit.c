/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

/**
 * Return an 8-bit pointer to read data from a buffer.
 */
const uint8 *buf_raw_read_map_8bit(const BUFFER *buf)
{
    mmu_read_port_open(buf->handle);
    return mmu_read_port_map_8bit(buf->handle, buf->outdex);
}
