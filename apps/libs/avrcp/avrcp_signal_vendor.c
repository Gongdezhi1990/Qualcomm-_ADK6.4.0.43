/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.



FILE NAME
    avrcp_signal_vendor.c

DESCRIPTION
    This file defines the internal functions for handling Vendor Specific 
    commands and responses

NOTES

*/


/****************************************************************************
    Header files
*/
#include <panic.h>
#include <sink.h>
#include <string.h>
#include <source.h>

#include "avrcp_signal_vendor.h"
#include "avrcp_metadata_transfer.h"
#include "avrcp_notification_handler.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
* NAME    
* avrcpSetVendorPacketHeader    
*
* DESCRIPTION
* Set Vendor dependent packet header.
*  
* PARAMETERS
*   ptr         -   pointer to the packet
*   req         -   Vendor dependent request.
*
* RETURNS
*    void
*******************************************************************************/
static void avrcpSetVendorPacketHeader(uint8 *ptr,
                     const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req)
{
    /* AVRCP header */
    ptr[0] = req->ctype;
    ptr[1] = ((req->subunit_type << AVRCP1_SUBUNIT_TYPE_SHIFT)
             & AVRCP1_SUBUNIT_TYPE_MASK) |
            (req->subunit_id & AVRCP1_SUBUNIT_ID_MASK);
    ptr[2] = AVRCP2_VENDORDEPENDENT;
    
    ptr[3] = (req->company_id >> 16) & 0xFF;
    ptr[4] = (req->company_id >> 8 ) & 0xFF;
    ptr[5] = (req->company_id      ) & 0xFF;
}

