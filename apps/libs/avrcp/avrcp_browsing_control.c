/****************************************************************************
Copyright (c) 2009 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_browsing_control.c

DESCRIPTION
This file defines the API Functions for sending all Browsing related 
requests and responses on a AVC control channel.  

Support of NowPlaying feature is mandatory if Browsing is supported. Support
of Search mandates the Browsing and NowPlaying feature as well.

Controller API functions are 
    -AvrcpPlayItem
    -AvrcpAddToNowPlaying

Target API functions are 
    - AvrcpPlayItemResponse
    - AvrcpAddToNowPlayingResponse
    - AvrcpEventNowContentChangedResponse
    - AvrcpEventUidsChangedResponse

NOTES

*/
/****************************************************************************
    Header files
*/
#include <panic.h>

#include "avrcp_metadata_transfer.h"
#include "avrcp_send_response.h"
#include "avrcp_notification_handler.h"
#include "avrcp_init.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/* Macros */
#define avrcpNowPlayingCfm(avrcp,status,pdu_id) avrcpSendCommonStatusCfm(avrcp,\
                       status, \
                      (uint16)((pdu_id==AVRCP_PLAY_ITEM_PDU_ID) \
                                        ? AVRCP_PLAY_ITEM_CFM \
                                        : AVRCP_ADD_TO_NOW_PLAYING_CFM))

/****************************************************************************
*NAME    
*  avrcpSendNowPlayingCommand
*
*DESCRIPTION
* Send a AddToNowPlaying or PlayItem Command to the Target.
***************************************************************************/    

static void avrcpSendNowPlayingCommand( AVRCP*              avrcp, 
                                        avrcp_browse_scope  scope,    
                                        avrcp_browse_uid    uid,  
                                        uint16              uid_counter,
                                        uint16              pdu_id)
{
    uint8 extra_params[AVRCP_PLAY_ITEM_SIZE];
    avrcp_status_code status;

    if((scope ==  avrcp_media_player_scope) ||
       (scope >   avrcp_now_playing_scope))       
    {
        avrcpNowPlayingCfm(avrcp, avrcp_rejected_invalid_param, pdu_id);
        return;
    }

    /* Fill in the Parameters */
    extra_params[0] = (uint8)scope;
    convertUint32ToUint8Values(&extra_params[1], uid.msb);
    convertUint32ToUint8Values(&extra_params[5], uid.lsb);
    AVRCP_UINT16_TO_UINT8(uid_counter, extra_params, 9);

    status = avrcpMetadataControlCommand(avrcp, pdu_id, 
                                       (pdu_id == AVRCP_PLAY_ITEM_PDU_ID)?
                                       avrcp_play_item:avrcp_add_to_now_playing,
                                       AVRCP_PLAY_ITEM_SIZE,
                                       extra_params, 0 , 0);

   /* Failure confirmation if the request got failed */
    if(status != avrcp_success)
    {
        avrcpNowPlayingCfm(avrcp, status, pdu_id);
    }
}


/****************************************************************************
*NAME    
*   AvrcpPlayItemRequest
*
*DESCRIPTION
*  API function to send PlayItem request to the Target .
*    
*PARAMETERS
*   avrcp              - Task
*   avrcp_browse_scope - scope
*   avrcp_browse_uid   - uid
*   uint16             - counter
*
*RETURN
*  AVRCP_PLAY_ITEM_CFM 
*****************************************************************************/
void AvrcpPlayItemRequest( AVRCP*              avrcp, 
                           avrcp_browse_scope  scope,    
                           avrcp_browse_uid    uid,  
                           uint16              uid_counter)
{
    avrcpSendNowPlayingCommand( avrcp, scope, 
                                uid, uid_counter, 
                                AVRCP_PLAY_ITEM_PDU_ID);
}

