/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.



FILE NAME
    avrcp_vendor.c

DESCRIPTION
    This file defines the APIs for Vendor Specific commands which includes
    common interface function for sending Vendor specific AVC commands.

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include "avrcp_signal_vendor.h"
#include "avrcp_metadata_transfer.h"

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
*NAME    
*   AvrcpVendorDependentRequest    
*
*DESCRIPTION
*   This function is called to send vendor specific data to the peer entity.
*   If the data_length is greater than the l2cap mtu then the message
*   become fragmented.
*   
*   subunit_type
*   subunit_id                - AV/C protocol - Used to form the targets address
*   company_id                - 24-bit unique ID obtained from IEEE RAC.
*   data                    - Required for the Vendor Unique operation.   
*   data_length               
*    
*                    -----------------------------------------------
*                   | MSB |     |    |     |    |    |    |    LSB  |
*                   |-----------------------------------------------|
*   opcode          |              VENDOR-DEPENDENT (0x00)          |
*                    -----------------------------------------------
*   operand(0)      |MSB                                            |
*   operand(1)      |                     company_id                |
*   operand(2)      |                                        LSB    |
*                   |-----------------------------------------------| 
*   operand(3)      |                                               |
*       :           |                vendor_dependent_data          |
*   operand(n)      |                                               | 
*                    -----------------------------------------------
*
*MESSAGE RETURNED
*   AVRCP_VENDORDEPENDENT_CFM is returned indicating the status 
*   of the connection request
*  
*****************************************************************************/
void AvrcpVendorDependentRequest(AVRCP *avrcp, 
                          avc_subunit_type subunit_type, 
                          avc_subunit_id subunit_id, 
                          uint8 ctype, 
                          uint32 company_id, 
                          uint16 data_length, 
                          Source data)
{
    
#ifdef AVRCP_DEBUG_LIB    
    if (subunit_type > 0x1F)
    {
        AVRCP_DEBUG(("Out of range subunit type  0x%x\n", subunit_type));
    }
    if (subunit_id > 0x07)
    {
        AVRCP_DEBUG(("Out of range subunit id  0x%x\n", subunit_id));
    }
    if (company_id > 0xFFFFFF)
    {
        AVRCP_DEBUG(("Out of range company id  0x%lx\n", company_id));
    }
#endif
    
    if(data_length > AVRCP_AVC_MAX_PDU_SIZE)
    {
        avrcpSendVendordependentCfmToClient(avrcp,
                                            avrcp_rejected_invalid_content, 0);
    }
    else if(isAvrcpConnected(avrcp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_VENDORDEPENDENT_REQ);
        message->company_id = company_id;
        message->subunit_type = subunit_type;
        message->subunit_id = subunit_id;
        message->ctype = ctype;
        message->data = data;
        message->data_length = data_length;
        message->extra_data[0] = 0;
        message->extra_data_len = 0;
        message->pending = avrcp_vendor;
        MessageSendConditionally(&avrcp->task, 
                                 AVRCP_INTERNAL_VENDORDEPENDENT_REQ, 
                                 message, &avrcp->pending);
    }
    else
    {
        /* Device not connected. reject the request*/
        avrcpSendVendordependentCfmToClient(avrcp, 
                                            avrcp_device_not_connected, 0);
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME    
*   AvrcpVendorDependentResponse    
*
*DESCRIPTION
*   This API is to send a response to the CT for a Vendor Dependent Command.  
*****************************************************************************/
void AvrcpVendorDependentResponse(AVRCP *avrcp, avrcp_response_type response)
{
    sendVendorDependentResponse(avrcp, response);
}
#endif /* !AVRCP_CT_ONLY_LIB*/

