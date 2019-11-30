/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.



FILE NAME
    avrcp_signal_passthrough.c

DESCRIPTION
    This file defines all Internal functions related to handling PASSTHROUGH. 
    This incluedes
    - PASSTHROUGH Command
    - Basic Group Navigation  commands


NOTES

*/


/****************************************************************************
    Header files
*/
#include <sink.h>
#include <source.h>
#include <panic.h>
#include <string.h>
#include "avrcp_signal_passthrough.h"
#include "avrcp_init.h"


#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/*!
***************************************************************************
* @brief
* Set PassThrough packet header.
*  
*   @param ptr         pointer to the packet
*   @param req         pointer to assthrough request
*
*   @returns void
*******************************************************************************/
static void avrcpSetPassthroughPacketHeader(uint8 *ptr, 
            const AVRCP_INTERNAL_PASSTHROUGH_REQ_T *req)
{
    /* AVRCP header */
    ptr[0] = AVRCP0_CTYPE_CONTROL;
    ptr[1] = ((req->subunit_type << AVRCP1_SUBUNIT_TYPE_SHIFT) 
                & AVRCP1_SUBUNIT_TYPE_MASK) | 
                (req->subunit_id & AVRCP1_SUBUNIT_ID_MASK);
    ptr[2] = AVRCP2_PASSTHROUGH;
    ptr[3] = req->opid | (req->state ? AVRCP3_PASSTHROUGH_STATE_MASK : 0);
    ptr[4] = (uint8)req->operation_data_length;
}


/*!
 ***************************************************************************
 *
 *  @brief
 *    This is the Internal function for handling AvrcpPassthrough() request. 
 *    
 *  @param avrcp   pointer to AVRCP Task
 *  @param req     Contains parameters for Passthrough command.
 *
 *  <tt>
 *                     -----------------------------------------------
 *                    | MSB |     |     |     |    |     |    |   LSB |
 *                    |-----------------------------------------------|
 *    opcode          |              PASSTHROUGH (0x7C)               |
 *                     -----------------------------------------------
 *    operand(0)      |state|           operation_id                  |
 *                      -----------------------------------------------
 *    operand(1)      |          operation data field length          |
 *                     -----------------------------------------------
 *    operand(2)      |     operation data(operation id dependant)    |
 *        :           |                                               |
 *                    -----------------------------------------------
 *  </tt>
 *
 *  @returns void
 *****************************************************************************/

void avrcpHandleInternalPassThroughReq( AVRCP           *avrcp, 
               const AVRCP_INTERNAL_PASSTHROUGH_REQ_T   *req)
{
    uint8* ptr;
    uint16 header_size;
    uint16 avctp_header_size= AVCTP_SINGLE_PKT_HEADER_SIZE; 
    uint16 packet_size = req->operation_data_length+ 
                        AVRCP_PASSTHROUGH_HEADER_SIZE;
    avrcp_status_code result = avrcp_success;


    /* Process only if packet fits in a single AVC command frame. */
    if(AVRCP_AVC_MAX_PKT_SIZE > packet_size) 
    {
        /* AVCTP Fragmentation applies only for vendor_unique commands*/
        if((packet_size < avrcp->l2cap_mtu) || (req->opid == opid_vendor_unique))
        {    
         /* Total Packet size of first AVCTP packet depends on Fragmentation */
            if(packet_size > avrcp->l2cap_mtu)
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
                header_size =  avctp_header_size+AVRCP_PASSTHROUGH_HEADER_SIZE;

                /* Grab Sink for this Size and Set the AVRCP Header.
                Check is not required since SinkSlacks returns success  */ 
                 ptr=avrcpGrabSink(avrcp->sink, header_size);
            
                /* AVRCP header */
                avrcpSetPassthroughPacketHeader(&ptr[avctp_header_size], req); 

                /* Send AVCTP Message */
                result=avrcpAvctpSendMessage(avrcp, AVCTP0_CR_COMMAND,
                                         ptr,AVRCP_PASSTHROUGH_HEADER_SIZE, 
                                         req->operation_data_length, 
                                         req->operation_data);
            }
            else
            {
                result= avrcp_no_resource;
            }
        }
        else
        {
            result= avrcp_no_resource;
        }
    }
    else
    {
        result = avrcp_rejected_invalid_content;
    }

    if (result == avrcp_success)
    {

        /* Set the pending flag and watchdog */
        avrcp->pending = req->pending;
       
        /* Set a watchdog timeout */
        MessageSendLater( &avrcp->task, 
                          AVRCP_INTERNAL_WATCHDOG_TIMEOUT, 
                          0, AVCTP_WATCHDOG_TIMEOUT );
    }
    else
    {   
        if(req->operation_data_length)
        {
            SourceEmpty(req->operation_data);
        }
        avrcpSendPassthroughCfmToClient(avrcp, result);
    }
}

