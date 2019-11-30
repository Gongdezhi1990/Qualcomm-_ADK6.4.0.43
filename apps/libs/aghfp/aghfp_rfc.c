/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_common.h"
#include "aghfp_rfc.h"
#include "aghfp_init.h"
#include "aghfp_receive_data.h"
#include "aghfp_slc_handler.h"
#include "aghfp_call_manager.h"

#include <panic.h>
#include <stream.h>

/*lint -e525 -e830 */

/* Convert from the rfcomm_connect_status returned by the connection lib. */
static aghfp_connect_status convertRfcommConnectStatus(rfcomm_connect_status status)
{
    switch (status)
    {
    case rfcomm_connect_success:
        return aghfp_connect_success;

    case rfcomm_connect_failed:
        return aghfp_connect_failed;

    case rfcomm_connect_pending:
    case rfcomm_connect_security_not_set:
    case rfcomm_connect_declined:
    case rfcomm_connect_channel_already_open:
    case rfcomm_connect_rejected_security:
    case rfcomm_connect_l2cap_error:
        /* TODO - Add more specific AGHFP error codes? */
        return aghfp_connect_failed;
        
    case rfcomm_connect_channel_not_registered:
        return aghfp_connect_server_channel_not_registered;

    case rfcomm_connect_res_ack_timeout:
        return aghfp_connect_timeout;

    case rfcomm_connect_rejected:
        return aghfp_connect_rejected;
        
    /* also check L2CAP status */
    case l2cap_connect_failed_security:
        return aghfp_connect_security_reject;

    default:
        /* All rfcomm disconnects should be handled above if we get this panic in debug lib */
        AGHFP_DEBUG_PANIC(("Unhandled rfc connect status 0x%x\n", status));

		/* Return generic connect fail in "release" lib variant */
        return aghfp_connect_failed;
    }
}


/* Convert from the rfcomm_disconnect_status returned by the connection lib. */
static aghfp_disconnect_status convertRfcommDisconnectStatus(rfcomm_disconnect_status status)
{
	switch (status)
	{
		case rfcomm_disconnect_success:
		case rfcomm_disconnect_normal_disconnect:
			return aghfp_disconnect_success;

		case rfcomm_disconnect_abnormal_disconnect:
        case rfcomm_disconnect_l2cap_link_loss:
			return aghfp_disconnect_link_loss;

        case rfcomm_disconnect_remote_refusal:
        case rfcomm_disconnect_dlc_already_exists:
        case rfcomm_disconnect_dlc_rej_security:
        case rfcomm_disconnect_invalid_server_channel:
        case rfcomm_disconnect_unknown_primitive:
        case rfcomm_disconnect_max_payload_exceeded:
        case rfcomm_disconnect_inconsistent_parameters:
        case rfcomm_disconnect_credit_flow_control_protocol_violation:
        case rfcomm_disconnect_unknown_sink:        
        case rfcomm_disconnect_l2cap_error:
        case rfcomm_disconnect_unknown:
		default:
			return aghfp_disconnect_error;
	}
}


/****************************************************************************
	Request an RFCOMM channel to be allocated.
*/
void aghfpHandleRfcommAllocateChannel(AGHFP *aghfp)
{
	/* Make the call to the connection lib */
	ConnectionRfcommAllocateChannel(&aghfp->task, 0);
}


/****************************************************************************
 Rfcomm channel has been allocated.
*/
void aghfpHandleRfcommRegisterCfm(AGHFP *aghfp, const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
	/* Check the result code */
	if (cfm->status == success)
	{
		/* Rfcomm channel allocation succeeded send an init cfm */
		aghfpSendInternalInitCfm(&aghfp->task, aghfp_init_success, cfm->server_channel);
	}
	else
	{
		/* Send an init cfm with error code indicating channel alloc failed */
		aghfpSendInternalInitCfm(&aghfp->task, aghfp_init_rfc_chan_fail, 0);
	}
}


/****************************************************************************
 Issue a request to the connection lib to create an RFCOMM connection.
*/
void aghfpHandleRfcommConnectRequest(AGHFP *aghfp, const AGHFP_INTERNAL_RFCOMM_CONNECT_REQ_T *req)
{
	if(!(aghfp->rfcomm_lock))
	{
		aghfp->rfcomm_lock = TRUE;
    	/* Issue the connect request to the connection lib. */
   		ConnectionRfcommConnectRequest(&aghfp->task, &req->addr, aghfp->local_rfc_server_channel, req->rfc_channel, 0);
	}
}


