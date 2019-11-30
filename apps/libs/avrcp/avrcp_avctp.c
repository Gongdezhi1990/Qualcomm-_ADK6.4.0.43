/****************************************************************************
Copyright (c) 2009 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    avrcp_avctp.c

DESCRIPTION
    Functions in this file handles AVCTP protocol transactions and 
    AVCTP message handling. All function prototypes are defined in 
    avrcp_signal_handler.h
*/


/****************************************************************************
    Header files
*/
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include "avrcp_common.h"
#include "avrcp_browsing_handler.h"

/*
 * helper macro for nested function call used a lot in
 * avrcpAvctpReceiveMessage()
 */
#define DROP_PACKET() SourceDrop(StreamSourceFromSink(avrcp->sink),packet_size)

#define VALID_PID(PTR,IDX) (((PTR)[(IDX)]==AVCTP1_PROFILE_AVRCP_HIGH) && ((PTR)[(IDX)+1]==AVCTP2_PROFILE_AVRCP_REMOTECONTROL))

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */
/****************************************************************************
*NAME    
*    avrcpAvctpSetCmdHeader    
*
*DESCRIPTION
*    This function is called to set the AVCTP header for START or SINGLE Packet
*    for commands.For Continue or End Packet call the Macro 
*    AVCTP_SET_PKT_HEADER.
*    
*PARAMETERS
*   avrcp         - Task
*   *ptr          - Pointer to the Start of AVCTP Packet in Sink 
*   packet_type   - MUST be AVCTP0_PACKET_TYPE_SINGLE/AVCTP0_PACKET_TYPE_START.
*   total_packets - Total Number of Packets
*
*RETURN
*******************************************************************************/
void avrcpAvctpSetCmdHeader(AVRCP *avrcp, 
                            uint8 *ptr, 
                            uint8 packet_type, 
                            uint8 total_packets)
{
    /* AVCTP header */
    ptr[0] = (uint8)((avrcpGetNextTransactionLabel(avrcp) << AVCTP0_TRANSACTION_SHIFT)
                     | packet_type 
                     | AVCTP0_CR_COMMAND);
    if (packet_type == AVCTP0_PACKET_TYPE_SINGLE)
    {
        ptr[1] = AVCTP1_PROFILE_AVRCP_HIGH;
        ptr[2] = AVCTP2_PROFILE_AVRCP_REMOTECONTROL;
    }
    else if (packet_type == AVCTP0_PACKET_TYPE_START)
    {
        ptr[1] = total_packets;
        ptr[2] = AVCTP1_PROFILE_AVRCP_HIGH;
        ptr[3] = AVCTP2_PROFILE_AVRCP_REMOTECONTROL;
    }
}

#endif /* !AVRCP_TG_ONLY_LIB */


/****************************************************************************
*NAME    
*    avrcpAvctpSendMessage    
*
*DESCRIPTION
*    This function is called to Frame the AVCTP messages (Command or Response)
*   and send it to the peer. Before calling this function, fill the AVRCP 
*   Message  Header in the sink. For sending a response, copy the AVCTP 
*   header received to *ptr before calling the function. Set the Profile ID 
*   in the response. Set the hdr_size=0 (no avrcp header) for
*   bad_profile response.   
*    
*PARAMETERS
*   avrcp            - Task
*   cr_type          - set to 0 for command and 2 for response    
*   ptr              - Pointer to the Start of AVCTP Packet in Sink 
*   hdr_size         - Header size of AVRCP message already in the sink 
*   data_len         - Length of AVRCP message data to send
*   data             - AVRCP Message data to append after AVRCP message head    
*
*RETURN
*   avrcp_status_code
*******************************************************************************/

