/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"
#include "bluestack/att_prim.h"
#include "bluestack_if/bluestack_if.h"

#include "pmalloc/pmalloc.h"

void ipc_send_free(IPC_SIGNAL_ID free_id, void *pointer)
{
    IPC_FREE free_msg;
    free_msg.pointer = pointer;
    ipc_send(free_id, &free_msg, sizeof(IPC_FREE));
}

void *ipc_send_smalloc(uint32 size_bytes, bool panic_on_failure)
{
    IPC_SMALLOC_REQ req;
    IPC_SMALLOC_RSP rsp;

    req.size_bytes = size_bytes;
    ipc_send(IPC_SIGNAL_ID_SMALLOC_REQ, &req, sizeof(IPC_SMALLOC_REQ));
    (void)ipc_recv(IPC_SIGNAL_ID_SMALLOC_RSP, &rsp);
    if (panic_on_failure && rsp.pointer == NULL)
    {
        panic(PANIC_SHARED_MEMORY_EXHAUSTION);
    }
    return rsp.pointer;
}



void ipc_malloc_msg_handler(IPC_SIGNAL_ID id, const void *msg)
{
    switch(id)
    {
    case IPC_SIGNAL_ID_PFREE:
    {
        void *pointer = ((const IPC_FREE *)msg)->pointer;
        pfree(pointer);
        break;
    }
    default:
        panic_diatribe(PANIC_IPC_UNHANDLED_MESSAGE_ID, id);
        /* no break */
    }
}
