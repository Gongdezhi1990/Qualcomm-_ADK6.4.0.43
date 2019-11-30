/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    sink_a2dp_capabilities.c
 
DESCRIPTION
    The data structures used for the capability exchange with a source device.
*/

#include "sink_a2dp_capabilities.h"
#include "a2dp.h"

/*!
 * The number of octets in each codec service capability
 */
typedef enum
{
    length_of_sbc_cap = 6,
    length_of_mp3_cap = 6,
    length_of_aac_cap = 8,
    length_of_faststream_cap = 10,
    length_of_aptx_cap = 9,
    length_of_aptx_ll_cap = 19,
    length_of_aptx_hd_cap = 13,
    length_of_tws_sbc_cap = OFFSET_FOR_LENGTH_OF_TWS_CAPS + length_of_sbc_cap,
    length_of_tws_mp3_cap = OFFSET_FOR_LENGTH_OF_TWS_CAPS + length_of_mp3_cap,
    length_of_tws_aac_cap = OFFSET_FOR_LENGTH_OF_TWS_CAPS + length_of_aac_cap,
    length_of_tws_aptx_cap = OFFSET_FOR_LENGTH_OF_TWS_CAPS + length_of_aptx_cap
} length_of_codec_service_capability_t;


/*! @name SBC configuration bit fields
*/
/*@{ */
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


/*! @name Default SBC Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.

    Support all features and full bitpool range. Note that we trust the source
    to choose a bitpool value suitable for the Bluetooth bandwidth.
*/
const uint8 sbc_caps_sink[16] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_sbc_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_16000     | SBC_SAMPLING_FREQ_32000    | SBC_SAMPLING_FREQ_44100    | SBC_SAMPLING_FREQ_48000    |
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*! @name SBC medium quality sink caps.
    Support all features and the max bitpool medium quality.
