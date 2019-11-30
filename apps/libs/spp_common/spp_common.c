/* Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd. */
/*  

FILE NAME
	spp_common.c        

DESCRIPTION
	Common SPP functions shared between the SPP Client (Device A) and the 
	SPP Server (Device B).

*/

#include <stdlib.h>
#include "spp_common.h"
#include "spp_common_private.h"

struct __SPP
{
    SPP_COMMON c;
};

#define CASE(from,to)  case (from): if ((from)==(to)) goto coerce; else return (to);

/*****************************************************************************/

spp_connect_status sppConvertRfcommConnectStatus(rfcomm_connect_status status)
{
    switch(status)
    {
    CASE(rfcomm_connect_success, spp_connect_success);
    CASE(rfcomm_connect_pending, spp_connect_pending);
    CASE(rfcomm_connect_channel_not_registered, spp_connect_channel_not_registered);
    CASE(rfcomm_connect_security_not_set, spp_connect_security_not_set);
    CASE(rfcomm_connect_declined, spp_connect_declined);
    CASE(rfcomm_connect_rejected, spp_connect_rejected);
    CASE(rfcomm_connect_channel_already_open, spp_connect_channel_already_open);
    CASE(rfcomm_connect_rejected_security, spp_connect_rejected_security);
    CASE(rfcomm_connect_res_ack_timeout, spp_connect_res_ack_timeout);
    CASE(rfcomm_connect_l2cap_error, spp_connect_l2cap_error);

    coerce: return (rfcomm_disconnect_status)status;       
    default:
        return spp_connect_failed;  
    }
}

/*****************************************************************************/

spp_disconnect_status sppConvertRfcommDisconnectStatus(rfcomm_disconnect_status status)
{
    switch(status)
    {
    CASE(rfcomm_disconnect_success, spp_disconnect_success);
    CASE(rfcomm_disconnect_normal_disconnect, spp_disconnect_normal_disconnect);
    CASE(rfcomm_disconnect_abnormal_disconnect, spp_disconnect_abnormal_disconnect);
    CASE(rfcomm_disconnect_remote_refusal, spp_disconnect_remote_refusal);
    CASE(rfcomm_disconnect_dlc_already_exists, spp_disconnect_dlc_already_exists);
    CASE(rfcomm_disconnect_dlc_rej_security, spp_disconnect_dlc_rej_security);
    CASE(rfcomm_disconnect_invalid_server_channel, spp_disconnect_invalid_server_channel);
    CASE(rfcomm_disconnect_unknown_primitive, spp_disconnect_unknown_primitive);
    CASE(rfcomm_disconnect_max_payload_exceeded, spp_disconnect_max_payload_exceeded);
    CASE(rfcomm_disconnect_inconsistent_parameters, spp_disconnect_inconsistent_parameters);
    CASE(rfcomm_disconnect_credit_flow_control_protocol_violation, spp_disconnect_credit_flow_control_protocol_violation);
    CASE(rfcomm_disconnect_unknown_sink, spp_disconnect_unknown_sink);
    CASE(rfcomm_disconnect_l2cap_link_loss, spp_disconnect_l2cap_link_loss);
    CASE(rfcomm_disconnect_l2cap_error, spp_disconnect_l2cap_error);
    
    coerce: return (rfcomm_disconnect_status)status;       
    default:
        return spp_disconnect_unknown;    
    }
}

/*****************************************************************************/

void sppSendConnectCfm(SPP *spp, MessageId id, spp_connect_status status)
{
    /* SPP_CLIENT_CONNECT_CFM and SPP_SERVER_CONNECT_CFM have the same structure
     * so it does not matter functionally which is used here.
     */
    MAKE_SPP_MESSAGE(SPP_CLIENT_CONNECT_CFM);

#ifdef SPP_DEBUG_LIB
    if (SPP_CLIENT_CONNECT_CFM != id && SPP_SERVER_CONNECT_CFM !=id)
        SPP_DEBUG(("Invalid Message id 0x%x\n", id));
#endif

    message->status     = status;
    if (status == spp_connect_success || status == spp_connect_pending)
    {
        message->spp            = spp;
        message->sink           = spp->c.sink;
        message->payload_size   = spp->c.max_payload_size;
    }
    else
    {
        message->spp            = 0;
        message->sink           = 0;
        message->payload_size   = 0;
    }
    MessageSend(spp->c.client_task, id, message);

    /* If the Connection failed then go to Disconnect state to tidy up. */
    if (status != spp_connect_success && status != spp_connect_pending)
    {
        SPP_STATE(sppDisconnecting);
        MessageSend(&spp->c.task, SPP_INTERNAL_DISCONNECT_REQ, 0);
    }
}

/*****************************************************************************/

