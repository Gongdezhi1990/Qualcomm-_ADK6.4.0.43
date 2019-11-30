/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "ipc/ipc_private.h"


/**
 * Sends the supplied message. The caller must check there is enough space in the
 * buffer to send the message.
 *
 * \note This function must be called with interrupts blocked!
 *
 * @param msg_id ID of messae to send
 * @param msg Pointer to message body
 * @param len_bytes Length of message body
 */
static void ipc_send_no_checks(IPC_SIGNAL_ID msg_id, const void *msg, uint16 len_bytes)
{
    uint8 *send = buf_map_front_msg(ipc_data.send);
    memcpy((void *)send, msg, len_bytes);
    /* Set the ID on behalf of the caller */
    ((IPC_HEADER *)send)->id = msg_id;
    buf_add_to_front(ipc_data.send, (uint16)len_bytes);
    /* Raise IPC interrupt.  It doesn't matter what we write */
    hal_set_reg_interproc_event_1(1);
}

/**
 * Tries to send the message Signal Interproc Event. The attempt fails if there
 * is not enough space in the send buffer to copy the message. The only reason
 * it should happen is if the signal is already pending.
 */
static void ipc_send_signal_interproc_event(void)
{
    if (BUF_NUM_MSGS_AVAILABLE(ipc_data.send) &&
    (BUF_GET_FREESPACE(&ipc_data.send->buf) >=
     sizeof(IPC_SIGNAL_INTERPROC_EVENT_PRIM)))
    {
        IPC_SIGNAL_INTERPROC_EVENT_PRIM sig_msg;
        memset(&sig_msg, 0, sizeof(sig_msg));
        ipc_send_no_checks(
            IPC_SIGNAL_ID_SIGNAL_INTERPROC_EVENT,
            &sig_msg,
            sizeof(sig_msg)
            );
    }
}

/**
 * Attempt to send the supplied message
 *
 * \note This function must be called with interrupts blocked!
 *
 * @param msg_id ID of messae to send
 * @param msg Pointer to message body
 * @param len_bytes Length of message body
 * @return TRUE if the message was successfully added to the IPC send buffer,
 * else FALSE
 */
static bool ipc_try_send(IPC_SIGNAL_ID msg_id, const void *msg, uint16 len_bytes)
{
    /* Always leave space to send an interproc event message. See B-204884. */
    if ((BUF_NUM_MSGS_AVAILABLE(ipc_data.send) > 1) &&
        (BUF_GET_FREESPACE(&ipc_data.send->buf) >=
     len_bytes + sizeof(IPC_SIGNAL_INTERPROC_EVENT_PRIM)))
    {
        ipc_send_no_checks(msg_id, msg, len_bytes);
        return TRUE;
    }
    return FALSE;
}


/**
 * Place the supplied message on the back-up queue
 *
 * \note This function must be called with interrupts blocked!
 *
 * @param msg_id Message ID
 * @param msg Message body
 * @param len_bytes Length of message body
 */
static void ipc_queue_msg(IPC_SIGNAL_ID msg_id, const void *msg,
                                                            uint16 len_bytes)
{
    ipc_queue_msg_core(&ipc_data.send_queue, msg_id, msg, len_bytes);

    /* Schedule another attempt to send */
    GEN_BG_INT(ipc);
}

void ipc_queue_msg_core(IPC_MSG_QUEUE **pqueue, IPC_SIGNAL_ID msg_id,
                        const void *msg, uint16 len_bytes)
{
    IPC_MSG_QUEUE **pnext = pqueue, *new;
    void *mem;
    while(*pnext != NULL)
    {
        pnext = &((*pnext)->next);
    }
    /* Allocate a block big enough for both queue entry structure and the
     * message*/
    mem = pmalloc(sizeof(IPC_MSG_QUEUE) + len_bytes);
    new = (IPC_MSG_QUEUE *)mem;
    new->next = NULL;
    new->msg_id = msg_id;
    new->msg = (void *)((char *)mem + sizeof(IPC_MSG_QUEUE));
    memcpy(new->msg, msg, len_bytes);
    new->length_bytes = len_bytes;
    *pnext = new;
}

bool ipc_clear_queue(void)
{
    IPC_MSG_QUEUE **pnext = &ipc_data.send_queue;
    while(*pnext != NULL)
    {
        IPC_MSG_QUEUE *msg_entry = *pnext;
        if (ipc_try_send(msg_entry->msg_id, msg_entry->msg,
                                                    msg_entry->length_bytes))
        {
            /* remove this entry from the list and continue */
            *pnext = (*pnext)->next;
            /* the queue entry and message are in a single pmalloc block, so
             * just one pfree is required */
            pfree(msg_entry);
        }
        else
        {
            /* Ran out of space again... */
            return FALSE;
        }
    }
    assert(ipc_data.send_queue == NULL);
    return TRUE;
}


void ipc_send(IPC_SIGNAL_ID msg_id, const void *msg, uint16 len_bytes)
{

    /* Check the length is a multiple of 4 */
    assert(!(len_bytes & 3));

    block_interrupts();
    /* Try and clear the queue first; if that succeeds, try and send the message
     * If the message isn't sent, queue it */
    if (!ipc_clear_queue() || !ipc_try_send(msg_id, msg, len_bytes))
    {
        ipc_send_signal_interproc_event();
        ipc_queue_msg(msg_id, msg, len_bytes);
    }
    unblock_interrupts();

}



void ipc_send_outband(IPC_SIGNAL_ID msg_id, void *payload,
                                                    uint32 payload_len_bytes)
{
    IPC_TUNNELLED_PRIM_OUTBAND prim;

    prim.length = payload_len_bytes;
    prim.payload = payload;
    ipc_send(msg_id, &prim, sizeof(IPC_TUNNELLED_PRIM_OUTBAND));
}


void ipc_send_bool(IPC_SIGNAL_ID msg_id, bool val)
{
    IPC_BOOL_RSP rsp;
    rsp.ret = val;
    ipc_send(msg_id, &rsp, sizeof(rsp));
}

void ipc_send_uint16(IPC_SIGNAL_ID msg_id, uint16 val)
{
    IPC_UINT16_RSP rsp;
    rsp.ret = val;
    ipc_send(msg_id, &rsp, sizeof(rsp));
}

void ipc_send_int16(IPC_SIGNAL_ID msg_id, int16 val)
{
    IPC_INT16_RSP rsp;
    rsp.ret = val;
    ipc_send(msg_id, &rsp, sizeof(rsp));
}

void ipc_send_signal(IPC_SIGNAL_ID sig_id)
{
    IPC_SIGNAL sig;
    sig.header.id = sig_id;
    ipc_send(sig_id, &sig, sizeof(sig));
}
