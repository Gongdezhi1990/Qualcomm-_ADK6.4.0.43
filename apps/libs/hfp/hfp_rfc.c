/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_rfc.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"
#include "hfp_service_manager.h"
#include "hfp_common.h"
#include "hfp_rfc.h"
#include "hfp_init.h"
#include "hfp_receive_data.h"
#include "hfp_slc_handler.h"
#include "hfp_sdp.h"
#include "hfp_audio_handler.h"

#include <panic.h>
#include <stream.h>
#include <sink.h>
#include <print.h>

/*lint -e525 -e830 */


static void hfpHandleRfcommAppInd(const bdaddr* bd_addr, bool accepted)
{
    MAKE_HFP_MESSAGE(HFP_SLC_CONNECT_IND);
    message->addr = *bd_addr;
    message->accepted = accepted;
    MessageSend(theHfp->clientTask, HFP_SLC_CONNECT_IND, message);
}

/****************************************************************************
NAME    
    hfpHandleRfcommConnectInd

DESCRIPTION
    Notification of an incoming rfcomm connection, pass this up to the app
    to decide whather to accept this or not.

RETURNS
    void
*/
void hfpHandleRfcommConnectInd(const CL_RFCOMM_CONNECT_IND_T *ind)
{
    bool              accept  = FALSE;
    hfp_link_data*    link    = hfpGetLinkFromBdaddr(&ind->bd_addr);
    hfp_service_data* service = hfpGetServiceFromChannel(ind->server_channel);
    
    /* Ignore invalid sink */
    if(!SinkIsValid(ind->sink))
        return;
    
    /* If this device has no existing connection and the requested server channel is free */
    if(service && !service->bitfields.busy)
    {
        if(link)
        {
            /* We have a link for this device, what to do next... */
            MessageId message_id = hfpGetLinkTimeoutMessage(link, HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND);
            if(link->bitfields.ag_link_loss_state == hfp_link_loss_recovery && MessageCancelFirst(&theHfp->task, message_id))
            {
                /* Link was lost but we cancelled the next connection, accept incoming */
                accept = TRUE;
            }
            else
            {
                switch(link->bitfields.ag_slc_state)
                {
                    case hfp_slc_idle:
                        /* Should be impossible but regardless... */
                        accept = TRUE;
                        break;
                    case hfp_slc_searching:
                        /* Abort outgoing connection in favour of incoming */
                        hfpSendSlcConnectCfmToApp(link, NULL, hfp_connect_failed_crossover);
                        accept = TRUE;
                        break;
                    case hfp_slc_outgoing:
                    {
                        /* Check the link again in 500ms and see if outgoing RFCOMM has completed/failed */
                        MAKE_HFP_MESSAGE(CL_RFCOMM_CONNECT_IND);
                        *message = *ind;
                        MessageSendLater(&theHfp->task, CL_RFCOMM_CONNECT_IND, message, 500);
                        return;
                    }
                    default:
                        /* We have a connection already, reject */
                        break;
                }
            }
        }
        else
        {
            /* No existing link, check we have resources available */
            link = hfpGetIdleLink();
            if(link) 
                accept = TRUE;
        }
    }
    
    /* Set up the link */
    if(accept) 
        hfpLinkSetup(link, service, &ind->bd_addr, ind->sink, hfp_slc_incoming);
    
    /* Use default max payload */
    ConnectionRfcommConnectResponse(&theHfp->task, accept, ind->sink, ind->server_channel, NULL);
    
    /* Tell the app if we accepted this connection or not */
    hfpHandleRfcommAppInd(&ind->bd_addr, accept);
}


/****************************************************************************
NAME    
    hfpHandleRfcommConnectCfm

DESCRIPTION
    Outcome of the RFCOMM connect request or response.

RETURNS
    void
*/
static void hfpHandleRfcommConnectCfm(hfp_link_data* link, Sink sink, rfcomm_connect_status status)
{
    hfp_connect_status cfm_status;
    
    /* If there's no link we rejected so ignore cfm */
    if(link)
    {
        switch(status)
        {
            case rfcomm_connect_success:
            {
                /* Schedule check for data in the buffer */
                MESSAGE_MAKE(message, MessageMoreData);
                message->source = StreamSourceFromSink(sink);
                MessageSend(&theHfp->task, MESSAGE_MORE_DATA, message);
                
                hfpSetLinkSlcState(link, hfp_slc_connected);
                
                /* RFCOMM connection is up! Check which profile is supported by this task */
                if (hfpLinkIsHsp(link))
                {
                    /* HSP supported - SLC is up so tell the app */
                    cfm_status = hfp_connect_success;
                }
                else
                {
                    /* Initiate SLC establishment - Don't send anything to the app yet */
                    hfpHandleBrsfRequest(link);
                    return;
                }
            }
            break;
            
            case rfcomm_connect_pending:
                /* Store the sink */
                hfpSetLinkSink(link, sink);
            return;
            
            case rfcomm_connect_declined:
                /* Ignore - app doesn't need to know */
            return;
            
            case rfcomm_connect_channel_not_registered:
            case rfcomm_connect_security_not_set:
                /* Channel not set up correctly, should never happen... */
                cfm_status = hfp_connect_server_channel_not_registered;
            break;
            
            case rfcomm_connect_rejected:
                /* AG rejected connection request */
                cfm_status = hfp_connect_rejected;
            break;
            
            case rfcomm_connect_rejected_security:
                /* Security failure */
                cfm_status = hfp_connect_rejected_security;
            break;
            
            case rfcomm_connect_res_ack_timeout:
                /* Timeout failure */
                cfm_status = hfp_connect_timeout;
            break;
            
            case rfcomm_connect_channel_already_open:
            case rfcomm_connect_l2cap_error:
            case rfcomm_connect_failed:
            default:
                /* Generic failure */
                cfm_status = hfp_connect_failed;
            break;
        }
        
        hfpSendSlcConnectCfmToApp(link, NULL, cfm_status);
    }
}


