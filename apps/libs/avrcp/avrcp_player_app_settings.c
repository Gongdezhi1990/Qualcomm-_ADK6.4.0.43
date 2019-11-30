/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    avrcp_player_app_settings.c

DESCRIPTION
This file defines the API Functions for following Player Application 
Settings Features.     

    - ListPlayerApplicationSettingsAttributes
    - ListPlayerApplicationSettingsValue
    - GetCurrentPlayerApplicationSettingValue
    - SetPlayerApplicationsSettingValue
    - GetPlayerApplicationsSettingsAttributeText
    - GetPlayerApplicationSettingValueText

    - InformDisplayableCharacterSet
    - InformBatteryStatusOfCT
*/


/****************************************************************************
    Header files
*/
#include <source.h>
#include "avrcp_metadata_transfer.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
* NAME    
* AvrcpListAppAttributeRequest    
*
* DESCRIPTION
* API to send ListPlayerApplicationSettingAttributes Command to the TG. On 
* completion this API, library returns the message AVRCP_LIST_APP_ATTRIBUTE_CFM.
* All parameters are described in the header file.
*******************************************************************************/
void AvrcpListAppAttributeRequest( AVRCP *avrcp )
{
    avrcp_status_code status = avrcpMetadataStatusCommand(avrcp,
                                        AVRCP_LIST_APP_ATTRIBUTES_PDU_ID, 
                                        avrcp_list_app_attributes,
                                        0, 0, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                            AVRCP_LIST_APP_ATTRIBUTE_CFM, 
                                            0, 0, 0);
    }
}


/****************************************************************************
* NAME    
* AvrcpListAppValueRequest    
*
* DESCRIPTION
* API to send ListPlayerApplicationSettingValue  to the CT. 
* All parameters are described in the header file.
*******************************************************************************/
void AvrcpListAppValueRequest(AVRCP *avrcp, uint16 attribute_id)
{
    uint8 status_params[] = {0};
    avrcp_status_code status;

    status_params[0] = (uint8)attribute_id;

    status = avrcpMetadataStatusCommand(avrcp, AVRCP_LIST_APP_VALUE_PDU_ID,
                                        avrcp_list_app_values, 
                                        sizeof(status_params), 
                                        status_params, 0, 0);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                             AVRCP_LIST_APP_VALUE_CFM, 
                                             0, 0, 0);
    }
}


/****************************************************************************
*NAME    
*    AvrcpGetAppValueRequest    
*
*DESCRIPTION
*  API function to send GetCurrentPlayerApplicationSettingValue Request from CT.
*  All parameters are described in the header file.
******************************************************************************/
void AvrcpGetAppValueRequest( AVRCP   *avrcp, 
                              uint16  size_attributes, 
                              Source  attributes )
{
    uint8 extra_params[AVRCP_APP_NUM_ATTR_HDR_SIZE];
    avrcp_status_code status;

    extra_params[0] = (uint8)size_attributes;

    status = avrcpMetadataStatusCommand(avrcp, AVRCP_GET_APP_VALUE_PDU_ID, 
                                        avrcp_get_app_values, 
                                        AVRCP_APP_NUM_ATTR_HDR_SIZE, 
                                        extra_params,
                                        size_attributes,
                                        attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status,
                                             AVRCP_GET_APP_VALUE_CFM, 0, 0, 0);
    }
}

/****************************************************************************
*NAME    
*    AvrcpSetAppValueRequest    
*
*DESCRIPTION
*  API function to send SetPlayerApplicationSettingValue request to TG.
*  All parameters are described in the header file.
******************************************************************************/
void AvrcpSetAppValueRequest( AVRCP *avrcp, 
                              uint16 size_attributes, 
                              Source attributes )
{
    avrcp_status_code status;
    uint8 num_attr[1];


    /* Size of Attributes should be multiple of 2 contains the ID & 
       Value combination */
    if(size_attributes % 2)
    {
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_rejected_invalid_param, 
                                   AVRCP_SET_APP_VALUE_CFM);
        return;
    }

    num_attr[0] = (uint8)(size_attributes/2);

    status = avrcpMetadataControlCommand(avrcp, AVRCP_SET_APP_VALUE_PDU_ID,
                                         avrcp_set_app_values, 
                                         1, num_attr,
                                         size_attributes,attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, AVRCP_SET_APP_VALUE_CFM);
    }
}

