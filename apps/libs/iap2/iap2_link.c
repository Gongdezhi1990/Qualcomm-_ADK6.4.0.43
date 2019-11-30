/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_link.c
DESCRIPTION
    Handles the link information and handling of received packet from remote device.
*/
#include <connection_no_ble.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include <vm.h>
#include "iap2_link.h"
#include "iap2_link_setup.h"
#include "iap2_packet.h"
#include "iap2_eap.h"

static void iap2LinkReset(iap2_link *link)
{
    MAKE_MESSAGE(msg, IAP2_LINK_RESET_IND);
    iap2_packet *packet, *packet_next;

    /* Reset link data */
    link->state = iap2_link_initialised;
    link->last_sent_seq = 0;
    link->last_sent_ack = 0;
    link->last_received_seq = 0;
    link->session_id_ctrl = DEFAULT_SESSION_ID_CTRL;
    link->session_id_ftp = DEFAULT_SESSION_ID_FTP;
    link->session_id_ea = DEFAULT_SESSION_ID_EA;

    /* Free the link resources */
    packet = link->tx_queue;
    while (packet)
    {
        packet_next = packet->next;
        iap2PacketDeallocate(packet);
        packet = packet_next;
    }

    link->tx_queue = NULL;

    msg->link = link;
    MessageSend(iap2_lib->app_task, IAP2_LINK_RESET_IND, msg);

    /* Send a message to ourselves to re-sync the link */
    MessageSend(&link->task, IAP2_INTERNAL_LINK_SYNC_REQ, NULL);
}

static void iap2LinkSleep(iap2_link *link)
{
    MAKE_MESSAGE(msg, IAP2_LINK_SLEEP_IND);
    iap2_packet *packet, *packet_next;

    /* Reset link data */
    link->state = iap2_link_initialised;
    link->last_sent_seq = 0;
    link->last_sent_ack = 0;
    link->last_received_seq = 0;
    link->session_id_ctrl = DEFAULT_SESSION_ID_CTRL;
    link->session_id_ftp = DEFAULT_SESSION_ID_FTP;
    link->session_id_ea = DEFAULT_SESSION_ID_EA;

    /* Free the link resources */
    packet = link->tx_queue;
    while (packet)
    {
        packet_next = packet->next;
        iap2PacketDeallocate(packet);
        packet = packet_next;
    }

    link->tx_queue = NULL;

    msg->link = link;
    MessageSend(iap2_lib->app_task, IAP2_LINK_SLEEP_IND, msg);
}

void Iap2IdentificationRequest(iap2_link *link)
{
    DEBUG_PRINT_LINK(("iAP2 link identification req link=0x%p\n", (void*)link));

    if (iap2LinkValidate(link) && (link->state == iap2_link_ident_start || link->state == iap2_link_auth_challenge_done))
    {
        link->state = iap2_link_ident_start;
        MessageSend(&link->task, IAP2_INTERNAL_LINK_IDENT_REQ, NULL);
    }
}

/*************************************************************************
NAME
    Iap2GetSlackForLink
    
DESCRIPTION
    Returns the available space in the sink associated with the given
    iAP2 link, accounting for the External Accessory Protocol overhead.
    
*/
uint16 Iap2GetSlackForLink(iap2_link *link)
{
    uint16 slack = 0;
    
    if (link && link->sink)
    {
        uint16 sink_slack = SinkSlack(link->sink);
        
        if (sink_slack >= IAP2_PACKET_HEADER_LEN + IAP2_PACKET_EA_HEADER_LEN + 1)
        {
            slack = sink_slack - IAP2_PACKET_HEADER_LEN - IAP2_PACKET_EA_HEADER_LEN - 1;
        }
    }
    
    return slack;
}

