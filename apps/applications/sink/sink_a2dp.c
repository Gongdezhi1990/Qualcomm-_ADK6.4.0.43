/*
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.  
*/

/*!
@file
@ingroup sink_app
@brief   a2dp initialisation and control functions
*/

#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_statemanager.h"
#include "sink_states.h"
#include "sink_a2dp.h"
#include "sink_a2dp_capabilities.h"
#include "sink_a2dp_aptx_adaptive.h"
#include "sink_devicemanager.h"
#include "sink_link_policy.h"
#include "sink_audio.h"
#include "sink_usb.h"
#include "sink_wired.h"
#include "sink_scan.h"
#include "sink_audio_routing.h"
#include "sink_slc.h"
#include "sink_device_id.h"
#include "sink_partymode.h"
#include "sink_config.h"
#include "sink_auth.h"
#include "sink_peer.h"
#include "sink_avrcp.h"
#include "sink_peer_qualification.h"
#include "sink_linkloss.h"
#include "sink_callmanager.h"
#include "sink_volume.h"

#include "sink_gaia.h"

#ifdef ENABLE_SUBWOOFER
#include "sink_swat.h"
#endif
#include "sink_hfp_data.h"
#include "sink_music_processing.h"
#include "sink_auto_power_off.h"

#ifdef ENABLE_AVRCP
#include "sink_tones.h"
#endif

#include "sink_ba.h"
#include "sink_powermanager.h"

#include <util.h>
#include <bdaddr.h>
#include <a2dp.h>
#include <connection.h>
#include <hfp.h>
#include <stdlib.h>
#include <panic.h>
#include <ps.h>
#include <message.h>
#include <audio_plugin_music_variants.h>
#include <audio_music_processing.h>
#include <audio_config.h>
#include <config_store.h>
#include <gain_utils.h>
#include <audio.h>

#include "sink_slc.h"
#include "sink_upgrade.h"

#include "config_definition.h"
#include "sink_a2dp_config_def.h"
#include "sink_malloc_debug.h"

#ifdef SBC_ENCODER_CONFORMANCE
#include "sink_sbc.h"
#endif

#ifdef DEBUG_A2DP
#define A2DP_DEBUG(x) DEBUG(x)
#ifdef ENABLE_PEER
static const char * const rdname[] = { "Unknown", "Non-Peer", "Peer" };
#endif
#else
#define A2DP_DEBUG(x)
#endif

/* Helper macro for converting a bypass bit flag into a boolean type. */
#define MusicEnhancementBypassed(enhancements, bypass_flag) (((enhancements) & (bypass_flag)) == (bypass_flag))

#ifdef ENABLE_PEER
#define SBC_SAMPLING_FREQ_16000        128
#define SBC_SAMPLING_FREQ_32000         64
#define SBC_SAMPLING_FREQ_44100         32
#define SBC_SAMPLING_FREQ_48000         16
#define SBC_CHANNEL_MODE_MONO            8
#define SBC_CHANNEL_MODE_DUAL_CHAN       4
#define SBC_CHANNEL_MODE_STEREO          2
#define SBC_CHANNEL_MODE_JOINT_STEREO    1

#define SBC_BLOCK_LENGTH_4             128
#define SBC_BLOCK_LENGTH_8              64
#define SBC_BLOCK_LENGTH_12             32
#define SBC_BLOCK_LENGTH_16             16
#define SBC_SUBBANDS_4                   8
#define SBC_SUBBANDS_8                   4
#define SBC_ALLOCATION_SNR               2
#define SBC_ALLOCATION_LOUDNESS          1

/* True Wireless Stereo Codec ID configured using either SBC, MP3, AAC or APTX type capabilities.
  * As a default configuration refer tws_sbc_caps for mandate SBC type media codec service category fixed offset value */
#define MEDIA_TRANSPORT_SERVICE_CATEGORY_OFFSET     2
#define CONFIGURED_MEDIA_CODEC_OFFSET     12

/* Codec caps to use for a TWS Source SEP when 16KHz SBC is required */
const uint8 sbc_caps_16k[8] =
{
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_16000 |  SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_16 | SBC_SUBBANDS_8 | SBC_ALLOCATION_SNR,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY
};


/* Codec caps to use for a TWS Source SEP when 32KHz SBC is required */
const uint8 sbc_caps_32k[8] =
{
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_32000 |  SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_16 | SBC_SUBBANDS_8 | SBC_ALLOCATION_SNR,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY
};


/* Codec caps to use for a TWS Source SEP when 44.1KHz SBC is required */
const uint8 sbc_caps_44k1[8] =
{
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_44100 |  SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_16 | SBC_SUBBANDS_8 | SBC_ALLOCATION_SNR,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY
};


/* Codec caps to use for a TWS Source SEP when 48KHz SBC is required */
const uint8 sbc_caps_48k[8] =
{
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_48000 |  SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_16 | SBC_SUBBANDS_8 | SBC_ALLOCATION_SNR,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY
};
#endif /* ENABLE_PEER */

static const sep_config_type sbc_sep_snk = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(sbc_caps_sink), sbc_caps_sink };
static const sep_config_type sbc_med_sep_snk = { SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(sbc_med_caps_sink), sbc_med_caps_sink };

#ifdef ENABLE_PEER
    static const sep_config_type sbc_sep_src = { SOURCE_SEID_MASK | SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(sbc_caps_sink), sbc_caps_sink };   /* Source shares same caps as sink */
#ifdef PEER_TWS
#ifndef SBC_ENCODER_CONFORMANCE
    static const sep_config_type tws_sbc_sep_snk = { TWS_SEID_MASK | SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(tws_sbc_caps), tws_sbc_caps };
    static const sep_config_type tws_sbc_sep_src = { SOURCE_SEID_MASK | TWS_SEID_MASK | SBC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(tws_sbc_caps), tws_sbc_caps };
#endif /* SBC_ENCODER_CONFORMANCE */
#endif /* PEER_TWS */
#endif /* ENABLE_PEER */

/* not all codecs are available for some configurations, include this define to have access to all codec types  */
#ifdef INCLUDE_A2DP_EXTRA_CODECS
    static const sep_config_type mp3_sep_snk = { MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(mp3_caps_sink), mp3_caps_sink };
    static const sep_config_type aac_sep_snk = { AAC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(aac_caps_sink), aac_caps_sink };
    static const sep_config_type aptx_sep_snk = { APTX_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(aptx_caps_sink), aptx_caps_sink };
    static const sep_config_type aptxhd_sep_snk = { APTXHD_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(aptxhd_caps_sink), aptxhd_caps_sink };
#ifdef INCLUDE_APTX_ADAPTIVE
    static const sep_config_type aptx_ad_sep_snk = {APTX_ADAPTIVE_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(aptx_ad_caps_sink), aptx_ad_caps_sink };
#endif
#ifdef ENABLE_PEER
#ifdef PEER_AS
    static const sep_config_type mp3_sep_src = { SOURCE_SEID_MASK | MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(mp3_caps_sink), mp3_caps_sink };   /* Source shares same caps as sink */
    static const sep_config_type aac_sep_src = { SOURCE_SEID_MASK | AAC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(aac_caps_sink), aac_caps_sink };   /* Source shares same caps as sink */
    static const sep_config_type aptx_sep_src = { SOURCE_SEID_MASK | APTX_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(aptx_caps_sink), aptx_caps_sink };   /* Source shares same caps as sink */
#elif defined PEER_TWS
    static const sep_config_type tws_mp3_sep_snk = { TWS_SEID_MASK | MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(tws_mp3_caps), tws_mp3_caps };
    static const sep_config_type tws_mp3_sep_src = { SOURCE_SEID_MASK | TWS_SEID_MASK | MP3_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(tws_mp3_caps), tws_mp3_caps };
    static const sep_config_type tws_aac_sep_snk = { TWS_SEID_MASK | AAC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(tws_aac_caps), tws_aac_caps };
    static const sep_config_type tws_aac_sep_src = { SOURCE_SEID_MASK | TWS_SEID_MASK | AAC_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(tws_aac_caps), tws_aac_caps };
    static const sep_config_type tws_aptx_sep_snk = { TWS_SEID_MASK | APTX_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(tws_aptx_caps), tws_aptx_caps };
    static const sep_config_type tws_aptx_sep_src = { SOURCE_SEID_MASK | TWS_SEID_MASK | APTX_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(tws_aptx_caps), tws_aptx_caps };
#ifdef INCLUDE_APTX_ADAPTIVE
    static const sep_config_type tws_aptx_ad_sep_snk = { TWS_SEID_MASK | APTX_ADAPTIVE_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(tws_aptx_ad_caps), tws_aptx_ad_caps };
    static const sep_config_type tws_aptx_ad_sep_src = { SOURCE_SEID_MASK | TWS_SEID_MASK | APTX_ADAPTIVE_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_source, 0, 0, sizeof(tws_aptx_ad_caps), tws_aptx_ad_caps };
#endif /* INCLUDE_APTX_ADAPTIVE */
#endif /* PEER_AS */
#endif /* ENABLE_PEER */
#ifdef INCLUDE_FASTSTREAM
    static const sep_config_type faststream_sep = { FASTSTREAM_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(faststream_caps_sink), faststream_caps_sink };
#endif /* INCLUDE_FASTSTREAM */
#ifdef INCLUDE_APTX_ACL_SPRINT
    static const sep_config_type aptx_sprint_sep = { APTX_SPRINT_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(aptx_acl_sprint_caps_sink), aptx_acl_sprint_caps_sink };
    static const sep_config_type aptx_ll_sep = { APTX_LL_SEID, KALIMBA_RESOURCE_ID, sep_media_type_audio, a2dp_sink, 1, 0, sizeof(aptx_ll_caps_sink), aptx_ll_caps_sink };
#endif /* INCLUDE_APTX_ACL_SPRINT */
#endif /* INCLUDE_A2DP_EXTRA_CODECS */


#define NUM_SEPS (sizeof(codecList)/sizeof(codec_list_element))

typedef struct
{
    unsigned                enable_bit:8;      /* The bit position in CONFIG_USR_xxx to enable the codec. */
    unsigned                include_mask:8;    /* Bit mask to include a codec based on application feature set */
    const sep_config_type   *config;    /* The SEP config data. These configs are defined above. */
    const TaskData          *plugin;    /* The audio plugin to use. */
} codec_list_element;

    /* Table which indicates which A2DP codecs are avaiable on the device.
       Add other codecs in priority order, from top to bottom of the table.
    */
    static const codec_list_element codecList[] =
    {
#ifdef INCLUDE_A2DP_EXTRA_CODECS
#ifdef INCLUDE_APTX_ACL_SPRINT
        /* AptX Sprint (low latency) Sink SEP */
        {APTX_SPRINT_CODEC_BIT, STD_SNK, &aptx_ll_sep, &csr_aptx_acl_sprint_decoder_plugin.data},
        {APTX_SPRINT_CODEC_BIT, STD_SNK, &aptx_sprint_sep, &csr_aptx_acl_sprint_decoder_plugin.data},
#endif
        /* High quality Sink SEPs */
#ifdef INCLUDE_APTX_ADAPTIVE
        {APTX_AD_CODEC_BIT, STD_SNK, &aptx_ad_sep_snk, &csr_aptx_ad_decoder_plugin.data},
#endif /* INCLUDE_APTX_ADAPTIVE */
        {APTXHD_CODEC_BIT, STD_SNK, &aptxhd_sep_snk, &csr_aptxhd_decoder_plugin.data},
        {APTX_CODEC_BIT, STD_SNK, &aptx_sep_snk, &csr_aptx_decoder_plugin.data},
        {AAC_CODEC_BIT, STD_SNK, &aac_sep_snk, &csr_aac_decoder_plugin.data},
        {MP3_CODEC_BIT, STD_SNK, &mp3_sep_snk, &csr_mp3_decoder_plugin.data},
#ifdef ENABLE_PEER
#ifdef PEER_AS
        /* ShareMe Source SEPs */
        {APTX_CODEC_BIT, STD_SRC, &aptx_sep_src, &csr_aptx_decoder_plugin.data},
        {AAC_CODEC_BIT, STD_SRC, &aac_sep_src, &csr_aac_decoder_plugin.data},
        {MP3_CODEC_BIT, STD_SRC, &mp3_sep_src, &csr_mp3_decoder_plugin.data},
#endif
#ifdef PEER_TWS
        /* TWS Source and Sink SEPs */ /* TODO: These could be moved as they don't need to be part of default search order - create lists for when using A2dpMedisOpenReq() ? */
#ifdef INCLUDE_APTX_ADAPTIVE
        {APTX_AD_CODEC_BIT, TWS_SRC, &tws_aptx_ad_sep_src, &csr_tws_master_aptx_ad_decoder_plugin.data},
        {APTX_AD_CODEC_BIT, TWS_SNK, &tws_aptx_ad_sep_snk, &csr_tws_slave_aptx_ad_decoder_plugin.data},
#endif /* INCLUDE_APTX_ADAPTIVE */
        {APTX_CODEC_BIT, TWS_SRC, &tws_aptx_sep_src, &csr_tws_master_aptx_decoder_plugin.data},
        {APTX_CODEC_BIT, TWS_SNK, &tws_aptx_sep_snk, &csr_tws_slave_aptx_decoder_plugin.data},
        {AAC_CODEC_BIT, TWS_SRC, &tws_aac_sep_src, &csr_tws_master_aac_decoder_plugin.data},
        {AAC_CODEC_BIT, TWS_SNK, &tws_aac_sep_snk, &csr_tws_slave_aac_decoder_plugin.data},
        {MP3_CODEC_BIT, TWS_SRC, &tws_mp3_sep_src, &csr_tws_master_mp3_decoder_plugin.data},
        {MP3_CODEC_BIT, TWS_SNK, &tws_mp3_sep_snk, &csr_tws_slave_mp3_decoder_plugin.data},
#endif
#endif
#ifdef INCLUDE_FASTSTREAM
        /* Faststream Sink SEP */
        {FASTSTREAM_CODEC_BIT, STD_SNK, &faststream_sep, &csr_faststream_sink_plugin.data},
#endif
#endif

#ifdef ENABLE_PEER
#ifdef PEER_TWS
#ifndef SBC_ENCODER_CONFORMANCE
        /* TWS Source and Sink SEPs */

        {SBC_CODEC_BIT, TWS_SRC, &tws_sbc_sep_src, &csr_tws_master_sbc_decoder_plugin.data},
        {SBC_CODEC_BIT, TWS_SNK, &tws_sbc_sep_snk, &csr_tws_slave_sbc_decoder_plugin.data},
#endif /* SBC_ENCODER_CONFORMANCE */
#endif /* PEER_TWS */
        /* Mandatory SBC Source SEP for both ShareMe and TWS */
        {SBC_CODEC_BIT, STD_SRC | TWS_SRC, &sbc_sep_src, &csr_sbc_decoder_plugin.data},
#endif /* ENABLE_PEER */
        /* Mandatory SBC Sink SEP */
        {SBC_CODEC_BIT, STD_SNK | TWS_SNK, &sbc_sep_snk, &csr_sbc_decoder_plugin.data}
    };

typedef struct
{
    unsigned codec_bit:8;
    unsigned codec_seid:8;
}codec_seid_t;

#define TWS_SUPPORTED_CODECS ((1 << MP3_CODEC_BIT) | (1 << AAC_CODEC_BIT) | (1 << APTX_CODEC_BIT) | (1 << APTX_AD_CODEC_BIT))

/* Extra time delay between a peer connecting and paging the next device in the pdl.
   It is to allow the peer relay state to change to streaming before we check if
   we can connect to the next device in the pdl. */
#define A2DP_PEER_PDL_PAGING_DELAY D_SEC(2)


/*This array maps the codec bits to SEID */
static const codec_seid_t optional_codec_seids[] =
{
    {1 << MP3_CODEC_BIT         , MP3_SEID} ,

    {1 << AAC_CODEC_BIT         , AAC_SEID} ,

    {1 << FASTSTREAM_CODEC_BIT  , FASTSTREAM_SEID} ,

    {1 << APTX_CODEC_BIT        , APTX_SEID} ,

    {1 << APTX_SPRINT_CODEC_BIT , APTX_LL_SEID},

    {1 << APTX_SPRINT_CODEC_BIT , APTX_SPRINT_SEID},

    {1 << APTXHD_CODEC_BIT      , APTXHD_SEID},

    {1 << APTX_AD_CODEC_BIT     , APTX_ADAPTIVE_SEID},

    {0                          , INVALID_SEID}
};

#ifdef ENABLE_PEER
/* After a timeout try to force a paused non-peer a2dp device to suspend its
   a2dp media channel. This is to avoid dropouts on the peer a2dp stream
   if it starts playing before the device has suspended its
   media channel itself. */
#define A2DP_PAUSE_SUSPEND_TIMER D_SEC(4)

#endif

/* TODO: Optimise memory usage */
typedef struct
{
    bool connected[MAX_A2DP_CONNECTIONS];                           /* :1 */
    bool media_reconnect[MAX_A2DP_CONNECTIONS];                     /* :1 */
    bool micMuted[MAX_A2DP_CONNECTIONS];                            /* :1 */
#ifdef PEER_SCATTERNET_DEBUG   /* Scatternet debugging only */
    bool invert_ag_role[MAX_A2DP_CONNECTIONS];                      /* :1 */
#endif
#ifdef ENABLE_PEER
    uint16 peer_link_loss_reconnect;                                /* :16 - used on MessageSendConditionally */
#endif
    remote_features peer_features[MAX_A2DP_CONNECTIONS];            /* :4 */
    remote_device peer_device[MAX_A2DP_CONNECTIONS];                /* :2 */
    uint16 peer_version[MAX_A2DP_CONNECTIONS];                      /* :16 */
    peer_buffer_level peer_dsp_required_buffering_level[MAX_A2DP_CONNECTIONS];   /* :1 */
    PeerStatus local_peer_status[MAX_A2DP_CONNECTIONS];             /* :2 */
    PeerStatus remote_peer_status[MAX_A2DP_CONNECTIONS];            /* :2 */
    bool playing[MAX_A2DP_CONNECTIONS];                             /* :1 */
    a2dp_link_role link_role[MAX_A2DP_CONNECTIONS];                 /* :2 */
    a2dp_suspend_state SuspendState[MAX_A2DP_CONNECTIONS];          /* :2 */
    uint8 av_source[MAX_A2DP_CONNECTIONS];                          /* :2 */
    uint8 device_id[MAX_A2DP_CONNECTIONS];                          /* :3 */
    uint8 stream_id[MAX_A2DP_CONNECTIONS];                          /* :1 */
    uint8 seid[MAX_A2DP_CONNECTIONS];                               /* :6 */
    uint16 latency[MAX_A2DP_CONNECTIONS];                           /* :16 */
    uint8 list_id[MAX_A2DP_CONNECTIONS];                            /* :8 */
    bdaddr bd_addr[MAX_A2DP_CONNECTIONS];
    uint16 clockMismatchRate[MAX_A2DP_CONNECTIONS];                 /* :16 */
    volume_info a2dp_volume[MAX_A2DP_CONNECTIONS];
#ifdef ENABLE_AVRCP
    avrcpSupport avrcp_support[MAX_A2DP_CONNECTIONS];               /* :2 */
#endif
    A2dpPluginConnectParams  a2dp_audio_connect_params;
    A2dpPluginModeParams     a2dp_audio_mode_params;

    unsigned    local_peer_optional_codecs:8;
    unsigned    remote_peer_optional_codecs:8;
    uint8*    dest_service_caps;          /* Remote device Reconfigure capabilities. ( used for PTS TWS qualification ) */
    bdaddr reconnected_ag_address;        /* BD Address of the AG to be reconnected*/

    uint16      ui_connect_update_delay;  /* Delay messages that cause UI updates for connection by this amount in ms. */

    unsigned    remote_ag_connection:1;
    unsigned    a2dp_initialised:1;
    /* The below flag is used for SNK Role PTS qualification. when this flag gets enabled then
     * the application will not resume stream after receiving the stream suspend confirmation.
     */
    unsigned    qual_disable_stream_resume:1;
    unsigned    unused13:13;

    bool paused_using_avrcp[MAX_A2DP_CONNECTIONS];
}a2dp_data;

static a2dp_data gA2dpData;

#define A2DP_DATA  gA2dpData

static uint16 a2dpMapMusicModeEnhancementsToA2dpData(a2dp_session_config_def_t *a2dp_config_data);
static void a2dpMapMusicModeEnhancementsToSessionData(uint16 music_mode_enhancements, a2dp_session_config_def_t *a2dp_config_data);
static void handleA2dpReconfigureCfm(uint8 deviceId,uint8 streamId,a2dp_status_code status);
static void handleA2dpReconfigureInd(uint8 deviceId, uint8 streamId);
static void sinkUpdateCodecConfiguredToApp( uint8 deviceId,uint8 streamId);
#define MAX_EQ_BANDS 5

/****************************************************************************
  FUNCTIONS
*/

#if 0
/* Calculates Volume = (Gain / Scale) * Range
   Gain is 0..Scale, thus Gain/Scale is 0..1  and hence Volume is 0..Range

   Method of calculation keeps everything as 16-bit arithmetic
*/
#define HI(n) ((uint8)((n) >> 8))
#define LO(n) ((uint8)(n))
static uint16 calculateVolume (uint16 gain, uint16 scale, uint16 range)
{
    volume = (HI(gain) * HI(range)) / HI(scale);
    volume += (HI(gain) * LO(range)) / HI(scale);
    volume += (LO(gain) * HI(range)) / LO(scale);
    volume += (LO(gain) * LO(range)) / LO(scale);

    return volume;
}
#endif


/*************************************************************************
NAME
    a2dpIsIndexPeer

DESCRIPTION
    Determines whether the provided a2dp index is of a peer device or not.

RETURNS
    TRUE if the index is that of a peer device, false otherwise.

**************************************************************************/
#ifdef ENABLE_PEER

bool a2dpIsIndexPeer(uint16 index)
{
    if (SinkA2dpIsInitialised() && (getA2dpPeerRemoteDevice(index) == remote_device_peer))
    {
        return TRUE;
    }
    return FALSE;
}

#endif


/*************************************************************************
NAME
    sinkA2dpGetSupportedOptionalcodecs

DESCRIPTION
    Function to read A2DP config data and return the supported optional codecs

RETURNS
    TRUE: Success, FALSE: Failure

**************************************************************************/
static bool sinkA2dpGetSupportedOptionalcodecs(uint8 *a2dp_optional_codecs)
{
    bool status = FALSE;
    a2dp_config_def_t *a2dp_config_data = NULL;

    if (configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {

        *a2dp_optional_codecs = a2dp_config_data->A2dpOptionalCodecsEnabled;

        if(sink_ba_appmode_broadcaster == sinkBroadcastAudioGetMode())
        {
            /* In Broadcaster mode only standard SBC and AAC codecs are supported */
            *a2dp_optional_codecs &= sinkBroadcastAudioGetOptionalCodecsMask();
        }
        else
        {

            /* if aptX Low Latency is enabled, automatically enable standard aptX */
            if(*a2dp_optional_codecs & (1<<APTX_SPRINT_CODEC_BIT))
            {
                *a2dp_optional_codecs |= (1<<APTX_CODEC_BIT);
            }

        }
        status = TRUE;
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return status;
}

/*************************************************************************
NAME
    setupAudioConnectParams

DESCRIPTION
    Function to initialize the audio plugin connect parameters

RETURNS
    None

**************************************************************************/
static void setupAudioConnectParams(void)
{
    A2dpPluginConnectParams *audio_connect_params;

    audio_connect_params = getAudioPluginConnectParams();
    /* Make sure all references to mic parameters point to the right place */
    sinkAudioGetCommonMicParams(&audio_connect_params->mic_params);
    sinkAudioGetCommonAnalogueInputParams(&audio_connect_params->analogue_in_params);

    audio_connect_params->silence_threshold = sinkAudioGetSilenceThreshold();
    audio_connect_params->silence_trigger_time = sinkAudioGetSilenceTriggerTime();
}

/*************************************************************************
NAME
    setupAudioModeParams

DESCRIPTION
    Function to initialize the audio mode setting parameters

RETURNS
    None

**************************************************************************/
static void setupAudioModeParams(void)
{
    /* set default microphone source for back channel enabled dsp apps */
    setExternalMic(EXTERNAL_MIC_NOT_FITTED);
    updateMicMuteState(SEND_PATH_UNMUTE);

    /* Set default TWS audio routing modes */
    setTWSRoutingMode(tws_master, sinkAudioGetTwsMasterAudioRouting());
    setTWSRoutingMode(tws_slave, sinkAudioGetTwsSlaveAudioRouting());

    AudioSetTwsChannelModes(NULL, getTWSRoutingMode(tws_master), getTWSRoutingMode(tws_slave));
}

#if defined ENABLE_PEER || defined ENABLE_AVRCP
/*************************************************************************
NAME
    getA2dpAgToReconnect

DESCRIPTION
    Function to retunr the BD address of the AG to be reconnected

RETURNS
    Address of the AG to be reconnected

**************************************************************************/
static bdaddr* getA2dpAgToReconnect(void)
{
    return &A2DP_DATA.reconnected_ag_address;
}
#endif
/*************************************************************************
NAME
    getCodecIncludeMask

DESCRIPTION
    Determines mask of codec types to use based on a set of application
    features, namely whether TWS and/or ShareMe are supported and in which
    AVDTP roles (Source or Sink).

RETURNS
    Bitmask of codec types to use

**************************************************************************/
static uint8 getCodecIncludeMask (void)
{
    uint8 include_mask;

    /* Always include standard Sink SEPs */
    include_mask = STD_SNK;

    /* In BA Mode only standard sink type is supported */
    if(!sinkBroadcastAudioIsActive())
    {
#ifdef ENABLE_PEER
        if (PeerIsShareMeSourceEnabled())
        {
            include_mask |= STD_SRC;
        }

        if (PeerIsTwsSinkEnabled())
        {
            include_mask |= TWS_SNK;
        }

        if (PeerIsTwsSourceEnabled())
        {
            include_mask |= TWS_SRC;
        }
#endif
    }

    return include_mask;
}

static void updateStateManagerFollowingA2dpSuspend(void)
{
    /* no longer streaming so enter connected state if applicable */
    if(stateManagerGetState() == deviceA2DPStreaming)
    {
        /* the enter connected state function will determine if the signalling
        channel is still open and make the approriate state change */
        stateManagerEnterConnectedState();
    }
}

static bool pauseUsingAvrcp(a2dp_index_t index)
{
    bool status = FALSE;

#ifdef ENABLE_AVRCP
    uint8 i = 0;
    /* does the device support AVRCP and is AVRCP currently connected to this device? */
    for_all_avrcp(i)
    {
        /* ensure media is streaming and the avrcp channel is that requested to be paused */
        if ((sinkAvrcpIsConnected(i))&&(getA2dpStatusFlag(CONNECTED, index))&&
                (BdaddrIsSame(getA2dpLinkBdAddr(index), sinkAvrcpGetLinkBdAddr(i))))
        {
            /* check whether the a2dp connection is streaming data */
            if (A2dpMediaGetState(getA2dpLinkDataDeviceId(i), getA2dpLinkDataStreamId(i)) == a2dp_stream_streaming)
            {
                /* attempt to pause the a2dp stream */
                status = sinkAvrcpPlayPauseRequest(i,AVRCP_PAUSE);
            }
            break;
        }
    }
#endif

    return status;
}

static void stopA2dpMediaStream(a2dp_index_t index)
{
    if(!A2dpMediaSuspendRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)))
    {
        A2dpMediaCloseRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
    }
}

