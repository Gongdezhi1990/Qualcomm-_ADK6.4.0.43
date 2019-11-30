/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_masTask_handler.c
    
DESCRIPTION
   This file implements all incoming and Outgoing MAS client messages.
*/

#include <stream.h>
#include <source.h>
#include <obex.h>
#include <panic.h>
#include <stdlib.h>
#include "mapc_private.h"
#include "mapc_extern.h"



/* Type Header values */
const uint8 notification_type[] = "x-bt/MAP-NotificationRegistration";
const uint8 get_folder_type[] = "x-obex/folder-listing";
const uint8 get_msg_list_type[] = "x-bt/MAP-msg-listing";
const uint8 get_put_msg_type[] = "x-bt/message";
const uint8 update_inbox_type[] ="x-bt/MAP-messageUpdate";
const uint8 set_msg_type[] = "x-bt/messageStatus";

/**************************************************************************
 * NAME:
 *  mapcMasConvertStatus
 *
 * DESCRIPTION
 *  Convert the OBEX status to MAP status
 ************************************************************************/
static MapcStatus mapcMasConvertStatus( ObexStatus status)
{
    if( ( status == obex_success ) || ( status == obex_remote_success )) 
    {
        return mapc_success;
    }

    if ( (status == obex_bad_request ) || ( status == obex_forbidden ) )
    {
        return mapc_command_rejected;
    }

    if( status == obex_continue ) return mapc_pending;
    if( status == obex_not_found ) return mapc_object_not_found;
    if( status == obex_unauthorized ) return mapc_object_protected;

    return mapc_failure;
}

/**************************************************************************
 * NAME:
 *  mapcFrame2ByteAppParams
 *
 * DESCRIPTION
 *  Frame application parameters of 2 Byte length
 ************************************************************************/
static void mapcFrame2ByteAppParams( uint8* ptr, uint8 param, uint16 value ) 
{
    ptr[0] = param;
    ptr[1] = MAPC_2BYTE_SIZE; /* Two byte length */
    ptr[2] = value >> 8;
    ptr[3] = value & 0xFF;
}

/************************************************************************
 * NAME
 *  mapcFrameByteParam
 *
 * DESCRIPTION
 *  Frame One byte application parameter
 *************************************************************************/
static void mapcFrameByteAppParams( uint8* ptr, uint8 param, uint8 value )
{
    ptr[0] = param;
    ptr[1] = MAPC_BYTE_SIZE;
    ptr[2] = value;
}

/************************************************************************
 * NAME
 *  mapcFrame4ByteParam
 *
 * DESCRIPTION
 *  Frame One byte application parameter
 *************************************************************************/
static void mapcFrame4ByteAppParams( uint8* ptr, uint8 param, uint32 value ){
    ptr[0] = param;
    ptr[1] = MAPC_4BYTE_SIZE;
    ptr[2] = value >> 24 ;
    ptr[3] = (value >> 16) & 0xFF;
    ptr[4] = (value >> 8 ) & 0xFF;
    ptr[5] = value & 0xFF;
}


/**************************************************************************
 * NAME:
 *  mapcFrameSeqHdr
 *
 * DESCRIPTION
 *  Frame header to a function request
 ************************************************************************/
static bool mapcFrameSeqHdr( Mas masTask, 
                             const uint8* hdr, 
                             uint16 len,
                             ObexSeqHeaderId id )
{
    Source src = 0;

    if( hdr && len )  src = StreamRegionSource( hdr, len );

    if( ObexObjAddVariableHdr( masTask->session, id , len, src ) )
    {
        SourceEmpty( src );
        return TRUE;
    }
    else
    {
        SourceEmpty( src );
        return FALSE;
    }
}

/**************************************************************************
 * NAME:
 *  mapcFrameListAppParams
 *
 * DESCRIPTION
 *  Frame application parameters for Get Listing, First 8 bytes in the
 *  List data must be free. List data will be freed in this function. 
 ************************************************************************/
