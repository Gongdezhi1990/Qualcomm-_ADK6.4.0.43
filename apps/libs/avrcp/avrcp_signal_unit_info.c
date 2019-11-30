/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.



FILE NAME
    avrcp_signal_unit_info.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include <sink.h>
#include <string.h>
#include "avrcp_signal_unit_info.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
* NAME    
* avrcpSendUnitInfoCfmToClient    
*
* DESCRIPTION
* Send application confirmation message for UNIT INFO to the CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   status      -   Status
*   unit_type   -   UNIT ID
*   unit        -   UNIT value
*   company     -   COMPANY ID 
*
* RETURNS
*    void
******************************************************************************/
void avrcpSendUnitInfoCfmToClient(AVRCP *avrcp, 
                                  avrcp_status_code status, 
                                  uint16 unit_type,
                                  uint16 unit, 
                                  uint32 company)
{
    MAKE_AVRCP_MESSAGE(AVRCP_UNITINFO_CFM);
    message->status = status;
    message->sink = avrcp->sink;
    message->unit_type = (avc_subunit_type) unit_type;
    message->unit = unit;
    message->company_id = company;    
    message->avrcp = avrcp;
    MessageSend(avrcp->clientTask, AVRCP_UNITINFO_CFM, message);
}

/****************************************************************************
* NAME    
* avrcpHandleInternalUnitInfoReq    
*
* DESCRIPTION
* Handle the application request at CT to send a Unit Info command to the TG.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpHandleInternalUnitInfoReq(AVRCP *avrcp)
{
    uint16 packet_size = 5 + AVRCP_TOTAL_HEADER_SIZE;
    
    Sink sink = avrcp->sink;

    uint8* ptr = avrcpGrabSink(sink, packet_size);

    if (!ptr)
    {    
        avrcpSendUnitInfoCfmToClient(avrcp, avrcp_no_resource, 
                                     0, 0, (uint32) 0);
    }
    else
    {
        /* AVCTP header */
        avrcpAvctpSetCmdHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_SINGLE, 0);

        /* AVRCP header */
        ptr[3] = AVRCP0_CTYPE_STATUS;
        ptr[4] = AVRCP1_UNIT;
        ptr[5] = AVRCP2_UNITINFO;

        /* UnitInfo set to all f's */
        memset(&ptr[6], 0xff, 5);

        /* Send the data */
        (void)SinkFlush(sink,packet_size);

        avrcp->pending = avrcp_unit_info;
        MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 
                         0, AVCTP_WATCHDOG_TIMEOUT);
    }    
}

/****************************************************************************
*NAME    
*    avrcpHandleUnitInfoResponse    
*
*DESCRIPTION
*    This function Handles the incoming Unif INfo Response
*                  -----------------------------------------------
*                  | MSB |     |    |     |    |     |     |   LSB |
*                  |-----------------------------------------------|
*   octet0         |   0000                |   response:STABLE     |
*                  |-----------------------------------------------|
*   octet1         |   Sub unit type       |   subunitID           |
*                  |-----------------------------------------------|
*    opcode        |               UNITINFO (0x30)                 |
*                  |-----------------------------------------------|
*    operand[0]    |                      0x07                     |
*                  |-----------------------------------------------|
*    operand[1]    |   unit_type:panel(9) |    Unit:0              |
*                  | ----------------------------------------------|
*    operand[2]-[4]|                      Company ID               |
*                  |-----------------------------------------------|
*
*MESSAGE RETURNED
*
*****************************************************************************/
void avrcpHandleUnitInfoResponse(AVRCP *avrcp, 
                                const uint8 *ptr, 
                                uint16 packet_size)
{ 

    /* Give to the application only if it is waiting */
    if(avrcp->pending == avrcp_unit_info)
    {
        if((packet_size >=  AVRCP_UNITINFO_SIZE) &&
            (ptr[AVRCP_CTYPE_OFFSET] == avctp_response_stable))
        {
            avrcpSendUnitInfoCfmToClient(avrcp, avrcp_success, 
                ptr[AVRCP_UNITINFO_PANEL_OFFSET] >> 
                    AVRCP4_UNITINFO_UNIT_TYPE_SHIFT, 
                ptr[AVRCP_UNITINFO_PANEL_OFFSET] & AVRCP4_UNITINFO_UNIT_MASK, 
                (((uint32)ptr[AVRCP_COMPANY_ID_OFFSET] << 16) & 0x00FF0000) |
                (((uint32)ptr[AVRCP_COMPANY_ID_OFFSET+1] << 8) & 0x0000FF00) | 
                ((uint32)ptr[AVRCP_COMPANY_ID_OFFSET+2] & 0x000000FF));
        }
        else
        {
            avrcpSendUnitInfoCfmToClient(avrcp, avrcp_fail, 0, 0, (uint32) 0);
        }

        /* No longer waiting */
        avrcp->pending = avrcp_none;
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
    }

    /* The source has been processed so drop it here. */
    avrcpSourceProcessed(avrcp,TRUE);
}

