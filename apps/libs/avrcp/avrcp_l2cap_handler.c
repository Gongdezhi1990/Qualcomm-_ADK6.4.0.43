/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_l2cap_handler.c
    
DESCRIPTION
    Internal functions to handle all Connection library 
    provided L2CAP interfaces.

*/
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <panic.h>
#include <sink.h>
#include <stream.h>
#include <connection.h>

#include "avrcp_l2cap_handler.h"
#include "avrcp_browsing_handler.h"
#include "avrcp_init.h"
#include "avrcp_profile_handler.h"

/****************************************************************************
* NAME    
*   convertL2capDisconnectStatus
*
*DESCRIPTION
*  Covert the CL provided status to avrcp Status code
****************************************************************************/
static avrcp_status_code convertL2capDisconnectStatus(
                                l2cap_disconnect_status l2cap_status)
{
    switch(l2cap_status)
    {
        case l2cap_disconnect_successful: return avrcp_success;
        case l2cap_disconnect_timed_out: return avrcp_timeout;        
        case l2cap_disconnect_error: return avrcp_fail;
        case l2cap_disconnect_no_connection: return avrcp_device_not_connected;
        case l2cap_disconnect_link_loss: return avrcp_link_loss;
        default: return avrcp_fail;
    }
}


/****************************************************************************
*NAME    
*    avrcpHandleL2capRegisterCfm
*
*DESCRIPTION
*    This function is called on receipt of an CL_L2CAP_REGISTER_CFM.
*****************************************************************************/
void avrcpHandleL2capRegisterCfm( const CL_L2CAP_REGISTER_CFM_T *cfm)
{
    if (cfm->status == success)
    {
        if(cfm->psm == AVCTP_BROWSING_PSM)
        {
            ConnectionL2capRegisterRequest( avrcpGetInitTask(), AVCTP_PSM, 0); 
        }
        else
        { 
            /* Register the AVRCP service record */
            avrcpRegisterServiceRecord(avrcpGetDeviceTask());
        }
    }
    else
    {
        avrcpSendInitCfmToClient((cfm->psm == AVCTP_PSM)?
                                  avrcp_unreg_browse:
                                  avrcp_unreg_none, 0, avrcp_fail);

    }
}


/****************************************************************************
*NAME    
*    avrcpHandleL2capConnectCfm
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_CONNECT_CFM message
*    indicating the outcome of the connect attempt.
*****************************************************************************/
void avrcpHandleL2capConnectCfm(AVRCP *avrcp, const CL_L2CAP_CONNECT_CFM_T *cfm)
{    
    avrcp_status_code status = avrcp_fail;

    AVRCP_INFO(("avrcpHandleL2capConnectCfm: status %u\n", cfm->status));

    if(cfm->status == l2cap_connect_pending) return;

    /* Inform SDP State machine about L2CAP connection Complete */
    avrcpSdpStateOnConnect(avrcp);

    if (cfm->status == l2cap_connect_success)
    {
        status = avrcp_success;
        
        /* Break any Source connections */
        StreamDisconnect(StreamSourceFromSink(cfm->sink), 0);

        /* Update the profile state to indicate we're connected */
        avrcpSetState(avrcp, avrcpConnected);

        /* Store the connection sink */
        avrcp->sink = cfm->sink;

        /* Store the bluetooth address of the device.*/
        memcpy(&avrcp->bd_addr, &(cfm->addr), sizeof(bdaddr));

        /* Store the mtu negotiated */
        avrcp->l2cap_mtu = cfm->mtu_remote;

         /* Check for data in the buffer */
        avrcpSourceProcessed(avrcp, TRUE);
    }
    else
    {
        avrcp_sdp_search temp = (avrcp_sdp_search)avrcp->sdp_search_mode;

        if (cfm->status == l2cap_connect_timeout)
            status = avrcp_timeout;
        else
            status = avrcp_fail;

        /* If the connect attempt failed, reset the local state. */
        avrcpResetValues(avrcp);
        
        /* keep the search mode intact and disable the search repeat */
        avrcp->sdp_search_mode = temp;
        avrcp->bitfields.sdp_search_repeat = 1;
    }    

    /* Send the cfm message to the client */
    avrcpSendConnectCfmToApp(avrcp, status, &cfm->addr, cfm->sink);
}