static void notifyAssociatedCodecExit(a2dp_index_t index)
{
    a2dp_codec_settings * codec_settings = NULL;
    codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
    if(codec_settings != NULL)
    {
        sinkA2dpIndicateCodecExit(codec_settings->seid);
        free(codec_settings);
    }
}

static void suspendA2dpStream(a2dp_index_t index, bool try_avrcp_pause)
{
    a2dpSetSuspendState(index, a2dp_local_suspended);
    notifyAssociatedCodecExit(index);
    
    try_avrcp_pause &= sinkA2dpIsStreamingAllowed();

    if(try_avrcp_pause && pauseUsingAvrcp(index))
        A2DP_DATA.paused_using_avrcp[index] = TRUE;
    else
        stopA2dpMediaStream(index);

    updateStateManagerFollowingA2dpSuspend();
}

static bool isA2dpStreamSuspendable(a2dp_index_t index)
{
    return getA2dpStatusFlag(CONNECTED, index);
}

static bool a2dpIndexIsValidSource(a2dp_index_t index)
{
    if (SinkA2dpIsInitialised() && (getA2dpPeerRemoteDevice(index) != remote_device_peer))
        return TRUE;

    return FALSE;
}

static void SuspendA2dpStreamNoAutomaticResume(a2dp_index_t index)
{
    if(a2dpIndexIsValidSource(index) && isA2dpStreamSuspendable(index))
    {
        setA2dpStatusFlag(PLAYING, index, FALSE);
        suspendA2dpStream(index, FALSE);
    }
}

static void suspendAllA2dpStreams(bool no_automatic_resume)
{
    uint8 index = 0;
    for(index = 0; index < MAX_A2DP_CONNECTIONS; index++)
    {
        if(!sinkA2dpIsA2dpLinkSuspended(index))
        {
            if(getA2dpPeerRemoteDevice(index) == remote_device_peer)
            {
                #ifdef ENABLE_PEER
                if(a2dpIssuePeerSuspendRequest())
                {
                    updateCurrentState(RELAY_STATE_SUSPENDING);
                    peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_RELAY_UNAVAILABLE);
                }
                #endif
            }
            else if (no_automatic_resume)
            {
                SuspendA2dpStreamNoAutomaticResume(index);
            }
            else
            {
                SuspendA2dpStream(index);
            }
        }
    }
}

/*************************************************************************
NAME
    SinkA2dpIsInitialised

DESCRIPTION
    Test if the sink_a2dp module data has been initialised.

RETURNS
    TRUE if InitA2dp has been called; FALSE otherwise.

**************************************************************************/
bool SinkA2dpIsInitialised(void)
{
    return (A2DP_DATA.a2dp_initialised ? TRUE : FALSE);
}

/*************************************************************************
NAME
    getA2dpLinkBdAddr

DESCRIPTION
    Returns the address of A2DP Remote Device bd address for the specified index.

RETURNS
    Remote device bd addr

**************************************************************************/
bdaddr *getA2dpLinkBdAddr(uint16 index)
{
    return &A2DP_DATA.bd_addr[index];
}

/*************************************************************************
NAME
    setA2dpLinkBdAddr

DESCRIPTION
    Updates the A2DP Remote Device bd address for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpLinkBdAddr(uint16 index, bdaddr a2dpBdAddr)
{
    A2DP_DATA.bd_addr[index] = a2dpBdAddr;
}

/*************************************************************************
NAME
    getA2dpLinkBdAddrForIndex

DESCRIPTION
    Returns the A2DP Remote Device bd address for the specified index.

RETURNS
    Remote device bd addr

**************************************************************************/
bdaddr getA2dpLinkBdAddrForIndex(uint16 index)
{
    return A2DP_DATA.bd_addr[index];
}


