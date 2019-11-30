/*!
\copyright  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_av_caps.c
\brief      Defines the A2DP capabilities
*/


#include <a2dp.h>
#include <panic.h>

#include "av_headset.h"

#define SPLIT_IN_4_OCTETS(x) (x >> 24) & 0xFF, (x >> 16) & 0xFF, (x >> 8) & 0xFF, x & 0xFF

/*@{ \name SBC configuration bit fields*/
/*! [Octet 0] Support for 16kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_16000        128
/*! [Octet 0] Support for 32kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_32000         64
/*! [Octet 0] Support for 44.1kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_44100         32
/*! [Octet 0] Support for 48kHz Sampling frequency */
#define SBC_SAMPLING_FREQ_48000         16
/*! [Octet 0] Support for Mono channel mode */
#define SBC_CHANNEL_MODE_MONO            8
/*! [Octet 0] Support for Dualchannel mode */
#define SBC_CHANNEL_MODE_DUAL_CHAN       4
/*! [Octet 0] Support for Stereo channel mode */
#define SBC_CHANNEL_MODE_STEREO          2
/*! [Octet 0] Support for Joint Stereo channel mode */
#define SBC_CHANNEL_MODE_JOINT_STEREO    1

/*! [Octet 1] Support for a block length of 4 */
#define SBC_BLOCK_LENGTH_4             128
/*! [Octet 1] Support for a block length of 8 */
#define SBC_BLOCK_LENGTH_8              64
/*! [Octet 1] Support for a block length of 12 */
#define SBC_BLOCK_LENGTH_12             32
/*! [Octet 1] Support for a block length of 16 */
#define SBC_BLOCK_LENGTH_16             16
/*! [Octet 1] Support for 4 subbands */
#define SBC_SUBBANDS_4                   8
/*! [Octet 1] Support for 8 subbands */
#define SBC_SUBBANDS_8                   4
/*! [Octet 1] Support for SNR allocation */
#define SBC_ALLOCATION_SNR               2
/*! [Octet 1] Support for Loudness allocation */
#define SBC_ALLOCATION_LOUDNESS          1

/*! [Octet 2] Minimum bitpool supported */
#define SBC_BITPOOL_MIN                  2
/*! [Octet 2] Maximum bitpool supported */
#define SBC_BITPOOL_MAX                250
/*! [Octet 2] Maximum bitpool for Medium quality */
#define SBC_BITPOOL_MEDIUM_QUALITY      35
/*! [Octet 2] Maximum bitpool for High quality */
#define SBC_BITPOOL_HIGH_QUALITY        53


/*@} */


/*! Default SBC Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.

    Support all features and full bitpool range. Note that we trust the source
    to choose a bitpool value suitable for the Bluetooth bandwidth.
*/
const uint8 sbc_caps_sink[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_44100     | SBC_SAMPLING_FREQ_48000    |
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    2,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};

const uint8 sbc_caps_src[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_44100     | SBC_SAMPLING_FREQ_48000    |
    SBC_CHANNEL_MODE_MONO,

    SBC_BLOCK_LENGTH_16         | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};

/*@{ \name AAC/AAC+ configuration bit fields*/

/*! [Octet 0] Support for MPEG-2 AAC LC */
#define AAC_MPEG2_AAC_LC        (1<<7)
/*! [Octet 0] Support for MPEG-4 AAC LC */
#define AAC_MPEG4_AAC_LC        (1<<6)
/*! [Octet 0] Support for MPEG-4 AAC LTP */
#define AAC_MPEG4_AAC_LTP       (1<<5)
/*! [Octet 0] Support for MPEG-4 AAC Scalable */
#define AAC_MPEG4_AAC_SCALE     (1<<4)

