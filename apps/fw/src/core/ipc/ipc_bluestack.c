/* Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"
#include "bluestack_if/bluestack_if.h"
#include "trap_api/trap_api.h"

void ipc_send_bluestack(uint16 protocol, void *prim)
{
    IPC_BLUESTACK_PRIM ipc_prim;
    ipc_prim.protocol = protocol;
    ipc_prim.prim = prim;
    ipc_send(IPC_SIGNAL_ID_BLUESTACK_PRIM, &ipc_prim,
                                                    sizeof(IPC_BLUESTACK_PRIM));
}

void ipc_bluestack_handler(IPC_SIGNAL_ID id, const void *msg)
{
    const IPC_BLUESTACK_PRIM *ipc_prim;
    const IPC_APP_MESSAGE_RECEIVED *ipc_rcvd_msg;
    UNUSED(ipc_rcvd_msg);
    assert(id == IPC_SIGNAL_ID_BLUESTACK_PRIM);
    ipc_prim = (const IPC_BLUESTACK_PRIM *)msg;
    trap_api_send_bluestack_message(ipc_prim->protocol, ipc_prim->prim);
}
