/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_element_attributes_handler.h
    
DESCRIPTION
    This Header file declares all function prototypes for 
    GetElementAttributes Feature
    
*/

#ifndef AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_
#define AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_

#include "avrcp_common.h"

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
NAME    
    avrcpHandleGetElementAttributesCommand

DESCRIPTION
    Handle Get Element attributes command PDU received from CT.
*/
void avrcpHandleGetElementAttributesCommand(AVRCP           *avrcp, 
                                            const uint8*    data, 
                                            uint16          packet_size);



/****************************************************************************
NAME    
    avrcpHandleInternalGetElementAttributesResponse

DESCRIPTION
    Respond to an AVRCP_ELEMENT_ATTRIBUTES_IND message.
*/
void avrcpHandleInternalGetElementAttributesResponse(AVRCP *avrcp, 
                AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T *res);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_ELEMENT_ATTRIBUTES_HANDLER_H_ */