/*! [Octet 1] Support for 8kHz sampling frequency */
#define AAC_SAMPLE_8000         (1<<7)
/*! [Octet 1] Support for 11025Hz sampling frequency */
#define AAC_SAMPLE_11025        (1<<6)
/*! [Octet 1] Support for 12kHz sampling frequency */
#define AAC_SAMPLE_12000        (1<<5)
/*! [Octet 1] Support for 16kHz sampling frequency */
#define AAC_SAMPLE_16000        (1<<4)
/*! [Octet 1] Support for 22050Hz sampling frequency */
#define AAC_SAMPLE_22050        (1<<3)
/*! [Octet 1] Support for 24kHz sampling frequency */
#define AAC_SAMPLE_24000        (1<<2)
/*! [Octet 1] Support for 32kHz sampling frequency */
#define AAC_SAMPLE_32000        (1<<1)
/*! [Octet 1] Support for 44.1kHz sampling frequency */
#define AAC_SAMPLE_44100        (1<<0)
/*! [Octet 2] Support for 48kHz sampling frequency */
#define AAC_SAMPLE_48000        (1<<7)
/*! [Octet 2] Support for 64kHz sampling frequency */
#define AAC_SAMPLE_64000        (1<<6)
/*! [Octet 2] Support for 88.2kHz sampling frequency */
#define AAC_SAMPLE_88200        (1<<5)
/*! [Octet 2] Support for 96kHz sampling frequency */
#define AAC_SAMPLE_96000        (1<<4)
/*! [Octet 2] Support for using 1 channel */
#define AAC_CHANNEL_1           (1<<3)
/*! [Octet 2] Support for using 2 channels */
#define AAC_CHANNEL_2           (1<<2)

/*! [Octet 3] Support for Variable Bit Rate */
#define AAC_VBR                 (1<<7)

/*! The maximum AAC bitrate */
#define AAC_BITRATE 264630
/*! Most significant word of the AAC bitrate */
#define AAC_BITRATE_MSW UINT32_MSW(AAC_BITRATE)
/*! Least significant word of the AAC bitrate */
#define AAC_BITRATE_LSW UINT32_LSW(AAC_BITRATE)

/*! AAC bitrate [Octet 3]  */
#define AAC_BITRATE_3       UINT16_LSO(AAC_BITRATE_MSW)

/*! AAC bitrate [Octet 4] */
#define AAC_BITRATE_4       UINT16_MSO(AAC_BITRATE_LSW)

/*! AAC bitrate [Octet 5] */
#define AAC_BITRATE_5       UINT16_LSO(AAC_BITRATE_LSW)

/*@} */