static bool mapcFrameListAppParams( Mas masTask, 
                                    uint16 maxListCount,
                                    uint16 startOffset,
                                    uint16 listLen,
                                    uint8* listData )
{
    bool result = FALSE;

   /* Create the application header for 2 headers*/
    mapcFrame2ByteAppParams( &listData[0], 
                             mapc_app_max_list_count,
                             maxListCount );

    mapcFrame2ByteAppParams( &listData[MAPC_2BYTE_APP_HDR_SIZE],
                             mapc_app_start_offset,
                             startOffset ); 

    if( mapcFrameSeqHdr( masTask, listData, 
                         listLen, obex_app_params ) )  result = TRUE;
    free( listData );
    return result;
}

/**************************************************************************
 * NAME:
 *  mapcFrameListMsgParams
 *
 * DESCRIPTION
 *  Frame application parameters for Get Message Listing
 ************************************************************************/
static bool mapcFrameListMsgParams( Mas masTask,
                                    uint16 maxListCount,
                                    uint16 startOffset,
                                    MapcMessageFilter  filter )
{
    uint16 offset =  MAPC_LIST_APP_PARAM_SIZE;
    uint16 listLen = offset; 
    uint8  readStatus;
    uint8  msgType;
    uint8 *listData;

    if( filter & mapc_filter_params ) 
    { 
        listLen +=  MAPC_4BYTE_APP_HDR_SIZE;
    }

    msgType = filter &  MAPC_MSG_TYPE_MASK;
    if( msgType )
    {
        listLen += MAPC_BYTE_APP_HDR_SIZE; 
    }

    readStatus = filter & MAPC_READ_STATUS_MASK;
    if( readStatus )
    {
        listLen += MAPC_BYTE_APP_HDR_SIZE;
        readStatus >>= MAPC_READ_STATUS_SHIFT; 
    }

    listData = PanicUnlessMalloc( listLen );
    /* maxListCount and SatrtOffset will be filled later. fill others */
    /* FilterMsgType */

    if( msgType )
    {
        mapcFrameByteAppParams( &listData[offset], 
                                 mapc_app_filter_message_type,
                                msgType);
        offset +=  MAPC_BYTE_APP_HDR_SIZE;
    }

    if( readStatus )
    {
        mapcFrameByteAppParams( &listData[offset], 
                                mapc_app_filter_read_status,
                                readStatus);
        offset +=  MAPC_BYTE_APP_HDR_SIZE;
    }

    /* ReadStatus */
    if( filter & mapc_filter_params )
    {
        mapcFrame4ByteAppParams( &listData[offset], 
                                mapc_app_param_mask,
                                MAPC_DEF_FILTER_PARAM );
                                
    }

    return mapcFrameListAppParams( masTask, 
                                   maxListCount,
                                   startOffset,
                                   listLen,
                                   listData );
}

/**************************************************************************
 * NAME:
 *  mapcMasParseListResp
 *
 * DESCRIPTION
 *  Parse the Listing response to get the size of the object and body 
 * 
 * PARAMETERS
 *  masTask - The MAS Task
 *  status  - The listing status
 *  sizeId  - The application parameter ID for size.
 *  len     - Length of listing response parameters
 *  src     - The source buffer
 *
 * return the length on the object
 ************************************************************************/