/*************************************************************************
NAME
    findCurrentA2dpSource

DESCRIPTION
    Attempts to obtain index of a connected A2dp Source that has established
    a media channel

RETURNS
    TRUE if found, FALSE otherwise

**************************************************************************/
bool findCurrentA2dpSource (a2dp_index_t* index)
{

    PEER_DEBUG(("findCurrentA2dpSource\n"));

    if (SinkA2dpIsInitialised())
    {
        uint16 i;
        for (i = 0; i<MAX_A2DP_CONNECTIONS; i++)
        {
            PEER_DEBUG(("... pri:%u\n", i));

            if (getA2dpStatusFlag(CONNECTED, i))
            {   /* Found a connected device */
                uint8 device_id = getA2dpLinkDataDeviceId(i);
                uint8 stream_id = getA2dpLinkDataStreamId(i);

                PEER_DEBUG(("...... dev:%u str:%u state:%u\n", device_id, stream_id, A2dpMediaGetState(device_id, stream_id)));

                switch ( A2dpMediaGetState(device_id, stream_id) )
                {
                case a2dp_stream_opening:
                case a2dp_stream_open:
                case a2dp_stream_starting:
                case a2dp_stream_streaming:
                case a2dp_stream_suspending:
                    PEER_DEBUG(("......... role:%u\n",A2dpMediaGetRole(device_id, stream_id)));
                    if ( A2dpMediaGetRole(device_id, stream_id)==a2dp_sink )
                    {   /* We have a sink endpoint active to the remote device, therefore it is a source */
                        PEER_DEBUG(("............ found sink\n"));

                        if (index != NULL)
                        {
                            *index = (a2dp_index_t)i;
                        }
                        return TRUE;
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }

    return FALSE;
}

/*************************************************************************
NAME
    findCurrentStreamingA2dpSource

DESCRIPTION
    Attempts to obtain index of a connected A2dp Source that has established
    a media channel and is streaming

RETURNS
    TRUE if found, FALSE otherwise

**************************************************************************/
bool findCurrentStreamingA2dpSource (a2dp_index_t* index)
{

    PEER_DEBUG(("findCurrentStreamingA2dpSource\n"));

    if (SinkA2dpIsInitialised())
    {
        uint16 i;
        for (i = 0; i<MAX_A2DP_CONNECTIONS; i++)
        {
            PEER_DEBUG(("... pri:%u\n", i));

            if (getA2dpStatusFlag(CONNECTED,i))
            {   /* Found a connected device */
                uint8 device_id = getA2dpLinkDataDeviceId(i);
                uint8 stream_id = getA2dpLinkDataStreamId(i);

                PEER_DEBUG(("...... dev:%u str:%u state:%u\n", device_id, stream_id, A2dpMediaGetState(device_id, stream_id)));

                switch ( A2dpMediaGetState(device_id, stream_id) )
                {
                case a2dp_stream_starting:
                case a2dp_stream_streaming:
                case a2dp_stream_suspending:
                    PEER_DEBUG(("......... role:%u\n",A2dpMediaGetRole(device_id, stream_id)));
                    if ( A2dpMediaGetRole(device_id, stream_id)==a2dp_sink )
                    {   /* We have a sink endpoint active to the remote device, therefore it is a source */
                        PEER_DEBUG(("............ found sink\n"));

                        if (index != NULL)
                        {
                            *index = (a2dp_index_t)i;
                        }
                        return TRUE;
                    }
                    break;

                default:
                    break;
                }
            }
        }
    }

    return FALSE;
}

/*************************************************************************
NAME
sinkA2dpGetOptionalCodecsEnabledFlag

DESCRIPTION
    Get the status of A2DP Optional Codecs enabled.

**************************************************************************/
uint8 sinkA2dpGetOptionalCodecsEnabledFlag(void)
{
    uint8 option_codec_enabled = 0;
    a2dp_config_def_t *a2dp_config_data = NULL;

    if(configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        option_codec_enabled = a2dp_config_data->A2dpOptionalCodecsEnabled;
        /* if aptX Low Latency is enabled, automatically enable standard aptX */
        if(option_codec_enabled & (1<<APTX_SPRINT_CODEC_BIT))
        {
            option_codec_enabled |= (1<<APTX_CODEC_BIT);
        }
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return option_codec_enabled;
}

/*************************************************************************
NAME
    sinkA2dpEnabled

DESCRIPTION
    Check the status of A2DP enable streaming feature bit.

RETURNS
    TRUE if A2DP streaming is enabled, FALSE otherwise

**************************************************************************/
bool sinkA2dpEnabled(void)
{
    a2dp_config_def_t *a2dp_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        enabled = a2dp_config_data->EnableA2dpStreaming;
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return enabled;
}

/*************************************************************************
NAME
    sinkA2dpGetDeviceClass

DESCRIPTION
    Get the A2DP Device Class.

RETURNS
    uint8

**************************************************************************/
uint8 sinkA2dpGetDeviceClass(void)
{
    a2dp_config_def_t *a2dp_config_data;
    uint8 a2dpclass = 0;

    if(configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        a2dpclass = a2dp_config_data->a2dpclass;
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return a2dpclass;
}

/*************************************************************************
NAME
    sinkA2dpOpenMediaOnConnection

DESCRIPTION
    Check the status of A2DP Media Open upon connection feature bit.

RETURNS
    TRUE if A2DP Media can open on connection, FALSE otherwise

**************************************************************************/
bool sinkA2dpOpenMediaOnConnection(void)
{
    a2dp_config_def_t *a2dp_config_data = NULL;
    bool enabled = FALSE;

    if(configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        enabled = a2dp_config_data->EnableA2dpMediaOpenOnConnection;
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return enabled;

}
/*************************************************************************
NAME
    getA2dpLinkLossReconnectTimeout

DESCRIPTION
    Returns the A2DP link Loss reconnection timeout.

RETURNS
    A2DP link loss reconnection timeout

**************************************************************************/
uint16 getA2dpLinkLossReconnectTimeout(void)
{
    a2dp_config_def_t *a2dp_config_data = NULL;
    uint16 timeout = 0;

    if(configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        timeout = a2dp_config_data->A2dpLinkLossReconnectionTime_s;
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return timeout;
}

bool setA2dpLinkLossReconnectTimeout(uint16 timeout)
{
    a2dp_config_def_t *a2dp_config_data = NULL;

    if (configManagerGetWriteableConfig(A2DP_CONFIG_BLK_ID, (void **)&a2dp_config_data, 0))
    {
        a2dp_config_data->A2dpLinkLossReconnectionTime_s = timeout;
        configManagerUpdateWriteableConfig(A2DP_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkA2dpGetDefaultVolumeInPercentage

DESCRIPTION
    Returns default A2DP volume level.

RETURNS
    Default A2DP volume level.

**************************************************************************/
uint8 sinkA2dpGetDefaultVolumeInPercentage(void)
{
    a2dp_config_def_t *config_data = NULL;
    uint8 music_volume_level = 0;

    if (configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&config_data))
    {
        music_volume_level = config_data->DefaultA2dpVolume;
        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }
    return music_volume_level;
}

/*************************************************************************
NAME
    sinkA2dpSetDefaultVolumeInPercentage

DESCRIPTION
    Updates default A2DP volume level.

RETURNS
    bool TRUE if volume was set correctly, FALSE otherwise.

**************************************************************************/
bool sinkA2dpSetDefaultVolumeInPercentage(uint8 volume)
{
    a2dp_config_def_t *config_data = NULL;

    if (configManagerGetWriteableConfig(A2DP_CONFIG_BLK_ID, (void **)&config_data, 0))
    {
        config_data->DefaultA2dpVolume = volume;
        configManagerUpdateWriteableConfig(A2DP_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*************************************************************************
NAME
    sinkA2dpGetDefaultVolumeInSteps

DESCRIPTION
    Returns default A2DP volume level in steps.

RETURNS
    int16 Default A2DP volume level in steps.

**************************************************************************/
int16 sinkA2dpGetDefaultVolumeInSteps(void)
{
    volume_group_config_t volume_config;

    sinkVolumeGetGroupConfig(audio_output_group_main, &volume_config);
    return GainUtilsConvertScaledVolumePercentageToStep(sinkA2dpGetDefaultVolumeInPercentage() * VOLUME_PERCENTAGE_SCALING_FACTOR,
                                                        &volume_config);
}

/*************************************************************************
NAME
    sinkA2dpGetDefaultAuxVolumeInSteps

DESCRIPTION
    Returns default A2DP aux volume level in steps.

RETURNS
    int16 Default A2DP aux volume level in steps.

**************************************************************************/
int16 sinkA2dpGetDefaultAuxVolumeInSteps(void)
{
    volume_group_config_t volume_config;

    sinkVolumeGetGroupConfig(audio_output_group_aux, &volume_config);
    return GainUtilsConvertScaledVolumePercentageToStep(sinkA2dpGetDefaultVolumeInPercentage() * VOLUME_PERCENTAGE_SCALING_FACTOR,
                                                        &volume_config);
}

/*************************************************************************
NAME
    getA2dpStatusFlag

DESCRIPTION
    Returns the status of indicated A2DP status flag for the specified index.

RETURNS
    Flag status

**************************************************************************/
bool getA2dpStatusFlag(a2dp_status_flag flag, uint16 index)
{
    bool result= FALSE;

    if (SinkA2dpIsInitialised())
    {
        switch(flag)
        {
            case REMOTE_AG_CONNECTION:
                result = A2DP_DATA.remote_ag_connection;
                break;

            case CONNECTED:
                result = A2DP_DATA.connected[index];
                break;

            case MEDIA_RECONNECT:
                result = A2DP_DATA.media_reconnect[index];
                break;

            case PLAYING:
                result = A2DP_DATA.playing[index];
                break;

            case QUAL_DISABLE_STREAM_RESUME:
                result = A2DP_DATA.qual_disable_stream_resume;
                break;
        }
    }

    return result;
}

/*************************************************************************
NAME
    setA2dpStatusFlag

DESCRIPTION
    Sets the status of indicated A2DP status flag for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpStatusFlag(a2dp_status_flag flag, uint16 index, bool value)
{
    switch(flag)
    {
        case REMOTE_AG_CONNECTION:
            /* Update the remote connection flag */
            A2DP_DATA.remote_ag_connection = value;
            break;

        case CONNECTED:
            A2DP_DATA.connected[index] = value;
            break;

        case MEDIA_RECONNECT:
            A2DP_DATA.media_reconnect[index] = value;
            break;

        case PLAYING:
            A2DP_DATA.playing[index] = value;
            break;

        case QUAL_DISABLE_STREAM_RESUME:
            A2DP_DATA.qual_disable_stream_resume = value;
            break;
   }
}

/*************************************************************************
NAME
    getA2dpPeerRemoteFeatures

DESCRIPTION
    Returns the remote A2DP peer supported features for the specified index.

RETURNS
    remote_features

**************************************************************************/
remote_features getA2dpPeerRemoteFeatures(uint16 index)
{
    return A2DP_DATA.peer_features[index];
}

/*************************************************************************
NAME
    setA2dpPeerRemoteFeatures

DESCRIPTION
    Updates the remote A2DP peer supported features for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPeerRemoteFeatures(uint16 index, remote_features peer_features)
{
    A2DP_DATA.peer_features[index] = peer_features;
}

/*************************************************************************
NAME
    getA2dpPeerRemoteDevice

DESCRIPTION
    Returns the remote A2DP peer device for the specified index.

RETURNS
    remote_features

**************************************************************************/
remote_device getA2dpPeerRemoteDevice(uint16 index)
{
    return A2DP_DATA.peer_device[index];
}

/*************************************************************************
NAME
    setA2dpPeerRemoteDevice

DESCRIPTION
    Updates the remote A2DP peer device for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPeerRemoteDevice(uint16 index, remote_device peer_device)
{
    A2DP_DATA.peer_device[index] = peer_device;
}

/*************************************************************************
NAME
    getA2dpPeerStatus

DESCRIPTION
    Returns the local or remote peer status for the specified index.

RETURNS
    peer status

**************************************************************************/
PeerStatus getA2dpPeerStatus(uint16 index, peer_type peer)
{
    PeerStatus peer_status = A2DP_DATA.local_peer_status[index];

    if(peer == LOCAL_PEER)
        peer_status = A2DP_DATA.local_peer_status[index];
    else if(peer == REMOTE_PEER)
        peer_status = A2DP_DATA.remote_peer_status[index];

    return peer_status;
}

/*************************************************************************
NAME
    setA2dpPeerStatus

DESCRIPTION
    Updates the local or remote peer status for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPeerStatus(uint16 index, peer_type peer, PeerStatus peer_status)
{
    if(peer == LOCAL_PEER)
        A2DP_DATA.local_peer_status[index] = peer_status;
    else if(peer == REMOTE_PEER)
        A2DP_DATA.remote_peer_status[index] = peer_status;
}

/*************************************************************************
NAME
    getA2dpLinkRole

DESCRIPTION
    Returns the A2DP link Role for the specified index.

RETURNS
    A2DP link role

**************************************************************************/
a2dp_link_role getA2dpLinkRole(uint16 index)
{
    return A2DP_DATA.link_role[index];
}

/*************************************************************************
NAME
    setA2dpLinkRole

DESCRIPTION
    Updates the A2DP link role for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpLinkRole(uint16 index, a2dp_link_role role)
{
    A2DP_DATA.link_role[index] = role;
}

a2dp_suspend_state SinkA2dpGetSuspendState(uint16 index)
{
    return A2DP_DATA.SuspendState[index];
}

static void setA2dpSuspendState(uint16 index, a2dp_suspend_state state)
{
    A2DP_DATA.SuspendState[index] = state;
}

/*************************************************************************
NAME
    getA2dpPdlListId

DESCRIPTION
    Returns the A2DP Remote Device list ID in PDL for the specified index.

RETURNS
    Remote device bd addr

**************************************************************************/
uint8 getA2dpPdlListId(uint16 index)
{
    return A2DP_DATA.list_id[index];
}

/*************************************************************************
NAME
    setA2dpPdlListId

DESCRIPTION
    Updates the remote device A2DP List ID in PDL for the specified index.

RETURNS
    None

**************************************************************************/
void setA2dpPdlListId(uint16 index, uint8 list_id)
{
    A2DP_DATA.list_id[index] = list_id;
}

/*************************************************************************
NAME
    getA2dpPeerOptionalCodecs

DESCRIPTION
    Returns the A2DP peer optional codecs

RETURNS
    peer optional codecs

**************************************************************************/
uint8 getA2dpPeerOptionalCodecs(peer_type peer)
{
    uint8 codecs = A2DP_DATA.local_peer_optional_codecs;

    if(peer == LOCAL_PEER)
        codecs = A2DP_DATA.local_peer_optional_codecs;
    else if(peer == REMOTE_PEER)
        codecs = A2DP_DATA.remote_peer_optional_codecs;

    return codecs;
}

/*************************************************************************
NAME
    setA2dpPeerOptionalCodecs

DESCRIPTION
    Updates the A2DP peer optional codecs

RETURNS
    None

**************************************************************************/
void setA2dpPeerOptionalCodecs(peer_type peer, uint8 codecs)
{

    if(peer == LOCAL_PEER)
        A2DP_DATA.local_peer_optional_codecs = codecs;
    else if(peer == REMOTE_PEER)
        A2DP_DATA.remote_peer_optional_codecs = codecs;
}

/*************************************************************************
NAME
    getA2dpReconfigServiceCaps

DESCRIPTION
    Returns the Remote Reconfig service capabilities used for PTS qualification

RETURNS
    Remote reconfig caps

**************************************************************************/
uint8* getA2dpReconfigServiceCaps(void)
{
     return A2DP_DATA.dest_service_caps;
}

/*************************************************************************
NAME
    setA2dpReconfigServiceCaps

DESCRIPTION
    Stores the Remote Reconfig service capabilities used for PTS qualification

RETURNS
    None

**************************************************************************/
void setA2dpReconfigServiceCaps(uint8 *service_caps)
{
     A2DP_DATA.dest_service_caps  = service_caps;
}

/*************************************************************************
NAME
    getAudioPluginConnectParams

DESCRIPTION
    Returns the A2DP audio plugin Connect parameters

RETURNS
    A2DP audio Plugin connect parameters

**************************************************************************/
A2dpPluginConnectParams* getAudioPluginConnectParams(void)
{
    return &A2DP_DATA.a2dp_audio_connect_params;
}

/*************************************************************************
NAME
    getAudioPluginModeParams

DESCRIPTION
    Returns the A2DP audio plugin Mode parameters

RETURNS
    A2DP audio Plugin Mode parameters

**************************************************************************/
A2dpPluginModeParams *getAudioPluginModeParams(void)
{
    return &A2DP_DATA.a2dp_audio_mode_params;
}

/*************************************************************************
NAME
    getMusicProcessingMode

DESCRIPTION
    Returns the Audio plugin Music Mode processing

RETURNS
    Audio Plugin Music mode processing

**************************************************************************/
A2DP_MUSIC_PROCESSING_T getMusicProcessingMode(void)
{
    return A2DP_DATA.a2dp_audio_mode_params.music_mode_processing;
}

/*************************************************************************
NAME
    setMusicProcessingMode

DESCRIPTION
    Sets the Audio plugin Music Mode processing

RETURNS
    None

**************************************************************************/
void setMusicProcessingMode(A2DP_MUSIC_PROCESSING_T music_processing)
{
    if(music_processing == A2DP_MUSIC_PROCESSING_FULL_NEXT_EQ_BANK)
    {
        peq_config_t config = peq_config_default;
        config = AudioMusicProcessingGetPeqConfig();
        if ((config == peq_config_6) || (config == peq_config_default))
        {
            config = peq_config_0;
        }
        else
        {
            config++;
        }

        music_processing = sinkMusicProcessingEqBankToProcessingMode(config);
    }
    A2DP_DATA.a2dp_audio_mode_params.music_mode_processing = music_processing;
}


/*************************************************************************
NAME
    getMusicProcessingBypassFlags

DESCRIPTION
    Returns the music processing bypass flags

RETURNS
    Music processing bypass flags

**************************************************************************/
uint16 getMusicProcessingBypassFlags(void)
{
    return A2DP_DATA.a2dp_audio_mode_params.music_mode_enhancements;
}

/*************************************************************************
NAME
    setMusicProcessingBypassFlags

DESCRIPTION
    Updates the music processing bypass flags

RETURNS
    None

**************************************************************************/
void setMusicProcessingBypassFlags(uint16 bypass_flags)
{
    A2DP_DATA.a2dp_audio_mode_params.music_mode_enhancements = bypass_flags;
}

/*************************************************************************
NAME
    getA2dpLinkDataPeerVersion

DESCRIPTION
    Returns the Peer version

RETURNS
   Peer version

**************************************************************************/
uint16 getA2dpLinkDataPeerVersion(uint16 index)
{
    return A2DP_DATA.peer_version[index];
}

/*************************************************************************
NAME
    setA2dpLinkDataPeerVersion

DESCRIPTION
    Updates Peer Version

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataPeerVersion(uint16 index,uint16 version)
{
    A2DP_DATA.peer_version[index] = version;
}

/*************************************************************************
NAME
    getA2dpLinkDataStreamId

DESCRIPTION
    Returns the Steam Id

RETURNS
    Stream ID

**************************************************************************/
uint16 getA2dpLinkDataStreamId(uint16 index)
{
    return A2DP_DATA.stream_id[index];
}

/*************************************************************************
NAME
    setA2dpLinkDataStreamId

DESCRIPTION
    Updates Stream ID

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataStreamId(uint16 index,uint16 streamind)
{
    A2DP_DATA.stream_id[index] = streamind;
}

/*************************************************************************
NAME
    getA2dpLinkDataDeviceId

DESCRIPTION
    Returns the device ID

RETURNS
    Device ID

**************************************************************************/
uint16 getA2dpLinkDataDeviceId(uint16 index)
{
    return A2DP_DATA.device_id[index];
}

/*************************************************************************
NAME
    setA2dpLinkDataDeviceId

DESCRIPTION
    Updates Device ID

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataDeviceId(uint16 index,uint16 deviceid)
{
     A2DP_DATA.device_id[index] = deviceid;
}

/*************************************************************************
NAME
    getA2dpLinkDataSeId

DESCRIPTION
    Returns the SEID

RETURNS
    SEID

**************************************************************************/
uint16 getA2dpLinkDataSeId(uint16 index)
{
     return A2DP_DATA.seid[index];
}

/*************************************************************************
NAME
    setA2dpLinkDataSeId

DESCRIPTION
    Updates Seid

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataSeId(uint16 index,uint16 seid)
{
    A2DP_DATA.seid[index] = seid;
}

/*************************************************************************
NAME
    getA2dpLinkDataLatency

DESCRIPTION
    Returns Data Latency

RETURNS
    Data latency

**************************************************************************/
uint16 getA2dpLinkDataLatency(uint16 index)
{
    return A2DP_DATA.latency[index];
}

/*************************************************************************
NAME
    setA2dpLinkDataLatency

DESCRIPTION
    Updates Data latency

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataLatency(uint16 index,uint16 latency)
{
    A2DP_DATA.latency[index] = latency;

}

/*************************************************************************
NAME
    getA2dpLinkDataClockMismatchRate

DESCRIPTION
    Returns Clock Mismatch rate

RETURNS
    Clock mismatch rate

**************************************************************************/
uint16 getA2dpLinkDataClockMismatchRate(uint16 index)
{
    return A2DP_DATA.clockMismatchRate[index];
}

/*************************************************************************
NAME
    setA2dpLinkDataClockMismatchRate

DESCRIPTION
    Updates Clock Mismatch Rate

RETURNS
    None

**************************************************************************/
void setA2dpLinkDataClockMismatchRate(uint16 index,uint16 clkrate)
{
    A2DP_DATA.clockMismatchRate[index] = clkrate;
}

/*************************************************************************
NAME
    getTWSRoutingMode

DESCRIPTION
    Returns the Peer TWS audio routing mode

RETURNS
    peer TWS audio routing mode

**************************************************************************/
PeerTwsAudioRouting getTWSRoutingMode(tws_device_type peer_mode)
{
    if(peer_mode == tws_master)
        return A2DP_DATA.a2dp_audio_mode_params.master_routing_mode;
    else
        return A2DP_DATA.a2dp_audio_mode_params.slave_routing_mode;
}

/*************************************************************************
NAME
    setTWSRoutingMode

DESCRIPTION
    Updates the Peer TWS audio routing mode

RETURNS
    None

**************************************************************************/
void setTWSRoutingMode(tws_device_type peer_mode, PeerTwsAudioRouting routing_mode)
{
    if(peer_mode == tws_master)
        A2DP_DATA.a2dp_audio_mode_params.master_routing_mode = routing_mode;
    else
        A2DP_DATA.a2dp_audio_mode_params.slave_routing_mode = routing_mode;
}

/*************************************************************************
NAME
    setExternalMic

DESCRIPTION
    Updates the External mic setting

RETURNS
    None

**************************************************************************/
void setExternalMic(uint8 external_mic)
{
    A2DP_DATA.a2dp_audio_mode_params.external_mic_settings = external_mic;
}

/*************************************************************************
NAME
    updateMicMuteState

DESCRIPTION
    Updates the External mic mute state for back channel enabled streaming (such as APTX-ll)

RETURNS
    None

**************************************************************************/
void updateMicMuteState(uint8 state)
{
    A2DP_DATA.a2dp_audio_mode_params.mic_mute = state;
}

/****************************************************************************
NAME
    sinkA2dpSetA2dpDataFromSessionData

DESCRIPTION
    Sets A2DP related data using stored session data values.

RETURNS
    None

**************************************************************************/
void sinkA2dpSetA2dpDataFromSessionData(void)
{
    a2dp_session_config_def_t *a2dp_config_data = NULL;

    if (configManagerGetReadOnlyConfig(A2DP_SESSION_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        setMusicProcessingMode(sinkMusicProcessingEqBankToProcessingMode(a2dp_config_data->selected_eq_bank));
        setMusicProcessingBypassFlags(a2dpMapMusicModeEnhancementsToA2dpData(a2dp_config_data));

        configManagerReleaseConfig(A2DP_SESSION_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
    sinkA2dpSetSessionDataFromA2dpData

DESCRIPTION
    Sets A2DP related session data using current data values.

RETURNS
    None

**************************************************************************/
void sinkA2dpSetSessionDataFromA2dpData(void)
{
    a2dp_session_config_def_t *a2dp_config_data = NULL;

    if (configManagerGetWriteableConfig(A2DP_SESSION_CONFIG_BLK_ID, (void **)&a2dp_config_data, 0))
    {
        a2dp_config_data->selected_eq_bank = sinkMusicProcessingEqProcessingModeToBankNum(getMusicProcessingMode());
        a2dpMapMusicModeEnhancementsToSessionData(getMusicProcessingBypassFlags(), a2dp_config_data);

        configManagerUpdateWriteableConfig(A2DP_SESSION_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
    a2dpMapMusicModeEnhancementsToA2dpData

DESCRIPTION
    Helper call to map all enabled music mode enhancements to run data.

RETURNS
    uint16 music_mode_enhancements

**************************************************************************/
static uint16 a2dpMapMusicModeEnhancementsToA2dpData(a2dp_session_config_def_t *a2dp_config_data)
{
    uint16 music_mode_enhancements = 0x0000;

    if (a2dp_config_data->dither_bypass) music_mode_enhancements |= MUSIC_CONFIG_DITHER_BYPASS;
    if (a2dp_config_data->compander_bypass) music_mode_enhancements |= MUSIC_CONFIG_COMPANDER_BYPASS;
    if (a2dp_config_data->spatial_enhance_bypass) music_mode_enhancements |= MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS;
    if (a2dp_config_data->bass_enhance_bypass) music_mode_enhancements |= MUSIC_CONFIG_BASS_ENHANCE_BYPASS;
    if (a2dp_config_data->user_eq_bypass) music_mode_enhancements |= MUSIC_CONFIG_USER_EQ_BYPASS;
    if (a2dp_config_data->eq_flat_enable) music_mode_enhancements |= MUSIC_CONFIG_EQFLAT;
    if (a2dp_config_data->speaker_eq_bypass) music_mode_enhancements |= MUSIC_CONFIG_SPKR_EQ_BYPASS;
    if (a2dp_config_data->subwoofer_bypass) music_mode_enhancements |= MUSIC_CONFIG_CROSSOVER_BYPASS;
    if (a2dp_config_data->audio_enhancement_disable) music_mode_enhancements |= MUSIC_CONFIG_DATA_VALID;

    return music_mode_enhancements;
}

/****************************************************************************
NAME
    a2dpMapMusicModeEnhancementsToSessionData

DESCRIPTION
    Helper call to map all enabled music mode enhancements to session data.

RETURNS
    None

**************************************************************************/
static void a2dpMapMusicModeEnhancementsToSessionData(uint16 music_mode_enhancements, a2dp_session_config_def_t *a2dp_config_data)
{
    a2dp_config_data->dither_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_DITHER_BYPASS);
    a2dp_config_data->compander_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_COMPANDER_BYPASS);
    a2dp_config_data->spatial_enhance_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS);
    a2dp_config_data->bass_enhance_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_BASS_ENHANCE_BYPASS);
    a2dp_config_data->user_eq_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_USER_EQ_BYPASS);
    a2dp_config_data->eq_flat_enable = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_EQFLAT);
    a2dp_config_data->speaker_eq_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_SPKR_EQ_BYPASS);
    a2dp_config_data->subwoofer_bypass = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_CROSSOVER_BYPASS);
    a2dp_config_data->audio_enhancement_disable = MusicEnhancementBypassed(music_mode_enhancements, MUSIC_CONFIG_DATA_VALID);
}

void sinkA2dpRenegotiateCodecsForAllSources(void)
{
    uint16 i;

    for_all_a2dp(i)
    {
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            sinkA2dpRenegotiateCodecsIfNotSupported(i);
        }
    }

}

bool sinkA2dpRenegotiateCodecsIfNotSupported(a2dp_index_t a2dp_source_index)
{
    if(sinkA2dpSourceHasUnsupportedCodec(a2dp_source_index))
    {
        A2DP_DATA.reconnected_ag_address = A2DP_DATA.bd_addr[a2dp_source_index];
        A2DP_DEBUG(("A2DP: setting address for reconnect lap 0x%x\n", (uint16)A2DP_DATA.reconnected_ag_address.lap));


        /* Kick off reconnection to this AV source to negotiate one of the enabled codecs */
        setA2dpStatusFlag(MEDIA_RECONNECT, a2dp_source_index, TRUE);
        A2dpMediaCloseRequest(getA2dpLinkDataDeviceId(a2dp_source_index), getA2dpLinkDataStreamId(a2dp_source_index));

        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

bool sinkA2dpSourceHasUnsupportedCodec(a2dp_index_t a2dp_source_index)
{
    uint8  seid = getA2dpLinkDataSeId(a2dp_source_index);
    a2dp_sep_status sep_status = A2dpCodecGetAvailable(getA2dpLinkDataDeviceId(a2dp_source_index), seid);

    A2DP_DEBUG(("A2DP: sinkA2dpSourceHasUnsupportedCodec sep_status 0x%x\n", sep_status));

    if((sep_status != A2DP_SEP_ERROR) && (sep_status & A2DP_SEP_UNAVAILABLE))
    {
        A2DP_DEBUG(("A2DP: sinkA2dpSourceHasUnsupportedCodec unsupported seid is 0x%x\n", seid));
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

void sinkA2dpRestrictSupportedCodecs(uint16 supported_codecs)
{
    uint16 index = 0;

    while(optional_codec_seids[index].codec_seid != INVALID_SEID)
    {
        if(!(supported_codecs & optional_codec_seids[index].codec_bit))
        {
            A2DP_DEBUG(("A2DP: disabling codec seid = 0x%x\n", optional_codec_seids[index].codec_seid));

            /* Disable the incompatible codecs */
            A2dpCodecSetAvailable(a2dp_primary, optional_codec_seids[index].codec_seid, FALSE);
            A2dpCodecSetAvailable(a2dp_secondary, optional_codec_seids[index].codec_seid, FALSE);
        }
        index++;
    }
}

void sinkA2dpEnableAllSupportedCodecs(void)
{
    uint16 index = 0;
    uint8 a2dp_optional_codecs=0x00;

    a2dp_optional_codecs = sinkA2dpGetOptionalCodecsEnabledFlag();

    while(optional_codec_seids[index].codec_seid != INVALID_SEID)
    {
        if((a2dp_optional_codecs & optional_codec_seids[index].codec_bit))
        {
            A2DP_DEBUG(("A2DP: enable optional codec seid = 0x%x\n", optional_codec_seids[index].codec_seid));

            /* Enable the optional codecs */
            A2dpCodecSetAvailable(a2dp_primary, optional_codec_seids[index].codec_seid, TRUE);
            A2dpCodecSetAvailable(a2dp_secondary, optional_codec_seids[index].codec_seid, TRUE);
        }
        index++;
    }
}


uint16 sinkA2dpGetCodecBitMaskFromSeid(uint16 seid)
{
    uint16 index = 0;

    while(optional_codec_seids[index].codec_seid != INVALID_SEID)
    {
        if(optional_codec_seids[index].codec_seid == seid)
        {
            return optional_codec_seids[index].codec_bit;
        }
        ++index;
    }

    return 0;
}

#ifdef ENABLE_AVRCP
/*************************************************************************
NAME
    getAgAvrcpSupportStatus

DESCRIPTION
    Returns the status of AVRCP support on AG

RETURNS
    avrcp support status

**************************************************************************/
avrcpSupport getAgAvrcpSupportStatus(uint16 index)
{
    return A2DP_DATA.avrcp_support[index];
}

/*************************************************************************
NAME
    setAgAvrcpSupportStatus

DESCRIPTION
    Updates the AVRCP support status of the AG

RETURNS
    None

**************************************************************************/
void setAgAvrcpSupportStatus(uint16 index, avrcpSupport support)
{
    A2DP_DATA.avrcp_support[index] = support;
}
#endif

#ifdef ENABLE_PEER
/*************************************************************************
NAME
    getA2dpPeerLinkLossReconnect

DESCRIPTION
    Returns the status of peer link loss reconnect flag.

RETURNS
    TRUE if peer link loss reconnect is enabled, FALSE otherwise

**************************************************************************/
bool getA2dpPeerLinkLossReconnect(void)
{
    return (bool) A2DP_DATA.peer_link_loss_reconnect;
}

/*************************************************************************
NAME
    getA2dpPeerLinkLossReconnectPtr

DESCRIPTION
    Returns a pointer to a uint16 holding the peer link loss reconnect
    flag, for use with MessageSendConditionally()

RETURNS
    Pointer to the status flag.

**************************************************************************/
uint16 *getA2dpPeerLinkLossReconnectPtr(void)
{
    return &A2DP_DATA.peer_link_loss_reconnect;
}

/*************************************************************************
NAME
    setA2dpPeerLinkLossReconnect

DESCRIPTION
    Sets the status of peer link loss reconnect flag.

RETURNS
    None

**************************************************************************/
void setA2dpPeerLinkLossReconnect(bool enable)
{
    A2DP_DATA.peer_link_loss_reconnect = enable;
}

/*************************************************************************
NAME
    getPeerDSPRequiredBufferingLevel

DESCRIPTION
    Returns the A2DP peer DSP required buffering level.

RETURNS
    peer buffer level

**************************************************************************/
peer_buffer_level getPeerDSPRequiredBufferingLevel(uint16 index)
{
    return A2DP_DATA.peer_dsp_required_buffering_level[index];
}

/*************************************************************************
NAME
    setPeerDSPRequiredBufferingLevel

DESCRIPTION
    Sets the A2DP peer DSP required buffering level.

RETURNS
    None

**************************************************************************/
void setPeerDSPRequiredBufferingLevel(uint16 index, peer_buffer_level buffer_level)
{
    A2DP_DATA.peer_dsp_required_buffering_level[index] = buffer_level;
}

#ifdef PEER_SCATTERNET_DEBUG   /* Scatternet debugging only */
/*************************************************************************
NAME
    sinkA2dpIsAgRoleToBeInverted

DESCRIPTION
    Returns the status of Ag Role invert flag.

RETURNS
    TRUE: If set, False otherwise

**************************************************************************/
bool sinkA2dpIsAgRoleToBeInverted(uint16 index)
{
    return A2DP_DATA.invert_ag_role[index];
}

/*************************************************************************
NAME
    sinkA2dpIsAgRoleToBeInverted

DESCRIPTION
    Returns the status of Ag Role invert flag.

RETURNS
    TRUE: If set, False otherwise

**************************************************************************/
void sinkA2dpInvertAgRole(uint16 index)
{
    A2DP_DATA.invert_ag_role[index] = !A2DP_DATA.invert_ag_role[index];
}
#endif

/*************************************************************************
NAME
    getCodecSettings

DESCRIPTION
    Attempts to obtain the locally supplied codec setting for the specified SEID

RETURNS
    Pointer to codec settings structure, if found. NULL otherwise

**************************************************************************/
static a2dp_codec_settings * getCodecSettings (uint8 seid)
{
    uint16 i;

    for (i=0; i<NUM_SEPS; i++)
    {
        if (codecList[i].config && (codecList[i].config->seid == seid))
        {
            a2dp_codec_settings * codec_settings = (a2dp_codec_settings *)PanicNull( malloc(sizeof(a2dp_codec_settings) + codecList[i].config->size_caps) );
            memset(codec_settings, 0, sizeof(a2dp_codec_settings) +  codecList[i].config->size_caps);  /* Zero the Codec settings */

            codec_settings->size_configured_codec_caps = codecList[i].config->size_caps;
            memmove(codec_settings->configured_codec_caps, codecList[i].config->caps, codecList[i].config->size_caps);

            codec_settings->seid = seid;

            return codec_settings;
        }
    }
    return NULL;
}

static bool isAptxAdCodecInUse(uint16 av_id)
{
    bool in_use = FALSE;
    a2dp_codec_settings * codec_settings = NULL;

    uint8 device_id = getA2dpLinkDataDeviceId(av_id);
    uint8 stream_id = getA2dpLinkDataStreamId(av_id);
    codec_settings = A2dpCodecGetSettings(device_id, stream_id);

    if(codec_settings != NULL)
    {
        if (codec_settings->seid == APTX_ADAPTIVE_SEID)
            in_use = TRUE;
        free(codec_settings);
    }

    return in_use;
}

static void reconnectAvSource(uint16 av_id)
{
    a2dp_index_t index;

    if (findCurrentStreamingA2dpSource(&index) && (index == av_id))
    {
        A2DP_DEBUG(("A2DP: reconnectAvSource because AG seid \n"));

        A2DP_DATA.reconnected_ag_address = A2DP_DATA.bd_addr[av_id];
    }
    else
    {
        BdaddrSetZero(getA2dpAgToReconnect());
    }

    /* Kick off reconnection to this AV source to negotiate one of the enabled codecs */
    setA2dpStatusFlag(MEDIA_RECONNECT, av_id, TRUE);
    A2dpMediaCloseRequest(getA2dpLinkDataDeviceId(av_id), getA2dpLinkDataStreamId(av_id));
}

/*************************************************************************
NAME
    disableIncompatibleOptionalCodecs

DESCRIPTION
    Determines the incompatible codecs between the peer devices and disables them.

RETURNS
    void

**************************************************************************/
static void disableIncompatibleOptionalCodecs(void)
{
    uint16 local_optional_codecs;
    uint16 remote_optional_codecs;
    uint16 compatible_optional_codecs;

    if(peerGetLocalSupportedCodecs(&local_optional_codecs) && peerGetRemoteSupportedCodecs(&remote_optional_codecs))
    {
        compatible_optional_codecs = local_optional_codecs & remote_optional_codecs & TWS_SUPPORTED_CODECS;

        A2DP_DEBUG(("A2DP: local_optional_codecs = 0x%x\n", local_optional_codecs));
        A2DP_DEBUG(("A2DP: remote_optional_codecs = 0x%x\n", remote_optional_codecs));
        A2DP_DEBUG(("A2DP: TWS_SUPPORTED_CODECS = 0x%x\n", TWS_SUPPORTED_CODECS));
        A2DP_DEBUG(("A2DP: compatible_optional_codecs = 0x%x\n", compatible_optional_codecs));

        if(compatible_optional_codecs != local_optional_codecs)
        {
            sinkA2dpRestrictSupportedCodecs(compatible_optional_codecs);
        }
    }
}

/*************************************************************************
NAME
    enableOptionalCodecs

DESCRIPTION
    Enables the support for optional codecs.

RETURNS

**************************************************************************/
static void enableOptionalCodecs(void)
{
    uint16 local_optional_codecs;
    uint16 index = 0;

    if(peerGetLocalSupportedCodecs(&local_optional_codecs))
    {
        while(optional_codec_seids[index].codec_seid != INVALID_SEID)
        {
            if(local_optional_codecs & optional_codec_seids[index].codec_bit)
            {
                A2DP_DEBUG(("A2DP: enableOptionalCodecs seid = 0x%x\n", optional_codec_seids[index].codec_seid));

                /* Enable the optional extra codec */
                A2dpCodecSetAvailable(a2dp_primary, optional_codec_seids[index].codec_seid, TRUE);
                A2dpCodecSetAvailable(a2dp_secondary, optional_codec_seids[index].codec_seid, TRUE);
            }
            index++;
        }
    }
}

static void updateServiceCapsSupportedWhenEnteringTwsModeIfNeededReconnectAvSource(void)
{
    uint16 source_index;
    bool aptx_ad_cap_changed = sinkA2dpAptxAdUpdateServiceCapWhenEnteringTwsMode();

    /* Disable the incompatible codecs between the peer devices */
    disableIncompatibleOptionalCodecs();

    if (a2dpGetSourceIndex(&source_index))
    {
        bool sep_in_use_not_supported = (aptx_ad_cap_changed && isAptxAdCodecInUse(source_index)) || sinkA2dpSourceHasUnsupportedCodec(source_index);

        if (sep_in_use_not_supported || PeerReconnectAgOnPeerConnectionEnabled())
            reconnectAvSource(source_index);
    }
}

static void restoreServiceCapsSupportedWhenExitingTwsModeIfNeededReconnectAvSource(void)
{
    uint16 source_index;
    bool aptx_ad_cap_changed = sinkA2dpAptxAdRestoreServiceCapWhenExitingTwsMode();

    /* Re-enable the optional codecs if disabled for this peer session */
    enableOptionalCodecs();

    if (a2dpGetSourceIndex(&source_index))
    {
        /* TWS limits the modes and bitrate aptX Adaptive can use, so we want to inform the source it can now remove these limitations */
        if (aptx_ad_cap_changed && isAptxAdCodecInUse(source_index))
            reconnectAvSource(source_index);
    }
}

/*************************************************************************
NAME
    a2dpGetPeerIndex

DESCRIPTION
    Attempts to obtain the index into A2DP_DATA structure for a currently
    connected Peer device.

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
bool a2dpGetPeerIndex (uint16* index)
{
    uint8 i;

    /* go through A2dp connection looking for device_id match */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected check its device id */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            /* if a device_id match is found return its value and a
               status of successful match found */
            if(getA2dpPeerRemoteDevice(i) == remote_device_peer)
            {
                if (index)
                {
                    *index = i;
                }
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */
    return FALSE;
}

/*************************************************************************
NAME
    a2dpGetSourceIndex

    Attempts to obtain the index into A2DP_DATA structure for a currently
    connected A2dp Source device.

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
bool a2dpGetSourceIndex (uint16* index)
{
    uint8 i;

    /* go through A2dp connection looking for device_id match */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected check its device id */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            /* if a device_id match is found return its value and a
               status of successful match found */
            if(getA2dpPeerRemoteDevice(i) == remote_device_nonpeer)
            {
                if (index)
                {
                    *index = i;
                }
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */
    return FALSE;
}

/*************************************************************************
NAME
    a2dpGetSourceSink

DESCRIPTION
    Attempts to obtain the media sink for a currently connected A2dp Source.

RETURNS
    Handle to media sink if present, NULL otherwise

**************************************************************************/
Sink a2dpGetSourceSink (void)
{
    uint16 av_id;

    if (a2dpGetSourceIndex(&av_id))
    {
        return A2dpMediaGetSink(getA2dpLinkDataDeviceId(av_id), getA2dpLinkDataStreamId(av_id));
    }

    return (Sink)NULL;
}

/*************************************************************************
NAME
    modifyCodecSettings

DESCRIPTION
    Modifies the supplied general codec capabilities to provide specific
    capabilities for the rate specified.

RETURNS
    None

**************************************************************************/
static void modifyCodecSettings (a2dp_codec_settings * codec_settings, uint32 rate)
{
    uint8 * codec_caps = NULL;
    uint16 codec_caps_size;

    /* There is no codec configured, nothing to modify, return */
    if(codec_settings == NULL)
    {
        return;
    }

    codec_caps = codec_settings->configured_codec_caps;
    codec_caps_size = codec_settings->size_configured_codec_caps;

    A2DP_DEBUG(("A2DP: modifyCodecSettings   codec_caps=0x%p   size=%u   rate=%lu\n",codec_caps,codec_caps_size,rate));

    /* Scan through codec caps for the Media Codec service category */
    while (codec_caps_size && codec_caps && (codec_caps[0] != AVDTP_SERVICE_MEDIA_CODEC))
    {
        uint16 category_size = codec_caps[1] + 2;

        codec_caps += category_size;
    }

    if (codec_caps_size && codec_caps && (codec_caps[0] == AVDTP_SERVICE_MEDIA_CODEC))
    {   /* Media Codec service category located */
        const uint8 *new_codec_caps = NULL;

        switch (rate)
        {
        case 16000:
            new_codec_caps = sbc_caps_16k;
            break;
        case 32000:
            new_codec_caps = sbc_caps_32k;
            break;
        case 44100:
            new_codec_caps = sbc_caps_44k1;
            break;
        case 48000:
            new_codec_caps = sbc_caps_48k;
            break;
        }

        A2DP_DEBUG(("A2DP: Found AVDTP_SERVICE_MEDIA_CODEC, attempting to modify caps...\n"));
        A2DP_DEBUG(("A2DP: new_caps=0x%p   codec_caps[1]=%u  codec_caps[3]=%u   new_caps[1]=%u  new_caps[3]=%u\n", new_codec_caps, codec_caps[1], codec_caps[3], new_codec_caps[1], new_codec_caps[3]));
        if (new_codec_caps && (codec_caps[1] == new_codec_caps[1]) && (codec_caps[3] == new_codec_caps[3]))
        {   
            /* Check category size and seid of new caps match current caps, before updating */
            memcpy(codec_caps, new_codec_caps, new_codec_caps[1] + 2);
            
#ifdef SBC_ENCODER_CONFORMANCE
            /* Replace the source caps with the user configured caps */
            codec_caps[4]  = SinkSbcGetSamplingFreq() | SinkSbcGetChannelMode();
            codec_caps[5]  = SinkSbcGetBlockLength() | SinkSbcGetSubbands() | SinkSbcGetAllocationMethod();
            codec_caps[6]  = SinkSbcGetMinBitpool();
            codec_caps[7]  = SinkSbcGetMaxBitpool();
#endif

        }
    }
}

/*************************************************************************
NAME
    handleA2dpCodecConfigureIndFromPeer

DESCRIPTION
    All Peer specific SEPs are configured to ask the app to provide the appropriate parameters
    when attempting to issue an AVDTP_SET_CONFIGURATION_CMD for the relay stream.
    This function obtains the codec settings for the current source (Wired, USB or A2DP) and uses
    these to set the configuration for the relay stream.

RETURNS
    None

**************************************************************************/
static void handleA2dpCodecConfigureIndFromPeer (A2DP_CODEC_CONFIGURE_IND_T* ind)
{
    a2dp_index_t index;
    a2dp_codec_settings* codec_settings = NULL;
    RelaySource current_source = peerCurrentRelaySource();
    uint8* conf_caps = NULL;
    uint16 size_conf_caps = 0;
    uint8 *peer_caps = ind->codec_service_caps;
    uint16 size_peer_caps = ind->size_codec_service_caps;

    PEER_DEBUG(("A2DP: handleA2dpCodecConfigureIndFromPeer dev:%u seid:0x%X\n", ind->device_id, ind->local_seid));

    switch (current_source)
    {
        case RELAY_SOURCE_NULL:
            PEER_DEBUG(("... Streaming Av Src not found\n"));
            break;

        case RELAY_SOURCE_ANALOGUE:
        {
            uint32 samp_freq;

            codec_settings = getCodecSettings( SBC_SEID );
            analogGetAudioRate(&samp_freq);
            modifyCodecSettings(codec_settings, samp_freq);
            break;
        }
        case RELAY_SOURCE_USB:
        {
#ifdef ENABLE_USB
            uint32 samp_freq;

            codec_settings = getCodecSettings( SBC_SEID );
            UsbDeviceClassGetValue(USB_DEVICE_CLASS_GET_VALUE_SPEAKER_SAMPLE_FREQ, &samp_freq);
            modifyCodecSettings(codec_settings, samp_freq);
#endif
            break;
        }
        case RELAY_SOURCE_A2DP:
            if (findCurrentA2dpSource( &index ))
            {
                uint8 device_id = getA2dpLinkDataDeviceId(index);
                uint8 stream_id = getA2dpLinkDataStreamId(index);
                codec_settings = A2dpCodecGetSettings(device_id, stream_id);
            }
            break;
    }

    if (codec_settings)
    {
        if ( !(codec_settings->seid & (SOURCE_SEID_MASK | TWS_SEID_MASK)) &&    /* Double check AV Source is using a standard Sink SEP */
              ((ind->local_seid & SOURCE_SEID_MASK) == SOURCE_SEID_MASK) &&     /* Double check Relay stream is using Source SEP */
              (codec_settings->seid == (ind->local_seid & BASE_SEID_MASK)) )    /* Same base codec id being used by AV Source and Relay stream */
        {
            conf_caps = codec_settings->configured_codec_caps;
            size_conf_caps = codec_settings->size_configured_codec_caps;
            /* Source and sink seids use matching codec */
            /* Request same codec configuration settings as AV Source for peer device */
            PEER_DEBUG(("... Configuring codec dev:%u local codec_caps_size=%u seid=0x%X\n",ind->device_id, size_peer_caps, ind->local_seid));
            PEER_DEBUG(("remote codec_caps_size=%u seid=0x%X\n", size_conf_caps, codec_settings->seid));

            if (ind->local_seid & TWS_SEID_MASK)
            {   /* TWS Source SEP - place Sink codec caps into TWS codec caps */
                uint16 size_to_copy = size_conf_caps - MEDIA_TRANSPORT_SERVICE_CATEGORY_OFFSET; /* copy after media transport service category */
                uint8 *new_peer_caps = NULL;
                uint16 new_size_peer_caps = 0;

                if (size_to_copy > (size_peer_caps - CONFIGURED_MEDIA_CODEC_OFFSET))
                {
                    /* Looks like we need to more memory for peer caps as AV caps size is more than peer caps */
                    new_size_peer_caps = size_to_copy + CONFIGURED_MEDIA_CODEC_OFFSET;
                    new_peer_caps = PanicUnlessMalloc(sizeof(uint8) * new_size_peer_caps);
                    memset(new_peer_caps, 0, new_size_peer_caps);
                    memmove(new_peer_caps, peer_caps, size_peer_caps);
                    /* re-set the peer caps pointer */
                    peer_caps = new_peer_caps;
                    size_peer_caps = new_size_peer_caps;
                    PEER_DEBUG(("New peer codec_caps_size=%u\n", size_peer_caps));
                }
                /* TODO: Copy each service capability separately */
                memcpy(peer_caps + CONFIGURED_MEDIA_CODEC_OFFSET, conf_caps + MEDIA_TRANSPORT_SERVICE_CATEGORY_OFFSET, size_to_copy);
                size_to_copy = size_peer_caps;
                A2dpCodecConfigureResponse(ind->device_id, TRUE, ind->local_seid, size_to_copy, peer_caps);
                /* done with sending caps */
                if(new_peer_caps)
                {
                    free(new_peer_caps);
                    new_peer_caps = NULL;
                }
            }
            else
            {
                /* In some TC, PTS expects SRC to open the media channel, then ADK tries to configure audio SRC's caps.
                    But PTS does not expect delay reporting capabilities to be set. Since PTS does not support delay reporting,
                    we need to remove that before configuring PTS (if at all the audio SRC's caps support delay reporting).
                    This wrapper function has been written to take care of the situation */
                if(peerQualificationReplaceDelayReportServiceCaps(peer_caps, &size_peer_caps, conf_caps, size_conf_caps))
                {
                    A2dpCodecConfigureResponse(ind->device_id, TRUE, ind->local_seid, size_peer_caps, peer_caps);
                }
                else
                {
                    /* ShareMe Source SEP - use Sink codec caps for ShareMe codec */
                    A2dpCodecConfigureResponse(ind->device_id, TRUE, ind->local_seid, size_conf_caps, conf_caps);
                }
            }
        }
        else
        {   /* Source and sink seids do not use matching codec */
            PEER_DEBUG(("... Non matching codecs dev:%u seid:0x%X\n",ind->device_id, ind->local_seid));
            A2dpCodecConfigureResponse(ind->device_id, FALSE, ind->local_seid, 0, NULL);
        }

        free(codec_settings);
    }
    else
    {   /* Reject as we don't have an active source */
        A2dpCodecConfigureResponse(ind->device_id, FALSE, ind->local_seid, 0, NULL);
    }
}


/*************************************************************************
NAME
    openPeerStream

DESCRIPTION
    Requests to open a media channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
static bool openPeerStream (uint16 Id, uint8 base_seid)
{
    PEER_DEBUG(("openPeerStream Dev=%u base_seid=%02X",Id, base_seid));

    base_seid &= BASE_SEID_MASK;

    /* Don't open AAC based stream to a peer device supporting ShareMe only */
    if (!(getA2dpPeerRemoteFeatures(Id) & remote_features_tws_a2dp_sink) && (base_seid == AAC_SEID))
    {
        PEER_DEBUG(("  unsuitable\n"));
        return FALSE;
    }

    if ((base_seid) && sinkA2dpIsStreamingAllowed())
    {
        uint8 seid_list[3];
        uint8 seid_list_size = 0;

        PEER_DEBUG(("  seid_list=["));
#ifdef PEER_TWS
        seid_list[seid_list_size++] = base_seid | (SOURCE_SEID_MASK | TWS_SEID_MASK);
        PEER_DEBUG(("%02X, ", seid_list[seid_list_size-1]));
#endif
#ifdef PEER_AS
        seid_list[seid_list_size++] = base_seid | SOURCE_SEID_MASK;
        PEER_DEBUG(("%02X, ", seid_list[seid_list_size-1]));
#endif
        seid_list[seid_list_size++] = SBC_SEID | SOURCE_SEID_MASK;      /* Always request a standard SBC source SEP, to support standard sink devices */
        PEER_DEBUG(("%02X]  size=%u\n", seid_list[seid_list_size-1], seid_list_size));

        /*return A2dpMediaOpenRequestEx(A2DP_DATA.device_id[Id], seid_list_size, seid_list, sizeof(a2dp_media_conftab), a2dp_media_conftab);*/
        return A2dpMediaOpenRequest(getA2dpLinkDataDeviceId(Id), seid_list_size, seid_list);
    }

    PEER_DEBUG(("  unsuitable\n"));
    return FALSE;
}

/*************************************************************************
NAME
    a2dpIssuePeerOpenRequest

DESCRIPTION
    Issues a request to opens a media stream to a currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerOpenRequest (void)
{
    uint16 peer_id;

    if ((a2dpGetPeerIndex(&peer_id)) && sinkA2dpIsStreamingAllowed())
    {
        uint8 seid = INVALID_SEID;
        uint16 av_id;
        RelaySource current_source = peerCurrentRelaySource();

        peerClaimRelay(TRUE);

        PEER_DEBUG(("issuePeerOpenRequest peer=%u  av=%u\n",peer_id,current_source));

        switch (current_source)
        {
        case RELAY_SOURCE_NULL:
            break;
        case RELAY_SOURCE_ANALOGUE:
        case RELAY_SOURCE_USB:
            seid = SBC_SEID;
            break;
        case RELAY_SOURCE_A2DP:
            if (a2dpGetSourceIndex(&av_id))
            {
                seid = getA2dpLinkDataSeId(av_id);
            }
            break;
        }

        return openPeerStream(peer_id, seid);
    }

    return FALSE;
}


/*************************************************************************
NAME
    a2dpIssuePeerCloseRequest

DESCRIPTION
    Issues a request to close the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerCloseRequest (void)
{
    uint16 peer_id;

    if (a2dpGetPeerIndex(&peer_id))
    {
        PEER_DEBUG(("issuePeerCloseRequest peer=%u\n",peer_id));

        peerClaimRelay(FALSE);

        /* Ensure suspend state is set as streaming has now ceased */
        a2dpSetSuspendState(peer_id, a2dp_local_suspended);
        avrcpUpdatePeerPlayStatus(avrcp_play_status_stopped);

        return A2dpMediaCloseRequest(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id));
    }

    return FALSE;
}

/*************************************************************************
NAME
    a2dpIssuePeerStartRequest

DESCRIPTION
    Issues a request to start the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerStartRequest (void)
{
    uint16 peer_id;

    if (a2dpGetPeerIndex(&peer_id) && sinkA2dpIsStreamingAllowed())
    {
        PEER_DEBUG(("   issuing start request...\n"));

        avrcpUpdatePeerPlayStatus(avrcp_play_status_playing);

        return A2dpMediaStartRequest(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id));
    }

    return FALSE;
}


/*************************************************************************
NAME
    a2dpIssuePeerSuspendRequest

DESCRIPTIONDESCRIPTION
    Issues a request to suspend the relay channel to the currently connected Peer

RETURNS
    TRUE if request issued, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerSuspendRequest (void)
{
    uint16 peer_id;

    if (a2dpGetPeerIndex(&peer_id))
    {
        PEER_DEBUG(("issuePeerSuspendRequest peer=%u\n",peer_id));

        /* Ensure suspend state is set as streaming has now ceased */
        a2dpSetSuspendState(peer_id, a2dp_local_suspended);
        avrcpUpdatePeerPlayStatus(avrcp_play_status_paused);

        return A2dpMediaSuspendRequest(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id));
    }

    return FALSE;
}


/*************************************************************************
NAME
    a2dpIssuePeerStartResponse

DESCRIPTION
    Issues a start response to a Peer based on availability of the relay channel

RETURNS
    TRUE if response sent, FALSE otherwise

**************************************************************************/
bool a2dpIssuePeerStartResponse (void)
{
    uint16 peer_id;

    if (a2dpGetPeerIndex(&peer_id))
    {
        PEER_DEBUG(("issuePeerStartResponse peer=%u\n",peer_id));

        if ( peerIsRelayAvailable() )
        {   /* Accept start request */
            return A2dpMediaStartResponse(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id), TRUE);
        }
        else
        {   /* Reject start request as not in a state to receive audio */
            return A2dpMediaStartResponse(getA2dpLinkDataDeviceId(peer_id), getA2dpLinkDataStreamId(peer_id), FALSE);
        }
    }

    return FALSE;
}

#endif

/*************************************************************************
NAME
    a2dpSetPlayingState

DESCRIPTION
    Logs the current AVRCP play status for the specified A2DP connection and
    updates the Suspend State for the Media channel so that it reflects the true
    media playing status.

    The Suspend State defines the overall state (suspended or not suspended) of a media stream.
    It is determined by using a combination of the AVDTP stream state and the AVRCP play status.
    Ultimately, the AVDTP stream state drives the suspend state far more strongly than the AVRCP
    play status, as the AVDTP stream state tells us if data is actually being transmitted over
    an A2DP media channel.
    Certainly, when the AVDTP stream state moves from a suspended to a streaming state, then we
    take that as a definitive indication that streaming has started and the A2DP media stream is
    not in a suspended state.  In this instance AVRCP play status often lags behind the AVDTP
    stream state and we can receive actual indications from a Source that the play status is
    still stopped/paused even though the AVDTP stream state is set to streaming.  This is
    probably down to latency between layers of the OS and applications on the Source.
    When an AG stops streaming then behaviour is a little different.  Here AVRCP play status will
    often lead AVDTP stream state by several seconds.  This is the one instance we allow the AVRCP
    play status to drive the Suspend State.  This is to allow an A2DP source to be marked as
    suspended and allow far faster automatic changes to another Source (analogue/USB) that may be
    streaming.

RETURNS
    None

**************************************************************************/
void a2dpSetPlayingState (uint16 id, bool playing)
{
    A2DP_DEBUG(("A2DP: a2dpSetPlayingState id %u playing %u\n", id, playing));

    if (SinkA2dpIsInitialised() && getA2dpStatusFlag(PLAYING, id) != playing)
    {
        setA2dpStatusFlag(PLAYING, id, playing);

        if (!playing)
        {   /* Play state has changed from playing to not playing.  This change in AVRCP play status is likely to lead a change */
            /* to the AVDTP stream state by several seconds.  Mark the stream as suspended so we can allow automatic changes to */
            /* the relayed source far sooner than waiting for the AVDTP stream state to change.                                 */
            a2dpSetSuspendState(id, a2dp_remote_suspended);

#ifdef ENABLE_PEER
            if(!a2dpIsIndexPeer(id))
            {
                peerClaimRelay(FALSE);
            }
#endif

#ifdef ENABLE_PEER
            linklossProtectStreaming(linkloss_stream_protection_off);

            /* Re-enable stream protection on the link it is required on. */
            if (!peerLinkRecoveryWhileStreamingEnabled())
            {
                linklossProtectStreaming(linkloss_stream_protection_on);
            }
#endif
        }
        else
        {   /* Play state has changed from not playing to playing.  In this instance we drive the suspend state directly from   */
            /* the AVDTP stream state                                                                                           */
            a2dp_stream_state a2dp_state = A2dpMediaGetState(getA2dpLinkDataDeviceId(id), getA2dpLinkDataStreamId(id));

            if ((a2dp_state == a2dp_stream_starting) || (a2dp_state == a2dp_stream_streaming))
            {   /* A2DP media stream is streaming (or heading towards it) */
                a2dpSetSuspendState(id, a2dp_not_suspended);

#ifdef ENABLE_PEER
                if(!a2dpIsIndexPeer(id))
                {
                    peerClaimRelay(TRUE);
                }
#endif

                /*Ensure that the device is not currently streaming from a different A2DP, if its found to be streaming then pause this incoming stream  */
#ifdef ENABLE_AVRCP
                a2dpPauseNonRoutedSource(id);
#endif
            }
            else
            {
                a2dpSetSuspendState(id, a2dp_remote_suspended);

#ifdef ENABLE_PEER
                if(!a2dpIsIndexPeer(id))
                {
                    peerClaimRelay(FALSE);
                }
#endif
            }
        }
    }
}

#ifdef ENABLE_PEER

/*************************************************************************
NAME
    sinkA2dpHandlePeerAvrcpConnectCfm

DESCRIPTION
    Configure initial relay availability when a Peer connects

RETURNS
    None

**************************************************************************/
void sinkA2dpHandlePeerAvrcpConnectCfm (uint16 peer_id, bool successful)
{
    if (successful)
    {
        if (!PeerDetermineTwsForcedDownmixMode())
        {
        /*  Force Downmix feature not available, get routing from attributes  */
            bdaddr local_bd_address;
            sinkDataGetLocalBdAddress(&local_bd_address);
    
            /* Exchange the right/left channel information */
            if((peerCompareBdAddr(&local_bd_address, getA2dpLinkBdAddr(peer_id)))
             || peerDoesRemotePeerSupportAudioRoutingConfiguration() == FALSE)
            {
                sink_attributes attributes;
    
                /* Get the peer's attributes and use the stored TWS audio routing preference*/
                deviceManagerGetAttributes(&attributes, getA2dpLinkBdAddr(peer_id));
                /* Relay that information to the peer */
                peerSetPeerAudioRouting(attributes.master_routing_mode, attributes.slave_routing_mode);
            }
        }

        /* Set initial local status now peers have connected */
        if(sinkAudioIsAudioRouted())
        {
            if ((sinkCallManagerGetHfpSink(hfp_primary_link) && (sinkCallManagerGetHfpCallState(hfp_primary_link) > hfp_call_state_idle))
                || (sinkCallManagerGetHfpSink(hfp_secondary_link) && (sinkCallManagerGetHfpCallState(hfp_secondary_link) > hfp_call_state_idle)))
            {   /* Call is active, so set flag to indicate that this device does not want the relay channel to be used */
                 peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_ACTIVE | PEER_STATUS_CHANGE_RELAY_AVAILABLE | PEER_STATUS_CHANGE_RELAY_FREED);
            }
            else
            {   /* No call active, set relay channel as free for use */
                if(LR_CURRENT_ROLE_MASTER == getA2dpLinkRole(peer_id))
                    peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_INACTIVE | PEER_STATUS_CHANGE_RELAY_AVAILABLE | PEER_STATUS_CHANGE_RELAY_CLAIMED );
                else
                    peerUpdateLocalStatusChangeNoKick(PEER_STATUS_CHANGE_CALL_INACTIVE | PEER_STATUS_CHANGE_RELAY_AVAILABLE );
            }
        }
        else
        {   /* No audio routed, thus relay channel is completely free for use */
             peerUpdateLocalStatusChange(PEER_STATUS_CHANGE_CALL_INACTIVE | PEER_STATUS_CHANGE_RELAY_AVAILABLE | PEER_STATUS_CHANGE_RELAY_FREED);
        }
    }
    /* now that a peer avrcp link is available, update the connected peer status */
    MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
}

#endif  /* ENABLE_PEER */

/*************************************************************************
NAME
    a2dpSetSuspendState

DESCRIPTION
    Sets the suspend state for the specified device

RETURNS
    None

**************************************************************************/
void a2dpSetSuspendState (uint16 id, a2dp_suspend_state state)
{
    if (state == a2dp_not_suspended)
    {   /* Returning to the unsuspended state */
        A2DP_DATA.paused_using_avrcp[id] = FALSE;
        setA2dpSuspendState(id, state);
    }
    else
    {   /* Only update the suspend state of the device only if it was not set to remote or locally suspended  previously */
        if (SinkA2dpGetSuspendState(id) == a2dp_not_suspended)
        {
            setA2dpSuspendState(id, state);
        }
    }
    A2DP_DEBUG(("A2dp: SuspendState[%u] = %d\n",id, SinkA2dpGetSuspendState(id)));
}

#ifdef ENABLE_AVRCP
/*************************************************************************
NAME
    a2dpPauseNonRoutedSource

DESCRIPTION
    Check whether the a2dp connection is present and streaming data and that the audio is routed,
    if thats true then pause/stop the incoming stream corresponding to the input deviceId.

RETURNS
    None

**************************************************************************/
void a2dpPauseNonRoutedSource(uint16 id)
{
/* Only certain TWS use cases require us to attempt to pause source devices */
#ifdef ENABLE_PEER
    uint16 avrcpIndex;
    uint16 peerIndex;


    if((SinkA2dpGetSuspendState(id) == a2dp_not_suspended)
       && (a2dpGetPeerIndex(&peerIndex))
       && (sinkAvrcpGetIndexFromBdaddr(getA2dpLinkBdAddr(id), &avrcpIndex, TRUE))
      )
    {
        /* Only pause source if we have a peer connected and is required for a TWS use case */
        if((getA2dpPeerRemoteDevice(id) == remote_device_nonpeer) &&
           (peerIsSingleDeviceOperationEnabled())  &&
           (getA2dpPeerStatus(peerIndex, REMOTE_PEER) & PEER_STATUS_IN_CALL)
          )
        {
            A2DP_DEBUG(("A2dp: SuspendState - Stop playing Id = %d \n",id));
            a2dpAvrcpStop(avrcpIndex);
            a2dpSetSuspendState(id , a2dp_local_suspended);
        }
        /* Only pause if we are the slave device and both master and slave are attempting to play*/
        if ((peerIndex != id)
            && (getA2dpLinkRole(peerIndex) == LR_CURRENT_ROLE_SLAVE)
            && (getA2dpStatusFlag(PLAYING, peerIndex))
            && (getA2dpStatusFlag(PLAYING, id))
           )
        {
            A2DP_DEBUG(("A2dp: stopping avrcpIndex=%u\n", avrcpIndex));
            a2dpAvrcpStop(avrcpIndex);
        }
    }
#else
    UNUSED(id);
#endif /* ENABLE_PEER */
}


/*************************************************************************
NAME
    a2dpAvrcpStop

DESCRIPTION
    This function requests for playback to stop via avrcp,

RETURNS
    None
**************************************************************************/
void a2dpAvrcpStop(const uint16 avrcpIndex)
            {
#ifdef ENABLE_PEER
                /* cancel any queued ff or rw requests and then stop the streaming*/
                MessageCancelAll (&theSink.task, EventUsrAvrcpFastForwardPress);
                MessageCancelAll (&theSink.task, EventUsrAvrcpRewindPress);

                sinkAvrcpStopRequest(avrcpIndex);
#else
    UNUSED(avrcpIndex);
#endif /* ENABLE_PEER */
}
#endif

/*************************************************************************
NAME
    InitA2dp

DESCRIPTION
    This function initialises the A2DP library and supported codecs

RETURNS
    A2DP_INIT_CFM message returned, handled by A2DP message handler
**************************************************************************/
void InitA2dp(void)
{
    uint16 i;
    sep_data_type seps[NUM_SEPS]= {{0}};
    uint8               number_of_seps = 0;
    uint8 a2dp_optional_codecs = 0;
    a2dp_config_def_t *a2dp_config_data;

    A2DP_DEBUG(("INIT: A2DP\n"));
    A2DP_DEBUG(("INIT: NUM_SEPS=%u\n",NUM_SEPS));

    /* initialise structure to 0 */
    memset(&A2DP_DATA, 0, sizeof(a2dp_data));

    /* initialise device and stream id's to invalid as 0 is a valid value */
    setA2dpLinkDataDeviceId(0,INVALID_DEVICE_ID);
    setA2dpLinkDataStreamId(0, INVALID_STREAM_ID);
    setA2dpLinkDataDeviceId(1, INVALID_DEVICE_ID);
    setA2dpLinkDataStreamId(1, INVALID_STREAM_ID);

    /* Initialize the plugin audio connect  parameters */
    setupAudioConnectParams();
    setupAudioModeParams();

    sinkA2dpAptxAdInitServiceCapability();

    if (sinkA2dpGetSupportedOptionalcodecs(&a2dp_optional_codecs)
        && configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **)&a2dp_config_data))
    {
        /* only continue and initialise the A2DP library if it's actually required,
           library functions will return false if it is uninitialised */
        if(a2dp_config_data->EnableA2dpStreaming)
        {
            uint8 codec_include_mask = getCodecIncludeMask();

            /* Only register codecs that are both included due to the correct application feature set and enabled via config */
            for (i=0; i<NUM_SEPS; i++)
            {
                if (codecList[i].config && (codecList[i].include_mask & codec_include_mask))
                {
                    if (codecList[i].enable_bit==SBC_CODEC_BIT || (a2dp_optional_codecs & (1<<codecList[i].enable_bit)))
                    {
                            seps[number_of_seps].sep_config = codecList[i].config;
                            seps[number_of_seps].in_use = FALSE;
                            
                            if ((codecList[i].config->seid == SBC_SEID) && (a2dp_config_data->sbc_uses_medium_bitpool))
                            {
                                seps[number_of_seps].sep_config = &sbc_med_sep_snk;
                            }
                            
                            number_of_seps++;
                            A2DP_DEBUG(("INIT: Codec included (seid=0x%X)\n",codecList[i].config->seid));                            
                    }
                }
            }

            /* No active connections yet so default link loss protection to off. */
            sinkDataSetStreamProtectionState(0);
            sinkDataSetLinkLossBDaddr(NULL);

            /* Initialise the A2DP library */
#ifdef ENABLE_PEER

#ifdef INCLUDE_A2DP_EXTRA_CODECS
            setA2dpPeerOptionalCodecs(LOCAL_PEER, a2dp_config_data->A2dpOptionalCodecsEnabled);
#endif

            if ((!sinkBroadcastAudioIsActive()) && (PeerIsShareMeSourceEnabled() || PeerIsTwsSourceEnabled()))
            {
                /* We support some form of source role, so ensure A2DP library advertises A2DP Source SDP record */
                A2dpInit(&theSink.task, A2DP_INIT_ROLE_SINK | A2DP_INIT_ROLE_SOURCE, NULL, number_of_seps, seps, a2dp_config_data->A2dpLinkLossReconnectionTime_s);
            }
            else
#endif
            {
                A2dpInit(&theSink.task, A2DP_INIT_ROLE_SINK, NULL, number_of_seps, seps, a2dp_config_data->A2dpLinkLossReconnectionTime_s);
            }
            A2DP_DATA.ui_connect_update_delay = a2dp_config_data->A2dpConnectionUpdateDelay_ms;
        }

        configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);
    }

    A2DP_DATA.a2dp_initialised = TRUE;
}

