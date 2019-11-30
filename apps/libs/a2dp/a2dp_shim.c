/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */
#include <panic.h>
#include <message.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <vm.h>
#include <file.h>
#include <kalimba.h>
#include <kalimba_standard_messages.h>
#include "a2dp_private.h"
#include "a2dp_shim.h"
#include <transform.h>
#include <stream.h>
#include <sink.h>
#include <vmal.h>
#include <source.h>

#define SBC_SEID                1
#define MP3_SEID                2
#define KALIMBA_RESOURCE_ID     1

/* SBC configuration bit fields */
#define SBC_SAMPLING_FREQ_16000        128   /* Octet 0 */
#define SBC_SAMPLING_FREQ_32000         64
#define SBC_SAMPLING_FREQ_44100         32
#define SBC_SAMPLING_FREQ_48000         16
#define SBC_CHANNEL_MODE_MONO            8
#define SBC_CHANNEL_MODE_DUAL_CHAN       4
#define SBC_CHANNEL_MODE_STEREO          2
#define SBC_CHANNEL_MODE_JOINT_STEREO    1
#define SBC_BLOCK_LENGTH_4             128   /* Octet 1 */
#define SBC_BLOCK_LENGTH_8              64
#define SBC_BLOCK_LENGTH_12             32
#define SBC_BLOCK_LENGTH_16             16
#define SBC_SUBBANDS_4                   8
#define SBC_SUBBANDS_8                   4
#define SBC_ALLOCATION_SNR               2
#define SBC_ALLOCATION_LOUDNESS          1
#define SBC_BITPOOL_MIN                  2   /* Octet 2 (min bitpool)  /  Octet 3 (max bitpool) */
#define SBC_BITPOOL_MAX                250
#define SBC_BITPOOL_MEDIUM_QUALITY      32
#define SBC_BITPOOL_HIGH_QUALITY        50

/* MP3 configuration bit fields */
#define MP3_LAYER_I                    128   /* Octet 0 */
#define MP3_LAYER_II                    64
#define MP3_LAYER_III                   32
#define MP3_CRC_PROTECTION              16
#define MP3_CHANNEL_MODE_MONO            8
#define MP3_CHANNEL_MODE_DUAL_CHAN       4
#define MP3_CHANNEL_MODE_STEREO          2
#define MP3_CHANNEL_MODE_JOINT_STEREO    1
#define MP3_SAMPLING_FREQ_16000         32   /* Octet 1 */
#define MP3_SAMPLING_FREQ_22050         16
#define MP3_SAMPLING_FREQ_24000          8
#define MP3_SAMPLING_FREQ_32000          4
#define MP3_SAMPLING_FREQ_44100          2
#define MP3_SAMPLING_FREQ_48000          1
#define MP3_VBR                        128   /* Octet 2 */
#define MP3_BITRATE_VALUE_1110          64
#define MP3_BITRATE_VALUE_1101          32
#define MP3_BITRATE_VALUE_1100          16
#define MP3_BITRATE_VALUE_1011           8
#define MP3_BITRATE_VALUE_1010           4
#define MP3_BITRATE_VALUE_1001           2
#define MP3_BITRATE_VALUE_1000           1
#define MP3_BITRATE_VALUE_0111         128   /* Octet 3 */
#define MP3_BITRATE_VALUE_0110          64
#define MP3_BITRATE_VALUE_0101          32
#define MP3_BITRATE_VALUE_0100          16
#define MP3_BITRATE_VALUE_0011           8
#define MP3_BITRATE_VALUE_0010           4
#define MP3_BITRATE_VALUE_0001           2
#define MP3_BITRATE_VALUE_FREE           1


/*
    SBC sink caps.
    Support all features and full bitpool range. Note that we trust the source
    to choose a bitpool value suitable for the Bluetooth bandwidth.
*/
static const uint8 sbc_caps_sink[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_16000     + SBC_SAMPLING_FREQ_32000    + SBC_SAMPLING_FREQ_44100    + SBC_SAMPLING_FREQ_48000    +
    SBC_CHANNEL_MODE_MONO       + SBC_CHANNEL_MODE_DUAL_CHAN + SBC_CHANNEL_MODE_STEREO    + SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          + SBC_BLOCK_LENGTH_8         + SBC_BLOCK_LENGTH_12        + SBC_BLOCK_LENGTH_16        +
    SBC_SUBBANDS_4              + SBC_SUBBANDS_8             + SBC_ALLOCATION_SNR         + SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    2,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,      /* New for AVDTP v1.3 */
    0
};


