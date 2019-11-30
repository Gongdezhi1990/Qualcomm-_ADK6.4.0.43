/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint8 *
buf_raw_read_write_map_8bit(BUFFER *buf)
{
    mmu_read_port_open(buf->handle);
    mmu_write_port_open(buf->handle);
    (void) mmu_read_port_map_8bit(buf->handle, buf->outdex);
    return mmu_write_port_map_8bit(buf->handle, buf->outdex);
}
