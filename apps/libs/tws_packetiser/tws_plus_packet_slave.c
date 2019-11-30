/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_plus_packet_slave.c
*/

#include "packet_slave.h"
#include "frame_info.h"
#include "rtp.h"
#include <packetiser_helper.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>

static bool twsPlusPacketSlaveInit(packet_slave_t *packet,
                                   const uint8 *packet_buffer, uint32 packet_len,
                                   tws_packetiser_slave_config_t *config)
{
    tws_plus_packet_slave_t *slave = &packet->slave.tws_plus;
    if (slave && packet_buffer && packet_len && config)
    {
        slave->packet = packet_buffer;
        slave->len = packet_len;
        slave->ptr = packet_buffer;
        slave->frames = 0;
        memset(&slave->frame_info_history_sbc, 0, sizeof(slave->frame_info_history_sbc));

        if (config->codec == TWS_PACKETISER_CODEC_SBC ||
            config->codec == TWS_PACKETISER_CODEC_APTX)
        {
            slave->config = config;
            return TRUE;
        }
    }
    return FALSE;
}

static void twsPlusPacketSlaveUninit(packet_slave_t *packet)
{
    UNUSED(packet);
}

/* For slave, calculate the number of unread bytes in the packet */
static uint32 twsPlusPacketCalcUnread(tws_plus_packet_slave_t *slave)
{
    ptrdiff_t unread = slave->packet + slave->len - slave->ptr;
    if (unread < 0)
        Panic();
    return unread;
}

/* Read a byte from the slave and increment the packet read pointer */
#define READ_INCR(SLAVE) *SLAVE->ptr++

/* On TRUE function exit, the ptr will address the first audio frame in the
   packet.
   On FALSE function exit, the ptr is essentially invalid and should not be
   used further. */
static bool twsPlusPacketReadHeader(packet_slave_t *packet, rtime_t *ttp,
                                    packetiser_helper_scmst_t *scmst,
                                    bool *complete)
{
    tws_plus_packet_slave_t *slave = &packet->slave.tws_plus;
    uint32 min_size = RTP_MINIMUM_HEADER_SIZE +
                      (slave->config->cp_header_enabled ? SCMS_HEADER_SIZE : 0) +
                      (slave->config->codec == TWS_PACKETISER_CODEC_SBC ? SBC_MEDIA_PAYLOAD_HEADER_SIZE : 0);

    if (twsPlusPacketCalcUnread(slave) >= min_size)
    {
        uint8 version = READ_INCR(slave);
        if (version == RTP0_VERSION_2)
        {
            uint8 payload_type_marker = READ_INCR(slave);
            if ((payload_type_marker >= RTP1_PAYLOAD_TYPE_FIRST_DYNAMIC) &&
                (payload_type_marker <= RTP1_PAYLOAD_TYPE_LAST_DYNAMIC))
            {
                uint32 timestamp = 0;
                uint32 i;
                
                /* Skip sequence number, assume no missed packets */
                slave->ptr += 2;

                for (i = 0; i < 4; i++)
                {
                    timestamp <<= 8;
                    timestamp |= READ_INCR(slave);
                }
                *ttp = timestamp;

                /* Skip ssrcs */
                slave->ptr += 4;

                *scmst = packetiser_helper_scmst_copy_allowed;
                if (slave->config->cp_header_enabled)
                {
                    *scmst = READ_INCR(slave);
                }

                if (slave->config->codec == TWS_PACKETISER_CODEC_SBC)
                {
                    /* Skip over SBC media header */
                    slave->ptr++;
                }
                *complete = TRUE;
                return TRUE;
            }
        }
    }
    return FALSE;
}

/* TWS+ packets do not have spadj. */
static bool twsPlusPacketReadMiniSpadj(packet_slave_t *packet, rtime_spadj_mini_t *spadjm)
{
    UNUSED(packet);
    *spadjm = 0;
    return TRUE;
}

/* Assumes ptr addresses the next frame in the packet. */
static bool twsPlusPacketReadAudioFrame(packet_slave_t *packet, uint8 *dest, uint32 frame_length, uint32 *frame_number)
{
    tws_plus_packet_slave_t *slave = &packet->slave.tws_plus;
    if (twsPlusPacketCalcUnread(slave) >= frame_length)
    {
        memcpy(dest, slave->ptr, frame_length);
        slave->ptr += frame_length;
        *frame_number = slave->frames++;
        return TRUE;
    }
    return FALSE;
}

/* Assumes ptr addresses the the next frame in the packet. */
static bool twsPlusPacketReadAudioFrameInfo(packet_slave_t *packet, frame_info_t *frame_info)
{
    tws_plus_packet_slave_t *slave = &packet->slave.tws_plus;
    uint32 unread = twsPlusPacketCalcUnread(slave);
    return (slave->config->codec == TWS_PACKETISER_CODEC_SBC) ?
        frameInfoSBC(slave->ptr, unread, frame_info, &slave->frame_info_history_sbc) :
        frameInfoAptx(unread, frame_info);
}

static uint32 twsPlusPacketGetTTPLenBits(packet_slave_t *packet)
{
    UNUSED(packet);
    return 32;
}

const packet_slave_functions_t packet_slave_funcs_tws_plus = {
    .init = twsPlusPacketSlaveInit,
    .unInit = twsPlusPacketSlaveUninit,
    .readHeader = twsPlusPacketReadHeader,
    .readMiniSpadj = twsPlusPacketReadMiniSpadj,
    .readAudioFrame = twsPlusPacketReadAudioFrame,
    .readAudioFrameInfo = twsPlusPacketReadAudioFrameInfo,
    .getTTPLenBits = twsPlusPacketGetTTPLenBits
};