/*************************************************************************
NAME
    getA2dpIndex

DESCRIPTION
    This function tries to find a device id match in the array of a2dp links
    to that device id passed in

RETURNS
    match status of true or false
**************************************************************************/
bool getA2dpIndex(uint8 DeviceId, uint16 * Index)
{
    uint8 i;

    /* go through A2dp connection looking for device_id match */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected check its device id */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            /* if a device_id match is found return its value and a
               status of successful match found */
            if(getA2dpLinkDataDeviceId(i) == DeviceId)
            {
                *Index = i;
                A2DP_DEBUG(("A2dp: getIndex = %d\n",i));
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */
    return FALSE;
}


/*************************************************************************
NAME
    getA2dpIndexFromSink

DESCRIPTION
    This function tries to find the a2dp device associated with the supplied
    sink.  The supplied sink can be either a signalling or media channel.

RETURNS
    match status of true or false
**************************************************************************/
bool getA2dpIndexFromSink(Sink sink, uint16 * Index)
{
    uint8 i;

    if (!sink || !SinkA2dpIsInitialised())
    {
        return FALSE;
    }

    /* go through A2dp connection looking for sink match */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected check its device id */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            /* if a device_id match is found return its value and a
               status of successful match found */
            if(A2dpSignallingGetSink(getA2dpLinkDataDeviceId(i)) == sink)
            {
                *Index = i;
                return TRUE;
            }

            if(A2dpMediaGetSink(getA2dpLinkDataDeviceId(i), getA2dpLinkDataStreamId(i)) == sink)
            {
                *Index = i;
                return TRUE;
            }
        }
    }

    /* no matches found so return not successful */
    return FALSE;
}

/*************************************************************************
NAME
    getA2dpStreamData

DESCRIPTION
    Function to retreive media sink and state for a given A2DP source

RETURNS
    void
**************************************************************************/
void getA2dpStreamData(a2dp_index_t index, Sink* sink, a2dp_stream_state* state)
{
    *state = a2dp_stream_idle;
    *sink  = (Sink)NULL;

    /* A2DP_DEBUG(("A2dp: getA2dpStreamData(%u)\n",(uint16)priority)); */
    if (SinkA2dpIsInitialised())
    {
         /* A2DP_DEBUG(("A2dp: getA2dpStreamData - peer=%u connected=%u\n",A2DP_DATA.peer_device[priority], getA2dpStatusFlag(CONNECTED, priority))); */
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            *state = A2dpMediaGetState(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) ;
            *sink  = A2dpMediaGetSink(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) ;
             /* A2DP_DEBUG(("A2dp: getA2dpStreamData - state=%u sink=0x%X\n",*state, (uint16)*sink)); */
        }
    }
}

/*************************************************************************
NAME
    getA2dpStreamRole

DESCRIPTION
    Function to retrieve the role (source/sink) for a given A2DP source

RETURNS
    void
**************************************************************************/
void getA2dpStreamRole(a2dp_index_t index, a2dp_role_type* role)
{
    *role = a2dp_role_undefined;

    if (SinkA2dpIsInitialised())
    {
        /* A2DP_DEBUG(("A2dp: getA2dpStreamRole - peer=%u connected=%u\n",A2DP_DATA.peer_device[priority],getA2dpStatusFlag(CONNECTED, priority))); */
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            *role = A2dpMediaGetRole(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) ;
            /* A2DP_DEBUG(("A2dp: getA2dpStreamRole - role=%u priority=%u\n",*role,priority)); */
        }
    }
}

/*************************************************************************
NAME
    getA2dpPlugin

DESCRIPTION
    This function returns the task of the appropriate audio plugin to be used
    for the selected codec type when connecting audio

RETURNS
    task of relevant audio plugin
**************************************************************************/
Task getA2dpPlugin(uint8 seid)
{
    uint16 i;

    for (i=0; i<NUM_SEPS; i++)
    {
        if (codecList[i].config && (codecList[i].config->seid == seid))
        {
            if(useBroadcastPlugin())
            {
                if(SBC_SEID == seid)
                    return (Task)&csr_ba_sbc_decoder_plugin.data;
                else if (AAC_SEID == seid)
                    return (Task)&csr_ba_aac_decoder_plugin.data;
            }
            else
            {
                return (Task)codecList[i].plugin;
            }
        }
    }

    /* No plugin found so Panic */
    Panic();
    return 0;
}


/*************************************************************************
NAME
    openStream

DESCRIPTION


RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
static bool openStream (uint16 Id, uint8 seid)
{
    if (((seid & SOURCE_SEID_MASK) == 0) && sinkA2dpIsStreamingAllowed())
    {   /* Ensure a source SEID has not been requested */
        if (seid)
        {   /* Request to use the specified seid only */
            /*return A2dpMediaOpenRequestEx(A2DP_DATA.device_id[Id], 1, &seid, sizeof(a2dp_media_conftab), a2dp_media_conftab);*/
            return A2dpMediaOpenRequest(getA2dpLinkDataDeviceId(Id), 1, &seid);
        }
        else
        {   /* Use default seid list, as specified in call to A2dpInit() */
            /*return A2dpMediaOpenRequestEx(A2DP_DATA.device_id[Id], 0, NULL, sizeof(a2dp_media_conftab), a2dp_media_conftab);*/
            return A2dpMediaOpenRequest(getA2dpLinkDataDeviceId(Id), 0, NULL);
        }
    }

    return FALSE;
}


/****************************************************************************
NAME
    sinkA2dpSetLinkRole

DESCRIPTION
    Updates stored BT role for specified device

RETURNS
    None

**************************************************************************/
void sinkA2dpSetLinkRole (Sink sink, hci_role role)
{
    uint16 index;

    A2DP_DEBUG(("sinkA2dpSetLinkRole  sink=%p  role=%s\n", (void*)sink, (role == hci_role_master) ? "master" : "slave"));

    if (getA2dpIndexFromSink(sink, &index))
    {
#ifdef ENABLE_PEER
        a2dp_link_role last_role = getA2dpLinkRole(index);
#endif

        if (role == hci_role_master)
        {
            A2DP_DEBUG(("    setting  link_role[%u] = LR_CURRENT_ROLE_MASTER\n",index));
            setA2dpLinkRole(index, LR_CURRENT_ROLE_MASTER);
        }
        else
        {
            A2DP_DEBUG(("    setting  link_role[%u] = LR_CURRENT_ROLE_SLAVE\n",index));
            setA2dpLinkRole(index, LR_CURRENT_ROLE_SLAVE);
        }

#ifdef ENABLE_PEER
        if (last_role == LR_CHECK_ROLE_PENDING_START_REQ)
        {
            A2DP_DEBUG(("    start was pending...\n"));
            a2dpIssuePeerStartRequest();
        }
#endif
    }
}

/****************************************************************************
NAME
    sinkA2dpInitComplete

DESCRIPTION
    Sink A2DP initialisation has completed, check for success.

RETURNS
    void
**************************************************************************/
void sinkA2dpInitComplete(const A2DP_INIT_CFM_T *msg)
{
    /* check for successful initialisation of A2DP libraray */
    if(msg->status == a2dp_success)
    {
        A2DP_DEBUG(("A2DP Init Success\n"));
    }
    else
    {
        A2DP_DEBUG(("A2DP Init Failed [Status %d]\n", msg->status));
        Panic();
    }
}