/*! Default AAC/AAC+ Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.

    Support all features.
*/
static const uint8 aac_caps_sink[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    8,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_MPEG2_4_AAC,

    AAC_MPEG2_AAC_LC | AAC_MPEG4_AAC_LC,
    AAC_SAMPLE_44100,
    AAC_SAMPLE_48000 | AAC_CHANNEL_1 | AAC_CHANNEL_2,
    AAC_VBR | AAC_BITRATE_3,
    AAC_BITRATE_4,
    AAC_BITRATE_5,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    2,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*@{ \name Common APTX configuration bit fields */
#define APTX_SAMPLING_FREQ_44100         32
#define APTX_SAMPLING_FREQ_48000         16
#define APTX_CHANNEL_MODE_STEREO          2
#define APTX_CHANNEL_MODE_MONO            8

/*@} */

/*! Default apt-X Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.
*/
static const uint8 aptx_caps_sink[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    9,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    (A2DP_APT_VENDOR_ID >> 24) & 0xFF,    /* A2DP_APT_VENDOR_ID is defined backwards (0x4f000000 for ID 0x4f), so write octets in reverse order */
    (A2DP_APT_VENDOR_ID >> 16) & 0xFF,
    (A2DP_APT_VENDOR_ID >>  8) & 0xFF,
    (A2DP_APT_VENDOR_ID >>  0) & 0xFF,

    (A2DP_CSR_APTX_CODEC_ID >> 8) & 0xFF, /* A2DP_CSR_APTX_CODEC_ID is defined backwares (0x0100 for ID 0x01), so write octets in reverse order */
    (A2DP_CSR_APTX_CODEC_ID >> 0) & 0xFF,

    APTX_SAMPLING_FREQ_44100 | APTX_SAMPLING_FREQ_48000 | APTX_CHANNEL_MODE_STEREO,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    2,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};

static const uint8 aptx_tws_caps[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    9,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    (A2DP_QTI_VENDOR_ID >> 24) & 0xFF,      /* A2DP_QTI_VENDOR_ID is defined backwards (0xd7000000 for ID 0xd7), so write octets in reverse order */
    (A2DP_QTI_VENDOR_ID >> 16) & 0xFF,
    (A2DP_QTI_VENDOR_ID >>  8) & 0xFF,
    (A2DP_QTI_VENDOR_ID >>  0) & 0xFF,

    (A2DP_QTI_APTX_TWS_PLUS_CODEC_ID >> 8) & 0xFF,
    (A2DP_QTI_APTX_TWS_PLUS_CODEC_ID >> 0) & 0xFF,

    APTX_SAMPLING_FREQ_44100    + APTX_SAMPLING_FREQ_48000    +
    APTX_CHANNEL_MODE_MONO,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};

static const uint8 sbc_tws_caps[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    16,     /* Length */
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    (A2DP_QTI_VENDOR_ID >> 24) & 0xFF,      /* A2DP_QTI_VENDOR_ID is defined backwards (0xd7000000 for ID 0xd7), so write octets in reverse order */
    (A2DP_QTI_VENDOR_ID >> 16) & 0xFF,
    (A2DP_QTI_VENDOR_ID >>  8) & 0xFF,
    (A2DP_QTI_VENDOR_ID >>  0) & 0xFF,

    (A2DP_QTI_SBC_TWS_PLUS_CODEC_ID >> 8) & 0xFF,
    (A2DP_QTI_SBC_TWS_PLUS_CODEC_ID >> 0) & 0xFF,

    AVDTP_SERVICE_MEDIA_CODEC,
    6,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_48000     | SBC_SAMPLING_FREQ_44100    |
    SBC_CHANNEL_MODE_MONO,

    SBC_BLOCK_LENGTH_16         | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};

/*!
    @brief The CSR True Wireless Stereo codec capabilities for AAC.
*/
const uint8 aac_tws_caps[] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    18, /* Length */
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    (A2DP_CSR_VENDOR_ID >> 24) & 0xFF,
    (A2DP_CSR_VENDOR_ID >> 16) & 0xFF,
    (A2DP_CSR_VENDOR_ID >>  8) & 0xFF,
    (A2DP_CSR_VENDOR_ID >>  0) & 0xFF,

    (A2DP_CSR_TWS_AAC_CODEC_ID >> 8) & 0xFF,
    (A2DP_CSR_TWS_AAC_CODEC_ID >> 0) & 0xFF,

    /* Embed AAC capabilities */
    AVDTP_SERVICE_MEDIA_CODEC,
    8,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG2_4_AAC,

    AAC_MPEG2_AAC_LC | AAC_MPEG4_AAC_LC,
    AAC_SAMPLE_8000 | AAC_SAMPLE_11025 | AAC_SAMPLE_12000 | AAC_SAMPLE_16000 | AAC_SAMPLE_22050 | AAC_SAMPLE_24000 | AAC_SAMPLE_32000 | AAC_SAMPLE_44100,
    AAC_SAMPLE_48000 | AAC_CHANNEL_1 | AAC_CHANNEL_2,
    AAC_VBR | AAC_BITRATE_3,
    AAC_BITRATE_4,
    AAC_BITRATE_5,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


#define APTX_AD_SAMPLING_FREQ_48000 (1 << 4)
#define APTX_AD_SAMPLING_FREQ_44100 (1 << 3)

#define APTX_AD_CHANNEL_MODE_TWS_PLUS (1 << 5)

#define APTX_AD_LL_TTP_MIN_IN_1MS  0        // Minimum latency in milliseconds for low-latency mode 
#define APTX_AD_LL_TTP_MAX_IN_4MS  75       // Max latency for low-latency mode in 4ms units (i.e. 75*4ms)
#define APTX_AD_HQ_TTP_MIN_IN_1MS  0        // Minimum latency in milliseconds for HQ mode 
#define APTX_AD_HQ_TTP_MAX_IN_4MS  75       // Max latency for HQ mode in 4ms units (i.e. 75*4ms)
#define APTX_AD_TWS_TTP_MIN_IN_1MS 100      // Minimum latency in milliseconds for TWS mode 
#define APTX_AD_TWS_TTP_MAX_IN_4MS 75       // Max latency for TWS mode in 4ms units (i.e. 75*4ms)

#define APTX_AD_CAPABILITY_EXTENSION_VERSION_NUMBER           0x01
#define APTX_AD_SUPPORTED_FEATURES                            0x0000000F
#define APTX_AD_FIRST_SETUP_PREFERENCE                        0x02
#define APTX_AD_SECOND_SETUP_PREFERENCE                       0x03
#define APTX_AD_THIRD_SETUP_PREFERENCE                        0x03
#define APTX_AD_FOURTH_SETUP_PREFERENCE                       0x03
#define APTX_AD_NO_FURTHER_EXPANSION                          0x00
#define APTX_AD_CAPABILITY_EXTENSION_END                      0x00

const uint8 aptx_adaptive_tws_snk_caps[48] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    42,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    (A2DP_QTI_VENDOR_ID >> 24) & 0xFF,
    (A2DP_QTI_VENDOR_ID >> 16) & 0xFF,
    (A2DP_QTI_VENDOR_ID >>  8) & 0xFF,
    (A2DP_QTI_VENDOR_ID >>  0) & 0xFF,

    (A2DP_QTI_APTX_AD_CODEC_ID >> 8) & 0xFF,
    (A2DP_QTI_APTX_AD_CODEC_ID >> 0) & 0xFF,

    APTX_AD_SAMPLING_FREQ_48000 | APTX_AD_SAMPLING_FREQ_44100,
    APTX_AD_CHANNEL_MODE_TWS_PLUS,

    APTX_AD_LL_TTP_MIN_IN_1MS,
    APTX_AD_LL_TTP_MAX_IN_4MS,
    APTX_AD_HQ_TTP_MIN_IN_1MS,
    APTX_AD_HQ_TTP_MAX_IN_4MS,
    APTX_AD_TWS_TTP_MIN_IN_1MS,
    APTX_AD_TWS_TTP_MAX_IN_4MS,

    0x00,

    APTX_AD_CAPABILITY_EXTENSION_VERSION_NUMBER,
    SPLIT_IN_4_OCTETS(APTX_AD_SUPPORTED_FEATURES),
    APTX_AD_FIRST_SETUP_PREFERENCE,
    APTX_AD_SECOND_SETUP_PREFERENCE,
    APTX_AD_THIRD_SETUP_PREFERENCE,
    APTX_AD_FOURTH_SETUP_PREFERENCE,
    APTX_AD_NO_FURTHER_EXPANSION,
    APTX_AD_CAPABILITY_EXTENSION_END,

    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
    0x00, 0x00,
        
    AVDTP_SERVICE_DELAY_REPORTING,
    0
};




#define DECODE_RESOURCE_ID (1)      /*!< Resource ID for endpoint definition, indicating decoding. That is, incoming audio */
#define ENCODE_RESOURCE_ID (2)      /*!< Resource ID for endpoint definition, indicating encoding. That is, outgoing audio */

/*!@{ \name Standard TWS sink endpoints
    \brief Predefined endpoints for audio Sink end point configurations, applicable to standard TWS and incoming TWS+ */
    /*! SBC */
const sep_config_type av_sbc_snk_sep     = {AV_SEID_SBC_SNK,      DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(sbc_caps_sink),  sbc_caps_sink};
    /*! AAC */
const sep_config_type av_aac_snk_sep     = {AV_SEID_AAC_SNK,      DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(aac_caps_sink),  aac_caps_sink};
    /*! APTX */
const sep_config_type av_aptx_snk_sep    = {AV_SEID_APTX_SNK,     DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(aptx_caps_sink), aptx_caps_sink};
/*!@} */

/*!@{ \name TWS sink endpoints
    \brief Predefined endpoints for audio Sink end point configurations, for receiving audio */
    /*! APTX mono */
const sep_config_type av_aptx_mono_tws_snk_sep = {AV_SEID_APTX_MONO_TWS_SNK, DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(aptx_tws_caps), aptx_tws_caps};
    /*! SBC mono */
const sep_config_type av_sbc_mono_tws_snk_sep = {AV_SEID_SBC_MONO_TWS_SNK,  DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(sbc_tws_caps), sbc_tws_caps};
    /*! AAC stereo */
const sep_config_type av_aac_stereo_tws_snk_sep = {AV_SEID_AAC_STEREO_TWS_SNK, DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(aac_tws_caps), aac_tws_caps};
/*!@} */

/*!@{ \name Standard or TWS configurable sink endpoints
    \brief Predefined endpoints for audio Sink end point configurations, for receiving audio */
/*! aptX adaptive */
const sep_config_type av_aptx_adaptive_tws_snk_sep = {AV_SEID_APTX_ADAPTIVE_SNK, DECODE_RESOURCE_ID, sep_media_type_audio, a2dp_sink, TRUE, 0, sizeof(aptx_adaptive_tws_snk_caps), aptx_adaptive_tws_snk_caps};
/*!@} */

/*!@{ \name TWS source endpoints
    \brief Predefined endpoints for audio Source end point configurations, for forwarding audio */
    /*! APTX mono */
const sep_config_type av_aptx_mono_tws_src_sep = {AV_SEID_APTX_MONO_TWS_SRC, ENCODE_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(aptx_tws_caps), aptx_tws_caps};
    /*! SBC mono */
const sep_config_type av_sbc_mono_tws_src_sep  = {AV_SEID_SBC_MONO_TWS_SRC,  ENCODE_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_tws_caps), sbc_tws_caps};
    /*! AAC mono */
