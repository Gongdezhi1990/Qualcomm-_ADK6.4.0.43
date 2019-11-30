/****************************************************************************
Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_packet.c
DESCRIPTION
    Handling of iAP2 packet to be sent to the remote device.
*/

#include <connection_no_ble.h>
#include <vm.h>
#include <sink.h>
#include "iap2_packet.h"
#include "iap2_link.h"
#include "iap2_link_setup.h"
#include "iap2_cp.h"

iap2_packet *iap2PacketAllocate(uint16 size_payload)
{
    iap2_packet *packet = (iap2_packet*)malloc(sizeof(iap2_packet));

    if (packet)
    {
        memset(packet, 0, sizeof(iap2_packet));
        packet->task.handler = iap2PacketHandler;
        packet->session_type = iap2_session_type_none;

        if (size_payload)
        {
            packet->size_payload = size_payload;
            packet->payload = malloc(size_payload);

            if (!packet->payload)
            {
                free(packet);
                return NULL;
            }
        }
    }

    return packet;
}

void iap2PacketDeallocate(iap2_packet *packet)
{
    if (packet)
    {
        if (packet->payload && !packet->static_payload)
            free(packet->payload);

        MessageFlushTask(&packet->task);
        free(packet);
    }
}

bool iap2PacketValidate(iap2_link *link, iap2_packet *packet)
{
    iap2_packet *temp_packet = link->tx_queue;

    /* Find the packet in the tx queue */
    if (!packet)
        return FALSE;

    while (temp_packet && temp_packet != packet)
        temp_packet = temp_packet->next;

    if (!temp_packet)
        return FALSE;

    return TRUE;
}

bool iap2PacketQueueSlack(iap2_link *link)
{
    /* Check the packets in queue */
    uint16 queue_size = 0;
    iap2_packet *packet = link->tx_queue;

    while (packet)
    {
        queue_size++;
        packet = packet->next;
    }

    return (queue_size < MIN(link->param.MaxNumOutstandingPackets, IAP2_MAX_TX_QUEUE_SIZE));
}

iap2_status_t iap2PacketEnqueue(iap2_link *link, iap2_packet *packet)
{
    iap2_status_t status = iap2_status_success;
    iap2_packet *temp_packet = link->tx_queue;

    /* Calculate seq number */
    if (!temp_packet)
    {
        packet->seq_num = link->last_sent_seq + 1;
    }
    else
    {
        while (temp_packet->next)
            temp_packet = temp_packet->next;

        packet->seq_num = temp_packet->seq_num + 1;
    }

    DEBUG_PRINT_PACKET(("iAP2 packet enqueue link=0x%p packet=0x%p seq=%d\n", (void*)link, (void*)packet, packet->seq_num));

    if ((status = iap2PacketFlush(link, packet)) == iap2_status_success)
    {
        link->last_sent_seq = packet->seq_num;

        /* Add the packet to the end of the tx queue if we are not a ZeroACK link */
        if (!link->param.supportZeroAckLink)
        {
            packet->next = NULL;

            if (!link->tx_queue)
                link->tx_queue = packet;
            else
                temp_packet->next = packet;
        }
        else
            iap2PacketDeallocate(packet);
    }

    return status;
}

