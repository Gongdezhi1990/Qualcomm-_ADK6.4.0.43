/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint16 *
buf_raw_write_map_16bit_save_state(BUFFER *buf, buf_mapping_state *save_state)
{
    buf_save_state( save_state );

    /*
     * Use both ports to yield a read-write pointer.
     *
     * \port    This trick assumes read & write mappings coincide.
     */
    mmu_read_port_open(buf->handle);
    mmu_write_port_open(buf->handle);

    (void) mmu_read_port_map_16bit_le(buf->handle, buf->index);
    return mmu_write_port_map_16bit_le(buf->handle, buf->index);
}