/****************************************************************************
* NAME    
* avrcpSendSubunitInfoCfmToClient    
*
* DESCRIPTION
* Send application confirmation message for SUB UNIT INFO at CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   status      -   Status
*   page        -   page number
*   page_data   -   page data
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendSubunitInfoCfmToClient(AVRCP *avrcp, 
                                    avrcp_status_code status, 
                                    uint8 page, 
                                    const uint8 *page_data)
{
    MAKE_AVRCP_MESSAGE(AVRCP_SUBUNITINFO_CFM);
    message->status = status;
    message->page = page;

    if (page_data)
    {
        memcpy(message->page_data, page_data, sizeof(message->page_data));
    }
    else
    {
        memset(message->page_data, 0, sizeof(message->page_data));
    }

    message->sink = avrcp->sink;
    message->avrcp = avrcp;
    MessageSend(avrcp->clientTask, AVRCP_SUBUNITINFO_CFM, message);
}

/****************************************************************************
* NAME    
* avrcpHandleInternalSubUnitInfoReq    
*
* DESCRIPTION
* Handle the application request at CT to send a SubUnit Info command to the TG.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   req         -   Request parameters from the application.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpHandleInternalSubUnitInfoReq(AVRCP *avrcp, 
       const AVRCP_INTERNAL_SUBUNITINFO_REQ_T *req)
{
    uint16 packet_size = 5 + AVRCP_TOTAL_HEADER_SIZE;
    
    Sink sink = avrcp->sink;
    uint8* ptr = avrcpGrabSink(sink, packet_size);

    if (!ptr)
        avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_no_resource, 0, 0);
    else
    {
        /* AVCTP header */
        avrcpAvctpSetCmdHeader(avrcp, &ptr[0], AVCTP0_PACKET_TYPE_SINGLE, 0);

        /* AVRCP header */
        ptr[3] = AVRCP0_CTYPE_STATUS;
        ptr[4] = AVRCP1_UNIT;
        ptr[5] = AVRCP2_SUBUNITINFO;

        /* Fill in operand[0] - page and extension code*/
        ptr[6] = ((req->page << AVRCP3_SUBUNITINFO_PAGE_SHIFT) & 0x70) | 
                 AVRCP3_SUBUNITINFO_EXTEND_MASK;
        
        /* Set Page data to 0xFF */
        ptr[7] = 0xFF;
        ptr[8] = 0xFF;
        ptr[9] = 0xFF;
        ptr[10] = 0xFF;

        /* Send the data */
        (void) SinkFlush(sink,packet_size);

        avrcp->pending = avrcp_subunit_info;
        MessageSendLater(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 
                         0, AVCTP_WATCHDOG_TIMEOUT);
    }    
}

