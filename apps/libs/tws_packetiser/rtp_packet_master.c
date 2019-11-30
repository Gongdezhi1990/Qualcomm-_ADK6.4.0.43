/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    rtp_packet_master.c
*/

#include "packet_master.h"
#include "rtp_packet_master.h"
#include <packetiser_helper.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <util.h>
#include <vmtypes.h>
#include <rtp.h>

/* For master, calculate the number of bytes space in the packet */
static uint32 calcSpace(rtp_packet_master_t *p)
{
    ptrdiff_t space = p->buffer + p->len - p->ptr;
    if (space < 0)
        Panic();
    return space;
}

/* Return a 32 bit random number */
static uint32 UtilRandom32(void)
{
    return ((uint32)UtilRandom() << 16) | (uint32)UtilRandom();
}

/* Initialise the packet master */
static bool init(packet_master_t *packet, tws_packetiser_codec_t codec, bool content_protection_header_enabled)
{
    if (packet && (codec == TWS_PACKETISER_CODEC_SBC || codec == TWS_PACKETISER_CODEC_APTX))
    {
        rtp_packet_master_t *rtp = &packet->master.rtp;
        rtp->codec = codec;
        /* For security reasons, the sequence number, timestamp and SSRC are
           initialised with a random value. */
        rtp->sequence_number = UtilRandom();
        rtp->time_stamp = UtilRandom32();
        rtp->ssrc = UtilRandom32();
        rtp->cp_header_enabled = content_protection_header_enabled;
        memset(&rtp->frame_info_history_sbc, 0, sizeof(rtp->frame_info_history_sbc));
        return TRUE;
    }
    return FALSE;
}

/* Initialise prior to creating packet */
static bool packetInit(packet_master_t *packet, uint8 *buffer, uint32 buffer_len)
{
    if (packet && buffer && (buffer_len >= (RTP_MINIMUM_HEADER_SIZE + SCMS_HEADER_SIZE)))
    {
        rtp_packet_master_t *rtp = &packet->master.rtp;
        rtp->buffer = buffer;
        rtp->len = buffer_len;
        rtp->ptr = buffer;
        rtp->frames = 0;
        return TRUE;
    }
    return FALSE;
}

/* Return the length of the packet */
static uint32 packetLength(packet_master_t *packet)
{
    rtp_packet_master_t *rtp = &packet->master.rtp;
    ptrdiff_t packet_length = rtp->ptr - rtp->buffer;
    if (packet_length < 0)
        Panic();
    return packet_length;
}

/* Return the length of the RTP header */
static uint32 headerLength(packet_master_t *packet, uint32 number_audio_frames)
{
    rtp_packet_master_t *rtp = &packet->master.rtp;
    uint32 length = RTP_MINIMUM_HEADER_SIZE;
    /* There is no RTP per-audio-frame header */
    UNUSED(number_audio_frames);
    if (rtp->cp_header_enabled)
    {
        length += SCMS_HEADER_SIZE;
    }
    if (rtp->codec == TWS_PACKETISER_CODEC_SBC)
    {
        length += SBC_MEDIA_PAYLOAD_HEADER_SIZE;
    }
    return length;
}

/* Avoid zero branches as we are using function pointers, we have no RTP header
   in this instance.
*/
static uint32 headerLengthZero(packet_master_t *packet, uint32 number_audio_frames)
{
    UNUSED(packet);
    UNUSED(number_audio_frames);
    return 0;
}

static void writeHeaderHelper(rtp_packet_master_t *rtp, rtime_t timestamp,
                              packetiser_helper_scmst_t scmst)
{
    uint8 *dest = rtp->buffer;

    /* version 2 */
    dest[0] = RTP0_VERSION_2;
    dest[1] = RTP1_PAYLOAD_TYPE_FIRST_DYNAMIC;

    /* sequence number */
    dest[2] = (uint8)(rtp->sequence_number >> 8);
    dest[3] = (uint8)(rtp->sequence_number & 0xff);

    /* time stamp */
    dest[4] = (uint8)(timestamp >> 24);
    dest[5] = (uint8)(timestamp >> 16);
    dest[6] = (uint8)(timestamp >> 8);
    dest[7] = (uint8)(timestamp & 0xff);

    /* ssrc */
    dest[8] = (uint8)(rtp->ssrc >> 24);
    dest[9] = (uint8)(rtp->ssrc >> 16);
    dest[10] = (uint8)(rtp->ssrc >> 8);
    dest[11] = (uint8)(rtp->ssrc & 0xff);

    if (rtp->cp_header_enabled)
    {
        dest[RTP_MINIMUM_HEADER_SIZE] = scmst;
    }

    /* The media payload header for SBC is written when the packet is finalised */

    /* Set the pointer where audio frames will be written */
    rtp->ptr = dest + RTP_MINIMUM_HEADER_SIZE;
    rtp->ptr += rtp->cp_header_enabled ? SCMS_HEADER_SIZE : 0;
    rtp->ptr += (rtp->codec == TWS_PACKETISER_CODEC_SBC) ? SBC_MEDIA_PAYLOAD_HEADER_SIZE : 0;
}