*/
const uint8 sbc_med_caps_sink[16] = {
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_sbc_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_SBC,

    SBC_SAMPLING_FREQ_16000     | SBC_SAMPLING_FREQ_32000    | SBC_SAMPLING_FREQ_44100    | SBC_SAMPLING_FREQ_48000    |
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,

    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,

    SBC_BITPOOL_MIN,
    SBC_BITPOOL_MEDIUM_QUALITY,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*! @name MP3 configuration bit fields
*/
/*@{ */
/*! [Octet 0] Support for Layer I (mp1) */
#define MP3_LAYER_I                    128
/*! [Octet 0] Support for Layer II (mp2) */
#define MP3_LAYER_II                    64
/*! [Octet 0] Support for Layer III (mp3) */
#define MP3_LAYER_III                   32
/*! [Octet 0] Support for CRC Protection */
#define MP3_CRC_PROTECTION              16
/*! [Octet 0] Support for Mono channel mode */
#define MP3_CHANNEL_MODE_MONO            8
/*! [Octet 0] Support for Dual channel mode */
#define MP3_CHANNEL_MODE_DUAL_CHAN       4
/*! [Octet 0] Support for Stereo channel mode */
#define MP3_CHANNEL_MODE_STEREO          2
/*! [Octet 0] Support for Joint stereo channel mode */
#define MP3_CHANNEL_MODE_JOINT_STEREO    1

/*! [Octet 1] Support for 16kHz sampling frequency */
#define MP3_SAMPLING_FREQ_16000         32
/*! [Octet 1] Support for 22050Hz sampling frequency */
#define MP3_SAMPLING_FREQ_22050         16
/*! [Octet 1] Support for 24kHz sampling frequency */
#define MP3_SAMPLING_FREQ_24000          8
/*! [Octet 1] Support for 32kHz sampling frequency */
#define MP3_SAMPLING_FREQ_32000          4
/*! [Octet 1] Support for 44.1kHz sampling frequency */
#define MP3_SAMPLING_FREQ_44100          2
/*! [Octet 1] Support for 48kHz sampling frequency */
#define MP3_SAMPLING_FREQ_48000          1

/*! [Octet 2] Support for Variable bit rate */
#define MP3_VBR                        128
/*! [Octet 2] Support for bit rate 1110 */
#define MP3_BITRATE_VALUE_1110          64
/*! [Octet 2] Support for bit rate 1101 */
#define MP3_BITRATE_VALUE_1101          32
/*! [Octet 2] Support for bit rate 1100 */
#define MP3_BITRATE_VALUE_1100          16
/*! [Octet 2] Support for bit rate 1011 */
#define MP3_BITRATE_VALUE_1011           8
/*! [Octet 2] Support for bit rate 1010 */
#define MP3_BITRATE_VALUE_1010           4
/*! [Octet 2] Support for bit rate 1001 */
#define MP3_BITRATE_VALUE_1001           2
/*! [Octet 2] Support for bit rate 1000 */
#define MP3_BITRATE_VALUE_1000           1

/*! [Octet 3] Support for bit rate 0111 */
#define MP3_BITRATE_VALUE_0111         128
/*! [Octet 3] Support for bit rate 0110 */
#define MP3_BITRATE_VALUE_0110          64
/*! [Octet 3] Support for bit rate 0101 */
#define MP3_BITRATE_VALUE_0101          32
/*! [Octet 3] Support for bit rate 0100 */
#define MP3_BITRATE_VALUE_0100          16
/*! [Octet 3] Support for bit rate 0011 */
#define MP3_BITRATE_VALUE_0011           8
/*! [Octet 3] Support for bit rate 0010 */
#define MP3_BITRATE_VALUE_0010           4
/*! [Octet 3] Support for bit rate 0001 */
#define MP3_BITRATE_VALUE_0001           2
/*! [Octet 3] Support for bit rate 0000 */
#define MP3_BITRATE_VALUE_FREE           1
/*@} */


/*! @name Default MP3 Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.

    Support all features except:
    - MPEG-1 Layer 3
    - MPF-1
    - free rate
*/
const uint8 mp3_caps_sink[16] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_mp3_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,

    MP3_LAYER_III | MP3_CRC_PROTECTION | MP3_CHANNEL_MODE_MONO | MP3_CHANNEL_MODE_DUAL_CHAN | MP3_CHANNEL_MODE_STEREO | MP3_CHANNEL_MODE_JOINT_STEREO,

    MP3_SAMPLING_FREQ_16000 | MP3_SAMPLING_FREQ_22050 | MP3_SAMPLING_FREQ_24000 | MP3_SAMPLING_FREQ_32000 | MP3_SAMPLING_FREQ_44100 | MP3_SAMPLING_FREQ_48000,

    MP3_VBR                    | MP3_BITRATE_VALUE_1110      | MP3_BITRATE_VALUE_1101      | MP3_BITRATE_VALUE_1100    |
    MP3_BITRATE_VALUE_1011     | MP3_BITRATE_VALUE_1010      | MP3_BITRATE_VALUE_1001      | MP3_BITRATE_VALUE_1000,

    MP3_BITRATE_VALUE_0111     | MP3_BITRATE_VALUE_0110      | MP3_BITRATE_VALUE_0101      | MP3_BITRATE_VALUE_0100    |
    MP3_BITRATE_VALUE_0011     | MP3_BITRATE_VALUE_0010      | MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*! @name AAC/AAC+ configuration bit fields
*/
/*@{ */
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
/*! [Octet 3] Support for 320kbps (320000bps) = 0x04E200 */
#define AAC_BITRATE_256_3       (0x04)

/*! [Octet 4] Support for 320kbps (320000bps) */
#define AAC_BITRATE_256_4       (0xE2)

/*! [Octet 5] Support for 320kbps (320000bps) */
#define AAC_BITRATE_256_5       (0x00)
/*@} */


/*! @name Default AAC/AAC+ Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.

    Support all features.
*/
const uint8 aac_caps_sink[18] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aac_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG2_4_AAC,

    AAC_MPEG2_AAC_LC | AAC_MPEG4_AAC_LC,
    AAC_SAMPLE_8000 | AAC_SAMPLE_11025 | AAC_SAMPLE_12000 | AAC_SAMPLE_16000 | AAC_SAMPLE_22050 | AAC_SAMPLE_24000 | AAC_SAMPLE_32000 | AAC_SAMPLE_44100,
    AAC_SAMPLE_48000 | AAC_CHANNEL_1 | AAC_CHANNEL_2,
    AAC_VBR | AAC_BITRATE_256_3,
    AAC_BITRATE_256_4,
    AAC_BITRATE_256_5,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*! @name SBC configuration bit fields
*/
/*@{ */
/*! [Octet 6] Support for music */
#define FASTSTREAM_MUSIC              0x01
/*! [Octet 6] Support for voice */
#define FASTSTREAM_VOICE              0x02
/*! [Octet 7] Support for 48.0kHz sampling frequency */
#define FASTSTREAM_MUSIC_SAMP_48000   0x01
/*! [Octet 7] Support for 44.1kHz sampling frequency */
#define FASTSTREAM_MUSIC_SAMP_44100   0x02
/*! [Octet 7] Support for 16kHz sampling frequency */
#define FASTSTREAM_VOICE_SAMP_16000   0x20
/*@} */


/*! @name Default Fastream Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.
*/
const uint8 faststream_caps_sink[14] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_faststream_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_FASTSTREAM_CODEC_ID),

    FASTSTREAM_MUSIC | FASTSTREAM_VOICE,
    FASTSTREAM_MUSIC_SAMP_48000 | FASTSTREAM_MUSIC_SAMP_44100 | FASTSTREAM_VOICE_SAMP_16000
};


