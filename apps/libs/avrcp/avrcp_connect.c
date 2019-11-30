/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avcrp_connect.c
    
DESCRIPTION
    This file defines the APIs for establishing AVRCP Signaling and 
    Browsing channels.     
*/

#include <bdaddr.h>
#include <connection.h>
#include <sink.h>
#include <panic.h>

#include "avrcp_connect_handler.h"
#include "avrcp_sdp_handler.h"
#include "avrcp_l2cap_handler.h"

/****************************************************************************
* NAME    
* avrcpSendInternalConnectResp    
*
* DESCRIPTION
* Queue an internal connect response to process. 
*******************************************************************************/
static void avrcpSendInternalConnectResp(AVRCP  *avrcp, 
                                         uint16 connection_id, 
                                         uint16 signal_id,
                                         bool   accept,
                                         bool   browsing)
{
    Task task = &avrcp->task;
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_RES);
    message->connection_id = connection_id;
    message->signal_id = signal_id;
    message->accept = accept;

    if( browsing )
    {
        if( avrcp->avbp_task )
        {
            task = avrcp->avbp_task ;
        }
        else
        {
            message->accept = FALSE;
        }
    } 
    MessageSend( task, AVRCP_INTERNAL_CONNECT_RES, message);
}

/****************************************************************************
* NAME    
* avrcpSendInternalConnectResp    
*
* DESCRIPTION
* Queue an internal connect request to process. 
*******************************************************************************/
static void avrcpSendInternalConnectReq(AVRCP *avrcp,
                                        const bdaddr *bd_addr,
                                        bool  browsing)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_CONNECT_REQ);
    message->bd_addr = *bd_addr;

    /* Establish the Browsing channel */
    if (browsing)
    {
        MessageSendConditionally(avrcp->avbp_task,
                                 AVRCP_INTERNAL_CONNECT_REQ,
                                 message,
                                 &avrcp->sdp_search_mode);
    }
    else
    {
        /* Control channel Connect */
        MessageSend(&avrcp->task,
                    AVRCP_INTERNAL_CONNECT_REQ,
                    message);
    }
}


/****************************************************************************
*NAME    
*    AvrcpConnectRequest    
*
*DESCRIPTION
*  API function to establish AVRCP Connection.
*    
*PARAMETERS
*   theApptask       - The application Task
*   bd_addr          - Remote Device Bluetooth Address
*
*RETURN
*   TRUE if AVRCP connection is initiated else FALSE.
*   AVRCP_CONNECT_CFM
*******************************************************************************/
bool AvrcpConnectRequest(Task theAppTask, const bdaddr *bd_addr)
{
    AVRCP *avrcp;

#ifdef AVRCP_DEBUG_LIB    
    if (!bd_addr)
    {
        AVRCP_DEBUG(("Null Bluetooth address pointer\n"));
    }
#endif

    /* Pass internal connect request message to exising task or newly created
       task if no task for the BDADDR exists.  The task state machine will
       process the message as appropriate.
    */
    avrcp = avrcpFindTask(bd_addr);
    if (!avrcp)
    {
        /* Create the AVRCP task. */
        avrcp = avrcpCreateTask( theAppTask, avrcpReady, bd_addr);
    }

    avrcpSendInternalConnectReq(avrcp, bd_addr, 0);

    return TRUE;
}

/****************************************************************************
*NAME    
*    AvrcpBrowseConnectRequest    
*
*DESCRIPTION
*  API function to establish AVRCP Control and Browsing channel Connection.
*  Before establishing the control channel connection,it performs an SDP query.
*  if the application wants to skip teh SDP query , call AvrcpConnect() before
*  calling this API.    
*    
*PARAMETERS
*   avrcp            - Task
*   bd_addr          - Remote Device Bluetooth Address
*
*RETURN
*   AVRCP_BROWSE_CONNECT_CFM
******************************************************************************/
void AvrcpBrowseConnectRequest(AVRCP* avrcp, const bdaddr *bd_addr)
{    

    if (isAvrcpConnected(avrcp))
    {
        tp_bdaddr tp_addr;

        /* Check it is connected to the same device */
        SinkGetBdAddr(avrcp->sink, &tp_addr);

        if(!BdaddrIsSame(bd_addr, &tp_addr.taddr.addr))
        {
            avrcpSendBrowseConnectCfmToApp( avrcp,
                                            avrcp_device_not_connected);
            return;
        }
    }
    else
    {
        /* Attempt to establish the Control Channel */
        avrcpSendInternalConnectReq(avrcp, bd_addr, 0);
    }

    /* Establish the Browsing channel */
    if(avrcp->avbp_task)
    {
        avrcpSendInternalConnectReq(avrcp, bd_addr, 1);
    }
    else
    {
        avrcpSendBrowseConnectCfmToApp( avrcp,
                                        avrcp_browsing_fail);
    }
}


/***************************************************************************
*NAME    
*    AvrcpConnectResponse    
*
*DESCRIPTION
*  API function to send Connect Response
*    
*PARAMETERS
*   theAppTask       - The application Task
*   connection_id    - Same ID received in AVRCP_CONNECT_IND
*   signal_id        - The signalling Identifier
*   accept           - TRUE to accept connection and FALSE to reject.
*   bd_addr         - Remote Device Bluetooth Address.
*
*RETURN
***************************************************************************/
void AvrcpConnectResponse(Task  theAppTask,
                          uint16 connection_id, 
                          uint16 signal_id,
                          bool accept,
                          const bdaddr *bd_addr)
{
    AVRCP *avrcp;

    /* Find task for this device */
    avrcp = avrcpFindTask(bd_addr);
#ifdef AVRCP_DEBUG_LIB    
    if (!avrcp)
    {
        AVRCP_DEBUG(("No AVRCP task found\n"));
    }
    else
#endif
    {
        avrcp->clientTask = theAppTask;
        avrcpSendInternalConnectResp(avrcp, connection_id, signal_id, accept, 0);
    }
}

