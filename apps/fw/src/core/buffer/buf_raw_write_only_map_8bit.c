/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint8 *
buf_raw_write_only_map_8bit(BUFFER *buf)
{
    mmu_write_port_open(buf->handle);
    return mmu_write_port_map_8bit(buf->handle, buf->index);
}
