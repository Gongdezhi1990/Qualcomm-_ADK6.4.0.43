/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implements IPC support for PIO traps.
 */

#include "ipc/ipc_private.h"


void ipc_send_pio(IPC_PIO *req, IPC_SIGNAL_ID signal)
{
    ipc_send(signal, req, sizeof(IPC_PIO));
    (void)ipc_recv(IPC_SIGNAL_ID_PIO_RSP, req);
}