/****************************************************************************
NAME    
    hfpHandleRfcommServerConnectCfm

DESCRIPTION
    Outcome of the RFCOMM connect response.

RETURNS
    void
*/
void hfpHandleRfcommServerConnectCfm(const CL_RFCOMM_SERVER_CONNECT_CFM_T* cfm)
{
    hfp_link_data* link = hfpGetLinkFromBdaddr(&cfm->addr);
    hfpHandleRfcommConnectCfm(link, cfm->sink, cfm->status);
}


/****************************************************************************
NAME    
    hfpHandleRfcommClientConnectCfm

DESCRIPTION
    Outcome of the RFCOMM connect request.

RETURNS
    void
*/
void hfpHandleRfcommClientConnectCfm(const CL_RFCOMM_CLIENT_CONNECT_CFM_T* cfm)
{
    hfp_link_data* link = hfpGetLinkFromBdaddr(&cfm->addr);
    hfpHandleRfcommConnectCfm(link, cfm->sink, cfm->status);
}


/****************************************************************************
NAME
    hfpHandleRfcommPortnegInd

DESCRIPTION
    Remote device has sent us a portneg request, respond

RETURNS
    void
*/
void hfpHandleRfcommPortnegInd(const CL_RFCOMM_PORTNEG_IND_T* ind)
{
    /* If this was a request send our default port params, otherwise accept any requested changes */
    ConnectionRfcommPortNegResponse(&theHfp->task, ind->sink, ind->request ? NULL : &ind->port_params);
}


