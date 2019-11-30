/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_player_app_settings_handler.c

DESCRIPTION
    This file defines the internal functions for handling
    Player Application Settings Features.     

    - ListPlayerApplicationSettingsAttributes
    - ListPlayerApplicationSettingsValue
    - GetCurrentPlayerApplicationSettingValue
    - SetPlayerApplicationsSettingValue
    - GetPlayerApplicationsSettingsAttributeText
    - GetPlayerApplicationSettingValueText

    - InformDisplayableCharacterSet
    - InformBatteryStatusOfCT 

    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include "avrcp_player_app_settings_handler.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_init.h"
#include <stddef.h>

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
 *NAME    
 *    avrcpHandleListAppAttributesCommand    
 *
 *DESCRIPTION
 *  Handle the incoming PlayerApplicationListAttributes Command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 ****************************************************************************/
void avrcpHandleListAppAttributesCommand(AVRCP *avrcp)
{
    if (!isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()))
    {
        sendListAttrResponse(avrcp, avctp_response_not_implemented, 0, 0);
    }
    else
    {
        /* We know this PDU cannot be fragmented. 
            Pass the request directly to the client. */    
        avrcpSendCommonMetadataInd(avrcp, AVRCP_LIST_APP_ATTRIBUTE_IND);
    }
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalListAppAttributesResponse   
 *
 *DESCRIPTION
 *  Send PlayerApplicationListAttributes response to CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  res         -  Response data from app.
 ****************************************************************************/
void avrcpHandleInternalListAppAttributesResponse(AVRCP *avrcp, 
                      AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T *res)
{
    uint16 size_mandatory_data = 1;
    uint8 mandatory_data[1];
    uint16 param_length = 1;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response,
                                                 AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        param_length += res->size_attributes_list;

        /* Insert the mandatory data */
        mandatory_data[0] = res->size_attributes_list;
    }

    prepareMetadataStatusResponse(avrcp, res->response,
                                 AVRCP_LIST_APP_ATTRIBUTES_PDU_ID,
                                 param_length, res->attributes_list,
                                 size_mandatory_data, mandatory_data);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleListAppValuesCommand    
 *
 *DESCRIPTION
 *  Handle the incoming PlayerApplicationListValues Command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  attribute_id -  Requested attribute ID for Value.
 ****************************************************************************/
void avrcpHandleListAppValuesCommand(AVRCP *avrcp, uint16 attribute_id)
{

    if (!isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()))
    {
        sendListValuesResponse(avrcp, avctp_response_not_implemented, 0, 0);
    }
    else
    {
        /* We know this PDU cannot be fragmented.
           Pass the request directly to the client. */
        MAKE_AVRCP_MESSAGE(AVRCP_LIST_APP_VALUE_IND);
        message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
        message->transaction =  avrcp->bitfields.rsp_transaction_label;
#endif
        message->attribute_id = attribute_id;
        MessageSend(avrcp->clientTask, AVRCP_LIST_APP_VALUE_IND, message);
    }
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalListAppValuesResponse    
 *
 *DESCRIPTION
 *  Send PlayerApplicationListValues response to CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  res         -  Application response. 
 ****************************************************************************/
void avrcpHandleInternalListAppValuesResponse(AVRCP *avrcp,
                 AVRCP_INTERNAL_LIST_APP_VALUE_RES_T *res)
{
    uint16 size_mandatory_data = 1;
    uint8 mandatory_data[1];
    uint16 param_length = 1;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                            AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        param_length += res->size_values_list;

        /* Insert the mandatory data */
        mandatory_data[0] = res->size_values_list;
    }

    prepareMetadataStatusResponse(avrcp, res->response,
                                 AVRCP_LIST_APP_VALUE_PDU_ID,
                                 param_length, res->values_list,
                                 size_mandatory_data, mandatory_data);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleGetAppValuesCommand    
 *
 *DESCRIPTION
 *  Handle the incoming getPlayerApplicationValues Command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  ptr         -  Received Data in the command
 *  packet_size -  size of data
 ****************************************************************************/
void avrcpHandleGetAppValuesCommand(AVRCP *avrcp, 
                                    const uint8* ptr,
                                    uint16 packet_size)
{
    uint16 data_offset=1;
    Source source=0;

    if(isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()) && packet_size)
    {
        packet_size-= data_offset;

        if(packet_size)
        {
            source=avrcpSourceFromConstData(avrcp, 
                                            ptr+data_offset,
                                            packet_size);
            if(!source)
            {
                packet_size = 0;
            }
        }

        avrcpSendCommonFragmentedMetadataInd(avrcp, AVRCP_GET_APP_VALUE_IND, 
                                             ptr[0], 
                                             packet_size, source);
    }
    else
    {
        avrcpSendRejectMetadataResponse(avrcp, 
                                        avctp_response_not_implemented,
                                        AVRCP_GET_APP_VALUE_PDU_ID);
    }
}


