/****************************************************************************
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup rtp_decode
 * \file  rtp_decode_struct.h
 * \ingroup capabilities
 *
 * Rtp decode operator header file containing type definitions shared
 * between C and asm. <br>
 *
 */

#ifndef RTP_DECODE_STRUCT_H
#define RTP_DECODE_STRUCT_H
/*****************************************************************************
Include Files
*/
#include "buffer/octet_buffer.h"
#include "stream/stream_audio_data_format.h"
#include "ttp/ttp.h"

/****************************************************************************
Public Constant Definitions
*/
#define DEBUG_RTP_DECODE

#ifdef DEBUG_RTP_DECODE
#define TIMESTAMP_BUFFER_SIZE (8)
#endif

/****************************************************************************
Public Type Declarations
*/

/** Rtp working modes  */
typedef enum RTP_WORKING_MODES_STRUCT
{
    RTP_PASSTHROUGH = 0,    /* When no RTP header is present. */
    RTP_DECODE = 1,         /* Decodes the RTP header and calls the TTP module to get the
                               time to play information. */
    RTP_STRIP = 2,          /* Decodes the RTP header and strips it. */
    RTP_TTP_ONLY = 3,       /* Append time-to-play to streams with no RTP header */
    RTP_NR_OF_MODES
} RTP_WORKING_MODES;


/** Rtp codec type  */
typedef enum RTP_CODEC_STRUCT
{
    APTX = 0,                /* APTX */
    SBC = 1,                 /* SBC */
    ATRAC = 2,               /* ATRAC */
    MP3 = 3,                 /* MP3 */
    AAC = 4,                 /* AAC (not supported yet) */
    APTXHD = 5,              /* APTXHD */
    NR_OF_CODECS
} RTP_CODEC_TYPE;

/** AAC utility select  */
typedef enum AAC_UTILITY_STRUCT
{
    AAC_LC_OVER_LATM = 0,    /* AAC-LC transported ove LATM  */
    AAC_GENERIC = 1,         /* GEN */
    NR_OF_AAC_UTILITIES
} AAC_UTILITY_SEL;



/** Rtp decode capability specific extra operator data */
typedef struct RTP_DECODE_OP_DATA
{
    /** The audio data format configurations of the input terminal */
    AUDIO_DATA_FORMAT ip_format;

    /** The audio data format configurations of the output terminal */
    AUDIO_DATA_FORMAT op_format;

    /* Working mode. */
    RTP_WORKING_MODES mode;

    /** sequence number */
    unsigned int sequence_number;

    /** SCMS content protection state. 0 if disabled. */
    unsigned int scms_enable:1;

    /** Codec Type */
    RTP_CODEC_TYPE codec_type;

    /**
     * To get the sample count for AAC frame an AAC decoder is used. This decoder must
     * be set with OPMSG_RTP_DECODE_ID_SET_AAC_DECODER operator message.
     */
    void* aac_codec;

    AAC_UTILITY_SEL aac_utility_select;

    /**
     * True, if this tag holds the continuation (second or third fragment) for the
     * encoded frame.
     */
    bool continuation_for_frame;

    /**
     * For decoded frames over multiple RTP packets only the first time of arrival is
     * used for timestamp generation.
     */
    unsigned first_tag_time_of_arrival;

    /**
     * Size of the payload header. Can be different depending on the packet.
     * See constants in rtp_decode_private.h.
     */
    unsigned int payload_header_size;

#ifdef DEBUG_RTP_DECODE
    unsigned rtp_decode_late;
    unsigned rtp_decode_lost;
    unsigned rtp_decode_packet_count;
    unsigned rtp_decode_last_tag_index;
    unsigned rtp_decode_last_tag_length;
#endif

    /** The buffer at the input terminal for this channel */
    octet_buffer *ip_buffer;

    /** The buffer at the output terminal for this channel */
    octet_buffer *op_buffer;

    /**
     * Internal clone of the output buffer used to hold the data until a full frame is
     * in it to get the sample count.
     */
    octet_buffer *clone_op_buffer;
    
    /** Location of the playing state flag signalled by the connected source endpoint */
    bool *play;

    /** Time to play context. */
    ttp_context* ttp_instance;

    /** Latency buffer size */
    unsigned latency_buffer_size;

    /** Configured sample rate */
    unsigned sample_rate;

    /** Max output packet length (for unframed data i.e. aptX only) */
    unsigned max_packet_len;
    
    /** ToA timestamp from the last metadata tag processed */
    TIME last_tag_timestamp;

    /** TRUE if last_tag_timestamp has a valid ToA */
    bool last_toa_valid;

    /** Octets written since the last output metadata tag */
    unsigned last_op_tag_octets;

    /** TRUE if RTP decode should kick forwards if limited by output space */
    bool kick_on_full_output;

} RTP_DECODE_OP_DATA;


/** Rtp decode will try to decode the encoded frame header for a better sample count
 * determination and to tag each encoded frame for TWS case. */
typedef struct RTP_FRAME_DECODE_DATA
{
    /** Field signals if the encoded frame header decode was successful. */
    bool valid;

    /** Frame size in octets. */
    unsigned frame_length;

    /** Samples in a frame. */
    unsigned frame_samples;

    /** Number of frames in the payload. */
    unsigned nr_of_frames;
} RTP_FRAME_DECODE_DATA;

#endif /* RTP_DECODE_STRUCT_H */
