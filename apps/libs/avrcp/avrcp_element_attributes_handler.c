/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_element_attributes_handler.c

DESCRIPTION
    This file defines the internal functions for handling GetElementAttributes    
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include "avrcp_element_attributes_handler.h"
#include "avrcp_metadata_transfer.h"

#include <stddef.h>

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
* NAME    
*   avrcpHandleGetElementAttributesCommand    
*
* DESCRIPTION
*   Handle the incoming Get Element Attributes command
* 
* PARAMETERS
*   avrcp       - AVRCP Instance
*   ptr         - Pointer to the data packet
*   packet_size - Packet size
*******************************************************************************/
void avrcpHandleGetElementAttributesCommand(AVRCP       *avrcp,
                                        const uint8     *ptr, 
                                        uint16          packet_size)
{
    uint16 data_offset=9;
    uint32 identifier_high = 0, identifier_low = 0;
    uint8 attribute_count=0;
    Source source=0;
    
   if(packet_size >= AVRCP_GET_ELEMENTS_HDR_SIZE)
   {
        data_offset=9;
        identifier_high = convertUint8ValuesToUint32(&ptr[0]);
        identifier_low = convertUint8ValuesToUint32(&ptr[4]);
        attribute_count= ptr[8];
        packet_size -= data_offset;

        if(packet_size)
        {
            if((source=avrcpSourceFromConstData(avrcp, 
                                            ptr+data_offset, 
                                            packet_size)) == NULL)
            {
                packet_size = 0;
            }
        }
    }


   {
    MAKE_AVRCP_MESSAGE(AVRCP_GET_ELEMENT_ATTRIBUTES_IND);

    message->avrcp = avrcp;
    message->identifier_high = identifier_high;
    message->identifier_low = identifier_low;
    message->number_of_attributes = attribute_count;
    message->size_attributes = packet_size;
    message->attributes = source;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->metadata_packet_type = avrcp_packet_type_single;
    message->transaction =  avrcp->bitfields.rsp_transaction_label;
    message->no_packets = 1;
    message->ctp_packet_type = AVCTP0_PACKET_TYPE_SINGLE;
    message->data_offset = 0;
#endif 

    MessageSend(avrcp->clientTask, AVRCP_GET_ELEMENT_ATTRIBUTES_IND, message);
   }
}


/****************************************************************************
* NAME    
*   AvrcpGetElementAttributesResponse    
*
* DESCRIPTION
*   Internal function for AvrcpGetCapabilitiesResponse
*
* RETURNS
*    void
*******************************************************************************/
void avrcpHandleInternalGetElementAttributesResponse(AVRCP      *avrcp, 
                   AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T  *res)
{
    uint16 size_mandatory_data = 1; /* Just error code for failure */
    uint8 mandatory_data[1];
    uint16 param_length = 1;

    /* Get the error status code */
    mandatory_data[0] = avrcpGetErrorStatusCode(&res->response, 
                                                AVRCP0_CTYPE_STATUS);

    if (res->response == avctp_response_stable)
    {
        param_length = size_mandatory_data + res->size_attributes_list;

        /* Insert the right mandatory data */
        mandatory_data[0] = (uint8)res->number_of_attributes;
    }

    prepareMetadataStatusResponse(  avrcp, 
                                    res->response, 
                                    AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID, 
                                    param_length, 
                                    res->attributes_list, 
                                    size_mandatory_data, 
                                    mandatory_data);
}
#endif /* !AVRCP_CT_ONLY_LIB*/

