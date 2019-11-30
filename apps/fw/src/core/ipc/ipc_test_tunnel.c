/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"

#include "gen/k32/test_tunnel_prim.h"
#include "appcmd/appcmd.h"


void ipc_test_tunnel_handler(IPC_SIGNAL_ID id, const void *msg,
                                                uint16 msg_length_bytes)
{
    /*
     * Test tunnel primitives are sent out-of-band from IPC's point of view
     */
    const uint32 *test_tunnel_prim;
    const TEST_TUNNEL_HEADER *test_tunnel_hdr;
    const IPC_TUNNELLED_PRIM_OUTBAND *prim =
                                        (const IPC_TUNNELLED_PRIM_OUTBAND *)msg;

    switch(id)
    {
    case IPC_SIGNAL_ID_TEST_TUNNEL_PRIM:
        UNUSED(prim);
        UNUSED(test_tunnel_hdr);
        UNUSED(test_tunnel_prim);
        UNUSED(msg_length_bytes);
        /* When test tunnel primitives are sent to P1 they are probably
         * always going to be customer API driver messages
         * We'll just call functions directly */
        break;
    default:
        panic_diatribe(PANIC_IPC_UNHANDLED_MESSAGE_ID, id);
    }
}