/*!
 ***************************************************************************
 *
 *  @brief
 *  This function Handles the incoming PassThrough command response
 *    for the command we send
 *    
 * PARAMETERS
 *   @param avrcp           pointer to AVRCP Task
 *   @param ptr             pointer to the received AVRCP command
 *   @param packet_size     packet size
 *    
 *  Expected Response looks like
 *
 *  <tt>
 *                    -----------------------------------------------
 *                  | MSB |     |     |     |     |     |    |  LSB |
 *                  |-----------------------------------------------|
 *  Octet0          |        0000           | ctype: ACCEPTED(9)    |
 *                  |-----------------------------------------------|
 *  octet1          |   Subunit_type        |   Subunit_ID          |
 *                  |-----------------------------------------------|
 *    opcode        |              PASSTHROUGH (0x7C)               |
 *                   -----------------------------------------------
 *    operand(0)    |state|            operation_id                 |
 *                   -----------------------------------------------
 *    operand(1)    |          operation data field length          |
 *                  -----------------------------------------------
 *    operand(2)    |     operation data(operation id dependant)    |
 *        :         |                                               |
 *                   -----------------------------------------------
 *  </tt>
 *
 *  @returns void
 ***************************************************************************/
void avrcpHandlePassthroughResponse(AVRCP *avrcp, 
                                    const uint8 *ptr, 
                                    uint16 packet_size)
{

    /* If it is Group, Handle seperate */
    if((avrcp->pending == avrcp_next_group ) ||
       (avrcp->pending == avrcp_previous_group))
    {
       if(packet_size > AVRCP_GROUP_HEADER_SIZE)
       {
            /* Correct the packet length */
            avrcp->av_msg_len    -= (packet_size - AVRCP_GROUP_HEADER_SIZE);
       }

       avrcpHandleGroupResponse(avrcp, ptr, packet_size);
    }
    else if(avrcp->pending == avrcp_passthrough)
    {

        uint16 total_pkt_len = ptr[AVRCP_DATA_LEN_OFFSET] +
                                AVRCP_PASSTHROUGH_HEADER_SIZE;

        
        if(packet_size > total_pkt_len)
        {
            /* Correct the packet len */
            avrcp->av_msg_len    -= (packet_size - total_pkt_len);
        }

        /* Process only if avrcp sends a request */
        if(ptr[AVRCP_CTYPE_OFFSET] ==  avctp_response_accepted)
        {
              avrcpSendPassthroughCfmToClient(avrcp, avrcp_success);
        }
        else if(ptr[AVRCP_CTYPE_OFFSET] == avctp_response_not_implemented)
        {
            avrcpSendPassthroughCfmToClient(avrcp, avrcp_unsupported);
        }
        else
        {
            /* default to a fail handles avctp_response_rejected etc */
            avrcpSendPassthroughCfmToClient(avrcp, avrcp_fail);
        }

        /* No longer waiting */
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
        avrcp->pending = avrcp_none;
    }
    else
    {
        AVRCP_INFO(("avrcpHandlePassthroughResponse: Wrong pending:%d.\n",
                    avrcp->pending));
    }

    /* The source has been processed so drop it here. */
    avrcpSourceProcessed(avrcp, TRUE);
}