/*! @name Common APTX configuration bit fields
*/
/*@{ */
#define APTX_SAMPLING_FREQ_44100 (1 << 5)
#define APTX_SAMPLING_FREQ_48000 (1 << 4)
#define APTX_CHANNEL_MODE_STEREO (1 << 1)
/*@} */


/*! @name Default apt-X Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.
*/
const uint8 aptx_caps_sink[19] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aptx_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_APT_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_APTX_CODEC_ID),

    APTX_SAMPLING_FREQ_44100 | APTX_SAMPLING_FREQ_48000 | APTX_CHANNEL_MODE_STEREO,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,

    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


#define APTX_ACL_SPRINT_SBC_BIDIR       0x1
#define APTX_ACL_SPRINT_NEW_CAPS        0x2

/*! @name Default apt-X Sprint Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.
*/
const uint8 aptx_acl_sprint_caps_sink[27] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aptx_ll_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_APTX_ACL_SPRINT_CODEC_ID),

    APTX_SAMPLING_FREQ_44100 | APTX_SAMPLING_FREQ_48000 | APTX_CHANNEL_MODE_STEREO,

#ifdef CVC_BACK_CHANNEL
    APTX_ACL_SPRINT_SBC_BIDIR | APTX_ACL_SPRINT_NEW_CAPS,
#else
    APTX_ACL_SPRINT_NEW_CAPS,
#endif
    RESERVED, /* Decoder doesn't store any values */
    RESERVED, /* These are just sent from the encoder */
    RESERVED, /* during caps exchange */
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB
};


#define APTX_LL_SBC_BIDIR       0x1
#define APTX_LL_NEW_CAPS        0x2

/*! @name Default apt-X LL Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.
*/
const uint8 aptx_ll_caps_sink[27] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aptx_ll_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_QTI_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_QTI_APTX_LL_CODEC_ID),

    APTX_SAMPLING_FREQ_44100 | APTX_SAMPLING_FREQ_48000 | APTX_CHANNEL_MODE_STEREO,

#ifdef CVC_BACK_CHANNEL
    APTX_LL_SBC_BIDIR | APTX_LL_NEW_CAPS,
#else
    APTX_LL_NEW_CAPS,
#endif
    RESERVED, /* Decoder doesn't store any values */
    RESERVED, /* These are just sent from the encoder */
    RESERVED, /* during caps exchange */
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB
};


/*! @name Default aptX-HD Capabilities
    Default capabilities that an application can pass to the A2DP library during initialisation.
*/
const uint8 aptxhd_caps_sink[23] = 
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aptx_hd_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,
    
    SPLIT_IN_4_OCTETS(A2DP_QTI_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_QTI_APTXHD_CODEC_ID),
    
    APTX_SAMPLING_FREQ_44100 | APTX_SAMPLING_FREQ_48000 | APTX_CHANNEL_MODE_STEREO,
    
    RESERVED,
    RESERVED,
    RESERVED,
    RESERVED,

    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,
    
    AVDTP_SERVICE_DELAY_REPORTING,
    0
};

