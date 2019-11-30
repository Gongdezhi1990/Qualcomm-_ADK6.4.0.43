/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_send_response.c        

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include <string.h>
#include "avrcp_send_response.h"
#include "avrcp_notification_handler.h"


#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
* NAME    
* sendPassThroughResponse    
*
* DESCRIPTION
* Send PassThrough response to the CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   Response code. 
*
* RETURNS
*    void
*******************************************************************************/
void sendPassThroughResponse(AVRCP *avrcp, avrcp_response_type response)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_PASSTHROUGH_RES);

#ifdef AVRCP_DEBUG_LIB    
    if (response > avctp_response_interim)
    {
        AVRCP_DEBUG(("Out of range response  0x%x\n", response));
    }
#endif

    message->response = response;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_PASSTHROUGH_RES, message);
}

/****************************************************************************
* NAME    
* sendUnitInfoResponse    
*
* DESCRIPTION
* Send UNIT_INFO response to the CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   accept      -   TRUE on command success and FALSE on command reject.
*   uint_type   -   Subunit type ID. 
*   uint8       -   Unit ID.
*   company_id  -   Company ID to send in the response. 
*
* RETURNS
*    void
*******************************************************************************/
void sendUnitInfoResponse(AVRCP *avrcp, 
                         bool accept,
                         avc_subunit_type unit_type, 
                         uint8 unit, 
                         uint32 company_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_UNITINFO_RES);

#ifdef AVRCP_DEBUG_LIB    
    if (unit_type > 0x1F)
    {
        AVRCP_DEBUG(("Out of range subunit type  0x%x\n", unit_type));
    }
    if (unit > 0x07)
    {
        AVRCP_DEBUG(("Out of range unit  0x%x\n", unit));
    }
    if (company_id > 0xFFFFFF)
    {
        AVRCP_DEBUG(("Out of range company id  0x%lx\n", company_id));
    }
#endif

    message->accept = accept;
    message->unit_type = unit_type;
    message->unit = unit;
    message->company_id = company_id;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_UNITINFO_RES, message);
}

/****************************************************************************
* NAME    
* sendSubunitInfoResponse    
*
* DESCRIPTION
* Send SUB_UNIT_INFO response to the CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   accept      -   TRUE on command success and FALSE on command reject.
*   page_data   -   page_data 
*
* RETURNS
*    void
*******************************************************************************/
void sendSubunitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SUBUNITINFO_RES);
    message->accept= accept;

    if (accept)
        memcpy(message->page_data, page_data, sizeof(message->page_data));
    else
        memset(message->page_data, 0, sizeof(message->page_data));

    MessageSend(&avrcp->task, AVRCP_INTERNAL_SUBUNITINFO_RES, message);
}

/****************************************************************************
* NAME    
* sendVendorDependentResponse    
*
* DESCRIPTION
* Send Vendor Dependent response to the CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*
* RETURNS
*    void
*******************************************************************************/
void sendVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_VENDORDEPENDENT_RES);

#ifdef AVRCP_DEBUG_LIB    
    if (response > avctp_response_interim)
    {
        AVRCP_DEBUG(("Out of range response  0x%x\n", response));
    }
#endif

    message->response= response;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_VENDORDEPENDENT_RES, message);
}


/****************************************************************************
* NAME    
* avrcpSendGetCapsResponse    
*
* DESCRIPTION
* Send response for GetCapabilities command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   caps        -   Capability ID
*   size_caps_list- Size of data in Source
*   caps_list   -   Source contains list of capabilities for the response. 
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendGetCapsResponse(AVRCP *avrcp, 
                            avrcp_response_type response, 
                            avrcp_capability_id caps, 
                            uint16 size_caps_list, 
                            Source caps_list)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_CAPS_RES);
    message->caps_id = caps;        
    message->response = response;        
    message->size_caps_list = size_caps_list;
    message->caps_list = caps_list;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_CAPS_RES, message);
}

/****************************************************************************
* NAME    
* sendGetElementsResponse    
*
* DESCRIPTION
* Send response for GetElementAttributes command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   number_of_attributes - Number of attributes
*   size_attributes      - Size of attributes
*   attributes           - Source contains the list of attributes. 
*
* RETURNS
*    void
*******************************************************************************/
void sendGetElementsResponse(AVRCP *avrcp,
                             avrcp_response_type response, 
                             uint16 number_of_attributes, 
                             uint16 size_attributes,
                             Source attributes)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES);
    message->response = response;
    message->number_of_attributes = number_of_attributes;
    message->size_attributes_list = size_attributes;
    message->attributes_list = attributes;
    MessageSend(&avrcp->task,
                 AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES, message);
}

/****************************************************************************
* NAME    
* sendGroupResponse    
*
* DESCRIPTION
* Send response for GroupNavigation PassThrough command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*
* RETURNS
*    void
*******************************************************************************/
void sendGroupResponse(AVRCP *avrcp, avrcp_response_type response)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GROUP_RES);   
    message->response = response;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_GROUP_RES, message);
}


/****************************************************************************
* NAME    
* sendPlayStatusResponse    
*
* DESCRIPTION
* Send response for PlayStatus command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   song_length -   Length of Song
*   song_elapsed-   Elapsed Song duration
*   play_status -   Play status
*
* RETURNS
*    void
*******************************************************************************/
void sendPlayStatusResponse(AVRCP *avrcp,
                            avrcp_response_type response, 
                            uint32 song_length, 
                            uint32 song_elapsed, 
                            avrcp_play_status play_status)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_PLAY_STATUS_RES);
    message->response = response;
    message->song_length = song_length;
    message->song_elapsed = song_elapsed;
    message->play_status = play_status;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_PLAY_STATUS_RES, message);
}

