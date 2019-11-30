/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */
#include "buffer/buffer_private.h"

void
buf_raw_read_update_restore_state(BUFFER *buf, uint16 octets,
                                  const buf_mapping_state *save_state)
{
    buf_raw_read_update(buf, octets);
    buf_restore_state(save_state);
}