iap2_status_t iap2PacketFlush(iap2_link *link, iap2_packet *packet)
{
    /* Packet len = 9 byte header + payload header + payload + payload checksum */
    iap2_status_t status = iap2_status_success;
    uint16 packet_len = IAP2_PACKET_HEADER_LEN + packet->size_payload + 1;
    uint16 claim_len = IAP2_PACKET_HEADER_LEN;

    if (packet->session_type == iap2_session_type_ctrl)
    {
        packet_len += IAP2_PACKET_CTRL_HEADER_LEN;
        claim_len += IAP2_PACKET_CTRL_HEADER_LEN;
    }
    else if (packet->session_type == iap2_session_type_ea)
    {
        packet_len += IAP2_PACKET_EA_HEADER_LEN;
        claim_len += IAP2_PACKET_EA_HEADER_LEN;
    }

    if (packet->id.ctrl_message == (uint16)AuthenticationCertificate ||
        packet->id.ctrl_message == (uint16)AuthenticationResponse)
    {
        /* Two special cases: AuthenticationCertificate & AuthenticationResponse */
        /* Both message payload is going to be read from CP and written to Sink directly */
        claim_len += IAP2_CTRL_PARAM_HEADER_LEN;
    }
    else
        claim_len += packet->size_payload + 1;

    if (SinkSlack(link->sink) >= claim_len)
    {
        uint8 checksum = 0;
        uint8 *header = PanicUnlessMalloc(IAP2_PACKET_HEADER_LEN - 1);
        uint8 *sink_ptr = SinkMap(link->sink) + SinkClaim(link->sink, claim_len);

        /* Increment retransmit count */
        packet->retrans_count++;

        header[iap2_packet_sop_msb] = 0xFF;
        header[iap2_packet_sop_lsb] = 0x5A;
        header[iap2_packet_len_msb] = (packet_len >> 8) & 0xff;
        header[iap2_packet_len_lsb] = packet_len & 0xff;

        if (link->state == iap2_link_initialised)
            header[iap2_packet_ctrl_byte] = iap2_ctrl_bit_syn;
        else if (link->state == iap2_link_sync_retry)
            header[iap2_packet_ctrl_byte] = iap2_ctrl_bit_syn | iap2_ctrl_bit_ack;
        else
            header[iap2_packet_ctrl_byte] = iap2_ctrl_bit_ack;

        header[iap2_packet_seq_num] = packet->seq_num;

        if (link->param.supportZeroAckLink ||
            ((uint8)(link->last_received_seq - link->last_sent_ack) & 0xff) < MAX(link->param.MaxCumulativeAcks, 1))
            header[iap2_packet_ack_num] = link->last_received_seq;
        else
            header[iap2_packet_ack_num] = ((link->last_sent_ack + MAX(link->param.MaxCumulativeAcks, 1)) & 0xff);

        if (packet->session_type == iap2_session_type_ctrl)
            header[iap2_packet_session_id] = link->session_id_ctrl;
        else if (packet->session_type == iap2_session_type_ea)
            header[iap2_packet_session_id] = link->session_id_ea;
        else if (packet->session_type == iap2_session_type_ftp)
            header[iap2_packet_session_id] = link->session_id_ftp;
        else
            header[iap2_packet_session_id] = 0;

        DEBUG_PRINT_PACKET(("iAP2 packet flush link=0x%p packet=0x%p seq=%d ack=%d\n", (void*)link, (void*)packet, packet->seq_num, header[iap2_packet_ack_num]));

        /* Update the last send ACK */
        if (header[iap2_packet_ctrl_byte] & iap2_ctrl_bit_ack)
            link->last_sent_ack = header[iap2_packet_ack_num];

        checksum -= iap2Checksum(header, IAP2_PACKET_HEADER_LEN - 1);

        memmove(sink_ptr, header, IAP2_PACKET_HEADER_LEN - 1);
        sink_ptr += IAP2_PACKET_HEADER_LEN - 1;
        *sink_ptr++ = checksum & 0xff;

        free(header);

        /* Process packet payload */
        checksum = 0;

        if (packet->session_type == iap2_session_type_ctrl)
        {
            /* Control message header */
            *sink_ptr++ = 0x40;
            checksum += 0x40;
            *sink_ptr++ = 0x40;
            checksum += 0x40;
            *sink_ptr++ = ((packet->size_payload + IAP2_PACKET_CTRL_HEADER_LEN) >> 8) & 0xff;
            checksum += ((packet->size_payload + IAP2_PACKET_CTRL_HEADER_LEN) >> 8) & 0xff;
            *sink_ptr++ = (packet->size_payload + IAP2_PACKET_CTRL_HEADER_LEN) & 0xff;
            checksum += (packet->size_payload + IAP2_PACKET_CTRL_HEADER_LEN) & 0xff;
            *sink_ptr++ = (packet->id.ctrl_message >> 8) & 0xff;
            checksum += (packet->id.ctrl_message >> 8) & 0xff;
            *sink_ptr++ = packet->id.ctrl_message & 0xff;
            checksum += packet->id.ctrl_message & 0xff;

            if (packet->id.ctrl_message == (uint16)AuthenticationCertificate ||
                packet->id.ctrl_message == (uint16)AuthenticationResponse)
            {
                /* Two special cases: AuthenticationCertificate & AuthenticationResponse */
                /* Both messages are read from CP and has parameter ID 0 */
                sink_ptr += Iap2CtrlParamWriteHeader(sink_ptr, 0, packet->size_payload - IAP2_CTRL_PARAM_HEADER_LEN);

                checksum += (packet->size_payload >> 8) & 0xff;
                checksum += packet->size_payload & 0xff;

                if (packet->id.ctrl_message == (uint16)AuthenticationCertificate)
                    iap2CpReadAuthCert(link, packet->size_payload - IAP2_CTRL_PARAM_HEADER_LEN, checksum);
                else if (packet->id.ctrl_message == (uint16)AuthenticationResponse)
                    iap2CpReadChallengeRsp(link, packet->size_payload - IAP2_CTRL_PARAM_HEADER_LEN, checksum);
            }
            else if (packet->id.ctrl_message == (uint16)IdentificationInformation)
            {
                /* Write IdentificationInformation and checksum directly to Sink to save VM buffer */
                iap2IdentInfoPayloadGenerate(sink_ptr);
                /* Calculate the checksum on Sink buffer. */
                /* It is slow but thinkging it is done only one time during link setup it shouldn't affect too much */
                checksum += iap2Checksum(sink_ptr, packet->size_payload);
                sink_ptr += packet->size_payload;
                /* Write the checksum */
                *sink_ptr = (0 - checksum) & 0xff;
            }
            else
            {
                /* Write the paylod */
                if (packet->size_payload)
                {
                    memmove(sink_ptr, packet->payload, packet->size_payload);
                    sink_ptr += packet->size_payload;
                    checksum += iap2Checksum(packet->payload, packet->size_payload);
                }

                /* Write the checksum */
                *sink_ptr = (0 - checksum) & 0xff;
            }
        }
        else
        {
            if (packet->session_type == iap2_session_type_ea)
            {
                /* EA message header */
                *sink_ptr++ = (packet->id.ea_session >> 8) & 0xff;
                checksum += (packet->id.ea_session >> 8) & 0xff;
                *sink_ptr++ = packet->id.ea_session & 0xff;
                checksum += packet->id.ea_session & 0xff;
            }

            /* Write the paylod */
            if (packet->size_payload)
            {
                memmove(sink_ptr, packet->payload, packet->size_payload);
                sink_ptr += packet->size_payload;
                checksum += iap2Checksum(packet->payload, packet->size_payload);
            }

            /* Write the checksum */
            *sink_ptr = (0 - checksum) & 0xff;
        }

        /* Flush the packet */
        SinkFlush(link->sink, claim_len);
    }
    else
    {
        DEBUG_PRINT_PACKET(("iAP2 packet flush failed link=0x%p packet=0x%p seq=%d\n", (void*)link, (void*)packet, packet->seq_num));
        status = iap2_status_out_of_resources;
    }

    if (link->state < iap2_link_sync_done ||
        (!link->param.supportZeroAckLink && link->state >= iap2_link_ready))
    {
        /* Schedule a retransmit message for this packet */
        MAKE_MESSAGE(msg, IAP2_INTERNAL_PACKET_RETRANS_REQ);
        msg->link = link;
        MessageSendLater(&packet->task,
                         IAP2_INTERNAL_PACKET_RETRANS_REQ,
                         msg,
                         (link->state < iap2_link_sync_done) ? D_SEC(1) : link->param.RetransmissionTimeout);
        DEBUG_PRINT_PACKET(("iAP2 packet retransmit scheduled link=0x%p packet=0x%p seq=%d\n", (void*)link, (void*)packet, packet->seq_num));
    }

    /* It's safe to cancel ack req if we do not have any packets to ack */
    if (link->last_received_seq == link->last_sent_ack)
        MessageCancelAll(&link->task, IAP2_INTERNAL_ACK_FLUSH_REQ);

    return status;
}