/****************************************************************************
*NAME    
*    AvrcpGetAppAttributeTextRequest    
*
*DESCRIPTION
*  API function to send GetPlayerApplicationSettingAttributeText Request from CT.
*  All parameters are described in the header file.
*****************************************************************************/
void AvrcpGetAppAttributeTextRequest( AVRCP *avrcp, 
                                      uint16 size_attributes, 
                                      Source attributes )
{
    uint8 extra_params[AVRCP_APP_NUM_ATTR_HDR_SIZE];
    avrcp_status_code status;

    extra_params[0] = (uint8)size_attributes;

    status = avrcpMetadataStatusCommand(avrcp, 
                                    AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID, 
                                    avrcp_get_app_attribute_text, 
                                    AVRCP_APP_NUM_ATTR_HDR_SIZE, extra_params, 
                                    size_attributes,
                                    attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                            AVRCP_GET_APP_ATTRIBUTE_TEXT_CFM,
                                            0, 0,  0);
    }
}

/****************************************************************************
*NAME    
*    AvrcpGetAppValueText    
*
*DESCRIPTION
*  API function to send GetAppSettingValueText Request from CT.
*  All parameters are described in the header file.   
*****************************************************************************/
void AvrcpGetAppValueTextRequest(   AVRCP *avrcp, 
                                    uint16 attribute_id, 
                                    uint16 size_values, 
                                    Source values )
{
    uint8 extra_params[AVRCP_APP_VAL_TXT_HDR_SIZE];
    avrcp_status_code status;

    extra_params[0] = (uint8)attribute_id;
    extra_params[1] = (uint8)size_values;

    status = avrcpMetadataStatusCommand(avrcp, 
                                        AVRCP_GET_APP_VALUE_TEXT_PDU_ID, 
                                        avrcp_get_app_value_text,  
                                         AVRCP_APP_VAL_TXT_HDR_SIZE,
                                        extra_params,size_values,values);

    if (status != avrcp_success)
    {
        avrcpSendCommonFragmentedMetadataCfm(avrcp, status, 
                                              AVRCP_GET_APP_VALUE_TEXT_CFM, 
                                             0, 0, 0);
    }
}

/****************************************************************************
*NAME    
*    AvrcpInformBatteryStatusRequest    
*
*DESCRIPTION
*  API function to send InformBatteryStatusCommandOfCT command to TG.
*  All parameters are described in the header file.   
******************************************************************************/
void AvrcpInformBatteryStatusRequest(AVRCP *avrcp, 
                                  avrcp_battery_status battery_status)
{
    uint8 status_params[] = {0};
    avrcp_status_code status;

    status_params[0] = battery_status;

    status = avrcpMetadataControlCommand(avrcp,
                                         AVRCP_INFORM_BATTERY_STATUS_PDU_ID,
                                         avrcp_battery_information,
                                         sizeof(status_params),
                                         status_params,0,0);
    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, 
                                   AVRCP_INFORM_BATTERY_STATUS_CFM);
    }
}

/****************************************************************************
*NAME    
*    AvrcpInformCharacterSetRequest   
*
*DESCRIPTION
*  API function to send InformDisplayableCharacterSet command to TG.
*****************************************************************************/
void AvrcpInformCharacterSetRequest( AVRCP   *avrcp, 
                                     uint16  size_attributes, 
                                     Source  attributes )
{
    avrcp_status_code status;
    uint8 num_char_set[1];

    /* Validate the number of attributes are in pair */
    if(size_attributes % 2)
    {
        avrcpSendCommonMetadataCfm(avrcp, 
                                   avrcp_rejected_invalid_param, 
                                   AVRCP_INFORM_CHARACTER_SET_CFM);
        return;
    }

    num_char_set[0] = (uint8)(size_attributes/2);

    status = avrcpMetadataControlCommand(avrcp, 
                                         AVRCP_INFORM_CHARACTER_SET_PDU_ID,
                                         avrcp_character_set, 1, num_char_set,
                                         size_attributes, attributes);

    if (status != avrcp_success)
    {
        avrcpSendCommonMetadataCfm(avrcp, status, 
                                   AVRCP_INFORM_CHARACTER_SET_CFM);
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
* NAME    
* AvrcpListAppAttributeResponse    
*
* DESCRIPTION
* API to send ListPlayerApplicationSettingAttributes response to the CT. 
* All parameters are described in the header file.
*******************************************************************************/
void AvrcpListAppAttributeResponse( AVRCP *avrcp, 
                                    avrcp_response_type response, 
                                    uint16 size_attributes, 
                                    Source attributes )
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_list_app_attributes)
    {
        sendListAttrResponse(avrcp, response, size_attributes, attributes);
    }
    else
    {
        SourceEmpty( attributes );
        AVRCP_INFO(("AvrcpListAppSettingAttributeResponse:"
              "CT not waiting for response\n"));
    }
}

/****************************************************************************
* NAME    
* AvrcpListAppValueResponse    
*
* DESCRIPTION
* API to send ListPlayerApplicationSettingValueResponse to the CT. 
* All parameters are described in the header file.
*******************************************************************************/
void AvrcpListAppValueResponse( AVRCP              *avrcp, 
                                avrcp_response_type response, 
                                uint16              size_values, 
                                Source              values )
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_list_app_values)
    {
        sendListValuesResponse(avrcp, response, size_values, values);
    }
    else
    {
        SourceEmpty( values );
        AVRCP_INFO(("AvrcpListAppSettingValueResponse:"
              "CT not waiting for response\n"));
    }
}