/****************************************************************************
NAME
    findUnconnectedA2dpDevice

DESCRIPTION
    Attempts to locate a device that is paging us based on its bluetooth address.
    An entry will exist for devices we are considering allowing connection to in
    the A2DP_DATA structure.  However, the connected flag will be FALSE.

RETURNS
    TRUE if device found, FALSE otherwise.
**************************************************************************/
static bool isUnconnectedA2dpDevice (const bdaddr *bd_addr, uint16* idx)
{
    for_all_a2dp(*idx)
    {
        if (!getA2dpStatusFlag(CONNECTED, *idx) && BdaddrIsSame(bd_addr, getA2dpLinkBdAddr(*idx)))
        {   /* Found the unconnected device */
            return TRUE;
        }
    }

    /* Device is either unknown to us or already connected */
    return FALSE;
}


/****************************************************************************
NAME
    getKnownPeerDeviceAttributes

DESCRIPTION
    Reads the peer device attributes.

RETURNS
    Void
**************************************************************************/
#ifdef ENABLE_PEER
static void getKnownPeerDeviceAttributes(const uint16 priority , sink_attributes *attributes)
{
    /* Get any known peer device attributes */
    deviceManagerGetDefaultAttributes(attributes, dev_type_ag);
    deviceManagerGetAttributes(attributes, getA2dpLinkBdAddr(priority));
    setA2dpPeerRemoteDevice(priority, attributes->peer_device);
    setA2dpPeerRemoteFeatures(priority, attributes->peer_features);
    setA2dpLinkDataPeerVersion(priority, attributes->peer_version);
    setPeerDSPRequiredBufferingLevel(priority, getPeerDSPBufferingRequired(attributes->peer_version));
}
#endif

/****************************************************************************
NAME
    isAnyA2dpSourcePlaying

DESCRIPTION
    Checks if any of the A2DP sources are in PLAYING state

RETURNS
    TRUE - if atleast one A2DP source is in PLAYING state
    FALSE - if all A2DP sources are not in PLAYING state
**************************************************************************/
static bool isAnyA2dpSourcePlaying(void)
{
    uint16 index;

    for_all_a2dp(index)
    {
        if (getA2dpStatusFlag(PLAYING, index))
            return TRUE;
    }
    return FALSE;
}

/****************************************************************************
NAME
    issueA2dpSignallingConnectResponse

DESCRIPTION
    Issue response to a signalling channel connect request, following discovery of the
    remote device type.

RETURNS
    void
**************************************************************************/
void issueA2dpSignallingConnectResponse(const bdaddr *bd_addr, bool accept)
{
    uint16 idx;

    A2DP_DEBUG(("issueA2dpSignallingConnectResponse\n"));

    if (!SinkA2dpIsInitialised())
    {
        A2DP_DEBUG(("A2DP module not initialised\n"));
        return;
    }

    if (!isUnconnectedA2dpDevice(bd_addr, &idx))
    {   /* We have no knowledge of this device */
        A2DP_DEBUG(("device unknown or already connected!\n"));
        return;
    }

    if (!accept)
    {
        A2DP_DEBUG(("Reject\n"));
        A2dpSignallingConnectResponse(getA2dpLinkDataDeviceId(idx), FALSE);
        return;
    }

    {
#ifdef ENABLE_PEER
        sink_attributes attributes;

        A2DP_DEBUG(("   peer = %u\n",getA2dpPeerRemoteDevice(idx)));
        A2DP_DEBUG(("   features = %u\n",getA2dpPeerRemoteFeatures(idx)));
        A2DP_DEBUG(("   version = 0x%X\n", getA2dpLinkDataPeerVersion(idx)));

        deviceManagerGetDefaultAttributes(&attributes, dev_type_ag);
        deviceManagerGetAttributes(&attributes, bd_addr);
        attributes.peer_device = getA2dpPeerRemoteDevice(idx);
        attributes.peer_features = getA2dpPeerRemoteFeatures (idx);
        attributes.peer_version = getA2dpLinkDataPeerVersion(idx);
        attributes.peer_supported_optional_codecs = getA2dpPeerOptionalCodecs(REMOTE_PEER);

        deviceManagerStoreAttributes(&attributes, bd_addr);

        if(!peerLinkReservedCanDeviceConnect(bd_addr))
        {
            /* If there is already an AG connected and the PeerLinkReserved is set, then do not allow
                             another AG to connect */
            A2DP_DEBUG(("Reject\n"));
            A2dpSignallingConnectResponse(getA2dpLinkDataDeviceId(idx),FALSE);
            return;
        }
#endif  /*ENABLE_PEER */

        A2DP_DEBUG(("Accept\n"));
        A2dpSignallingConnectResponse(getA2dpLinkDataDeviceId(idx),TRUE);

#ifdef ENABLE_AVRCP
        sinkAvrcpCheckManualConnectReset(bd_addr);
#endif
    }
}

/*************************************************************************
NAME
    handleA2DPSignallingConnectInd

DESCRIPTION
    handle a signalling channel connect indication

RETURNS

**************************************************************************/
void handleA2DPSignallingConnectInd(uint8 DeviceId, bdaddr SrcAddr)
{
#ifdef ENABLE_PEER
    sink_attributes attributes;
#endif
    /* before accepting check there isn't already a signalling channel connected to another AG */
    if ( (sinkA2dpEnabled()) &&
         ((!getA2dpStatusFlag(CONNECTED, a2dp_primary)) || (!getA2dpStatusFlag(CONNECTED, a2dp_secondary))))
    {
        /* store the device_id for the new connection in the first available storage position */
        uint16 priority = (!getA2dpStatusFlag(CONNECTED, a2dp_primary)) ? a2dp_primary : a2dp_secondary;

        A2DP_DEBUG(("Signalling Success, Device ID = %x\n",DeviceId));
        setA2dpStatusFlag(CONNECTED, priority, FALSE);
        setA2dpLinkDataDeviceId(priority, DeviceId);
        setA2dpLinkBdAddr(priority, SrcAddr);
        setA2dpPdlListId(priority, 0);

#ifdef ENABLE_PEER
        /* Check if the bd address of the connected Ag is the same as that connected to the peer,
            if so then disconnect the ag  */
        if(BdaddrIsSame(peerGetRemotePeerAgBdAddress() , &SrcAddr))
        {
            A2DP_DEBUG(("Reject\n"));
            A2dpSignallingConnectResponse(DeviceId,FALSE);
            sinkDisconnectSlcFromDevice(&SrcAddr);
            return;
        }

        getKnownPeerDeviceAttributes(priority , &attributes);

        if (getA2dpPeerRemoteDevice(priority) == remote_device_unknown)
        {   /* Determine remote device type before accepting connection */
            if (!peerCredentialsRequest(&SrcAddr))
            {   /* Peer credentials not requested */
                if (sinkInquiryIsInqSessionPeer())
                {   /* We are initiating a Peer session */
                    A2DP_DEBUG(("Peer Device SDP record not requested, assuming Peer\n"));

                    /* Assume device is a ShareMe sink without custom avrcp operation and allow to connect */
                    attributes.peer_device = remote_device_peer;
                    attributes.peer_features = remote_features_shareme_a2dp_sink;
                    attributes.peer_version = 0;
                    deviceManagerStoreAttributes(&attributes, &SrcAddr);
                }
                else
                {
                    /* Assume device is a standard source */
                    attributes.peer_device = remote_device_nonpeer;
                    attributes.peer_features = remote_features_none;
                    attributes.peer_version = 0;
                    deviceManagerStoreAttributes(&attributes, &SrcAddr);
                }

                if(!peerLinkReservedCanDeviceConnect(&SrcAddr))
                {
                    /* If there is already an AG connected and the PeerLinkReserved is set, then do not allow
                                       another AG to connect */
                    A2DP_DEBUG(("Reject\n"));
                    A2dpSignallingConnectResponse(DeviceId,FALSE);
                }
                else
                {
                    A2DP_DEBUG(("Accept\n"));

                    if (attributes.peer_device == remote_device_nonpeer)
                    {
                        /* indicate that this is a remote connection */
                        setA2dpStatusFlag(REMOTE_AG_CONNECTION, INVALID_ID, TRUE);
                    }

                    A2dpSignallingConnectResponse(DeviceId,TRUE);
#ifdef ENABLE_AVRCP
                    sinkAvrcpCheckManualConnectReset(&SrcAddr);
#endif
                }
            }
        }
        else
        {
            /* Update the optional codecs supported by the remote device */
            peerSetRemoteSupportedCodecs(attributes.peer_supported_optional_codecs);

            if(!peerLinkReservedCanDeviceConnect(&SrcAddr))
            {
                /* If there is already an AG connected and the PeerLinkReserved is set, then do not allow
                              another AG to connect */
                A2DP_DEBUG(("Reject\n"));
                A2dpSignallingConnectResponse(DeviceId,FALSE);
            }
            else
#else
            setA2dpPeerRemoteDevice(priority, remote_device_nonpeer);
#endif
            {
                A2DP_DEBUG(("Accept\n"));

                if (getA2dpPeerRemoteDevice(priority) == remote_device_nonpeer)
                {
                    /* indicate that this is a remote connection */
                    setA2dpStatusFlag(REMOTE_AG_CONNECTION, INVALID_ID, TRUE);
                }

                A2dpSignallingConnectResponse(DeviceId,TRUE);
#ifdef ENABLE_AVRCP
                sinkAvrcpCheckManualConnectReset(&SrcAddr);
#endif
            }
#ifdef ENABLE_PEER
        }
#endif
    }
    else
    {
        A2DP_DEBUG(("Reject\n"));
        A2dpSignallingConnectResponse(DeviceId,FALSE);
    }
}


/*************************************************************************
NAME
    handleA2DPSignallingConnected

DESCRIPTION
    handle a confirm of a signalling channel connected message

RETURNS

**************************************************************************/
void handleA2DPSignallingConnected(a2dp_status_code status, uint8 DeviceId, bdaddr SrcAddr, bool locally_initiated)
{
    sink_attributes attributes;

    /* Use default attributes if none exist is PS */
    deviceManagerGetDefaultAttributes(&attributes, dev_type_ag);
    deviceManagerGetAttributes(&attributes, &SrcAddr);

    /* Decrement outstanding paging attempts */
    sinkInquiryDecrementPagingCount();
    
    /* Continue connection procedure */
    if(!getA2dpStatusFlag(REMOTE_AG_CONNECTION, INVALID_ID))
    {
        uint32 delay = sinkDataSecondAGConnectDelayTimeout();

#ifdef ENABLE_PEER
        /* If we have just connected to a peer it may already have an active
           a2dp stream. If so, we need to delay polling the next device in
           the PDL to give the peer relay state time to update to
           streaming. This is so we can tell it is streaming when
           deviceManagerCanConnect is called and stop the connection.

           Otherwise paging the device will interrupt the peer a2dp stream -
           a known limitation. */
        if (attributes.peer_device == remote_device_peer)
        {
            A2DP_DEBUG(("A2DP: Delaying next slc connect request by up to 4 seconds\n"));
            /* add a random delay of up to 4 seconds to prevent reconnection attempts on both
               sides occuring at the same time every reconnection attempt cycle */
            delay += (UtilRandom() >> 4);
        }
#endif
        MessageSendLater(&theSink.task, EventSysContinueSlcConnectRequest, 0, delay);
    }
    else
    {
        /* reset remote connection indication flag */
        setA2dpStatusFlag(REMOTE_AG_CONNECTION, INVALID_ID, FALSE);
    }

    /* check for successful connection */
    if (status != a2dp_success)
    {
        uint16 priority;
        uint16 numReconnectAttempts = sinkDataGetNoOfReconnectionAttempts();

        A2DP_DEBUG(("Signalling Failed device=%u [Status %d]\n", DeviceId, status));

#ifdef ENABLE_PEER
        if ( (sinkInquiryIsInqSessionPeer()) && (status == a2dp_operation_fail) &&
             isUnconnectedA2dpDevice(&SrcAddr, &priority) &&
             (getA2dpPeerRemoteDevice(priority) == remote_device_peer) )
        {   /* A rejected pairing with a Peer device */
            A2DP_DEBUG(("Remove Peer %u from PDL\n", priority));
            ConnectionSmDeleteAuthDevice(getA2dpLinkBdAddr(priority));
        }
#endif
        /* If the device failed to connect A2DP, decrement the connection attempts */
        if(numReconnectAttempts)
        {
            sinkDataSetNoOfReconnectionAttempts(numReconnectAttempts - 1);
        }

        /* If necessary, clear appropriate link data structure which will have been filled on an incoming connection */
        if ((status != a2dp_wrong_state) && (status != a2dp_max_connections))   /* TODO: Temp fix */
        {
            if ( BdaddrIsSame(&SrcAddr, getA2dpLinkBdAddr(priority=a2dp_primary)) ||
                 BdaddrIsSame(&SrcAddr, getA2dpLinkBdAddr(priority=a2dp_secondary)) )
            {
                A2DP_DEBUG(("Clearing link data for %u\n", priority));
                setA2dpPeerRemoteDevice(priority, remote_device_unknown);
                setA2dpPeerRemoteFeatures(priority, remote_features_none);
                setA2dpLinkDataPeerVersion(priority, 0);
                setA2dpPeerStatus(priority, LOCAL_PEER, 0);
                setA2dpPeerStatus(priority, REMOTE_PEER, 0);
                setA2dpStatusFlag(CONNECTED, priority, FALSE);
                setA2dpLinkDataDeviceId(priority, INVALID_DEVICE_ID);
                setA2dpPdlListId(priority, 0);
                setA2dpLinkRole(priority, LR_UNKNOWN_ROLE);
                BdaddrSetZero(getA2dpLinkBdAddr(priority));
#ifdef ENABLE_PEER
                setPeerDSPRequiredBufferingLevel(priority, getPeerDSPBufferingRequired(0));
#endif
                linklossResetStreamProtection(priority);
            }
        }

#ifdef ENABLE_AVRCP
        sinkAvrcpCheckManualConnectReset(&SrcAddr);
#endif

        /* if a failed inquiry connect then restart it */
        if(locally_initiated)
        {
            bool inquiringPeer = (sinkInquiryCheckInqActionRssi() && sinkInquiryIsInqSessionPeer());
            if (inquiringPeer || sinkInquiryIsStateConnecting())
            {
                /* connection now failed/complete, update current state */
                sinkInquirySetInquiryState(inquiry_complete);
                inquiryStop();
                if(inquiringPeer)
                    inquiryPair( inquiry_session_peer, FALSE );
            }
        }
    }
    /* connection was successful */
    else
    {
        /* Send a message to request a role indication and make necessary changes as appropriate, message
           will be delayed if a device initiated connection to another device is still in progress */
        A2DP_DEBUG(("handleA2DPSignallingConnected: Asking for role check\n"));

        /* cancel any link loss reminders */
        linklossCancelLinkLossTone();

        /* cancel any pending messages and replace with a new one */
        MessageCancelFirst(&theSink.task , EventSysCheckRole);
        MessageSendConditionally (&theSink.task , EventSysCheckRole , NULL , sinkDataIsConnectionInProgress()  );

        /* check for a link loss condition, if the device has suffered a link loss and was
           succesfully reconnected by the a2dp library a 'signalling connected' event will be
           generated, check for this and retain previous connected ID for this indication */
        if(((getA2dpStatusFlag(CONNECTED, a2dp_primary))&&(BdaddrIsSame(&SrcAddr, getA2dpLinkBdAddr(a2dp_primary))))||
           ((getA2dpStatusFlag(CONNECTED, a2dp_secondary))&&(BdaddrIsSame(&SrcAddr, getA2dpLinkBdAddr(a2dp_secondary)))))
        {
            /* reconnection is the result of a link loss, don't assign a new id */
            A2DP_DEBUG(("Signalling Connected following link loss [Status %d]\n", status));
        }
        else
        {
            /* store the device_id for the new connection in the first available storage position */
            if (BdaddrIsSame(&SrcAddr, getA2dpLinkBdAddr(a2dp_primary)) ||
                (BdaddrIsZero(getA2dpLinkBdAddr(a2dp_primary)) && !getA2dpStatusFlag(CONNECTED, a2dp_primary)))
            {
                A2DP_DEBUG(("Signalling Success, Primary ID = %x\n",DeviceId));
                setA2dpStatusFlag(CONNECTED, a2dp_primary, TRUE);
                setA2dpLinkDataDeviceId(a2dp_primary, DeviceId);
                setA2dpLinkBdAddr(a2dp_primary, SrcAddr);
                setA2dpPdlListId(a2dp_primary, deviceManagerSetPriority(&SrcAddr));
                setA2dpStatusFlag(MEDIA_RECONNECT,a2dp_primary, FALSE);
                setA2dpLinkDataLatency(a2dp_primary, 0);
                setA2dpPeerStatus(a2dp_primary, LOCAL_PEER, 0);
                setA2dpPeerStatus(a2dp_primary, REMOTE_PEER, 0);
                setA2dpPeerRemoteFeatures(a2dp_primary, remote_features_none);
                setA2dpLinkDataPeerVersion(a2dp_primary, 0);
                setA2dpLinkRole(a2dp_primary, LR_UNKNOWN_ROLE);
                setA2dpStatusFlag(PLAYING, a2dp_primary, FALSE);
#ifdef ENABLE_PEER
                setPeerDSPRequiredBufferingLevel(a2dp_primary, getPeerDSPBufferingRequired(0));
#else
                setA2dpPeerRemoteDevice(a2dp_primary, remote_device_nonpeer);
#endif
                linklossResetStreamProtection(a2dp_primary);
            }
            /* this is the second A2DP signalling connection */
            else if (BdaddrIsSame(&SrcAddr,getA2dpLinkBdAddr(a2dp_secondary)) ||
                     (BdaddrIsZero(getA2dpLinkBdAddr(a2dp_secondary)) && !getA2dpStatusFlag(CONNECTED, a2dp_secondary)))
            {
                A2DP_DEBUG(("Signalling Success, Secondary ID = %x\n",DeviceId));
                setA2dpStatusFlag(CONNECTED, a2dp_secondary, TRUE);
                setA2dpLinkDataDeviceId(a2dp_secondary, DeviceId);
                setA2dpLinkBdAddr(a2dp_secondary, SrcAddr);
                setA2dpPdlListId(a2dp_secondary, deviceManagerSetPriority(&SrcAddr));
                setA2dpStatusFlag(MEDIA_RECONNECT, a2dp_secondary, FALSE);
                setA2dpLinkDataLatency(a2dp_secondary, 0);
                setA2dpPeerStatus(a2dp_secondary, LOCAL_PEER, 0);
                setA2dpPeerStatus(a2dp_secondary, REMOTE_PEER, 0);
                setA2dpPeerRemoteFeatures(a2dp_secondary, remote_features_none);
                setA2dpLinkDataPeerVersion(a2dp_secondary, 0);
                setA2dpLinkRole(a2dp_secondary, LR_UNKNOWN_ROLE);
                setA2dpStatusFlag(PLAYING, a2dp_secondary, FALSE);
#ifdef ENABLE_PEER
                setPeerDSPRequiredBufferingLevel(a2dp_secondary, getPeerDSPBufferingRequired(0));
#else
                setA2dpPeerRemoteDevice(a2dp_secondary, remote_device_nonpeer);
#endif
                linklossResetStreamProtection(a2dp_secondary);
            }
        }

        /* Ensure the underlying ACL is encrypted */
        ConnectionSmEncrypt( &theSink.task , A2dpSignallingGetSink(DeviceId) , TRUE );
        ConnectionSetLinkSupervisionTimeout(A2dpSignallingGetSink(DeviceId), SINK_LINK_SUPERVISION_TIMEOUT);

        /* If the device is off then disconnect */
        if (stateManagerGetState() == deviceLimbo)
        {
            A2dpSignallingDisconnectRequest(DeviceId);
        }
        else
        {
            a2dp_index_t index;
    
            /* For a2dp connected Tone and Voice Prompt */
            MessageSendLater(&theSink.task, EventSysA2dpConnected, 0, A2DP_DATA.ui_connect_update_delay);

            /* if using multipoint and both devices are connected disable connectable */
            slcMultipointDisableConnectable();

            /* find structure index of deviceId */
            if(getA2dpIndex(DeviceId, (uint16*)&index))
            {
#ifdef ENABLE_PARTYMODE
                /* check whether party mode is enabled */
                if(sinkPartymodeIsEnabled() && sinkPartymodeGetOperatingMode())
                {
                    /* Cancel any existing partymode timer running for AG */
                    MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1 + index));
                    /* start a timer when a device connects in party mode, if no music is played before the timeout
                       occurs the device will get disconnected to allow other devices to connect, this timer is a configurable
                       item in Sink configuration tool */
                    MessageSendLater(&theSink.task,(EventSysPartyModeTimeoutDevice1 + index),0,D_SEC(sinkPartymodeGetMusicTimeOut()));
                    /* set paused flag */
                    if(index == a2dp_primary)
                    {
                       sinkPartymodePauseAudio(partymode_primary, FALSE);
                    }
                    else
                    {
                       sinkPartymodePauseAudio(partymode_secondary, FALSE);
                    }
                    sinkUpdatePartyModeStateChangeEvent(EventSysPartyModeConnected);
                }
#endif
                /* update master volume level */
                sinkA2dpSetA2dpVolumeAtIndex(index, attributes.a2dp.volume);
                setA2dpLinkDataClockMismatchRate(index, attributes.a2dp.clock_mismatch);

#ifdef ENABLE_PEER
                if ((slcDetermineConnectAction() & AR_Rssi) && sinkinquiryGetInquiryResults() != NULL)
                {   /* Set attributes when connecting from a Peer Inquiry */
                    remote_device new_device = sinkInquiryGetPeerInqResult(sinkInquiryGetConnAttemptingIndex());

                    if (attributes.peer_device == remote_device_unknown && new_device != remote_device_unknown)
                    {
                        attributes.peer_device = new_device;
                        attributes.peer_supported_optional_codecs = getA2dpPeerOptionalCodecs(REMOTE_PEER);

                        if (attributes.peer_device == remote_device_peer)
                        {
                            attributes.peer_features = sinkInquiryGetPeerFeatures(sinkInquiryGetConnAttemptingIndex());
                            attributes.peer_version = sinkInquiryGetPeerVersion();
                        }
                    }
                }

                if (attributes.peer_device != remote_device_unknown)
                {   /* Only update link data if device type is already known to us */
                    setA2dpPeerRemoteDevice(index, attributes.peer_device);
                    setA2dpPeerRemoteFeatures(index, attributes.peer_features);
                    setA2dpLinkDataPeerVersion(index, attributes.peer_version);
                    setPeerDSPRequiredBufferingLevel(index, getPeerDSPBufferingRequired(attributes.peer_version));

                    /* If the remote device is a peer update the remote
                       supported codecs from the stored sink_attributes. */
                    if (attributes.peer_device == remote_device_peer)
                    {
                        peerSetRemoteSupportedCodecs(attributes.peer_supported_optional_codecs);
                    }
                }
                 /* Reset peer_link_loss_reconnect if a remote peer device connects or if both the a2dp links are connected */
                if ((getA2dpPeerRemoteDevice(index) == remote_device_peer ) ||
                    (getA2dpStatusFlag(CONNECTED, a2dp_primary) && getA2dpStatusFlag(CONNECTED, a2dp_secondary)))
                {
                    setA2dpPeerLinkLossReconnect(FALSE);
                }
                A2DP_DEBUG(("Remote device type = %u\n",getA2dpPeerRemoteDevice(index)));
                A2DP_DEBUG(("Remote device features = 0x%x\n",getA2dpPeerRemoteFeatures(index)));
                A2DP_DEBUG(("Remote device version = 0x%X\n",getA2dpLinkDataPeerVersion(index)));
#endif
               /* if connected as a result of an inquiry connect, stop further attempts */
                if(sinkInquiryIsStateConnecting())
                {
                    /* connection now complete, update current state */
                    sinkInquirySetInquiryState(inquiry_complete);
                    /* and stop further attempts */
                    inquiryStop();
                }

                /* We are now connected */
                if (stateManagerGetState() < deviceConnected && stateManagerGetState() != deviceLimbo)
                {
#ifdef ENABLE_PEER
                    if ((stateManagerGetState() == deviceConnDiscoverable) &&
                        (attributes.peer_device == remote_device_peer) &&
                        (!sinkInquiryIsInqSessionPeer()))
                    {
                        sinkInquirySetInquirySession(inquiry_session_normal);
                        stateManagerEnterConnDiscoverableState(TRUE);
                    }
                    else
#endif
                    {
                        stateManagerEnterConnectedState();
                    }
                }

                /* Make sure we store this device */
                attributes.profiles |= sink_a2dp;
                deviceManagerStoreAttributes(&attributes, &SrcAddr);

                /* Update the linkloss managemt for this device*/
                linklossUpdateManagement(getA2dpLinkBdAddr(index));

                /* check on signalling check indication if the a2dp was previously in a suspended state,
                   this can happen if the device has suspended a stream and the phone has chosen to drop
                   the signalling channel completely, open the media connection or the feature to open a media
                   channel on signalling connected option is enabled */
#ifdef ENABLE_PEER
                if (getA2dpPeerRemoteDevice(index) != remote_device_peer)
#endif
                {   /* Unknown or non-peer device */
#ifdef ENABLE_PEER
                    /* Check if the bd address of the connected Ag is the same as that connected to the peer,
                         if so then disconnect the ag  */
                    if(BdaddrIsSame(peerGetRemotePeerAgBdAddress() , &SrcAddr) || !peerLinkReservedCanDeviceConnect(&SrcAddr))
                    {
                        sinkDisconnectSlcFromDevice(&SrcAddr);
                        disconnectA2dpAvrcpFromDevice(&SrcAddr);
                    }
                    else
                    {
                        /*If the A2DP has connected successfully to a non-peer device then notify this to the peer device */
                        sinkAvrcpUpdatePeerWirelessSourceConnected(A2DP_AUDIO, &SrcAddr);
                    }


                    /* If peer is already streaming and a new AG is
                       connecting we need to turn on a2dp stream protection. */
                    if (!peerLinkRecoveryWhileStreamingEnabled()
                        && !peerIsThisDeviceTwsMaster()
                        && (peerCurrentRelayState() >= RELAY_STATE_STARTING))
                    {
                        linklossProtectStreaming(linkloss_stream_protection_on);
                    }

                    PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE CONNECTED");
#endif
                    if ((SinkA2dpGetSuspendState(index) == a2dp_local_suspended) ||
                        (sinkA2dpOpenMediaOnConnection()))
                    {
                        connectA2dpStream( index, D_SEC(5) );
                    }
                }
#ifdef ENABLE_PEER
                else
                {   /* Peer device */
                    updateServiceCapsSupportedWhenEnteringTwsModeIfNeededReconnectAvSource();

                    /* Update permanent pairing */
                    peerUpdatePairing(index, &attributes);

                    /* Update the extended state to both AG and Peer connected state
                       when the peer comes as a second connected device */
                    updateExtendedState( deviceConnected );

                    /* Kick Peer state machine to start relaying audio, if necessary */
                    peerAdvanceRelayState(RELAY_EVENT_CONNECTED);

                    if (!peerLinkRecoveryWhileStreamingEnabled())
                    {
                        linklossProtectStreaming(linkloss_stream_protection_on);
                    }
                }
#endif
            }

            /* if rssi pairing check to see if need to cancel rssi pairing or not */
            if(sinkInquiryCheckInqActionRssi())
            {
                /* if rssi pairing has completed then stop it progressing further */
                if(!((sinkDataGetPDLLimitforRSSIPairing())&&( ConnectionTrustedDeviceListSize() < sinkDataGetPDLLimitforRSSIPairing())))
                {
                    inquiryStop();
                }
            }

#ifdef ENABLE_AVRCP

            if(avrcpAvrcpIsEnabled())
            {
                if(BdaddrIsSame(getA2dpAgToReconnect() , &SrcAddr))
                {
                     sinkAvrcpEnableManualConnect(TRUE);
                }

                if (sinkAvrcpIsManualConnectEnabled())
                {
                    sinkAvrcpSetPlayAddress(SrcAddr);
                }

                if(getA2dpIndex(DeviceId, (uint16*)&index))
                {
#ifdef ENABLE_PEER
                    /* Peer devices that support custom AVRCP operation do not need to delay initial AVRCP connection */
                    if ((getA2dpPeerRemoteDevice(index) == remote_device_peer) && (getA2dpPeerRemoteFeatures(index) & remote_features_peer_avrcp) && locally_initiated)
                    {
                        sinkAvrcpConnect(getA2dpLinkBdAddr(index), DEFAULT_AVRCP_NO_CONNECTION_DELAY);
                    }
                    else
#endif
                    {
                        sinkAvrcpConnect(getA2dpLinkBdAddr(index), DEFAULT_AVRCP_1ST_CONNECTION_DELAY);
                    }
                }
            }
#endif

#ifdef ENABLE_PEER
            /* Check if the bd address of the connected Ag is the same as that connected to the peer,
            if so then attempt no profile connections */
            if(BdaddrIsSame(peerGetRemotePeerAgBdAddress() , &SrcAddr))
            {
                 /* Don't attempt for other profile connection as this is a peer device */
            }
            else
#endif
            {
                 deviceManagerHandleProfilesConnection(&SrcAddr);
            }

        }
    }
    /* update the number and type of devices connected */
    MessageSendLater(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL, A2DP_DATA.ui_connect_update_delay);
}


