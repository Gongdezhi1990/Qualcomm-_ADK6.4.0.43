/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_notification.c

DESCRIPTION
This file defines the APIs for Notifications feature, which includes
    - GetPlayStatusRequest
    - RegisterNotificationRequest
    - EVENT_PLAY_STATUS_CHANGED
    - EVENT_TRACK_CHANGED
    - EVENT_TRACK_REACHED_END
    - EVENT_TRACK_REACHED_START
    - EVENT_PLAYBACK_POS_CHANGED
    - EVENT_BATTERY_STATUS_CHANGED
    - EVENT_SYSTEM_STATUS_CHANGED
    - EVENT_PLAYER_APPLICATION_SETTINGS_CHANGED   
    - EVENT_VOLUME_CHANGED
    - EVENT_ADDRESSED_PLAYER_CHANGED 

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include <source.h>
#include "avrcp_notification_handler.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
*NAME    
*    AvrcpGetPlayStatusRequest    
*
*DESCRIPTION
*  API function to request GetPlayStatus
*    
*PARAMETERS
*   avrcp            - Task
*
*RETURN
*  AVRCP_GET_PLAY_STATUS_CFM 
*****************************************************************************/
void AvrcpGetPlayStatusRequest( AVRCP *avrcp )
{
    avrcp_status_code status = avrcpMetadataStatusCommand(avrcp, 
                                                AVRCP_GET_PLAY_STATUS_PDU_ID,
                                                avrcp_get_play_status, 
                                                0, 0, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendGetPlayStatusCfm(avrcp, status, 0, 0, 0, 0);
    }
}


/****************************************************************************
 *NAME    
 *    AvrcpRegisterNotificationRequest    
 *
 *DESCRIPTION
 *  CT shall use this API function to register Event Notifications 
 *
 *PARAMETERS
 * avrcp            -   AVRCP Entity
 * event_id         -   Event to be registered with TG for notifications.
 * playback_interval-   Only applicable to EVENT_PLAYBACK_POS_CHANGED 
 *
 *MESSAGE RETURNED
 * EVENT_IND messages on Event notifications from TG.
*****************************************************************************/
void AvrcpRegisterNotificationRequest(AVRCP                    *avrcp, 
                               avrcp_supported_events   event_id, 
                               uint32                   playback_interval)
{
    uint8 params[5];
    avrcp_status_code status;

    params[0] = event_id & 0xFF;
    convertUint32ToUint8Values(&params[1], playback_interval);

    status = avrcpMetadataStatusCommand(avrcp, 
                                        AVRCP_REGISTER_NOTIFICATION_PDU_ID, 
                                        event_id + avrcp_events_start_dummy, 
                                        sizeof(params), params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendRegisterNotificationFailCfm(avrcp, status, event_id);
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
*NAME    
*    AvrcpGetPlayStatusResponse    
*
*DESCRIPTION
*  API function to respond to the AVRCP_GET_PLAY_STATUS_IND message at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - avctp_response_stable on success
*   song_length - The total length of the playing song in milliseconds.
*   song_elapsed- The current position of the playing song in
*                 milliseconds elapsed.    
*   play_status - Current status of playing media.
*                       
*
*RETURN
*  AVRCP_GET_PLAY_STATUS_CFM 
*****************************************************************************/

void AvrcpGetPlayStatusResponse(AVRCP               *avrcp, 
                                avrcp_response_type response, 
                                uint32              song_length, 
                                uint32              song_elapsed, 
                                avrcp_play_status   play_status)
{
    /* Only send a response if the command was received. */
    if (avrcp->bitfields.block_received_data == avrcp_get_play_status)
    {
        sendPlayStatusResponse(avrcp, response, song_length,
                               song_elapsed, play_status);
    }
    else
    {
        AVRCP_INFO(("AvrcpGetPlayStatusResponse: CT not waiting for response\n"));
    }
}

/****************************************************************************
*NAME    
*    AvrcpEventPlaybackStatusChangedResponse    
*
*DESCRIPTION
*  API function to notify Play back status change event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*   play_status - Current status of playing media.
*
*RETURN
*****************************************************************************/
void AvrcpEventPlaybackStatusChangedResponse(AVRCP              *avrcp,
                                            avrcp_response_type response, 
                                            avrcp_play_status   play_status)
{
    /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_PLAYBACK_STATUS_CHANGED))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES);
        message->response = response;
        message->play_status = play_status;
        MessageSend(&avrcp->task, 
                   AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES, message);
    }
    else
    {
        AVRCP_INFO(("AvrcpEventPlaybackStatusChangedResponse: "
               "Event not registered\n"));
    }
}


/****************************************************************************
*NAME    
*    AvrcpEventTrackChangedResponse    
*
*DESCRIPTION
*  API function to notify Track change event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*  track_index_high - Top 4 bytes of the currently selected track.
*  track_index_low  - Bottom 4 bytes of the currently selected track. 
*
*RETURN
*****************************************************************************/

