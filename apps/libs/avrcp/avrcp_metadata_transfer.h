/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_metadata_transfer.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_METADATA_TRANSFER_H_
#define AVRCP_METADATA_TRANSFER_H_

#include "avrcp_common.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/*************************************************************************
 * Macros map to functions 
 *************************************************************************/
#define avrcpMetadataControlCommand avrcpSendMetadataCommand
#define avrcpMetadataStatusCommand  avrcpSendMetadataCommand

/****************************************************************************
NAME
    avrcpSendMetadataCommand

DESCRIPTION
    Prepares and send Metadata command for to set the values at remote device.
*/
avrcp_status_code avrcpSendMetadataCommand(AVRCP         *avrcp, 
                                           uint8          id, 
                                           avrcpPending   pending, 
                                           uint16         extra_param_len,
                                           uint8*         extra_param,
                                           uint16         data_size, 
                                           Source         data);


/****************************************************************************
NAME    
    avrcpSendMetadataFailCfmToClient

DESCRIPTION
    Send a confirmation message to the application depending on the Metadata
    command sent.
*/
void avrcpSendMetadataFailCfmToClient(AVRCP *avrcp, avrcp_status_code status);

/****************************************************************************
NAME    
    avrcpHandleMetadataResponse

DESCRIPTION
    Handle a Metadata Transfer specific PDU response (encapsulated within a 
    vendordependent PDU).
*/
void avrcpHandleMetadataResponse(   AVRCP       *avrcp, 
                                    const uint8 *ptr, 
                                    uint16      packet_size);

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
NAME    
    avrcpHandleMetadataCommand

DESCRIPTION
    Handle a Metadata Transfer specific PDU command (encapsulated within a 
    vendordependent PDU).
*/
void avrcpHandleMetadataCommand(AVRCP       *avrcp, 
                                const uint8 *ptr, 
                                uint16      packet_size);


/****************************************************************************
NAME    
   avrcpHandleCommonMetadataControlResponse    

DESCRIPTION
  Handle AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES message
*/
void avrcpHandleCommonMetadataControlResponse(AVRCP*         avrcp,
                const AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES_T *message); 


/****************************************************************************
NAME    
    prepareMetadataStatusResponse

DESCRIPTION
    Prepare the Metadata response for a command type of STATUS.
*/
void prepareMetadataStatusResponse(AVRCP                *avrcp,
                                 avrcp_response_type    response, 
                                 uint8                  id, 
                                 uint16                 param_length, 
                                 Source                 data_list, 
                                 uint16                 size_mandatory_data, 
                                 uint8                  *mandatory_data);

/****************************************************************************
NAME    
    avrcpSendMetadataResponse

DESCRIPTION
    Prepare and send the Metadata response.
*/
void avrcpSendMetadataResponse(AVRCP                *avrcp,
                               avrcp_response_type   response,
                               uint8                 pdu_id, 
                               Source                caps_list, 
                               avrcp_packet_type     metadata_packet_type, 
                               uint16                param_length, 
                               uint16                size_mandatory_data, 
                               uint8                *mandatory_data);



/****************************************************************************
NAME
   avrcpSendRejectMetadataResponse 

DESCRIPTION
    Post a message to send reject response for the received metadata command.
*/
void avrcpSendRejectMetadataResponse(AVRCP              *avrcp, 
                                    avrcp_response_type response, 
                                    uint8               id);

/****************************************************************************
NAME
   avrcpHandleInternalRejectMetadataResponse 

DESCRIPTION
   Internal function to send metadata reject response.
*/
void avrcpHandleInternalRejectMetadataResponse(AVRCP              *avrcp, 
                                              avrcp_response_type response, 
                                              uint8               id);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_METADATA_TRANSFER_H_ */
