/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_notification.c
    
DESCRIPTION
    This file defines all APIs for the Message Notification Service (MNS) 
    Initialization and Connection management. The library allows only one 
    active application to wait for incoming MNS connections from the remote 
    device. It does not block multiple MNS connect request from the same
    MSE device. It is the applications responsibility to accept only 
    1 MNS connection per device for specification compliance and limit the
    maximum number of MSE remote devices for optimal resource usage.
   
*/

#include <panic.h>
#include <obex.h>
#include "mapc.h"
#include "mapc_extern.h"

/************************************************************************
 *NAME
 *  MapcMnsStart
 *
 *DESCRIPTION
 *  API to start the Message Notification Service
 *
 *PARAMETERS
 *  Refer mapc.h for details.
 ***********************************************************************/
void MapcMnsStart( Task theAppTask, bool popSdpRec, uint8 mnsChannel )
{
    MAPC_ASSERT(theAppTask);

    if(!mapcRegisterMnsApp( theAppTask, popSdpRec, mnsChannel ))
    {
        /* MNS has already started */
        MAKE_MAPC_MESSAGE( MAPC_MNS_START_CFM );
        message->status = mapc_mns_started;
        MessageSend( theAppTask, MAPC_MNS_START_CFM, message );  
    }
}

/************************************************************************
 *NAME
 *  MapcMnsShutdown
 *
 *DESCRIPTION
 *  API to shutdown the Message Notification Service
 *
 *PARAMETERS
 *  Refer mapc.h for details.
 ***********************************************************************/
void MapcMnsShutdown( uint32 recordHandle, uint8 mnsChannel )
{
    MAPC_ASSERT(recordHandle);
    MAPC_ASSERT(mnsChannel);
    MAPC_ASSERT(mapcGetRegAppTask())

    mapcUnregisterMnsApp( recordHandle, mnsChannel );
}


/**************************************************************************
 *NAME
 *  MapcMnsDisconnectResponse
 *
 *DESCRIPTION
 *  The API to Delete the Session. The application must call on receiving 
 *  MAPC_MNS_DISCONNECT_IND message
 *
 *PARAMETERS
 *  Refer mapc.h for details 
 **************************************************************************/
void MapcMnsDisconnectResponse( Mns session )
{
    MAPC_ASSERT( session );
    mapcPostTaskDelete( session );
}

/**************************************************************************
 *NAME
 *  MapcMnsConnectResponse
 *
 *DESCRIPTION
 *  The API to accept or reject the incoming MNS connect request.
 *
 *PARAMETERS
 *  Refer mapc.h for details 
 **************************************************************************/
void MapcMnsConnectResponse( Task theAppTask, 
                             const bdaddr* addr,
                             uint8  mnsChannel,
                             bool   accept,
                             Sink   connectID )
{
    MAPC_ASSERT( theAppTask );
    MAPC_ASSERT( addr );

    mapcPostConnectResp( theAppTask, addr, mnsChannel, accept, connectID );
}

/************************************************************************
 *NAME
 *  MapcMnsSendEventResponse
 *
 *DESCRIPTION
 *  API to  acknowledge MAPC_MNS_SEND_EVENT_IND
 *
 *PARAMETERS
 *  Refer mapc.h for details.
 ***********************************************************************/
void MapcMnsSendEventResponse( Mns mnsSession, MapcResponse response )
{
    ObexResponse obexResponse;

    switch( response )
    {
        case mapc_success:
            obexResponse = obex_remote_success;
            break;

        case mapc_pending:
            obexResponse = obex_continue;
            break;

        case mapc_invalid_state:
            obexResponse = obex_srv_unavailable;
            break;

        case mapc_object_not_found:
            obexResponse = obex_not_found;
            break;

        case mapc_object_protected:
            obexResponse = obex_unauthorized; 
            break;

        case mapc_command_rejected:
            obexResponse = obex_forbidden;
            break;

        case mapc_invalid_parameter:
            obexResponse =obex_precon_fail; 
            break;
        
        default:
            obexResponse = obex_unknown_error; 
            break;
            
    }
   
    mapcPostMnsSendEventResp( mnsSession, obexResponse );  
}

/**************************************************************************
 *NAME
 * MapcMasSetNotificationRequest
 *
 *DESCRIPTION
 *  The API request to register or unregister message notifications.
 *
 *PARAMETERS
 *  Refer mapc.h for details 
 **************************************************************************/
void MapcMasSetNotificationRequest( Mas masSession, bool regStatus )
{
    MAPC_ASSERT( masSession );
    mapcPostRegisterNotification( masSession, regStatus );

}