static uint16 mapcMasParseListResp( Mapc masTask,
                                    MapcStatus status,
                                    MapcAppParams sizeId,
                                    uint16 len,
                                    Source src )
{
    uint16 objLen = 0;
    const uint8* appParam;
    ObexSeqHeaderId id = ( status == mapc_pending )? obex_body_hdr: 
                                                     obex_ebody_hdr;

    /* Get the Folder/Message Listing size if it is the first packet.
       If it is not the first, it must have received before. */
    appParam = ObexObjMapHdrValue( obex_app_params, len, src, &objLen );
    if( appParam )
    {
        uint16 offset = 0;
        while(objLen >= (offset + MAPC_2BYTE_APP_HDR_SIZE ))
        {
            if( appParam[offset] == sizeId )
            {
                offset += MAPC_2BYTE_SIZE; /* skip header */
                masTask->lastAppParam = (appParam[offset] << 8) | 
                                        (appParam[offset+1] & 0xFF);
                break;
            }
            else
            {
                offset += (MAPC_2BYTE_SIZE + appParam[offset+1]);
            }
        }
    }
   
    if( !ObexObjFilterHdrValue( masTask->session, id, &objLen ) ) return 0;

    return objLen;
}

/**************************************************************************
 * NAME:
 *  mapcMasHandleNotificationRegResp
 *
 * DESCRIPTION
 *  Handle the notification registration response
 *
 * PARAMETERS
 *  masTask - The session task
 *  status  - Status for Notification registration request
 *  len     - length of the headers in the response
 *  src     - Source containing all OBEX response headers.
 ************************************************************************/
static void mapcMasHandleNotificationRegResp( Mapc masTask, 
                                            MapcStatus status,
                                            uint16 len, 
                                            Source src )
{
    mapcMasSetNotificationCfm( masTask, status );

    /* Drop the packet as there is nothing meaningful to parse */
    SourceDrop(src, len);
} 

/**************************************************************************
 * NAME:
 *  mapcMasHandleFolderListingResp
 *
 * DESCRIPTION
 *  Handle the folder listing response
 *
 * PARAMETERS
 *  masTask - The session task
 *  status  - Status for folder listing request
 *  len     - length of the headers in the response
 *  src     - Source containing all OBEX response headers.
 ************************************************************************/
static void mapcMasHandleFolderListingResp( Mapc masTask, 
                                            MapcStatus status,
                                            uint16 len, 
                                            Source src )
{
    uint16 objLen = 0;

    if( ( status == mapc_success ) || ( status == mapc_pending ) )
    {
        objLen = mapcMasParseListResp( masTask, status,
                                       mapc_app_folder_listing_size,
                                       len, src );

        if( objLen > SourceSize(src) ) status = mapc_invalid_parameter;

        mapcMasGetFolderCfm( masTask, status, 
                             masTask->lastAppParam, objLen, src );
    }
    else
    {
        mapcMasGetFolderErrCfm( masTask, status );
    }
} 

/**************************************************************************
 * NAME:
 *  mapcMasHandleGetMessageResp
 *
 * DESCRIPTION
 *  Handle the get message 
 *
 * PARAMETERS
 *  masTask - The session task
 *  status  - Status for Get message request
 *  len     - length of the headers in the response
 *  src     - Source containing all OBEX response headers.
 ************************************************************************/
static void mapcMasHandleGetMessageResp( Mapc masTask, 
                                         MapcStatus status )
{
    Source src;
    uint16 objLen = 0;
    ObexSeqHeaderId id = ( status == mapc_pending )? obex_body_hdr: 
                                                     obex_ebody_hdr;

    src = ObexObjFilterHdrValue( masTask->session, id, &objLen );  

    if( objLen > SourceSize(src) ) status = mapc_invalid_parameter;

    mapcMasGetMessageCfm( masTask, status, objLen, src );
}


/**************************************************************************
 * NAME:
 *  mapcMasHandleMessagesListingResp
 *
 * DESCRIPTION
 *  Handle the messages listing response
 *
 * PARAMETERS
 *  masTask - The session task
 *  status  - Status for messages listing request
 *  len     - length of the headers in the response
 *  src     - Source containing all OBEX response headers.
 ************************************************************************/