static void hfpHandleRfcommDisconnect(Sink sink, rfcomm_disconnect_status status)
{
    /* Get the link that disconnected - panic if this fails */
    hfp_link_data* link = PanicNull(hfpGetLinkFromSink(sink));
    
    if (link->bitfields.ag_slc_state < hfp_slc_complete)
    {
        /* Rfcomm connection has been shutdown during the SLC connection process */
        /* Cancel the AT response timeout message because we'll have no more AT cmds being sent */
        (void) MessageCancelAll(&theHfp->task, hfpGetLinkTimeoutMessage(link, HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND));
        
        /* Report a failed connect attempt to app */
        hfpSendSlcConnectCfmToApp(link, NULL, hfp_connect_slc_failed);
    }
    else
    {
        if (link->audio_sink)
        {
            /* We have a SCO/eSCO active - tidy up and inform app of that disappearing first */
            hfpManageSyncDisconnect(link);
        }

        switch (status)
        {
            case rfcomm_disconnect_success:
            case rfcomm_disconnect_normal_disconnect:
                hfpSendSlcDisconnectIndToApp(link, hfp_disconnect_success);
            break;
            
            case rfcomm_disconnect_abnormal_disconnect:
                hfpSendSlcDisconnectIndToApp(link, hfp_disconnect_abnormally);
            break;
            
            case rfcomm_disconnect_l2cap_link_loss:
                hfpHandleRfcommLinkLoss(link);
            break;
            
            default:
                hfpSendSlcDisconnectIndToApp(link, hfp_disconnect_error);
            break;
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleRfcommDisconnectInd

DESCRIPTION
    Indication that the RFCOMM connection has been disconnected.

RETURNS
    void
*/
void hfpHandleRfcommDisconnectInd(const CL_RFCOMM_DISCONNECT_IND_T* ind)
{
    hfpHandleRfcommDisconnect(ind->sink, ind->status);
    ConnectionRfcommDisconnectResponse(ind->sink);
}


/****************************************************************************
NAME    
    hfpHandleRfcommDisconnectCfm

DESCRIPTION
    Indication that call to ConnectionRfcommDisconnect has completed

RETURNS
    void
*/
void hfpHandleRfcommDisconnectCfm(const CL_RFCOMM_DISCONNECT_CFM_T* cfm)
{
    /* Ignore CFM if link is already gone */
    if(cfm->status != rfcomm_disconnect_unknown_sink)
        hfpHandleRfcommDisconnect(cfm->sink, cfm->status);
}


/***************************************************************************
NAME    
    hfpHandleRfcommLinkLoss

DESCRIPTION
    Start link loss procedure
*/
void hfpHandleRfcommLinkLoss(hfp_link_data* link)
{
    /* If HFP configured to reconnect... */
    if(link->bitfields.manage_linkloss && theHfp->bitfields.link_loss_time)
    {
        /* Send timeout */
        MessageId message_id = hfpGetLinkTimeoutMessage(link, HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND);
        MessageSendLater(&theHfp->task, message_id, NULL, D_MIN(theHfp->bitfields.link_loss_time));
        PRINT(("Reconnect for %d mins\n", theHfp->bitfields.link_loss_time));
        
        /* Kick off reconnect */
        link->bitfields.ag_link_loss_state = hfp_link_loss_recovery;
    }
    
    /* Either attempt reconnect or send disconnect ind to app */
    hfpHandleRfcommLinkLossReconnect(link);
}


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossReconnect

DESCRIPTION
    Attempt to reconnect a dropped link
*/
void hfpHandleRfcommLinkLossReconnect(hfp_link_data* link)
{
    bdaddr bd_addr;
    
    if(link->bitfields.manage_linkloss && (link->bitfields.ag_link_loss_state == hfp_link_loss_recovery) && hfpGetLinkBdaddr(link, &bd_addr))
    {
        /* Remember the serviceto use*/
        hfp_service_data* service = link->service;
        
        /* Send a message to the app to let it know we're trying to reconnect */
        MAKE_HFP_MESSAGE(HFP_SLC_LINK_LOSS_IND);
        message->priority = hfpGetLinkPriority(link);
        message->status   = hfp_link_loss_recovery;
        
        /* Reset the link (but don't promote secondary) */
        hfpLinkReset(link, FALSE);
        
        /* Restore link information we want to keep */
        link->bitfields.ag_link_loss_state = hfp_link_loss_recovery;
        link->bitfields.ag_profiles_to_try = service->bitfields.profile;
        link->bitfields.priority           = message->priority;
        
        /* Kick off an SDP search */
        hfpGetProfileServerChannel(link, service, &bd_addr);
        
        MessageSend(theHfp->clientTask, HFP_SLC_LINK_LOSS_IND, message);
    }
    else
    {
        /* Give up on the reconnection */
        hfpSendSlcDisconnectIndToApp(link, hfp_disconnect_link_loss);
    }
}


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossComplete

DESCRIPTION
    Attempt to reconnect a dropped link has completed
*/
void hfpHandleRfcommLinkLossComplete(hfp_link_data* link, hfp_connect_status status)
{
    MessageId message_id;
    
    if(status == hfp_connect_success)
    {
        /* Tell the app the link is re-established */
        MAKE_HFP_MESSAGE(HFP_SLC_LINK_LOSS_IND);
        message->priority = hfpGetLinkPriority(link);
        message->status   = hfp_link_loss_none;
        MessageSend(theHfp->clientTask, HFP_SLC_LINK_LOSS_IND, message);
        
        /* Reset the link loss state */
        link->bitfields.ag_link_loss_state = hfp_link_loss_none;
        
        /* Cancel the link loss timeout */
        message_id = hfpGetLinkTimeoutMessage(link, HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND);
        (void)MessageCancelFirst(&theHfp->task, message_id);
    }
    else
    {
        /* Default to no delay (if timed out/aborted) */
        uint32 delay = 0;
        message_id = hfpGetLinkTimeoutMessage(link, HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND);
        
        /* Try and connect again in 10 seconds if still in recovery state */
        if(link->bitfields.ag_link_loss_state == hfp_link_loss_recovery)
            delay = D_SEC(theHfp->bitfields.link_loss_interval);
        
        /* Either schedule reconnect or send disconnect indication */
        MessageSendLater(&theHfp->task, message_id, NULL, delay);
    }
}


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossAbort

DESCRIPTION
    Attempt to reconnect a dropped link has been aborted by the application
*/
void hfpHandleRfcommLinkLossAbort(hfp_link_data* link, bool force_disconnect)
{
    MessageId message_id;
    /* Set the link loss state to abort */
    link->bitfields.ag_link_loss_state = hfp_link_loss_abort;
    
    message_id = hfpGetLinkTimeoutMessage(link, HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND);
    /* Attempt to cancel pending reconnect message */
    if(MessageCancelFirst(&theHfp->task, message_id))
    {
        /* If we cancelled the next reconnect then finish */
        hfpHandleRfcommLinkLossReconnect(link);
    }
    else if(force_disconnect)
    {
        /* Abort connection attempt in progresss */
        hfpSendCommonInternalMessage(HFP_INTERNAL_SLC_DISCONNECT_REQ, link);
    }
}


/***************************************************************************
NAME
    hfpHandleRfcommLinkLossTimeout

DESCRIPTION
    Attempt to reconnect a dropped link has timed out
*/
void hfpHandleRfcommLinkLossTimeout(hfp_link_data* link)
{
    /* Time out, but allow any attempt in progress to complete */
    hfpHandleRfcommLinkLossAbort(link, FALSE);
    /* Set the link loss state to timeout only if link loss abort is still not complete */
    if(link->bitfields.ag_link_loss_state == hfp_link_loss_abort)
    {
        link->bitfields.ag_link_loss_state = hfp_link_loss_timeout;
    }
}


/*lint +e525 +e830 */