/*************************************************************************
NAME
    connectA2dpStream

DESCRIPTION
    Issues a request to the A2DP library to establish a media stream to a
    remote device.  The request can be delayed by a certain amount of time
    if required.

RETURNS

**************************************************************************/
void connectA2dpStream (a2dp_index_t index, uint16 delay)
{
    A2DP_DEBUG(("A2dp: connectA2dpStream delay=%u\n", delay));

    if (!delay)
    {
        if (SinkA2dpIsInitialised() && getA2dpStatusFlag(CONNECTED, index))
        {
#ifdef ENABLE_PEER
            if (getA2dpPeerRemoteDevice(index) == remote_device_unknown)
            {   /* Still waiting for Device Id SDP search outcome issued in handleA2DPSignallingConnected() */
                EVENT_STREAM_ESTABLISH_T *message = PanicUnlessNew(EVENT_STREAM_ESTABLISH_T);

                message->a2dp_index = index;
                MessageSendLater(&theSink.task, EventSysStreamEstablish, message, 200);  /* Ideally we'd send conditionally, but there isn't a suitable variable */

                A2DP_DEBUG(("local device is unknown, re-issue stream establish event\n"));
            }
            else if (getA2dpPeerRemoteDevice(index) == remote_device_nonpeer)
#endif
            {   /* Open media channel to AV Source */
                A2DP_DEBUG(("local device is non-peer (AV Source)\n"));
                if (A2dpMediaGetState(getA2dpLinkDataDeviceId(index), 0) == a2dp_stream_idle)
                {
                    A2DP_DEBUG(("AV Source stream idle\n"));
                    A2DP_DEBUG(("Send open req to AV Source, using defualt seid list\n"));
                    openStream(index, 0);
                }
            }
        }
    }
    else
    {
        EVENT_STREAM_ESTABLISH_T *message = PanicUnlessNew(EVENT_STREAM_ESTABLISH_T);

        message->a2dp_index = index;
        MessageSendLater(&theSink.task, EventSysStreamEstablish, message, delay);

        A2DP_DEBUG(("... wait for %u msecs\n", delay));
    }
}


/*************************************************************************
NAME
    handleA2DPOpenInd

DESCRIPTION
    handle an indication of an media channel open request, decide whether
    to accept or reject it

RETURNS

**************************************************************************/
void handleA2DPOpenInd(uint8 DeviceId, uint8 seid)
{
    A2DP_DEBUG(("A2dp: OpenInd DevId = %d, seid = 0x%X\n",DeviceId, seid));

    UNUSED(seid); /* Can be used depending on compile-time definitions */

    if (sinkA2dpIsStreamingAllowed())
    {
#ifdef ENABLE_PEER
        {
            uint16 Id;
            bdaddr bd_addr;

            /*Get the A2DP index from the BD Address corresponding to the DeviceId */
            if(A2dpDeviceGetBdaddr(DeviceId, &bd_addr) && getA2dpIndexFromBdaddr(&bd_addr , &Id))
            {
                /* Always accept an open indication, regardless of whether it comes from an AV Source / Peer */
                setA2dpLinkDataSeId(Id, seid);

                if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
                {
                    peerAdvanceRelayState(RELAY_EVENT_OPENING);
                }

                A2DP_DEBUG(("Ind from %s,  Sending open resp\n", rdname[getA2dpPeerRemoteDevice(Id)]));
                A2dpMediaOpenResponse(DeviceId, TRUE);
            }
        }
#else
        UNUSED(seid);

        /* accept this media connection */
        if(A2dpMediaOpenResponse(DeviceId, TRUE))
        {
            uint16 Id;
            A2DP_DEBUG(("Open Success\n"));

            /* find structure index of deviceId */
            if(getA2dpIndex(DeviceId, &Id))
                setA2dpLinkDataDeviceId(Id, DeviceId);

        }
#endif
    }
    else
    {
        A2dpMediaOpenResponse(DeviceId, FALSE);
    }
}

/****************************************************************************************
NAME
    playCodecPromptIfNewDevice

DESCRIPTION
    Checks if the incoming device has played prompt already first time after power cycle.
    If not then it is connecting first time so prompt will be played . If its reconnection
    then prompt will not be played.

RETURNS
    None

*******************************************************************************************/
static void playCodecPromptIfNewDevice(bdaddr *bd_addr ,uint8 DeviceId,uint8 StreamId)
{
    sink_attributes attributes;

    /*Read device attributes to check if codec audio prompt played or not */
    deviceManagerGetAttributes(&attributes,bd_addr );

    if(!attributes.audio_prompt_play_status)
    {
        sinkUpdateCodecConfiguredToApp(DeviceId,StreamId);
        attributes.audio_prompt_play_status = TRUE;
        deviceManagerStoreAttributes(&attributes, bd_addr);
    }
}
/*************************************************************************
NAME
    handleA2DPOpenCfm

DESCRIPTION
    handle a successful confirm of a media channel open

RETURNS

**************************************************************************/
void handleA2DPOpenCfm(uint8 DeviceId, uint8 StreamId, uint8 seid, a2dp_status_code status)
{
    bool status_avrcp = FALSE;

    /* ensure successful confirm status */
    if (status == a2dp_success)
    {
        uint16 Id;
        bdaddr bd_addr;
#ifdef ENABLE_AVRCP
        uint16 i;
#endif
        A2DP_DEBUG(("Open Success\n"));

        /*Get the A2DP index from the BD Address corresponding to the DeviceId */
        if(A2dpDeviceGetBdaddr(DeviceId, &bd_addr) && getA2dpIndexFromBdaddr(&bd_addr , &Id))
        {
            A2DP_DEBUG(("Open Success - id=%u\n",Id));

            /* set the current seid */
            setA2dpLinkDataDeviceId(Id, DeviceId);
            setA2dpLinkDataStreamId(Id, StreamId);
            setA2dpLinkDataSeId(Id, seid);
            sinkBroadcastAudioSetMasterRole(Id);
            setA2dpStatusFlag(MEDIA_RECONNECT, Id, FALSE);

            /* update the link policy */
            linkPolicyUseA2dpSettings(DeviceId, StreamId, A2dpSignallingGetSink(DeviceId));

#ifdef ENABLE_PEER
            if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
            {
                peerAdvanceRelayState(RELAY_EVENT_OPENED);

                /* Send the audio enhancement  and user eq settings(if present) to the peer(slave) if this device is a master*/
                peerSendAudioEnhancements();

#if defined ENABLE_GAIA && defined ENABLE_GAIA_PERSISTENT_USER_EQ_BANK
                peerSendUserEqSettings();
#endif
            }
            else
            {
                MessageCancelAll(&theSink.task, EventSysStreamEstablish);
                if(BdaddrIsSame(getA2dpLinkBdAddr(Id), getA2dpAgToReconnect()))
                {
                    BdaddrSetZero(getA2dpAgToReconnect());
                }

                PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE OPENED");
            }
#endif

               playCodecPromptIfNewDevice(&bd_addr,DeviceId,StreamId);

           /* Start the Streaming if if in the suspended state.  Don't issue any AVRCP/AVDTP commands to a Peer device as these are managed separately */
           if ((SinkA2dpGetSuspendState(Id) == a2dp_local_suspended) && (getA2dpPeerRemoteDevice(Id) != remote_device_peer))
           {
#ifdef ENABLE_AVRCP
                /* does the device support AVRCP and is AVRCP currently connected to this device? */
                for_all_avrcp(i)
                {
                    /* ensure media is streaming and the avrcp channel is that requested to be paused */
                    if ((sinkAvrcpIsConnected(i))&&
                        (BdaddrIsSame(getA2dpLinkBdAddr(Id), sinkAvrcpGetLinkBdAddr(i))))
                    {
                        /* attempt to resume playing the a2dp stream */
                        status_avrcp = sinkAvrcpPlayPauseRequest(i,AVRCP_PLAY);
                        A2DP_DEBUG(("Open Success - suspended - avrcp play\n"));
                        break;
                    }
                }
#endif
                /* if not avrcp enabled, use media start instead */
                if(!status_avrcp)
                {
                    A2dpMediaStartRequest(DeviceId, StreamId);
                    A2DP_DEBUG(("Open Success - suspended - start streaming\n"));
                }

                /* reset suspended state once start is sent*/
                a2dpSetSuspendState(Id, a2dp_not_suspended);
            }
        }
    }
    else
    {
        A2DP_DEBUG(("Open Failure [result = %d]\n", status));
#ifdef ENABLE_PEER
        {
            uint16 Id;
            bdaddr bd_addr;

            if(A2dpDeviceGetBdaddr(DeviceId, &bd_addr) && getA2dpIndexFromBdaddr(&bd_addr , &Id))
            {
                if ((getA2dpPeerRemoteDevice(Id) == remote_device_peer))
                {
                    if (status != a2dp_no_signalling_connection)
                    {
                        peerAdvanceRelayState(RELAY_EVENT_NOT_OPENED);
                    }
                }
                else
                {
                    PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE OPEN FAILED");
                }
            }
        }
#endif
    }
}

static void peerHandleA2dpHandleCloseOfRemotePeerDevice(uint16 a2dp_index, a2dp_status_code status)
{
#ifdef ENABLE_PEER
    /* Peer device has closed its media channel, now look to see if AV source is trying to initiate streaming */

    if(status == a2dp_disconnect_link_loss)
    {
        /* Reset the peer features if the media channel has closed due to a linkloss, since we will re-populate this field
           when the peer is recovered from the linkloss and signalling channel connects */
        setA2dpPeerRemoteFeatures(a2dp_index, remote_features_none);
    }

    PEER_UPDATE_REQUIRED_RELAY_STATE("PEER RELAY CLOSE");
    peerAdvanceRelayState(RELAY_EVENT_CLOSED);
#else
    UNUSED(a2dp_index);
    UNUSED(status);
#endif
}

static void peerHandleA2dpHandleCloseOfRemoteNonPeerDevice(void)
{
#ifdef ENABLE_PEER
    /* AV Source closed it's media channel, update the required state */
    PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE CLOSED");
#endif
}

/*************************************************************************
NAME
    handleA2DPClose

DESCRIPTION
    handle the close of a media channel

RETURNS

**************************************************************************/
static void handleA2DPClose(uint8 DeviceId, uint8 StreamId, a2dp_status_code status)
{
#ifdef ENABLE_PEER
    uint8 remote_device_type   = remote_device_unknown;
    bool is_a2dp_index_present = FALSE;
#endif

    /* check the status of the close indication/confirm */
    if((status == a2dp_success) || (status == a2dp_disconnect_link_loss))
    {
        Sink sink = A2dpSignallingGetSink(DeviceId);
        uint16 a2dp_index = 0;
        bdaddr bd_addr;

        A2DP_DEBUG(("A2dp: Close DevId = %d, StreamId = %d status = %u\n",DeviceId,StreamId, status));

#ifdef ENABLE_PEER
        /* In a link loss we may not get a AVRCP command to stop playing,
           so make sure the a2dp stream protection is turned off. */
        is_a2dp_index_present = getA2dpIndex(DeviceId, &a2dp_index);
        remote_device_type = getA2dpPeerRemoteDevice(a2dp_index);

        if ((status == a2dp_disconnect_link_loss)&& (is_a2dp_index_present))
        {
            /* Protection should not be turned off if it is peer master and streaming AG1
               and peer slave goes away. Then peer slave should not be paged by it.*/
            if ((!peerIsThisDeviceTwsMaster() && (remote_device_type == remote_device_peer)) ||
                (peerIsThisDeviceTwsMaster() && (remote_device_type == remote_device_nonpeer)))
            {
            linklossProtectStreaming(linkloss_stream_protection_off);
            /* Re-enable stream protection on the link it is required on. */
            if (!peerLinkRecoveryWhileStreamingEnabled())
            {
                linklossProtectStreaming(linkloss_stream_protection_on);
            }
        }
        }
#endif

        audioUpdateAudioRoutingAfterDisconnect();

        /* update the link policy */
        linkPolicyUseA2dpSettings(DeviceId, StreamId, sink);

        /* change device state if currently in one of the A2DP specific states */
        if(stateManagerGetState() == deviceA2DPStreaming)
        {
            /* the enter connected state function will determine if the signalling
               channel is still open and make the approriate state change */
            stateManagerEnterConnectedState();
        }

        /* user configurable event notification */
        MessageSend(&theSink.task, EventSysA2dpDisconnected, 0);

        if(!isAnyA2dpSourcePlaying())
            sinkA2dpIndicateCodecExit(getA2dpLinkDataSeId(a2dp_index));

        /*As the A2DP media channel has been closed */
        if(A2dpDeviceGetBdaddr(DeviceId, &bd_addr) && getA2dpIndexFromBdaddr(&bd_addr , &a2dp_index))
        {
            /* Reset seid now that media channel has closed */
            setA2dpLinkDataSeId(a2dp_index, 0);

            if (getA2dpPeerRemoteDevice(a2dp_index) == remote_device_peer)
            {
                peerHandleA2dpHandleCloseOfRemotePeerDevice(a2dp_index, status);
            }
            else if (getA2dpPeerRemoteDevice(a2dp_index) == remote_device_nonpeer)
            {
                peerHandleA2dpHandleCloseOfRemoteNonPeerDevice();

                /* Cancel any pending media channel establishment timers */
                MessageCancelAll(&theSink.task, EventSysStreamEstablish);

                if (getA2dpStatusFlag(MEDIA_RECONNECT, a2dp_index))
                {   /* Disconnect signalling channel to AV Source (for IOP reasons) */
                    /* This call complements reconnectAvSource() */
                    A2dpSignallingDisconnectRequest(getA2dpLinkDataDeviceId(a2dp_index));
                }
            }

#ifdef ENABLE_AVRCP
            if(avrcpAvrcpIsEnabled())
            {
                /* assume device is stopped for AVRCP 1.0 devices */
                sinkAvrcpSetPlayStatusbyAddress(getA2dpLinkBdAddr(a2dp_index), avrcp_play_status_stopped);
            }
#endif
        }
    }
    else
    {
        A2DP_DEBUG(("A2dp: Close FAILED status = %d\n",status));
    }
}

/*************************************************************************
NAME
    handleA2DPSignallingDisconnected

DESCRIPTION
    handle the disconnection of the signalling channel
RETURNS

**************************************************************************/
void handleA2DPSignallingDisconnected(uint8 DeviceId, a2dp_status_code status,  bdaddr SrcAddr)
{
    uint16 Id;
    bool reconnect = FALSE;
#ifdef ENABLE_PEER
    bool peer_disconnected = FALSE;
#endif

#ifndef ENABLE_PEER
    UNUSED(status);
#endif

    A2DP_DEBUG(("A2DP: SigDiscon DevId = %d status %u SrcAddr [%x:%x:%lx]\n",
        DeviceId, status, SrcAddr.uap, SrcAddr.nap, SrcAddr.lap));

    /* If the connection event sent a delayed connection message, cancel it now to stop it arriving */
    if (A2DP_DATA.ui_connect_update_delay>0)
    {
        MessageCancelFirst(&theSink.task, EventSysA2dpConnected);
    }

    /* Cover LinkLoss event case first and cancel any link loss reminders.
    * This takes place here since the check below using getA2dpIndex will
    * return FALSE because device is not connected.
    * Make sure this is not an HFP profile (A2DP instead). */
    if(!sinkHfpDataGetSupportedProfile())
    {
        linklossCancelLinkLossTone();
    }

    /* check for successful disconnection status */
    if(getA2dpIndex(DeviceId, &Id))
    {
#ifdef ENABLE_PARTYMODE
        {
            /* check whether party mode is enabled */
            if(sinkPartymodeIsEnabled() && sinkPartymodeGetOperatingMode())
            {
                /* device disconnected, cancel its play music timeout */
                MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1 + Id));
                sinkUpdatePartyModeStateChangeEvent(EventSysPartyModeDisconnected);
            }
        }
#endif

        /* If the device is using a2dp only, i.e. no hfp, then we have to
           manage the link loss event here instead of leaving it to the
           hfp library. */
        if (!(conn_hfp & deviceManagerProfilesConnected(&SrcAddr)))
        {
            /* Cancel any repeating link loss tone */
            linklossCancelLinkLossTone();

#ifdef ENABLE_PEER
            /* If stream protection is on we do not get a A2DP_SIGNALLING_LINKLOSS_IND
               from the a2dp lib, so generate an EventSysLinkLoss here instead. */
            if(status == a2dp_disconnect_link_loss && linklossIsStreamProtected(&SrcAddr))
            {
                linklossSendLinkLossTone(getA2dpLinkBdAddr(Id), 0);
            }
#endif
        }

#ifdef ENABLE_PEER

        /* Update the peer device attribnutes if the disconnection is due to a linkloss */
        if(status == a2dp_disconnect_link_loss)
        {
            sink_attributes attributes;
            getKnownPeerDeviceAttributes(Id , &attributes);
        }

        if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
        {   /* A Peer device */

            if (peerObtainPairingMode(Id) == PEER_PAIRING_MODE_TEMPORARY)
            {
                /* Check if we need to wait for some-time, before removing peer device from DM, to allow peer device to connect */
                bdaddr *message = PanicUnlessNew(bdaddr);
                *message = *getA2dpLinkBdAddr(Id);
                setA2dpPeerLinkLossReconnect(FALSE);
                if (linklossIsStreamProtected(getA2dpLinkBdAddr(Id)) && (status == a2dp_disconnect_link_loss) && getA2dpLinkLossReconnectTimeout())
                {
                    /* Start timer to allow remote peer to reconnect before deleting the temporary pairing */
                    A2DP_DEBUG(("A2dp: Starting a link loss timer\n"));
                    setA2dpPeerLinkLossReconnect(TRUE);
                    MessageCancelFirst(&theSink.task , EventSysA2DPPeerLinkLossTimeout);
                    MessageSendLater( &theSink.task , EventSysA2DPPeerLinkLossTimeout , 0, D_SEC(getA2dpLinkLossReconnectTimeout()) );
                }
                MessageCancelFirst(&theSink.task , EventSysRemovePeerTempPairing);
                MessageSendConditionally( &theSink.task, EventSysRemovePeerTempPairing, message, getA2dpPeerLinkLossReconnectPtr());
            }

            /* Store knowledge of Peer disconnecting to use later once a2dp parameters reset */
            peer_disconnected = TRUE;

            BdaddrSetZero(getA2dpAgToReconnect());

            /* As the peer has disconnected, reset the remote_peer_ag_bd_addr*/
            BdaddrSetZero(peerGetRemotePeerAgBdAddress());
            peerSetRemotePeerAudioConnectionStatus(0);

            /* Store the attributes in PS */
            deviceManagerUpdateAttributes(&SrcAddr, sink_a2dp, 0, (a2dp_index_t)Id);
            
            /* reset any low battery condition reported by the peer slave device */
            powerManagerSetPeerSlaveLowBat(FALSE);

        }
        else
#endif
        {   /* Not a Peer device */

#ifdef ENABLE_PEER
            /*If the A2DP has disconnected from a non-peer device then notify this to the peer device */
            sinkAvrcpUpdatePeerSourceDisconnected(A2DP_AUDIO);
#endif

            if (getA2dpStatusFlag(MEDIA_RECONNECT, Id))
            {   /* A reconnect of signalling and media channel has been requested, due to AAC not being supported by ShareMe */
                reconnect = TRUE;
            }

            /* Store the attributes in PS */
            deviceManagerUpdateAttributes(&SrcAddr, sink_a2dp, 0, (a2dp_index_t)Id);
        }

        /* Reset the a2dp parameter values */
        setA2dpPeerRemoteDevice(Id, remote_device_unknown);
        setA2dpPeerRemoteFeatures(Id, 0);
        if(!(getA2dpPeerStatus(Id, LOCAL_PEER) & PEER_STATUS_POWER_OFF))
        {
            setA2dpPeerStatus(Id, LOCAL_PEER, 0);
        }
        setA2dpPeerStatus(Id, REMOTE_PEER, 0);
        setA2dpStatusFlag(MEDIA_RECONNECT, Id, FALSE);
        BdaddrSetZero(getA2dpLinkBdAddr(Id));
        setA2dpStatusFlag(CONNECTED, Id, FALSE);
        a2dpSetSuspendState(Id, a2dp_not_suspended);
        setA2dpLinkDataDeviceId(Id, INVALID_DEVICE_ID);
        setA2dpLinkDataStreamId(Id, INVALID_STREAM_ID);
        setA2dpPdlListId(Id, INVALID_LIST_ID);
        setA2dpLinkDataSeId(Id, 0);
        setA2dpStatusFlag(PLAYING, Id, FALSE);
#ifdef ENABLE_AVRCP
        setAgAvrcpSupportStatus(Id, avrcp_support_unknown);
#endif

        /* Sends the indication to the device manager to send an event out if a device has disconnected*/
        deviceManagerDeviceDisconnectedInd(&SrcAddr);

        if(status == a2dp_disconnect_link_loss)
        {
           MessageSend(&theSink.task, EventSysReconnectFailed, 0);
        }
        /*if the device is off then this is disconnect as part of the power off cycle, otherwise check
          whether device needs to be made connectable */
#ifdef ENABLE_PEER
        /* also account for this being part of a Single Device Mode power off */
        if ( (stateManagerGetState() != deviceLimbo)
            && !(getA2dpPeerStatus(Id, LOCAL_PEER) & PEER_STATUS_POWER_OFF) )
#else
        if ( stateManagerGetState() != deviceLimbo)
#endif
        {
            /* Kick role checking now a device has disconnected */
            linkPolicyCheckRoles();

            /* at least one device disconnected, re-enable connectable for another 60 seconds */
            sinkEnableMultipointConnectable();

            /* if the device state still shows connected and there are no profiles currently
               connected then update the device state to reflect the change of connections */
            if ((stateManagerIsConnected()) && (!deviceManagerNumConnectedDevs()))
            {
                stateManagerEnterConnectableState( FALSE ) ;
            }
            else
            {
                /* Update the extended state LED notification when there is a disconnection on A2DP
                   profile */
                updateExtendedState( stateManagerGetState() );
            }
        }

#ifdef ENABLE_PEER
        /* A Peer/Source disconnecting will/may affect relay state */
        if (peer_disconnected)
        {
            restoreServiceCapsSupportedWhenExitingTwsModeIfNeededReconnectAvSource();

            /* Peer signalling channel has gone so media channel has also.  Let state machine know */
            peerAdvanceRelayState(RELAY_EVENT_DISCONNECTED);

            if(getA2dpPeerStatus(Id, LOCAL_PEER) & PEER_STATUS_POWER_OFF)
            {
                MessageSend(&theSink.task, EventUsrPowerOff, 0);
            }
        }
        else
        {
            PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE DISCONNECTED");
        }
#endif

#ifdef ENABLE_AVRCP
        if(avrcpAvrcpIsEnabled())
        {
            sinkAvrcpDisconnect(&SrcAddr);
        }
#endif

        if (reconnect)
        {
            /* Kick off reconnect now */
            sinkA2dpSignallingConnectRequest(&SrcAddr);
        }
    }
    else
        A2DP_DEBUG(("A2dp: Sig Discon FAILED status = %d\n",status));

    /* update the number and type of devices connected */
    MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
}

/*************************************************************************
NAME
    handleA2DPLinklossReconnectCancel

DESCRIPTION
    handle the indication of a link loss reconnection cancel
RETURNS

**************************************************************************/
void handleA2DPLinklossReconnectCancel(uint8 DeviceId)
{
    A2DP_DEBUG(("A2dp: handleA2DPLinklossReconnectCancel DevId = %d\n",DeviceId));

    UNUSED(DeviceId); /* Used in debug only */

    /* on receiving the linkloss reconnection timeout cancelled, stop the tone reminder */
    linklossCancelLinkLossTone();
}

/*************************************************************************
NAME
    handleA2DPSignallingLinkloss

DESCRIPTION
    handle the indication of a link loss
RETURNS

**************************************************************************/
void handleA2DPSignallingLinkloss(uint8 DeviceId)
{
    uint16 Id;
    A2DP_DEBUG(("A2dp: handleA2DPSignallingLinkloss DevId = %d\n",DeviceId));

    if (getA2dpIndex(DeviceId, &Id))
    {
        /* Kick role checking now a device has disconnected */
        linkPolicyCheckRoles();

       if(getA2dpLinkLossReconnectTimeout() == 0)
       {
           setA2dpStatusFlag(CONNECTED, Id, FALSE);
       }

#ifdef ENABLE_PEER
        /* A Peer/Source disconnecting will/may affect relay state */
        if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
        {   /* Peer signalling channel has gone so media channel has also.  Let state machine know */
            PEER_UPDATE_REQUIRED_RELAY_STATE("PEER LINKLOSS");
            peerAdvanceRelayState(RELAY_EVENT_CLOSED);
        }
        else if (getA2dpPeerRemoteDevice(Id) == remote_device_nonpeer)
        {
            PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE LINKLOSS");
        }
#endif
        audioUpdateAudioRouting();

        if(sinkDataGoConnectableDuringLinkLoss() || (getA2dpPeerRemoteDevice(Id) == remote_device_peer))
        {   /* Go connectable if feature enabled or remote is a peer device */
            sinkEnableConnectable();
            MessageCancelAll(&theSink.task, EventSysConnectableTimeout);   /* Ensure connectable mode does not get turned off */
        }

        /* Send an event to notify the user of link loss */
        linklossCancelLinkLossTone();
        linklossSendLinkLossTone(getA2dpLinkBdAddr(Id), 0);
    }

    /* update the number and type of devices connected */
    MessageSend(&theSink.task, EventSysUpdateDevicesConnectedStatus, NULL);
}