/****************************************************************************
 *NAME    
 *    avrcpHandleInternalGetAppValueResponse    
 *
 *DESCRIPTION
 *  Send GetPlayerApplicationValues response to CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  res         -  Application response. 
 ****************************************************************************/
void avrcpHandleInternalGetAppValueResponse(AVRCP *avrcp,
               AVRCP_INTERNAL_GET_APP_VALUE_RES_T *res)
{
    uint8 mandatory_data[1];
    uint16 param_length = AVRCP_APP_NUM_ATTR_HDR_SIZE;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        param_length += res->size_values_list;
        /* Insert the mandatory data */
        mandatory_data[0] = (uint8)(res->size_values_list/2);
    }

    prepareMetadataStatusResponse(avrcp, res->response,
                                 AVRCP_GET_APP_VALUE_PDU_ID, 
                                 param_length, res->values_list,
                                 AVRCP_APP_NUM_ATTR_HDR_SIZE, mandatory_data);
}


/****************************************************************************
 *NAME    
 *    avrcpHandleSetAppValuesCommand    
 *
 *DESCRIPTION
 *  Handle the incoming SetPlayerApplicationValues Command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  ptr         -  Received Data in the command
 *  packet_size -  size of data
 ****************************************************************************/
void avrcpHandleSetAppValuesCommand(AVRCP *avrcp, 
                                    const uint8* ptr, 
                                    uint16 packet_size)
{
    uint16 data_offset=1;
    Source source=0;

    if(isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()) && packet_size)
    {
        packet_size-= data_offset;
    
        if(packet_size)
        {
                source=avrcpSourceFromConstData(avrcp,
                                                ptr+data_offset, 
                                                packet_size);
            if(!source)
            {
                packet_size = 0;
            }

        }

        avrcpSendCommonFragmentedMetadataInd(avrcp, AVRCP_SET_APP_VALUE_IND, 
                                             ptr[0], 
                                             packet_size, source);
    }
    else
    {
           avrcpSendRejectMetadataResponse(avrcp, 
                                           avctp_response_not_implemented,
                                           AVRCP_SET_APP_VALUE_PDU_ID);
    }
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalSetAppValueResponse    
 *
 *DESCRIPTION
 *  Send SetPlayerApplicationValues response to CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  res         -  Application response. 
 ****************************************************************************/
void avrcpHandleInternalSetAppValueResponse(AVRCP *avrcp, 
               AVRCP_INTERNAL_SET_APP_VALUE_RES_T *res)
{
    uint8 mandatory_data[1];

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_CONTROL);

    avrcpSendMetadataResponse(avrcp,  res->response, 
                              AVRCP_SET_APP_VALUE_PDU_ID, 0, 
                              avrcp_packet_type_single, 
                              AVRCP_ERROR_CODE_SIZE, AVRCP_ERROR_CODE_SIZE, 
                              mandatory_data);

}


/****************************************************************************
 *NAME    
 *    avrcpHandleGetAppAttributeTextCommand    
 *
 *DESCRIPTION
 *  Handle the incoming GetApplicationAttributesText Command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  ptr         -  Received Data in the command
 *  packet_size -  size of data
 ****************************************************************************/
void avrcpHandleGetAppAttributeTextCommand(AVRCP *avrcp, 
                                        const uint8* ptr, 
                                        uint16 packet_size)
{
    Source source=0;
    uint16 data_offset=1;

    if(isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()) && packet_size)
    {
        packet_size-= data_offset;

       if(packet_size)
       {
               source=avrcpSourceFromConstData(avrcp,
                                             ptr+data_offset, 
                                             packet_size);
            if(!source)
            {
                packet_size = 0;
            }
       }

        avrcpSendCommonFragmentedMetadataInd(avrcp, 
                                             AVRCP_GET_APP_ATTRIBUTE_TEXT_IND,
                                             ptr[0], 
                                             packet_size, source);
    }
    else
    {
        avrcpSendRejectMetadataResponse(avrcp, 
                                        avctp_response_not_implemented,
                                        AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID);
    }
}


/****************************************************************************
 *NAME    
 *    avrcpHandleInternalGetAppAttributeTextResponse    
 *
 *DESCRIPTION
 *  Send GetApplicationAttributesText response to CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  res         -  Application response. 
 ****************************************************************************/
void avrcpHandleInternalGetAppAttributeTextResponse(AVRCP *avrcp,
             AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T *res)
{
    uint8 mandatory_data[AVRCP_APP_NUM_ATTR_HDR_SIZE];
    uint16 param_length = AVRCP_APP_NUM_ATTR_HDR_SIZE;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        param_length += res->size_attributes_list;
    
        /* Insert the mandatory data */

        mandatory_data[0] = res->number_of_attributes;
    }

    prepareMetadataStatusResponse(avrcp, res->response, 
                                  AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID, 
                                  param_length, res->attributes_list, 
                                  AVRCP_APP_NUM_ATTR_HDR_SIZE, 
                                  mandatory_data);
}