/*!
 ***************************************************************************
 *  @brief
 *    This function handles the incoming Group Response
 *    
 *  @param avrcp                   pointer to AVRCP Task
 *  @param ptr                     pointer to response
 *  @param packet_size             packet size
 *
 *  <tt>
 *                   -----------------------------------------------
 *                  | MSB |     |      |     |    |      |     |LSB |
 *                  |-----------------------------------------------|
 *  octet 0         |       0x0             |  RESPONSE:ACCEPED     |
 *                  |-----------------------------------------------|
 *  octet 1         |  Sub Unit_type        |  Sub_unit ID : 0x0    |
 *                  |-----------------------------------------------|
 *  octet 2         |              PASSTHROUGH (0x7C)               |
 *                  | ----------------------------------------------|
 *    octet 3       |state|            operation_id(0x7E)           |
 *                  | ----------------------------------------------|
 *    octet 4       |          operation data field length:0x05     |
 *                  | ----------------------------------------------|
 *    octet 5 - 7   |     BT SIG Company ID                         |
 *                  |-----------------------------------------------|
 *  octet 8-9       |              Vendor Unique ID                 |
 *                  |-----------------------------------------------|
 *  </tt>
 *
 *  @returns void
 ******************************************************************************/
void avrcpHandleGroupResponse(AVRCP *avrcp, const uint8* ptr, uint16 packet_size)
{
    avrcp_status_code status = avrcp_fail;

    /* Check this is definitely a group response received. */
    if ((packet_size >= AVRCP_GROUP_HEADER_SIZE) && 
        ((ptr[AVRCP_OPID_OFFSET] & 0x7F) == opid_vendor_unique) && 
        (ptr[AVRCP_DATA_LEN_OFFSET] == 0x05) && 
        (avrcpGetCompanyId(ptr, AVRCP_COMPANY_ID_OFFSET) == AVRCP_BT_COMPANY_ID))
    {
        uint16 id;

        if (ptr[AVRCP_CTYPE_OFFSET] == avctp_response_accepted)
            status = avrcp_success;

        if ((ptr[AVRCP_VENDOR_UNIQUE_ID_OFFSET] << 8) | 
            ptr[AVRCP_VENDOR_UNIQUE_ID_OFFSET+1])
            id = AVRCP_PREVIOUS_GROUP_CFM;
        else
            id = AVRCP_NEXT_GROUP_CFM;

        /* No longer waiting */
        (void) MessageCancelAll(&avrcp->task, AVRCP_INTERNAL_WATCHDOG_TIMEOUT);
        avrcp->pending = avrcp_none;
    
        avrcpSendCommonMetadataCfm(avrcp, status, id);
    }
    else
    {
        AVRCP_DEBUG(("Invalid Group Response Received\n"));
    }

    /* The source has been processed so drop it here. */
    avrcpSourceProcessed(avrcp, TRUE);
}

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */

/*!
 ***************************************************************************
 *  @brief
 *    This is the Internal function for handling AvrcpPassthroughResponse(). 
 *    AvrcpPassthroughResponse() does not provide parameter for
 *    operation_data, hence this will be always a single packet.
 * 
 *    
 *  @param avrcp            pointer to AVRCP Task.
 *  @param res              pointer to parameters for Passthrough response
 *
 *  <tt>
 *                     -----------------------------------------------
 *                    | MSB |      |     |      |     |      |    |LSB |
 *                    |-----------------------------------------------|
 *    opcode          |              PASSTHROUGH (0x7C)               |
 *                     -----------------------------------------------
 *    operand(0)      |state|            operation_id                 |
 *                      -----------------------------------------------
 *    operand(1)      |          operation data field length          |
 *                     -----------------------------------------------
 *    operand(2)      |     operation data(operation id dependant)    |
 *        :           |                                               |
 *                     -----------------------------------------------
 *  </tt>
 *
 *  @returns void
 *****************************************************************************/
