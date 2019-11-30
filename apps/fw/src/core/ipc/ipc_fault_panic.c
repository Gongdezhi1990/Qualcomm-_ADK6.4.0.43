/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Implements IPC signalling for P1 faults.
 */

#include "ipc/ipc_private.h"



void ipc_send_fault(uint16 id, uint16 n, TIME t, uint32 arg)
{
    IPC_FAULT_PANIC req;

    req.id = id;
    req.n = n;
    req.t = t;
    req.arg = arg;
    ipc_send(IPC_SIGNAL_ID_FAULT, &req, sizeof(IPC_FAULT_PANIC));
}

void ipc_send_panic(uint16 id, TIME t, uint32 arg)
{
    panic_data->p1_deathbed_confession = (panicid)id;
    panic_data->p1_diatribe = (DIATRIBE_TYPE)arg;
    panic_data->p1_t = t;
    hal_set_reg_interproc_event_2(1);
}

void panic_interrupt_handler(void)
{
    panic_from_p0(panic_data->p0_deathbed_confession, panic_data->p0_t, panic_data->p0_diatribe);
}