/* Allocate a link and add it to the linked list */
iap2_link *iap2LinkAllocate(iap2_transport_t transport)
{
    iap2_link *link = (iap2_link*)MALLOC(sizeof(iap2_link));

    DEBUG_PRINT_LINK(("iAP2 link allocate link=0x%p\n", (void*)link));

    if (link)
    {
        /* Reset the link data */
        memset(link, 0, sizeof(iap2_link));

        link->transport = transport;
        link->session_id_ctrl = DEFAULT_SESSION_ID_CTRL;
        link->session_id_ea = DEFAULT_SESSION_ID_EA;
        link->session_id_ftp = DEFAULT_SESSION_ID_FTP;

        /* Add the link to the head of the linked list */
        link->next = iap2_lib->links;
        iap2_lib->links = link;

        /* Add the link handler */
        link->task.handler = iap2LinkHandler;
    }

    return link;
}

/* Remove a link from the linked list and free the resource */
void iap2LinkDeallocate(iap2_link *link)
{
    DEBUG_PRINT_LINK(("iAP2 link deallocate link=0x%p\n", (void*)link));

    if (link)
    {
        iap2_packet *packet, *packet_next;

        /* Unregister the task */
        MessageFlushTask(&link->task);

        /* Empty the source */
        if (link->sink)
        {
            Source source = StreamSourceFromSink(link->sink);
            SourceDrop(source, SourceSize(source));
        }

        /* Locate and remove the link from the linked list */
        if (iap2_lib->links)
        {
            if (link == iap2_lib->links)
            {
                /* This is the head */
                iap2_lib->links = link->next;
            }
            else
            {
                iap2_link *temp_link = iap2_lib->links;

                while(temp_link)
                {
                    if (temp_link->next == link)
                    {
                        temp_link->next = link->next;
                        break;
                    }

                    temp_link = temp_link->next;
                }
            }
        }

        /* Free the link resources */
        packet = link->tx_queue;
        while (packet)
        {
            packet_next = packet->next;
            iap2PacketDeallocate(packet);
            packet = packet_next;
        }
        free(link);
    }
}


void iap2RemoveLinkFromSessionAndMessage(iap2_link *link)
{
    if(link)
    {
        uint16 session_id;

        while((session_id = iap2EAGetSessionIdFromLink(link)) != INVALID_SESSION_ID)
        {
            uint16 protocol_id = iap2EAGetSessionProtocol(session_id);
            Task protocol_task = iap2EAGetProtocolTask(protocol_id);

            DEBUG_PRINT_LINK(("iAP2 Stop EAP p=%u s=%u\n", protocol_id, session_id));

            if (protocol_task)
            {
                MAKE_MESSAGE(stop_ea_msg, IAP2_EA_SESSION_STOP_IND);

                DEBUG_PRINT_LINK(("iAP2 Stop EAP p=%u s=%u\n", protocol_id, session_id));

                stop_ea_msg->link = NULL;
                stop_ea_msg->protocol_id = protocol_id;
                stop_ea_msg->session_id = session_id;

                MessageSend(protocol_task, IAP2_EA_SESSION_STOP_IND, stop_ea_msg);
            }

            iap2EASetSessionLink(session_id, NULL);
            iap2EASetSessionProtocol(session_id, 0);
        }
    }
}

bool iap2LinkValidate(iap2_link *link)
{
    iap2_link *temp = iap2_lib->links;

    while (temp)
    {
        if (temp == link)
            return TRUE;

        temp = temp->next;
    }

    return FALSE;
}

void iap2HandleInternalLinkEakInd(iap2_link *link, const IAP2_INTERNAL_LINK_EAK_IND_T *ind)
{
    iap2_packet *packet = link->tx_queue;
    uint16 i;

    /* Payload contains a list of out-of-sequence seq numbers */
    /* Re-send the packets in the tx queue if they don't match any of the seq numbers in the payload */

    while (packet)
    {
        for (i = 0; i < ind->size_payload; i++)
        {
            if (packet->seq_num == ind->payload[i])
                break;
        }

        if (i >= ind->size_payload)
        {
            /* This packet need to be re-sent */
            iap2PacketFlush(link, packet);
        }

        packet = packet->next;
    }
}

