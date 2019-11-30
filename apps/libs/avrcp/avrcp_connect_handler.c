/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_connect_handler.c
    
DESCRIPTION
    This file defines all internal functions to handle L2CAP connections 
    for Signaling and Browsing channel.
*/
#include <panic.h>
#include <stdlib.h>
#include <connection.h>
#include "avrcp_connect_handler.h"

const uint16 retx_conftab[] = 
    {
        L2CAP_AUTOPT_SEPARATOR,

        /* F&EC Mode-  Retransmission mode, no fallback mode */
        L2CAP_AUTOPT_FLOW_MODE, 
        BKV_16_FLOW_MODE(FLOW_MODE_ENHANCED_RETRANS,0),

         /* Local MTU - Exact value */
        L2CAP_AUTOPT_MTU_IN, AVRCP_MTU_DEFAULT,

        /* Remote MTU - Minimum MTU Allowed */
        L2CAP_AUTOPT_MTU_OUT, AVBP_MIN_REMOTE_MTU,

         /* Local Flush Timeout  - Accept Non-default Timeout*/
        L2CAP_AUTOPT_FLUSH_OUT,
        BKV_UINT32R(DEFAULT_L2CAP_FLUSH_TIMEOUT,0), 

        /*  Local Max Transmit Out */
        L2CAP_AUTOPT_FLOW_MAX_RETX_OUT, 
        BKV_UINT16R(AVBP_DEFAULT_MAX_TRANSMIT, AVBP_MAX_TRANSMIT), 

        /* Ward off reconfiguration attempts */
         L2CAP_AUTOPT_DISABLE_RECONF,
         0xFFFF
    };

const uint16 sig_conftab[] = 
    {
        L2CAP_AUTOPT_SEPARATOR,
        
        /* F&EC Mode-  Retransmission mode, BASIC fallback mode */
        L2CAP_AUTOPT_FLOW_MODE, 
        BKV_16_FLOW_MODE(FLOW_MODE_ENHANCED_RETRANS, MODE_MASK_BASIC),

         /* Local MTU - Exact value */
        L2CAP_AUTOPT_MTU_IN, AVRCP_MTU_DEFAULT,

        /* Remote MTU - Minimum MTU Allowed */
        L2CAP_AUTOPT_MTU_OUT, AVRCP_MTU_MINIMUM,

         /* Local Flush Timeout  - Accept Non-default Timeout*/
        L2CAP_AUTOPT_FLUSH_OUT,
        BKV_UINT32R(DEFAULT_L2CAP_FLUSH_TIMEOUT,0), 

        /* Ward off reconfiguration attempts */
         L2CAP_AUTOPT_DISABLE_RECONF,
         0xFFFF
    };


/****************************************************************************
NAME    
    avrcpSendConnectCfmToApp

DESCRIPTION
    This function sends Connect Confirmation message to the application 
*/
void avrcpSendConnectCfmToApp( AVRCP *avrcp,
                               avrcp_status_code status,
                               const bdaddr* addr,
                               Sink sink )
{
    if (status != avrcp_success)
    {
        /* Send a Request to delete the Task */
        MessageSendConditionally(&avrcp->task,
                                 AVRCP_INTERNAL_TASK_DELETE_REQ, 0,
                                 &avrcp->sdp_search_mode);
    }

    if (avrcp->clientTask)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_CONNECT_CFM);
        message->bd_addr = *addr;
        message->status = status;

        if (status == avrcp_success)
        {
            message->avrcp = avrcp,
            message->sink = sink;
        }
        else
        {
            message->avrcp = NULL;
            message->sink = 0;

        }

        MessageSend( avrcp->clientTask, AVRCP_CONNECT_CFM, message );
    }

}

/***************************************************************************
NAME    
    avrcpSendBrowseConnectCfmToApp

DESCRIPTION
   Send AVRCP_BROWSE_CONNECT_CFM Message to the application
*/
void avrcpSendBrowseConnectCfmToApp(AVRCP               *avrcp,
                                    avrcp_status_code   status)
{
    MAKE_AVRCP_MESSAGE(AVRCP_BROWSE_CONNECT_CFM);
    message->status = status;
    message->avrcp = avrcp;
    message->max_browse_data = 0;

    if(status == avrcp_success)
    {
        message->max_browse_data = 
                ((AVBP*)avrcp->avbp_task)->avbp_mtu - AVBP_MAX_FIXED_PDU_SIZE;
    }
    else
    {
       avrcpResetAvbpValues((AVBP*)avrcp->avbp_task);
    }
        
    MessageSend(avrcp->clientTask,AVRCP_BROWSE_CONNECT_CFM, message);
}



