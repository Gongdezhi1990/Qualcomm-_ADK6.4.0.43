/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packet_slave.c
*/

#include "packet_slave.h"
#include "tws_packet_slave.h"
#include "tws_packet_private.h"
#include "frame_info.h"
#include <packetiser_helper.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <extended_audio_header.h>

/* For slave, calculate the number of unread bytes in the packet */
static uint32 twsPacketCalcUnread(tws_packet_slave_t *tp)
{
    ptrdiff_t unread = tp->packet + tp->len - tp->ptr;
    if (unread < 0)
        Panic();
    return unread;
}

/* Read the 24-bit TTP from src. */
static bool readTTP(tws_packet_slave_t *tp, rtime_t *ttp)
{
    uint32 sizeof_ttpb = sizeof(ttp_bytes_t);
    if (twsPacketCalcUnread(tp) >= sizeof_ttpb)
    {
        /* Read TTP from the start of the packet */
        const uint8* src = tp->packet;
        *ttp = (rtime_t)src[0] << 16;
        *ttp |= (rtime_t)src[1] << 8;
        *ttp |= src[2];
        tp->ptr = src + sizeof_ttpb;
        return TRUE;
    }
    return FALSE;
}

/* Read frame info - appropriate if there is only one codec frame per packet. */
static bool twsPacketReadAudioFrameInfoOnePerPacket(tws_packet_slave_t *tp,
                                                    frame_info_t *frame_info)
{
    /* If present, use the length from the extended audio header, otherwise,
       assume one frame per packet. In this case, the frame must fill the remainder 
       of the packet, assuming any mini spadj has already been read */
    uint32 len = tp->eah_reader ? eahReaderGetFrameLen(tp->eah_reader, tp->frames) :
                                  twsPacketCalcUnread(tp);    
    frame_info->samples = tp->frame_samples;
    frame_info->length = len;
    return (len != 0);
}

/* Read frame info from a SBC frame header. Note that the header CRC is not
 calculated and checked */
static bool twsPacketReadAudioFrameInfoSBC(tws_packet_slave_t *tp,
                                           frame_info_t *frame_info)
{
    uint32 unread = twsPacketCalcUnread(tp);
    return frameInfoSBC(tp->ptr, unread, frame_info, &tp->frame_info_history_sbc);
}

/* Read frame info for standard aptX. Since aptX is essentially frameless, this
   function will return info for all the aptX data in the packet. */
static bool twsPacketReadAudioFrameInfoAptX(tws_packet_slave_t *tp,
                                            frame_info_t *frame_info)
{
    /* If present, use the length from the extended audio header, otherwise,
       assume one frame per packet. In this case, the frame must fill the remainder 
       of the packet, assuming any mini spadj has already been read */
    uint32 len = tp->eah_reader ? eahReaderGetFrameLen(tp->eah_reader, tp->frames) :
                                  twsPacketCalcUnread(tp);
    return frameInfoAptx(len, frame_info);
}