void AvrcpEventTrackChangedResponse(AVRCP               *avrcp, 
                                    avrcp_response_type response, 
                                    uint32              track_index_high, 
                                    uint32              track_index_low)
{
    /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_TRACK_CHANGED))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES);
        message->response = response;
        message->track_index_high = track_index_high;
        message->track_index_low = track_index_low;
        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES, message);
    }
    else
    {
        AVRCP_INFO(("AvrcpEventTrackChangedResponse: Event not registered\n"));
    }
}


/****************************************************************************
*NAME    
*    AvrcpEventTrackReachedEndResponse    
*
*DESCRIPTION
*  API function to notify End of Track event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*
*RETURN
*****************************************************************************/

void AvrcpEventTrackReachedEndResponse(AVRCP *avrcp, 
                                       avrcp_response_type response)
{
    avrcpSendEventResponse(avrcp, response, EVENT_TRACK_END); 
}


/****************************************************************************
*NAME    
*    AvrcpEventTrackReachedStartResponse    
*
*DESCRIPTION
*  API function to notify Start of Track event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*
*RETURN
*****************************************************************************/

void AvrcpEventTrackReachedStartResponse(AVRCP               *avrcp, 
                                         avrcp_response_type response)
{
    avrcpSendEventResponse(avrcp, response, EVENT_TRACK_START);
}


/****************************************************************************
*NAME    
*    AvrcpEventTrackReachedStartResponse    
*
*DESCRIPTION
*  API function to notify playback position change event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*   playback_pos - Current playback position in milliseconds.
*
*RETURN
*****************************************************************************/

void AvrcpEventPlaybackPosChangedResponse(AVRCP *avrcp, 
                                          avrcp_response_type response, 
                                          uint32 playback_pos)
{
    /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_PLAYBACK_POS_CHANGED))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES);
        message->response = response;
        message->playback_pos = playback_pos;
        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES, 
                    message);
    }
    else
    {
        AVRCP_INFO(("AvrcpEventPlaybackPosChangedResponse:"
                    "Event not registered\n"));
    }
}

/****************************************************************************
*NAME    
*    AvrcpEventBattStatusChangedResponse    
*
*DESCRIPTION
*  API function to notify battery status change event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*   battery_status - Current battery status
*
*RETURN
*****************************************************************************/
void AvrcpEventBattStatusChangedResponse(AVRCP *avrcp, 
                                        avrcp_response_type  response, 
                                        avrcp_battery_status battery_status)
{
    /* Only send a response if this event was registered by the CT. */
    if(isEventRegistered(tg, avrcp, EVENT_BATTERY_STATUS_CHANGED))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES);
        message->response = response;
        message->battery_status = battery_status;
        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES, message);
    }
    else
    {
        AVRCP_INFO(("AvrcpEventBattStatusChangedResponse:"
                    "Event not registered\n"));
    }
}

/****************************************************************************
*NAME    
*    AvrcpEventSystemStatusChangedResponse    
*
*DESCRIPTION
*  API function to notify battery status change event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*   battery_status - Current battery status
*
*RETURN
*****************************************************************************/
void AvrcpEventSystemStatusChangedResponse(AVRCP *avrcp, 
                                           avrcp_response_type response, 
                                           avrcp_system_status system_status)
{
    /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_SYSTEM_STATUS_CHANGED))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES);
        message->response = response;
        message->system_status = system_status;
        MessageSend(&avrcp->task,
                    AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES, message);
    }
    else
    {
        AVRCP_INFO(("AvrcpEventSystemStatusChangedResponse:"
                    "Event not registered\n"));
    }
}

/****************************************************************************
*NAME    
*    AvrcpEventPlayerAppSettingChangedResponse    
*
*DESCRIPTION
*  API function to notify player app setting change event at TG.
*    
*PARAMETERS
*   avrcp       - Task
*   response    - On success , avctp_response_interim for intermediate response
*                 and avctp_response_changed for final response.
*   size_attributes - The length of the supplied attribute data (in bytes)
*   attributes -  The list of attribute data returned in the response.
*
*RETURN
*****************************************************************************/
void AvrcpEventPlayerAppSettingChangedResponse(AVRCP    *avrcp, 
                                              avrcp_response_type response, 
                                              uint16    size_attributes, 
                                              Source    attributes)
{
    /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_PLAYER_SETTING_CHANGED))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES);
        message->response = response;
        message->size_attributes = size_attributes;
        message->attributes = attributes;
        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES,
                    message);
    }
    else
    {
        SourceEmpty( attributes );
        AVRCP_INFO(("AvrcpEventPlayerAppSettingChangedResponse: "
                "Event not registered\n"));
    }
}
#endif /* !AVRCP_CT_ONLY_LIB*/