/****************************************************************************
*NAME    
*    avrcpHandleSubUnitInfoResponse    
*
*DESCRIPTION
*    This function Handles the incoming SubUnit Response
*                   -----------------------------------------------
*                   | MSB |      |     |    |    |     |     |  LSB |
*                   |-----------------------------------------------|
*   octet0          |   0000                |   response:STABLE     |
*                   |-----------------------------------------------|
*   octet1          |   Sub unit type       |   subunitID           |
*                   |-----------------------------------------------|
*    octet2         |               UNITINFO (0x31)                 |
*                   |-----------------------------------------------|
*    octet3         |0  | page(0)         |0  | Extensioncode(7)    |
*                    -----------------------------------------------
*    octet4         |   unit_type:panel(9) |   sub_Unit:0           |
*                    -----------------------------------------------
*    5-7            |                      FF                       |
*                    -----------------------------------------------
*
*MESSAGE RETURNED
*
*****************************************************************************/
void avrcpHandleSubUnitInfoResponse(AVRCP *avrcp, 
                                    const uint8 *ptr, 
                                    uint16 packet_size)
{
    if(avrcp->pending == avrcp_subunit_info)
    {
        if((packet_size >=  AVRCP_UNITINFO_SIZE) &&
            (ptr[AVRCP_CTYPE_OFFSET] == avctp_response_stable))
        {
            avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_success, 
               ptr[AVRCP_SUBUNIT_PAGE_OFFSET] >> AVRCP3_SUBUNITINFO_PAGE_SHIFT, 
               &ptr[AVRCP_SUBUNIT_PAGE_DATA_OFFSET]);
        }
        else
        {
            avrcpSendSubunitInfoCfmToClient(avrcp, avrcp_fail, 0, 0);
        }

        /* No longer waiting */
        avrcp->pending = avrcp_none;
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
    }

    /* The source has been processed so drop it here. */
    avrcpSourceProcessed(avrcp,TRUE);
}


#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
/****************************************************************************
*NAME    
*    avrcpHandleInternalUnitInfoRes    
*
*DESCRIPTION
*    This is the internal function for AvrcpUnitInfoResponse
*                  -----------------------------------------------
*                  | MSB |      |      |    |      |     |    |LSB |
*                  |-----------------------------------------------|
*   octet0         |   0000                |   response:STABLE     |
*                  |-----------------------------------------------|
*   octet1         |   Sub unit type       |   subunitID           |
*                  |-----------------------------------------------|
*    opcode        |               UNITINFO (0x30)                 |
*                   -----------------------------------------------
*    operand[0]    |                      0x07                     |
*                   -----------------------------------------------
*    operand[1]    |   unit_type:panel(9) |    Unit:0              |
*                   -----------------------------------------------
*    operand[2]-[4]|                      Company ID               |
*                  -----------------------------------------------
*
*MESSAGE RETURNED
*
*****************************************************************************/
void avrcpHandleInternalUnitInfoRes( AVRCP *avrcp,
                              const  AVRCP_INTERNAL_UNITINFO_RES_T *res)
{
    uint16 offset      = AVRCP_TOTAL_HEADER_SIZE;
    uint16 packet_size = AVCTP_SINGLE_PKT_HEADER_SIZE + AVRCP_UNITINFO_SIZE;
    uint8  *pdu        = NULL;

    if (!avrcp->av_msg)
    {
        AVRCP_DEBUG(("Invalid Response state\n")); 
        return;
    }


    if (!res->accept)
    {
        avrcpSendAvcResponse(   avrcp,
                                AVRCP_UNITINFO_SIZE,
                                avctp_response_rejected, 0, (Source)NULL);
        return;
    }

    /* No fragmentation for Unit INFO*/
    if((pdu = avrcpGrabSink(avrcp->sink, packet_size)) != NULL)
    {
        memmove(pdu, avrcp->av_msg, offset);
               
        pdu[offset]   = 0x07; /* magic number from spec */
        pdu[offset+1] = (res->unit_type <<
                        AVRCP4_UNITINFO_UNIT_TYPE_SHIFT) |
                       (res->unit & AVRCP4_UNITINFO_UNIT_MASK);
        pdu[offset+2] = (res->company_id >> 16) & 0xff;
        pdu[offset+3] = (res->company_id >> 8) & 0xff;
        pdu[offset+4] = res->company_id & 0xff;
        pdu[AVCTP_SINGLE_PKT_HEADER_SIZE] = avctp_response_stable;
        pdu[AVCTP_HEADER_START_OFFSET] ^= AVCTP0_CR_MASK;
                
        /* Send the data */
        (void)SinkFlush(avrcp->sink, packet_size);

        /* Allow data arriving to be processed. */
        avrcpUnblockReceivedData(avrcp);
    }
    else
    {
        /* Else Internal problem .. */
        Panic();
    }
}