/****************************************************************************
 *NAME    
 *    avrcpHandleGetAppValueTextCommand   
 *
 *DESCRIPTION
 *  Handle the incoming GetApplicationValueText Command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  ptr         -  Received Data in the command
 *  packet_size -  size of data
 ****************************************************************************/
void avrcpHandleGetAppValueTextCommand(AVRCP *avrcp,  
                                      const uint8* data, 
                                      uint16 packet_size)
{
    if(isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()) && (packet_size > 1))
    {
        Source source=0;
        uint16 data_offset=2;

        packet_size-= data_offset;

       if(packet_size)
       {
            source=avrcpSourceFromConstData(avrcp, data+data_offset,
                                            packet_size);
            if(!source)
            {
                packet_size=0;
            }
       }

        {
        MAKE_AVRCP_MESSAGE(AVRCP_GET_APP_VALUE_TEXT_IND);

        message->avrcp = avrcp;
        message->attribute_id = data[0];
        message->number_of_attributes = data[1];
        message->size_attributes = packet_size;
        message->attributes = source;


#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
        message->metadata_packet_type = avrcp_packet_type_single;
        message->transaction =  avrcp->bitfields.rsp_transaction_label;
        message->no_packets = 0;
        message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
        message->data_offset = 0;
#endif

        MessageSend(avrcp->clientTask, AVRCP_GET_APP_VALUE_TEXT_IND, message);
        }    
    }
    else
    {
        avrcpSendRejectMetadataResponse(avrcp, 
                                        avctp_response_not_implemented,
                                        AVRCP_GET_APP_VALUE_PDU_ID);
    }
}



/****************************************************************************
 *NAME    
 *    avrcpHandleInternalGetAppValueTextResponse   
 *
 *DESCRIPTION
 *  Send GetApplicationValueText response to CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  res         -  Application response. 
 ****************************************************************************/
void avrcpHandleInternalGetAppValueTextResponse(AVRCP *avrcp,
              AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T *res)
{
    uint8 mandatory_data[AVRCP_APP_NUM_ATTR_HDR_SIZE];
    uint16 param_length = AVRCP_APP_NUM_ATTR_HDR_SIZE;

    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        param_length += res->size_values_list;

        /* Insert the mandatory data */
        mandatory_data[0] = res->number_of_values;
    }

    prepareMetadataStatusResponse(avrcp, res->response,
                                  AVRCP_GET_APP_VALUE_TEXT_PDU_ID, 
                                  param_length, res->values_list, 
                                  AVRCP_APP_NUM_ATTR_HDR_SIZE, 
                                  mandatory_data);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInformBatteryStatusCommand   
 *
 *DESCRIPTION
 *  Handle the incoming InformBatteryStatusOfCT command from CT. 
 *
 *PARAMETERS
 *  AVRCP          -  AVRCP Instance
 *  battery_status -  Battery status
 ****************************************************************************/
void avrcpHandleInformBatteryStatusCommand(AVRCP *avrcp, uint8 battery_status)
{
    /*    
        Process the command PDU. If app settings are not enabled at 
        this side then reject the  command immediately. If they are enabled 
        then send an indication up to the app so it can respond. 
    */
    if(!isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()))
    {

        avrcpSendMetaDataControlResponse(avrcp, 
                                         avctp_response_not_implemented, 
                                         AVRCP_INFORM_BATTERY_STATUS_PDU_ID);
    }
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INFORM_BATTERY_STATUS_IND);
        message->avrcp = avrcp;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
        message->transaction =  avrcp->bitfields.rsp_transaction_label;
#endif
        message->battery_status = (avrcp_battery_status)battery_status;
       MessageSend(avrcp->clientTask, AVRCP_INFORM_BATTERY_STATUS_IND, message);
    }
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInformCharSetCommand   
 *
 *DESCRIPTION
 *  Handle the incoming InformCharSetCommand command from CT. 
 *
 *PARAMETERS
 *  AVRCP       -  AVRCP Instance
 *  ptr         -  Received Data in the command
 *  packet_size -  size of data
 ****************************************************************************/
void avrcpHandleInformCharSetCommand(AVRCP *avrcp,
                                     const uint8* ptr, 
                                     uint16 packet_size)
{
    if(isAvrcpPlayerSettingsEnabled(avrcpGetDeviceTask()) && packet_size)
    {
        Source source=0;
        uint16 data_offset=1;

        packet_size-= data_offset;
        if(packet_size)
        {
            if((source=avrcpSourceFromConstData(avrcp, 
                                                ptr+data_offset, 
                                                packet_size)) == NULL)
            {
                packet_size=0;
            }
        }


        avrcpSendCommonFragmentedMetadataInd(avrcp, 
                                             AVRCP_INFORM_CHARACTER_SET_IND, 
                                             ptr[0], 
                                             packet_size, source);
    }
    else
    {
        avrcpSendRejectMetadataResponse(avrcp,
                                       avctp_response_not_implemented,
                                       AVRCP_INFORM_CHARACTER_SET_PDU_ID);
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/