/****************************************************************************
*NAME    
*    avrcpHandleL2capConnectInd
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_CONNECT_IND message.
*    This message indicates that a remote device is attempting to establish
*    an L2CAP connection to this device on the AVCTP PSM.
******************************************************************************/
void avrcpHandleL2capConnectInd(const CL_L2CAP_CONNECT_IND_T *ind)
{        
    if (ind->psm == AVCTP_PSM)
    {
        AVRCP *avrcp;

        /* Check if there's a task for this device already, if not create task */
        avrcp = avrcpFindTask(&ind->bd_addr);
        if (avrcp)
        {
            if (avrcp->bitfields.state != avrcpReady)
            {
                AVRCP_INFO(("avrcpHandleL2capConnectInd: rejecting\n"));

                /* Create new temporary task to reject this connection */
                avrcp = avrcpCreateTask(NULL, avrcpConnecting, &ind->bd_addr);

                /* Reject incoming connection */
                ConnectionL2capConnectResponse(&avrcp->task, FALSE,
                                               ind->psm, ind->connection_id, ind->identifier,
                                               0, NULL);

                /* We're done, so return */
                return;
            }
        }
        else
        {
            AVRCP_INFO(("avrcpHandleL2capConnectInd: creating task\n"));
            avrcp = avrcpCreateTask(avrcpGetDeviceTask()->app_task, avrcpConnecting, &ind->bd_addr);
        }

        /* Pass connection indication to application */
        {
            MAKE_AVRCP_MESSAGE(AVRCP_CONNECT_IND);
            message->bd_addr       = ind->bd_addr;
            message->connection_id = ind->connection_id;
            message->signal_id     = ind->identifier;
            MessageSend(avrcpGetDeviceTask()->app_task, AVRCP_CONNECT_IND,
                        message);
        }
    }
    else
    {
        /* Browsing channel */
        MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_CONNECT_IND);
        message->bd_addr       = ind->bd_addr;
        message->connection_id = ind->connection_id;
        message->signal_id     = ind->identifier;
        MessageSend(avrcpGetDeviceTask()->app_task, AVRCP_BROWSE_CONNECT_IND,
                    message);
    }
}


/****************************************************************************
*NAME
*    avrcpHandleL2capConnectIndReject
*
*DESCRIPTION
*    The profile instance is in the wrong state, automatically reject the
*    connect request.
*****************************************************************************/
void avrcpHandleL2capConnectIndReject(Task task,
                   const CL_L2CAP_CONNECT_IND_T *ind)
{
    AVRCP_INFO(("avrcpHandleL2capConnectIndReject: rejecting\n"));

    /* Send a connect response rejecting the connection. */
    ConnectionL2capConnectResponse(task, 0,
                                   ind->psm, ind->connection_id,
                                   ind->identifier, 0, NULL);
}


/****************************************************************************
*NAME    
*    avrcpHandleAbvpConnectCfm
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_CONNECT_CFM message 
*    on a browsing channel.
******************************************************************************/
void avrcpHandleAvbpConnectCfm(AVBP *avbp,
                         const CL_L2CAP_CONNECT_CFM_T *cfm)
{
    AVRCP *avrcp = (AVRCP*)avbp->avrcp_task;
    avrcp_status_code status = avrcp_fail;

    if(cfm->status == l2cap_connect_pending) return;

    if(!isAvbpConnecting(avbp))
    {
        /* Wrong State */
        AVRCP_INFO(("avrcpHandleAbvpConnectCfm in wrong state\n"));
        return;
    }

    if ((cfm->status == l2cap_connect_success) &&
        isAvrcpConnected(avrcp))
    {
        status = avrcp_success;
        
        /* Break any Source connections */
        StreamDisconnect(StreamSourceFromSink(cfm->sink), 0);

        /* Update the profile state to indicate we're connected */
        avrcpSetAvbpState(avbp, avbpConnected);

        avbp->avbp_sink = cfm->sink;
        avbp->avbp_mtu = cfm->mtu_remote;

        /* Check for Data */
        avbpHandleReceivedData(avbp);
    }
    else
    {
        if (cfm->status ==  l2cap_connect_success)
        {
            /* Disconnect the Browsing Channel */
             avrcpHandleInternalBrowseDisconnectReq(avbp, 0);
        }
        else if (cfm->status == l2cap_connect_timeout)
        {
              status = avrcp_timeout;
        }

        /* Reset the Browsing Task */
        avrcpResetAvbpValues(avbp);
    }

    avrcpSendBrowseConnectCfmToApp( avrcp, status);

}

