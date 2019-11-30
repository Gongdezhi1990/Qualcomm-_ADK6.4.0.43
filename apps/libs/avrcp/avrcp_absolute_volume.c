/****************************************************************************
Copyright (c) 2009 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_absolute_volume.c

DESCRIPTION
This file defines the API Functions and internal functions for handling 
Absolute Volume Feature. Absolute Volume feature is a Mandatory feature for
version 1.4 category 2 devices.

Controller API functions are 
    -AvrcpSetAbsoluteVolumeRequest

Target API functions are 
    - AvrcpSetAbsoluteVolumeResponse
    - AvrcpEventVolumeChangedResponse

Controller Event Messages are 
    - AVRCP_EVENT_VOLUME_CHANGED_IND
    - AVRCP_SET_ABSOLUTEVOLUME_CFM

Target Event Messages are
    - AVRCP_SET_ABSOLUTEVOLUME_IND     

NOTES

*/

/****************************************************************************
    Header files
*/
#include <panic.h>
#include "avrcp_absolute_volume.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
 
/****************************************************************************
*NAME    
*    AvrcpSetAbsoluteVolumeRequest    
*
*DESCRIPTION
*  API function to send SetAbsoluteVolume Request from CT.
*    
*PARAMETERS
*   avrcp            - Task
*   uint8            - Requested volume change.0x0 as Minimum and 0x7F as Max 
*
*RETURN
*******************************************************************************/
void  AvrcpSetAbsoluteVolumeRequest( AVRCP *avrcp, uint8  volume)
{
    avrcp_status_code status;

    /* volume should not be more than the maximum allowed volume */
    if(volume  >  AVRCP_MAX_VOL_MASK)
    {
        volume = AVRCP_MAX_VOL_MASK;
    }

    status = avrcpMetadataControlCommand( avrcp, 
                                    AVRCP_SET_ABSOLUTE_VOLUME_PDU_ID,
                                    avrcp_absolute_volume,1/*sizeof(volume)*/,
                                    &volume, 0, 0);

    /* Failure confirmation if the request got failed */
    if(status != avrcp_success)
    {
        avrcpSendAbsoluteVolumeCfm(avrcp, status, &volume);
    }
}

/****************************************************************************
*NAME    
*    avrcpSendAbsoluteVolumeCfm    
*
*DESCRIPTION
*   Internal function to send result after completing the operation of 
*   SetAbsoluteVolume request.
*      
*PARAMETERS
*   avrcp                   - Task
*   avrcp_status_code       - status
*   data                    - pointer to parameters
*
*RETURN
******************************************************************************/
void avrcpSendAbsoluteVolumeCfm(AVRCP               *avrcp,
                                avrcp_status_code   status,
                                const uint8         *data)
{
    MAKE_AVRCP_MESSAGE(AVRCP_SET_ABSOLUTE_VOLUME_CFM);

    message->avrcp = avrcp; 
    message->status = status; 
    message->volume=0; 

    if(!(status & AVRCP_ERROR_STATUS_BASE) && data)
    {
        /* First octet in the data contains the Volume */
        message->volume = data[0];
    }

    MessageSend(avrcp->clientTask, AVRCP_SET_ABSOLUTE_VOLUME_CFM, message);
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only Lib */

/****************************************************************************
*NAME    
*    AvrcpSetAbsoluteVolumeResponse
*
*DESCRIPTION
*  API function to respond to SetAbsoluteVolume request. TG application shall
*  call function on receiving AVRCP_SET_ABSOLUTE_VOLUME_IND. If the volume 
*  got changed due to this procedure, Application must call 
*  AvrcpEventVolumeChangedResponse() if CT has registered for 
*  EVENT_VOLUME_CHANGED notification.
*    
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Expected responses are 
*                             avctp_response_accepted or avctp_response_rejected
*                             or avrcp_response_rejected_internal_error.
*   uint8                   - Volume at TG. 0x0 as Minimum and 0x7F as Max 
*
*RETURN
*******************************************************************************/
void AvrcpSetAbsoluteVolumeResponse(AVRCP               *avrcp, 
                                    avrcp_response_type response, 
                                    uint8               volume) 
{

    if (avrcp->bitfields.block_received_data == avrcp_absolute_volume)
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES); 

        message->volume=(volume < AVRCP_MAX_VOL_MASK)?volume:AVRCP_MAX_VOL_MASK;
        message->response=response;
        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES,
                    message);
    }
    else
    {
        AVRCP_INFO(("AvrcpSetAbsoluteVolumeResponse: CT is not waiting for"
                     " the response\n"));  
    }
} 