/****************************************************************************
*NAME    
*    AvrcpBrowseConnectResponse    
*
*DESCRIPTION
*  API function to send Connect Response
*    
*PARAMETERS
*   avrcp            - Task
*   connection_id    - Same ID received in AVRCP_BROWSE_CONNECT_IND
*   accept           - TRUE to accept connection and FALSE to reject.
*
*RETURN
*****************************************************************************/
void AvrcpBrowseConnectResponse( AVRCP *avrcp, 
                                 uint16 connection_id, 
                                 uint16 signal_id,
                                 bool accept)
{
    avrcpSendInternalConnectResp(avrcp, connection_id, signal_id, accept, 1);
}


/****************************************************************************
*NAME    
*    AvrcpDisconnectRequest    
*
*DESCRIPTION
*  This function is called to request an AVRCP disconnection.  
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
    AVRCP_DISCONNECT_IND
*****************************************************************************/
void AvrcpDisconnectRequest(AVRCP *avrcp)
{
    MessageSend(&avrcp->task, AVRCP_INTERNAL_DISCONNECT_REQ, 0);
}


/****************************************************************************
*NAME    
*    AvrcpBrowseDisconnectRequest    
*
*DESCRIPTION
*  This function is called to request an AVRCP Browsing Channel disconnection.  
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
    AVRCP_BROWSE_DISCONNECT_IND
*****************************************************************************/
void AvrcpBrowseDisconnectRequest(AVRCP *avrcp)
{
     MessageSend(avrcp->avbp_task, AVRCP_INTERNAL_DISCONNECT_REQ, 0);
}


/****************************************************************************
NAME    
    AvrcpGetSink    

DESCRIPTION
    This function is called to retrieve the connection Sink.  

PARAMETER RETURNED
    The connection sink. This will be 0 if no connection exists.
*****************************************************************************/
Sink AvrcpGetSink(AVRCP *avrcp)
{
    if (!avrcp)
    {
#ifdef AVRCP_DEBUG_LIB
        AVRCP_DEBUG(("AvrcpGetSink NULL AVRCP instance\n"));
#endif
        return (Sink)0;
    }

    return avrcp->sink;
}


/****************************************************************************
*NAME    
*    AvrcpGetSupportedFeatures    
*
*DESCRIPTION
*  This function is used by the CT to retrieve the supported features
*  of the TG. This will include  which category commands are supported, 
*  and if player application settings or group navigation is supported for
*  Metadata extensions.
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
*  AVRCP_GET_SUPPORTED_FEATURES_CFM 
******************************************************************************/
void AvrcpGetSupportedFeatures(AVRCP *avrcp)
{
    if(isAvrcpConnected(avrcp))
    {
        if(isAvrcpSdpDone(avrcp))
        {
            avrcpSendGetSupportedFeaturesCfm(avrcp, avrcp_success, 
                                             avrcp->bitfields.remote_features); 
        }
        else
        {
            MessageSendConditionally(&avrcp->task, AVRCP_INTERNAL_GET_FEATURES,
                                     0, &avrcp->sdp_search_mode);
        }
    }
    else
    {
        avrcpSendGetSupportedFeaturesCfm(avrcp, avrcp_device_not_connected, 0);
    }
}

/****************************************************************************
*NAME    
*    AvrcpGetProfileExtensions    
*
*DESCRIPTION
*   This function is used to retrieve if any profile
*   extensions are available on the peer device(Typically remote TG).
*   This will return profile extension flag value if the remote device 
*   supports  AVRCP Metadata extensions or Browsing channel support.    
*    
*PARAMETERS
*   avrcp            - Task
*
*MESSAGE RETURNED
*  AVRCP_GET_EXTENSIONS_CFM 
******************************************************************************/
void AvrcpGetProfileExtensions(AVRCP *avrcp)
{
    if(isAvrcpConnected(avrcp))
    {
        if(isAvrcpSdpDone(avrcp))
        {
            avrcpSendGetExtensionsCfm(avrcp, avrcp_success, 
                                      avrcp->bitfields.remote_extensions);
        }
        else
        {
            MessageSendConditionally(&avrcp->task, 
                                    AVRCP_INTERNAL_GET_EXTENSIONS,
                                    0, &avrcp->sdp_search_mode);
        }
    }
    else
    {
        avrcpSendGetExtensionsCfm(avrcp, avrcp_device_not_connected, 0);    
    }
}


/****************************************************************************
*NAME    
*    AvrcpSourceProcessed    
*
*DESCRIPTION
*   When  the application has finished with the Source data provided
*   by the library, this API MUST be called, otherwise the library won't
*   process any more data arriving until the application calls any other 
*   request or response API.
*    
*PARAMETERS
*   avrcp            - Task
*
MESSAGE RETURNED
*******************************************************************************/
void AvrcpSourceProcessed(AVRCP *avrcp)
{
    if(avrcp->bitfields.fragment != avrcp_packet_type_single ) return;

    if(avrcp->bitfields.block_received_data == avrcp_app_unknown )
    { 
        avrcpUnblockReceivedData(avrcp);
    }
    else
    {
        avrcpSourceProcessed(avrcp, FALSE);
    }
}
