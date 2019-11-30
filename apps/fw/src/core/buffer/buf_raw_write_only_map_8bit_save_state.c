/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file buf_raw_write_only_map_8bit_save_state.c
 * Contains the corresponding function.
 */

#include "buffer/buffer_private.h"

uint8 *
buf_raw_write_only_map_8bit_save_state(BUFFER *buf, buf_mapping_state *save_state)
{
    buf_save_state( save_state );
    mmu_write_port_open(buf->handle);
    return mmu_write_port_map_8bit(buf->handle, buf->index);
}
