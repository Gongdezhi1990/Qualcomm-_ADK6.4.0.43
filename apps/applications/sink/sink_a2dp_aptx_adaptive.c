/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   sink a2dp related functions specific to aptX Adaptive capability
*/

#include "sink_a2dp_aptx_adaptive.h"
#include "sink_a2dp_capabilities.h"
#include "sink_configmanager.h"
#include "sink_a2dp_config_def.h"

#include <byte_utils.h>
#include <audio_config.h>
#include <a2dp.h>

#ifdef INCLUDE_APTX_ADAPTIVE

/*!
 * The number of octets the aptX Adaptive service capability
 */
typedef enum
{
    length_of_aptx_adaptive_cap = 42,
    length_of_tws_aptx_adaptive_cap = OFFSET_FOR_LENGTH_OF_TWS_CAPS + length_of_aptx_adaptive_cap
} length_of_aptx_ad_service_capability_t;

/*!
 * Bits to set for each sampling rate of the aptX Adaptive service capability
 */
typedef enum
{
    aptx_ad_sample_rate_48000 = (1 << 4),
    aptx_ad_sample_rate_44100 = (1 << 3)
} aptx_ad_sample_rates_t;

typedef enum
{
    aptx_ad_setup_preference_1 = 0,
    aptx_ad_setup_preference_2 = 1,
    aptx_ad_setup_preference_3 = 2,
    aptx_ad_setup_preference_4 = 3
} setup_preference_priority_t;

#define APTX_AD_CAPABILITY_EXTENSION_END (0xAA)

/* Max TTP latency values in the service capability are not in units of 1ms */
#define MAX_TTP_LATENCY_UNIT_IN_MS (4)

#define REPEAT_OCTET_5_TIMES(x)  (x), (x), (x), (x), (x)
#define REPEAT_OCTET_10_TIMES(x) REPEAT_OCTET_5_TIMES(x), REPEAT_OCTET_5_TIMES(x)
#define REPEAT_OCTET_30_TIMES(x) REPEAT_OCTET_10_TIMES(x), REPEAT_OCTET_10_TIMES(x), REPEAT_OCTET_10_TIMES(x)
#define REPEAT_OCTET_32_TIMES(x) REPEAT_OCTET_30_TIMES(x), (x), (x)

#define APTX_AD_EMBEDDED_SERVICE_CAPABILITY \
    AVDTP_SERVICE_MEDIA_CODEC, \
    length_of_aptx_adaptive_cap, \
    AVDTP_MEDIA_TYPE_AUDIO << 2, \
    AVDTP_MEDIA_CODEC_NONA2DP,\
    SPLIT_IN_4_OCTETS(A2DP_QTI_VENDOR_ID), \
    SPLIT_IN_2_OCTETS(A2DP_QTI_APTX_AD_CODEC_ID), \
    aptx_ad_sample_rate_48000 | aptx_ad_sample_rate_44100, \
    aptx_ad_channel_mode_joint_stereo, \
    REPEAT_OCTET_32_TIMES(RESERVED), \
    AVDTP_SERVICE_CONTENT_PROTECTION, \
    LENGTH_OF_CP_TYPE_SCMS_VALUE, \
    AVDTP_CP_TYPE_SCMS_LSB, \
    AVDTP_CP_TYPE_SCMS_MSB, \
    AVDTP_SERVICE_DELAY_REPORTING, \
    0

/*!
 * Default aptX Adaptive Capabilities for the application to pass to the A2DP library during initialisation.
 *  NOTE: The capability is modified by sinkA2dpAptxAdInitServiceCapability() before passing it to the A2DP library,
 *  therefore it is the end result of this modification that reflects the "real" service capability and this array
 *  initialisation is merely used as a base to simplify the code it.
 *  The octets populated by sinkA2dpAptxAdInitServiceCapability() are initialised here as RESERVED (as well as octets
 *  that are actually reserved for future use).
 */
uint8 aptx_ad_caps_sink[52] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    APTX_AD_EMBEDDED_SERVICE_CAPABILITY
};

/*!
 * True Wireless Stereo service capability for aptX Adaptive
 */
const uint8 tws_aptx_ad_caps[62] =
{
    AVDTP_SERVICE_MEDIA_TRANSPORT,
    0,
    AVDTP_SERVICE_MEDIA_CODEC,
    length_of_tws_aptx_adaptive_cap,
    AVDTP_MEDIA_TYPE_AUDIO << 2,
    AVDTP_MEDIA_CODEC_NONA2DP,
    SPLIT_IN_4_OCTETS(A2DP_CSR_VENDOR_ID),
    SPLIT_IN_2_OCTETS(A2DP_CSR_TWS_APTX_AD_CODEC_ID),
    APTX_AD_EMBEDDED_SERVICE_CAPABILITY
};

static uint8 original_channel_mode;

static aptx_ad_advertise_options_t getAdvertiseOption(void)
{
    aptx_ad_advertise_options_t option_selected = aptx_ad_advertise_r1;
    a2dp_config_def_t *a2dp_config_data;

    if (configManagerGetReadOnlyConfig(A2DP_CONFIG_BLK_ID, (const void **) &a2dp_config_data))
        option_selected = a2dp_config_data->aptx_ad_a2dp_cap;

    configManagerReleaseConfig(A2DP_CONFIG_BLK_ID);

    return option_selected;
}

