/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_frame_metadata.c
*/

#include "packetiser_helper.h"

/*! The start index of the TTP field */
#define AUDIO_FRAME_METADATA_TTP_INDEX 0
/*! The start index of the sample period adjustment field */
#define AUDIO_FRAME_METADATA_SAMPLE_PERIOD_ADJUSTMENT_INDEX 4
/*! The start index of the start of stream field */
#define AUDIO_FRAME_METADATA_START_OF_STREAM_INDEX 8
/*! The bit mask for the start of stream bit */
#define AUDIO_FRAME_METADATA_START_OF_STREAM_MASK 0x80

/*! Read four octets from src to form a uint32 */
static uint32 uint32Read(const uint8 *src)
{
    uint32 val;
    val = ((uint32)src[0] & 0xff);
    val |= ((uint32)src[1] & 0xff) << 8;
    val |= ((uint32)src[2] & 0xff) << 16;
    val |= ((uint32)src[3] & 0xff) << 24;
    return val;
}

/*! Write four octets from val to dest */
static void uint32Write(uint8 *dest, uint32 val)
{
    dest[0] = val & 0xff;
    dest[1] = (val >> 8) & 0xff;
    dest[2] = (val >> 16) & 0xff;
    dest[3] = (val >> 24) & 0xff;
}

/*! Get the TTP from the header */
static uint32 audioFrameMetadataGetTTP(const uint8 *hdr)
{
    return uint32Read(hdr + AUDIO_FRAME_METADATA_TTP_INDEX);
}

/*! Get the sample period adjustment from the header */
static uint32 audioFrameMetadataGetSamplePeriodAdjustment(const uint8 *hdr)
{
    return uint32Read(hdr + AUDIO_FRAME_METADATA_SAMPLE_PERIOD_ADJUSTMENT_INDEX);
}

/*! Get the start of stream flag from the header */
static bool audioFrameMetadataGetStartOfStream(const uint8 *hdr)
{
    uint8 start_of_stream;
    hdr += AUDIO_FRAME_METADATA_START_OF_STREAM_INDEX;
    start_of_stream = *hdr & AUDIO_FRAME_METADATA_START_OF_STREAM_MASK;
    return  start_of_stream ? TRUE : FALSE;
}

/*! Set the TTP in the header */
static void audioFrameMetadataSetTTP(uint8 *hdr, uint32 ttp)
{
    uint32Write(hdr + AUDIO_FRAME_METADATA_TTP_INDEX, ttp);
}

/*! Set the sample period adjustment in the header */
static void audioFrameMetadataSetSamplePeriodAdjustment(uint8 *hdr, uint32 adjustment)
{
    uint32Write(hdr + AUDIO_FRAME_METADATA_SAMPLE_PERIOD_ADJUSTMENT_INDEX, adjustment);
}

/*! Set the start of stream flag in the header */
static void audioFrameMetadataSetStartOfStream(uint8 *hdr, bool start_of_stream)
{
    hdr[AUDIO_FRAME_METADATA_START_OF_STREAM_INDEX] = 0;
    if (start_of_stream)
        hdr[AUDIO_FRAME_METADATA_START_OF_STREAM_INDEX] |= AUDIO_FRAME_METADATA_START_OF_STREAM_MASK;
    else
        hdr[AUDIO_FRAME_METADATA_START_OF_STREAM_INDEX] &= ~AUDIO_FRAME_METADATA_START_OF_STREAM_MASK;
}

/*! Write the header with the fields in the md structure */
void PacketiserHelperAudioFrameMetadataSet(const audio_frame_metadata_t *md,
                                           uint8 *hdr)
{
    audioFrameMetadataSetTTP(hdr, md->ttp);
    audioFrameMetadataSetSamplePeriodAdjustment(hdr, md->sample_period_adjustment);
    audioFrameMetadataSetStartOfStream(hdr, md->start_of_stream);
}

/*! Parse the header to populate the md structure */
void PacketiserHelperAudioFrameMetadataGet(const uint8 *hdr,
                                           audio_frame_metadata_t *md)
{
    md->ttp = audioFrameMetadataGetTTP(hdr);
    md->sample_period_adjustment = audioFrameMetadataGetSamplePeriodAdjustment(hdr);
    md->start_of_stream = audioFrameMetadataGetStartOfStream(hdr);
}

/*! Read the header from a source then parse the header to populate the md structure */
bool PacketiserHelperAudioFrameMetadataGetFromSource(Source source,
                                                     audio_frame_metadata_t *md)
{
    /* The header must be the correct length */
    if (SourceSizeHeader(source) == AUDIO_FRAME_METADATA_LENGTH)
    {
        const uint8 *hdr = (uint8*)SourceMapHeader(source);
        if (hdr)
        {
            PacketiserHelperAudioFrameMetadataGet(hdr, md);
            return TRUE;
        }
    }
    return FALSE;
}
