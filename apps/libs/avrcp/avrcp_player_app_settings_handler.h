/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_player_app_settings_handler.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_PLAYER_APP_SETTINGS_HANDLER_H_
#define AVRCP_PLAYER_APP_SETTINGS_HANDLER_H_


#include "avrcp_private.h"

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
NAME    
    avrcpHandleListAppAttributesCommand

DESCRIPTION
    Handle List App Attributes command PDU received from CT.
*/
void avrcpHandleListAppAttributesCommand(AVRCP *avrcp);



/****************************************************************************
NAME    
    avrcpHandleInternalListAppAttributesResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES message.
*/
void avrcpHandleInternalListAppAttributesResponse(AVRCP *avrcp,
                       AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleListAppValuesCommand

DESCRIPTION
    Handle List App Values command PDU received from CT.
*/
void avrcpHandleListAppValuesCommand(AVRCP *avrcp, uint16 attribute_id);



/****************************************************************************
NAME    
    avrcpHandleInternalListAppValuesResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_LIST_APP_VALUE_RES message.
*/
void avrcpHandleInternalListAppValuesResponse(AVRCP *avrcp, 
                AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleGetAppValuesCommand

DESCRIPTION
    Handle Get App Values command PDU received from CT.
*/
void avrcpHandleGetAppValuesCommand(AVRCP *avrcp,
                                    const uint8* ptr, 
                                    uint16 packet_size);




/****************************************************************************
NAME    
    avrcpHandleInternalGetAppValueResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_GET_APP_VALUE_RES message.
*/
void avrcpHandleInternalGetAppValueResponse(AVRCP *avrcp,
                AVRCP_INTERNAL_GET_APP_VALUE_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleSetAppValuesCommand

DESCRIPTION
    Handle Set App Values command PDU received from CT.
*/
void avrcpHandleSetAppValuesCommand(AVRCP *avrcp,
                                    const uint8* ptr, 
                                    uint16 packet_size);


/****************************************************************************
NAME    
    avrcpHandleInternalSetAppValueResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_SET_APP_VALUE_RES message.
*/
void avrcpHandleInternalSetAppValueResponse(AVRCP *avrcp, 
                AVRCP_INTERNAL_SET_APP_VALUE_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleGetAppAttributeTextCommand

DESCRIPTION
    Handle Get App Attribute Text command PDU received from CT.
*/
void avrcpHandleGetAppAttributeTextCommand(AVRCP *avrcp,  
                                  const uint8* ptr, 
                                  uint16 packet_size);



/****************************************************************************
NAME    
    avrcpHandleInternalGetAppAttributeTextResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES message.
*/
void avrcpHandleInternalGetAppAttributeTextResponse(AVRCP *avrcp,
                      AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleGetAppValueTextCommand

DESCRIPTION
    Handle Get App Value Text command PDU received from CT.
*/
void avrcpHandleGetAppValueTextCommand(AVRCP *avrcp,
                                      const uint8* ptr, 
                                      uint16 packet_size);


/****************************************************************************
NAME    
    avrcpHandleInternalGetAppValueTextResponse

DESCRIPTION
    Internal handler for the AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES message.
*/
void avrcpHandleInternalGetAppValueTextResponse(AVRCP *avrcp, 
              AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *res);


/****************************************************************************
NAME    
    avrcpHandleInformBatteryStatusCommand

DESCRIPTION
    Handle a InformBatteryStatusOfCT PDU command received from the CT.
*/
void avrcpHandleInformBatteryStatusCommand(AVRCP *avrcp, uint8 battery_status);


/****************************************************************************
NAME    
    avrcpHandleInformCharSetCommand

DESCRIPTION
    Process the command PDU that has arrrived from the CT.
*/
void avrcpHandleInformCharSetCommand(AVRCP *avrcp,  
                                     const uint8* ptr, 
                                     uint16 packet_size);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_PLAYER_APP_SETTINGS_HANDLER_H_ */