void iap2HandleInternalLinkDetachReq(iap2_link *link)
{
    if (link->state == iap2_link_detaching)
    {
        if (link->tx_queue == NULL)
        {
            /* We do not have any packets to be ACKed, the link is ready to be detached */
            MAKE_MESSAGE_WITH_LEN(msg, IAP2_LINK_DETACH_CFM, sizeof(iap2_link));

            link->state = iap2_link_detached;

            /* Remove the Sink task */
            /*MessageSinkTask(link->sink, NULL);*/

            msg->status = iap2_status_success;
            msg->link = link;
            msg->sink = link->sink;
            /* Create a copy of the link instance */
            msg->size_link_data = sizeof(iap2_link);
            memmove(msg->link_data, link, sizeof(iap2_link));
            ((iap2_link *)msg->link_data)->next = NULL;

            MessageSend(iap2_lib->app_task, IAP2_LINK_DETACH_CFM, msg);
        }
        else
        {
            /* We still have packets to be ACKed, wait 1 ms */
            MessageSendLater(&link->task, IAP2_INTERNAL_LINK_DETACH_REQ, NULL, 1);
        }
    }
}

void iap2HandleMessageMoreData(iap2_link *link, MessageMoreData *message)
{
    /* All iAP2 traffic is handled by this function */
    uint8 *data = (uint8 *)SourceMap(message->source);
    uint16 size_data = SourceSize(message->source);
    uint16 size_processed = 0;

    DEBUG_PRINT_LINK(("iAP2 message more data link=0x%p state=%d len=%d\n", (void*)link, link->state, size_data));

    if (link->state == iap2_link_connecting_init)
    {
        /* We do not have a complete packet here, just match the sequence */
        size_processed = iap2HandleDataLinkInit(link, data, size_data);
    }
    else
    {
        uint8 *header = PanicUnlessMalloc(IAP2_PACKET_HEADER_LEN - 1);
        uint16 packet_len;
        uint8 checksum;

        /* Do not process any data if we are detaching and no packets to be ACKed */
        while ((link->state < iap2_link_detaching || link->tx_queue != NULL) &&
               size_data - size_processed >= IAP2_PACKET_HEADER_LEN)
        {
            /* Try to match start of packet sequence 0xFF 0x5A */
            while (size_processed < size_data)
            {
                if (data[size_processed] == 0xFF &&
                    data[size_processed + 1] == 0x5A)
                    break;

                size_processed++;
            }

            DEBUG_PRINT_LINK(("iAP2 packet found processed=%d size=%d\n", size_processed, size_data));

            /* Terminate if not enough data */
            if (size_data - size_processed < IAP2_PACKET_HEADER_LEN)
                break;

            /* Copy the header */
            memmove(header, &data[size_processed], IAP2_PACKET_HEADER_LEN - 1);
            checksum = data[size_processed + IAP2_PACKET_HEADER_LEN - 1];

            /* Verify the header checksum */
            if (((iap2Checksum(header, IAP2_PACKET_HEADER_LEN - 1) + checksum) & 0xff) != 0)
            {
                /* This is not a valid header, ignore it */
                DEBUG_PRINT_LINK(("iAP2 packet header checksum mismatch\n"));
                size_processed++;
                continue;
            }

            packet_len = ((uint16)header[iap2_packet_len_msb] << 8) | header[iap2_packet_len_lsb];

            /* Terminate if packet len is too small */
            if (packet_len < IAP2_PACKET_HEADER_LEN)
            {
                size_processed++;
                DEBUG_PRINT_LINK(("iAP2 packet length too small packet_len=%d\n", packet_len));
                continue;
            }

            /* Terminate if payload is not complete */
            if (size_data - size_processed < packet_len)
            {
                DEBUG_PRINT_LINK(("iAP2 packet payload not complete packet_len=%d actual=%d\n", packet_len, size_data - size_processed));
                break;
            }

            /* Check for RST or SLP bits in the control bytes */
            if (header[iap2_packet_ctrl_byte] & iap2_ctrl_bit_rst)
            {
                iap2LinkReset(link);
                size_processed = size_data;
                break;
            }
            else if (header[iap2_packet_ctrl_byte] & iap2_ctrl_bit_slp)
            {
                iap2LinkSleep(link);
                size_processed = size_data;
                break;
            }

            /* Handle the ACK now */
            if (header[iap2_packet_ctrl_byte] & iap2_ctrl_bit_ack)
            {
                DEBUG_PRINT_LINK(("iAP2 packet received seq=%d ack=%d\n", header[iap2_packet_seq_num], header[iap2_packet_ack_num]));
                iap2PacketHandleAck(link, header[iap2_packet_ack_num]);
            }

            /* Next check for SYN and EAK payload */
            if (header[iap2_packet_ctrl_byte] & iap2_ctrl_bit_syn)
            {
                /* We have a SYN payload */
                MAKE_MESSAGE_WITH_LEN(msg, IAP2_INTERNAL_LINK_SYNC_IND, packet_len - IAP2_PACKET_HEADER_LEN - 1);
                DEBUG_PRINT_LINK(("iAP2 SYN packet received seq=%d\n", header[iap2_packet_seq_num]));
                msg->size_payload = packet_len - IAP2_PACKET_HEADER_LEN - 1;
                memmove(msg->payload, &data[size_processed + IAP2_PACKET_HEADER_LEN], msg->size_payload);
                checksum = data[size_processed + packet_len - 1];

                if (((iap2Checksum(msg->payload, msg->size_payload) + checksum) & 0xff) != 0)
                {
                    /* This is not a valid header, ignore it */
                    DEBUG_PRINT_LINK(("iAP2 SYN payload checksum mismatch\n"));
                    free(msg);
                    size_processed++;
                    continue;
                }

                MessageSend(&link->task, IAP2_INTERNAL_LINK_SYNC_IND, msg);
            }
            else if (header[iap2_packet_ctrl_byte] & iap2_ctrl_bit_eak)
            {
                /* We have a EAK payload */
                MAKE_MESSAGE_WITH_LEN(msg, IAP2_INTERNAL_LINK_EAK_IND, packet_len - IAP2_PACKET_HEADER_LEN - 1);
                DEBUG_PRINT_LINK(("iAP2 EAK packet received seq=%d\n", header[iap2_packet_seq_num]));
                msg->size_payload = packet_len - IAP2_PACKET_HEADER_LEN - 1;
                memmove(msg->payload, &data[size_processed + IAP2_PACKET_HEADER_LEN], msg->size_payload);
                checksum = data[size_processed + packet_len - 1];

                if (((iap2Checksum(msg->payload, msg->size_payload) + checksum) & 0xff) != 0)
                {
                    /* This is not a valid payload, ignore it */
                    DEBUG_PRINT_LINK(("iAP2 EAK payload checksum mismatch\n"));
                    free(msg);
                    size_processed++;
                    continue;
                }

                MessageSend(&link->task, IAP2_INTERNAL_LINK_EAK_IND, msg);
            }
            else if (packet_len > IAP2_PACKET_HEADER_LEN)
            {
                /* We have a general session message */
                /* If a packet is received out of sequence, drop it */
                /* We do not have enough buffer to store it */
                if (header[iap2_packet_seq_num] != (uint8)((link->last_received_seq + 1) & 0xff))
                {
                    DEBUG_PRINT_LINK(("iAP2 packet received out of sequence seq=%d last=%d\n", header[iap2_packet_seq_num], link->last_received_seq));
                    size_processed += packet_len;
                    continue;
                }

                /* Payload is different from packet types */

                if (header[iap2_packet_session_id] == link->session_id_ctrl &&
                    packet_len >= IAP2_PACKET_HEADER_LEN + IAP2_PACKET_CTRL_HEADER_LEN + 1 &&
                    data[size_processed + IAP2_PACKET_HEADER_LEN] == 0x40 &&
                    data[size_processed + IAP2_PACKET_HEADER_LEN + 1] == 0x40)
                {
                    /* We have a control message */
                    uint16 message_len = ((uint16)data[size_processed + IAP2_PACKET_HEADER_LEN + 2] << 8) |
                                         data[size_processed + IAP2_PACKET_HEADER_LEN + 3];
                    iap2_control_message message_id = (iap2_control_message)(((uint16)data[size_processed + IAP2_PACKET_HEADER_LEN + 4] << 8) |
                                                                             data[size_processed + IAP2_PACKET_HEADER_LEN + 5]);

                    checksum = data[size_processed + packet_len - 1];
                    checksum += 0x40 << 1;
                    checksum += (message_len & 0xff) + (message_len >> 8);
                    checksum += ((uint16)message_id & 0xff) + ((uint16)message_id >> 8);

                    if (packet_len != message_len + IAP2_PACKET_HEADER_LEN + 1)
                    {
                        DEBUG_PRINT_LINK(("iAP2 control message length mismatch packet=%d message=%d\n", packet_len, message_len));
                        size_processed++;
                        continue;
                    }
                    else
                    {
                        MAKE_MESSAGE_WITH_LEN(msg, IAP2_CONTROL_MESSAGE_IND, message_len - IAP2_PACKET_CTRL_HEADER_LEN);
                        msg->link = link;
                        msg->message_id = message_id;
                        msg->size_payload = message_len - IAP2_PACKET_CTRL_HEADER_LEN;

                        if (msg->size_payload)
                        {
                            memmove(msg->payload, &data[size_processed + IAP2_PACKET_HEADER_LEN + IAP2_PACKET_CTRL_HEADER_LEN], msg->size_payload);
                            checksum += iap2Checksum(msg->payload, msg->size_payload);
                        }

                        DEBUG_PRINT_LINK(("iAP2 control message received seq=%d message=%x payload=%d\n", header[iap2_packet_seq_num], message_id, msg->size_payload));

                        if ((checksum & 0xff) != 0)
                        {
                            /* This is not a valid payload, ignore it */
                            DEBUG_PRINT_LINK(("iAP2 control message payload checksum mismatch\n"));
                            free(msg);
                            size_processed++;
                            continue;
                        }

                        /* Route the control message to different destinations based on message ID */
                        if (message_id == RequestAuthenticationCertificate ||
                            message_id == RequestAuthenticationChallengeResponse)
                        {
                            /* Authentication request messages routed internally */
                            MessageSend(&link->task, IAP2_INTERNAL_LINK_AUTH_IND, msg);
                        }
                        else if (message_id == AuthenticationFailed)
                        {
                            /* Notify the app with authentication failed */
                            MAKE_MESSAGE_WITH_STATUS(conn_msg, IAP2_CONNECT_CFM, iap2_status_authentication_fail);
                            conn_msg->link = NULL;
                            conn_msg->bd_addr = link->remote_addr;
                            conn_msg->sink = NULL;
                            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, conn_msg);
                            free(msg);
                            link->state = iap2_link_error;

                            /* Disconnect the link */
                            ConnectionRfcommDisconnectRequest(&link->task, link->sink);
                        }
                        else if (message_id == AuthenticationSucceeded)
                        {
                            /* Authentication succeeded, mark it and wait for start identification */
                            free(msg);
                            link->state = iap2_link_auth_challenge_done;
                        }
                        else if (message_id == StartIdentification)
                        {
                            /* Notify the app with connection succeeded and start identification */
                            MAKE_MESSAGE_WITH_STATUS(conn_msg, IAP2_CONNECT_CFM, iap2_status_success);
                            conn_msg->link = link;
                            conn_msg->bd_addr = link->remote_addr;
                            conn_msg->sink = link->sink;
                            MessageSend(iap2_lib->app_task, IAP2_CONNECT_CFM, conn_msg);
                            free(msg);
                            link->state = iap2_link_ident_start;
                        }
                        else if (message_id == IdentificationAccepted ||
                                 message_id == IdentificationRejected)
                        {
                            /* Notify the app with identification cfm */
                            MAKE_MESSAGE(ident_msg, IAP2_IDENTIFICATION_CFM);

                            ident_msg->link = link;
                            ident_msg->status = (message_id == IdentificationAccepted) ? iap2_status_success : iap2_status_fail;
                            ident_msg->error_mask = 0;

                            if (message_id == IdentificationRejected)
                            {
                                uint16 i = 0, param_id;
                                while (i < msg->size_payload)
                                {
                                    param_id = ((uint16)msg->payload[i + 2] << 8) | msg->payload[i + 3];
                                    ident_msg->error_mask |= 1UL << param_id;
                                    i += ((uint16)msg->payload[i] << 8) | msg->payload[i + 1];
                                }
                            }

                            link->state = (message_id == IdentificationAccepted) ? iap2_link_ready: iap2_link_auth_challenge_done;

                            free(msg);

                            link->last_received_seq = header[iap2_packet_seq_num];
                            MessageSend(iap2_lib->app_task, IAP2_IDENTIFICATION_CFM, ident_msg);
                        }
                        else if (message_id == StartExternalAccessoryProtocolSession)
                        {
                            Task protocol_task;
                            uint16 session_id;
                            uint8 protocol_id;
                            
                            Iap2CtrlParamReadUint8(msg->payload, msg->size_payload, 0, &protocol_id);
                            Iap2CtrlParamReadUint16(msg->payload, msg->size_payload, 1, &session_id);
                            free(msg);
                           
                            protocol_task = iap2EAGetProtocolTask(protocol_id);
                           
                            if (protocol_task)
                            {
                                MAKE_MESSAGE(start_ea_msg, IAP2_EA_SESSION_START_IND);

                                DEBUG_PRINT_LINK(("iAP2 Start EAP p=%u s=%u\n", protocol_id, session_id));

                                iap2EASetSessionProtocol(session_id, protocol_id);
                                iap2EASetSessionLink(session_id, link);
                                
                                start_ea_msg->link = link;
                                start_ea_msg->protocol_id = protocol_id;
                                start_ea_msg->session_id = session_id;

                                link->last_received_seq = header[iap2_packet_seq_num];
                                MessageSend(protocol_task, IAP2_EA_SESSION_START_IND, start_ea_msg);
                            }                            
                        }
                        else if (message_id == StopExternalAccessoryProtocolSession)
                        {
                            Task protocol_task;
                            uint16 session_id;
                            uint8 protocol_id;
                            
                            Iap2CtrlParamReadUint16(msg->payload, msg->size_payload, 0, &session_id);
                            free(msg);
                            
                            protocol_id = iap2EAGetSessionProtocol(session_id);
                            protocol_task = iap2EAGetProtocolTask(protocol_id);
                            iap2EASetSessionProtocol(session_id, 0);
                            
                            if (protocol_task)
                            {
                                MAKE_MESSAGE(stop_ea_msg, IAP2_EA_SESSION_STOP_IND);

                                DEBUG_PRINT_LINK(("iAP2 Stop EAP p=%u s=%u\n", protocol_id, session_id));

                                stop_ea_msg->link = link;
                                stop_ea_msg->protocol_id = protocol_id;
                                stop_ea_msg->session_id = session_id;

                                link->last_received_seq = header[iap2_packet_seq_num];
                                MessageSend(protocol_task, IAP2_EA_SESSION_STOP_IND, stop_ea_msg);
                            }
                        }
                        else
                        {
                            /* All other messages sent to application */
                            link->last_received_seq = header[iap2_packet_seq_num];
                            MessageSend(iap2_lib->app_task, IAP2_CONTROL_MESSAGE_IND, msg);
                        }
                    }
                }
                else if (header[iap2_packet_session_id] == link->session_id_ea &&
                         packet_len >= IAP2_PACKET_HEADER_LEN + IAP2_PACKET_EA_HEADER_LEN + 1)
                {
                    Task protocol_task;
                    uint16 session_id;
                    uint8 protocol_id;
                    
                    MAKE_MESSAGE_WITH_LEN(msg, IAP2_EA_SESSION_DATA_IND, packet_len - IAP2_PACKET_HEADER_LEN - IAP2_PACKET_EA_HEADER_LEN - 1);
                    
                    session_id = ((uint16)data[size_processed + IAP2_PACKET_HEADER_LEN] << 8) |
                                      data[size_processed + IAP2_PACKET_HEADER_LEN + 1];
                    protocol_id = iap2EAGetSessionProtocol(session_id);
                    protocol_task = iap2EAGetProtocolTask(protocol_id);
                    
                    /* We have an EA message */
                    checksum = data[size_processed + packet_len - 1];
                    msg->link = link;
                    msg->protocol_id = protocol_id;
                    msg->session_id = session_id;
                   
                    checksum += session_id & 0xff;
                    checksum += session_id >> 8;
                    msg->size_payload = packet_len - IAP2_PACKET_HEADER_LEN - IAP2_PACKET_EA_HEADER_LEN - 1;
                    if (msg->size_payload)
                    {
                        memmove(msg->payload, &data[size_processed + IAP2_PACKET_HEADER_LEN + IAP2_PACKET_EA_HEADER_LEN], msg->size_payload);
                        checksum += iap2Checksum(msg->payload, msg->size_payload);
                    }

                    DEBUG_PRINT_LINK(("iAP2 EA message received seq=%d session=%d payload=%d\n", header[iap2_packet_seq_num], session_id, msg->size_payload));

                    if ((checksum & 0xff) != 0)
                    {
                        /* This is not a valid payload, ignore it */
                        DEBUG_PRINT_LINK(("iAP2 EA message payload checksum mismatch\n"));
                        free(msg);
                        size_processed++;
                        continue;
                    }

                    link->last_received_seq = header[iap2_packet_seq_num];
                    MessageSend(protocol_task, IAP2_EA_SESSION_DATA_IND, msg);
                }
                else
                {
                    /* Unknown message, ignore */
                    DEBUG_PRINT_LINK(("iAP2 unknown message\n"));
                    size_processed++;
                    continue;
                }
            }

            /* Successfully parsed the message */
            size_processed += packet_len;

            /* update the last received seq */
            link->last_received_seq = header[iap2_packet_seq_num];

            DEBUG_PRINT_LINK(("iAP2 packet process done processed=%d size=%d\n", size_processed, size_data));
        }

        free(header);

        /* Check if we need to schedule an ACK */
        if (link->state >= iap2_link_sync_done &&
            link->state <= iap2_link_detaching &&
            link->last_sent_ack != link->last_received_seq &&
            !link->param.supportZeroAckLink)
        {
            while (link->last_sent_ack != link->last_received_seq)
            {
                if (((uint8)(link->last_received_seq - link->last_sent_ack) & 0xff) >= MAX(link->param.MaxCumulativeAcks, 1))
                {
                    /* Send an ACK now if the un-acked packets exceeds the MaxCumulativeAcks */
                    uint8 ack_to_send = (link->last_sent_ack + MAX(link->param.MaxCumulativeAcks, 1)) & 0xff;
                    DEBUG_PRINT_LINK(("iAP2 ACK send now ack=%d\n", ack_to_send));
                    iap2PacketAckFlush(link, ack_to_send);
                }
                else if (link->state == iap2_link_detaching)
                {
                    /* Send an ACK now if we are detaching */
                    iap2PacketAckFlush(link, link->last_received_seq);
                }
                else
                {
                    /* Send an ACK with CumulativeAckTimeout */
                    DEBUG_PRINT_LINK(("iAP2 ACK scheduled with timeout ack=%d\n", link->last_received_seq));
                    MessageSendLater(&link->task, IAP2_INTERNAL_ACK_FLUSH_REQ, NULL, link->param.CumulativeAckTimeout);
                    break;
                }
            }
        }
    }

    /* Drop the processed data */
    if (size_processed)
        SourceDrop(message->source, size_processed);
}