avrcp_status_code avrcpAvctpSendMessage( AVRCP     *avrcp,
                                         uint8      cr_type,
                                         uint8     *ptr,
                                         uint16     hdr_size,
                                         uint16     data_len,
                                         Source     data)   
{
    uint8  avctp_pkt_type = AVCTP0_PACKET_TYPE_SINGLE;
    uint8  start_head;
    unsigned i;
    uint8  no_complete_packets=0;
    unsigned msg_len=data_len;
    unsigned pkt_size = (unsigned)(hdr_size+data_len+AVCTP_SINGLE_PKT_HEADER_SIZE);
    avrcp_status_code result = avrcp_success;
    Sink sink=avrcp->sink;

    /* 
     * Check the packet size is greater than L2CAP MTU Size. Fragmentation is 
     * required for AVCTP packets greater than MTU Size
     */
    if(pkt_size >  avrcp->l2cap_mtu)
    {
        avctp_pkt_type = AVCTP0_PACKET_TYPE_START;
    }

    if(avctp_pkt_type ==  AVCTP0_PACKET_TYPE_START)
    {
        msg_len = (unsigned)(avrcp->l2cap_mtu - (hdr_size + AVCTP_START_PKT_HEADER_SIZE));

        /* Calculate the Number of complete_packets.
         * Note that this appears to end up 1 larger than necessary. This is 
         * probably due to the adjustment in msg_len above.
         */
        no_complete_packets = (uint8)(((data_len - msg_len )
                                     / (unsigned)(avrcp->l2cap_mtu - AVCTP_CONT_PKT_HEADER_SIZE))
                                     + 1);

        /* If there is a reminder of bytes add 1 more */
        if((data_len - msg_len ) % 
            (unsigned)(avrcp->l2cap_mtu - AVCTP_CONT_PKT_HEADER_SIZE))
            no_complete_packets++;
    
        /* Packet size */
        pkt_size = avrcp->l2cap_mtu ;

        /* Fill no_complete_packets to the header */
        ptr[AVCTP_NUM_PKT_OFFSET] = no_complete_packets;
    }

    /* Fill the AVCTP Header */
    if(cr_type == AVCTP0_CR_COMMAND)
    {
        /* Frame the AVCTP header for command. no_complete_packets required
           for start packet is already filled in the header. For a
           Target only library this code is unreachable. */
        avrcpAvctpSetCmdHeader( avrcp,
                                 &ptr[AVCTP_HEADER_START_OFFSET],
                                  avctp_pkt_type, 
                                  no_complete_packets);
    }
    else
    { 
        /* Complete the AVCTP header for response. no_complete_packets and
           PID are already populated. For a Controller only library, this
           code is unreachable. */
        ptr[AVCTP_HEADER_START_OFFSET] =
            (uint8)((ptr[AVCTP_HEADER_START_OFFSET] & AVCTP_TRANSACTION_MASK) |
            avctp_pkt_type | AVCTP0_CR_RESPONSE);

        
        /* Set Bad Profile if there is no AVRCP header to follow */
        if(!hdr_size)
        {
            /* As per AVCTP spection and section 7.2 Handling of Messages for Not Registered Profiles. A target AVCTP entity shall not 
              * hand down command messages that do not carry a registered PID: in this case AVCTP shall directly return a response message 
              * made of the 3-octets packet header with the PID value copied from the command and the IPID bit set to 1. The message shall 
              * not contain any response frame data. */

            ptr[AVCTP_HEADER_START_OFFSET] |= AVCTP0_IPID;
            memmove(&ptr[AVCTP_SINGLE_PKT_PID_OFFSET],
                    &avrcp->av_msg[AVCTP_SINGLE_PKT_PID_OFFSET],
                    AVCTP_PID_SIZE);
        } 

    }

    /* Store first octet for future */
    start_head = ptr[AVCTP_HEADER_START_OFFSET];

    /* Before calling this function , Sink Space must be checked for 
     * first message If StreamMove() fails, Target may not get the 
     * entire data and it may drop 
     */
      
    if(msg_len)
    {
        StreamMove(sink, data, (uint16)msg_len);

        /* Reduce the data length */
        data_len = (uint16)(data_len - msg_len);
    }

    /* Send the data */
    (void)SinkFlush(sink, (uint16)pkt_size);

    /* Send the rest of AVCTP fragments. Start with 2 since we already sent 1 */
    for (i = 2 ; i <= no_complete_packets; i++)
    {
        ptr= avrcpGrabSink(sink, AVCTP_CONT_PKT_HEADER_SIZE);
        if(!ptr)
        {
            result = avrcp_no_resource;
            break;
        }
        
        if(i < no_complete_packets)
        {
            AVCTP_SET_CONT_PKT_HEADER(ptr[AVCTP_HEADER_START_OFFSET],
                                      start_head);
            msg_len = (uint16)(avrcp->l2cap_mtu - AVCTP_CONT_PKT_HEADER_SIZE);
        }
        else
        {
            AVCTP_SET_END_PKT_HEADER(ptr[AVCTP_HEADER_START_OFFSET],start_head);
            msg_len = data_len;
            pkt_size = msg_len + AVCTP_END_PKT_HEADER_SIZE;
        }

        /*  Copy the data to Sink. Sink Space must be validated 
            before calling this function.*/
        StreamMove(sink, data , (uint16)msg_len);

           /* Send the data */
        (void)SinkFlush(sink, (uint16)pkt_size);

        data_len = (uint16)(data_len - msg_len);
    }

    /* If Still Data to send in Source. Drop that much data */
    if (data_len)
    {
        SourceDrop(data,data_len);
    }

    return result;
}