/****************************************************************************
*NAME    
*    AvrcpGetAppValueResponse    
*
*DESCRIPTION
*  API function to send GetCurrentPlayerApplicationSettingValue response to CT.
*  All parameters are described in the header file.
******************************************************************************/
void AvrcpGetAppValueResponse( AVRCP *avrcp, 
                               avrcp_response_type response, 
                               uint16 size_values, 
                               Source values)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_get_app_values)
    {
        sendGetValuesResponse(avrcp, response, size_values, values);
    }
    else
    {
        SourceEmpty( values );
        AVRCP_INFO(("AvrcpGetCurrentAppSettingValueResponse:"
               "CT not waiting for response\n"));
    }
}


/****************************************************************************
*NAME    
*    AvrcpSetAppValueResponse    
*
*DESCRIPTION
*  API function to send SetPlayerApplicationSettingValue response to CT.
*  All parameters are described in the header file.
******************************************************************************/
void AvrcpSetAppValueResponse(AVRCP *avrcp, avrcp_response_type response)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_set_app_values)
    {
        sendSetValuesResponse(avrcp, response);
    }
    else
    {
        AVRCP_INFO(("AvrcpSetAppSettingValueResponse: "
                "CT not waiting for response\n"));
    }
}

/****************************************************************************
*NAME    
*    AvrcpGetAppAttributeTextResponse    
*
*DESCRIPTION
*  API function to send GetPlayerApplicationSettingAttributeText response to CT.
*  All parameters are described in the header file.
******************************************************************************/
void AvrcpGetAppAttributeTextResponse(  AVRCP *avrcp, 
                                        avrcp_response_type response, 
                                        uint16 number_of_attributes, 
                                        uint16 size_attributes, 
                                        Source attributes)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_get_app_attribute_text)
    {
        sendGetAttributeTextResponse(avrcp, response, 
                                    number_of_attributes, size_attributes,
                                    attributes);
    }
    else
    {
        SourceEmpty( attributes );
        AVRCP_INFO(("AvrcpGetAppSettingAttributeTextResponse:"
                " CT not waiting for response\n"));
    }
}



/****************************************************************************
*NAME    
*    AvrcpGetAppValueTextResponse    
*
*DESCRIPTION
*  API function to send GetApplicationSettingValueText Response to CT.
*  All parameters are described in the header file.   
******************************************************************************/
void AvrcpGetAppValueTextResponse( AVRCP *avrcp, 
                                   avrcp_response_type response, 
                                   uint16 number_of_values,
                                   uint16 size_values, 
                                   Source values)
{
    /* Send the response only if the command arrived to start with. */
    if (avrcp->bitfields.block_received_data == avrcp_get_app_value_text)
    {
        sendGetValueTextResponse(avrcp, response, number_of_values,
                                 size_values, values);
    }
    else
    {
        SourceEmpty( values );
        AVRCP_INFO(("AvrcpGetAppSettingValueResponse:"
                " CT not waiting for response\n"));
    }
}


/****************************************************************************
*NAME    
*    AvrcpInformBatteryStatusResponse    
*
*DESCRIPTION
*  API function to send InformBatteryStatusCommandOfCT response to CT.
*  All parameters are described in the header file.   
******************************************************************************/
void AvrcpInformBatteryStatusResponse(AVRCP *avrcp,
                                      avrcp_response_type response)
{
    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->bitfields.block_received_data == avrcp_battery_information)
    {
        avrcpSendMetaDataControlResponse(avrcp, 
                                         response, 
                                         AVRCP_INFORM_BATTERY_STATUS_PDU_ID);
    }
    else
    {
        AVRCP_INFO(("AvrcpInformBatteryStatusOfCtResponse:"
                "CT not waiting for response\n"));
    }
}

/****************************************************************************
*NAME    
*    AvrcpInformCharacterSetResponse   
*
*DESCRIPTION
*  API function to send AvrcpInformDisplayableCharacterSetResponse command to CT.
******************************************************************************/
void AvrcpInformCharacterSetResponse(AVRCP *avrcp, 
                                     avrcp_response_type response)
{
    /* Only allow a response to be sent if the corresponding command arrived. */
    if (avrcp->bitfields.block_received_data == avrcp_character_set)
    {
        avrcpSendMetaDataControlResponse(avrcp, 
                                           response, 
                                           AVRCP_INFORM_CHARACTER_SET_PDU_ID);
    }
    else
    {
        AVRCP_INFO(("AvrcpInformDisplayableCharacterSetResponse:"
               "CT not waiting for response\n"));
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/
