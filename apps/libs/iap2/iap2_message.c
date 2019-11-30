/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    iap2_message.c
DESCRIPTION
   This file handles the iAP2 protocol message protocol for control and EAP packets.
*/

#include "iap2_private.h"
#include "iap2_link.h"
#include "iap2_packet.h"

iap2_status_t Iap2ControlMessage(iap2_link *link, iap2_control_message message_id, uint16 size_payload, const uint8 *payload, bool static_payload)
{
    iap2_status_t status = iap2_status_success;

    if (!iap2LinkValidate(link))
        status = iap2_status_link_not_found;
    else if (link->state != iap2_link_ready)
        status = iap2_status_link_not_ready;
    else if (!iap2PacketQueueSlack(link))
        status = iap2_status_queue_full;
    else
    {
        iap2_packet *packet = iap2PacketAllocate(0);

        if (!packet)
            status = iap2_status_out_of_resources;
        else
        {
            packet->session_type = iap2_session_type_ctrl;
            packet->id.ctrl_message = (uint16)message_id;

            if (size_payload && payload)
            {
                packet->size_payload = size_payload;
                packet->payload = (uint8 *)payload;
                packet->static_payload = static_payload;
            }

            if ((status = iap2PacketEnqueue(link, packet)) != iap2_status_success)
            {
                packet->payload = NULL;
                iap2PacketDeallocate(packet);
            }
        }
    }

    return status;
}

iap2_status_t Iap2EAMessage(iap2_link *link, uint16 session_id, uint16 size_payload, const uint8 *payload, bool static_payload)
{
    iap2_status_t status = iap2_status_success;

    if (!iap2LinkValidate(link))
        status = iap2_status_link_not_found;
    else if (link->state != iap2_link_ready)
        status = iap2_status_link_not_ready;
    else if (!iap2PacketQueueSlack(link))
        status = iap2_status_queue_full;
    else if (!size_payload || !payload)
        status = iap2_status_invalid_param;
    else
    {
        iap2_packet *packet = iap2PacketAllocate(0);

        if (!packet)
            status = iap2_status_out_of_resources;
        else
        {
            packet->session_type = iap2_session_type_ea;
            packet->id.ea_session = session_id;
            packet->size_payload = size_payload;
            packet->payload = (uint8 *)payload;
            packet->static_payload = static_payload;

            if ((status = iap2PacketEnqueue(link, packet)) != iap2_status_success)
            {
                packet->payload = NULL;
                iap2PacketDeallocate(packet);
            }
        }
    }

    return status;
}