/* Write the RTP header. On exit from the function, the ptr points to the address
   where the first audio frame will be written. */
static void writeHeaderRTP(packet_master_t *packet, rtime_t ttp,
                           packetiser_helper_scmst_t scmst)
{
    rtp_packet_master_t *rtp = &packet->master.rtp;
    UNUSED(ttp);
    writeHeaderHelper(rtp, rtp->time_stamp, scmst);
}

/* We do not want to write an RTP header, but using function pointers we want to ensure
   no zero branches. */
static void writeNoHeader(packet_master_t *packet, rtime_t ttp,
                           packetiser_helper_scmst_t scmst)
{
    UNUSED(packet);
    UNUSED(ttp);
    UNUSED(scmst);
}

/* TWS+ also uses the RTP header, but the time stamp for TWS+ is non-standard.
   It is a 1us resolution time-to-play (ttp) */
static void writeHeaderTWSPlus(packet_master_t *packet, rtime_t ttp,
                               packetiser_helper_scmst_t scmst)
{
    writeHeaderHelper(&packet->master.rtp, ttp, scmst);
}

static bool enoughSpaceToWriteFrame(rtp_packet_master_t *rtp, uint32 frame_length)
{
    return (calcSpace(rtp) >= frame_length);
}

static bool frameInfo(rtp_packet_master_t *rtp, const uint8 *src, uint32 frame_length,
                      frame_info_t *frame_info)
{
    bool frame_info_valid = FALSE;
    switch (rtp->codec)
    {
        case TWS_PACKETISER_CODEC_SBC:
            frame_info_valid = frameInfoSBC(src, frame_length, frame_info, &rtp->frame_info_history_sbc);
            break;
        case TWS_PACKETISER_CODEC_APTX:
            frame_info_valid = frameInfoAptx(frame_length, frame_info);
            break;
        default:
            break;
    }
    return frame_info_valid;
}

/* Write an audio frame to the packet */
static bool writeAudioFrame(packet_master_t *packet, const uint8 *src, uint32 frame_length,
                            audio_frame_metadata_t *fmd)
{
    rtp_packet_master_t *rtp = &packet->master.rtp;

    UNUSED(fmd);

    if (enoughSpaceToWriteFrame(rtp, frame_length))
    {
        if (rtp->codec != TWS_PACKETISER_CODEC_SBC || rtp->frames < SBC_MAX_FRAMES)
        {
            frame_info_t frame_info;
            if (frameInfo(rtp, src, frame_length, &frame_info))
            {
                memcpy(rtp->ptr, src, frame_length);
                rtp->ptr += frame_length;
                rtp->frames++;
                rtp->time_stamp += frame_info.samples;
            }
            else
            {
                /* The header is invalid, don't copy the frame to the packet.
                   Return TRUE so the caller drops the frame and continues, hopefully
                   the next frame will be valid */
            }
            return TRUE;
        }
    }
    return FALSE;
}

/* The packetiser is dropping an audio frame */
static void droppedAudioFrame(packet_master_t *packet, const uint8 *src, uint32 frame_length,
                              audio_frame_metadata_t *fmd)
{
    rtp_packet_master_t *rtp = &packet->master.rtp;
    frame_info_t frame_info;
    UNUSED(fmd);

    if (frameInfo(rtp, src, frame_length, &frame_info))
    {
        /* Keep track of time passing due to dropped packets */
        rtp->time_stamp += frame_info.samples;
    }
    /* Increment the sequence number to indicate lost packets */
    rtp->sequence_number++;
}

/* The packetiser is about to transmit the frame - finalise the packet */
static void finalise(packet_master_t *packet)
{
    rtp_packet_master_t *rtp = &packet->master.rtp;

    /* Update the media payload header for SBC with the number of frames in the packet */
    if (rtp->codec == TWS_PACKETISER_CODEC_SBC)
    {
        uint32 index = RTP_MINIMUM_HEADER_SIZE + (rtp->cp_header_enabled ? SCMS_HEADER_SIZE : 0);
        rtp->buffer[index] = rtp->frames;
    }

    rtp->sequence_number++;
}

const packet_master_functions_t packet_master_funcs_rtp = {
    .init = init,
    .packetInit = packetInit,
    .packetLength = packetLength,
    .headerLength = headerLength,
    .writeHeader = writeHeaderRTP,
    .writeAudioFrame = writeAudioFrame,
    .droppedAudioFrame = droppedAudioFrame,
    .finalise = finalise
};

const packet_master_functions_t packet_master_funcs_no_header = {
    .init = init,
    .packetInit = packetInit,
    .packetLength = packetLength,
    .headerLength = headerLengthZero,
    .writeHeader = writeNoHeader,
    .writeAudioFrame = writeAudioFrame,
    .droppedAudioFrame = droppedAudioFrame,
    .finalise = finalise
};

const packet_master_functions_t packet_master_funcs_tws_plus = {
    .init = init,
    .packetInit = packetInit,
    .packetLength = packetLength,
    .headerLength = headerLength,
    .writeHeader = writeHeaderTWSPlus,
    .writeAudioFrame = writeAudioFrame,
    .droppedAudioFrame = droppedAudioFrame,
    .finalise = finalise
};
