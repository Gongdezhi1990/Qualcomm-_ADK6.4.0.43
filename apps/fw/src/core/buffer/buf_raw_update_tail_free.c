/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * Contains the corresponding function.
 */
#include "buffer/buffer_private.h"

/**
 * Update the tail pointer, freeing any memory no longer needed.
 */
void
buf_raw_update_tail_free(BUFFER *buf, uint16 new_tail)
{
    new_tail = (uint16)(new_tail & (BUF_GET_SIZE_OCTETS(buf) - 1));
    /* P1 can't free its own pages: it needs to delegate this to P0 */
    ipc_send_buf_raw_update_tail_free(buf, new_tail);
}
