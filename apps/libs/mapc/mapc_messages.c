/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_messages.c
    
DESCRIPTION
    This file implements all outgoing messages from the library and 
    all incoming messages to the library.
*/

#include <obex.h>
#include <panic.h>
#include <string.h>
#include "mapc_extern.h"
#include "mapc_private.h"


/***************************************************************************
 * NAME
 *  mapcMnsStartCfm
 * 
 * DESCRIPTION
 *  Despatch a MAP_MNS_START_CFM Message to the application. 
 *
 * PARAMETERS 
 *  status      - Status of mapMnsStart() 
 *  channel     - RFCOMM Channel for MNS service.
 *  handle      - SDP Record handle
 **************************************************************************/
void mapcMnsStartCfm( MapcStatus status, uint8 channel, uint32 handle ) 
{
    MAKE_MAPC_MESSAGE( MAPC_MNS_START_CFM );

    /* Dispatch the CFM message to the application */
    message->status = status;
    message->mnsChannel = channel;
    message->sdpHandle = handle;
    MessageSend( mapcGetRegAppTask(), MAPC_MNS_START_CFM, message);  

    /* Remove the registered app on failure */
    if( status == mapc_failure ) mapcUnregMnsApp();

}

/***************************************************************************
 * NAME
 *  mapcMnsShutdownCfm
 * 
 * DESCRIPTION
 *  Despatch a MAP_MNS_SHUTDOWN_CFM Message to the application. 
 *
 * PARAMETERS 
 *  status      - Status of the Notificaton shutdown request
 **************************************************************************/
void mapcMnsShutdownCfm( MapcStatus status )
{
    MAKE_MAPC_MESSAGE( MAPC_MNS_SHUTDOWN_CFM );
    message->status = status;
    MessageSend( mapcGetRegAppTask(), MAPC_MNS_SHUTDOWN_CFM, message );

    if( status == mapc_success )  mapcUnregMnsApp();
}

/***************************************************************************
 * NAME
 *  mapcMnsConnectInd 
 *
 * DESCRIPTION
 *  Despatch a  Connect Indication message to the application
 ************************************************************************/
void mapcMnsConnectInd( OBEX_CONNECT_IND_T* msg )
{
    MAKE_MAPC_MESSAGE( MAPC_MNS_CONNECT_IND );
    message->addr = msg->addr;
    message->mnsChannel = msg->channel;
    message->connectID  = msg->sink;
    MessageSend( mapcGetRegAppTask(), MAPC_MNS_CONNECT_IND, message );
}

/***************************************************************************
 * NAME
 *   mapcConnectCfm
 *
 * DESCRIPTION
 *  Despatch a Connect CFM message to the application 
 *
 * PARAMETES
 *  mapSession      - MAP Session
 *  status          - Status of the connection 
 *  addr            - Remote BDADDR
 *  channel         - Server Channel   
 *  theAppTask      - The application task
 *************************************************************************/ 
void mapcConnectCfm( MAPC*        mapSession,  
                     MapcStatus   status,
                    const bdaddr  *addr,
                     uint8        channel)
{   
    MapcMessageId id;
    MAKE_MAPC_MESSAGE(MAPC_CONNECT_CFM);
   
    if( status == mapc_failure )
    {
        message->session = 0; 
    }  
    else
    {
        message->session = mapSession;
    }
        
    message->status = status;
    message->addr =   *addr;
    message->channel = channel;
    
    id = IsMapMasClient( mapSession->role )? MAPC_MAS_CONNECT_CFM : 
                                             MAPC_MNS_CONNECT_CFM; 
           
    MessageSend( mapSession->appTask , id, message ); 
}
 
/************************************************************************
 * NAME
 *  mapcDisconnectInd
 *
 * DESCRIPTION 
 *  Send a Disconnect Indication to the application
 * 
 * PARAMETES
 *  mapSession      - MAP Session
 ************************************************************************/
void mapcDisconnectInd( MAPC* mapcSession)
{
   MapcMessageId id;
   MAKE_MAPC_MESSAGE( MAPC_DISCONNECT_IND);

   id =  IsMapMasClient( mapcSession->role )? MAPC_MAS_DISCONNECT_IND : 
                                              MAPC_MNS_DISCONNECT_IND;

   message->session = mapcSession;
   MessageSend( mapcSession->appTask , id, message );
} 

/***************************************************************************
 * NAME
 *   mapcStatusCfm
 *
 * DESCRIPTION
 *  Despatch a Status only notification to the application
 *
 * PARAMETES
 *  session         - MAP Session
 *  status          - Status 
 **************************************************************************/