/*
    MP3 sink caps.
    Support all features except:
    - MPEG-1 Layer 3
    - MPF-1
    - free rate
*/
static const uint8 mp3_caps_sink[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,

    MP3_LAYER_III              + MP3_CRC_PROTECTION          +
    MP3_CHANNEL_MODE_MONO      + MP3_CHANNEL_MODE_DUAL_CHAN  + MP3_CHANNEL_MODE_STEREO     + MP3_CHANNEL_MODE_JOINT_STEREO,

    MP3_SAMPLING_FREQ_32000    + MP3_SAMPLING_FREQ_44100     + MP3_SAMPLING_FREQ_48000,

    MP3_VBR                    + MP3_BITRATE_VALUE_1110      + MP3_BITRATE_VALUE_1101      + MP3_BITRATE_VALUE_1100    +
    MP3_BITRATE_VALUE_1011     + MP3_BITRATE_VALUE_1010      + MP3_BITRATE_VALUE_1001      + MP3_BITRATE_VALUE_1000,

    MP3_BITRATE_VALUE_0111     + MP3_BITRATE_VALUE_0110      + MP3_BITRATE_VALUE_0101      + MP3_BITRATE_VALUE_0100    +
    MP3_BITRATE_VALUE_0011     + MP3_BITRATE_VALUE_0010      + MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */

    AVDTP_SERVICE_CONTENT_PROTECTION,
    2,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,      /* New for AVDTP v1.3 */
    0
};


/* SBC source caps */
static const uint8 sbc_caps_source[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_16000 + SBC_SAMPLING_FREQ_32000    + SBC_SAMPLING_FREQ_44100    + SBC_SAMPLING_FREQ_48000    +
    SBC_CHANNEL_MODE_MONO       + SBC_CHANNEL_MODE_DUAL_CHAN + SBC_CHANNEL_MODE_STEREO    + SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          + SBC_BLOCK_LENGTH_8          + SBC_BLOCK_LENGTH_12         + SBC_BLOCK_LENGTH_16         +
    SBC_SUBBANDS_4              + SBC_SUBBANDS_8              + SBC_ALLOCATION_SNR          + SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_MAX,

    AVDTP_SERVICE_DELAY_REPORTING,      /* New for AVDTP v1.3 */
    0
};

/*
    MP3 source caps.
    Support all features except:
    - MPEG-1 Layer 3
    - MPF-1
    - free rate
*/
static const uint8 mp3_caps_source[] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,

    MP3_LAYER_III              + MP3_CRC_PROTECTION          +
    MP3_CHANNEL_MODE_MONO      + MP3_CHANNEL_MODE_DUAL_CHAN  + MP3_CHANNEL_MODE_STEREO     + MP3_CHANNEL_MODE_JOINT_STEREO,

    MP3_SAMPLING_FREQ_32000    + MP3_SAMPLING_FREQ_44100     + MP3_SAMPLING_FREQ_48000,

    MP3_VBR                    + MP3_BITRATE_VALUE_1110      + MP3_BITRATE_VALUE_1101      + MP3_BITRATE_VALUE_1100    +
    MP3_BITRATE_VALUE_1011     + MP3_BITRATE_VALUE_1010      + MP3_BITRATE_VALUE_1001      + MP3_BITRATE_VALUE_1000,

    MP3_BITRATE_VALUE_0111     + MP3_BITRATE_VALUE_0110      + MP3_BITRATE_VALUE_0101      + MP3_BITRATE_VALUE_0100    +
    MP3_BITRATE_VALUE_0011     + MP3_BITRATE_VALUE_0010      + MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */

    AVDTP_SERVICE_CONTENT_PROTECTION,
    2,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,      /* New for AVDTP v1.3 */
    0
};

static const sep_config_type sbc_sep_sink = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(sbc_caps_sink), sbc_caps_sink };
static const sep_config_type mp3_sep_sink = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(mp3_caps_sink), mp3_caps_sink };

static const sep_config_type sbc_sep_source = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 1, 0, sizeof(sbc_caps_source), sbc_caps_source };
static const sep_config_type mp3_sep_source = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 1, 0, sizeof(mp3_caps_source), mp3_caps_source };

