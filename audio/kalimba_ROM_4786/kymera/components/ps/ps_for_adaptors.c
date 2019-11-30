/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file ps_for_adaptors.c
 * \ingroup ps
 *
 */

/****************************************************************************
Include Files
*/
#include "ps_for_adaptors.h"
#include "ps/ps_msg.h"
#include "sched_oxygen/sched_oxygen.h"
#include "pmalloc/pl_malloc.h"
#ifdef INSTALL_PS_FOR_ACCMD
#include "kip_msg_prim.h"
#include "ps_router/ps_router.h"
#include "fault/fault.h"
#include "sys_events.h"
#endif

#ifndef INSTALL_PS_FOR_ACCMD
void ps_signal_shutdown(unsigned conn_id, PS_SHUTDOWN_CALLBACK cback)
{
    tRoutingInfo routing;
    PS_MSG_DATA* ps_msg;

    /* Build the routing info, destination is not used in this case. */
    routing.src_id = conn_id;
    routing.dest_id = 0;

    /* Construct message to PS handler */
    ps_msg = xpnew(PS_MSG_DATA);

    /* If there isn't enough RAM at this point then we're screwed and the system
     * will have to cope with no tidy up happening.
     */
    if (ps_msg != NULL)
    {
        /* there is no PS data in this message */
        ps_msg->data_length = 0;
        ps_msg->callback = (void*)cback;
        ps_msg->key = 0;
        ps_msg->rank = 0;
        NOT_USED(routing);
        /* drop message on the platform-specific PS handler queue */
        put_message_with_routing(PS_MSG_TASK_QUEUE_ID, PS_SYSTEM_SHUTDOWN_MSG, (void*)ps_msg, &routing);
    }
}
#else /* INSTALL_PS_FOR_ACCMD */

/* TODO: Is it possible to get rid of argument cback? */
void ps_signal_shutdown(unsigned conn_id, PS_SHUTDOWN_CALLBACK cback)
{
    KIP_MSG_PS_SHUTDOWN_REQ *msg;
    PS_ROUTER_MSG *emsg;

    /* Allocated memory is freed in ps_router_shutdown_req. */
    msg = xpnew(KIP_MSG_PS_SHUTDOWN_REQ);
    if (msg == NULL)
    {
        fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, sizeof(KIP_MSG_PS_SHUTDOWN_REQ));
        return;
    }

    /* Build message. */
    KIP_MSG_PS_SHUTDOWN_REQ_CON_ID_SET(msg, 0);

    /* Allocate memory for encapsulated message. */
    /* Allocated memory is freed in ps_router_handler. */
    emsg = xpnew(PS_ROUTER_MSG);
    if (emsg == NULL)
    {
        pdelete(msg);
        fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, sizeof(PS_ROUTER_MSG));
        return;
    }

    /* Encapsulate message. */
    emsg->msg_id = KIP_MSG_ID_PS_SHUTDOWN_REQ;
    emsg->msg_length = (uint16) KIP_MSG_PS_SHUTDOWN_REQ_WORD_SIZE;
    emsg->msg_data = (uint16*) msg;
    
    put_message(PS_ROUTER_TASK_QUEUE_ID, KIP_MSG_ID_PS_SHUTDOWN_REQ, emsg);
}

void ps_read_resp(unsigned success, unsigned total_size, unsigned payload_size, uint16 *payload)
{
    KIP_MSG_PS_READ_RES *msg;
    PS_ROUTER_MSG *emsg;

    /* Allocated memory is freed in ps_router_read_resp. */
    msg = (KIP_MSG_PS_READ_RES*) xpnewn(KIP_MSG_PS_READ_RES_DATA_WORD_OFFSET + payload_size, uint16);
    if (msg == NULL)
    {
        fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY,
                       (KIP_MSG_PS_READ_RES_DATA_WORD_OFFSET + payload_size) * sizeof(uint16));
        return;
    }

    /* Build message. */
    KIP_MSG_PS_READ_RES_STATUS_SET(msg, success); /* TODO: Probably need fixing... */
    KIP_MSG_PS_READ_RES_CON_ID_SET(msg, 0);
    KIP_MSG_PS_READ_RES_KEY_ID_SET(msg, 0); /* TODO: Should not be needed. */
    KIP_MSG_PS_READ_RES_TOTAL_LEN_SET(msg, (uint16)total_size);
    memcpy(&msg->_data[KIP_MSG_PS_READ_RES_DATA_WORD_OFFSET], payload, payload_size * sizeof(uint16));
    
    /* Allocated memory is freed in ps_router_handler. */
    emsg = xpnew(PS_ROUTER_MSG);
    if (emsg == NULL)
    {
        pdelete(msg);
        fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, sizeof(PS_ROUTER_MSG));
        return;       
    }
    
    /* Encapsulate message. */
    emsg->msg_id = KIP_MSG_ID_PS_READ_RES;
    emsg->msg_length = (uint16) (KIP_MSG_PS_READ_RES_DATA_WORD_OFFSET + payload_size);
    emsg->msg_data = (uint16*) msg;
    put_message(PS_ROUTER_TASK_QUEUE_ID, KIP_MSG_ID_PS_READ_RES, (void*)emsg);
}

void ps_write_resp(unsigned success)
{
    KIP_MSG_PS_WRITE_RES *msg;
    PS_ROUTER_MSG *emsg;

    /* Allocated memory is freed in ps_router_write_resp. */
    msg = xpnew(KIP_MSG_PS_WRITE_RES);
    if (msg == NULL)
    {
        fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, sizeof(KIP_MSG_PS_WRITE_RES));
        return;
    }

    /* Build message. */
    KIP_MSG_PS_WRITE_RES_STATUS_SET(msg, success ? 0 : 1);
    KIP_MSG_PS_WRITE_RES_CON_ID_SET(msg, 0);
    KIP_MSG_PS_WRITE_RES_KEY_ID_SET(msg, 0); /* TODO: Should not be needed. */
    KIP_MSG_PS_WRITE_RES_RANK_SET(msg, 0); /* TODO: Should not be needed. */

    /* Allocated memory is freed in ps_router_handler. */
    emsg = xpnew(PS_ROUTER_MSG);
    if (emsg == NULL)
    {
        pdelete(msg);
        fault_diatribe(FAULT_AUDIO_INSUFFICIENT_MEMORY, sizeof(PS_ROUTER_MSG));
        return;
    }

    /* Encapsulate message. */
    emsg->msg_id = KIP_MSG_ID_PS_WRITE_RES;
    emsg->msg_length = (uint16) KIP_MSG_PS_WRITE_RES_WORD_SIZE;
    emsg->msg_data = (uint16*) msg;
    
    put_message(PS_ROUTER_TASK_QUEUE_ID, KIP_MSG_ID_PS_WRITE_RES, emsg);
}

void ps_register(unsigned conn_id)
{
    set_system_event(SYS_EVENT_PS_READY);
    ps_router_set_conn_id(conn_id);
}
#endif