void mapcStatusCfm( MAPC* session,  MapcMessageId id, MapcStatus status) 
{
    MAKE_MAPC_MESSAGE( MAPC_INT_STATUS_CFM );
    message->session = session;
    message->status = status;
    MessageSend( session->appTask, id, message ); 
} 


/***************************************************************************
 * NAME
 *  mapcMnsSendEventInd
 *
 * DESCRIPTION
 *  Despatch MAPC_MNS_SEND_EVENT_IND message to the application.
 *
 * PARAMETERS
 *  session - MAP Session
 *  final   - TRUE for final Indication message
 *  len     - Length of the data in the source
 *  src     - Source containing the message 
 **************************************************************************/
void mapcMnsSendEventInd( MAPC* session, bool final, uint16 len, Source src )
{
    MAKE_MAPC_MESSAGE( MAPC_MNS_SEND_EVENT_IND );
    message->mnsSession = session;
    message->masInstanceId = session->lastAppParam;
    message->moreData = !final;
    message->eventReport = src;
    message->sourceLen = len;
    MessageSend( session->appTask, MAPC_MNS_SEND_EVENT_IND, message );
}


/***************************************************************************
 * NAME
 *  mapcMasGetMessageCfm
 *
 * DESCRIPTION
 *  Despatch a GetMessage Confirmation Message
 *
 * PARAMETERS
 *  session - MAP Session 
 *  status  - Status of the function
 *  len      - Length of the current object
 *  src      - Source containing the object.
 **************************************************************************/
void mapcMasGetMessageCfm( MAPC* session, 
                           MapcStatus status , 
                           uint16 len, 
                           Source src )
{
    MAKE_MAPC_MESSAGE( MAPC_MAS_GET_MESSAGE_CFM );

    message->masSession = session;
    message->status = status;
    message->sourceLen = len;
    message->bMessage = src;
    MessageSend( session->appTask,  MAPC_MAS_GET_MESSAGE_CFM, message ); 
}

/***************************************************************************
 * NAME
 *  mapcMasPutMessageCfm
 *
 * DESCRIPTION
 *  Despatch a PutMessage Confirmation Message
 *
 * PARAMETERS
 *  session - MAP Session 
 *  status  - Status of the function
 *  len     - Length of the current object. This must not exceed 
 *            MAPC_HANDLE_LEN
 *  data    - The name header if not NULL
 **************************************************************************/
void mapcMasPutMessageCfm( MAPC* session, 
                           MapcStatus status , 
                           uint16 len, 
                           const uint8* nameHdr )
{
    uint16 handleLen = (len+1)/2;
    uint8 i,j ;
    MAKE_MAPC_MESSAGE_WITH_LEN( MAPC_MAS_PUT_MESSAGE_CFM, handleLen );

    message->masSession = session;
    message->status = status;
    message->handleLen = handleLen;

    /* Convert back from unicode */
    for(i=0, j= 1; i < handleLen ; i++, j+=2) 
    {
        message->handle[i] = nameHdr[j];        
    }

    MessageSend( session->appTask,  MAPC_MAS_PUT_MESSAGE_CFM, message );
}

/***************************************************************************
 * NAME
 *  mapcMasListingCfm
 *
 * DESCRIPTION
 *  Despatch a MAS Listing confirmation with data and Size.
 *
 * PARAMETERS
 *  session - MAP Session 
 *  id      - Message ID 
 *  status  - Status of the function
 *  remainLen- Remaining size of the object
 *  len      - Length of the current object
 *  src      - Source containing the object.
 **************************************************************************/
void mapcMasListingCfm( MAPC* session, 
                         MapcMessageId id,
                         MapcStatus status , 
                         uint16 remainLen,
                         uint16 len, 
                         Source src )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_FUNCTION_CFM );

    message->session = session;
    message->status = status;
    message->remainLen = remainLen;
    message->srcLen = len;
    message->src = src;
    MessageSend( session->appTask, id, message ); 

}



/************************************************************************
 * NAME
 *  mapcPostConnectResp
 *
 * DESCRIPTION 
 *  Post a Connect response to the library
 * 
 * PARAMETERS
 * theAppTask   - The MNS application Task to send MAPC_MNS_CONNECT_CFM 
 * bdaddr       - The MSE BDADDR
 * channel      - The MNS Session server channel 
 * accept       - TRUE to accept the connection
 * connectID    - Connection ID
 ************************************************************************/
void mapcPostConnectResp(  Task theAppTask, 
                           const bdaddr* addr,
                           uint8  channel,
                           bool   accept,
                           Sink   connectID )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_CONNECT_RESP );

    message->theAppTask = theAppTask;
    message->addr = *addr;
    message->channel = channel;
    message->accept = accept;
    message->sink = connectID;
    
    MessageSend( mapcGetProfileTask(), MAPC_INT_CONNECT_RESP, message );
}