static const sep_config_type sbc_sep_source_no_select = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(sbc_caps_source), sbc_caps_source };



static uint8 *reconfig_caps = NULL;


void A2dpHandleComplexMessage(Task task, MessageId id, Message message)
{
#if 0
    switch (id)
    {
        case A2DP_CODEC_SETTINGS_IND:
            {
                A2DP_CODEC_SETTINGS_IND_T *our_msg = (A2DP_CODEC_SETTINGS_IND_T*)message;

                A2DP_CODEC_SETTINGS_IND_TEST_EXTRA_T *pdu = malloc(sizeof(A2DP_CODEC_SETTINGS_IND_TEST_EXTRA_T));
                memset(pdu, 0, sizeof(A2DP_CODEC_SETTINGS_IND_TEST_EXTRA));

                pdu->a2dp = our_msg->a2dp;
                pdu->rate = our_msg->rate;
                pdu->channel_mode = our_msg->channel_mode;
                pdu->seid = our_msg->seid;
                pdu->content_protection = our_msg->codecData.content_protection;
                pdu->voice_rate = our_msg->codecData.voice_rate;
                pdu->bitpool = our_msg->codecData.bitpool;
                pdu->format = our_msg->codecData.format;
                pdu->packet_size = our_msg->codecData.packet_size;

                MessageSend(task, A2DP_CODEC_SETTINGS_IND_TEST_EXTRA, pdu);
            }
            break;
        default:
            Panic();
            break;
    }
#else
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);

    Panic();
#endif
}


void A2dpInitTestExtraDefault( Task theAppTask, uint8 role, bool enable_mp3 )
{

    sep_data_type sep[6];
    uint8 number_of_seps = 0;


    if( role & A2DP_INIT_ROLE_SINK)
    {
        sep[number_of_seps].sep_config = &sbc_sep_sink;
        sep[number_of_seps].in_use = FALSE;
        number_of_seps++;

        if(enable_mp3)
        {
            sep[number_of_seps].sep_config = &mp3_sep_sink;
            sep[number_of_seps].in_use = FALSE;
            number_of_seps++;
        }
    }

    if( role & A2DP_INIT_ROLE_SOURCE)
    {
        sep[number_of_seps].sep_config = &sbc_sep_source;
        sep[number_of_seps].in_use = FALSE;
        number_of_seps++;

        if(enable_mp3)
        {
            sep[number_of_seps].sep_config = &mp3_sep_source;
            sep[number_of_seps].in_use = FALSE;
            number_of_seps++;
        }
    }

    A2dpInit(theAppTask, role, NULL, number_of_seps, sep, 10);
}


void A2dpInitTestExtraAppselect(Task theAppTask, uint16 linkloss_timeout)
{
    sep_data_type sep[1];

    sep[0].sep_config = &sbc_sep_source_no_select;
    sep[0].in_use = FALSE;

    A2dpInit(theAppTask, A2DP_INIT_ROLE_SOURCE, NULL, 1, sep, linkloss_timeout);
}


void A2dpConnectTestExtra(bdaddr *addr)
{
    if( !A2dpSignallingConnectRequest( addr ) ) Panic();

}
void A2dpConnectResponseTestExtra(uint16 device_id, bool accept)
{

    if( !A2dpSignallingConnectResponse( device_id, accept) ) Panic();

}
void A2dpDisconnectTestExtra(uint16 device_id)
{
    if(!A2dpSignallingDisconnectRequest(device_id) ) Panic();

}

void A2dpOpenTestExtra(uint16 device_id)
{

    if(! A2dpMediaOpenRequest( device_id, 0, NULL ) ) Panic();

}
void A2dpOpenResponseTestExtra(uint16 device_id, bool accept)
{

    if( !A2dpMediaOpenResponse( device_id, accept ) ) Panic();

}
void A2dpCloseTestExtra(uint16 device_id, uint16 stream_id)
{

    if( !A2dpMediaCloseRequest( device_id, stream_id ) ) Panic();
}
void A2dpMediaStartTestExtra(uint16 device_id, uint16 stream_id)
{

    if( !A2dpMediaStartRequest( device_id, stream_id ) ) Panic();

}

void A2dpMediaStartResponseTestExtra(uint16 device_id, uint16 stream_id, bool accept)
{
    if( !A2dpMediaStartResponse( device_id, stream_id, accept ) ) Panic();
}


