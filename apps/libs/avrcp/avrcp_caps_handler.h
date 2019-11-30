/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_caps_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_CAPS_HANDLER_H_
#define AVRCP_CAPS_HANDLER_H_

#include "avrcp_common.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
NAME    
    avrcpSendGetCapsCfm

DESCRIPTION
    Send a successful AVRCP_GET_CAPS_CFM message to the client task.
*/
void avrcpSendGetCapsCfm(   AVRCP               *avrcp, 
                            avrcp_status_code   status, 
                            uint16              metadata_packet_type, 
                            avrcp_capability_id caps_id, 
                            uint16              number_of_caps, 
                            uint16              data_length, 
                            Source              source);

/****************************************************************************
NAME    
    avrcpHandleGetCapsResponse

DESCRIPTION
    Handle a Get Caps response PDU arriving from the TG.
*/
void  avrcpHandleGetCapsResponse(AVRCP               *avrcp, 
                                avrcp_response_type response, 
                                uint16              meta_packet_type, 
                                const uint8*        data,
                                uint16              packet_size);

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
NAME    
    avrcpHandleInternalGetCapsResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_GET_CAPS_RES message.
*/
void avrcpHandleInternalGetCapsResponse(AVRCP                         *avrcp,
                                        AVRCP_INTERNAL_GET_CAPS_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleGetCapsCommand

DESCRIPTION
    Handle a Get Caps command PDU arriving from the CT.
*/
void avrcpHandleGetCapsCommand(AVRCP *avrcp, avrcp_capability_id caps);
#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_CAPS_HANDLER_H_ */