/****************************************************************************
*NAME    
*    avrcpHandleL2capDisconnectInd
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
*    This message indicates that an L2CAP connection has been disconnected.
*******************************************************************************/
void avrcpHandleL2capDisconnectInd(AVRCP            *avrcp, 
                    const CL_L2CAP_DISCONNECT_IND_T *ind)
{    
    AVRCP_INFO(("avrcpHandleL2capDisconnectInd: status %u\n", ind->status));
    
    /* Disconnect the Browsing channel first */
    avrcpHandleInternalBrowseDisconnectReq((AVBP*)avrcp->avbp_task, 0);

    /* Cleaning up the AVRCP data for the active connection during link loss,
         before setting it to ready state for the incoming reconnection establishment */
    if(convertL2capDisconnectStatus(ind->status) == avrcp_link_loss)
    {
        avrcpCheckForUnreleasedRegionSource(avrcp);
    }

    /* Set to Ready */
    avrcp->bitfields.state = avrcpReady;

    /* Send message to the application */
    avrcpSendCommonCfmMessageToApp(AVRCP_DISCONNECT_IND,
                                   convertL2capDisconnectStatus(ind->status),
                                   ind->sink, avrcp);

    MessageSendConditionally(&avrcp->task,
                             AVRCP_INTERNAL_TASK_DELETE_REQ, 0,
                             &avrcp->sdp_search_mode);

    /* Dispose of data in the Source */
    StreamConnectDispose(StreamSourceFromSink(ind->sink));

    /* Send Disconnect Response */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);    
}

/****************************************************************************
*NAME    
*    avrcpHandleL2capDisconnectCfm
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_DISCONNECT_CFM message.
*******************************************************************************/
void avrcpHandleL2capDisconnectCfm(AVRCP            *avrcp,
                  const CL_L2CAP_DISCONNECT_CFM_T   *cfm)
{    
    AVRCP_INFO(("avrcpHandleL2capDisconnectCfm: deleting task\n"));

    /* Send message to the application */
    avrcpSendCommonCfmMessageToApp(AVRCP_DISCONNECT_IND, 
                                   convertL2capDisconnectStatus(cfm->status),
                                   cfm->sink, avrcp);

    /* Set to Ready */
    avrcp->bitfields.state = avrcpReady;

    MessageSendConditionally(&avrcp->task,
                             AVRCP_INTERNAL_TASK_DELETE_REQ, 0,
                             &avrcp->sdp_search_mode);
    

    if (SinkIsValid(cfm->sink))
    {
        /* Dispose of data in the Source */
        StreamConnectDispose(StreamSourceFromSink(cfm->sink));
    }
}



/****************************************************************************
*NAME    
*    avrcpHandleBrowseDisconnectInd
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_DISCONNECT_IND message.
*    This message indicates that an L2CAP connection has been disconnected.
***************************************************************************/
void avrcpHandleBrowseDisconnectInd(AVBP            *avbp, 
                    const CL_L2CAP_DISCONNECT_IND_T *ind)
{
   /* Send message to the application */
    avrcpSendCommonCfmMessageToApp(AVRCP_BROWSE_DISCONNECT_IND, 
                                   convertL2capDisconnectStatus(ind->status), 
                                   ind->sink, (AVRCP*)avbp->avrcp_task);

    avrcpResetAvbpValues(avbp);

    /* Send Disconnect Response */
    ConnectionL2capDisconnectResponse(ind->identifier, ind->sink);
    
    /* Dispose of data in the Source */
    StreamConnectDispose(StreamSourceFromSink(ind->sink));
}


/****************************************************************************
*NAME    
*    avrcpHandleBrowseDisconnectCfm
*
*DESCRIPTION
*    This function is called on receipt of a CL_L2CAP_DISCONNECT_CFM message.
***************************************************************************/
void avrcpHandleBrowseDisconnectCfm(AVBP            *avbp, 
                    const CL_L2CAP_DISCONNECT_CFM_T *cfm)
{
   /* Send message to the application */
    avrcpSendCommonCfmMessageToApp(AVRCP_BROWSE_DISCONNECT_IND, 
                                   convertL2capDisconnectStatus(cfm->status), 
                                   cfm->sink, (AVRCP*)avbp->avrcp_task);

    avrcpResetAvbpValues(avbp);
    
    /* Dispose of data in the Source */
    StreamConnectDispose(StreamSourceFromSink(cfm->sink));
}


