/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_notification_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_NOTIFICATION_HANDLER_H_
#define AVRCP_NOTIFICATION_HANDLER_H_

#include "avrcp_metadata_transfer.h"

/* Macros */
#define clearRegisterNotification(ct_or_tg, avrcp, event) \
            avrcp->ct_or_tg##_registered_events &= (uint16)~(1 << (event))

#define registerEventNotification(ct_or_tg, avrcp, event) \
            avrcp->ct_or_tg##_registered_events |= (uint16)(1<<(event))


#define isEventRegistered(ct_or_tg, avrcp, event) \
            (avrcp->ct_or_tg##_registered_events & (1<< (event)))

#define GetNotificationTransaction(ct_or_tg, avrcp, event) \
            (avrcp->ct_or_tg##_notify_transaction_label[(event)-1])

#define SetNotificationBit(event_type) \
            ((uint16)(1 << ((event_type) - avrcp_events_start_dummy)))
        
/* Events associated with the current player are set (bits 1..5 and bits 8..9) 
   and other bits are reset. Bit 0 is unused and ignored always.*/
#define AVRCP_PLAYER_ID_ASSOCIATED_EVENTS         0x033E

/* Set Bit 12 for clearing event uids */
#define AVRCP_BROWSED_PLAYER_ASSOCIATED_EVENTS    0x1000

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/***************************************************************************
NAME    
    avrcpSendNotification

DESCRIPTION
    Send indication of notification event up to the app.
*/
void avrcpSendNotification( AVRCP *avrcp,  
                            avrcp_response_type response, 
                            const uint8* ptr, 
                            uint16 packet_size);

/****************************************************************************
NAME    
    avrcpSendRegisterNotificationFailCfm

DESCRIPTION
    Send an AVRCP_REGISTER_NOTIFICATION_CFM failure message to the client.
*/
void avrcpSendRegisterNotificationFailCfm(AVRCP *avrcp, 
                                         avrcp_status_code status,
                                         avrcp_supported_events event_id);


/****************************************************************************
NAME    
    avrcpSendGetPlayStatusCfm

DESCRIPTION
    Send an AVRCP_GET_PLAY_STATUS_CFM message to the client.
*/
void avrcpSendGetPlayStatusCfm(AVRCP *avrcp, 
                            avrcp_status_code status, 
                            uint32 song_length, 
                            uint32 song_elapsed, 
                            avrcp_play_status play_status, 
                            uint8 transaction); 


/****************************************************************************
NAME    
    avrcpHandleGetPlayStatusResponse

DESCRIPTION
    Handle Get Play Status response received from the TG.
*/
void avrcpHandleGetPlayStatusResponse(AVRCP *avrcp, 
                                      avrcp_status_code status,
                                      uint8 transaction, 
                                      const uint8 *ptr, 
                                      uint16 packet_size);

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
NAME    
    avrcpHandleRegisterNotificationCommand

DESCRIPTION
    Handle Register Notification command received from the CT.
*/
void avrcpHandleRegisterNotificationCommand(AVRCP *avrcp, const uint8 *ptr);


/****************************************************************************
NAME    
    avrcpHandleInternalEventPlaybackStatusChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventPlaybackStatusChangedResponse(AVRCP *avrcp,
              const AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventTrackChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES.
*/
void avrcpHandleInternalEventTrackChangedResponse(AVRCP *avrcp,
               const AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T *res);



/****************************************************************************
NAME    
    avrcpHandleInternalEventPlaybackPosChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES.
*/
void avrcpHandleInternalEventPlaybackPosChangedResponse(AVRCP *avrcp,
              const AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventBattStatusChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventBattStatusChangedResponse(AVRCP *avrcp,
              const AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventSystemStatusChangedResponse

DESCRIPTION
    Handle internal message AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES.
*/
void avrcpHandleInternalEventSystemStatusChangedResponse(AVRCP *avrcp,
              const AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInternalEventPlayerAppSettingChangedResponse

DESCRIPTION
   Handle internal message AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES.
*/
void avrcpHandleInternalEventPlayerAppSettingChangedResponse(AVRCP *avrcp,
             const AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T *res);

/****************************************************************************
NAME    
    sendRegisterNotificationResponse

DESCRIPTION
    Send Register Notification response. 
*/
void sendRegisterNotificationResponse(AVRCP *avrcp, 
                                avrcp_response_type response, 
                                uint16 size_mandatory, 
                                uint8 *mandatory, 
                                uint16 size_attributes, 
                                Source attributes);


/****************************************************************************
NAME    
    avrcpHandleInternalGetPlayStatusResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_GET_PLAY_STATUS_RES message.
*/
void avrcpHandleInternalGetPlayStatusResponse(AVRCP *avrcp,
               AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T *res);

/****************************************************************************
NAME    
    avrcpHandleInternalAbsoluteVolumeEvent

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES message.
*/
void avrcpHandleInternalAbsoluteVolumeEvent(AVRCP                *avrcp,
                    const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T  *res);

/****************************************************************************
NAME    
   avrcpHandleInternalAddressPlayerChangedEvent

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES
*/
void avrcpHandleInternalAddressPlayerChangedEvent(AVRCP              *avrcp,
           const  AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES_T *res);

/****************************************************************************
NAME    
    avrcpRejectRegisterNotifications

DESCRIPTION
    Internal handler to reject a group of registered notifications
*/
void avrcpRejectRegisterNotifications(  AVRCP*               avrcp, 
                                        uint16                event_bits, 
                                        avrcp_response_type   response);

/****************************************************************************
NAME    
  avrcpHandleInternalCommonEventResponse

DESCRIPTION
   Internal function to send event Notification 
*/
void avrcpHandleInternalCommonEventResponse(AVRCP                   *avrcp,
                                const AVRCP_INTERNAL_EVENT_COMMON_RES_T   *res);

/****************************************************************************
NAME    
  avrcpHandleInternalEventUidsChangedEvent

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES
*/


void avrcpHandleInternalEventUidsChangedEvent(AVRCP         *avrcp,
                const AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES_T     *res);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_NOTIFICATION_HANDLER_H_ */

