/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"
#include "sched/sched.h"

void ipc_background_handler(void)
{
    /*
     * Attempt to send queued messages
     */
    block_interrupts();
    if (!ipc_clear_queue())
    {
        /* Couldn't post them all, so reschedule self */
        GEN_BG_INT(ipc);
    }
    unblock_interrupts();

    /*
     * Process messages in the IPC recv buffer without reference to any blocking
     * msg ID.  The background handler is only called when there *isn't* a
     * blocking call waiting - if there is, control returns from the interrupt
     * direct to ipc_recv() instead
     */
    if (ipc_recv_handler(NULL) != NULL)
    {
        /* The dynamic callback queue contained a dummy entry indicating a
         * blocking message and has returned us the payload.  This is
         * impossible*/
        panic(PANIC_IPC_UNEXPECTED_BLOCKING_MSG);
    }
}


void ipc_send_sched(qid remote_qid, uint16 mi, void *mv)
{
    IPC_SCHED_MSG_PRIM ipc_prim;
    ipc_prim.qid = remote_qid;
    ipc_prim.mi = mi;
    ipc_prim.mv = mv;
    ipc_send(IPC_SIGNAL_ID_SCHED_MSG_PRIM, &ipc_prim, sizeof(IPC_SCHED_MSG_PRIM));
}

void ipc_sched_handler(IPC_SIGNAL_ID id, const void *msg)
{
    const IPC_SCHED_MSG_PRIM *prim = (const IPC_SCHED_MSG_PRIM *)msg;
    assert(id == IPC_SIGNAL_ID_SCHED_MSG_PRIM);
    (void)put_message(prim->qid, prim->mi, prim->mv);
}