/*************************************************************************
NAME
    handleA2DPStartInd

DESCRIPTION
    handle the indication of media start ind
RETURNS

**************************************************************************/
void handleA2DPStartInd(uint8 DeviceId, uint8 StreamId)
{
    if (sinkA2dpIsStreamingAllowed())
    {
#ifdef ENABLE_PEER
        uint16 Id;
        bdaddr bd_addr;

        /*Get the A2DP index from the BD Address corresponding to the DeviceId */
        if(A2dpDeviceGetBdaddr(DeviceId, &bd_addr) && getA2dpIndexFromBdaddr(&bd_addr , &Id))
        {
            /* update the link policy */
            linkPolicyUseA2dpSettings(DeviceId, StreamId, A2dpSignallingGetSink(DeviceId));

            /* Ensure suspend state is cleared now streaming has started/resumed */
            a2dpSetSuspendState(Id, a2dp_not_suspended);

            if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
            {   /* Just accept media stream from peer device */
                peerAdvanceRelayState(RELAY_EVENT_STARTING);
                A2dpMediaStartResponse(DeviceId, StreamId, TRUE);
            }
            else
            {
                /* Open ind from true AV source */
                disconnectScoIfConnected(bd_addr);
                peerClaimRelay(TRUE);
                PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE STARTING");
                if ( !peerIsRelayAvailable() || !peerCheckSource(RELAY_SOURCE_A2DP) )
                {   /* Either no peer device connected or we have a peer that has control of the relay channel. In either case just accept the media stream */
                    A2DP_DEBUG(("Relay channel NOT available\n"));
                    A2DP_DEBUG(("Send start resp to AV Source\n"));
                    A2dpMediaStartResponse(DeviceId, StreamId, TRUE);
                }
                else /* A timer is set to send a response directly to the Source if Peer device is taking too much time to respond */
                {
                    A2DP_MEDIA_START_IND_T* message = PanicUnlessNew(A2DP_MEDIA_START_IND_T) ;
                    message->device_id = DeviceId ;
                    message->stream_id = StreamId ;
                    MessageSendLater(&theSink.task, EventSysPeerStreamTimeout, message, PEER_STREAMING_TIMEOUT) ;
                }
            }
        }
#else /* ENABLE_PEER */

        /* Always accept the media stream */
        A2dpMediaStartResponse(DeviceId, StreamId, TRUE);

#ifdef ENABLE_PARTYMODE
        {
            uint16 Id;

            /* check whether party mode is enabled */
            if((getA2dpIndex(DeviceId, &Id)) && sinkPartymodeIsEnabled() && sinkPartymodeGetOperatingMode())
            {
                /* device streaming, cancel its play music timeout */
                MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1 + Id));
            }
        }
#endif  /* ENABLE_PARTYMODE */

#endif /* ENABLE_PEER */
    }
    else
    {
        A2dpMediaStartResponse(DeviceId, StreamId, FALSE);
    }
}

/*************************************************************************
NAME
    handleA2DPStartStreaming

DESCRIPTION
    handle the indication of media start cfm
RETURNS

**************************************************************************/
void handleA2DPStartStreaming(uint8 DeviceId, uint8 StreamId, a2dp_status_code status)
{
    /* check success status of indication or confirm */
    if(status == a2dp_success)
    {
        uint16 Id;
        Sink sink = A2dpMediaGetSink(DeviceId, StreamId);

        A2DP_DEBUG(("A2dp: StartStreaming DevId = %d, StreamId = %d\n",DeviceId,StreamId));
        /* find structure index of deviceId */
        if(getA2dpIndex(DeviceId, &Id))
        {

#ifdef ENABLE_PARTYMODE
            /* check whether party mode is enabled */
            if(sinkPartymodeIsEnabled() && sinkPartymodeGetOperatingMode())
            {
                /* device streaming, cancel its play music timeout */
                MessageCancelAll(&theSink.task,(EventSysPartyModeTimeoutDevice1 + Id));
            }
#endif
            /* Ensure suspend state is cleared now streaming has started/resumed */
            a2dpSetSuspendState(Id, a2dp_not_suspended);

            /*Ensure that the device is not currently streaming from a different A2DP, if its found to be streaming then pause this incoming stream  */
#ifdef ENABLE_AVRCP
            a2dpPauseNonRoutedSource(Id);
#endif

            /* route the audio using the appropriate codec/plugin */
            audioUpdateAudioRouting();

            /* enter the stream a2dp state if not in a call */
            stateManagerEnterA2dpStreamingState();

            /* update the link policy */
            linkPolicyUseA2dpSettings(DeviceId, StreamId, sink);

            /* set the current seid */
            setA2dpLinkDataStreamId(Id, StreamId);

#ifdef ENABLE_PEER
            if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
            {   /* Peer media channel has started */
                peerAdvanceRelayState(RELAY_EVENT_STARTED);
                /* Clear the protection on the peer link in case it was
                   enabled when not playing audio. */
                linklossResetStreamProtection(Id);
                linklossUpdateManagement(getA2dpLinkBdAddr(Id));

                /* Re-enable stream protection on the link it is required on. */
                if (!peerLinkRecoveryWhileStreamingEnabled())
                {
                    linklossProtectStreaming(linkloss_stream_protection_on);
                }
            }
            else
            {
                PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE STARTED");
            }
#endif

#ifdef ENABLE_AVRCP
            if(avrcpAvrcpIsEnabled())
            {
                /* assume device is playing for AVRCP 1.0 devices */
                sinkAvrcpSetPlayStatusbyAddress(getA2dpLinkBdAddr(Id), avrcp_play_status_playing);
            }
#endif

#ifdef ENABLE_PEER
            /* Only update EQ mode for non peer devices i.e. for streams to be rendered locally */
            if (getA2dpPeerRemoteDevice(Id) != remote_device_peer)
#endif
            {
                /* Set the Stored EQ mode, ensure the DSP is currently streaming A2DP data before trying to
               set EQ mode as it might be that the device has a SCO routed instead */
                if(sinkAudioGetRoutedAudioSink() == sink)
                {
                    A2DP_DEBUG(("A2dp: StartStreaming Set EQ mode = %d\n", getMusicProcessingMode()));

                    /* set both EQ and Enhancements enables */
                    AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());

                    /* update the Current EQ enable status since its setting is now dictated by the
                       master peer device */
                    MessageSend(&theSink.task, EventSysEQRefreshStatus, 0);

                    AudioSetMusicProcessingEnhancements (getMusicProcessingMode(), getMusicProcessingBypassFlags());
                }
                else
                {
                    A2DP_DEBUG(("A2dp: Wrong sink Don't Set EQ mode = %d\n", getMusicProcessingMode()));
                }
            }
            /* when using the Soundbar manual audio routing and subwoofer support,
               check to see if the a2dp audio is being routed, if not check whether
               an esco subwoofer channel is currently in use, if this is the case it
               will be necessary to pause this a2dp stream to prevent disruption
               of the subwoofer esco link due to link bandwidth limitations */
#ifdef ENABLE_SUBWOOFER
            suspendWhenSubwooferStreamingLowLatency(Id);
#endif
        }
    }
    else
    {
        A2DP_DEBUG(("A2dp: StartStreaming FAILED status = %d\n",status));
#ifdef ENABLE_PEER
        {
            uint16 Id;

            if (getA2dpIndex(DeviceId, &Id))
            {
                if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
                {   /* Peer has rejected start of media channel, need to respond to any outstanding request from AV source */
                    peerAdvanceRelayState(RELAY_EVENT_NOT_STARTED);
                }
                else
                {
                    PEER_UPDATE_REQUIRED_RELAY_STATE("A2DP SOURCE START FAILED");
                }
            }
        }
#endif
    }

}

static void updateStateOnA2dpSupend(uint16 Id)
{
    if(stateManagerGetState() == deviceA2DPStreaming)
    {
        /* the enter connected state function will determine if the signalling
           channel is still open and make the approriate state change
        */
        stateManagerEnterConnectedState();
        sinkPartyModeUpdateOnSuspend(Id);
    }
}

static void updateLocalSuspendedState(uint16 Id)
{
    a2dpSetSuspendState(Id, a2dp_local_suspended);
    audioUpdateAudioRoutingAfterDisconnect();
}

static void updateRemoteSuspendedState(uint16 Id)
{
    /* Ensure suspend state is set as streaming has now ceased */
    if ( sinkAvrcpIsAvrcpAudioSwitchingEnabled() )
    {
        a2dpSetSuspendState(Id, a2dp_remote_suspended);
    }
    else
    {
        setA2dpStatusFlag(PLAYING, Id, TRUE);
        a2dpSetPlayingState(Id, FALSE);
    }
    audioUpdateAudioRoutingAfterDisconnect();
}

static void updateLinkStatesFollowingSuspend(uint8 DeviceId, uint8 StreamId, uint16 Id)
{
    Sink sink = A2dpMediaGetSink(DeviceId, StreamId);

    linkPolicyUseA2dpSettings(DeviceId, StreamId, sink);
    peerUpdateRelayStateOnSuspend(Id);
    sinkAvrcpSetPlayingStateToPausedOnSuspend(Id);

#ifdef ENABLE_PEER
     A2DP_DEBUG(("A2DP: handleA2DPSuspendStreaming - cancel pause-suspend timeout a2dp_idx %u\n", Id));
     a2dpCancelPauseSuspendTimeout((a2dp_index_t)Id);
#endif
}

/*************************************************************************
NAME
    handleA2DPSuspendStreamingInd

DESCRIPTION
    handle the indication of media suspend ind
RETURNS

**************************************************************************/
static void handleA2DPSuspendStreamingInd(uint8 DeviceId, uint8 StreamId)
{
    uint16 Id;

    A2DP_DEBUG(("A2dp: Suspend IND DevId = %d, StreamId = %d\n",DeviceId,StreamId));

    /*Restart APD timer*/
    sinkStartAutoPowerOffTimer();

    if(getA2dpIndex(DeviceId, &Id))
    {
        updateStateOnA2dpSupend(Id);
        updateRemoteSuspendedState(Id);
        updateLinkStatesFollowingSuspend(DeviceId, StreamId, Id);
    }
}

/*************************************************************************
NAME
    handleA2DPSuspendStreamingCfm

DESCRIPTION
    handle the indication of media suspend cfm
RETURNS

**************************************************************************/
static void handleA2DPSuspendStreamingCfm(uint8 DeviceId, uint8 StreamId, a2dp_status_code status)
{
    A2DP_DEBUG(("A2dp: Suspend CFM DevId = %d, StreamId = %d\n",DeviceId,StreamId));

    /*Restart APD timer*/
    sinkStartAutoPowerOffTimer();

    if(status == a2dp_rejected_by_remote_device)
    {
        A2DP_DEBUG(("A2dp: Suspend Failed= %x, try close DevId = %d, StreamId = %d\n",status,DeviceId,StreamId));
        A2dpMediaCloseRequest(DeviceId, StreamId);
    }
    else
    {
        uint16 Id;
        if(getA2dpIndex(DeviceId, &Id))
        {
            updateStateOnA2dpSupend(Id);
            updateLocalSuspendedState(Id);
            updateLinkStatesFollowingSuspend(DeviceId, StreamId, Id);
        }
    }
}

void sinkA2dpSuspendAll(void)
{
    suspendAllA2dpStreams(FALSE);
}

void sinkA2dpSuspendAllNoAutomaticResume(void)
{
    suspendAllA2dpStreams(TRUE);
}

/*************************************************************************
NAME
    sinkA2dpResumePeerSession

DESCRIPTION
    Used to resume peer session

RETURNS
    None
**************************************************************************/

#ifdef ENABLE_PEER
void sinkA2dpResumePeerSession(void)
{
    uint8 index = 0;
    for(index = 0; index < MAX_A2DP_CONNECTIONS; index++)
    {
        if(getA2dpPeerRemoteDevice(index) == remote_device_peer)
        {
            if(SinkA2dpGetSuspendState(index) == a2dp_not_suspended)
            {
                if (a2dpIssuePeerOpenRequest())
                {
                    updateCurrentState( RELAY_STATE_OPENING );
                }
            }
            else if(SinkA2dpGetSuspendState(index) == a2dp_local_suspended)
            {
                a2dpIssuePeerStartRequest();
            }
        }
    }
}
#endif

/*************************************************************************
NAME
    SuspendA2dpStream

DESCRIPTION
    called when it is necessary to suspend an a2dp media stream due to
    having to process a call from a different AG. If the device supports
    AVRCP then issue a 'pause' which is far more reliable than trying a
    media_suspend request.

RETURNS

**************************************************************************/
void SuspendA2dpStream(a2dp_index_t index)
{
    if(a2dpIndexIsValidSource(index) && isA2dpStreamSuspendable(index))
    {
        A2DP_DEBUG(("A2dp: Suspend A2DP Stream\n"));
        suspendA2dpStream(index, TRUE);
    }
}

#ifdef ENABLE_PEER
/*************************************************************************
NAME
     a2dpSuspendNonRoutedStream

DESCRIPTION
    Special-case of sending an A2DP suspend to an AG when in TWS extended
    mode (an AG connected to each peer).
    The intention is to force the AG to suspend its A2DP stream so that it
    does not intefere with the A2DP stream from the peer device.

    It is a workaround for a known bluetooth bandwidth issue when
    handling > 1 A2DP stream in the 'streaming' state.

RETURNS
    none

**************************************************************************/
void  a2dpSuspendNonRoutedStream(a2dp_index_t index)
{
    uint16 peer_index;

    if (a2dpGetPeerIndex(&peer_index)
        && !a2dpIsIndexPeer(index)
        && !getA2dpStatusFlag(PLAYING, index))
    {
        A2DP_DEBUG(("A2dp:  a2dpSuspendNonRoutedStream\n"));

        /* set the local suspend status indicator */
        a2dpSetSuspendState(index, a2dp_local_suspended);

        /* attempt to suspend stream if avrcp pause was not successful, if not successful then close it */
        A2DP_DEBUG(("A2DP:   Sending suspend\n"));
        if(!A2dpMediaSuspendRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)))
        {
            A2DP_DEBUG(("A2DP:   Sending suspend failed\n"));
        }

        /* no longer streaming so enter connected state if applicable */
        if(stateManagerGetState() == deviceA2DPStreaming)
        {
            /* the enter connected state function will determine if the signalling
                        channel is still open and make the approriate state change */
            stateManagerEnterConnectedState();
        }
    }
}
#endif


a2dp_suspend_state sinkA2dpGetA2dpLinkSuspendState(a2dp_index_t index)
{
    return (SinkA2dpIsInitialised() ? SinkA2dpGetSuspendState(index) : a2dp_not_suspended);
}


bool sinkA2dpIsA2dpLinkSuspended(a2dp_index_t index)
{
    return (sinkA2dpGetA2dpLinkSuspendState(index) != a2dp_not_suspended);
}

#ifdef ENABLE_AVRCP
/*************************************************************************
NAME
    resumeUsingAvrcpPlay

DESCRIPTION
    Helper to resume a2dp streaming using AVRCP play
RETURNS
    TRUE if streaming resumes, otherwise FALSE
**************************************************************************/
static bool resumeUsingAvrcpPlay(a2dp_index_t index)
{
    bool status_avrcp = FALSE;
    uint16 avrcp_index;

    if (A2DP_DATA.paused_using_avrcp[index])
    {
        for_all_avrcp(avrcp_index)
        {
            if ((sinkAvrcpIsConnected(avrcp_index))&&(getA2dpStatusFlag(CONNECTED, index)) &&
                (BdaddrIsSame(getA2dpLinkBdAddr(index), sinkAvrcpGetLinkBdAddr(avrcp_index))))
            {
                /* attempt to resume playing the a2dp stream */
                status_avrcp = sinkAvrcpPlayPauseRequest(avrcp_index,AVRCP_PLAY);

                if(status_avrcp)
                {
                    sinkAvrcpSetPlayStatusbyIndex(index, avrcp_play_status_playing);
                    A2DP_DATA.paused_using_avrcp[index] = FALSE;
                }
                break;
            }
        }
    }

    return status_avrcp;
}
#endif

/*************************************************************************
NAME
    ResumeA2dpStream

DESCRIPTION
    Called to resume a suspended A2DP stream
RETURNS

**************************************************************************/
bool ResumeA2dpStream(a2dp_index_t index)
{
    bool ready_to_connect = FALSE;

    if (SinkA2dpIsInitialised() && (getA2dpPeerRemoteDevice(index) != remote_device_peer)
         && sinkA2dpIsStreamingAllowed())
    {
        A2DP_DEBUG(("A2dp: ResumeA2dpStream\n" )) ;

        /* need to check whether the signalling channel hsa been dropped by the AV/AG source */
        if(A2dpSignallingGetState(getA2dpLinkDataDeviceId(index)) == a2dp_signalling_connected)
        {
            a2dp_stream_state state = sinkA2dpGetStreamState(index);

            /* is media channel still open? or is it streaming already? */
            if(state == a2dp_stream_open)
            {
#ifdef ENABLE_AVRCP
                if(resumeUsingAvrcpPlay(index))
                {
                    /* update state */
                    a2dpSetSuspendState(index, a2dp_not_suspended);
                }
                else
#endif
                /* if not successful in resuming play via avrcp try a media start instead */
                {
                    A2DP_DEBUG(("A2dp: Media Start\n" )) ;
                    A2dpMediaStartRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
                    /* reset the SuspendState indicator */
                    a2dpSetSuspendState(index, a2dp_not_suspended);
                }
            }
            /* media channel wasn't open, source not supporting suspend */
            else if(state < a2dp_stream_open)
            {
                A2DP_DEBUG(("A2dp: Media Open\n" )) ;
                connectA2dpStream( index, 0 );
            }
            /* recovery if media has resumed streaming reconnect its audio */
            else if(state == a2dp_stream_streaming)
            {
#ifdef ENABLE_AVRCP
                resumeUsingAvrcpPlay(index);
#endif
                a2dpSetSuspendState(index, a2dp_not_suspended);
                ready_to_connect = TRUE;
            }
        }
        /* signalling channel is no longer present so attempt to reconnect it */
        else
        {
            A2DP_DEBUG(("A2dp: Connect Signalling\n" )) ;
            sinkA2dpSignallingConnectRequest(getA2dpLinkBdAddr(index));
        }
    }

    return ready_to_connect;
}

/*************************************************************************
NAME
    handleA2DPStoreClockMismatchRate

DESCRIPTION
    handle storing the clock mismatch rate for the active stream
RETURNS

**************************************************************************/
void handleA2DPStoreClockMismatchRate(uint16 clockMismatchRate)
{
    a2dp_stream_state a2dpStatePri = a2dp_stream_idle;
    a2dp_stream_state a2dpStateSec = a2dp_stream_idle;
    Sink a2dpSinkPri = 0;
    Sink a2dpSinkSec = 0;

    /* if a2dp connected obtain the current streaming state for primary a2dp connection */
    getA2dpStreamData(a2dp_primary, &a2dpSinkPri, &a2dpStatePri);

    /* if a2dp connected obtain the current streaming state for secondary a2dp connection */
    getA2dpStreamData(a2dp_secondary, &a2dpSinkSec, &a2dpStateSec);

    /* Determine which a2dp source this is for */
    if((a2dpStatePri == a2dp_stream_streaming) && (a2dpSinkPri == sinkAudioGetRoutedAudioSink()))
    {
        A2DP_DEBUG(("A2dp: store pri. clk mismatch = %x\n", clockMismatchRate));
        setA2dpLinkDataClockMismatchRate(a2dp_primary, clockMismatchRate);
    }
    else if((a2dpStateSec == a2dp_stream_streaming) && (a2dpSinkSec == sinkAudioGetRoutedAudioSink()))
    {
        A2DP_DEBUG(("A2dp: store sec. clk mismatch = %x\n", clockMismatchRate));
        setA2dpLinkDataClockMismatchRate(a2dp_secondary, clockMismatchRate);
    }
    else
    {
        A2DP_DEBUG(("A2dp: ERROR NO A2DP STREAM, clk mismatch = %x\n", clockMismatchRate));
    }
}

/*************************************************************************
 NAME
    handleA2DPUserEqBankUpdate

DESCRIPTION
    Handle notification from an audio plugin for DSP ready for data message in order to update user defined EQ bank
    when GAIA set EQ parameter commands are processed

RETURNS
**************************************************************************/
void handleA2DPUserEqBankUpdate(void)
{
    user_eq_bank_t *PEQ = NULL;

    PEQ = sinkAudioGetPEQ();

    if(PEQ && (PEQ->bands[0].Q != 0))
    {
        uint16 index = 0;
        eq_param_type_t param_type = 0;
        audio_plugin_user_eq_param_t param;

        param.id.bank = 1;
        param.id.band = 0;
        param.id.param_type = 0;
        param.value = MAX_EQ_BANDS; /* Number of Banks */
        AudioSetUserEqParameter(sinkAudioGetRoutedAudioTask(), &param);

        param.id.param_type = 1;
        param.value = PEQ->preGain;  /* Master Gain */
        AudioSetUserEqParameter(sinkAudioGetRoutedAudioTask(), &param);

        for(index = 0; index < MAX_EQ_BANDS; index++)
        {
            param.id.bank = 1;
            param.id.band = index + 1;
            for (param_type = 0; param_type < eq_param_max_param_types; param_type++)
            {
                param.id.param_type = param_type;
                switch (param_type)
                {
                    case eq_param_type_filter:
                        param.value = PEQ->bands[index].filter;
                        break;

                    case eq_param_type_cutoff_freq:
                        param.value = PEQ->bands[index].freq;
                        break;

                    case eq_param_type_gain:
                        param.value = PEQ->bands[index].gain;
                        break;

                    case eq_param_type_q:
                        param.value = PEQ->bands[index].Q;
                        break;

                    default:
                        break;
                }
                AudioSetUserEqParameter(sinkAudioGetRoutedAudioTask(), &param);
            }
        }
        AudioApplyUserEqParameters(sinkAudioGetRoutedAudioTask(), TRUE);
    }
}

/*************************************************************************
NAME
    getA2dpIndexFromBdaddr

DESCRIPTION
    Attempts to find a A2DP link data index based on the supplied bdaddr.

RETURNS
    TRUE if successful, FALSE otherwise

**************************************************************************/
bool getA2dpIndexFromBdaddr (const bdaddr *bd_addr, uint16 *index)
{
    /* go through A2dp connection looking for match */
    if (SinkA2dpIsInitialised())
    {
        for_all_a2dp(*index)
        {
            if (BdaddrIsSame(getA2dpLinkBdAddr(*index), bd_addr))
            {
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */
    return FALSE;
}

/*************************************************************************
NAME
    disconnectAllA2dpAVRCP

DESCRIPTION
    disconnect any a2dp and avrcp connections

RETURNS

**************************************************************************/
void disconnectAllA2dpAvrcp (bool disconnect_peer)
{
    uint8 i;

#ifdef ENABLE_AVRCP
    if(avrcpAvrcpIsEnabled())
    {
        sinkAvrcpDisconnectAll(disconnect_peer);
    }
#endif
    if (SinkA2dpIsInitialised())
    {
    /* disconnect any a2dp signalling channels */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected, disconnect it */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            if (((getA2dpPeerRemoteDevice(i)!=remote_device_peer) || disconnect_peer))
            {
                A2dpSignallingDisconnectRequest(getA2dpLinkDataDeviceId(i));
                }
            }
        }
    }
}

/*************************************************************************
NAME
    disconnectA2dpAvrcpFromDevice

DESCRIPTION
    disconnect A2DP and AVRCP connections with the device provided.

RETURNS

**************************************************************************/
void disconnectA2dpAvrcpFromDevice(const bdaddr *bdaddr_non_gaia_device)
{
    uint16 index = 0;

#ifdef ENABLE_AVRCP
    if(avrcpAvrcpIsEnabled())
    {
        sinkAvrcpDisconnect(bdaddr_non_gaia_device);
    }
#endif

    if(getA2dpIndexFromBdaddr(bdaddr_non_gaia_device, &index))
    {
        A2dpSignallingDisconnectRequest(getA2dpLinkDataDeviceId(index));
    }
}


/*************************************************************************
NAME
    a2dpGetNextAvBdAddress

DESCRIPTION
    Returns the BD address of the other connected non-peer device, if any.

RETURNS
    TRUE if successful, FALSE otherwise
**************************************************************************/
bool a2dpGetNextAvBdAddress(const bdaddr *bd_addr , bdaddr *next_bdaddr )
{
    uint16 index;
    bdaddr *a2dpBdAddr;

    if(getA2dpIndexFromBdaddr (bd_addr, &index))
    {
        a2dp_index_t other_device_index =
            !BdaddrIsSame(getA2dpLinkBdAddr(a2dp_primary),
            getA2dpLinkBdAddr(index)) ? a2dp_primary : a2dp_secondary;

        if(next_bdaddr && getA2dpStatusFlag(CONNECTED, other_device_index) &&
           (getA2dpPeerRemoteDevice(other_device_index) != remote_device_peer))
        {
             a2dpBdAddr = getA2dpLinkBdAddr(other_device_index);
             *next_bdaddr = *a2dpBdAddr;

            if(!BdaddrIsSame(bd_addr , next_bdaddr))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}


/*************************************************************************
NAME
    disconnectAllA2dpPeerDevices

DESCRIPTION
    Disconnect any a2dp connections to any peer devices

RETURNS
    TRUE is any peer devices disconnected, FALSE otherwise

**************************************************************************/
bool disconnectAllA2dpPeerDevices (void)
{
    uint8 i;
    bool disc_req = FALSE;

    if (SinkA2dpIsInitialised())
    {
        /* disconnect any a2dp signalling channels to peer devices */
        for_all_a2dp(i)
        {
            /* if the a2dp link is connected, disconnect it */
            if ((getA2dpStatusFlag(CONNECTED, i)) && (getA2dpPeerRemoteDevice(i)==remote_device_peer))
            {
                A2dpSignallingDisconnectRequest(getA2dpLinkDataDeviceId(i));
                disc_req = TRUE;
            }
        }
    }

    return disc_req;
}


/*************************************************************************
NAME
    handleA2DPSyncDelayInd

DESCRIPTION
    Handle request from A2DP library for a Sink device to supply an initial
    Synchronisation Delay (audio latency) report.

RETURNS

**************************************************************************/
void handleA2DPSyncDelayInd (uint8 device_id, uint8 seid)
{
    Task audio_plugin;

    if ((audio_plugin = getA2dpPlugin(seid)) != NULL)
    {
        uint16 index = 0;
        uint16 latency = 0;
        bool estimated = FALSE;

        if(sinkBroadcastAudioIsActive() && getA2dpIndex(device_id, &index))
        {
            A2dpMediaAvSyncDelayResponse(getA2dpLinkDataDeviceId(index), getA2dpLinkDataSeId(index), SinkBroadcasAudioGetA2dpLatency());
        }
        else
        {
            if ( AudioGetLatencyInTenthsOfMilliseconds(audio_plugin, &estimated, &latency) && getA2dpIndex(device_id, &index) )
            {
                if (estimated)
               {
                   /* if the Audio plugin has estimated the latency, then use the target instead */
                   /* units are 100us */
                   latency = 10 * audioPriorLatency(seid);
               }
                A2dpMediaAvSyncDelayResponse(getA2dpLinkDataDeviceId(index), getA2dpLinkDataSeId(index), latency);
            }
        }
    }
}

/*************************************************************************
NAME
    handleA2DPLatencyReport

DESCRIPTION
    Handle notification from an audio plugin raised due to the DSP providing
    a measured audio latency value.

RETURNS

**************************************************************************/
void handleA2DPLatencyReport (bool estimated, uint16 latency, Sink sink)
{
    uint16 index = 0;

    if (getA2dpIndexFromSink(sink, &index))
    {
        a2dp_codec_settings* codec_settings;
        setA2dpLinkDataLatency(index, latency);
        
        codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
        
        if(codec_settings)
        {
            if(codec_settings->codecData.latency_reporting)
            {
                if (estimated)
                {
                    A2dpMediaAvSyncDelayResponse(getA2dpLinkDataDeviceId(index), getA2dpLinkDataSeId(index), latency);
                }
                else
                {
                    A2dpMediaAvSyncDelayRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataSeId(index), latency);
                }
            }
            
            freePanic(codec_settings);
        }
    }
}

/*************************************************************************
NAME
    sinkCheckIfNewCodec

DESCRIPTION
   Checks if a2dp media reconfigure indication is due to new
   codec being selected and plays codec indication prompt if
   it is new codec.

RETURNS
   TRUE: If codec is new
   FALSE : If codec is not new

**************************************************************************/
static void sinkPlayPromptIfNewCodec(uint8 deviceId, uint8 streamId)
{
    uint16 a2dp_index = 0;
    a2dp_codec_settings * codec_settings = NULL;
    codec_settings = A2dpCodecGetSettings(deviceId, streamId);

    /*Get codec for this stream stored in sink app*/
    getA2dpIndex(deviceId, &a2dp_index);
    if(codec_settings->seid == getA2dpLinkDataSeId(a2dp_index))
    {
        A2DP_DEBUG(("A2DP: Codec is same\n"));
        return;
    }
    sinkUpdateCodecConfiguredToApp(deviceId,streamId);
}

/*************************************************************************
NAME
    sinkUpdateCodecConfiguredToApp

DESCRIPTION
   Send message to sink application which codec is configured.

RETURNS

**************************************************************************/

static void sinkUpdateCodecConfiguredToApp( uint8 deviceId,uint8 streamId)
{
    a2dp_codec_settings * codec_settings = NULL;

    codec_settings = A2dpCodecGetSettings(deviceId, streamId);

    if(codec_settings != NULL)
    {       
        switch(codec_settings->seid)
        {
            case APTX_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecAptxInUse , 0 ) ;
            break;

            case APTXHD_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecAptxHDInUse , 0 ) ;
            break;

            case APTX_LL_SEID:
            case APTX_SPRINT_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecAptxLLInUse , 0 ) ;
            break;

            case APTX_ADAPTIVE_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecAptxAdaptiveInUse , 0 ) ;
            break;

            case SBC_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecSbcInUse , 0 ) ;
            break;

            case MP3_SEID:
                MessageSend (sinkGetMainTask(), EventSysCodecMp3InUse , 0 ) ;
            break;

            case AAC_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecAacInUse , 0 ) ;
            break;

            case FASTSTREAM_SEID:
                MessageSend ( sinkGetMainTask(), EventSysCodecFaststream , 0 ) ;
            break;

            default:
            break;
        }
        free(codec_settings);
    }
}

