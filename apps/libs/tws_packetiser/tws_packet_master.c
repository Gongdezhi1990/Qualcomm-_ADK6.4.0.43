/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    tws_packet_master.c
*/

#include "packet_master.h"
#include "tws_packet_private.h"
#include <packetiser_helper.h>
#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <hydra_macros.h>

/* For master, calculate the number of bytes space in the packet */
static uint32 calcSpace(tws_packet_master_t *tp)
{
    ptrdiff_t space = tp->buffer + tp->len - tp->ptr;
    if (space < 0)
        Panic();
    return space;
}

/* Write the 24-bit TTP to dest */
static void writeTTP(tws_packet_master_t *tp, rtime_t ttp)
{
    uint32 sizeof_ttpb = sizeof(ttp_bytes_t);
    ttp_bytes_t t;
    t.ttp[0] = (ttp >> 16) & 0xff;
    t.ttp[1] = (ttp >> 8) & 0xff;
    t.ttp[2] = ttp & 0xff;
    /* Write TTP to the start of the buffer */
    memcpy(tp->buffer, &t, sizeof_ttpb);
    tp->ptr = tp->buffer + sizeof_ttpb;
}

static bool init(packet_master_t *packet, tws_packetiser_codec_t codec, bool content_protection_header_enabled)
{
    UNUSED(content_protection_header_enabled);
    if (packet)
    {
        tws_packet_master_t *tp = &packet->master.tws;
        tp->src_read_offset = 0;

        switch(codec)
        {
            case TWS_PACKETISER_CODEC_SBC:
                tp->one_frame_per_packet = FALSE;
                tp->fragmentation_allowed = FALSE;
                tp->mini_sample_period_adj_only_at_first_frame = FALSE;
                break;
            case TWS_PACKETISER_CODEC_AAC:
            case TWS_PACKETISER_CODEC_MP3:
                tp->one_frame_per_packet = TRUE;
                tp->fragmentation_allowed = TRUE;
                tp->mini_sample_period_adj_only_at_first_frame = FALSE;
                break;
            case TWS_PACKETISER_CODEC_APTX_AD:
                tp->one_frame_per_packet = TRUE;
                tp->fragmentation_allowed = FALSE;
                tp->mini_sample_period_adj_only_at_first_frame = FALSE;
                break;
            case TWS_PACKETISER_CODEC_APTX:
                tp->one_frame_per_packet = FALSE;
                tp->fragmentation_allowed = FALSE;
                tp->mini_sample_period_adj_only_at_first_frame = TRUE;
                break;
            default:
                return FALSE;
        }
        return TRUE;
    }
    return FALSE;
}

static bool packetInit(packet_master_t *packet, uint8 *buffer, uint32 buffer_len)
{
    if (packet && buffer && (buffer_len >= TWS_HEADER_LEN))
    {
        tws_packet_master_t *tp = &packet->master.tws;
        tp->buffer = buffer;
        tp->len = buffer_len;
        tp->ptr = buffer;
        tp->frames = 0;
        tp->has_fragment_latched = FALSE;
        return TRUE;
    }
    return FALSE;
}

static uint32 packetLength(packet_master_t *packet)
{
    tws_packet_master_t *tp = &packet->master.tws;
    ptrdiff_t packet_length = tp->ptr - tp->buffer;
    if (packet_length < 0)
        Panic();
    return packet_length;
}

/* Return the length of the TWS header */
static uint32 headerLength(packet_master_t *packet, uint32 number_audio_frames)
{
    UNUSED(packet);
    return sizeof(ttp_bytes_t) + TAG_LENGTH + (number_audio_frames * sizeof(rtime_spadj_mini_t));
}

/* On exit from the function, the ptr points to the address where the first
   audio frame will be written. The same TTP must be set for all packets containing
   fragments of the same frame. */
static void writeHeader(packet_master_t *packet, rtime_t ttp, packetiser_helper_scmst_t scmst)
{
    tws_packet_master_t *tp = &packet->master.tws;
    writeTTP(tp, ttp);
    /* Write audio data tag (extended audio header absent) */
    *tp->ptr++ = TAG_AUDIO_TYPE_MASK | TAG_SET_AUDIO_SCMST_TYPE(scmst);
}

static uint32 amount_to_copy(tws_packet_master_t *tp, uint32 frame_length)
{
    uint32 remaining = frame_length - tp->src_read_offset;
    uint32 spadj_space = (tp->src_read_offset == 0) ? sizeof(rtime_spadj_mini_t) : 0;
    uint32 space = calcSpace(tp);
    uint32 to_copy = 0;
    if (space > spadj_space)
    {
        to_copy = MIN(space - spadj_space, remaining);
    }
    return to_copy;
}

static bool isSamplePeriodAdjRequiredForEveryFrame(tws_packet_master_t *tp)
{
    return((tp->src_read_offset == 0) && (!tp->mini_sample_period_adj_only_at_first_frame));
}

static bool isFirstFrameAndSamplePeriodAdjRequired(tws_packet_master_t *tp)
{
    return((tp->frames == 0) && (tp->mini_sample_period_adj_only_at_first_frame));
}

static bool writeAudioFrame(packet_master_t *packet, const uint8 *src, uint32 frame_length,
                            audio_frame_metadata_t *fmd)
{
    tws_packet_master_t *tp = &packet->master.tws;
    uint32 to_copy = amount_to_copy(tp, frame_length);
    bool is_fragment = (to_copy != frame_length);

    if (!to_copy ||
        (tp->frames && (is_fragment || tp->has_fragment_latched || tp->one_frame_per_packet || fmd->start_of_stream)))
    {
        return FALSE;
    }

    /* Fail if we have a fragment but fragmentation is not allowed, something has gone
       wrong with mtu or configuration */
    if (is_fragment && !tp->fragmentation_allowed)
    {
        Panic();
    }

    if (isSamplePeriodAdjRequiredForEveryFrame(tp) ||
        isFirstFrameAndSamplePeriodAdjRequired(tp))
    {
        /* Each unfragmented frame or first frame fragment has a mini sample period adjustment,
           fragments after the first fragment do not have a mini spadj */
        *tp->ptr++ = RtimeSpadjFullToMini(fmd->sample_period_adjustment);
    }

    memcpy(tp->ptr, src + tp->src_read_offset, to_copy);
    tp->ptr += to_copy;
    tp->src_read_offset += to_copy;
    tp->frames++;
    tp->has_fragment_latched |= is_fragment;
    
    if (tp->src_read_offset == frame_length)
    {
        /* Frame is completely written */
        tp->src_read_offset = 0;
        return TRUE;
    }

    /* This packet contains an incomplete frame fragment, update packet header */
    tp->buffer[sizeof(ttp_bytes_t)] |= TAG_AUDIO_FRAME_INCOMPLETE_MASK;
    return FALSE;
}

static void droppedAudioFrame(packet_master_t *p, const uint8 *src, uint32 frame_length,
                              audio_frame_metadata_t *fmd)
{
    tws_packet_master_t *tp = &p->master.tws;
    tp->src_read_offset = 0;
    UNUSED(src);
    UNUSED(frame_length);
    UNUSED(fmd);
}

/*! The tws packetiser takes no action to finalise a frame prior to transmission */
static void finalise(packet_master_t *p)
{
    UNUSED(p);
}

const packet_master_functions_t packet_master_funcs_tws = {
    .init = init,
    .packetInit = packetInit,
    .packetLength = packetLength,
    .headerLength = headerLength,
    .writeHeader = writeHeader,
    .writeAudioFrame = writeAudioFrame,
    .droppedAudioFrame = droppedAudioFrame,
    .finalise = finalise
};
