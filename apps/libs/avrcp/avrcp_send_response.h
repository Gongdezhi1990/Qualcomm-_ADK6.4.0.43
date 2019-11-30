/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_send_response.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_SEND_RESPONSE_H_
#define AVRCP_SEND_RESPONSE_H_

#include "avrcp_private.h"

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
NAME    
    sendPassThroughResponse

DESCRIPTION
    This function is called to send a PassThrough response.
*/
void sendPassThroughResponse(AVRCP *avrcp, avrcp_response_type response);


/****************************************************************************
NAME    
    sendUnitInfoResponse

DESCRIPTION
    This function is called to send a UnitInfo response.
*/
void sendUnitInfoResponse(AVRCP *avrcp, bool accept, 
                          avc_subunit_type unit_type, uint8 unit,
                          uint32 company_id);


/****************************************************************************
NAME    
    sendSubunitInfoResponse

DESCRIPTION
    This function is called to send a SubunitInfo response.
*/
void sendSubunitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data);


/****************************************************************************
NAME    
    sendVendorDependentResponse

DESCRIPTION
    This function is called to send a VendorDependent response.
*/
void sendVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response);



/****************************************************************************
NAME    
    avrcpSendGetCapsResponse

DESCRIPTION
    This function is called to send an GetCaps response.
*/
void avrcpSendGetCapsResponse(AVRCP *avrcp, avrcp_response_type response,
                             avrcp_capability_id caps, uint16 size_caps_list,
                             Source caps_list);



/****************************************************************************
NAME    
    sendGetElementsResponse

DESCRIPTION
    This function is called to send an GetElements response.
*/
void sendGetElementsResponse(AVRCP *avrcp, avrcp_response_type response,
                             uint16 number_of_attributes, 
                             uint16 size_attributes, Source attributes);


/****************************************************************************
NAME    
    sendtGroupResponse

DESCRIPTION
    This function is called to send a Group response.
*/
void sendGroupResponse(AVRCP *avrcp, avrcp_response_type response);


/****************************************************************************
NAME    
    sendPlayStatusResponse

DESCRIPTION
    This function is called to send a PlayStatus response.
*/
void sendPlayStatusResponse(AVRCP *avrcp, avrcp_response_type response,
                            uint32 song_length, uint32 song_elapsed,
                            avrcp_play_status play_status);


/****************************************************************************
NAME    
    sendListAttrResponse

DESCRIPTION
    This function is called to send a ListAttribute response.
*/
void sendListAttrResponse(AVRCP *avrcp, avrcp_response_type response,
                         uint8 size_attributes, Source attributes);


/****************************************************************************
NAME    
    sendListValuesReject

DESCRIPTION
    This function is called to send a ListValues response.
*/
void sendListValuesResponse(AVRCP *avrcp, avrcp_response_type response, 
                            uint8 size_values, Source values);


/****************************************************************************
NAME    
    sendGetValuesResponse

DESCRIPTION
    This function is called to send a GetValues response.
*/
void sendGetValuesResponse(AVRCP *avrcp, avrcp_response_type response,
                           uint16 size_values, Source values);


/****************************************************************************
NAME    
    sendSetValuesResponse

DESCRIPTION
    This function is called to send a SetValues response.
*/
void sendSetValuesResponse(AVRCP *avrcp, avrcp_response_type response);


/****************************************************************************
NAME    
    sendGetAttributeTextResponse

DESCRIPTION
    This function is called to send a GetAttributeText response.
*/
void sendGetAttributeTextResponse(AVRCP *avrcp, avrcp_response_type response,
                                  uint8 number_of_attributes, 
                                  uint16 size_attributes, Source attributes);


/****************************************************************************
NAME    
    sendGetValueTextResponse

DESCRIPTION
    This function is called to send a GetValueText response.
*/
void sendGetValueTextResponse(AVRCP *avrcp, avrcp_response_type response, 
                              uint8 number_of_values, uint16 size_values, 
                              Source values);

/*************************************************************************
* NAME
*   avrcpSendEventResponse
*
*  Description
*       Send a Common Event Response
****************************************************************************/
void avrcpSendEventResponse(AVRCP                   *avrcp,
                           avrcp_response_type      response,
                           avrcp_supported_events   event); 

/*************************************************************************
* NAME
*   avrcpSendMetaDataControlResponse
*
*  Description
*       Send a Common Metadata Control Response
****************************************************************************/
void avrcpSendMetaDataControlResponse(AVRCP                   *avrcp,
                                      avrcp_response_type      response,
                                      uint8                    pdu_id);

#endif /* !AVRCP_CT_ONLY_LIB*/

#endif /* AVRCP_SEND_RESPONSE_H_ */

