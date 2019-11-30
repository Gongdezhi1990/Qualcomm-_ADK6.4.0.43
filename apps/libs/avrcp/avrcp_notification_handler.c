/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_notification_handler.c

DESCRIPTION
This file defines the internal functions for handling Notifications feature,
which includes
    - GetPlayStatus
    - RegisterNotification
    - EVENT_PLAY_STATUS_CHANGED
    - EVENT_TRACK_CHANGED
    - EVENT_TRACK_REACHED_END
    - EVENT_TRACK_REACHED_START
    - EVENT_PLAYBACK_POS_CHANGED
    - EVENT_BATTERY_STATUS_CHANGED
    - EVENT_SYSTEM_STATUS_CHANGED
    - EVENT_PLAYER_APPLICATION_SETTINGS_CHANGED    
    - EVENT_UIDS_CHANGED
    - EVENT_NOW_CONTENT_CHANGED
    - EVENT_ADDRESSED_PLAYER_CHANGED
    - EVENT_AVAILABLE_PLAYERS_CHANGED
    - EVENT_ABSOLUTE_VOLUME
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>

#include "avrcp_notification_handler.h"
#include "avrcp_continuation_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_common.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

/****************************************************************************
 *NAME    
 *    avrcpSendNotification  
 *
 *DESCRIPTION
 *   Send Notification Indications to the CT application.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * response         - Response code
 * ptr              - Pointer to the response data
 * packet_size      - Data size in ptr
 ****************************************************************************/
void avrcpSendNotification( AVRCP *avrcp,  
                            avrcp_response_type response, 
                            const uint8* ptr, 
                            uint16 packet_size)
{
    avrcp_supported_events event_id;
    uint16 data_len    = 0;
    uint16 data_start  = 0;
    uint16 transaction = (avrcp->av_msg[0] & AVCTP_TRANSACTION_MASK) >>
                         AVCTP0_TRANSACTION_SHIFT;
    if(!packet_size)
    {
        /* Unknown event. Ignore */
        return;
    }

    if(response == avctp_response_rejected)
    {
        for(event_id = 1; event_id <= AVRCP_MAX_NUM_EVENTS; event_id++)
        {
            if(isEventRegistered(ct, avrcp, event_id) &&
                GetNotificationTransaction(ct, avrcp, event_id) == transaction)
            {
                break;
            }
        }       

        /* data[0] would be error status code */
        response = ptr[data_start] | AVRCP_ERROR_STATUS_BASE;
    }
    else
    {
        event_id = ptr[data_start];
        data_start++;
        data_len = packet_size-data_start;
    }
    
    switch (event_id)
    {
    case avrcp_event_playback_status_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND);
            message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            
            message->response = response;

            /* EVENT_PLAYBACK_STATUS_CHANGED Length expected is 1 */
            if(data_len){
                message->play_status = ptr[data_start];
            }
            else
            {
                /* This will be error value on rejected response. 
                   App will ignore it */
                message->play_status = 0xFF;
            }
            MessageSend(avrcp->clientTask,
                         AVRCP_EVENT_PLAYBACK_STATUS_CHANGED_IND, message);
            
        }
        break;
    case avrcp_event_track_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_CHANGED_IND);
            message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            
            /*
             * If the Packet comes in continuation or end packet
             * fragmented, it may  not process
             * Identifier is 8 octets long
             */
            message->response = response;
            if(data_len >= 8)
            {
                message->track_index_high =
                               convertUint8ValuesToUint32(&ptr[data_start]);
                message->track_index_low = 
                               convertUint8ValuesToUint32(&ptr[data_start+4]);
            }
            else
            {
                message->track_index_high = 0;
                message->track_index_low = 0;
            }
            MessageSend(avrcp->clientTask, 
                        AVRCP_EVENT_TRACK_CHANGED_IND, message);
        }
        break;
    case avrcp_event_track_reached_end:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_REACHED_END_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            message->response = response;
            MessageSend(avrcp->clientTask, 
                        AVRCP_EVENT_TRACK_REACHED_END_IND, message);
        }
        break;
    case avrcp_event_track_reached_start:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_TRACK_REACHED_START_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            message->response = response;
            MessageSend(avrcp->clientTask, 
                        AVRCP_EVENT_TRACK_REACHED_START_IND, message);
        }
        break;
    case avrcp_event_playback_pos_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif

            message->response = response;
            /*
             * If the Packet comes in continuation or end packet fragmented,
             * it may  not process the Identifier which is 8 octets long
             */
            if(data_len >= 4)
            {
                message->playback_pos =
                             convertUint8ValuesToUint32(&ptr[data_start]);
            }
            else
            {
                message->playback_pos = 0;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_PLAYBACK_POS_CHANGED_IND, message);
        }
        break;
    case avrcp_event_batt_status_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_BATT_STATUS_CHANGED_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif
            message->response = response;
            if(data_len)
            {
                message->battery_status = ptr[data_start];
            }
            else
            {
                message->battery_status = 0;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_BATT_STATUS_CHANGED_IND, message);
        }
        break;
    case avrcp_event_system_status_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND);
            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