/****************************************************************************
NAME    
    avrcpHandleInternalConnectReq

DESCRIPTION
    This function handles an internally generated connect request message.
*/
void avrcpHandleInternalConnectReq(AVRCP *avrcp,
                                   const AVRCP_INTERNAL_CONNECT_REQ_T *req)
{
    /* Set the state to connecting */
    avrcpSetState(avrcp, avrcpConnecting);

    /* Initiate the L2CAP connection */
    ConnectionL2capConnectRequest(&avrcp->task, 
                                  &req->bd_addr, 
                                  AVCTP_PSM, 
                                  AVCTP_PSM, 
                                  CONFTAB_LEN(sig_conftab),
                                  sig_conftab);

    if (!isAvrcpSdpDone(avrcp))
    {
        /* Initiate an SDP query  */
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SDP_SEARCH_REQ);
        message->bd_addr = req->bd_addr;
        MessageSendConditionally(&avrcp->task,
                                 AVRCP_INTERNAL_SDP_SEARCH_REQ,
                                 message,
                                 &avrcp->sdp_search_mode);
    }
}

/****************************************************************************
NAME    
    avrcpHandleInternalBrowseConnectReq

DESCRIPTION
    This function handles an internally generated connect request message
    for Browsing Channel.
*/
void avrcpHandleInternalBrowseConnectReq(AVBP                   *avbp,
                             const AVRCP_INTERNAL_CONNECT_REQ_T *req)
{
    AVRCP *avrcp = (AVRCP*)avbp->avrcp_task;

    if(!isAvrcpConnected(avrcp))
    {
       /* Control Channel Connection failed */
       avrcpSendBrowseConnectCfmToApp( avrcp,
                                      avrcp_device_not_connected);
        return;
    }

    if(isAvrcpRemoteBrowseSupport(avrcp))
    {
        /* AVRCP Control channel will be established to reach here */
        avrcpSetAvbpState(avbp, avbpConnecting);

        ConnectionL2capConnectRequest(avrcp->avbp_task,
                                      &req->bd_addr, 
                                      AVCTP_BROWSING_PSM, 
                                      AVCTP_BROWSING_PSM, 
                                      CONFTAB_LEN(retx_conftab),
                                      retx_conftab);
     }
     else
     {
         /* Browsing Channel Connection failed */
          avrcpSendBrowseConnectCfmToApp( avrcp,
                                        avrcp_remote_browsing_not_supported);
     }
}
/****************************************************************************
NAME    
    avrcpHandleInternalConnectRes

DESCRIPTION
    This function handles an internally generated connect response message.
*/
void avrcpHandleInternalL2capConnectRes(AVRCP                        *avrcp, 
                                    const AVRCP_INTERNAL_CONNECT_RES_T *res)
{
    /* Send the response to the connection lib */
    ConnectionL2capConnectResponse(&avrcp->task, res->accept,
                                    AVCTP_PSM, res->connection_id, (uint8)res->signal_id,
                                    CONFTAB_LEN(sig_conftab), sig_conftab);
}

/****************************************************************************
*NAME    
*    avrcpHandleInternalBrowseConnectRes
*
*DESCRIPTION
*    This function handles an internally generated connect response message for 
*    browsing channel.
*****************************************************************************/
void avrcpHandleInternalBrowseConnectRes(AVBP                        *avbp, 
                                  const AVRCP_INTERNAL_CONNECT_RES_T *res)
{
    AVRCP *avrcp= (AVRCP*)avbp->avrcp_task;
    bool accept =  isAvrcpConnected(avrcp)?res->accept: FALSE;
    
    ConnectionL2capConnectResponse(avrcp->avbp_task,
                                accept,
                                AVCTP_BROWSING_PSM, 
                                res->connection_id, 
                                (uint8)res->signal_id,
                                CONFTAB_LEN(retx_conftab),
                                retx_conftab);
     if(accept)
     {
        avrcpSetAvbpState(((AVBP*)(avrcp->avbp_task)), avbpConnecting);
     }
     else
     {
        avrcpResetAvbpValues((AVBP*)avrcp->avbp_task);  
        avrcpSendBrowseConnectCfmToApp( avrcp,  avrcp_bad_state);
     }
}

/****************************************************************************
NAME    
    avrcpHandleInternalBrowseDisconnectReq

DESCRIPTION
    This function handles an internally generated Browse disconnect 
    request message.
*/
void avrcpHandleInternalBrowseDisconnectReq(AVBP *avbp, bool app)
{
    if(isAvbpCheckConnected(avbp))
    {
        avbp->bitfields.avbp_state = avbpConnecting;
        ConnectionL2capDisconnectRequest(&avbp->task, avbp->avbp_sink);
    }
    else if(app)
    {
        avrcpSendCommonCfmMessageToApp( AVRCP_BROWSE_DISCONNECT_IND, 
                                    avrcp_browsing_channel_not_connected, 0, 
                                    (AVRCP*)avbp->avrcp_task);
    }
}


/****************************************************************************
NAME    
    avrcpHandleInternalDisconnectReq

DESCRIPTION
    This function handles an internally generated disconnect request message.
*/
void avrcpHandleInternalDisconnectReq(AVRCP *avrcp)
{
    /* Disconnect Browsing channel if present */
    avrcpHandleInternalBrowseDisconnectReq((AVBP*)avrcp->avbp_task, 0);

    avrcp->bitfields.state = avrcpDisconnecting;
    ConnectionL2capDisconnectRequest(&avrcp->task, avrcp->sink);
}
