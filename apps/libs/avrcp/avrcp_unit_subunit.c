/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.



FILE NAME
    avrcp_unit_subunit.c      

DESCRIPTION
    This file defines the APIs for AVRCP UNIT_INFO and SUBUNIT_INFO commands.

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include "avrcp_signal_unit_info.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
*NAME    
*    AvrcpUnitInfoRequest
*
*DESCRIPTION
*   This function is called to request that a UnitInfo control command
*   is sent to the target on the connection identified by the sink.
*   
*   The UnitInfo command is used to obtain information that pertains to the
*   AV/C unit as a whole
*   
*                    ------------------------------------------
*                   | MSB |    |    |    |    |    |    | LSB |
*                   |-----------------------------------------|
*   octet 0         |    0000            | ctype:STATUS(0x01) |
*                   |-----------------------------------------|
*   octet 1         | SubUnit Type(0x1F) | SubUnit ID (0x7)   |
*                   |-----------------------------------------|
*   octet 3         |               UNITINFO (0x30)           |
*                   |-----------------------------------------|
*   octet 4         |                      0xFF               |
*                   |-----------------------------------------|
*   octet 5         |                      0xFF               |
*                   |-----------------------------------------|
*   octet 6         |                      0xFF               |
*                   |-----------------------------------------|
*   octet 7         |                      0xFF               |
*                   |-----------------------------------------|
*   octet 8         |                      0xFF               |
*                   |-----------------------------------------|
*
*MESSAGE RETURNED
*   AVRCP_UNITINFO_CFM - This message contains the unit_type and a
*   unique 24-bit Company ID
*****************************************************************************/
void AvrcpUnitInfoRequest(AVRCP *avrcp)
{
    if(isAvrcpConnected(avrcp))
    {
        MessageSendConditionally(   &avrcp->task, 
                                    AVRCP_INTERNAL_UNITINFO_REQ, 0,
                                    &avrcp->pending);
    }
    else
    {
        avrcpSendUnitInfoCfmToClient(avrcp, avrcp_device_not_connected,
                                     0, 0, (uint32) 0);

    }
}

/****************************************************************************
*NAME    
*  AvrcpSubUnitInfoRequest
*
*DESCRIPTION    
*   The SubUnitInfo command is used to obtain information about the subunit(s)
*   of a device.
*    
*   accept     - Flag accepting or rejecting request for SubUnitInfo
*   page_data  - Four entries from the subunit table for
*                the requested   page on the target device
*                              
*****************************************************************************/
void AvrcpSubUnitInfoRequest(AVRCP *avrcp, uint8 page)
{
    
#ifdef AVRCP_DEBUG_LIB    
    if (page > 0x07)
    {
        AVRCP_DEBUG(("Out of range page  0x%x\n", page));
    }
#endif
    if(isAvrcpConnected(avrcp))
    {
        MAKE_AVRCP_MESSAGE(AVRCP_INTERNAL_SUBUNITINFO_REQ);
        message->page = page;
        MessageSendConditionally(   &avrcp->task, 
                                    AVRCP_INTERNAL_SUBUNITINFO_REQ, 
                                    message,
                                    &avrcp->pending);
    }
    else
    {
        avrcpSendUnitInfoCfmToClient(avrcp, avrcp_device_not_connected,
                                     0, 0, (uint32) 0);
    }    
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME    
*    AvrcpUnitInfoResponse
*
*DESCRIPTION
*   This function is called to send response for a UNIT INFO request 
*   from the CT. All parameters are described in the header file. Response
*   format of UNIT INFO Command is.
*   
*                    ------------------------------------------
*                   | MSB |    |    |    |    |    |    | LSB |
*                   |-----------------------------------------|
*   octet 0         |    0000            |   response         |
*                   |-----------------------------------------|
*   octet 1         |    SubUnit Type    | SubUnit ID         |
*                   |-----------------------------------------|
*   octet 2         |       Opcode : UNIT_INFO (0x30)         |
*                   |-----------------------------------------|
*   octet 3         |                  0x07                   |
*                   | ----------------------------------------|
*   octet 4         |   unit_type        |     Unit           |
*                   | ----------------------------------------|
*   octet 5 -7      |                                         |
*                   |               COMPANY ID                |             
*                   |                                         |
*                   |_________________________________________|
*
*****************************************************************************/
void AvrcpUnitInfoResponse(AVRCP *avrcp,
                           bool accept, 
                           avc_subunit_type unit_type, 
                           uint8 unit,
                           uint32 company_id)
{
    sendUnitInfoResponse(avrcp, accept, unit_type, unit, company_id);
}




/****************************************************************************
*NAME    
*  AvrcpSubUnitInfoResponse
*
*DESCRIPTION    
*  This API is used to respond to SUB UNIT INFO command from the CT.   
*    
*   accept     - Flag accepting or rejecting request for SubUnitInfo
*           
*   page_data  - Four entries from the subunit table for
*                the requested   page on the target device
*                              
*****************************************************************************/
void AvrcpSubUnitInfoResponse(AVRCP *avrcp, bool accept, const uint8 *page_data)
{
    sendSubunitInfoResponse(avrcp, accept, page_data);
}

#endif /* !AVRCP_CT_ONLY_LIB*/