/*************************************************************************
NAME
    handleA2dpReconfigureInd

DESCRIPTION
    handle the indication of stream reconfigure

RETURNS
    None

**************************************************************************/
static void handleA2dpReconfigureInd(uint8 deviceId, uint8 streamId)
{
    sinkPlayPromptIfNewCodec(deviceId,streamId);
}

/*************************************************************************
NAME
    handleA2dpReconfigureCfm

DESCRIPTION
    handle the confirmation of stream reconfigure

RETURNS
    None

**************************************************************************/
static void handleA2dpReconfigureCfm(uint8 deviceId, uint8 streamId,a2dp_status_code status)
{
    uint16 Id;

    UNUSED(status); /* Can be used depending on compile-time definitions */

    getA2dpIndex(deviceId, &Id);

#ifdef ENABLE_PEER
    if (getA2dpPeerRemoteDevice(Id) == remote_device_peer)
    {
        handlePeerQualificationReconfigureCfm(deviceId,streamId,status);
    }
    else
#else
    UNUSED(status);
#endif
    {
        sinkPlayPromptIfNewCodec(deviceId,streamId);
    }
}

/*************************************************************************
NAME
    handleA2DPMessage

DESCRIPTION
    A2DP message Handler, this function handles all messages returned
    from the A2DP library and calls the relevant functions if required

RETURNS

**************************************************************************/
void handleA2DPMessage( Task task, MessageId id, Message message )
{
    UNUSED(task);

    switch (id)
    {
/******************/
/* INITIALISATION */
/******************/

        /* confirmation of the initialisation of the A2DP library */
        case A2DP_INIT_CFM:
            A2DP_DEBUG(("A2DP_INIT_CFM : \n"));
            sinkA2dpInitComplete((const A2DP_INIT_CFM_T *) message);
        break;

/*****************************/
/* SIGNALING CHANNEL CONTROL */
/*****************************/

        /* indication of a remote source trying to make a signalling connection */
        case A2DP_SIGNALLING_CONNECT_IND:
            A2DP_DEBUG(("A2DP_SIGNALLING_CHANNEL_CONNECT_IND : \n"));
            handleA2DPSignallingConnectInd( ((const A2DP_SIGNALLING_CONNECT_IND_T *)message)->device_id,
                                            ((const A2DP_SIGNALLING_CONNECT_IND_T *)message)->addr );
        break;

        /* confirmation of a signalling connection attempt, successful or not */
        case A2DP_SIGNALLING_CONNECT_CFM:
            A2DP_DEBUG(("A2DP_SIGNALLING_CHANNEL_CONNECT_CFM : \n"));
            handleA2DPSignallingConnected(((const A2DP_SIGNALLING_CONNECT_CFM_T*)message)->status,
                                          ((const A2DP_SIGNALLING_CONNECT_CFM_T*)message)->device_id,
                                          ((const A2DP_SIGNALLING_CONNECT_CFM_T*)message)->addr,
                                          ((const A2DP_SIGNALLING_CONNECT_CFM_T*)message)->locally_initiated);
        break;

        /* indication of a signalling channel disconnection having occured */
        case A2DP_SIGNALLING_DISCONNECT_IND:
            A2DP_DEBUG(("A2DP_SIGNALLING_CHANNEL_DISCONNECT_IND : \n"));
            handleA2DPSignallingDisconnected(((const A2DP_SIGNALLING_DISCONNECT_IND_T*)message)->device_id,
                                             ((const A2DP_SIGNALLING_DISCONNECT_IND_T*)message)->status,
                                             ((const A2DP_SIGNALLING_DISCONNECT_IND_T*)message)->addr);
        break;

/*************************/
/* MEDIA CHANNEL CONTROL */
/*************************/

        /* indication of a remote device attempting to open a media channel */
        case A2DP_MEDIA_OPEN_IND:
            A2DP_DEBUG(("A2DP_OPEN_IND : \n"));
            handleA2DPOpenInd(((const A2DP_MEDIA_OPEN_IND_T*)message)->device_id,
                              ((const A2DP_MEDIA_OPEN_IND_T*)message)->seid);
        break;

        /* confirmation of request to open a media channel */
        case A2DP_MEDIA_OPEN_CFM:
            A2DP_DEBUG(("A2DP_OPEN_CFM : \n"));
            handleA2DPOpenCfm(((const A2DP_MEDIA_OPEN_CFM_T*)message)->device_id,
                              ((const A2DP_MEDIA_OPEN_CFM_T*)message)->stream_id,
                              ((const A2DP_MEDIA_OPEN_CFM_T*)message)->seid,
                              ((const A2DP_MEDIA_OPEN_CFM_T*)message)->status);
        break;

        /* Stream has been reconfigured */
        case A2DP_MEDIA_RECONFIGURE_CFM:
            A2DP_DEBUG(("A2DP_MEDIA_RECONFIGURE_CFM : \n"));
            handleA2dpReconfigureCfm(((const A2DP_MEDIA_RECONFIGURE_CFM_T*)message)->device_id,
                              ((const A2DP_MEDIA_RECONFIGURE_CFM_T*)message)->stream_id,
                              ((const A2DP_MEDIA_RECONFIGURE_CFM_T*)message)->status);
            break;

        /* indication of a request to close the media channel, remotely generated */
        case A2DP_MEDIA_CLOSE_IND:
            A2DP_DEBUG(("A2DP_CLOSE_IND : \n"));
            handleA2DPClose(((const A2DP_MEDIA_CLOSE_IND_T*)message)->device_id,
                            ((const A2DP_MEDIA_CLOSE_IND_T*)message)->stream_id,
                            ((const A2DP_MEDIA_CLOSE_IND_T*)message)->status);
        break;

        /* confirmation of the close of the media channel, locally generated  */
        case A2DP_MEDIA_CLOSE_CFM:
           A2DP_DEBUG(("A2DP_CLOSE_CFM : \n"));
           handleA2DPClose(0,0,a2dp_success);
        break;

/**********************/
/*  STREAMING CONTROL */
/**********************/

        /* indication of start of media streaming from remote source */
        case A2DP_MEDIA_START_IND:
            A2DP_DEBUG(("A2DP_START_IND : \n"));
            handleA2DPStartInd(((const A2DP_MEDIA_START_IND_T*)message)->device_id,
                               ((const A2DP_MEDIA_START_IND_T*)message)->stream_id);
        break;

        /* confirmation of a local request to start media streaming */
        case A2DP_MEDIA_START_CFM:
            A2DP_DEBUG(("A2DP_START_CFM : \n"));
            handleA2DPStartStreaming(((const A2DP_MEDIA_START_CFM_T*)message)->device_id,
                                     ((const A2DP_MEDIA_START_CFM_T*)message)->stream_id,
                                     ((const A2DP_MEDIA_START_CFM_T*)message)->status);
        break;

        case A2DP_MEDIA_SUSPEND_IND:
            A2DP_DEBUG(("A2DP_SUSPEND_IND : \n"));
            handleA2DPSuspendStreamingInd(((const A2DP_MEDIA_SUSPEND_IND_T*)message)->device_id,
                                         ((const A2DP_MEDIA_SUSPEND_IND_T*)message)->stream_id);
        break;

        case A2DP_MEDIA_SUSPEND_CFM:
            A2DP_DEBUG(("A2DP_SUSPEND_CFM : \n"));
            handleA2DPSuspendStreamingCfm(((const A2DP_MEDIA_SUSPEND_CFM_T*)message)->device_id,
                                         ((const A2DP_MEDIA_SUSPEND_CFM_T*)message)->stream_id,
                                         ((const A2DP_MEDIA_SUSPEND_CFM_T*)message)->status);
        break;

/*************************/
/* MISC CONTROL MESSAGES */
/*************************/

        case A2DP_MEDIA_AV_SYNC_DELAY_UPDATED_IND:
            A2DP_DEBUG(("A2DP_MEDIA_AV_SYNC_DELAY_UPDATED_IND : seid=0x%X delay=%u\n",
                                ((const A2DP_MEDIA_AV_SYNC_DELAY_UPDATED_IND_T*)message)->seid,
                                ((const A2DP_MEDIA_AV_SYNC_DELAY_UPDATED_IND_T*)message)->delay));
             /* Only received for source SEIDs.  Use delay value to aid AV synchronisation */
        break;

        case A2DP_MEDIA_AV_SYNC_DELAY_IND:
            A2DP_DEBUG(("A2DP_MEDIA_AV_SYNC_DELAY_IND : seid=0x%X\n",
                                ((const A2DP_MEDIA_AV_SYNC_DELAY_IND_T*)message)->seid));
            handleA2DPSyncDelayInd(((const A2DP_MEDIA_AV_SYNC_DELAY_IND_T*)message)->device_id,
                                   ((const A2DP_MEDIA_AV_SYNC_DELAY_IND_T*)message)->seid);
        break;

        case A2DP_MEDIA_AV_SYNC_DELAY_CFM:
            A2DP_DEBUG(("A2DP_MEDIA_AV_SYNC_DELAY_CFM : \n"));
        break;

        /* link loss indication */
        case A2DP_SIGNALLING_LINKLOSS_IND:
            A2DP_DEBUG(("A2DP_SIGNALLING_LINKLOSS_IND : \n"));
            handleA2DPSignallingLinkloss(((const A2DP_SIGNALLING_LINKLOSS_IND_T*)message)->device_id);
        break;

        case A2DP_CODEC_CONFIGURE_IND:
            A2DP_DEBUG(("A2DP_CODEC_CONFIGURE_IND : \n"));
#ifdef ENABLE_PEER
            handleA2dpCodecConfigureIndFromPeer((A2DP_CODEC_CONFIGURE_IND_T *)message);
#endif
        break;

        case A2DP_ENCRYPTION_CHANGE_IND:
            A2DP_DEBUG(("A2DP_ENCRYPTION_CHANGE_IND : \n"));
        break;

        case A2DP_LINKLOSS_RECONNECT_CANCEL_IND:
            A2DP_DEBUG(("A2DP_LINKLOSS_RECONNECT_CANCEL_IND : \n"));
            handleA2DPLinklossReconnectCancel(((const A2DP_LINKLOSS_RECONNECT_CANCEL_IND_T*)message)->device_id);
        break;

        case A2DP_MEDIA_RECONFIGURE_IND:
            A2DP_DEBUG(("A2DP_MEDIA_RECONFIGURE_IND :\n"));
            handleA2dpReconfigureInd(((const A2DP_MEDIA_RECONFIGURE_IND_T*)message)->device_id,((const A2DP_MEDIA_RECONFIGURE_IND_T*)message)->stream_id);

        break;
        default:
            A2DP_DEBUG(("A2DP UNHANDLED MSG: 0x%x\n",id));
        break;
    }
}



#ifdef ENABLE_SUBWOOFER
/*************************************************************************
NAME
    suspendWhenSubwooferStreamingLowLatency

DESCRIPTION
    funnction to determine if a2dp stream should be suspended due to the use
    of the low latency subwoofer link. Streaming a2dp media will adversely affect
    the quality of the sub low latency link due to bluetooth link bandwidth
    limitations

RETURNS
    none

**************************************************************************/
void suspendWhenSubwooferStreamingLowLatency(uint16 Id)
{
    /* check whether this a2dp link got routed */
    if(sinkAudioGetRoutedAudioSource() != audio_source_a2dp_1 + Id)
    {
        /* this a2dp source is not currently routed, check for presence of subwoofer */
        if (SwatGetMediaLLState(sinkSwatGetDevId()) == swat_media_streaming)
        {
            /* sub woofer is currently streaming and using an esco connection
               it is necessary to suspend this a2dp stream to prevent sub stream dissruption */
            audioSuspendDisconnectAllA2dpMedia();
        }
    }
}
#endif

bool a2dpAudioSinkMatch(a2dp_index_t a2dp_link, Sink sink)
{
    uint8 device_id = getA2dpLinkDataDeviceId(a2dp_link);
    uint8 stream_id = getA2dpLinkDataStreamId(a2dp_link);

    if(getA2dpStatusFlag(CONNECTED, a2dp_link))
        if(A2dpMediaGetSink(device_id, stream_id) == sink)
            return TRUE;
    return FALSE;
}

bool a2dpA2dpAudioIsRouted(void)
{
    uint8 index;

    for(index = a2dp_primary; index < (a2dp_secondary+1); index++)
    {
        /* is a2dp connected? */
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            /* check whether the a2dp connection is present and streaming data and that the audio is routed */
            if(sinkAudioIsAudioRouted() && (sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index))))
            {
                return TRUE;
            }
        }
    }
    return FALSE;
}

a2dp_index_t a2dpGetRoutedInstanceIndex(void)
{
    a2dp_index_t index;

    for(index = a2dp_primary; index < (a2dp_secondary+1); index++)
    {
        /* is a2dp connected? */
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            /* check whether the a2dp connection is present and streaming data and that the audio is routed */
            if( sinkAudioIsAudioRouted() && (sinkAudioGetRoutedAudioSink() == A2dpMediaGetSink(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index))))
            {
                return index;
            }
        }
    }
    return a2dp_invalid;
}
#ifdef ENABLE_PEER
/*************************************************************************
NAME
    HandlePeerRemoveAuthDevice

DESCRIPTION
    Delete the link key, if there is no peer device connected,
    or another peer device connected

RETURNS
    none

**************************************************************************/
void HandlePeerRemoveAuthDevice(const bdaddr* message)
{
        uint16 peerIndex = 0;
        bdaddr addr = *message;
        A2DP_DEBUG(("EventSysRemovePeerTempPairing\n"));
        /* delete the link key, if there is no peer device connected, or another peer device connected */
        if(!(a2dpGetPeerIndex(&peerIndex)) || !(BdaddrIsSame(getA2dpLinkBdAddr(peerIndex), &addr)))
        {
          ConnectionSmDeleteAuthDevice(&addr);
        }
}
#endif

#ifdef ENABLE_PEER
/*************************************************************************
NAME
    a2dpStartPauseSuspendTimeout

DESCRIPTION
    Start a timeout after which we will ask the remote device to suspend
    its a2dp media channel.

    We don't send the suspend immediately because not all AG's behave
    well in that situation. Instead we delay it to give the AG time
    to send its own suspend.

RETURNS
    none

**************************************************************************/
void a2dpStartPauseSuspendTimeout(a2dp_index_t index)
{
    a2dpCancelPauseSuspendTimeout(index);

    A2DP_DEBUG(("A2DP: a2dpStartPauseSuspendTimeout idx %u\n", index));

    MessageSendLater(&theSink.task, (EventSysA2dpPauseSuspendTimeoutDevice1 + index), 0, A2DP_PAUSE_SUSPEND_TIMER);
}

/*************************************************************************
NAME
    a2dpCancelPauseSuspendTimeout

DESCRIPTION
    Cancel any outstanding suspend timeout for the given a2dp device.

RETURNS
    none

**************************************************************************/
void a2dpCancelPauseSuspendTimeout(a2dp_index_t index)
{
    A2DP_DEBUG(("A2DP: a2dpCancelPauseSuspendTimeout idx %u\n", index));
    MessageCancelAll(&theSink.task, EventSysA2dpPauseSuspendTimeoutDevice1 + index);
}
#endif

/*************************************************************************
NAME
    sinkA2dpGetA2dpVolumeFromBdaddr

DESCRIPTION
    Retrieve the A2DP volume for the connection to the device with the address specified.

RETURNS
    Returns TRUE if the volume was retrieved, FALSE otherwise.
    The actual volume is returned in the a2dp_volume variable.

**************************************************************************/
bool sinkA2dpGetA2dpVolumeFromBdaddr(const bdaddr *bd_addr, uint16 * const a2dp_volume)
{
    uint8 i;

    /* go through A2dp connection looking for match */
    for_all_a2dp(i)
    {
        /* if the a2dp link is connected check its bdaddr */
        if(getA2dpStatusFlag(CONNECTED, i))
        {
            /* if a match is found return its volume level and a
               status of successful match found */
            if(BdaddrIsSame(getA2dpLinkBdAddr(i), bd_addr))
            {
                *a2dp_volume = sinkA2dpGetA2dpVolumeInfoAtIndex(i)->main_volume;
                A2DP_DEBUG(("A2DP: getVolume = %d\n", i));
                return TRUE;
            }
        }
    }
    /* no matches found so return not successful */
    return FALSE;
}

/*************************************************************************
NAME
    sinkA2dpSetA2dpVolumeAtIndex

DESCRIPTION
    Sets the A2DP volume for the connection at the specified index.

**************************************************************************/
void sinkA2dpSetA2dpVolumeAtIndex(uint8 a2dp_index, uint16 a2dp_volume)
{
    A2DP_DATA.a2dp_volume[a2dp_index].main_volume = a2dp_volume;
}


/*************************************************************************
NAME
    sinkA2dpGetA2dpVolumeInfoAtIndex

DESCRIPTION
    Gets the volume_info structure of the connection at the specified index.

**************************************************************************/
volume_info * sinkA2dpGetA2dpVolumeInfoAtIndex(uint8 a2dp_index)
{
    return &A2DP_DATA.a2dp_volume[a2dp_index];
}

/*************************************************************************
NAME
    sinkA2dpSetA2dpVolumeInfoAtIndex

DESCRIPTION
    Sets the volume_info structure of the connection at the specified index.

**************************************************************************/
void sinkA2dpSetA2dpVolumeInfoAtIndex(uint8 a2dp_index, volume_info volume)
{
    A2DP_DATA.a2dp_volume[a2dp_index] = volume;
}

/*************************************************************************
NAME
    sinkA2dpSetA2dpAuxVolumeAtIndex

DESCRIPTION
    Sets the A2DP Aux volume for the connection at the specified index.

**************************************************************************/
void sinkA2dpSetA2dpAuxVolumeAtIndex(uint8 a2dp_index, uint16 a2dp_volume)
{
    A2DP_DATA.a2dp_volume[a2dp_index].aux_volume = a2dp_volume;
}

/*************************************************************************
NAME
    sinkA2dpGetA2dpVolumeInfoAtIndex

DESCRIPTION
    Gets the volume_info structure of the connection at the specified index.

**************************************************************************/
int16 SinkA2dpGetMainVolume(uint8 index)
{
    return A2DP_DATA.a2dp_volume[index].main_volume;
}

/*************************************************************************
NAME
    sinkA2dpAudioPrimaryOrSecondarySinkMatch

DESCRIPTION
    Checks the status of the two A2DP streams.

RETURNS
    TRUE if one of them is connected, otherwise FALSE.

**************************************************************************/
bool sinkA2dpAudioPrimaryOrSecondarySinkMatch(Sink sink)
{
    return (a2dpAudioSinkMatch(a2dp_primary, sink) ||
                a2dpAudioSinkMatch(a2dp_secondary, sink));
}

/*************************************************************************
NAME
    sinkA2dpGetStreamState

DESCRIPTION
    Get the stream state for the connected A2DP stream

RETURNS
    a2dp stream state

**************************************************************************/
a2dp_stream_state sinkA2dpGetStreamState(a2dp_index_t index)
{
    if (SinkA2dpIsInitialised())
    {
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            return A2dpMediaGetState(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
        }
    }
    return a2dp_stream_idle;
}

/*************************************************************************
NAME
    sinkA2dpGetRoleType

DESCRIPTION
    Get the a2dp media role type

RETURNS
    a2dp role type

**************************************************************************/
a2dp_role_type sinkA2dpGetRoleType(a2dp_index_t index)
{
    if (SinkA2dpIsInitialised())
    {
        if(getA2dpStatusFlag(CONNECTED, index))
        {
            return A2dpMediaGetRole(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index)) ;
        }
    }
    return a2dp_role_undefined;
}

Sink sinkA2dpGetAudioSink(a2dp_index_t a2dp_link)
{
    uint8 device_id = getA2dpLinkDataDeviceId(a2dp_link);
    uint8 stream_id = getA2dpLinkDataStreamId(a2dp_link);
    if(getA2dpStatusFlag(CONNECTED, a2dp_link))
    {
        return A2dpMediaGetSink(device_id, stream_id);
    }
    return NULL;
}

static bool isA2dpLinkPlaying(a2dp_index_t a2dp_index)
{
    bool is_playing_state = FALSE;
    if(sinkA2dpGetA2dpLinkSuspendState(a2dp_index) != a2dp_remote_suspended)
    {
        switch(sinkA2dpGetStreamState(a2dp_index))
        {
            case a2dp_stream_starting:
            case a2dp_stream_streaming:
                is_playing_state = TRUE;
                break;
            case a2dp_stream_open:
                {
                    uint16 avrcpIndex;
                    if(sinkAvrcpGetIndexFromBdaddr(getA2dpLinkBdAddr(a2dp_index), &avrcpIndex, TRUE))
                    {
                        is_playing_state = sinkAvrcpIsAvrcpLinkInPlayingState(avrcpIndex);
                    }
                }
                break;
            default:
                break;
        }
    }
    return is_playing_state;
}

static bool isA2dpLinkResumable(a2dp_index_t a2dp_index)
{
    bool is_resumable = FALSE;
    if(sinkA2dpGetA2dpLinkSuspendState(a2dp_index) == a2dp_local_suspended)
    {
        is_resumable = (sinkA2dpGetStreamState(a2dp_index) == a2dp_stream_open ? TRUE : FALSE);
    }
    return is_resumable;
}

/****************************************************************************
NAME
    sinkA2dpIndicateCodecExit

DESCRIPTION
    Sends an event to indicate which A2DP codec has exit

RETURNS
    void
*/

void sinkA2dpIndicateCodecExit(uint8 seid)
{
    A2DP_DEBUG(("sinkA2dpIndicateCodecExit prev seid %d\n", seid));

    switch( seid )
    {
        case SBC_SEID:
            MessageSend ( &theSink.task , EventSysCodecSbcDisconnected , 0 ) ;
            break;
        case MP3_SEID:
            MessageSend ( &theSink.task , EventSysCodecMp3Disconnected , 0 ) ;
            break;
        case AAC_SEID:
            MessageSend ( &theSink.task , EventSysCodecAacDisconnected , 0 ) ;
            break;
        case APTX_SEID:
            MessageSend ( &theSink.task , EventSysCodecAptxDisconnected , 0 ) ;
            break;
        case APTXHD_SEID:
            MessageSend ( &theSink.task , EventSysCodecAptxHDDisconnected , 0 ) ;
            break;
        case FASTSTREAM_SEID:
            MessageSend ( &theSink.task , EventSysCodecFaststreamDisconnected , 0 ) ;
            break;
        case APTX_SPRINT_SEID:
            MessageSend ( &theSink.task , EventSysCodecAptxLLDisconnected , 0 ) ;
            break;
        case APTX_ADAPTIVE_SEID:
            MessageSend(&theSink.task , EventSysCodecAptxAdaptiveDisconnected, 0);
            break;
        default:
            A2DP_DEBUG(("AUD: Unknown codec\n"));
            break;
    }

}

bool sinkA2dpIsA2dpSinkPlaying(a2dp_index_t a2dp_index)
{
    bool is_playing = FALSE;

    if(sinkA2dpGetRoleType(a2dp_index) == a2dp_sink
            && isA2dpLinkPlaying(a2dp_index))
    {
        is_playing = TRUE;
    }
    return is_playing;
}

bool sinkA2dpIsA2dpSinkRoutable(a2dp_index_t a2dp_index)
{
    bool is_routable = FALSE;
    if(sinkA2dpGetRoleType(a2dp_index) == a2dp_sink)
    {
        if(isA2dpLinkResumable(a2dp_index) || isA2dpLinkPlaying(a2dp_index))
        {
            is_routable = TRUE;
        }
    }
    return is_routable;
}

bool sinkA2dpGetSpeakerPeqBypass(void)
{
    bool speaker_peq_bypass;

    if((A2DP_DATA.a2dp_audio_mode_params.music_mode_enhancements & MUSIC_CONFIG_SPKR_EQ_BYPASS) == MUSIC_CONFIG_SPKR_EQ_BYPASS)
    {
        speaker_peq_bypass = TRUE;
    }
    else
    {
        speaker_peq_bypass = FALSE;
    }

    return speaker_peq_bypass;
}

/****************************************************************************
NAME    
    sinkA2dpSignallingConnectRequest
    
DESCRIPTION
    Issue an A2DP Signalling Connect request and increment paging count
    
RETURNS
    void
*/
void sinkA2dpSignallingConnectRequest(const bdaddr *addr)
{
    sinkInquiryIncrementPagingCount();
    A2dpSignallingConnectRequest(addr);
}

/****************************************************************************
NAME    
    sinkA2dpIsStreamingAllowed
    
DESCRIPTION
    Check if A2DP streaming is allowed:
    
RETURNS
    TRUE if A2DP streaming is allowed, FALSE if not allowed
*/
bool sinkA2dpIsStreamingAllowed(void)
{
    if (sinkUpgradeIsProtectAudio())/*Disabled by default*/
    {
        return TRUE;
    }
    else if (gaiaIsGattUpgradeInProgress())/*Enabled on a GATT Upgrade*/
    {
        return FALSE;
    }
    return TRUE;
}