/************************************************************************
 * NAME
 *  mapcPostMnsSendEventResp
 *
 * DESCRIPTION 
 *  Post a Response for SendEvent Indication Message 
 *
 * PARAMETES
 *   MnsSession     - The session 
 *   ObexResponse   - Obex Response.
 ************************************************************************/
void mapcPostMnsSendEventResp( MAPC* session, uint16 response)
{
    MAKE_MAPC_MESSAGE( MAPC_INT_SEND_EVENT_RESP );

    message->session = session;
    message->respCode = response;

    MessageSend( mapcGetProfileTask(), MAPC_INT_SEND_EVENT_RESP, message );
}






/************************************************************************
 * NAME
 *  mapcPostConnectReq
 *
 * DESCRIPTION 
 *  Post a Connect Request to the library
 * 
 * PARAMETERS
 * theAppTask   - The MAS application Task to send MAPC_MAS_CONNECT_CFM 
 * bdaddr       - The MSE BDADDR
 * channel      - The MAS Session server channel 
 ************************************************************************/
void mapcPostConnectReq( Task theAppTask, const bdaddr* addr, uint8 channel )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_CONNECT_REQ );

    message->theAppTask = theAppTask;
    message->addr = *addr;
    message->channel = channel;
    
    MessageSend( mapcGetProfileTask(), MAPC_INT_CONNECT_REQ, message );
}


/************************************************************************
 * NAME
 *  mapcPostTaskDelete
 *
 * DESCRIPTION 
 *  Post a request to delete the Task
 *
 * PARAMETERS
 *  session  - The session to be deleted .
 *************************************************************************/
void mapcPostTaskDelete( MAPC* session )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_TASK_DELETE );

    message->session = session ;

    MessageSend( mapcGetProfileTask(), MAPC_INT_TASK_DELETE, message );
}

/************************************************************************
* NAME
*   mapcPostRegisterNotification
*
* DESCRIPTION
*   Post a request to Message Notifcation Registration
* 
* PARAMETERS
*   session - The session
*   regStatus - TRUE for notification ON
**************************************************************************/
void mapcPostRegisterNotification( MAPC* session, bool regStatus )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_SET_NOTIFICATION );

    message->session = session;
    message->status =  (uint8) regStatus;

    MessageSend( mapcGetProfileTask() , MAPC_INT_SET_NOTIFICATION, message );
}

/************************************************************************
* NAME
*   mapcPostMasSetFolder
*
* DESCRIPTION
*   Post a request to Set Folder
* 
* PARAMETERS
*   session - The session to be deleted.
*   up      - TRUE to navigate backwards
*   name    - Name of the folder
*   len     - Length of the name string.
**************************************************************************/
void mapcPostMasSetFolder( MAPC* session, 
                           bool up, 
                           const uint8* name, 
                           uint16 nameLen )
{
    MAKE_MAPC_MESSAGE_WITH_LEN( MAPC_INT_SET_FOLDER, nameLen );

    message->session = session;
    message->flag = up;
    memmove( message->name, name, nameLen );
    message->nameLen = nameLen; 
   
    MessageSend( mapcGetProfileTask(), MAPC_INT_SET_FOLDER, message );
}

/************************************************************************
* NAME
*   mapcPostMasGetFolderListing
*
* DESCRIPTION
*   Post a request to get the Folder listing
* 
* PARAMETERS
*   session - The session to be deleted.
*   maxListCount - Maximum number of folders to be listed
*   listStartOffset - Offset of the first folder in the list
**************************************************************************/
void mapcPostMasGetFolderListing( MAPC* session, 
                                  uint16 maxListCount, 
                                  uint16 listStartOffset)
{
    MAKE_MAPC_MESSAGE( MAPC_INT_GET_FOLDER );

    message->session = session;
    message->maxListCount = maxListCount;
    message->startOffset = listStartOffset;

    MessageSend( mapcGetProfileTask(), MAPC_INT_GET_FOLDER, message );
}

/************************************************************************
* NAME
*   mapcPostMasGetMessagesListing
*
* DESCRIPTION
*   Post a request to get the Messages Listing
* 
* PARAMETERS
*   session - The session to be deleted.
*   name    - The name of the folder
*   nameLen - The name length
*   maxListCount - Maximum number of folders to be listed
*   listStartOffset - Offset of the first folder in the list
*   unread   - Retrieve only the unread messages
**************************************************************************/
void mapcPostMasGetMessagesListing( Mas session, 
                                   const uint8* name,
                                   uint16 nameLen,
                                   uint16 maxListCount, 
                                   uint16 listStartOffset,
                                   MapcMessageFilter filter)
{
    MAKE_MAPC_MESSAGE_WITH_LEN( MAPC_INT_GET_MSG_LIST, nameLen );

    message->session = session;
    message->nameLen = nameLen;
    message->maxListCount = maxListCount;
    message->startOffset = listStartOffset;
    message->filter = filter;
    memmove( message->name, name, nameLen );

    MessageSend( mapcGetProfileTask(), MAPC_INT_GET_MSG_LIST, message );
}