void sppHandleUnexpectedMsg(sppState state, MessageId id)
{
    state = state;
    id = id;
    SPP_DEBUG(("Unhandled message 0x%x, in SPPC State %d\n", id, state));
}

/*****************************************************************************/

void sppDeleteTask(SPP *spp)
{
    (void)MessageFlushTask(&spp->c.task);
    free(spp);
    SPP_PRINT(("SPP Task deleted.\n"));
}

/*****************************************************************************/

void sppHandleRfcommDisconnectInd(SPP *spp, CL_RFCOMM_DISCONNECT_IND_T *ind)
{
    MAKE_SPP_MESSAGE(SPP_DISCONNECT_IND);

    SPP_STATE(sppDisconnecting);
    
    message->status = sppConvertRfcommDisconnectStatus(ind->status);
    message->sink = ind->sink;
    message->spp = spp;
    MessageSend(spp->c.client_task, SPP_DISCONNECT_IND, message);
}

/*****************************************************************************/

void SppDisconnectResponse(SPP *spp)
{
#ifdef SPP_DEBUG_LIB
    if (!spp)
        SPP_DEBUG(("spp instance pointer is NULL!\n"));

    if (spp->c.state != sppDisconnecting)
        SPP_DEBUG(("spp task is not in the Disconnecting state!\n"));
#endif
    
    ConnectionRfcommDisconnectResponse(spp->c.sink);

    /* State should already be sppDisconnecting state. Send
     * the internal disconnect request to perform any tidy up
     * functions, which are different depending on if the 
     * SPP task is a Client or a Server.
     */
    MessageSend(&spp->c.task, SPP_INTERNAL_DISCONNECT_REQ, 0);
}

/*****************************************************************************/

void SppDisconnectRequest(SPP *spp)
{
#ifdef SPP_DEBUG_LIB
    if (!spp)
        SPP_DEBUG(("spp instance pointer is NULL!\n"));
#endif
    SPP_STATE(sppDisconnecting);
    ConnectionRfcommDisconnectRequest(&spp->c.task, spp->c.sink);
}

/*****************************************************************************/

void sppHandleRfcommDisconnectCfm(SPP *spp, const CL_RFCOMM_DISCONNECT_CFM_T *cfm)
{
    MAKE_SPP_MESSAGE(SPP_DISCONNECT_CFM);
    message->spp = spp;
    message->status = sppConvertRfcommDisconnectStatus(cfm->status);
    message->sink = cfm->sink;
    MessageSend(spp->c.client_task, SPP_DISCONNECT_CFM, message);
    
    /* State should already be sppDisconnecting state. Send
     * the internal disconnect request to perform any tidy up
     * functions, which are different depending on if the 
     * SPP task is a Client or a Server.
     */
    MessageSend(&spp->c.task, SPP_INTERNAL_DISCONNECT_REQ, 0);

}


/*****************************************************************************/

void sppForwardRfcommControlInd(SPP *spp, const CL_RFCOMM_CONTROL_IND_T *ind)
{
    MAKE_SPP_MESSAGE(CL_RFCOMM_CONTROL_IND);
    *message = *ind;
    MessageSend(spp->c.client_task, CL_RFCOMM_CONTROL_IND, message);
}

/*****************************************************************************/

void sppForwardSmEncryptionChangeInd(SPP *spp, const CL_SM_ENCRYPTION_CHANGE_IND_T *ind)
{
    MAKE_SPP_MESSAGE(CL_SM_ENCRYPTION_CHANGE_IND);
    *message = *ind;
    MessageSend(spp->c.client_task, CL_SM_ENCRYPTION_CHANGE_IND, message);
}

/*****************************************************************************/

void sppHandleRfcommPortnegInd(SPP *spp, CL_RFCOMM_PORTNEG_IND_T *ind)
{
    /* If this was a request send our default port params, otherwise accept any requested changes */
    ConnectionRfcommPortNegResponse(&spp->c.task, ind->sink, ind->request ? NULL : &ind->port_params);
}

/*****************************************************************************/

void sppForwardMessageMoreData(SPP *spp, const MessageMoreData *msg)
{
    
    MAKE_SPP_MESSAGE(SPP_MESSAGE_MORE_DATA);
    message->source = msg->source;
    message->spp = spp;
    MessageSend(spp->c.client_task, SPP_MESSAGE_MORE_DATA, message);
}

/*****************************************************************************/

void sppForwardMessageMoreSpace(SPP *spp, const MessageMoreSpace *msg)
{
    MAKE_SPP_MESSAGE(SPP_MESSAGE_MORE_SPACE);
    message->sink = msg->sink;
    message->spp = spp;
    MessageSend(spp->c.client_task, SPP_MESSAGE_MORE_SPACE, message);
}

