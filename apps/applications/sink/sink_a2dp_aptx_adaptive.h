/*
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/

/*!
@file
@ingroup sink_app
@brief   sink a2dp related functions specific to aptX Adaptive capability
*/

#ifndef _SINK_A2DP_APTX_ADAPTIVE_H_
#define _SINK_A2DP_APTX_ADAPTIVE_H_

#ifdef INCLUDE_APTX_ADAPTIVE

/* Cause a build error if ENABLE_AUDIO_TTP is not defined */
#ifndef ENABLE_AUDIO_TTP
#error ENABLE_AUDIO_TTP must be enabled for aptX Adaptive
#endif

extern uint8 aptx_ad_caps_sink[52];
extern const uint8 tws_aptx_ad_caps[62];

/*!
 *  These enum values must match the aptX Adaptive A2DP capability options defined in the config xml files.
 */
typedef enum
{
    aptx_ad_advertise_r1 = 0,
    aptx_ad_advertise_r2 = 1
} aptx_ad_advertise_options_t;

/*!
 * Bits to set for each channel mode of the aptX Adaptive service capability
 */
typedef enum {
    aptx_ad_channel_mode_stereo       = (1 << 1),
    aptx_ad_channel_mode_tws_stereo   = (1 << 2),
    aptx_ad_channel_mode_joint_stereo = (1 << 3),
    aptx_ad_channel_mode_tws_mono     = (1 << 4)
} aptx_ad_channel_mode_masks_t;

/*!
 * Octet offset from AVDTP_SERVICE_MEDIA_CODEC in aptX Adaptive decoder service capability.
 */
typedef enum {
    aptx_ad_channel_mode_offset                     = 11,
    aptx_ad_ll_ttp_min_offset                       = 12,
    aptx_ad_ll_ttp_max_offset                       = 13,
    aptx_ad_hq_ttp_min_offset                       = 14,
    aptx_ad_hq_ttp_max_offset                       = 15,
    aptx_ad_tws_ttp_min_offset                      = 16,
    aptx_ad_tws_ttp_max_offset                      = 17,
    aptx_ad_version_number_offset                   = 19,
    aptx_ad_capability_extension_end_offset_for_r1  = 20,
    aptx_ad_supported_features_start_offset         = 20,
    aptx_ad_setup_preference_start_offset           = 24,
    aptx_ad_capability_extension_end_offset_for_r2  = 29
} octet_offsets_in_aptx_ad_decoder_specific_caps_t;

/*************************************************************************
NAME
    sinkA2dpAptxAdInitServiceCapability

DESCRIPTION
    Initialise the aptx adaptive service capability based on configuration
    data from the Config Tool.

RETURNS
    None

**************************************************************************/
void sinkA2dpAptxAdInitServiceCapability(void);

/*************************************************************************
NAME
    sinkA2dpAptxAdUpdateServiceCapWhenEnteringTwsMode

DESCRIPTION
    Update the aptX adaptive service capability for TWS mode

RETURNS
    TRUE when capability changed, FALSE otherwise

**************************************************************************/
bool sinkA2dpAptxAdUpdateServiceCapWhenEnteringTwsMode(void);

/*************************************************************************
NAME
    sinkA2dpAptxAdRestoreServiceCapWhenExitingTwsMode

DESCRIPTION
    Restore aptX adaptive service capability when exiting TWS mode

RETURNS
    TRUE when capability changed, FALSE otherwise

**************************************************************************/
bool sinkA2dpAptxAdRestoreServiceCapWhenExitingTwsMode(void);

#else

#define sinkA2dpAptxAdInitServiceCapability() ((void)(0))
#define sinkA2dpAptxAdUpdateServiceCapWhenEnteringTwsMode() (FALSE)
#define sinkA2dpAptxAdRestoreServiceCapWhenExitingTwsMode() (FALSE)

#endif /* INCLUDE_APTX_ADAPTIVE */

#endif /* _SINK_A2DP_APTX_ADAPTIVE_H_ */