static bool isSinkInMonoChannelMode(void)
{
    return (AudioConfigGetRenderingMode() == single_channel_rendering);
}

static uint8 getChannelModeToBeUsedForTwsMode(void)
{
    if (isSinkInMonoChannelMode())
        return aptx_ad_channel_mode_tws_mono;
    else
        return aptx_ad_channel_mode_tws_stereo;
}

static uint8 * getStartOfCodecSpecificInformation(void)
{
    uint8 *service_caps = aptx_ad_caps_sink;

    while (service_caps[0] != AVDTP_SERVICE_MEDIA_CODEC)
        service_caps += service_caps[1] + 2;

    return service_caps;
}

static uint8 getChannelModeFromServiceCapability(void)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    return aptx_adaptive_codec_caps[aptx_ad_channel_mode_offset];
}

static void setChannelModeInServiceCapability(uint8 channel_mode)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_channel_mode_offset] = channel_mode;
}

static void setLowLatencyTtpBounds(ttp_latency_t ttp_latencies)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_ll_ttp_min_offset] = ttp_latencies.min_in_ms;
    aptx_adaptive_codec_caps[aptx_ad_ll_ttp_max_offset] = ttp_latencies.max_in_ms / MAX_TTP_LATENCY_UNIT_IN_MS;
}

static void setHighQualityTtpBounds(ttp_latency_t ttp_latencies)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_hq_ttp_min_offset] = ttp_latencies.min_in_ms;
    aptx_adaptive_codec_caps[aptx_ad_hq_ttp_max_offset] = ttp_latencies.max_in_ms / MAX_TTP_LATENCY_UNIT_IN_MS;
}

static void setTwsTtpBounds(ttp_latency_t ttp_latencies)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_tws_ttp_min_offset] = ttp_latencies.min_in_ms;
    aptx_adaptive_codec_caps[aptx_ad_tws_ttp_max_offset] = ttp_latencies.max_in_ms / MAX_TTP_LATENCY_UNIT_IN_MS;
}

static void setCapabilityVersionNumber(uint8 version_number)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_version_number_offset] = version_number;
}

static void setSupportedFeatures(uint32 supported_features)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    ByteUtilsSet4Bytes(aptx_adaptive_codec_caps, aptx_ad_supported_features_start_offset, supported_features);
}

static void setSetupPreference(uint8 setup_preference, setup_preference_priority_t priority)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_setup_preference_start_offset + priority] = setup_preference;
}

static void setCapabilityExtensionEndForR1(void)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_capability_extension_end_offset_for_r1] = APTX_AD_CAPABILITY_EXTENSION_END;
}

static void setCapabilityExtensionEndForR2(void)
{
    uint8 *aptx_adaptive_codec_caps = getStartOfCodecSpecificInformation();
    aptx_adaptive_codec_caps[aptx_ad_capability_extension_end_offset_for_r2] = APTX_AD_CAPABILITY_EXTENSION_END;
}

static void updateCapabilityBasedOnTheAdvertiseOption(aptx_ad_advertise_options_t advertise_option)
{
    switch (advertise_option)
    {
        case aptx_ad_advertise_r2:
            setCapabilityVersionNumber(0x01);
            setSupportedFeatures(0x0000000F);
            setSetupPreference(0x02, aptx_ad_setup_preference_1);
            setSetupPreference(0x03, aptx_ad_setup_preference_2);
            setSetupPreference(0x03, aptx_ad_setup_preference_3);
            setSetupPreference(0x03, aptx_ad_setup_preference_4);
            setCapabilityExtensionEndForR2();
            break;

        case aptx_ad_advertise_r1:
        default:
            setCapabilityVersionNumber(0x00);
            setCapabilityExtensionEndForR1();
            break;
    }
}

void sinkA2dpAptxAdInitServiceCapability(void)
{
    if (isSinkInMonoChannelMode())
        setChannelModeInServiceCapability(getChannelModeToBeUsedForTwsMode());

    setLowLatencyTtpBounds(AudioConfigGetA2DPTtpLatency());
    setHighQualityTtpBounds(AudioConfigGetA2DPTtpLatency());
    setTwsTtpBounds(AudioConfigGetTWSTtpLatency());
    updateCapabilityBasedOnTheAdvertiseOption(getAdvertiseOption());

    original_channel_mode = getChannelModeFromServiceCapability();
}

bool sinkA2dpAptxAdUpdateServiceCapWhenEnteringTwsMode(void)
{
    uint8 tws_channel_mode = getChannelModeToBeUsedForTwsMode();

    if (getChannelModeFromServiceCapability() != tws_channel_mode)
    {
        setChannelModeInServiceCapability(tws_channel_mode);
        return TRUE;
    }

    return FALSE;
}

bool sinkA2dpAptxAdRestoreServiceCapWhenExitingTwsMode(void)
{
    if (getChannelModeFromServiceCapability() != original_channel_mode)
    {
        setChannelModeInServiceCapability(original_channel_mode);
        return TRUE;
    }

    return FALSE;
}

#endif /* INCLUDE_APTX_ADAPTIVE */