/************************************************************************
* NAME
*   mapcPostMasGetMessage
*
* DESCRIPTION
*   Post a request to get Message
* 
* PARAMETERS
*   session - The session to be deleted.
*   handle  - 8 byte handle
*   native  - TRUE for Native and FALSE for UPF-8
**************************************************************************/
void mapcPostMasGetMessage( Mas session, const uint8* handle, bool native )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_GET_MSG );

    message->session = session;
    memmove( message->handle, handle, MAPC_HANDLE_LEN );
    message->charSet = native ? 0 : 1;

    MessageSend( mapcGetProfileTask(), MAPC_INT_GET_MSG, message ); 
}



/************************************************************************
* NAME
*   mapcPostMasContinue
*
* DESCRIPTION
*   Post a request to continue or terminate a multi packet operation
* 
* PARAMETERS
*   session - The session 
*   proceed - TRUE to continue and FALSE to terminate.
**************************************************************************/
void mapcPostMasContinue( Mas masSession, bool proceed )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_GET_CONTINUE );
    
    message->session = masSession;
    message->proceed = proceed;

    MessageSend(  mapcGetProfileTask(), MAPC_INT_GET_CONTINUE, message );
}

/************************************************************************
* NAME
*   mapcPostMasSetMessageStatus
*
* DESCRIPTION
*   Post a request to Set Message Status
* 
* PARAMETERS
*   session - The session
*   handle  - Message handle
*   indicator - The Status Indicator
*   value     - Status Indicator Value
**************************************************************************/
void mapcPostMasSetMessageStatus( MAPC* session,
                                  const uint8* handle, 
                                  uint8 indicator, 
                                  uint8 value )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_SET_MSG_STATUS );

    message->session = session;
    message->indicator = indicator;
    message->value = value;
    memmove( message->handle, handle, MAPC_HANDLE_LEN );

    MessageSend( mapcGetProfileTask(), MAPC_INT_SET_MSG_STATUS, message );
}

/************************************************************************
* NAME
*   mapcPostMasUpdateInbox
*
* DESCRIPTION
*   Post a request to update Inbox
* 
* PARAMETERS
*   session - The session
**************************************************************************/
void mapcPostMasUpdateInbox( MAPC* session ) 
{
    MAKE_MAPC_MESSAGE( MAPC_INT_UPDATE_INBOX );

    message->session = session;

    MessageSend( mapcGetProfileTask(), MAPC_INT_UPDATE_INBOX, message );
}

/************************************************************************
* NAME
*   mapcPostMasPutMessage
*
* DESCRIPTION
*   Post a request to push the message to the MSE
* 
* PARAMETERS
*   session - The session to be deleted.
*   name    - The name of the folder
*   nameLen - The name length
*   native  - TRUE for native, FALSE for UTF-8
*   moreData - TRUE for multi-put request. 
*   data  - Source containing the message data.
**************************************************************************/
void mapcPostMasPutMessage( Mas session, 
                            const uint8* name,
                            uint16 nameLen,
                            bool native,
                            bool moreData,
                            Source data )
{
    MAKE_MAPC_MESSAGE_WITH_LEN( MAPC_INT_PUT_MSG, nameLen );

    message->session = session;
    message->charSet = native? FALSE : TRUE;
    message->final = moreData? FALSE : TRUE; 
    message->data =  data;
    message->nameLen = nameLen;
    memmove( message->name, name, nameLen );
    MessageSend( mapcGetProfileTask(), MAPC_INT_PUT_MSG, message );
}

/************************************************************************
* NAME
*   mapcPostMasPutContinue
*
* DESCRIPTION
*   Post a request to push the message to the MSE continue
* 
* PARAMETERS
*   session - The session to be deleted.
*   moreData - TRUE if there is more data to send.
*   data -     The message data to be sent to the MSE
**************************************************************************/
void mapcPostMasPutContinue( Mas session, 
                             bool moreData,
                             Source data )
{
    MAKE_MAPC_MESSAGE( MAPC_INT_PUT_CONTINUE);

    message->session = session;
    message->final = moreData? FALSE : TRUE; 
    message->data = data;

    MessageSend( mapcGetProfileTask(), MAPC_INT_PUT_CONTINUE, message );
}