#endif

            message->response = response;
            if(data_len)
            {
                message->system_status = ptr[data_start];
            }
            else
            {
                message->system_status = 0;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_SYSTEM_STATUS_CHANGED_IND, message);
        }
        break;
    case avrcp_event_player_app_setting_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND);

            message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
        /* Deprecated fields will be removed later */
            message->transaction = transaction;
            message->no_packets = 0;
            message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
            message->data_offset = 0;
            message->metadata_packet_type = avrcp_packet_type_single;
#endif

            message->response = response;

            if(data_len >= 3){
                message->number_of_attributes = ptr[data_start];
                message->attributes =avrcpSourceFromConstData(avrcp, 
                                    ptr+data_start+1, data_len-1);
                message->size_attributes = data_len - 1;
            }
            else
            {
                message->number_of_attributes = 0;
                message->size_attributes = 0;
                message->attributes = 0;
            }

            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_PLAYER_APP_SETTING_CHANGED_IND, message);

        }
        break;
    case avrcp_event_now_playing_content_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND);
            message->avrcp = avrcp;
             message->response = response;
            MessageSend(avrcp->clientTask,
                       AVRCP_EVENT_NOW_PLAYING_CONTENT_CHANGED_IND, 
                       message);

        }
        break;

    case avrcp_event_available_players_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND);
            message->avrcp = avrcp;
            message->response = response;
            MessageSend(avrcp->clientTask,
                       AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND, message);
        }
        break;

    case  avrcp_event_addressed_player_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND);
            message->avrcp = avrcp;

            message->response = response;
            if(data_len >= 4)
            {
                AVRCP_UINT8_TO_UINT16(ptr, message->player_id, data_start);
                AVRCP_UINT8_TO_UINT16(ptr, message->uid_counter, data_start+2);
                                     
            }
            else
            {
                message->player_id = 0;
                message->uid_counter = 0;
            }

            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND, message);
        }
        break;

    case avrcp_event_uids_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_UIDS_CHANGED_IND);
            message->avrcp = avrcp;

            message->response = response;
            if(data_len >= 2)
            {
                AVRCP_UINT8_TO_UINT16(ptr,message->uid_counter,data_start);
            }
            else
            {
                message->uid_counter = 0;
            }

            MessageSend(avrcp->clientTask,
                       AVRCP_EVENT_UIDS_CHANGED_IND, 
                       message);
        }
    
        break;

    case avrcp_event_volume_changed:
        {
            MAKE_AVRCP_MESSAGE(AVRCP_EVENT_VOLUME_CHANGED_IND);

            message->avrcp = avrcp;

            message->response = response;
            if(data_len)
            {
                message->volume = ptr[data_start];
            }
            else
            {
                message->volume = 0;
            }
            MessageSend(avrcp->clientTask,
                        AVRCP_EVENT_VOLUME_CHANGED_IND, message);
        }   
        break;
    default:
        /* Unknown event. ignore */
        return;
    }

    if(response != avctp_response_interim)
    {
        clearRegisterNotification(ct, avrcp, event_id);
    }

    /* Clear the pending flag only if the library was waiting for 
       the corresponding REGISTER_NOTIFICATION response. 
       otherwise ignore */
    if((event_id + avrcp_events_start_dummy) == avrcp->pending)
    {
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
        avrcp->pending = avrcp_none;
    }

    return;

}

