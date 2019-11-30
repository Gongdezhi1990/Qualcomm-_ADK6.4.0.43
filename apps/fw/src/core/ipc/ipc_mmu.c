/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"

#include "mmu/mmu.h"

void ipc_send_buf_raw_update_tail_free(BUFFER *buf, uint16 new_tail)
{
    IPC_BUF_TAIL_FREE prim;
    prim.buffer = buf;
    prim.new_tail = new_tail;
    ipc_send(IPC_SIGNAL_ID_BUF_TAIL_FREE, (const void *)&prim, sizeof(prim));
}

void ipc_send_buf_update_behind_free(BUFFER_MSG *msg_buf)
{
    /* We give P0 the BUFFER_MSG's address and it'll perform the clear for
     * us.  Note that this relies on P1 always placing BUFFER_MSG structures
     * in shared memory. */
    IPC_BUF_UPDATE_BEHIND_FREE prim;
    prim.buffer_msg = msg_buf;
    ipc_send(IPC_SIGNAL_ID_BUF_UPDATE_BEHIND_FREE,
                                        (const void *)&prim, sizeof(prim));
}