/****************************************************************************
* NAME    
* avrcpSendVendordependentCfmToClient    
*
* DESCRIPTION
* Send application confirmation message for Vendor Dependent request to the CT.
*  
* PARAMETERS
*   avrcp       -   AVRCP Instance.
*   status      -   Status
*   response    -   Response code from peer. This will be deprecated.
*
* RETURNS
*    void
*******************************************************************************/
void avrcpSendVendordependentCfmToClient(AVRCP *avrcp, 
                                        avrcp_status_code status, 
                                        uint8 response)
{
    MAKE_AVRCP_MESSAGE(AVRCP_VENDORDEPENDENT_CFM);
    message->status = status;
    message->sink = avrcp->sink;
#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
    message->response = response;
#else
    UNUSED(response);
#endif
    message->avrcp = avrcp;
    MessageSend(avrcp->clientTask, AVRCP_VENDORDEPENDENT_CFM, message);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalVendorDependentReq    
 *
 *DESCRIPTION
 *This is the Internal function for AvrcpVendorDependent() request    
 *
 *PARAMETERS
 * avrcp    -       AVRCP Entity
 * req      -       Vendor Dependent Request
 *    
 *                  -----------------------------------------------
 *                 | MSB |     |    |     |    |     |    |    LSB |
 *                 |-----------------------------------------------|
 *  octet 1        |      0x0              | Ctype:0x0 (Control)   |
 *                 |-----------------------------------------------|
 *   octet 2       | SubUnit_type          |   subunit_id          |
 *                 |-----------------------------------------------|     
 *    opcode       |              VENDOR-DEPENDENT (0x00)          |
 *                  -----------------------------------------------
 *    operand(0)   |                                               |
 *    operand(1)   |                     company_id                |
 *    operand(2)   |                                               |
 *                 |-----------------------------------------------| 
 *    operand(3)   |                                               |
 *        :        |                vendor_dependent_data          |
 *    operand(n)   |                                               | 
 *                 -----------------------------------------------
 *
 *MESSAGE RETURNED
*****************************************************************************/
void avrcpHandleInternalVendorDependentReq(AVRCP *avrcp, 
                                const AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T *req)
{
    uint8* ptr;
    uint16 header_size= AVRCP_VENDOR_HEADER_SIZE; 
    avrcp_status_code result = avrcp_success;
    uint16 avctp_header_size= AVCTP_SINGLE_PKT_HEADER_SIZE; 
    uint16 max_mtu = avrcp->l2cap_mtu - AVCTP_SINGLE_PKT_HEADER_SIZE;
    uint16 packet_size = req->data_length+ AVRCP_VENDOR_HEADER_SIZE;

    /* Check whether it is a Meta Data Command. For Meta Data Commands, avrcp 
       fills the extra data before calling it */
    if (req->extra_data_len)
    {
        packet_size += req->extra_data_len;
        header_size += req->extra_data_len;

        /* Store the Transaction ID and Event for Notification registration */
        if(req->extra_data[0] == AVRCP_REGISTER_NOTIFICATION_PDU_ID)
        {
            uint8 event = req->pending - avrcp_events_start_dummy;

            if(isEventRegistered(ct, avrcp, event))
            {
                if(req->data_length)
                    SourceEmpty(req->data);
                avrcpSendRegisterNotificationFailCfm( avrcp, avrcp_busy, event );
                return;
            } 

            /* Store which event has been registered for. */
            registerEventNotification(ct, avrcp, event);

            /* Store the next transaction label associated with the event. */
            avrcp->ct_notify_transaction_label[event-1] = 
                                avrcp->bitfields.cmd_transaction_label;
        }
    }

    /* Packets will be always less than AVRCP_AVC_MAX_PKT_SIZE */
    /* Total Packet size of first AVCTP packet depends on Fragmentation */
    if(packet_size > max_mtu)
    {
        avctp_header_size =  AVCTP_START_PKT_HEADER_SIZE;

        /* Set the First Packet size as MTU */
        packet_size = avrcp->l2cap_mtu;
    }
    else
    {
        packet_size += AVCTP_SINGLE_PKT_HEADER_SIZE;
    }

    /* Check Sink Space is available for the First packet */
    if(packet_size < SinkSlack(avrcp->sink))
    {
        /* Grab Sink for this Size and Set the AVRCP Header.
           Check is not required since SinkSlacks returns success  */ 
        ptr=avrcpGrabSink(avrcp->sink, header_size+avctp_header_size);
            
        /* AVRCP header */
        avrcpSetVendorPacketHeader(&ptr[avctp_header_size], req);

        /* Copy the Meta Data information  */
        if(req->extra_data_len)
        {
            memmove(&ptr[avctp_header_size+AVRCP_VENDOR_HEADER_SIZE], 
                    req->extra_data, req->extra_data_len);
        }
            
        /* Send AVCTP Message */
        result=avrcpAvctpSendMessage(avrcp, AVCTP0_CR_COMMAND,
                                         ptr,header_size, 
                                         req->data_length, 
                                         req->data);
    }
    else
    {
        result= avrcp_no_resource;
    }


    if(result == avrcp_success)
    {
   
        avrcp->pending = req->pending;

       /* Set a watchdog timeout */
        MessageSendLater( &avrcp->task, 
                          AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 
                          0, AVCTP_WATCHDOG_TIMEOUT );

    }
    else    
    {
        /* Failure. Free all data */
        if(req->data_length)
        {
            SourceEmpty(req->data);
        }

        if (req->pending == avrcp_vendor)
        {
            avrcpSendVendordependentCfmToClient(avrcp, result, 0);
        }
        else
        {
            avrcp->pending = req->pending;
            avrcpSendMetadataFailCfmToClient(avrcp, result);
        }
        
    }
}

/****************************************************************************
 *NAME    
 *    avrcpHandleVendorResponse    
 *
 *DESCRIPTION
 * This function handles the incoming Vendor dependent response
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * res          -       Vendor Dependent Response
 *    
 *                  -----------------------------------------------
 *                  | MSB |     |      |     |      |     |    |LSB |
 *                  |-----------------------------------------------|
 *  octet 1         |      0x0              | Response:0xC (Stable) |
 *                  |-----------------------------------------------|
 *   octet 2        | SubUnit_type          |   subunit_id          |
 *                  |-----------------------------------------------|  
 *    opcode        |              VENDOR-DEPENDENT (0x00)          |
 *                  -----------------------------------------------
 *    operand(0)    |                                               |
 *    operand(1)    |                     company_id                |
 *    operand(2)    |                                               | 
 *                  |-----------------------------------------------| 
 *    operand(3)    |                                               |
 *        :         |                vendor_dependent_data          |
 *    operand(n)    |                                               | 
 *                  -----------------------------------------------
 *
 *MESSAGE RETURNED
*****************************************************************************/
void avrcpHandleVendorResponse(AVRCP *avrcp, 
                         const uint8 *ptr, 
                               uint16 packet_size)
{
    if(avrcp->pending == avrcp_vendor)
    {
        avrcp_response_type response =  ptr[AVRCP_CTYPE_OFFSET];

        /* No longer waiting */
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);

        /* Send Confirmation Message to the application */
        avrcpSendVendordependentCfmToClient(avrcp, 
                                            convertResponseToStatus(response), 
                                            response);
        /* No longer waiting */
        avrcp->pending = avrcp_none;

        /* The source has been processed so drop it here. */
        avrcpSourceProcessed(avrcp,TRUE);
        
    }
    else
    {
        if(packet_size < AVRCP_VENDOR_HEADER_SIZE + METADATA_HEADER_SIZE)
        {
            /* Received Vendor response but not waiting on response. */
            
            if (packet_size)
            {
                avrcp_response_type response =  ptr[AVRCP_CTYPE_OFFSET];
                /* Send Confirmation Message to the application */
                avrcpSendVendordependentCfmToClient(avrcp, 
                                                convertResponseToStatus(response), 
                                                response);
            }

            /* The source has been processed so drop it here. */
            avrcpSourceProcessed(avrcp,TRUE);
        }
        else
        {
            /* Response for Meta Data Request */
            avrcpHandleMetadataResponse(avrcp,  ptr , packet_size);
        }
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/****************************************************************************
 *NAME    
 *    avrcpHandleInternalVendorDependentRes    
 *
 *DESCRIPTION
 * This is the internal function for AvrcpVendorDependentResponse().
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * res          -       Vendor Dependent Response
 *    
 *                 -----------------------------------------------
 *                | MSB |     |    |     |     |     |    | LSB |
 *                |-----------------------------------------------|
 *  octet 1       |      0x0              | Response:0xC (Stable) |
 *                |-----------------------------------------------|
 *   octet 2      | SubUnit_type          |   subunit_id          |
 *                |-----------------------------------------------|  
 *    opcode      |              VENDOR-DEPENDENT (0x00)          |
 *                -----------------------------------------------
 *    operand(0)  |                                               |
 *    operand(1)  |                     company_id                |
 *    operand(2)  |                                               |
 *                |-----------------------------------------------| 
 *    operand(3)  |                                               |
 *        :       |                vendor_dependent_data          |
 *    operand(n)  |                                               | 
 *                -----------------------------------------------
 *
 *MESSAGE RETURNED
*****************************************************************************/
void avrcpHandleInternalVendorDependentRes(AVRCP *avrcp, 
                         const AVRCP_INTERNAL_VENDORDEPENDENT_RES_T *res)
{
    /* Currently Avrcp does not support sending any data with the response */

    /* Frame an AV/C message for VENDOR Response and send it */   
    avrcpSendAvcResponse(   avrcp, 
                            AVRCP_VENDOR_HEADER_SIZE, 
                            res->response, 0, (Source)NULL);
}

/****************************************************************************
 *NAME    
 *    avrcpHandleVendorCommand    
 *
 *DESCRIPTION
 * This function handles the incoming Vendor dependent command.
 *
 *PARAMETERS
 * avrcp        -       AVRCP Entity
 * ptr          -       Pointer to the received Vendor Dependent Request
 * packet_size  -       size of the request packet
 *    
 *                  -----------------------------------------------
 *                  | MSB |     |      |    |      |     |    | LSB |
 *                  |-----------------------------------------------|
 *  octet 1         |      0x0              | Ctype:0x0 (Control)   |
 *                  |-----------------------------------------------|
 *   octet 2        | SubUnit_type          |   subunit_id          |
 *                  |-----------------------------------------------|  
 *    opcode        |              VENDOR-DEPENDENT (0x00)          |
 *                   -----------------------------------------------
 *    operand(0)    |                                               |
 *    operand(1)    |                     company_id                |
 *    operand(2)    |                                               |
 *                  |-----------------------------------------------| 
 *    operand(3)    |                                               |
 *        :         |                vendor_dependent_data          |
 *    operand(n)    |                                               | 
 *                  -----------------------------------------------
 *
 *MESSAGE RETURNED
*****************************************************************************/
void avrcpHandleVendorCommand(AVRCP *avrcp, const uint8 *ptr, uint16 packet_size)
{
    uint16 op_len;
    uint32 cid;  


    /* Check the Request has Valid length */
    if(packet_size <  AVRCP_VENDOR_HEADER_SIZE) 
    {
        avrcpSendAvcResponse(avrcp, packet_size, avctp_response_rejected, 0,0 );
        return;
    }

    cid = avrcpGetCompanyId(ptr, AVRCP_VENDOR_COMPANY_ID_OFFSET);
    op_len = packet_size - AVRCP_VENDOR_HEADER_SIZE;


    if( ( avrcp->bitfields.fragment == avrcp_packet_type_continue ) ||
        ( avrcp->bitfields.fragment == avrcp_packet_type_end ) )
    {
         /* Ignore it silently due to resource restrictions */
         avrcp->bitfields.fragment = avrcp_packet_type_single;
         avrcpSourceProcessed( avrcp, TRUE );
         return;
    }

    if (cid == AVRCP_BT_COMPANY_ID)
    {

        if( op_len < METADATA_HEADER_SIZE) 
        { 
            /* Reject the fragmented metadata command */
             avrcpSendAvcResponse(avrcp, packet_size, 
                                  avctp_response_rejected, 0,0 );
        }
        else
        {
            avrcpHandleMetadataCommand(avrcp, ptr, packet_size);
        }
    }
    else
    {
        /* Send the ind message to the client */
        MAKE_AVRCP_MESSAGE_WITH_LEN(AVRCP_VENDORDEPENDENT_IND, 
         ((op_len > AVRCP_MAX_VENDOR_DATA)?AVRCP_MAX_VENDOR_DATA:op_len + 1));
        message->sink = avrcp->sink;
        message->avrcp = avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED 
    /* Deprecated fields will be removed later */
        message->transaction = (avrcp->av_msg[AVCTP_HEADER_START_OFFSET]
                 & AVCTP_TRANSACTION_MASK)>> AVCTP0_TRANSACTION_SHIFT;
        message->no_packets = 1;
#endif
        message->subunit_type = (avc_subunit_type) 
                 ((ptr[AVRCP_SUBUNIT_OFFSET] 
                 & AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT);
        message->subunit_id = ptr[AVRCP_SUBUNIT_OFFSET] & 
                              AVRCP1_SUBUNIT_ID_MASK;
        message->company_id = cid;
        message->command_type = ptr[AVRCP_CTYPE_OFFSET];
        message->size_op_data = (op_len > AVRCP_MAX_VENDOR_DATA)?
                                 AVRCP_MAX_VENDOR_DATA:op_len;
        if (op_len)
            memmove(message->op_data, ptr+AVRCP_VENDOR_HEADER_SIZE, message->size_op_data);
        else
            message->op_data[0] = 0;

        avrcpBlockReceivedData(avrcp, avrcp_vendor, 0);
        MessageSend(avrcp->clientTask, AVRCP_VENDORDEPENDENT_IND, message);
    }
}
#endif /* !AVRCP_CT_ONLY_LIB*/