/****************************************************************************
*NAME    
*   AvrcpEventVolumeChangedResponse 
*
*DESCRIPTION
*  TG shall use this API function to send volume change events if CT has
*  Registered for EVENT_VOLUME_CHANGED notifications. TG shall send an interim
*  response immediately after receiving a AVRCP_REGISTER_NOTIFICATION_IND  for 
*  EVENT_VOLUME_CHANGED from the CT before sending the final response on Volume 
*  change.  Otherwise lib will reject the Register Notification request from CT
*  after timeout of 1000ms. 
* 
*    
*PARAMETERS
*   avrcp                   - Task
*   avrcp_response_code     - response.  Response indicating whether the volume
*                             has been changed or the notification was 
*                             rejected. 
*                             Expected response values avctp_response_changed, 
*                             avrcp_response_interim or avctp_response_rejected.
*   uint8                   - Volume at TG. 0x0 as Minimum and 0x7F as Max 
*
*RETURN
*******************************************************************************/
void AvrcpEventVolumeChangedResponse(AVRCP               *avrcp, 
                                     avrcp_response_type response,
                                     uint8               volume)
{
      /* Only send a response if this event was registered by the CT. */
    if (isEventRegistered(tg, avrcp, EVENT_VOLUME_CHANGED))
    {
        /* Internal Event response */
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES); 

        message->volume=(volume < AVRCP_MAX_VOL_MASK)?volume:AVRCP_MAX_VOL_MASK;
        message->response=response;
        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES,
                    message);

    }
    else
    {
       AVRCP_INFO(("AvrcpEventVolumeChangedResponse: " 
                    "Event not registered\n"));
    }
}

/****************************************************************************
*NAME    
*    avrcpHandleSetAbsoluteVolumeCommand    
*
*DESCRIPTION
*   Internal function at TG to handle incoming SetAbsoluteVolume request 
    from CT.  
*      
*PARAMETERS
*   avrcp                   - Task
*   data                    - volume
*
*RETURN
******************************************************************************/
void avrcpHandleSetAbsoluteVolumeCommand(AVRCP *avrcp, uint8 volume)
{
    volume &= AVRCP_MAX_VOL_MASK;

    {
        /* Send an Indication to the application */
        MAKE_AVRCP_MESSAGE(AVRCP_SET_ABSOLUTE_VOLUME_IND); 
        message->avrcp = avrcp; 
        message->volume=volume;
        MessageSend(avrcp->clientTask,AVRCP_SET_ABSOLUTE_VOLUME_IND,message);
    }
}
        

/****************************************************************************
*NAME    
*    avrcpHandleInternalAbsoluteVolumeRsp    
*
*DESCRIPTION
*   Internal function at TG to send the notification for EVENT_VOLUME_CHANGED
*      
*PARAMETERS
*   avrcp                   - Task
*   res                     - response
*
*RETURN
******************************************************************************/
void avrcpHandleInternalAbsoluteVolumeRsp(AVRCP                 *avrcp,
                  const AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T   *res)
{
    uint8 mandatory_data[AVRCP_ERROR_CODE_SIZE]; /* 1 byte */
    avrcp_response_type response = res->response;

   /* Get the error status code */
    mandatory_data[0] = avrcpGetErrorStatusCode(&response,AVRCP0_CTYPE_CONTROL);

    if(response != avctp_response_rejected)
    {
        /* Not failure. Insert Volume instead */
        mandatory_data[0] = res->volume;
    }    
    
    /* Send a response to this PDU now. */
    avrcpSendMetadataResponse(avrcp, response,
                              AVRCP_SET_ABSOLUTE_VOLUME_PDU_ID,
                              0, avrcp_packet_type_single,
                              AVRCP_ERROR_CODE_SIZE , AVRCP_ERROR_CODE_SIZE,
                              mandatory_data);
}
#endif /* !AVRCP_CT_ONLY_LIB*/
