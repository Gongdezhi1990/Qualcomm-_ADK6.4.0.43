/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

/**
 * As \c buf_raw_read_map_8bit(), but save the buffer state.
 */
const uint8 *buf_raw_read_map_8bit_save_state(const BUFFER *buf,
                                              buf_mapping_state *save_state)
{
    buf_save_state(save_state);
    mmu_read_port_open(buf->handle);
    return mmu_read_port_map_8bit(buf->handle, buf->outdex);
}