const sep_config_type av_aac_stereo_tws_src_sep = {AV_SEID_AAC_STEREO_TWS_SRC,  ENCODE_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(aac_tws_caps), aac_tws_caps};
/*!@} */

/**/
const sep_config_type av_sbc_src_sep     = {AV_SEID_SBC_SRC,      ENCODE_RESOURCE_ID, sep_media_type_audio, a2dp_source, FALSE, 0, sizeof(sbc_caps_src),  sbc_caps_src};

/*! \brief Update sample rate in an SBC Mono TWS capability

    \param  caps        Pointer to a capability record. Such as retrieved by appA2dpFindServiceCategory(), 
                        \b see av_headset_a2dp.c.
    \param  sample_rate Rate we want to set.
 */
void appAvUpdateSbcMonoTwsCapabilities(uint8 *caps, uint32_t sample_rate)
{
    caps[14] &= ~(SBC_SAMPLING_FREQ_48000 | SBC_SAMPLING_FREQ_44100);
    switch (sample_rate)
    {
        case 48000:
            caps[14] |= SBC_SAMPLING_FREQ_48000;
            break;

        case 44100:
            caps[14] |= SBC_SAMPLING_FREQ_44100;
            break;

        default:
            Panic();
            break;
    }
}

/*! \brief Update sample rate in an SBC capability

    \param  caps        Pointer to a capability record. Such as retrieved by appA2dpFindServiceCategory(),
                        \b see av_headset_a2dp.c.
    \param  sample_rate Rate we want to set.
 */
