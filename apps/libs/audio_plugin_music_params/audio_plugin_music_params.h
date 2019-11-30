/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_plugin_music_params.h

DESCRIPTION
    Music (shared by all types of inputs) connect parameters definition.
*/

#ifndef AUDIO_PLUGIN_MUSIC_PARAMS_H_
#define AUDIO_PLUGIN_MUSIC_PARAMS_H_

#include <csrtypes.h>
#include <a2dp.h>
#include <audio_plugin_if.h>

/* audio mode pararmeters external mic settings, applicable to APTX ll + back channel application only */
#define SEND_PATH_MUTE 0
#define SEND_PATH_UNMUTE 1
#define EXTERNAL_MIC_NOT_FITTED 1
#define EXTERNAL_MIC_FITTED 2

typedef struct
{
    A2DP_MUSIC_PROCESSING_T music_mode_processing;
    uint16 music_mode_enhancements;
    unsigned unused:8;
    unsigned external_volume_enabled:1;
    unsigned master_routing_mode:2;
    unsigned slave_routing_mode:2;
    unsigned external_mic_settings:2;
    unsigned mic_mute:1;
}A2dpPluginModeParams;

#define LATENCY_LAST_MULTIPLIER   1     /* 1ms resolution */
#define LATENCY_CHANGE_MULTIPLIER 5     /* 5ms resolution */
#define LATENCY_TARGET_MULTIPLIER 5     /* 5ms resolution */
#define LATENCY_PERIOD_MULTIPLIER 100   /* 100ms resolution */

typedef struct
{
    unsigned last:12;        /* Last measured latency for the current codec, in ms. Zero implies no known last latency. */
    unsigned change:4;       /* Minimum latency change before a report issued, in multiple of 5ms. */
    unsigned target:8;       /* Target latency, in multiple of 5ms.  Zero implies no target latency. */
    unsigned period:8;       /* Minimum time between reports, in multiple of 100ms.  Zero means latency reporting disabled. */
} A2dpPluginLatencyParams;

typedef struct
{
    uint16 clock_mismatch;
    uint16 packet_size;
    uint16 silence_threshold ;   /* threshold (16 bit fractional value - aligned to MSB in DSP) */
    uint16 silence_trigger_time; /* trigger time in seconds (16 bit int) */
    A2dpPluginModeParams *mode_params;
    const voice_mic_params_t* mic_params;
    const analogue_input_params* analogue_in_params;
    usb_common_params* usb_params;
    AUDIO_SUB_TYPE_T sub_woofer_type;
    Sink sub_sink;
    A2dpPluginLatencyParams latency;
    unsigned sub_functionality_present:1;    /* flag to indicate that subwoofer has been compiled into the VM app, output is then forced to 48KHz */
    unsigned delay_volume_message_sending:1; /* flag used to delay the setting of the volume message to allow a soft transitional fade in */
    unsigned sub_is_available:1;     /* flag to indicate if a subwoofer signalling connection is present, used for delaying audio unmuting */
    unsigned sub_connection_state:2;
    unsigned content_protection:1;
    unsigned channel_mode:2;
    unsigned format:8;
    unsigned bitpool:8;
    unsigned peer_is_available:1;
    uint32 voice_rate;
    uint32 wired_audio_output_rate;
    aptx_sprint_params_type aptx_sprint_params;
    aptx_adaptive_params_t aptx_ad_params;
    spdif_ac3_configuration *spdif_ac3_config;
    spdif_configuration *spdif_config;
    Task ba_output_plugin;
} A2dpPluginConnectParams;

/* EQ constants */

#define A2DP_MUSIC_MAX_EQ_BANK (6)

/* The VM app should use this message to restore the EQ bank after a power cycle. */
#define A2DP_MUSIC_MSG_CUR_EQ_BANK   1
#define A2DP_MUSIC_MSG_ENHANCEMENTS  2

/* lower 3 bits of enchancement uint16 are the EQ selected */
#define A2DP_MUSIC_CONFIG_USER_EQ_SELECT 7

#define MUSIC_CONFIG_DATA_VALID         (0x8000)
#define MUSIC_CONFIG_CROSSOVER_BYPASS   (0x0800)
#define MUSIC_CONFIG_SPKR_EQ_BYPASS     (0x0400)
#define MUSIC_CONFIG_EQFLAT             (0x0200)
#define MUSIC_CONFIG_USER_EQ_BYPASS     (0x0100)
#define MUSIC_CONFIG_BASS_ENHANCE_BYPASS  (0x0080) /*Bass Boost and Bass Plus enhancements*/
#define MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS     (0x0040)  /*3D and 3DV enhancements*/
#define MUSIC_CONFIG_COMPANDER_BYPASS   (0x0020)
#define MUSIC_CONFIG_DITHER_BYPASS      (0x0010)
#define MUSIC_CONFIG_USER_EQ_SELECT     (0x0007)

/* Time To Play latencies. */
typedef struct
{
    uint16 min_in_ms;
    uint16 target_in_ms;
    uint16 max_in_ms;
} ttp_latency_t;

#endif /* AUDIO_PLUGIN_MUSIC_PARAMS_H_ */