void avrcpHandleInternalPassThroughRes(AVRCP *avrcp, 
                                 const AVRCP_INTERNAL_PASSTHROUGH_RES_T *res)
{
    /* Currently AVRCP does not send any data with Passthrough Response */

    /* Frame an AV/C message for PASSTHROUGH and send it */   
    avrcpSendAvcResponse(   avrcp, 
                            AVRCP_PASSTHROUGH_HEADER_SIZE, 
                            res->response, 0, (Source)NULL);

    return;
}

/*!
 ***************************************************************************
 *  @brief
 *  This function Handles the incoming PassThrough command 
 *  request from remote device
 *    
 *  @param avrcp                   pointer to AVRCP Task.
 *  @param ptr                     pointer to the received AVRCP command
 *  @param packet_size             packet size
 *    
 *  Expected Command looks like
 *
 *  <tt>
 *                     -----------------------------------------------
 *                    | MSB |     |     |     |     |      |   |  LSB |
 *                    |-----------------------------------------------|
 *  Octet0            |        0000           | ctype: CONTROL (0)    |
 *                    |-----------------------------------------------|
 *  octet1            |   Subunit_type        |   Subunit_ID          |
 *                    |-----------------------------------------------|
 *    opcode          |              PASSTHROUGH (0x7C)               |
 *                     -----------------------------------------------
 *    operand(0)      |state|           operation_id                  |
 *                      -----------------------------------------------
 *    operand(1)      |          operation data field length          |
 *                     -----------------------------------------------
 *    operand(2)      |     operation data(operation id dependant)    |
 *        :           |                                               |
 *                    -----------------------------------------------
 *  </tt>
 *
 * @returns void
 *******************************************************************************/
void avrcpHandlePassthroughCommand(AVRCP *avrcp, 
                                   const uint8 *ptr, 
                                   uint16 packet_size)
{
   uint16 op_data_len;
   uint16 total_pkt_len;

    /* Check the Request has Valid length */
    if(packet_size <  AVRCP_PASSTHROUGH_HEADER_SIZE) 
    {
        avrcpSendAvcResponse(avrcp, packet_size, avctp_response_rejected, 0,0 );
        return;
    }

    op_data_len = ptr[AVRCP_DATA_LEN_OFFSET] ;
    total_pkt_len = op_data_len + AVRCP_PASSTHROUGH_HEADER_SIZE;


    if(total_pkt_len > packet_size)
    {
        avrcpSendAvcResponse(avrcp, AVRCP_PASSTHROUGH_HEADER_SIZE, 
                             avctp_response_rejected, 0,0 );
        return;
    }
    else if(total_pkt_len < packet_size)
   {
       /* Correct the packet length */
       avrcp->av_msg_len -= (packet_size - total_pkt_len);
   }

    /* Check if there is a MetaGroup Command */
    if( (packet_size == AVRCP_PASSTHROUGH_GROUP_SIZE) &&
        ((ptr[AVRCP_OPID_OFFSET] & AVRCP3_PASSTHROUGH_OP_MASK) == 
          opid_vendor_unique) && 
          (avrcpGetCompanyId(ptr, AVRCP_COMPANY_ID_OFFSET ) == 
          AVRCP_BT_COMPANY_ID))
       
    {
        uint16 vendor_unique_id = (ptr[AVRCP_VENDOR_UNIQUE_ID_OFFSET]<< 8) |
                                   ptr[AVRCP_VENDOR_UNIQUE_ID_OFFSET+1];
        avrcpSendGroupIndToClient(avrcp, vendor_unique_id);
    }
    else
    {
        MAKE_AVRCP_MESSAGE_WITH_LEN(AVRCP_PASSTHROUGH_IND,op_data_len);

        avrcpBlockReceivedData(avrcp, avrcp_passthrough, 0);

        message->opid = (avc_operation_id) (ptr[AVRCP_OPID_OFFSET] &
                                            AVRCP3_PASSTHROUGH_OP_MASK);

#ifdef AVRCP_ENABLE_DEPRECATED 
        {
            /* Deprecated fields will be removed later */
            uint8 transaction = (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] 
                           & AVCTP_TRANSACTION_MASK)  >> AVCTP0_TRANSACTION_SHIFT;

            message->transaction = transaction;
        }
#endif

        message->state = ptr[AVRCP_OPID_OFFSET] & AVRCP3_PASSTHROUGH_STATE_MASK ? 1 : 0;
        message->subunit_type = (avc_subunit_type) ((ptr[AVRCP_SUBUNIT_OFFSET] & 
                      AVRCP1_SUBUNIT_TYPE_MASK) >> AVRCP1_SUBUNIT_TYPE_SHIFT);
        message->subunit_id = ptr[AVRCP_SUBUNIT_OFFSET] & AVRCP1_SUBUNIT_ID_MASK;
        message->size_op_data = op_data_len;
        memmove(&message->op_data[0],&ptr[AVRCP_DATA_LEN_OFFSET+1],op_data_len);
        message->sink = avrcp->sink;
        message->avrcp = avrcp;
                        
        MessageSend(avrcp->clientTask, AVRCP_PASSTHROUGH_IND, message);
    }
}