void appAvUpdateSbcCapabilities(uint8 *caps, uint32_t sample_rate)
{
    caps[4] &= ~(SBC_SAMPLING_FREQ_48000 | SBC_SAMPLING_FREQ_44100);
    switch (sample_rate)
    {
        case 48000:
            caps[4] |= SBC_SAMPLING_FREQ_48000;
            break;

        case 44100:
            caps[4] |= SBC_SAMPLING_FREQ_44100;
            break;

        default:
            Panic();
            break;
    }
}

/*! \brief Update sample rate in an APTX Mono TWS capability

    \param  caps        Pointer to a capability record. Such as retrieved by appA2dpFindServiceCategory(), 
                        \b see av_headset_a2dp.c.
    \param  sample_rate Rate we want to set.
 */
void appAvUpdateAptxMonoTwsCapabilities(uint8 *caps, uint32_t sample_rate)
{
    caps[10] &= ~(APTX_SAMPLING_FREQ_48000 | APTX_SAMPLING_FREQ_44100);
    switch (sample_rate)
    {
        case 48000:
            caps[10] |= APTX_SAMPLING_FREQ_48000;
            break;

        case 44100:
            caps[10] |= APTX_SAMPLING_FREQ_44100;
            break;

        default:
            Panic();
            break;
    }
}
