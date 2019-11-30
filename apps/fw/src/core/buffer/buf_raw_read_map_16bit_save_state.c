/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

/**
 * As \c buf_raw_read_map_16bit(), but save the buffer state.
 */
const uint16 *buf_raw_read_map_16bit_save_state(const BUFFER *buf,
                                                buf_mapping_state *save_state)
{
    buf_save_state(save_state);
    mmu_read_port_open(buf->handle);
    return mmu_read_port_map_16bit_le(buf->handle, buf->outdex);
}