/****************************************************************************
 *NAME    
 *
 *  avrcpHandleUnitInfoCommand 
 *
 *DESCRIPTION
 *  This function Handles the incoming Unit Info command request from 
 *   remote device
 *    
 * PARAMETERS
 *   avrcp                   - AVRCP Task.
 *   const uint8*            - Ptr to the received AVRCP command
 *   uint16                  - packet_size
 *    
 *  Expected Command looks like
 *                  -----------------------------------------------
 *                  | MSB |    |      |    |    |    |    |    LSB  |
 *                  |-----------------------------------------------|
 *  Octet0          |        0000           | ctype: STATUS (1)     |
 *                  |-----------------------------------------------|
 *  octet1          |   Subunit_type        |   Subunit_ID:ignore(7)|
 *                  |-----------------------------------------------|
 *    octet2        |            opcode:UNIT_INFO (30)              |
 *                  -----------------------------------------------
 *    octet3-7      |                      FF                       |
 *                  -----------------------------------------------
 *
 *MESSAGE RETURNED
 *     
 ****************************************************************************/
void avrcpHandleUnitInfoCommand(AVRCP       *avrcp,
                                const uint8 *ptr, 
                                uint16      packet_size)
{
    /* Check packet has a valid header and payload if any */
    UNUSED(ptr);

    /* Check the Request has Valid length */
    if(packet_size <  AVRCP_UNITINFO_SIZE) 
    {
        avrcpSendAvcResponse(avrcp, packet_size, avctp_response_rejected, 0,0 );
    }
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_UNITINFO_IND);
        message->sink = avrcp->sink;
        message->avrcp = avrcp;

        avrcpBlockReceivedData(avrcp, avrcp_unit_info, 0);
        MessageSend(avrcp->clientTask, AVRCP_UNITINFO_IND, message);
    }
}


