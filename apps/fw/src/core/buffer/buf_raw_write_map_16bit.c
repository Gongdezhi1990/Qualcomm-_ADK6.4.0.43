/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint16 *
buf_raw_write_map_16bit(BUFFER *buf)
{
    mmu_read_port_open(buf->handle);
    mmu_write_port_open(buf->handle);
    (void) mmu_read_port_map_16bit_le(buf->handle, buf->index);
    return mmu_write_port_map_16bit_le(buf->handle, buf->index);
}