static void removeFragments (AVRCP *avrcp)
{
    if((avrcp->bitfields.fragment != avrcp_packet_type_single) && avrcp->av_msg_len)
    {
        free(avrcp->av_msg);
        avrcp->av_msg_len = 0;
        avrcp->av_msg = NULL;
        avrcp->bitfields.fragment = avrcp_packet_type_single; /* Only fragmented packets use a malloc'd data area */
    }
    avrcp->avctp_packets_remaining = 0;
}

/****************************************************************************
*NAME    
*    avrcpAvctpReceiveMessage    
*
*DESCRIPTION
*    This function is called to process the AVCTP message received. 
*    
*PARAMETERS
*   avrcp                  - Task
*   * ptr                  - Received Data
*   packet_size            - packet_size
*
*RETURN
*  bool - Return TRUE if the message is ready to process, otherwise return 
*         FALSE
******************************************************************************/
bool avrcpAvctpReceiveMessage(AVRCP *avrcp,
                              const uint8 *ptr,
                              uint16 packet_size)
{
    bool    result = FALSE;
    uint16  packet_type = ptr[AVCTP_HEADER_START_OFFSET] & AVCTP0_PACKET_TYPE_MASK;
    uint8   cr_type = ptr[AVCTP_HEADER_START_OFFSET] & AVCTP0_CR_MASK; 
    uint8 *new_buffer;
    uint16 payload_size;

    switch (packet_type)
    {
    case AVCTP0_PACKET_TYPE_SINGLE: /* Fall through */
        if (packet_size < AVRCP_TOTAL_HEADER_SIZE)
        {   /* Bad Packet. Drop it */
            DROP_PACKET();
            break;
        }

        /* Drop any partially reconstructed message fragments */
        removeFragments( avrcp );

        /* Check the PID Values */
        if (!VALID_PID( ptr, AVCTP_SINGLE_PKT_PID_OFFSET ))
        {
        #ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
            if(cr_type == AVCTP0_CR_COMMAND)
            {
                avrcp->av_msg = (uint8*)ptr;
                avrcp->av_msg_len = packet_size;
                /* TODO: AV/C should not be involved with an AVCTP bad PID: */
                avrcpSendAvcResponse(avrcp,0,avctp_response_bad_profile, 0, (Source)NULL);
            }
            else
        #endif /* AVRCP_CT_ONLY_LIB */
            {
                DROP_PACKET();
            }
            break;
        }

        avrcp->av_msg = (uint8*)ptr;
        avrcp->av_msg_len = packet_size;
        avrcp->bitfields.fragment = avrcp_packet_type_single;
        result = TRUE;
        break;

    case AVCTP0_PACKET_TYPE_START:
        /* Drop any partially reconstructed message fragments */
        removeFragments( avrcp );

        /* There are a numerous circumstances when we would not expect to
         * receive a fragmented message, but there is nothing to be gained
         * from enforcing these restrictions.
         * The only crucial condition is that the total number of packets
         * must be more than 1 (so that we know the end packet has arrived at
         * the correct time)
         */
        if (packet_size < AVCTP_START_PKT_HEADER_SIZE
                || ptr[AVCTP_NUM_PKT_OFFSET] < 2)
        {   /* Bad Packet. Drop it */
            DROP_PACKET();
            break; 
        }

        /* Check the PID Values */
        if (!VALID_PID( ptr, AVCTP_START_PKT_PID_OFFSET ))
        {
        #ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
            if(cr_type == AVCTP0_CR_COMMAND)
            {
                avrcp->av_msg = (uint8*)ptr;
                avrcp->av_msg_len = packet_size;
                /* TODO: AV/C should not be involved with an AVCTP bad PID: */
                avrcpSendAvcResponse(avrcp,0,avctp_response_bad_profile, 0, (Source)NULL);
            }
            else
        #endif /* AVRCP_CT_ONLY_LIB */
            {
                DROP_PACKET();
            }
            break;
        }

        /* Begin reconstruction of fragmented message, off source */
        avrcp->av_msg = (uint8 *) malloc( packet_size );
        if (avrcp->av_msg == NULL)
        {
            /* No memory for the message buffer so just drop it.
             * Subsequent fragments will be dropped, too because
             * avctp_packets_remaining is still 0
             */
            DROP_PACKET();
            break;
        }
        memmove(avrcp->av_msg, ptr, packet_size);
        avrcp->av_msg_len = packet_size;
        
        /* Set the number of fragments to come */
        avrcp->avctp_packets_remaining = (uint8)(ptr[AVCTP_NUM_PKT_OFFSET] - 1);

        avrcp->bitfields.fragment = avrcp_packet_type_start;
        
        /* Drop message fragment as it has been copied to a separate area for reconstruction */
        DROP_PACKET();
        break;

    case AVCTP0_PACKET_TYPE_CONTINUE:
    case AVCTP0_PACKET_TYPE_END:  
        if ((avrcp->avctp_packets_remaining == 0)
            || (packet_size < AVCTP_CONT_PKT_HEADER_SIZE)
            || ((ptr[AVCTP_HEADER_START_OFFSET] & AVCTP_TRANSACTION_MASK) !=
                (avrcp->av_msg[AVCTP_HEADER_START_OFFSET] & AVCTP_TRANSACTION_MASK)))
        {  
            /* Bad Packet. Drop it */
            removeFragments(avrcp);
            DROP_PACKET();
            break;
        }

        /*
         * Copy the payload to an enlarged buffer,
         * skipping over the AVCTP header
         */
        payload_size = (uint16)(packet_size - AVCTP_CONT_PKT_HEADER_SIZE);
        new_buffer = (uint8 *) realloc(avrcp->av_msg, (size_t)(avrcp->av_msg_len + payload_size));

        if (!new_buffer)
        {
            /* realloc couldn't find enough memory */
            removeFragments(avrcp);  /* this will free the existing buffer which
                                        realloc will have preserved */
            DROP_PACKET();
            break;
        }
        avrcp->av_msg = new_buffer;
        memmove(avrcp->av_msg + avrcp->av_msg_len,
                ptr + AVCTP_CONT_PKT_HEADER_SIZE, payload_size);
        avrcp->av_msg_len = (uint16)(avrcp->av_msg_len + payload_size);
        
        avrcp->avctp_packets_remaining--;
        
        if((packet_type == AVCTP0_PACKET_TYPE_END)
                && (avrcp->avctp_packets_remaining == 0))
        {  
            /* Last fragment received at the right time so we can now process
             * the whole message */
            avrcp->bitfields.fragment = avrcp_packet_type_end;
            result = TRUE;
        }
        else if ((packet_type == AVCTP0_PACKET_TYPE_END)
                || (avrcp->avctp_packets_remaining == 0))
        {
            /*
             * either the END packet arrived early (indicating an L2CAP packet
             * was lost in transmission) or too many continue packets were sent
             * (due to a bug in the sender)
             */
            removeFragments(avrcp);
        }
        else
        {   /* More fragments to come */
            avrcp->bitfields.fragment = avrcp_packet_type_continue;
        }

        /* Drop message fragment as it has been copied to a separate area for reconstruction */
        DROP_PACKET();
        break;
        
    default:
       /* Unreachable */
       DROP_PACKET();
       break;
    }

    return result;
}