/****************************************************************************
* NAME    
* sendListAttrResponse    
*
* DESCRIPTION
* Send response for ListPlayerApplicationSettingAttributes command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   size_attributes - Size of attributes
*   attributes  -   List of attributes
*
* RETURNS
*    void
*******************************************************************************/
void sendListAttrResponse(AVRCP *avrcp,
                         avrcp_response_type response, 
                         uint8 size_attributes,
                         Source attributes)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES);
    message->response = response;
    message->size_attributes_list = size_attributes;
    message->attributes_list = attributes;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES, message);
}

/****************************************************************************
* NAME    
* sendListValuesResponse    
*
* DESCRIPTION
* Send response for ListPlayerApplicationSettingValues command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   size_values -   Size of values
*   values      -   Source containing the list of values. 
*
* RETURNS
*    void
*******************************************************************************/
void sendListValuesResponse(AVRCP *avrcp, 
                            avrcp_response_type response, 
                            uint8 size_values, 
                            Source values)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_LIST_APP_VALUE_RES);
    message->response = response;
    message->size_values_list = size_values;
    message->values_list = values;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_LIST_APP_VALUE_RES, message);
}

/****************************************************************************
* NAME    
* sendGetValuesResponse    
*
* DESCRIPTION
* Send response for GetPlayerApplicationSettingValue command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   size_values -   Size of values
*   values      -   Source containing the list of values. 
*
* RETURNS
*    void
*******************************************************************************/
void sendGetValuesResponse(AVRCP *avrcp,
                           avrcp_response_type response, 
                           uint16 size_values,
                           Source values)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_APP_VALUE_RES);
    message->response = response;
    message->size_values_list = size_values;
    message->values_list = values;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_APP_VALUE_RES, message);
}

/****************************************************************************
* NAME    
* sendSetValuesResponse    
*
* DESCRIPTION
* Send response for SetPlayerApplicationSettingValue command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*
* RETURNS
*    void
*******************************************************************************/
void sendSetValuesResponse(AVRCP *avrcp, avrcp_response_type response)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SET_APP_VALUE_RES);
    message->response = response;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_SET_APP_VALUE_RES, message);
}

/****************************************************************************
* NAME    
* sendGetAttributeTextResponse   
*
* DESCRIPTION
* Send response for GetPlayerApplicationSettingAttributeText command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   number_of_attributes - Number of attributes.
*   size_attributes      - Size if attributes. 
*   attributes           - Attributes List. 
*
* RETURNS
*    void
*******************************************************************************/
void sendGetAttributeTextResponse(AVRCP *avrcp,
                                  avrcp_response_type response, 
                                  uint8 number_of_attributes, 
                                  uint16 size_attributes, 
                                  Source attributes)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES);
    message->response = response;
    message->number_of_attributes = number_of_attributes;
    message->size_attributes_list = size_attributes;
    message->attributes_list = attributes;
    MessageSend(&avrcp->task, 
                AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES, message);
}

/****************************************************************************
* NAME    
* sendGetValueTextResponse   
*
* DESCRIPTION
* Send response for GetPlayerApplicationSettingValueText command to CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   number_of_values - Number of values.
*   size_values      - Size if values. 
*   values           - Values List. 
*
* RETURNS
*    void
*******************************************************************************/
void sendGetValueTextResponse(AVRCP *avrcp,
                             avrcp_response_type response, 
                             uint8 number_of_values, 
                             uint16 size_values, 
                             Source values)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES);
    message->response = response;
    message->number_of_values = number_of_values;
    message->size_values_list = size_values;
    message->values_list = values;
    MessageSend(&avrcp->task, AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES, message);
}

/*************************************************************************
* NAME
*   avrcpSendEventResponse
*
*  Description
*       Send a Common Event Response
*
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   event       -   Notification event ID.
*
* RETURNS
*    void
****************************************************************************/
void avrcpSendEventResponse(AVRCP                   *avrcp,
                           avrcp_response_type      response,
                           avrcp_supported_events   event) 
{
    if (isEventRegistered(tg, avrcp, event))
    {
        /* Internal Event response */
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_EVENT_COMMON_RES); 
        message->response=response;
        message->event = event;

        MessageSend(&avrcp->task, 
                    AVRCP_INTERNAL_EVENT_COMMON_RES,
                    message);

    }
    else
    {
       AVRCP_INFO((" %d Event not registered\n", event));
                   
    }
}

/*************************************************************************
* NAME
*   avrcpSendMetaDataControlResponse
*
*  Description
*       Send a Common Metadata Control Response
*
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   response    -   response code. 
*   pdu_id      -   PDU ID.
*
* RETURNS
*    void
****************************************************************************/
void avrcpSendMetaDataControlResponse(AVRCP                   *avrcp,
                                      avrcp_response_type      response,
                                      uint8                    pdu_id)
{
    MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES);
    message->response = response;
    message->pdu_id = pdu_id;
    MessageSend(&avrcp->task, 
                AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES,
                message);
} 
#endif /* !AVRCP_CT_ONLY_LIB*/

