/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint8 *
buf_raw_write_map_8bit_position(BUFFER *buf, uint16 position)
{
    /*
     * Use both ports to yield a read-write pointer.
     *
     * \port    This trick assumes read & write mappings coincide.
     */
    mmu_read_port_open(buf->handle);
    mmu_write_port_open(buf->handle);

    (void) mmu_read_port_map_8bit(buf->handle, position);
    return mmu_write_port_map_8bit(buf->handle, position);
}