/****************************************************************************
 *NAME    
 *    avrcpSendRegisterNotificationFailCfm  
 *
 *DESCRIPTION
 *   Send a failure confirmation for Register Notification request. This 
 *   will be send only during a local failure or response timeout for a 
 *   RegisterNotification() request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * status           - Status for avrcpRegisterNotification()
 * event_id         - Event ID for Register Notification request.
 ****************************************************************************/
void avrcpSendRegisterNotificationFailCfm(AVRCP *avrcp,
                                         avrcp_status_code status,
                                         avrcp_supported_events event_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_REGISTER_NOTIFICATION_CFM);

    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->transaction = 0;
#endif
    message->status = status;
    message->event_id = event_id;

    MessageSend(avrcp->clientTask, AVRCP_REGISTER_NOTIFICATION_CFM, message);
}


/****************************************************************************
 *NAME    
 *    avrcpSendGetPlayStatusCfm  
 *
 *DESCRIPTION
 *  Send an AVRCP_GET_PLAY_STATUS_CFM message to the client.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * status           - Application status
 * song_length      - Song length in GetPlayStatus response
 * song_elapsed     - Song position in GetPlayStatus response
 * play_status      - PlayStatus in GetPlayStatus response
 * 
 ****************************************************************************/
void avrcpSendGetPlayStatusCfm(AVRCP *avrcp, 
                               avrcp_status_code status, 
                               uint32 song_length, 
                               uint32 song_elapsed, 
                               avrcp_play_status play_status, 
                               uint8 transaction) 
{
    MAKE_AVRCP_MESSAGE(AVRCP_GET_PLAY_STATUS_CFM);

    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->transaction = transaction;
#else
    UNUSED(transaction);
#endif
    message->status = status;
    message->song_length = song_length;
    message->song_elapsed = song_elapsed;
    message->play_status = play_status;

    MessageSend(avrcp->clientTask, AVRCP_GET_PLAY_STATUS_CFM, message);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleGetPlayStatusResponse  
 *
 *DESCRIPTION
 *  Handle Get Play Status response received from the TG.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * status           - Application status
 * transaction      - Transaction ID
 * ptr              - Pointer to response data packet.
 * packet_size      - size of data packet.
 ****************************************************************************/
void avrcpHandleGetPlayStatusResponse(AVRCP             *avrcp, 
                                      avrcp_status_code status,
                                      uint8             transaction, 
                                      const  uint8      *ptr, 
                                      uint16            packet_size)
{
    uint32 song_length = 0, song_elapsed = 0;
    uint8 play_status =0;

    /* packet should be 9 bytes*/
    if((packet_size >= AVRCP_GET_PLAY_STATUS_SIZE) &&
       (status < AVRCP_ERROR_STATUS_BASE ))
    {
        /* Song length in 0-3 bytes */
        song_length = convertUint8ValuesToUint32(&ptr[0]);

        /* song elapsed in 4-7 bytes */
        song_elapsed = convertUint8ValuesToUint32(&ptr[4]);

        /* copy play status in the 8th byte*/
        play_status = ptr[8];
    } 

    avrcpSendGetPlayStatusCfm(avrcp, status,
                              song_length, song_elapsed, play_status, 
                              transaction);
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
 *NAME    
 *    sendRegisterNotificationResponse    
 *
 *DESCRIPTION
 *   This function is used by the TG to send Register Notification response. 
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * resp             - Response code
 * size_mandatory   - Data length in mandatory
 * mandatory        - It contains the library added response format header 
 * size_attributes  - Data length in attributes
 * attributes       - Application provided response data.
 ****************************************************************************/
void sendRegisterNotificationResponse(  AVRCP               *avrcp, 
                                        avrcp_response_type resp, 
                                        uint16              size_mandatory, 
                                        uint8               *mandatory, 
                                        uint16              size_attributes, 
                                        Source              attributes)
{
    /* 1 byte event and 1 byte error status code as data for 
       rejected response */ 
    uint8 error_data[AVRCP_DEFAULT_EVENT_DATA_SIZE];
    avrcp_packet_type  metadata_packet_type =  avrcp_packet_type_single;
    uint16 param_length=1;

    /* Clear the Notification if response is not interim*/
    if (resp != avctp_response_interim)
    {
        clearRegisterNotification(tg, avrcp, mandatory[0]);
    }

   /* Standardise the response code */
    error_data[0] = avrcpGetErrorStatusCode(&resp, AVRCP0_CTYPE_NOTIFY);

    if(resp == avctp_response_rejected) 
    {
        size_mandatory = param_length;
        size_attributes = 0;
        error_data[1] = mandatory[0]; /* PDU ID will not be sent to the peer */
        mandatory = &error_data[0];
    }
    else
    {
        param_length = size_mandatory + size_attributes;
        if (param_length > avrcp->av_max_data_size)
        {
            /* Not expecting a event notification response with 
               Fragmenetation */
            Panic();        
        }
    }
    
    avrcpSendMetadataResponse(avrcp,  resp, AVRCP_REGISTER_NOTIFICATION_PDU_ID,
                              attributes, metadata_packet_type, param_length, 
                              size_mandatory, mandatory);


}



/****************************************************************************
 *NAME    
 *    avrcpHandleRegisterNotificationCommand  
 *
 *DESCRIPTION
 *   Handle an incoming Register Notification command.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * ptr              - Data pointer to the command. 
 ****************************************************************************/
void avrcpHandleRegisterNotificationCommand(AVRCP *avrcp, const uint8 *ptr)
{
    uint8 event = ptr[0];

    MAKE_AVRCP_MESSAGE(AVRCP_REGISTER_NOTIFICATION_IND);
    message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->transaction = avrcp->bitfields.rsp_transaction_label;
#endif
    message->event_id = event;
    message->playback_interval = convertUint8ValuesToUint32(&ptr[1]);

    /* Store which event has been registered for. */
    registerEventNotification(tg, avrcp, event);
    /* Store the transaction label associated with the event. */
    avrcp->tg_notify_transaction_label[event-1] = avrcp->bitfields.rsp_transaction_label;

    MessageSend(avrcp->clientTask,AVRCP_REGISTER_NOTIFICATION_IND,message);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventPlaybackStatusChangedResponse  
 *
 *DESCRIPTION
 *  Send a play back status changed notification to CT on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventPlaybackStatusChangedResponse(AVRCP *avrcp,
                     const AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *res)
{
    /* 2 Byte length data for Play back status changed event */
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_playback_status_changed;
    mandatory_data[1] = res->play_status;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE,
                                     mandatory_data, 0, 0);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventTrackChangedResponse  
 *
 *DESCRIPTION
 *  Send a Track changed event notification to CT on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventTrackChangedResponse(AVRCP *avrcp,
                     const AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_TRACK_HDR_SIZE];

    mandatory_data[0] = avrcp_event_track_changed;
    convertUint32ToUint8Values(&mandatory_data[1], res->track_index_high);
    convertUint32ToUint8Values(&mandatory_data[5], res->track_index_low);

    sendRegisterNotificationResponse(avrcp, res->response, 
                                AVRCP_EVENT_TRACK_HDR_SIZE,
                                mandatory_data, 0, 0);
}
/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventPlaybackPosChangedResponse  
 *
 *DESCRIPTION
 *  Send a Playback position changed event notification to CT 
 *  on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventPlaybackPosChangedResponse(AVRCP *avrcp,
                 const AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *res)
{
    /* 5 Byte length data for play back position changed response */
    uint8 mandatory_data[AVRCP_EVENT_POS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_playback_pos_changed;
    convertUint32ToUint8Values(&mandatory_data[1], res->playback_pos);

    sendRegisterNotificationResponse(avrcp, res->response, 
                                    AVRCP_EVENT_POS_HDR_SIZE, 
                                    mandatory_data, 0, 0);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventBattStatusChangedResponse  
 *
 *DESCRIPTION
 *  Send a battery status changed response to CT on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventBattStatusChangedResponse(AVRCP *avrcp,
              const AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_batt_status_changed;
    mandatory_data[1] = res->battery_status;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE, 
                                     mandatory_data, 0, 0);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventSystemStatusChangedResponse  
 *
 *DESCRIPTION
 *  Send a System status changed response to CT on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventSystemStatusChangedResponse(AVRCP *avrcp,
               const AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_system_status_changed;
    mandatory_data[1] = res->system_status;

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE, 
                                     mandatory_data, 0, 0);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventPlayerAppSettingChangedResponse  
 *
 *DESCRIPTION
 *  Send a Player application settings changed response to CT 
 *  on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventPlayerAppSettingChangedResponse(AVRCP *avrcp,
             const AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *res)
{
    uint8 mandatory_data[AVRCP_EVENT_STATUS_HDR_SIZE];
    mandatory_data[0] = avrcp_event_player_app_setting_changed;
    mandatory_data[1] = (uint8)(res->size_attributes / 2);

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_EVENT_STATUS_HDR_SIZE,
                                     mandatory_data, res->size_attributes,
                                     res->attributes);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalEventUidsChangedEvent  
 *
 *DESCRIPTION
 *  Send a UIDs changed response to CT on application request.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - Notification response. 
 ****************************************************************************/
void avrcpHandleInternalEventUidsChangedEvent(AVRCP         *avrcp,
                const AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES_T     *res)
{
    uint8 mandatory[AVRCP_UID_CHANGED_EVENT_SIZE];
    mandatory[0] = avrcp_event_uids_changed;
    AVRCP_UINT16_TO_UINT8(res->uid_counter, mandatory, 1);

    sendRegisterNotificationResponse(avrcp, res->response,
                                     AVRCP_UID_CHANGED_EVENT_SIZE,
                                     mandatory, 0, 0);
}


/****************************************************************************
 *NAME    
 *    avrcpHandleInternalGetPlayStatusResponse  
 *
 *DESCRIPTION
 *  Send response for GetPlayStatus.
 *
 *PARAMETERS
 * avrcp            - AVRCP instance
 * res              - GetPlayStatus response from the application
 ****************************************************************************/
void avrcpHandleInternalGetPlayStatusResponse(AVRCP *avrcp,
                     AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *res)
{
    uint16 size_mandatory_data = 1;
    uint8 mandatory_data[9];
    uint16 param_length = 1;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        size_mandatory_data = 9;
        param_length = size_mandatory_data;
        /* Insert the mandatory data */
        convertUint32ToUint8Values(&mandatory_data[0], res->song_length);
        convertUint32ToUint8Values(&mandatory_data[4], res->song_elapsed);
        mandatory_data[8] = res->play_status;
    }

    avrcpSendMetadataResponse(avrcp,  res->response, 
                              AVRCP_GET_PLAY_STATUS_PDU_ID, 0, 
                              avrcp_packet_type_single, param_length, 
                              size_mandatory_data, mandatory_data);

}

/****************************************************************************
*NAME    
*    avrcpHandleInternalAbsoluteVolumeEvent    
*
*DESCRIPTION
*   Internal function to send the Event response for Absolute Volume
*
*PARAMETERS
* avrcp            - AVRCP instance
* res              - Absolute volume response
******************************************************************************/
void avrcpHandleInternalAbsoluteVolumeEvent(AVRCP               *avrcp,
                  const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T   *res)
{
    uint8 mandatory_data[AVRCP_DEFAULT_EVENT_DATA_SIZE]; /* 2 */

    mandatory_data[0] = avrcp_event_volume_changed;
    mandatory_data[1] = res->volume;
    
    /* Send the Notification response */
    sendRegisterNotificationResponse(avrcp, 
                                     res->response, 
                                     AVRCP_DEFAULT_EVENT_DATA_SIZE, 
                                     mandatory_data, 0, 0);
}

/****************************************************************************
*NAME    
*  avrcpHandleInternalAddressPlayerChangedEvent  
*
*DESCRIPTION
*   Internal function to send event Notification for Addressed Player Changed
*   event.
*
*PARAMETERS
* avrcp            - AVRCP instance
* res              - Address Player changed response.
******************************************************************************/
void avrcpHandleInternalAddressPlayerChangedEvent(AVRCP              *avrcp,
           const  AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES_T *res)
{
   uint8 mandatory_data[AVRCP_PLAYER_CHANGED_EVENT_SIZE]; /* 5 */

    /* Frame the packet */
    mandatory_data[0] = avrcp_event_addressed_player_changed;
    AVRCP_UINT16_TO_UINT8(res->player_id, mandatory_data, 1);
    AVRCP_UINT16_TO_UINT8(res->uid_counter, mandatory_data, 3);
 
    /* Send the Notification response */
    sendRegisterNotificationResponse(avrcp, 
                                     res->response, 
                                     AVRCP_PLAYER_CHANGED_EVENT_SIZE,
                                     mandatory_data, 0, 0);

    if(res->response == avctp_response_changed)
    {

        /* Reject all Events associated with the Current Player and clear that 
           Notifications.  All Associated events are defined in 
           Table 6.43 of AVRCP 1.4 specification  */
        avrcpRejectRegisterNotifications(avrcp, 
                              AVRCP_PLAYER_ID_ASSOCIATED_EVENTS,
                              avrcp_response_rejected_addressed_player_changed);
    }

}
  
/****************************************************************************
*NAME    
*  avrcpHandleInternalCommonEventResponse
*
*DESCRIPTION
*   Internal function to send event Notification 
*
*PARAMETERS
* avrcp            - AVRCP instance
* res              - Common event response.
******************************************************************************/
void avrcpHandleInternalCommonEventResponse(AVRCP                   *avrcp,
                                const AVRCP_INTERNAL_EVENT_COMMON_RES_T   *res)
{
    uint8 mandatory_data[AVRCP_EVENT_DEFAULT_HDR_SIZE];
    mandatory_data[0] = res->event;

    sendRegisterNotificationResponse(avrcp, res->response, 
                                    AVRCP_EVENT_DEFAULT_HDR_SIZE, 
                                    mandatory_data, 0, 0);
}

/****************************************************************************
*NAME    
*    avrcpRejectRegisterNotifications
*
*DESCRIPTION
*  Internal function to reset the Registered Notifications
*    
*PARAMETERS
*   avrcp            - Task
*   uint16           - Registered Events
*   avrcp_response_type - Response
*
*RETURN
*****************************************************************************/
void avrcpRejectRegisterNotifications(  AVRCP*               avrcp, 
                                        uint16                event_bits, 
                                        avrcp_response_type   response)
{
    uint8 id;
    uint16 events_enabled = avrcp->tg_registered_events & event_bits;
    
    for(id = 1; events_enabled; id++)
    {
        if((events_enabled >>= 1) & 0x01)
        {
           sendRegisterNotificationResponse(avrcp, response,
                                            AVRCP_EVENT_DEFAULT_HDR_SIZE,
                                            &id, 0, 0);        
        }
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/