/****************************************************************************
 *NAME    
 *   avbpAvctpProcessHeader  
 *
 *DESCRIPTION
 *  Validate and process the AVCTP header data in the received packet.
 *
 * PARAMETES
 * AVBP*        - Browsing Task
 * uint8*       - pointer to AVCTP packet
 * uint16       - packet_size
 *
 * RETURN
 * bool - FALSE on Failure and TRUE on Success
 ***************************************************************************/

bool avbpAvctpProcessHeader(    AVBP            *avbp, 
                                const uint8     *ptr, 
                                uint16          packet_size)
{
    if(packet_size < AVCTP_SINGLE_PKT_HEADER_SIZE)
    {
        return FALSE;
    }

    /* parse AVCTP Header*/
    if((ptr[AVCTP_HEADER_START_OFFSET] & AVCTP0_CR_MASK)
                                        == AVCTP0_CR_COMMAND)
    {
       /* Validate the PID Value for incoming command*/
        if((ptr[AVCTP_SINGLE_PKT_PID_OFFSET]!= AVCTP1_PROFILE_AVRCP_HIGH) ||
           (ptr[AVCTP_SINGLE_PKT_PID_OFFSET+1] != 
                                           AVCTP2_PROFILE_AVRCP_REMOTECONTROL)) 
        {
            uint8 *pid_err=avrcpGrabSink(avbp->avbp_sink,
                                         AVCTP_SINGLE_PKT_HEADER_SIZE);

            if(pid_err)
            {
                memmove(pid_err, ptr, AVCTP_SINGLE_PKT_HEADER_SIZE);
                pid_err[AVCTP_HEADER_START_OFFSET] |= AVCTP0_IPID | 
                                                      AVCTP0_CR_RESPONSE;
                (void)SinkFlush(avbp->avbp_sink,AVCTP_SINGLE_PKT_HEADER_SIZE);
            }
            return FALSE;
        }
    }
    else
    {
        /* Match the Outstanding Transaction ID */
        if((ptr[AVCTP_HEADER_START_OFFSET] >> AVCTP0_TRANSACTION_SHIFT) !=
             avbp->bitfields.trans_id)
        {
            AVRCP_INFO(("TRANSID Mismatch\n"));
            return FALSE;
        }

        if(ptr[AVCTP_HEADER_START_OFFSET] & AVCTP0_IPID)
        {
            AVRCP_INFO(("PID Error\n"));
            avbpSetChannelOnIncomingData(avbp, AVRCP_INVALID_PDU_ID, 0);
            return FALSE;
        }
    }

    /* Set the Outstanding Transaction ID now */
    avbp->bitfields.trans_id = ((uint8)(ptr[AVCTP_HEADER_START_OFFSET] >> AVCTP0_TRANSACTION_SHIFT) & 0x0F);

    return TRUE;
}


