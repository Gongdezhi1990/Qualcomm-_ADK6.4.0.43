/****************************************************************************
Copyright (c) 2009 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    avrcp_mediaplayer.c

DESCRIPTION
This file defines the API Functions and internal functions for handling 
the Media Player Selection feature. Media player selection is an optional
feature for Controller. This is a Mandatory feature for Target devices
that supports Browsing.


Controller API functions are 
    -AvrcpSetAddressedPlayerRequest

Target API functions are 
    - AvrcpSetAddressedPlayerResponse
    - AvrcpEventAddressedPlayerChangedResponse
    - AvrcpEventAvailablePlayerChangedResponse

Controller Event Messages are 
    - AVRCP_SET_ADDRESSED_PLAYER_CFM
    - AVRCP_EVENT_ADDRESSED_PLAYER_CHANGED_IND
    - AVRCP_EVENT_AVAILABLE_PLAYERS_CHANGED_IND

Target Event Messages are
    - AVRCP_SET_ADDRESSED_PLAYER_IND     
*/

/****************************************************************************
    Header files
*/
#include <panic.h>
#include "avrcp_mediaplayer.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"
#include "avrcp_send_response.h"
#include "avrcp_init.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
*NAME    
*   AvrcpSetAddressedPlayerRequest    
*
*DESCRIPTION
*  API function to send SetAddressedPlayer to the Target .
*    
*PARAMETERS
*   avrcp            - Task
*   uint16           - Unique Player ID
*
*RETURN
*  AVRCP_SET_ADDRESSED_PLAYER_CFM 
*****************************************************************************/
void AvrcpSetAddressedPlayerRequest( AVRCP*  avrcp, uint16  player_id )
{
    uint8 player[AVRCP_PLAYER_ID_SIZE];
    avrcp_status_code status;

    /* Convert to uint8 values */
    AVRCP_UINT16_TO_UINT8(player_id, player, 0);

    status = avrcpMetadataControlCommand(avrcp, 
                                         AVRCP_SET_ADDRESSED_PLAYER_PDU_ID,
                                         avrcp_set_addressed_player,
                                         AVRCP_PLAYER_ID_SIZE, 
                                         player, 0, 0);

    /* Failure confirmation if the request got failed */
    if(status != avrcp_success)
    {
        avrcpSendAddressedPlayerCfm(avrcp, status);
    }

}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME    
*   AvrcpSetAddressedPlayerResponse
*
*DESCRIPTION
*  API function to respond to SetAddressedPlayer request. TG application shall
*  call function on receiving AVRCP_SET_ADDRESSED_PLAYER_IND. if the addressed
*  player at TG changed due to this procedure, application must call 
*  AvrcpEventAddressedPlayerChangedResponse() before calling this API , 
*  if CT has registered  for EVENT_ADDRESSED_PLAYER_CHANGED Notification.
*    
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Expected responses are 
*                             avctp_response_accepted, avctp_response_rejected,
*                             avrcp_response_rejected_internal_error,  
*                             avrcp_response_rejected_invalid_player_id or 
*                             avrcp_response_rejected_no_available_players
*RETURN
*   None
*******************************************************************************/

void AvrcpSetAddressedPlayerResponse(AVRCP*              avrcp,
                                     avrcp_response_type response)
{

    if (avrcp->bitfields.block_received_data == avrcp_set_addressed_player)
    {
        avrcpSendMetaDataControlResponse(avrcp, 
                                        response, 
                                        AVRCP_SET_ADDRESSED_PLAYER_PDU_ID);
        
        clearRegisterNotification(tg, avrcp, EVENT_ADDRESSED_PLAYER_CHANGED);
    }
    else
    {
        AVRCP_INFO(("AvrcpSetAddressedPlayerResponse: CT is not waiting for"
                     " the response\n"));  
    }
}

/****************************************************************************
*NAME    
*   AvrcpEventAddressedPlayerChangedResponse
*
*DESCRIPTION
*  API function to notify CT on addressed player change at the TG. TG shall send
*  an interim response immediately with the current player id on receiving a 
*  AVRCP_REGISTER_NOTIFICATION_IND  for  avrcp_event_addressed_player_changed
*  from the CT before sending the final response.  
* 
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Expected responses are 
*                             avctp_response_changed, avctp_response_interim, 
*                              and avctp_response_rejected.
*   player_id               - The changed player ID for avctp_response_changed,
*                             current player id for avctp_response_interim.
*   uid_counter             - Current Non-Zero UID counter for database aware 
*                             players. 
*   
*RETURN
*   None
*******************************************************************************/
void AvrcpEventAddressedPlayerChangedResponse(AVRCP*    avrcp, 
                                    avrcp_response_type response, 
                                    uint16              player_id, 
                                    uint16              uid_counter)
{
      /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_ADDRESSED_PLAYER_CHANGED))
    {
        /* Internal Event response */
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES); 

        message->response=response;
        message->player_id = player_id;

        if(isAvrcpDatabaseEnabled(avrcpGetDeviceTask()))
        {
            message->uid_counter = uid_counter;
        }
        else
        {
            message->uid_counter = 0;
        }

        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES,
                    message);

    }
    else
    {
       AVRCP_INFO(("AvrcpEventAddressedPlayerChangedResponse: " 
                    "Event not registered\n"));
    }
}

/****************************************************************************
*NAME    
*   AvrcpEventAvailablePlayersChangedResponse
*
*DESCRIPTION
*  API function to notify CT on available players change at the TG. 
*  TG shall send an interim response immediately on receiving a
*  AVRCP_REGISTER_NOTIFICATION_IND for avrcp_event_available_players_changed,
*  if the final response requires more time.
* 
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Expected responses are 
*                             avctp_response_changed, avctp_response_interim, 
*                              and avctp_response_rejected.
*RETURN
*   None
*******************************************************************************/
void AvrcpEventAvailablePlayersChangedResponse( AVRCP*              avrcp,
                                                avrcp_response_type response) 
{
    avrcpSendEventResponse(avrcp, response, EVENT_AVAILABLE_PLAYERS_CHANGED);
}

#endif /* !AVRCP_CT_ONLY_LIB*/