/****************************************************************************
 Outcome of the RFCOMM connect request or response.
*/
static void aghfpHandleRfcommConnectCfm(AGHFP *aghfp, rfcomm_connect_status status)
{
    /* Check the status of the rfcomm connect cfm */
    if (status == rfcomm_connect_success)
    {
        /* RFCOMM connection is up! Check which profile is supported by this task */
        if (supportedProfileIsHsp(aghfp->supported_profile))
        {
            /* HSP supported - SLC is up so tell the app */
   			aghfpSendSlcConnectCfmToApp(aghfp_connect_success, aghfp);
        }
        else if (supportedProfileIsHfp(aghfp->supported_profile))
        {
            /* HFP supported - RFCOMM is up, so just wait for HF to send us some AT commends */
        }
        else
        {
            /* This should never happen */
            AGHFP_DEBUG_PANIC(("Unhandled profile type 0x%x\n", aghfp->supported_profile));
        }

  		/* Check for data in the buffer */
  		aghfpHandleReceivedData(aghfp, StreamSourceFromSink(aghfp->rfcomm_sink));
    }
    else
    {
        /* RFCOMM connect failed - Tell the app. */
        aghfpSendSlcConnectCfmToApp(convertRfcommConnectStatus(status), aghfp);
    }
	
	aghfp->rfcomm_lock = FALSE;
}


/****************************************************************************
 Outcome of the RFCOMM connect request.
*/
void aghfpHandleRfcommClientConnectCfm(AGHFP *aghfp, CL_RFCOMM_CLIENT_CONNECT_CFM_T* cfm)
{
    if(cfm->status == rfcomm_connect_pending)
        aghfp->rfcomm_sink = cfm->sink;
    else
        aghfpHandleRfcommConnectCfm(aghfp, cfm->status);
}


/****************************************************************************
 Outcome of the RFCOMM connect response.
*/
void aghfpHandleRfcommServerConnectCfm(AGHFP *aghfp, CL_RFCOMM_SERVER_CONNECT_CFM_T* cfm)
{
    aghfpHandleRfcommConnectCfm(aghfp, cfm->status);
}


/****************************************************************************
	Response to an incoming RFCOMM connect request.
*/
void aghfpHandleRfcommConnectResponse(AGHFP *aghfp, bool response, Sink sink, uint8 server_channel, const rfcomm_config_params *config)
{
	/* Issue a reject without passing this up to the app */
	ConnectionRfcommConnectResponse(&aghfp->task, response, sink, server_channel, config);
}


/****************************************************************************
	Notification of an incoming rfcomm connection, pass this up to the app
	to decide whether to accept this or not.
*/
void aghfpHandleRfcommConnectInd(AGHFP *aghfp, const CL_RFCOMM_CONNECT_IND_T *ind)
{
	/* Ask the app whether to accept this connection or not */
	MAKE_AGHFP_MESSAGE(AGHFP_SLC_CONNECT_IND);
	message->bd_addr = ind->bd_addr;
	message->aghfp = aghfp;
	MessageSend(aghfp->client_task, AGHFP_SLC_CONNECT_IND, message);

	/* Update the local state to indicate we're in the middle of connecting. */
	aghfpSetState(aghfp, aghfp_slc_connecting);
    
    /* Store the sink */
    aghfp->rfcomm_sink = ind->sink;
}


/****************************************************************************
	Issue an RFCOMM disconnect to the connection lib.
*/
void aghfpHandleRfcommDisconnectRequest(AGHFP *aghfp)
{
	/* Request the connection lib disconnects the RFCOMM connection */
	ConnectionRfcommDisconnectRequest(&aghfp->task, aghfp->rfcomm_sink);
}


/****************************************************************************
    Handle RFCOMM disconnection
*/
static void aghfpHandleRfcommDisconnect(AGHFP *aghfp, rfcomm_disconnect_status status)
{
    if ( aghfpCallManagerActive(aghfp) )
    {
        /* Inform Call Manager */
        aghfpManageCall(aghfp, CallEventSlcRemoved, aghfpConvertDisconnectStatusToCallFlag(convertRfcommDisconnectStatus(status)));
    }
    else
    {
        /* Convert the rfc disconnect status into its aghfp counterpart and send to app */
        aghfpSendSlcDisconnectIndToApp(aghfp, convertRfcommDisconnectStatus(status));
    }
}


/****************************************************************************
    Confirmation that the RFCOMM connection has been disconnected.
*/
void aghfpHandleRfcommDisconnectCfm(AGHFP *aghfp, const CL_RFCOMM_DISCONNECT_CFM_T *cfm)
{
    aghfpHandleRfcommDisconnect(aghfp, cfm->status);
}


/****************************************************************************
    Indication that the RFCOMM connection has been disconnected.
*/
void aghfpHandleRfcommDisconnectInd(AGHFP *aghfp, const CL_RFCOMM_DISCONNECT_IND_T *ind)
{
    ConnectionRfcommDisconnectResponse(aghfp->rfcomm_sink);
    aghfpHandleRfcommDisconnect(aghfp, ind->status);
}


/****************************************************************************
	Indication of port negotiation request.
*/
void aghfpHandleRfcommPortnegInd(AGHFP *aghfp, CL_RFCOMM_PORTNEG_IND_T* ind)
{
    /* If this was a request send our default port params, otherwise accept any requested changes */
    ConnectionRfcommPortNegResponse(&aghfp->task, ind->sink, ind->request ? NULL : &ind->port_params);
}

/*lint +e525 +e830 */