void A2dpMediaSuspendTestExtra(uint16 device_id, uint16 stream_id)
{

    if(!A2dpMediaSuspendRequest( device_id, stream_id) ) Panic();

}


void A2dpMediaAvSyncDelayResponseTestExtra(uint16 device_id, uint8 stream_id, uint16 delay)
{
    if( !A2dpMediaAvSyncDelayResponse( device_id, stream_id, delay ) ) Panic();
}


void A2dpSendMediaPacketTestExtra(uint16 device_id, uint16 stream_id)
{
    static const uint8 pkt[] = { 0x80, 0x01, 0x50, 0xcf, 0xcb, 0xd7, 0xd0, 0x20, 0xfa, 0x84, 
                                 0xea, 0x9b, 0x05, 0x9c, 0xbd, 0x3b, 0xff, 0xfc, 0xca, 0x88,
                                 0x78, 0x33, 0x98, 0x86, 0x85, 0x66, 0x7f, 0xbe, 0xee, 0xdd };
    Sink media_sink = A2dpMediaGetSink( device_id, stream_id );
    uint8 *dest = SinkMap(media_sink) +  SinkClaim(media_sink, sizeof(pkt));
    memmove(dest, pkt, sizeof(pkt));
    SinkFlush(media_sink, sizeof(pkt));
}



void A2dpReconfigureTestExtra(uint16 device_id, uint16 stream_id, uint16 size_sep_caps, uint8 *sep_caps)
{
    if (reconfig_caps)
    {
        free(reconfig_caps);
        reconfig_caps = NULL;
    }

    if (size_sep_caps)
    {
        reconfig_caps = (uint8 *)malloc(size_sep_caps);
        memmove(reconfig_caps, sep_caps, size_sep_caps);
        A2dpMediaReconfigureRequest( device_id, stream_id, size_sep_caps, reconfig_caps);
    }
}

#ifndef HYDRACORE
static const char sbc_encoder[] = "sbc_encoder/sbc_encoder.kap";
void A2dpStartKalimbaStreaming(const A2DP *a2dp, uint16 media_sink)
{
    FILE_INDEX index = FILE_NONE;
    Transform t, l_t, r_t;
    
    Source l_src = StreamAudioSource(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A);
    Source r_src = StreamAudioSource(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_1, AUDIO_CHANNEL_B);
    Sink   l_snk = StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A);
    Sink   r_snk = StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_1, AUDIO_CHANNEL_B);

    UNUSED(a2dp);

    /* load SBC codec */
    index = FileFind(FILE_ROOT, sbc_encoder, sizeof(sbc_encoder)-1);
    if (!KalimbaLoad(index))
        /* codec load failure, Panic as the test isn't going to work now */
        Panic();

    /* Init and configure RTP */
    t = VmalTransformRtpSbcEncode(StreamKalimbaSource(0), (Sink)media_sink);
    TransformConfigure(t, VM_TRANSFORM_RTP_ENCODE_PACKET_SIZE, 668);
    TransformConfigure(t, VM_TRANSFORM_RTP_ENCODE_MANAGE_TIMING, FALSE);
    (void)TransformStart(t);

    /* Configure SBC encoding format */
    (void)PanicFalse(KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_PARAMS, 0x00bd, 0, 0, 0));
    (void)PanicFalse(KalimbaSendMessage(KALIMBA_MSG_SBCENC_SET_BITPOOL, 0x0030, 0, 0, 0));

    SourceClose(l_src);
    SourceClose(r_src);
    SinkClose(l_snk);
    SinkClose(r_snk);
    
    (void) SourceConfigure(l_src, STREAM_CODEC_INPUT_RATE, 44100);
    (void) SourceConfigure(r_src, STREAM_CODEC_INPUT_RATE, 44100);
    (void) SourceSynchronise(l_src, r_src);
    
    /* set up ADCs */
    l_t = StreamConnect(l_src, StreamKalimbaSink(0));
    r_t = StreamConnect(r_src, StreamKalimbaSink(1));
    (void)TransformStart(l_t);
    (void)TransformStart(r_t);
    
    /* Start decode */
    (void) PanicFalse(KalimbaSendMessage(KALIMBA_MSG_GO,0,0,0,0));
}

void A2dpStopKalimbaStreaming(void)
{
    KalimbaPowerOff();
}
#endif