/****************************************************************************
 *NAME    
 *   avbpAvctpFrameHeader   
 *
 *DESCRIPTION
 *   Frame the AVCTP Header for Browsing packet
 *
 *PARAMETERS
 * AVBP     - Browsing Task 
 * uint8*   - Pointer to the Start of the claimed sink
 * bool     - TRUE if it is response
 ***************************************************************************/
void avbpAvctpFrameHeader(AVBP *avbp, uint8 *ptr , bool response)
{

    if(response)
    {
        ptr[AVCTP_HEADER_START_OFFSET] =  AVCTP0_CR_RESPONSE;        
    }
    else
    {
       ptr[AVCTP_HEADER_START_OFFSET] =  AVCTP0_CR_COMMAND;
    
        /* get Next Transaction ID */
       if(!(avbp->bitfields.trans_id++))
       {
            avbp->bitfields.trans_id = 1;
       }    
    }
 
    ptr[AVCTP_HEADER_START_OFFSET] |= (uint8)(avbp->bitfields.trans_id << AVCTP0_TRANSACTION_SHIFT);
    ptr[AVCTP_SINGLE_PKT_PID_OFFSET] = AVCTP1_PROFILE_AVRCP_HIGH;
    ptr[AVCTP_SINGLE_PKT_PID_OFFSET+1] = AVCTP2_PROFILE_AVRCP_REMOTECONTROL;
}