/*!
 ***************************************************************************
 *  @brief
 *    This is the Internal function for sending the Group Response
 *    
 *  @param avrcp                   pointer to AVRCP Task.
 *  @param res                     AVRCP response
 *
 *  <tt>
 *                   -----------------------------------------------
 *                  | MSB |    |      |     |      |     |     |LSB |
 *                  |-----------------------------------------------|
 *  octet 0         |       0x0             |  Ctype: 0x0(Control)  |
 *                  |-----------------------------------------------|
 *  octet 1         |  Sub Unit_type        |  Sub_unit ID : 0x0    |
 *                  |-----------------------------------------------|
 *  octet 2         |              PASSTHROUGH (0x7C)               |
 *                  | ----------------------------------------------|
 *    octet 3       |state|            operation_id(0x7E)           |
 *                  | ----------------------------------------------|
 *    octet 4       |          operation data field length:0x05     |
 *                  | ----------------------------------------------|
 *    octet 5 - 7   |     BT SIG Company ID                         |
 *                  |-----------------------------------------------|
 *  octet 8-9       |              Vendor Unique ID                 |
 *                  |-----------------------------------------------|
 *  </tt>
 *
 *  @returns void
 *****************************************************************************/
void avrcpHandleInternalGroupResponse(AVRCP *avrcp, 
            const AVRCP_INTERNAL_GROUP_RES_T *res)
{
    /* Header will be copied from the Request message */
     avrcpSendAvcResponse(  avrcp, 
                            AVRCP_GROUP_HEADER_SIZE, 
                            res->response, 0, (Source)NULL);
}



/**************************************************************************/
void avrcpSendGroupIndToClient(AVRCP *avrcp, uint16 vendor_id)
{
    if (!(avrcpGetDeviceTask()->bitfields.local_target_features & 
         AVRCP_GROUP_NAVIGATION))
    {
        sendGroupResponse(avrcp, avctp_response_not_implemented);
        return;
    }
    else
    {
        if (vendor_id)
        {
            avrcpBlockReceivedData(avrcp, avrcp_previous_group, 0);
            avrcpSendCommonMetadataInd(avrcp, AVRCP_PREVIOUS_GROUP_IND);
        }
        else
        {
            avrcpBlockReceivedData(avrcp, avrcp_next_group, 0);
            avrcpSendCommonMetadataInd(avrcp, AVRCP_NEXT_GROUP_IND);
        }
    }
}

#endif /* !AVRCP_CT_ONLY_LIB*/