static bool twsPacketSlaveInit(packet_slave_t *packet,
                               const uint8 *packet_buffer, uint32 packet_len,
                               tws_packetiser_slave_config_t *config)
{
    tws_packet_slave_t *tp = &packet->slave.tws;
    if (tp && packet_buffer && packet_len && config)
    {
        tp->packet = packet_buffer;
        tp->len = packet_len;
        tp->ptr = packet_buffer;
        tp->frames = 0;
        tp->eah_reader = NULL;
        memset(&tp->frame_info_history_sbc, 0, sizeof(tp->frame_info_history_sbc));

        /* For SBC: the SBC frame header will always be used to determine frame
           length and frame samples. If the frame has an extended audio header
           it will be ignored.
           For AAC/MP3/aptX: if a packet has an extended audio frame header, the
           frame length provided by the extended audio header will be used.
           If the packet has no extended audio frame header, it is assumed there
           is only one frame per packet.
        */
        switch(config->codec)
        {
            case TWS_PACKETISER_CODEC_SBC:
                /* The samples per frame is calculated from the SBC header */
                tp->frame_samples = 0;
                tp->read_audio_frame_info_fptr = twsPacketReadAudioFrameInfoSBC;
                tp->fragmentation_allowed = FALSE;
                break;
            case TWS_PACKETISER_CODEC_AAC:
                tp->frame_samples = 1024;
                tp->read_audio_frame_info_fptr = twsPacketReadAudioFrameInfoOnePerPacket;
                tp->fragmentation_allowed = TRUE;
                break;
            case TWS_PACKETISER_CODEC_MP3:
                tp->frame_samples = 1152;
                tp->read_audio_frame_info_fptr = twsPacketReadAudioFrameInfoOnePerPacket;
                tp->fragmentation_allowed = TRUE;
                break;
            case TWS_PACKETISER_CODEC_APTX:
                /* The number of samples in the aptX packet is proportional to
                   the amount of data in the packet. */
                tp->frame_samples = 0;
                tp->read_audio_frame_info_fptr = twsPacketReadAudioFrameInfoAptX;
                tp->fragmentation_allowed = FALSE;
                break;
            case TWS_PACKETISER_CODEC_APTX_AD:
                /* The number of samples in the aptX adaptive packet varies */
                tp->frame_samples = 0;
                tp->read_audio_frame_info_fptr = twsPacketReadAudioFrameInfoOnePerPacket;
                tp->fragmentation_allowed = FALSE;
                break;
            default:
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

static void twsPacketSlaveUninit(packet_slave_t *packet)
{
    tws_packet_slave_t *tp = &packet->slave.tws;
    eahReaderDestroy(tp->eah_reader);
    tp->eah_reader = NULL;
}

/* On TRUE function exit, the ptr will address the first audio frame in the
   packet.
   On FALSE function exit, the ptr is essentially invalid and should not be
   used further. */
static bool twsPacketReadHeader(packet_slave_t *packet, rtime_t *ttp,
                                packetiser_helper_scmst_t *scmst,
                                bool *complete)
{
    uint32 unread;
    tws_packet_slave_t *tp = &packet->slave.tws;

    if (!readTTP(tp, ttp))
    {
        return FALSE;
    }

    /* Read tagged data until the audio tag is found or the end of the packet
       is reached. */
    while(0 != (unread = twsPacketCalcUnread(tp)))
    {
        uint8 tag = *tp->ptr++;
        unread -= TAG_LENGTH;

        if(TAG_IS_NON_AUDIO(tag))
        {
            /* Get length of tag and tag data */
            uint32 len = TAG_GET_NON_AUDIO_LENGTH(tag);
            if (len > unread)
            {
                /* Framing error with tag/data in packet */
                break;
            }
            /* Step over the tag data to the next tag */
            tp->ptr += len;
        }
        else
        {
            /* Audio tag has been found. The audio tag is the final tag */
            bool frame_complete = TAG_AUDIO_FRAME_COMPLETE(tag);
            *scmst = TAG_GET_AUDIO_SCMST_TYPE(tag);
            *complete = frame_complete;
            if (!frame_complete && !tp->fragmentation_allowed)
            {
                Panic();
            }

            if (TAG_AUDIO_EXTENDED_HEADER_IS_PRESENT(tag))
            {
                if (!frame_complete)
                {
                    /* The eah only supports complete unfragmented frames */
                    break;
                }
                /* The packet has an extended audio header. Read the header
                   then validate the frame lengths match the amount of data
                   remaining in the packet */
                if (NULL != (tp->eah_reader = eahReaderInit(tp->ptr, unread)))
                {
                    if (eahReaderPacketLengthMismatch(tp->eah_reader, unread,
                                                      sizeof(rtime_spadj_mini_t)))
                    {
                        eahReaderDestroy(tp->eah_reader);
                        tp->eah_reader = NULL;
                        break;
                    }
                    /* Skip over the eah now its read */
                    tp->ptr += eahReaderGetEahLen(tp->eah_reader);
                }
                else
                {
                    /* Unable to init extended audio header */
                    break;
                }
            }
            return TRUE;
        }
    }
    return FALSE;
}

/* Assumes ptr addresses the spadjm of the next frame in the packet. */
static bool twsPacketReadMiniSpadj(packet_slave_t *packet, rtime_spadj_mini_t *spadjm)
{
    tws_packet_slave_t *tp = &packet->slave.tws;
    if (twsPacketCalcUnread(tp) >= sizeof(rtime_spadj_mini_t))
    {
        *spadjm = *tp->ptr++;
        return TRUE;
    }
    return FALSE;
}

/* Assumes ptr addresses the next frame in the packet. */
static bool twsPacketReadAudioFrame(packet_slave_t *packet, uint8 *dest, uint32 frame_length, uint32 *frame_number)
{
    tws_packet_slave_t *tp = &packet->slave.tws;
    if (twsPacketCalcUnread(tp) >= frame_length)
    {
        memcpy(dest, tp->ptr, frame_length);
        tp->ptr += frame_length;
        *frame_number = tp->frames++;
        return TRUE;
    }
    return FALSE;
}

/* Assumes ptr addresses the the next frame in the packet. */
static bool twsPacketReadAudioFrameInfo(packet_slave_t *packet,
                                        frame_info_t *frame_info)
{
    tws_packet_slave_t *tp = &packet->slave.tws;
    if (tp->read_audio_frame_info_fptr)
    {
        return tp->read_audio_frame_info_fptr(tp, frame_info);
    }
    return FALSE;
}

static uint32 twsPacketGetTTPLenBits(packet_slave_t *packet)
{
    UNUSED(packet);
    return 8 * sizeof(ttp_bytes_t);
}

const packet_slave_functions_t packet_slave_funcs_tws = {
    .init = twsPacketSlaveInit,
    .unInit = twsPacketSlaveUninit,
    .readHeader = twsPacketReadHeader,
    .readMiniSpadj = twsPacketReadMiniSpadj,
    .readAudioFrame = twsPacketReadAudioFrame,
    .readAudioFrameInfo = twsPacketReadAudioFrameInfo,
    .getTTPLenBits = twsPacketGetTTPLenBits
};