/****************************************************************************
*NAME    
*   AvrcpAddToNowPlayingRequest
*
*DESCRIPTION
*  API function to send AddToNowPlaying request to the Target .
*    
*PARAMETERS
*   avrcp              - Task
*   avrcp_browse_scope - scope
*   avrcp_browse_uid   - uid
*   uint16             - counter
*
*RETURN
*  AVRCP_PLAY_ITEM_CFM 
*****************************************************************************/
void AvrcpAddToNowPlayingRequest( AVRCP*              avrcp, 
                                  avrcp_browse_scope  scope,    
                                  avrcp_browse_uid    uid,  
                                  uint16              uid_counter)
{
    avrcpSendNowPlayingCommand( avrcp, scope, 
                                uid, uid_counter, 
                                AVRCP_ADD_TO_NOW_PLAYING_PDU_ID);
}


#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
*NAME    
*   AvrcpPlayItemResponse
*
*DESCRIPTION
*  API function to send PlayItem response to the CT . Application must call 
*  this function in response to AVRCP_PLAY_ITEM_IND message.
*    
*PARAMETERS
*   avrcp              - Task
*   avrcp_response_type - Response indicating whether request was accepted or
*                         rejected. All valid responses are defined in 
*                         avrcp_response_type.
*
*****************************************************************************/
void AvrcpPlayItemResponse( AVRCP*              avrcp,  
                            avrcp_response_type response)
{
    if (avrcp->bitfields.block_received_data ==  avrcp_play_item)
    {
        avrcpSendMetaDataControlResponse(avrcp, response,
                                        AVRCP_PLAY_ITEM_PDU_ID);
    }
    else
    {
        AVRCP_INFO(("AvrcpPlayItemResponse: CT is not waiting for"
                     " the response\n")); 
    }      
}

/****************************************************************************
*NAME    
*   AvrcpAddToNowPlayingResponse
*
*DESCRIPTION
*  API function to sendAddToNowPlaying response to the CT . Application must 
*  call  this API in response to AVRCP_ADDTO_NOW_PLAYING_IND
*    
*PARAMETERS
*   avrcp              - Task
*   avrcp_response_type - Response indicating whether request was accepted or
*                         rejected. All valid responses are defined in 
*                         avrcp_response_type.
*
*****************************************************************************/
void AvrcpAddToNowPlayingResponse( AVRCP*              avrcp,  
                                   avrcp_response_type response)
{
    if (avrcp->bitfields.block_received_data ==  avrcp_add_to_now_playing)
    {
        avrcpSendMetaDataControlResponse(avrcp, response,
                                   AVRCP_ADD_TO_NOW_PLAYING_PDU_ID);
    }
    else
    {
        AVRCP_INFO(("AvrcpAddToNowPlayingResponse: CT is not waiting for"
                     " the response\n")); 
    }      
}

/****************************************************************************
*NAME 
*   AvrcpEventNowPlayingContentChangedResponse
*
*DESCRIPTION
*   This function is used by the TG to notify CT on Now Playing
*    Content Changed Response.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type - Response indicating whether request was accepted or
*                         rejected. All valid responses are defined in 
*                         avrcp_response_type.
*
*****************************************************************************/
void AvrcpEventNowPlayingContentChangedResponse(AVRCP                *avrcp,
                                                avrcp_response_type  response)
{
    avrcpSendEventResponse(avrcp, response, EVENT_PLAYING_CONTENT_CHANGED);
}

/****************************************************************************
*NAME 
*   AvrcpEventUidsChangedResponse
*
*DESCRIPTION
*   This function is used by the TG to notify CT on UIDs changed Event.
*
*PARAMETRS
*   avrcp              - Task
*   avrcp_response_type - Response indicating whether request was accepted or
*                         rejected. All valid responses are defined in 
*                         avrcp_response_type.
*   uint16              - UID Counter
*
*****************************************************************************/
void AvrcpEventUidsChangedResponse(AVRCP                *avrcp,
                                   avrcp_response_type  response,
                                   uint16               uid_counter)
{

    if (isEventRegistered(tg, avrcp, EVENT_UIDS_CHANGED))
    {
        /* Internal Event response */
       MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES); 
       message->response=response;

       if(isAvrcpDatabaseEnabled(avrcpGetDeviceTask()))
       {
           message->uid_counter = uid_counter;
       }
       else
       {
           message->uid_counter = 0;
       }

      MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES,
                    message);

    }
    else
    {
       AVRCP_INFO(("AvrcpEventUidsChangedResponse: " 
                    "Event not registered\n"));
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/