void iap2PacketHandleAck(iap2_link *link, uint8 ack_num)
{
    iap2_packet *packet = link->tx_queue;
    iap2_packet *temp;

    DEBUG_PRINT_PACKET(("iAP2 packet handle ACK link=0x%p ack=%d\n", (void*)link, ack_num));

    while (packet && packet->seq_num != ack_num)
        packet = packet->next;

    if (packet)
    {
        /* All packets before this one should be released */
        do
        {
            temp = link->tx_queue;
            link->tx_queue = link->tx_queue->next;
            DEBUG_PRINT_PACKET(("iAP2 packet dequeue link=0x%p packet=0x%p seq=%d\n", (void*)link, (void*)temp, temp->seq_num));
            iap2PacketDeallocate(temp);
        } while (temp != packet);
    }
}

void iap2PacketAckFlush(iap2_link *link, uint8 ack_num)
{
    if (SinkSlack(link->sink) >= IAP2_PACKET_HEADER_LEN)
    {
        uint8 header[IAP2_PACKET_HEADER_LEN - 1];
        uint8 *sink_ptr = SinkMap(link->sink) + SinkClaim(link->sink, IAP2_PACKET_HEADER_LEN);

        header[iap2_packet_sop_msb] = 0xFF;
        header[iap2_packet_sop_lsb] = 0x5A;
        header[iap2_packet_len_msb] = 0;
        header[iap2_packet_len_lsb] = IAP2_PACKET_HEADER_LEN;
        header[iap2_packet_ctrl_byte] = iap2_ctrl_bit_ack;
        header[iap2_packet_seq_num] = link->last_sent_seq;
        header[iap2_packet_ack_num] = ack_num;
        header[iap2_packet_session_id] = 0;

        memmove(sink_ptr, header, IAP2_PACKET_HEADER_LEN - 1);
        sink_ptr += IAP2_PACKET_HEADER_LEN - 1;
        *sink_ptr = (0 - iap2Checksum(header, IAP2_PACKET_HEADER_LEN - 1)) & 0xff;
        SinkFlush(link->sink, IAP2_PACKET_HEADER_LEN);

        link->last_sent_ack = ack_num;

        DEBUG_PRINT_PACKET(("iAP2 packet ack sent link=0x%p ack=%d\n", (void*)link, ack_num));
    }

    /* It's safe to cancel ack req if we do not have any packets to ack */
    if (link->last_received_seq == link->last_sent_ack)
        MessageCancelAll(&link->task, IAP2_INTERNAL_ACK_FLUSH_REQ);
}