/*!
    @brief The CSR True Wireless Stereo Codec ID for SBC.
*/
const uint8 tws_sbc_caps[26] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_tws_sbc_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_TWS_SBC_CODEC_ID),

    /* The embedded capability needs to be the same as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_sbc_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_SBC,
    SBC_SAMPLING_FREQ_16000     | SBC_SAMPLING_FREQ_32000    | SBC_SAMPLING_FREQ_44100    | SBC_SAMPLING_FREQ_48000    |
    SBC_CHANNEL_MODE_MONO       | SBC_CHANNEL_MODE_DUAL_CHAN | SBC_CHANNEL_MODE_STEREO    | SBC_CHANNEL_MODE_JOINT_STEREO,
    SBC_BLOCK_LENGTH_4          | SBC_BLOCK_LENGTH_8         | SBC_BLOCK_LENGTH_12        | SBC_BLOCK_LENGTH_16        |
    SBC_SUBBANDS_4              | SBC_SUBBANDS_8             | SBC_ALLOCATION_SNR         | SBC_ALLOCATION_LOUDNESS,
    SBC_BITPOOL_MIN,
    SBC_BITPOOL_HIGH_QUALITY,

    /* The TWS capability needs to support the same services as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,
    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*!
    @brief The CSR True Wireless Stereo Codec ID for MP3.
*/
const uint8 tws_mp3_caps[26] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_tws_mp3_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_TWS_MP3_CODEC_ID),

    /* The embedded capability needs to be the same as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_mp3_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO,
    MP3_LAYER_III | MP3_CRC_PROTECTION | MP3_CHANNEL_MODE_MONO | MP3_CHANNEL_MODE_DUAL_CHAN | MP3_CHANNEL_MODE_STEREO | MP3_CHANNEL_MODE_JOINT_STEREO,
    MP3_SAMPLING_FREQ_16000 | MP3_SAMPLING_FREQ_22050 | MP3_SAMPLING_FREQ_24000 | MP3_SAMPLING_FREQ_32000 | MP3_SAMPLING_FREQ_44100 | MP3_SAMPLING_FREQ_48000,
    MP3_VBR                    | MP3_BITRATE_VALUE_1110      | MP3_BITRATE_VALUE_1101      | MP3_BITRATE_VALUE_1100    |
    MP3_BITRATE_VALUE_1011     | MP3_BITRATE_VALUE_1010      | MP3_BITRATE_VALUE_1001      | MP3_BITRATE_VALUE_1000,
    MP3_BITRATE_VALUE_0111     | MP3_BITRATE_VALUE_0110      | MP3_BITRATE_VALUE_0101      | MP3_BITRATE_VALUE_0100    |
    MP3_BITRATE_VALUE_0011     | MP3_BITRATE_VALUE_0010      | MP3_BITRATE_VALUE_0001,    /* All bit rates except 'free' */

    /* The TWS capability needs to support the same services as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,
    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*!
    @brief The CSR True Wireless Stereo Codec ID for AAC.
*/
const uint8 tws_aac_caps[28] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_tws_aac_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_TWS_AAC_CODEC_ID),

    /* The embedded capability needs to be the same as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aac_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_MPEG2_4_AAC,
    AAC_MPEG2_AAC_LC | AAC_MPEG4_AAC_LC,
    AAC_SAMPLE_8000 | AAC_SAMPLE_11025 | AAC_SAMPLE_12000 | AAC_SAMPLE_16000 | AAC_SAMPLE_22050 | AAC_SAMPLE_24000 | AAC_SAMPLE_32000 | AAC_SAMPLE_44100,
    AAC_SAMPLE_48000 | AAC_CHANNEL_1 | AAC_CHANNEL_2,
    AAC_VBR | AAC_BITRATE_256_3,
    AAC_BITRATE_256_4,
    AAC_BITRATE_256_5,

    /* The TWS capability needs to support the same services as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,
    AVDTP_SERVICE_DELAY_REPORTING,
    0
};


/*!
    @brief The CSR True Wireless Stereo Codec ID for AptX.
*/
const uint8 tws_aptx_caps[29] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_tws_aptx_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,

    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_TWS_APTX_CODEC_ID),

    /* The embedded capability needs to be the same as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_aptx_cap,
    AVDTP_MEDIA_TYPE_AUDIO<<2,
    AVDTP_MEDIA_CODEC_NONA2DP,
    SPLIT_IN_4_OCTETS(A2DP_APT_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_APTX_CODEC_ID),
    APTX_SAMPLING_FREQ_44100 | APTX_SAMPLING_FREQ_48000 | APTX_CHANNEL_MODE_STEREO,

    /* The TWS capability needs to support the same services as the respective standard a2dp capability to ensure they are compatible */
    AVDTP_SERVICE_CONTENT_PROTECTION,
    LENGTH_OF_CP_TYPE_SCMS_VALUE,
    AVDTP_CP_TYPE_SCMS_LSB,
    AVDTP_CP_TYPE_SCMS_MSB,
    AVDTP_SERVICE_DELAY_REPORTING,
    0
};