/****************************************************************************
*NAME    
*    avrcpHandleInternalSubUnitInfoRes    
*
*DESCRIPTION
*    This is the internal function for AvrcpSubUnitInfoResponse
*                   -----------------------------------------------
*                   | MSB |     |      |     |    |     |    |  LSB |
*                   |-----------------------------------------------|
*   octet0          |   0000                |   response:STABLE     |
*                   |-----------------------------------------------|
*   octet1          |   Sub unit type       |   subunitID           |
*                   |-----------------------------------------------|
*    octet2         |               UNITINFO (0x31)                 |
*                   |-----------------------------------------------|
*    octet3         |0  | page(0)             |0  | Extensioncode(7)|
*                   -----------------------------------------------
*    octet4         |   unit_type:panel(9) |    sub_Unit:0          |
*                    -----------------------------------------------
*    5-7            |                      FF                       |
*                    -----------------------------------------------
*
*MESSAGE RETURNED
*
*****************************************************************************/
void avrcpHandleInternalSubUnitInfoRes( AVRCP *avrcp, 
                 const AVRCP_INTERNAL_SUBUNITINFO_RES_T *res)
{

    uint16 offset=AVCTP_SINGLE_PKT_HEADER_SIZE+AVRCP_SUBUNIT_PAGE_DATA_OFFSET;
    uint16 packet_size=AVRCP_UNITINFO_SIZE+AVCTP_SINGLE_PKT_HEADER_SIZE;

    if (avrcp->av_msg)
    {
        if (!res->accept)
        {
            avrcpSendAvcResponse(   avrcp,
                                    AVRCP_UNITINFO_SIZE,
                                    avctp_response_rejected, 0, (Source)NULL);
        }
        else
        {
         
            /* Library received only a single packet Response */
            /* Write back our values and send response - 
                No fragmentation for Unit INFO*/
            
            uint8 *pdu = avrcpGrabSink(avrcp->sink, packet_size);
            
            if(pdu)
            {
                memmove(pdu, avrcp->av_msg, offset);
               
                /* Library received only a single packet Request */
                /* Copy back the page data */
                memmove(&pdu[offset], &res->page_data[0], PAGE_DATA_LENGTH);
                
                pdu[AVCTP_SINGLE_PKT_HEADER_SIZE] = avctp_response_stable;
                pdu[AVCTP_HEADER_START_OFFSET] ^= AVCTP0_CR_MASK;
                
                /* Send the data */
                (void)SinkFlush(avrcp->sink, packet_size);
           }
            
           /* Lib ignores the message if it is not able to send it */ 
           /* The source has been processed so drop it here. */
              avrcpSourceProcessed(avrcp, TRUE);   
            
       }
    }
    
    /* Allow data arriving to be processed. */
    avrcpUnblockReceivedData(avrcp);

}

/****************************************************************************
 *NAME    
 *
 *  avrcpHandleSubUnitInfoCommand 
 *
 *DESCRIPTION
 *  This function Handles the incoming SubUnit Info command request
 *    from remote device
 *    
 * PARAMETERS
 *   avrcp                   - AVRCP Task.
 *   const uint8*            - Ptr to the received AVRCP command
 *   uint16                  - packet_size
 *    
 *  Expected Command looks like
 *                  -----------------------------------------------
 *                  | MSB |     |    |     |   |     |     |    LSB |
 *                  |-----------------------------------------------|
 *  Octet0          |        0000           | ctype: STATUS (1)     |
 *                  |-----------------------------------------------|
 *  octet1          |   Subunit_type        |   Subunit_ID:ignore(7)|
 *                  |-----------------------------------------------|
 *  octet2          |            opcode:SUB_UNIT_INFO (31)          |
 *                   -----------------------------------------------
 *  octet3-7        |                      FF                       |
 *                    -----------------------------------------------
 *
 *MESSAGE RETURNED
 *     
 ****************************************************************************/

void avrcpHandleSubUnitInfoCommand(AVRCP *avrcp,
                                   const uint8 *ptr, 
                                   uint16 packet_size)
{
    /* Check packet has a valid header and payload if any */

    /* Check the Request has Valid length */
    if(packet_size <  AVRCP_UNITINFO_SIZE) 
    {
        avrcpSendAvcResponse(avrcp, packet_size, avctp_response_rejected, 0,0 );
    }
    else
    {
        MAKE_AVRCP_MESSAGE(AVRCP_SUBUNITINFO_IND);
        message->page = ptr[AVRCP_SUBUNIT_PAGE_OFFSET] >> 
                        AVRCP3_SUBUNITINFO_PAGE_SHIFT;
        message->sink = avrcp->sink;
        message->avrcp = avrcp;

        avrcpBlockReceivedData(avrcp, avrcp_subunit_info, 0);
        MessageSend(avrcp->clientTask, AVRCP_SUBUNITINFO_IND, message);
    }

}

#endif /* !AVRCP_CT_ONLY_LIB*/