void iap2HandleInternalAckFlushReq(iap2_link *link)
{
    /* Make sure we did not send this ACK yet */
    if (link->last_sent_ack != link->last_received_seq)
        iap2PacketAckFlush(link, link->last_received_seq);
}

void iap2HandleInternalPacketRetransReq(iap2_packet *packet, IAP2_INTERNAL_PACKET_RETRANS_REQ_T *req)
{
    if (iap2LinkValidate(req->link) && iap2PacketValidate(req->link, packet))
    {
        /* Packet is still in the queue, check retransmit count */
        if ((req->link->state < iap2_link_sync_done && packet->retrans_count > 30) ||
            (req->link->state >= iap2_link_sync_done && packet->retrans_count > req->link->param.MaxNumRetransmissions))
        {
            MAKE_MESSAGE_WITH_STATUS(msg, IAP2_DISCONNECT_IND, iap2_status_max_retransmission);

            DEBUG_PRINT_PACKET(("iAP2 packet retransmit too many link=0x%p packet=0x%p seq=%d retrans=%d\n",
                                (void*)req->link, (void*)packet, packet->seq_num, packet->retrans_count));

            ConnectionRfcommDisconnectRequest(&req->link->task, req->link->sink);
            msg->link = req->link;
            msg->bd_addr = req->link->remote_addr;
            MessageSend(iap2_lib->app_task, IAP2_DISCONNECT_IND, msg);
            req->link->state = iap2_link_error;
        }
        else
        {
            /* We need to resend the packet */
            DEBUG_PRINT_PACKET(("iAP2 packet retransmit link=0x%p packet=0x%p seq=%d retrans=%d\n",
                                (void*)req->link, (void*)packet, packet->seq_num, packet->retrans_count));

            iap2PacketFlush(req->link, packet);
        }
    }
}

