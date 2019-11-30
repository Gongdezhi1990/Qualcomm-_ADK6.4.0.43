/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_packet.h
DESCRIPTION
    Header file for iAP2 packet handling.
*/
#ifndef __IAP2_PACKET_H__
#define __IAP2_PACKET_H__

#include "iap2_private.h"

#define IAP2_PACKET_HEADER_LEN              (9)
#define IAP2_PACKET_CTRL_HEADER_LEN         (6)
#define IAP2_PACKET_EA_HEADER_LEN           (2)
#define IAP2_CTRL_PARAM_HEADER_LEN          (4)

typedef enum
{
    iap2_packet_sop_msb,
    iap2_packet_sop_lsb,
    iap2_packet_len_msb,
    iap2_packet_len_lsb,
    iap2_packet_ctrl_byte,
    iap2_packet_seq_num,
    iap2_packet_ack_num,
    iap2_packet_session_id,
    iap2_packet_hdr_chechsum
} iap2_packet_byte;

iap2_packet *iap2PacketAllocate(uint16 size_payload);
void iap2PacketDeallocate(iap2_packet *packet);
bool iap2PacketValidate(iap2_link *link, iap2_packet *packet);
bool iap2PacketQueueSlack(iap2_link *link);
iap2_status_t iap2PacketEnqueue(iap2_link *link, iap2_packet *packet);
iap2_status_t iap2PacketFlush(iap2_link *link, iap2_packet *packet);
void iap2PacketHandleAck(iap2_link *link, uint8 ack_num);
void iap2PacketAckFlush(iap2_link *link, uint8 ack_num);
void iap2HandleInternalAckFlushReq(iap2_link *link);
void iap2HandleInternalPacketRetransReq(iap2_packet *packet, IAP2_INTERNAL_PACKET_RETRANS_REQ_T *req);

#endif /* __IAP2_PACKET_H__ */