static void mapcMasHandleMessagesListingResp( Mapc masTask, 
                                              MapcStatus status,
                                              uint16 len, 
                                              Source src )
{
    uint16 objLen = 0;

    if( ( status == mapc_success ) || ( status == mapc_pending ) )
    {
        objLen = mapcMasParseListResp( masTask, status,
                                       mapc_app_message_listing_size,
                                       len, src );

        if( objLen > SourceSize(src) ) status = mapc_invalid_parameter;

        mapcMasGetMsgListCfm( masTask, status, 
                              masTask->lastAppParam, objLen, src );
    }
    else
    {
        mapcMasGetMsgListErrCfm( masTask, status );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasHandlePutMessageResp
 *
 * DESCRIPTION
 *  Handle the put message 
 *
 * PARAMETERS
 *  masTask - The session task
 *  status  - Status for Put message request
 *  len     - length of the headers in the response
 *  src     - Source containing all OBEX response headers.
 ************************************************************************/
static void mapcMasHandlePutMessageResp( Mapc masTask, 
                                         MapcStatus status ) 
{
    uint16 objLen = 0;
    const uint8* handle = NULL;
        
    if( (status == mapc_pending) && (SourceSize( masTask->srcOut)) )
    {
        /* Send message internally */
        MAKE_MAPC_MESSAGE( MAPC_INT_PUT_CONTINUE );
        
        message->session = masTask;
        message->data = masTask->srcOut;
        message->final = masTask->finalPut;
        MessageSend( mapcGetProfileTask(), MAPC_INT_PUT_CONTINUE, message );
        return;
    }

    /* Remote is supposed to send the name handle once the entire data 
       is pushed */
    if( ( masTask->finalPut ) && 
        (status == mapc_success || status == mapc_pending) )
    {
       Source src =  ObexObjFilterHdrValue( masTask->session,
                                            obex_name_hdr,
                                            &objLen ) ;
        if( objLen > SourceSize(src) ) status = mapc_invalid_parameter;
        else handle = SourceMap( src );
    }
    
    /* Send the message CFM to the application */
    mapcMasPutMessageCfm( masTask, status, objLen, handle );

    return;
}

 

/************************************************************************
 * NAME:
 *  mapcMasIntSetMessage
 *
 * DESCRIPTION
 *  Set Message Status
 ************************************************************************/
void mapcMasIntSetMessage( MAPC_INT_SET_MSG_STATUS_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(set_msg_type) + MAPC_SET_MSG_HDR_SIZE;
    MapcStatus status = mapc_failure;

    if( !IsMapcConnected( masTask ) || !IsMapcFuncFree( masTask ) )  
    {
        mapcMasSetMessageCfm( masTask,  mapc_invalid_state );
        return;  
    }

    /* Size is less than 255. This should be going in a single packet */
    if( ( ObexObjNew( masTask->session ) >= reqSize ) )
    {
        uint8   appHdr[MAPC_BYTE_APP_HDR_SIZE * 2];

        /* Create the application header */
        mapcFrameByteAppParams( &appHdr[0],
                                mapc_app_status_indicator, 
                                message->indicator );

        mapcFrameByteAppParams( &appHdr[MAPC_BYTE_APP_HDR_SIZE], 
                                mapc_app_status_value,
                                message->value );

        /* Add All headers */
        if( mapcFrameSeqHdr( masTask, message->handle, 
                                MAPC_HANDLE_LEN, obex_name_hdr ) &&
            mapcFrameSeqHdr( masTask, set_msg_type, 
                                sizeof(set_msg_type), obex_type_hdr ) &&
            mapcFrameSeqHdr( masTask, appHdr, MAPC_BYTE_APP_HDR_SIZE * 2,
                             obex_app_params ) && 
            mapcFrameSeqHdr( masTask, NULL, 1, obex_ebody_hdr ) ) 

        {
            ObexPutRequest( masTask->session, TRUE );
            MAPC_SET_FUNCTION( masTask, mapc_message_status );
            status = mapc_success;
        }
    }

    if( status != mapc_success )
    {
        ObexObjDelete( masTask->session );
        mapcMasSetMessageCfm( masTask, status );           
    }
}

/*************************************************************************
 * NAME:
 *  mapcMasIntUpdateInbox
 *
 * DESCRIPTION
 *  Update the MSE inbox
 **************************************************************************/
void mapcMasIntUpdateInbox( MAPC_INT_UPDATE_INBOX_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(update_inbox_type) + MAPC_UPDATE_INBOX_HDR_SIZE;

    if( !IsMapcConnected( masTask ) || !IsMapcFuncFree( masTask ) )  
    {
        mapcMasUpdateInboxCfm( masTask,  mapc_invalid_state );
        return;  
    }

    /* Size is less than 255. This should be going in a single packet */
    if( ( ObexObjNew( masTask->session ) >= reqSize ) &&
          mapcFrameSeqHdr( masTask, update_inbox_type,
                           sizeof(update_inbox_type),obex_type_hdr ) &&
          mapcFrameSeqHdr( masTask, NULL, 1, obex_ebody_hdr ) )
    { 
        ObexPutRequest( masTask->session, TRUE );
        MAPC_SET_FUNCTION( masTask, mapc_update_inbox );
    }
    else
    {
        ObexObjDelete( masTask->session );
        mapcMasUpdateInboxCfm( masTask, mapc_failure );
    }
}


/**************************************************************************
 * NAME:
 *  mapcMasIntSetNotification
 *
 * DESCRIPTION
 *  Register or Unregister the Message Notification
 ************************************************************************/
void mapcMasIntSetNotification( MAPC_INT_SET_NOTIFICATION_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(notification_type) + MAPC_NOTIFICATION_HDR_SIZE;
    MapcStatus status = mapc_failure;

    if( !IsMapcConnected( masTask ) || !IsMapcFuncFree( masTask ) )  
    {
        mapcMasSetNotificationCfm( masTask,  mapc_invalid_state );
        return;  
    }

    /* Size is less than 255. This should be going in a single packet */
    if( ( ObexObjNew( masTask->session ) >= reqSize ) )
    {
        uint8  appHdr[MAPC_BYTE_APP_HDR_SIZE];

        /* Create the application header */
        mapcFrameByteAppParams( &appHdr[0],
                                mapc_app_notification_status,
                                message->status );

        /* Add all Headers */
        if( mapcFrameSeqHdr( masTask, notification_type,
                             sizeof(notification_type),obex_type_hdr ) &&
            mapcFrameSeqHdr( masTask, appHdr, MAPC_BYTE_APP_HDR_SIZE, 
                             obex_app_params ) &&
            mapcFrameSeqHdr( masTask, NULL, 1, obex_ebody_hdr ) )
        {
            ObexPutRequest( masTask->session, TRUE );
            MAPC_SET_FUNCTION( masTask, mapc_reg_notification );
            status = mapc_success;
        }
    }

    /* Error - Delete the Object */
    if( status != mapc_success )
    {
        ObexObjDelete( masTask->session );
        mapcMasSetNotificationCfm( masTask, status );
    }
}


/**************************************************************************
 * NAME:
 *  mapcMasIntGetMsg
 *
 * DESCRIPTION
 *  Send a request to get the message
 ************************************************************************/
void mapcMasIntGetMsg( MAPC_INT_GET_MSG_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(get_put_msg_type) + MAPC_GET_MSG_HDR_SIZE ; 
    MapcStatus status = mapc_failure;

    if( !IsMapcConnected( masTask ) || !IsMapcFuncFree( masTask ) )  
    {
        mapcMasGetMsgErrCfm( masTask , mapc_invalid_state );
        return;  
    }

    /* Size is less than 255. This should be going in a single packet */
    if( ( ObexObjNew( masTask->session ) >= reqSize ) )
    {
        uint8   appHdr[MAPC_BYTE_APP_HDR_SIZE * 2];

        /* Create the application header */
        mapcFrameByteAppParams( &appHdr[0], 
                                mapc_app_attachment,
                                0 ); 

        mapcFrameByteAppParams( &appHdr[MAPC_BYTE_APP_HDR_SIZE], 
                                mapc_app_char_set,
                                message->charSet );

        /* Add All headers */
        if( mapcFrameSeqHdr( masTask, message->handle, 
                                MAPC_HANDLE_LEN, obex_name_hdr ) &&
            mapcFrameSeqHdr( masTask, get_put_msg_type, 
                             sizeof(get_put_msg_type), obex_type_hdr ) &&
            mapcFrameSeqHdr( masTask, appHdr, MAPC_BYTE_APP_HDR_SIZE * 2,
                             obex_app_params ) )
        {
            ObexGetRequest( masTask->session, TRUE );
            MAPC_SET_FUNCTION( masTask, mapc_get_message );
            status = mapc_success;
        }
    }

    if( status != mapc_success )
    {
        ObexObjDelete( masTask->session );
        mapcMasGetMsgErrCfm( masTask, status );           
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasIntGetMsgList
 *
 * DESCRIPTION
 *  Send a request to get the messages listing
 ************************************************************************/
void mapcMasIntGetMsgList( MAPC_INT_GET_MSG_LIST_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(get_msg_list_type) + 
                     MAPC_GET_MSG_LIST_HDR_SIZE +
                     message->nameLen * 2; 

    if( !IsMapcConnected( masTask ) || !IsMapcFuncFree( masTask ) )  
    {
        mapcMasGetMsgListErrCfm( masTask , mapc_invalid_state );
        return;  
    }

    masTask->lastAppParam = MAPC_MAX_LIST_OBJECT_SIZE; /* reset to max*/

    /* Claim OBEX Object space */
    if( ( ObexObjNew( masTask->session ) >= reqSize )  &&
          mapcFrameSeqHdr( masTask, get_msg_list_type,
                           sizeof( get_msg_list_type), obex_type_hdr ) && 
          mapcFrameSeqHdr( masTask, message->name, 
                           message->nameLen, obex_name_hdr ) &&   
          mapcFrameListMsgParams( masTask, message->maxListCount,
                                  message->startOffset,
                                  message->filter ) )
    {
        ObexGetRequest( masTask->session, TRUE );
        MAPC_SET_FUNCTION( masTask, mapc_message_listing );
    }
    else
    {
        ObexObjDelete( masTask->session );
        mapcMasGetMsgListErrCfm( masTask, mapc_failure );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasIntPutMessage
 *
 * DESCRIPTION
 *  Send a request to push message
 ************************************************************************/
void mapcMasIntPutMessage( MAPC_INT_PUT_MSG_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(get_put_msg_type) + MAPC_PUT_MSG_HDR_SIZE +
                    message->nameLen * 2 ; /* name is unicode */
    MapcStatus status = mapc_failure;

    if( !IsMapcConnected( masTask ) || !IsMapcFuncFree( masTask ) )  
    {
        mapcMasPutMsgCfm( masTask , mapc_invalid_state );
        return;  
    }

    /* Claim OBEX Object space and add the headers */
    if( ObexObjNew( masTask->session ) >= reqSize )
    {
        /* Add 3 Application headers */
        uint8 appHdr[MAPC_BYTE_APP_HDR_SIZE * 3];

        /* Create the application header, 
           transparent and retry are currently pre defined values. */
        mapcFrameByteAppParams( &appHdr[0], 
                                mapc_app_transparent,
                                MAPC_PUSH_TRANSPARENT ); 

        mapcFrameByteAppParams( &appHdr[MAPC_BYTE_APP_HDR_SIZE], 
                                mapc_app_char_set,
                                message->charSet);

        mapcFrameByteAppParams( &appHdr[MAPC_BYTE_APP_HDR_SIZE * 2],
                                mapc_app_retry,
                                MAPC_PUSH_RETRY );

        /* All headers except body must be able to fit in the
           single OBEX packet  since the minimum maxPaktLen is 255. */
        if(mapcFrameSeqHdr( masTask, get_put_msg_type, 
                            sizeof(get_put_msg_type), obex_type_hdr ) &&
           mapcFrameSeqHdr( masTask, message->name, 
                           message->nameLen, obex_name_hdr ) &&
           mapcFrameSeqHdr( masTask, appHdr, MAPC_BYTE_APP_HDR_SIZE * 3,
                             obex_app_params ) )
        {
            ObexPutRequest( masTask->session, FALSE );
            MAPC_SET_FUNCTION( masTask, mapc_push_message );
            masTask->srcOut = message->data;
            masTask->finalPut = message->final;
            status = mapc_success;
        }
    }

    if( status != mapc_success )
    {
        ObexObjDelete( masTask->session );
        mapcMasPutMsgCfm( masTask , mapc_invalid_state );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasIntPutContinue
 *
 * DESCRIPTION
 *  Send a request to push message
 ************************************************************************/
void mapcMasIntPutContinue( MAPC_INT_PUT_CONTINUE_T* message )
{
    Mapc masTask = message->session;
    uint16 srcSize = SourceSize( message->data );
    uint16 reqSize;
    bool   final = FALSE;

    if( ( !IsMapcConnected( masTask ) ) || 
        ( masTask->function != mapc_push_message ) )  
    {
        mapcMasPutMsgCfm( masTask , mapc_invalid_state );
        return;  
    }

    if( !(reqSize = ObexObjNew( masTask->session )) )
    {
        /* This must not happen */
        MAPC_ERR(("Buffer Overflow"));
        SourceEmpty(message->data);
        ObexAbort( masTask->session );
        return;
    }

    if( reqSize > srcSize )
    {
        reqSize = srcSize;
        final = message->final;
    }

    /* This must not return failure since reqSize <= ObexObjNew() */
    if( ObexObjAddVariableHdr( masTask->session, 
                                 final? obex_ebody_hdr: obex_body_hdr ,
                               reqSize,
                               message->data ) ) 
    {
        ObexPutRequest( masTask->session, final );
        masTask->srcOut = message->data;
        masTask->finalPut = message->final;
    }
    else
    {
        /* This must not happen since reqSize is returned by ObexNew*/
        MAPC_ERR(("Failed to frame header"));
        SourceEmpty(message->data);
        ObexAbort( masTask->session );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasIntGetFolder
 *
 * DESCRIPTION
 *  Send a request to get the folder listing
 ************************************************************************/
void mapcMasIntGetFolder( MAPC_INT_GET_FOLDER_T* message )
{
    Mapc   masTask = message->session;
    uint16 reqSize = sizeof(get_folder_type) + MAPC_GET_FOLDER_HDR_SIZE;  
                                       
    if( !IsMapcConnected( masTask )  || !IsMapcFuncFree( masTask ))   
    {
        mapcMasGetFolderErrCfm( masTask , mapc_invalid_state );
        return;  
    }

    masTask->lastAppParam = MAPC_MAX_LIST_OBJECT_SIZE; /* reset to max*/

    /* Send the packet */
    if( ( ObexObjNew( masTask->session ) >= reqSize )  &&
        mapcFrameSeqHdr( masTask, get_folder_type, sizeof(get_folder_type),
                         obex_type_hdr ) &&
        mapcFrameListFolderParams( masTask, message->maxListCount,
                                message->startOffset ) )
    {
        ObexGetRequest( masTask->session, TRUE );
        MAPC_SET_FUNCTION( masTask, mapc_folder_listing );
    }
    else
    {
        ObexObjDelete( masTask->session );
        mapcMasGetFolderErrCfm( masTask, mapc_failure );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasIntContinue
 *
 * DESCRIPTION
 *  Send a request to continue or terminate the current Get request
 ************************************************************************/
void mapcMasIntGetContinue( MAPC_INT_GET_CONTINUE_T* message)
{
    Mapc masTask = message->session;

    if( !IsMapcConnected( masTask ) ) return; /* Ignore */

    if(message->proceed)
    {
        if( ( masTask->function ==  mapc_folder_listing ) ||
            ( masTask->function ==  mapc_message_listing ) ||
            ( masTask->function ==  mapc_get_message) )
        {
            ObexGetRequest( masTask->session, TRUE );  
        }
        /* else ignore this request */
    }
    else
    {
        ObexSourceDrop( masTask->session);
        
        /* Abort only if any transaction is in progress */
        if( IsMapcFuncBlocked( masTask) ) ObexAbort( masTask->session );
    }
}

/**************************************************************************
 * NAME:
 *  mapcMasIntSetFolder
 *
 * DESCRIPTION
 *  Send a request to Set the remote Folder
 ************************************************************************/
void mapcMasIntSetFolder( MAPC_INT_SET_FOLDER_T* message )
{
    Mapc masTask = message->session;

    if( IsMapcConnected( masTask ) && IsMapcFuncFree( masTask ) )  
    {
        ObexSetPathRequest( masTask->session, 
                    (message->flag)? obex_folder_out: obex_folder_in , 
                     message->nameLen,
                     message->name );
        MAPC_SET_FUNCTION( masTask, mapc_set_folder );
    }
    else
    {
        mapcMasSetFolderCfm( masTask, mapc_invalid_state );
    }

}

/**************************************************************************
 * NAME:
 *  mapcMasHandleSetPathCfm
 *
 * DESCRIPTION
 *  Handle the Confirmation message
 ************************************************************************/
void mapcMasHandleSetPathCfm( Mapc masTask, OBEX_SET_PATH_CFM_T* message )
{
    MapcStatus status;

    if( (IsMapcConnected( masTask )) && 
        (masTask->function == mapc_set_folder) )
    {
        status = mapcMasConvertStatus( message->status );
        MAPC_RESET_FUNCTION( masTask );
        mapcMasSetFolderCfm( masTask, status );
    }
}


/**************************************************************************
 * NAME:
 *  mapcMasHandleGetPutCfm
 *
 * DESCRIPTION
 *  MAS client to handle a PUT CFM message
 ************************************************************************/
void mapcMasHandleGetPutCfm( Mapc masTask, OBEX_GETPUT_CFM_T* message )
{
    MapcStatus status;

    if( ! IsMapcConnected( masTask) ) 
    {
        /* Invalid State */
        MAPC_INFO(("Not connected\n"));
        return;
    }
 
    status = mapcMasConvertStatus( message->status );

    switch( masTask->function)
    {
        case mapc_reg_notification:
            mapcMasHandleNotificationRegResp( masTask, status,
                                            message->sourceLen,
                                            message->source  );
            break;

        case mapc_folder_listing:
            mapcMasHandleFolderListingResp( masTask, status,
                                            message->sourceLen,
                                            message->source  );
            break;

        case mapc_message_listing:
            mapcMasHandleMessagesListingResp( masTask, status,
                                              message->sourceLen,
                                              message->source  );

            break;

        case mapc_get_message:
            mapcMasHandleGetMessageResp( masTask, status );
            break;

        case mapc_update_inbox:
            mapcMasUpdateInboxCfm( masTask, status );
            break;

        case mapc_message_status:
            mapcMasSetMessageCfm( masTask, status );
            break;

        case mapc_push_message:
            mapcMasHandlePutMessageResp(masTask, status ); 
            break;

        default:
            MAPC_ERR(("Unknown function"));
            return;
    }

    if( status != mapc_pending) MAPC_RESET_FUNCTION( masTask ); 
}

