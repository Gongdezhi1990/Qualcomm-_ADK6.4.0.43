/*
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    This file handles all Synchronous connection messages
*/

/****************************************************************************
    Header files
*/
#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_audio.h"
#include "sink_statemanager.h"
#include "sink_pio.h"
#include "sink_tones.h"
#include "sink_volume.h"
#include "sink_speech_recognition.h"
#include "sink_wired.h"
#include "sink_display.h"
#include "sink_audio_routing.h"
#include "sink_devicemanager.h"
#include "sink_debug.h"
#include "sink_partymode.h"
#include "sink_anc.h"
#include "sink_fm.h"
#include "sink_config.h"
#include "sink_hfp_data.h"
#include "sink_usb.h"
#include "sink_peer.h"
#include "sink_callmanager.h"
#include "sink_ba_receiver.h"
#include "sink_malloc_debug.h"

#include "sink_swat.h"
#include "sink_avrcp.h"
#include "sink_ba.h"
#include "sink_powermanager.h"
#include "sink_anc.h"
#include "sink_va_if.h"
#include "sink_wired.h"
#include "sink_peer.h"
#include "sink_gaia.h"

#include <connection.h>
#include <a2dp.h>
#include <hfp.h>
#include <stdlib.h>
#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_config.h>
#include <sink.h>
#include <bdaddr.h>
#include <vm.h>
#include <config_store.h>
#include <byte_utils.h>
#include <broadcast_context.h>





#ifdef DEBUG_AUDIO
#define AUD_DEBUG(x) DEBUG(x)
#else
#define AUD_DEBUG(x)
#endif


/* Byte offsets of the User EQ bank - Pre Gain field*/
#define PRE_GAIN_LO_OFFSET 0
#define PRE_GAIN_HI_OFFSET 1

#define MAXIMUM_NUMBER_OF_SOURCES   8

/*Byte offsets used in mapping the parameters defined in the user_eq_band_t structure
 * into a uint8 array.
 */
typedef enum
{
    BAND_FILTER_OFFSET,
    BAND_FREQ_LO_OFFSET,
    BAND_FREQ_HI_OFFSET,
    BAND_GAIN_LO_OFFSET,
    BAND_GAIN_HI_OFFSET,
    BAND_Q_LO_OFFSET,
    BAND_Q_HI_OFFSET
}user_eq_band_param_offset_t;

/* User defined EQ bank settings */
typedef enum{
    BYPASS,
    FIRST_ORDER_LOW_PASS,
    FIRST_ORDER_HIGH_PASS,
    FIRST_ORDER_ALL_PASS,
    FIRST_ORDER_LOW_SHELF,
    FIRST_ORDER_HIGH_SHELF,
    FIRST_ORDER_TILT,
    SECOND_ORDER_LOW_PASS,
    SECOND_ORDER_HIGH_PASS,
    SECOND_ORDER_ALL_PASS,
    SECOND_ORDER_LOW_SHELF,
    SECOND_ORDER_HIGH_SHELF,
    SECOND_ORDER_TILT,
    PARAMETRIC_EQ,

    FORCE_ENUM_TO_MIN_16BIT(eq_filter)
}user_eq_filter_type_t;

typedef enum
{
    a2dp_priority_normal,
    a2dp_priority_reversed
} a2dp_source_priority_t;

/* Sink Audio global data */
typedef struct  __sink_audio_global_data_t
{
    sink_audio_writeable_config_def_t audio_session_data;
    audio_instance_t        routed_audio_instance;
    audio_instance_t        routed_voice_instance;
    audio_sources           routed_audio_source;        /* Currently routed audio source */
    voice_mic_params_t      voice_mic_params;
    analogue_input_params   analogue_input_params;
    a2dp_source_priority_t  a2dp_source_priority;
    unsigned                manual_source_selection:1;
    unsigned                :5;
    unsigned                gated_audio:8;              /* Bitmask indicating which audio sources are prevented from being routed */
    unsigned                audioAmpReady:1;            /* bit to indicate readiness of audio amplifier */
    unsigned                silence_detected:1;         /* Flag to indicates whether silence detected while streaming media.*/
}sink_audio_global_data_t;

static sink_audio_global_data_t gAudioData;

#define AUDIO_GDATA  gAudioData

/* Sink Audio writeable/session config data */
typedef struct __sink_audio_writeable_configdata_t
{
    user_eq_bank_t        *PEQ;
    /* runtime data for the currently routed audio source */
    audio_sources          requested_audio_source;
}sink_audio_writeable_configdata_t;



static audio_sources audioGetHighestPriorityAudioSourceAvailable(void);
static bool audioRouteScoOutsideOfCallIfAvailable(void);
static bool audioRouteFMIfAvailable(void);
static bool audioRouteActiveCallScoIfAvailable(void);
static bool audioRouteA2dpIfAvailable(audio_sources source);
static bool audioRouteUsbVoiceIfAvailable(void);
static bool audioRouteSource(audio_sources source);
static bool audioRouteAnalogIfAvailable(void);
static bool audioRouteSpdifIfAvailable(void);
static bool audioRouteI2SIfAvailable(void);
static bool audioRouteUsbIfAvailable(void);
static bool audioRouteBaIfAvailable(void);
static audio_sources getNextSourceInSequence(audio_sources source);
static audio_sources getAudioSourceFromEventUsrSelectAudioSource(const MessageId EventUsrSelectAudioSource);
static bool audioIsAudioSourceRoutable(audio_sources source);
static bool audioIsA2dpAudioRoutable(a2dp_index_t index);
static audio_sources audioGetAudioSourceToRoute(void);
static bool audioIsAudioSourceEnabledByUser(audio_sources source);
static audio_sources getNextRoutableSource(audio_sources current_source);
static bool audioRouteScoSink(Sink sco_sink_to_route);
static bool audioRouteUsbVoiceSink(Sink sink_to_route);
static int16 audioGetVolumeForSink(Sink sink);
static void audioRouteVoiceSource(void);
static bool isAudioSourceA2dp(audio_sources source);
static void sinkAudioSetRequestedAudioSource(audio_sources source);
static void sinkAudioReadMicParams(audio_mic_params *global, audio_mic_params_t *config);
static void sinkAudioReadAnalogueInputParams(analogue_input_params *global, analogue_input_params_t *config);
static void sinkAudioSetRoutedVoiceInstance(audio_instance_t instance);
static void updateVolumeBasedOnRoutedSources(void);
static void setA2dpSourcePriority(a2dp_source_priority_t priority);
static a2dp_source_priority_t getA2dpSourcePriority(void);
static void sinkAudioConfigurePcmInputs(void);
static void adjustVolumeIfBatteryLowLimitationInplace(volume_info *volume);
static bool handleAudioActiveDelayRequest(void);
static a2dp_index_t getA2dpIndexFromSource(audio_sources source);
static audio_sources getA2dpSourceFromIndex(a2dp_index_t index);
static audio_sources getAudioSourceFromConfiguredPriority(unsigned priority);
static void getSourcePriorities(audio_sources * source_priority);
static bool isCallIndicationActive(void);
static bool sinkAudioSilentAnalogueIsRouted(void);
static void audioSuspendDisconnectVoiceSource(void);

/*************************************************************************
NAME
    isAudioGated

DESCRIPTION
    Determines whether we should block the given source because either the
    input is gated for all states or the current state does not allow playing
    from this source.

    The state test is necessary for directly attached audio inputs because we
    have the option of whether to play them while in limbo.

INPUTS
    audio_gated_mask

RETURNS
    TRUE if the source should be gated
    FALSE otherwise

*/
bool isAudioGated (audio_gating audio_gated_mask)
{
    bool gated_in_all_states = !!(AUDIO_GDATA.gated_audio & audio_gated_mask);
    bool is_direct_connection = (audio_gated_mask & (audio_gate_usb | audio_gate_wired));
    bool in_limbo = (stateManagerGetState() == deviceLimbo);
    bool gated;

    AUD_DEBUG(("isAudioGated() check = 0x%X, current = 0x%X\n",audio_gated_mask, AUDIO_GDATA.gated_audio));
    /* For audio sources which are capable of playing in Limbo we must first check
       the current status of VM.*/
    if((audio_gate_wired == audio_gated_mask) || (audio_gate_usb == audio_gated_mask))
    {
        if (gated_in_all_states)
        {
            gated = TRUE;
        }
        else if (is_direct_connection)
        {
            gated = (in_limbo && (!SinkWiredPlayUsbAndWiredInLimbo()));
        }
        else
        {
            /*
             * As far as routing logic is concerned, source can be played.
             * (Note that whether or not the source is available is a separate
             * question - e.g. remote sources will not be connected in limbo)
             */
            gated = FALSE;
        }

        return gated;

    }
    else
        return gated_in_all_states;
}

audio_gating audioGateAudio (uint16 audio_gated_mask)
{
    AUDIO_GDATA.gated_audio |= audio_gated_mask;

    AUD_DEBUG(("audioGateAudio(0x%X)  0x%X\n",audio_gated_mask,AUDIO_GDATA.gated_audio));
    return AUDIO_GDATA.gated_audio;
}

audio_gating audioUngateAudio (uint16 audio_ungated_mask)
{
    AUDIO_GDATA.gated_audio &= ~audio_ungated_mask;

    AUD_DEBUG(("audioUngateAudio(0x%X)  0x%X\n",audio_ungated_mask, AUDIO_GDATA.gated_audio));
    return AUDIO_GDATA.gated_audio;
}

/****************************************************************************
NAME
    audioRouteIsAutoSuspendOnCallEnabled

DESCRIPTION
    Returns the status of auto suspend on call feature bit

RETURNS
    TRUE if auto suspend is enabled, FALSE otherwise
*/
static bool audioRouteIsAutoSuspendOnCallEnabled(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    bool auto_suspend_enabled = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        auto_suspend_enabled = read_data->AssumeAutoSuspendOnCall;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return auto_suspend_enabled;
}

/****************************************************************************
NAME
    sinkAudioSetRequestedAudioSource

DESCRIPTION
    Updates the requested Audio source.

RETURNS
    None
*/
static void sinkAudioSetRequestedAudioSource(audio_sources source)
{
    AUDIO_GDATA.audio_session_data.requested_audio_source = source;
}

/****************************************************************************
NAME
    getVoiceMicFromConfig

DESCRIPTION
    Adds the chosen voice mics from the config to global structure

RETURNS
    None
*/
static audio_mic_params_t* getVoiceMicFromConfig(mic_selection microphone, sink_audio_readonly_config_def_t *config)
{
    audio_mic_params_t *mic_params = &config->common_mic_params.mic_1;

    switch(microphone)
    {
        case microphone_1:
            mic_params = &config->common_mic_params.mic_1;
            break;
        case microphone_2:
            mic_params = &config->common_mic_params.mic_2;
            break;
        case microphone_3:
            mic_params = &config->common_mic_params.mic_3;
            break;
        case microphone_4:
            mic_params = &config->common_mic_params.mic_4;
            break;
        case microphone_5:
            mic_params = &config->common_mic_params.mic_5;
            break;
        case microphone_6:
            mic_params = &config->common_mic_params.mic_6;
            break;
        default:
            break;
    }

    return mic_params;
}

/****************************************************************************
NAME
      sinkAudioInit

DESCRIPTION
    Reads the PSKEY containing the audio routing information

RETURNS
      void
*/
void sinkAudioInit( void )
{
    sink_audio_readonly_config_def_t *read_data = NULL;

    /* Initialise global data to 0 */
    memset(&AUDIO_GDATA, 0, ( sizeof(sink_audio_global_data_t)));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        /* A working copy of the voice_mic_params_t is needed because other parts
           of the sink app / audio plugins use a pointer to the voice_mic_params_t
           inside sink_audio_routing instead of copying it by value. */

        sinkAudioReadMicParams(&AUDIO_GDATA.voice_mic_params.mic_a, getVoiceMicFromConfig(read_data->voice_mic_a, read_data));
        sinkAudioReadMicParams(&AUDIO_GDATA.voice_mic_params.mic_b, getVoiceMicFromConfig(read_data->voice_mic_b, read_data));

        sinkAudioReadAnalogueInputParams(&AUDIO_GDATA.analogue_input_params, &read_data->analogue_input);

        AUDIO_GDATA.manual_source_selection = read_data->PluginFeatures.manual_source_selection;

        AudioConfigSetQuality(audio_stream_voice, read_data->audio_quality.voice_quality);
        AudioConfigSetQuality(audio_stream_music, read_data->audio_quality.music_quality);

        AudioConfigSetMicrophoneBiasVoltage(MIC_BIAS_0, read_data->mic_bias_drive_voltage.mic_bias_0_voltage);
        AudioConfigSetMicrophoneBiasVoltage(MIC_BIAS_1, read_data->mic_bias_drive_voltage.mic_bias_1_voltage);
		
        AudioConfigSetRenderingMode(read_data->AudioRenderingMode);

        AudioConfigSetUseSramForA2dp(read_data->useSram);
        
        AudioConfigSetMusicProcessingCompanderBand(read_data->CompanderBandSelection);

        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }

    sinkAudioConfigurePcmInputs();
}

/****************************************************************************
NAME
    sinkAudioGetPluginFeatures

DESCRIPTION
    Returns the audio plugin features.

RETURNS
    Plugin features
*/
AudioPluginFeatures sinkAudioGetPluginFeatures(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    AudioPluginFeatures audio_plugin_features;

    memset(&audio_plugin_features, 0, sizeof (audio_plugin_features));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        audio_plugin_features.manual_source_selection = read_data->PluginFeatures.manual_source_selection;
        audio_plugin_features.audio_input_routing = read_data->PluginFeatures.audio_input_routing;
        audio_plugin_features.spdif_supported_data_types = read_data->PluginFeatures.spdif_supported_data_types;
        audio_plugin_features.dut_input = read_data->PluginFeatures.dut_input;
        audio_plugin_features.force_resampling_of_tones = read_data->PluginFeatures.force_resampling_of_tones;
        audio_plugin_features.use_one_mic_back_channel = read_data->PluginFeatures.use_one_mic_back_channel;
        audio_plugin_features.use_two_mic_back_channel = read_data->PluginFeatures.use_two_mic_back_channel;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);

        audio_plugin_features.stereo = TRUE;
    }

    audio_plugin_features.force_resampling_of_tones |= sinkAncIsEnabled();

    return audio_plugin_features;
}

/****************************************************************************
NAME
    sinkAudioIsManualSrcSelectionEnabled

DESCRIPTION
    Returns the status of manual source selection feature bit.

RETURNS
    TRUE If manual source selection is enabled, FALSE otherwise
*/
bool sinkAudioIsManualSrcSelectionEnabled(void)
{
    return AUDIO_GDATA.manual_source_selection;
}

/****************************************************************************
NAME
    sinkAudioGetAudioInputRoute

DESCRIPTION
    Audio input interface to use as source.

RETURNS
    Input audio source
*/
AUDIO_ROUTE_T sinkAudioGetAudioInputRoute(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    AUDIO_ROUTE_T audio_route = AUDIO_ROUTE_INTERNAL;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        audio_route = read_data->PluginFeatures.audio_input_routing;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return audio_route;
}

/****************************************************************************
NAME
    sinkAudioGetSilenceThreshold

DESCRIPTION
    Returns the configured silence detection threshold

RETURNS
    threshold
*/
uint16 sinkAudioGetSilenceThreshold(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint16 silence_threshold = 0x0; /* By default silence detection is set to disabled */

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        silence_threshold = read_data->SilenceDetSettings.threshold;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return silence_threshold;
}

/****************************************************************************
NAME
    sinkAudioGetSilenceTriggerTime

DESCRIPTION
    Returns the silence detection trigger time

RETURNS
    trigger time in seconds (16 bit)
*/
uint16 sinkAudioGetSilenceTriggerTime(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint16 silence_trigger_time = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        silence_trigger_time = read_data->SilenceDetSettings.trigger_time;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return silence_trigger_time;
}

/****************************************************************************
NAME
    sinkAudioAmpPowerUpTime

DESCRIPTION
    Return the configured time to power up amp before unmuting (milliseconds)

RETURNS
    Amp power up time before unmute in seconds (16 bit)
*/
uint16 sinkAudioAmpPowerUpTime(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint16 amp_unmute_time = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        amp_unmute_time = read_data->AudioAmpUnmuteTime_ms;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return amp_unmute_time;
}

bool sinkAudioSetAmpPowerUpTime(uint16 time)
{
    sink_audio_readonly_config_def_t *read_data = NULL;

    if (configManagerGetWriteableConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (void **)&read_data, 0))
    {
        read_data->AudioAmpUnmuteTime_ms = time;
        configManagerUpdateWriteableConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    sinkAudioSourceDisconnectDelay

DESCRIPTION
    Return the audio routing delay after source disconnection

RETURNS
    Audio routing delay after input source disconnection
*/
uint16 sinkAudioSourceDisconnectDelay(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint16 delay = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        delay = read_data->audio_switch_delay_after_disconn;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return delay;
}

/****************************************************************************
NAME
    sinkTwsAudioActiveDelay

DESCRIPTION
    Return the TWS audio active delay in milliseconds after TWS audio is routed.

RETURNS
    TWS Audio active delay after TWS audio is routed.
*/
uint16 sinkTwsAudioActiveDelay(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint16 delay = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        delay = read_data->TwsAudioActiveDelay_ms;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return delay;
}

/****************************************************************************
NAME
    sinkAudioAmpPowerDownTime

DESCRIPTION
    Return the configured time to mute before powering down amp(milliseconds)

RETURNS
    Time to mute amp before power down (16 bit)
*/
uint16 sinkAudioAmpPowerDownTime(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint16 amp_mute_time = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        amp_mute_time = read_data->AudioAmpMuteTime_ms;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return amp_mute_time;
}

bool sinkAudioSetAmpPowerDownTime(uint16 time)
{
    sink_audio_readonly_config_def_t *read_data = NULL;

    if (configManagerGetWriteableConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (void **)&read_data, 0))
    {
        read_data->AudioAmpMuteTime_ms = time;
        configManagerUpdateWriteableConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/****************************************************************************
NAME
    sinkAudioGetMic1Params

DESCRIPTION
    Return the configured mic 1 settings

RETURNS
    Mic 1 settings
*/
audio_mic_params sinkAudioGetMic1Params(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    audio_mic_params mic_params;

    memset(&mic_params, 0, sizeof (mic_params));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        sinkAudioReadMicParams(&mic_params, &read_data->common_mic_params.mic_1);
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_params;
}

/****************************************************************************
NAME
    sinkAudioGetMic1bParams

DESCRIPTION
    Return the configured mic 2 settings

RETURNS
    Mic 2 settings
*/
audio_mic_params sinkAudioGetMic2Params(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    audio_mic_params mic_params;

    memset(&mic_params, 0, sizeof (mic_params));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        sinkAudioReadMicParams(&mic_params, &read_data->common_mic_params.mic_2);
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_params;
}

/****************************************************************************
NAME
    sinkAudioGetMic3Params

DESCRIPTION
    Return the configured mic 3 settings

RETURNS
    Mic 3 settings
*/
audio_mic_params sinkAudioGetMic3Params(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    audio_mic_params mic_params;

    memset(&mic_params, 0, sizeof (mic_params));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        sinkAudioReadMicParams(&mic_params, &read_data->common_mic_params.mic_3);
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_params;
}

/****************************************************************************
NAME
    sinkAudioGetMic4Params

DESCRIPTION
    Return the configured mic 4 settings

RETURNS
    Mic 4 settings
*/
audio_mic_params sinkAudioGetMic4Params(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    audio_mic_params mic_params;

    memset(&mic_params, 0, sizeof (mic_params));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        sinkAudioReadMicParams(&mic_params, &read_data->common_mic_params.mic_4);
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_params;
}

/****************************************************************************
NAME
    sinkAudioGetMic5Params

DESCRIPTION
    Return the configured mic 5 settings

RETURNS
    Mic 5 settings
*/
audio_mic_params sinkAudioGetMic5Params(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    audio_mic_params mic_params;

    memset(&mic_params, 0, sizeof (mic_params));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        sinkAudioReadMicParams(&mic_params, &read_data->common_mic_params.mic_5);
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_params;
}

/****************************************************************************
NAME
    sinkAudioGetMic6Params

DESCRIPTION
    Return the configured mic 6 settings

RETURNS
    Mic 6 settings
*/
audio_mic_params sinkAudioGetMic6Params(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    audio_mic_params mic_params;

    memset(&mic_params, 0, sizeof (mic_params));

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        sinkAudioReadMicParams(&mic_params, &read_data->common_mic_params.mic_6);
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_params;
}

/****************************************************************************
NAME
    sinkAudioMicAPreAmpEnabled

DESCRIPTION
    Determines if pre amp is enabled or mic a (analogue only)

RETURNS
    TRUE if mic a pre amp is enabled, FALSE otherwise
*/
bool sinkAudioMicAPreAmpEnabled(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    bool mic_a_pre_amp = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        mic_a_pre_amp = read_data->common_mic_params.mic_1.pre_amp;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_a_pre_amp;
}

/****************************************************************************
NAME
    sinkAudioMicBPreAmpEnabled

DESCRIPTION
    Determines if pre amp is enabled or mic b (analogue only)

RETURNS
    TRUE if mic b pre amp is enabled, FALSE otherwise
*/
bool sinkAudioMicBPreAmpEnabled(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    bool mic_b_pre_amp = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        mic_b_pre_amp = read_data->common_mic_params.mic_2.pre_amp;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return mic_b_pre_amp;
}

/****************************************************************************
NAME
    sinkAudioGetTwsMasterAudioRouting

DESCRIPTION
    Returns the TWS master audio routing mode.

RETURNS
    TWS master audio routing mode
*/
uint8 sinkAudioGetTwsMasterAudioRouting(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 tws_master_audio_routing = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        tws_master_audio_routing = read_data->TwsMasterAudioRouting;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return tws_master_audio_routing;
}

/****************************************************************************
NAME
    sinkAudioGetTwsSlaveAudioRouting

DESCRIPTION
    Returns the TWS slave audio routing mode.

RETURNS
    TWS slave audio routing mode
*/
uint8 sinkAudioGetTwsSlaveAudioRouting(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 tws_slave_audio_routing = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        tws_slave_audio_routing = read_data->TwsSlaveAudioRouting;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return tws_slave_audio_routing;
}

/****************************************************************************
NAME
    sinkAudioGetAudioActivePio

DESCRIPTION
    Returns the configured PIO for Audio Active.

RETURNS
    Audio active pio
*/
uint8 sinkAudioGetAudioActivePio(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 audio_active_pio = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        audio_active_pio = read_data->AudioActivePIO;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return audio_active_pio;
}

/****************************************************************************
NAME
    sinkAudioGetAudioActivePio

DESCRIPTION
    Returns the configured PIO for Aux out detect.

RETURNS
    Aux out detect pio
*/
uint8 sinkAudioGetAuxOutDetectPio(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 aux_out_detect_pio = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        aux_out_detect_pio = read_data->aux_out_detect;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return aux_out_detect_pio;
}

/****************************************************************************
NAME
    sinkAudioGetPowerOnPio

DESCRIPTION
    Returns the configured PIO for Power on.

RETURNS
    Power on pio
*/
uint8 sinkAudioGetPowerOnPio(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 power_on_pio = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        power_on_pio = read_data->PowerOnPIO;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return power_on_pio;
}

/****************************************************************************
NAME
    sinkAudioGetAudioMutePio

DESCRIPTION
    Returns the configured PIO for Audio mute.

RETURNS
    Audio mute pio
*/
uint8 sinkAudioGetAudioMutePio(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 audio_mute_pio = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        audio_mute_pio = read_data->AudioMutePIO;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return audio_mute_pio;
}

/****************************************************************************
NAME
    sinkAudioGetHighestPriorityAudioSource

DESCRIPTION
    Returns the configured highest priority audio source

RETURNS
    priority 1 source
*/
static audio_sources sinkAudioGetHighestPriorityAudioSource(void)
{
    audio_sources source_priority_list[MAXIMUM_NUMBER_OF_SOURCES];

    getSourcePriorities(source_priority_list);

    return source_priority_list[0];
}

/****************************************************************************
NAME
    sinkAudioGetDefaultAudioSource

DESCRIPTION
    Returns the default audio source set in configuration

RETURNS
    default source
*/
uint8 sinkAudioGetDefaultAudioSource(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    uint8 default_source = 0x0;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        default_source = read_data->defaultSource;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return default_source;
}


/****************************************************************************
NAME
    sinkAudioRoutingGetSessionData

DESCRIPTION
    Sets Audio related data using stored session data values.

RETURNS
    None
*/
void sinkAudioRoutingGetSessionData(void)
{
    sink_audio_writeable_config_def_t *session_data = NULL;
    uint16 size = 0;

    size = configManagerGetReadOnlyConfig(SINK_AUDIO_WRITEABLE_CONFIG_BLK_ID, (const void **)&session_data);

    if (size)
    {
        AUDIO_GDATA.audio_session_data = *session_data;
        configManagerReleaseConfig(SINK_AUDIO_WRITEABLE_CONFIG_BLK_ID);
    }
    else
    {
        AUD_DEBUG(("AUDIO:sinkAudioGetWriteableConfig(): Error In reading Config , Block ID: SINK_AUDIO_WRITEABLE_CONFIG_BLK_ID"));
    }
}

/****************************************************************************
NAME
    sinkAudioRoutingSetSessionData

DESCRIPTION
    Sets Audio related session data using using current data values.

RETURNS
    None
*/
void sinkAudioRoutingSetSessionData(void)
{
    sink_audio_writeable_config_def_t *write_data = NULL;

    if (configManagerGetWriteableConfig(SINK_AUDIO_WRITEABLE_CONFIG_BLK_ID, (void **)&write_data, 0))
    {
        *write_data = AUDIO_GDATA.audio_session_data;
        configManagerUpdateWriteableConfig(SINK_AUDIO_WRITEABLE_CONFIG_BLK_ID);
    }
}

/****************************************************************************
NAME
    sinkAudioGetRequestedAudioSource

DESCRIPTION
    Get the requested Audio source.

RETURNS
    Requested audio source
*/
audio_sources sinkAudioGetRequestedAudioSource(void)
{
    return AUDIO_GDATA.audio_session_data.requested_audio_source;
}

/****************************************************************************
NAME
    sinkAudioGetRoutedAudioSource

DESCRIPTION
    Get the currently routed Audio source.

RETURNS
    routed audio source
*/
audio_sources sinkAudioGetRoutedAudioSource(void)
{
    return AUDIO_GDATA.routed_audio_source;
}

/****************************************************************************
NAME
    sinkAudioSetRoutedAudioSource

DESCRIPTION
    Updates the currently routed Audio source.

RETURNS
    None
*/
void sinkAudioSetRoutedAudioSource(audio_sources source)
{
     AUDIO_GDATA.routed_audio_source= source;

    if(isAudioSourceA2dp(source))
    {
        a2dp_source_priority_t priority = (source == audio_source_a2dp_1 ? a2dp_priority_normal : a2dp_priority_reversed);
        setA2dpSourcePriority(priority);
        AUD_DEBUG(("A2DP source priority = %d\n", priority));
    }
}


/****************************************************************************
NAME
    sinkAudioGetPEQ

DESCRIPTION
    Returns the user eq bank settings

RETURNS
    user_eq_bank_t
*/
user_eq_bank_t *sinkAudioGetPEQ(void)
{
    return &AUDIO_GDATA.audio_session_data.PEQ;
}

/****************************************************************************
NAME
    sinkAudioPeerGetPEQ

DESCRIPTION
    Return the PEQ settings to be sent to currently connected Peer.

RETURNS
    None
*/
bool sinkAudioPeerGetPEQ(uint8 *user_eq_params)
{
    user_eq_bank_t *PEQ = NULL;
    bool status = FALSE;
    uint16 i;
    PEQ = sinkAudioGetPEQ();

    if((user_eq_params != NULL) && (PEQ != NULL))
    {
        /*Store the data into the uint8 array */
        user_eq_params[PRE_GAIN_LO_OFFSET] = LOBYTE(PEQ->preGain);
        user_eq_params[PRE_GAIN_HI_OFFSET] = HIBYTE(PEQ->preGain);

        /*Increment the data pointer with the no. of bytes required to store the Pre-gain value */
        user_eq_params += USER_EQ_PARAM_PRE_GAIN_SIZE ;

        for(i=0; i<NUM_USER_EQ_BANDS ; i++)
        {
            user_eq_params[BAND_FILTER_OFFSET]= (uint8)PEQ->bands[i].filter;
            user_eq_params[BAND_FREQ_LO_OFFSET]=  LOBYTE(PEQ->bands[i].freq);
            user_eq_params[BAND_FREQ_HI_OFFSET]=  HIBYTE(PEQ->bands[i].freq);
            user_eq_params[BAND_GAIN_LO_OFFSET]=  LOBYTE(PEQ->bands[i].gain);
            user_eq_params[BAND_GAIN_HI_OFFSET]=  HIBYTE(PEQ->bands[i].gain);
            user_eq_params[BAND_Q_LO_OFFSET]=  LOBYTE(PEQ->bands[i].Q);
            user_eq_params[BAND_Q_HI_OFFSET]=  HIBYTE(PEQ->bands[i].Q);

            /*Increment the data pointer with the no. of bytes required to store all the band parameters */
            user_eq_params += USER_EQ_BAND_PARAMS_SIZE ;
        }

        status = TRUE;
    }

    return status;
}

/****************************************************************************
NAME
    sinkAudioPeerUpdatePEQ

DESCRIPTION
    Update the PEQ settings as received from the connected Peer.

RETURNS
    None
*/
void sinkAudioPeerUpdatePEQ(uint8* peer_updated_eq)
{
    user_eq_bank_t *PEQ = NULL;
    uint16 i;
    PEQ = sinkAudioGetPEQ();

    PEQ->preGain = MAKEWORD(peer_updated_eq[PRE_GAIN_LO_OFFSET] , peer_updated_eq[PRE_GAIN_HI_OFFSET]);

    /*Increment the data pointer with the no. of bytes required to store the Pre-gain value */
    peer_updated_eq += USER_EQ_PARAM_PRE_GAIN_SIZE;

    for(i=0; i<NUM_USER_EQ_BANDS; i++)
    {
        PEQ->bands[i].filter = peer_updated_eq[BAND_FILTER_OFFSET];
        PEQ->bands[i].freq = MAKEWORD(peer_updated_eq[BAND_FREQ_LO_OFFSET], peer_updated_eq[BAND_FREQ_HI_OFFSET]);
        PEQ->bands[i].gain = MAKEWORD(peer_updated_eq[BAND_GAIN_LO_OFFSET], peer_updated_eq[BAND_GAIN_HI_OFFSET]);
        PEQ->bands[i].Q = MAKEWORD(peer_updated_eq[BAND_Q_LO_OFFSET], peer_updated_eq[BAND_Q_HI_OFFSET]);

        /*Increment the data pointer with the no. of bytes required to store all the band parameters */
        peer_updated_eq += USER_EQ_BAND_PARAMS_SIZE ;
    }
}

/****************************************************************************
NAME
    sinkAudioGetRoutedAudioInstance

DESCRIPTION
    Returns the currently routed audio instance.

RETURNS
    Currently routed instance
*/
static audio_instance_t sinkAudioGetRoutedAudioInstance(void)
{
    return AUDIO_GDATA.routed_audio_instance;
}

bool sinkAudioIsAudioRouted(void)
{
    return sinkAudioGetRoutedAudioInstance() != NULL;
}

/****************************************************************************
NAME
    sinkAudioGetRoutedAudioSink

DESCRIPTION
    Returns the currently routed audio sink.

RETURNS
    Currently routed sink
*/
Sink sinkAudioGetRoutedAudioSink(void)
{
    Sink routed_sink = (Sink)NULL;
    if(sinkAudioIsAudioRouted())
    {
        routed_sink = AudioInstanceGetSink(sinkAudioGetRoutedAudioInstance());
    }
    return routed_sink;
}

/****************************************************************************
NAME
    sinkAudioGetRoutedAudioTask

DESCRIPTION
    Returns the currently routed audio task.

RETURNS
    Currently routed task
*/
Task sinkAudioGetRoutedAudioTask(void)
{
    Task routed_task = (Task)NULL;
    if(sinkAudioIsAudioRouted())
    {
        routed_task = AudioInstanceGetPluginTask(sinkAudioGetRoutedAudioInstance());
    }
    return routed_task;
}

/****************************************************************************
NAME
    sinkAudioGetRoutedVoiceInstance

DESCRIPTION
    Returns the currently routed voice instance.

RETURNS
    Currently routed instance
*/
static audio_instance_t sinkAudioGetRoutedVoiceInstance(void)
{
    return AUDIO_GDATA.routed_voice_instance;
}

bool sinkAudioIsVoiceRouted(void)
{
    return (sinkAudioGetRoutedVoiceInstance() ? TRUE : FALSE);
}

/****************************************************************************
NAME
    sinkAudioGetRoutedVoiceSink

DESCRIPTION
    Returns the currently routed voice sink.

RETURNS
    Currently routed sink
*/
Sink sinkAudioGetRoutedVoiceSink(void)
{
    Sink routed_sink = (Sink)NULL;
    if(sinkAudioIsVoiceRouted())
    {
        routed_sink = AudioInstanceGetSink(sinkAudioGetRoutedVoiceInstance());
    }
    return routed_sink;
}

/****************************************************************************
NAME
    sinkAudioGetRoutedVoiceTask

DESCRIPTION
    Returns the currently routed voice task.

RETURNS
    Currently routed task
*/
Task sinkAudioGetRoutedVoiceTask(void)
{
    Task routed_task = (Task)NULL;
    if(sinkAudioIsVoiceRouted())
    {
        routed_task = AudioInstanceGetPluginTask(sinkAudioGetRoutedVoiceInstance());
    }
    return routed_task;
}

/****************************************************************************
NAME
    sinkAudioSetRoutedAudioInstance

DESCRIPTION
    Updates the currently routed audio instance.

RETURNS
    None
*/
void sinkAudioSetRoutedAudioInstance(audio_instance_t instance)
{
    AUDIO_GDATA.routed_audio_instance = instance;
}

/****************************************************************************
NAME
    sinkAudioSetRoutedVoiceInstance

DESCRIPTION
    Updates the currently routed voice instance.

RETURNS
    None
*/
static void sinkAudioSetRoutedVoiceInstance(audio_instance_t instance)
{
    AUDIO_GDATA.routed_voice_instance = instance;
}

/****************************************************************************
NAME
    sinkAudioReSetGatedAudio

DESCRIPTION
    Resets the gated audio flag.

RETURNS
    None
*/
void sinkAudioReSetGatedAudio(void)
{
    AUDIO_GDATA.gated_audio = 0;
}

/****************************************************************************
NAME
    sinkAudioIsAmpReady

DESCRIPTION
    Returns the status of amplifier ready flag.

RETURNS
    TRUE: if audio amplifier is ready, FALSE otherwise
*/
bool sinkAudioIsAmpReady(void)
{
    return AUDIO_GDATA.audioAmpReady;
}

/****************************************************************************
NAME
    sinkAudioSetAmpReady

DESCRIPTION
    Updates the status of amplifier ready flag.

RETURNS
    None
*/
void sinkAudioSetAmpReady(bool amp_ready)
{
    AUDIO_GDATA.audioAmpReady = amp_ready;
}

/****************************************************************************
NAME
    sinkAudioIsSilenceDetected

DESCRIPTION
    Returns the status of Silence detection flag.

RETURNS
    TRUE: If silence detected, FALSE otherwise
*/
bool sinkAudioIsSilenceDetected(void)
{
    return AUDIO_GDATA.silence_detected;
}

/****************************************************************************
NAME
    sinkAudioSetSilence

DESCRIPTION
    Updates the status of silence detection flag.

RETURNS
    None
*/
void sinkAudioSetSilence(bool silence)
{
    AUDIO_GDATA.silence_detected= silence;
}

/****************************************************************************
NAME
    sinkAudioReadMicParams

DESCRIPTION
    Helper function to populate global mic params from the config read params

RETURNS
    None
*/
static void sinkAudioReadMicParams(audio_mic_params *global, audio_mic_params_t *config)
{
    global->bias_config = config->bias_config;
    global->is_digital  = config->is_digital;
    global->pre_amp = config->pre_amp;
    global->pio = config->pio;
    global->instance = config->instance;
    global->gain = config->gain;
	global->channel = config->channel;
}

/****************************************************************************
NAME
    sinkAudioReadAnalogueInputParams

DESCRIPTION
    Helper function to populate global analogue input params from the config read params

RETURNS
    None
*/
static void sinkAudioReadAnalogueInputParams(analogue_input_params *global, analogue_input_params_t *config)
{
    global->pre_amp = config->pre_amp;
    global->instance = config->instance;
    global->gain = config->gain;
    global->enable_24_bit_resolution = config->enable_24_bit_resolution;
}

/****************************************************************************
NAME
    sinkAudioGetCommonMicParams

DESCRIPTION
    Retrieve the common mic params

RETURNS
    None
*/
void sinkAudioGetCommonMicParams(const voice_mic_params_t ** mic_params)
{
    /* Return the read mic params from global audio data */
    *mic_params = &AUDIO_GDATA.voice_mic_params;
}

/****************************************************************************
NAME
    sinkAudioGetCommonAnalogueInputParams

DESCRIPTION
    Retrieve the common analogue input is params

RETURNS
    None
*/
void sinkAudioGetCommonAnalogueInputParams(const analogue_input_params ** analogue_params)
{
    /* Return the read analogue in params from global audio data */
    *analogue_params = &AUDIO_GDATA.analogue_input_params;
}

/****************************************************************************
NAME
    sinkAudioLowPowerCodecEnabled

DESCRIPTION
    Returns the status of low power audio codec use  feature bit

RETURNS
    TRUE if Low power audio codec  use is enabled, FALSE otherwise
*/
bool sinkAudioLowPowerCodecEnabled(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    bool low_power_codec_enabled = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        low_power_codec_enabled = read_data->UseLowPowerAudioCodecs;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return low_power_codec_enabled;
}



/****************************************************************************
NAME
    audioSwitchToAudioSource

DESCRIPTION
    Switch audio routing to the source passed in, it may not be possible
    to actually route the audio at that point if audio sink for that source
    is not available at that time.

RETURNS
    None
*/
void audioSwitchToAudioSource(audio_sources source)
{
    if(audioIsAudioSourceEnabledByUser(source))
    {
        sinkAudioSetRequestedAudioSource(source);
        audioUpdateAudioRouting();
    }
}

/****************************************************************************
NAME
    audioIsReadyForAudioRouting

DESCRIPTION
    Fundamental checks before we continue with routing an audio source.

RETURNS
    TRUE if checks passed, FALSE otherwise.
*/
static bool audioIsReadyForAudioRouting(void)
{
    /* invalidate at-limit status */
    sinkVolumeResetMaxAndMinVolumeFlag();

    /* ensure initialisation is complete before attempting to route audio */
    if(sinkDataGetSinkInitialisingStatus())
    {
        return FALSE;
    }


    return TRUE;
}

static bool audioRouteAudioFromTWSPeer(void)
{
    if(peerIsRemotePeerInCall())
    {
        if(peerIsSingleDeviceOperationEnabled())
        {
            audioSuspendDisconnectAudioSource();
        }
        return TRUE;
    }
    if(peerIsDeviceSlaveAndStreaming() && (peerIsRemotePeerInCall() == FALSE))
    {
        uint16 peer_index;
        if(a2dpGetPeerIndex(&peer_index))
        {
            return audioRouteA2dpIfAvailable(getA2dpSourceFromIndex((a2dp_index_t)peer_index));
        }
    }
    return FALSE;
}

static void audioRouteAudioSource(void)
{
    if(sinkCheckPartyModeAudio())
    {
        AUD_DEBUG(("AUD: party mode\n"));
    }
    else if(audioRouteAudioFromTWSPeer())
    {
        AUD_DEBUG(("AUD: Routed TWS Peer\n"));
    }
    else if(audioRouteSource(audioGetAudioSourceToRoute()))
    {
        AUD_DEBUG(("AUD: Routed Audio Source = %d\n", sinkAudioGetRoutedAudioSource()));
    }
    else if(audioRouteScoOutsideOfCallIfAvailable())
    {
        AUD_DEBUG(("AUD: Routed SCO outside of call\n"));
    }
}

void audioUpdateAudioActivePio(void)
{
    if(sinkAudioIsAudioRouted() && !sinkAudioSilentAnalogueIsRouted())
    {
        PioDrivePio(PIO_AUDIO_ACTIVE, TRUE);
}
    else if (!IsAudioBusy())
    { 
        PioDrivePio(PIO_AUDIO_ACTIVE, FALSE);
    }
}

void enableAudioActivePio(void)
{
    PioDrivePio(PIO_AUDIO_ACTIVE, TRUE);
}

void disableAudioActivePio(void)
{
    PioDrivePio(PIO_AUDIO_ACTIVE, FALSE);
}

void disableAudioActivePioWhenAudioNotBusy(void)
{
    MessageSendConditionallyOnTask(&theSink.task, EventSysCheckAudioAmpDrive, 0, AudioBusyPtr());
}

static void updateControlPlugin(void)
{
    Task control_plugin = (Task)NULL;

    if(sinkAudioIsVoiceRouted())
    {
        control_plugin = sinkAudioGetRoutedVoiceTask();
    }
    else if(sinkAudioIsAudioRouted())
    {
        control_plugin = sinkAudioGetRoutedAudioTask();
    }

    AudioOverrideControlPlugin(control_plugin);
}

/****************************************************************************
NAME
    handleAudioActiveDelayRequest

DESCRIPTION
    Handle an audio active delay request.
    A delay in activating the audio may be configured for a TWS slave to reduce audio pops when 
     an AG starts A2DP streaming for TWS audio.

RETURNS
    Returns TRUE when message 'EventSysEnableAudioActive' was sent to delay audio enable for a TWS slave.
    Returns FALSE when no audio enable delay request was sent.
*/
static bool handleAudioActiveDelayRequest(void)
{
    bool message_sent = FALSE ;
    
    if (deviceManagerIsPeerConnected() && sinkAudioIsAudioRouted() &&
        !getAudioPluginConnectParams()->silence_threshold &&
        !((stateManagerGetState() >= deviceOutgoingCallEstablish) && (stateManagerGetState() <= deviceActiveCallNoSCO)))
    {
        uint16 delay = sinkTwsAudioActiveDelay() ; 
        
        if (delay > 0 )
        {
            sinkCancelAndSendLater(EventSysEnableAudioActive, (uint32)delay);
            message_sent = TRUE ;
        }
    }

    if (!message_sent)
    {
        audioUpdateAudioActivePio();
    }
    
    return message_sent ;
}

static void updateActiveAvrcpInstanceBasedOnRequestedSource(void)
{
    if(sinkAudioIsManualSrcSelectionEnabled() && sinkAudioIsAudioRouted() == FALSE
                && isAudioSourceA2dp(sinkAudioGetRequestedAudioSource()))
    {
        a2dp_index_t index = getA2dpIndexFromSource(sinkAudioGetRequestedAudioSource());
        sinkAvrcpSetActiveConnectionFromBdaddr(getA2dpLinkBdAddr(index));
    }
}

static void audioPostRoutingAudioConfiguration(void)
{
    /* Configure subwoofer audio if the correct connection is available for the current audio source,
    otherwise make correct connection to the subwoofer */
    if(sinkAudioIsAudioRouted())
    {
        subwooferCheckConnection(TRUE);
    }

    updateActiveAvrcpInstanceBasedOnRequestedSource();

    /* Make sure soft mute is correct */
    VolumeApplySoftMuteStates();

#if defined ENABLE_PEER && defined(PEER_TWS)
    /* Based on the dealy timer value and peer open request flag delay or relay the AV source. */
    if(peerOpenRequestForQualificationEnabled() && sinkDataGetSourceSteamingDelay())
    {
        /* For some TC's where PTS expects SRC to start the stream.
            With this flag, application shall delay the audio streaming to the PTS 
            and allow DSP to load patches before start forwarding it */
        peerDelaySourceStream();
    }
    else
    {
#endif
    /* Relay any AV Source stream to a connected Peer */
    peerRelaySourceStream();
#if defined ENABLE_PEER && defined(PEER_TWS)
    }
#endif

    if(sinkAudioIsAudioRouted())
    {
        if(peerHandlesVolumeThroughSyncCommands())
        {
            peerSyncVolumeIfMaster();
        }
        else
        {
            int16 volume = audioGetVolumeForSink(sinkAudioGetRoutedAudioSink());
            displayUpdateVolume((VOLUME_NUM_VOICE_STEPS * volume)/sinkVolumeGetNumberOfVolumeSteps(audio_output_group_main));
            updateSwatVolume(volume);
        }
    }

    handleAudioActiveDelayRequest();

    baAudioPostRoutingAudioConfiguration();
    /* Resume VA session if it was previously halted */
    SinkVaResumeSession();
}

static bool activeFeaturesOverrideRouting(void)
{
    return (speechRecognitionIsActive() || sinkAncIsTuningModeActive());
}

static void audioRouteVoiceSource(void)
{
    if((!audioRouteActiveCallScoIfAvailable()) && (!audioRouteUsbVoiceIfAvailable()))
    {
        if(sinkAudioIsVoiceRouted())
        {
            audioSuspendDisconnectVoiceSource();
            updateVolumeBasedOnRoutedSources();
            AUD_DEBUG(("AUD: Routed Voice Source = %d\n", (HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink())-1)));
        }
    }
}

/****************************************************************************
NAME
    audioUpdateAudioRouting

DESCRIPTION
    Handle the routing of audio sources based on current status and other
    priority features like Speech Recognition, TWS and others.

RETURNS
    None
*/
void audioUpdateAudioRouting(void)
{
    AUD_DEBUG(("AUD: Deliver EventSysCheckAudioRouting\n"));

    if(!audioIsReadyForAudioRouting())
    {
        AUD_DEBUG(("AUD: Not ready for routing\n"));
        return;
    }

    if(activeFeaturesOverrideRouting())
    {
        audioSuspendDisconnectAudioSource();
        audioDisconnectRoutedVoice();
        AUD_DEBUG(("AUD: Disconnected due to active features\n"));
        return;
    }

    audioRouteVoiceSource();
    audioRouteAudioSource();

    audioPostRoutingAudioConfiguration();
}

/****************************************************************************
NAME
    audioRoutingProcessUpdateMsg

DESCRIPTION
    Single call which looks after the input audio source routing, triggered by
    a queued event.

RETURNS
    None.
*/
void audioRoutingProcessUpdateMsg(void)
{
    audioUpdateAudioRouting();
}

/****************************************************************************
NAME
    audioUpdateAudioRoutingAfterDisconnect

DESCRIPTION
    Handle the routing of audio sources based on current status and other
    priority features after certain delay (in ms).  The delay is configurable
    by the user through sink configurator tool

RETURNS
    None
 */
void audioUpdateAudioRoutingAfterDisconnect(void)
{
    sinkCancelAndSendLater(EventSysCheckAudioRouting, (uint32)sinkAudioSourceDisconnectDelay());
}
/*************************************************************************
NAME
    audioGetHighestPriorityAudioSourceAvailable

DESCRIPTION
    Helper function to retrieve the next available source from the user
    defined sequence, without user's input.

INPUTS
    None

RETURNS
    TRUE when successful otherwise FALSE.

*/
static audio_sources audioGetHighestPriorityAudioSourceAvailable(void)
{
    audio_sources source = sinkAudioGetHighestPriorityAudioSource();

    while(audioIsAudioSourceRoutable(source) == FALSE)
    {
        source = getNextSourceInSequence(source);

        if(sinkAudioGetHighestPriorityAudioSource() == source)
        {
            source = audio_source_none;
            break;
        }
    }

    AUD_DEBUG(("highest priority source today is %d\n", source));
    return source;
}

static bool isAudioSourceA2dp(audio_sources source)
{
    return (source == audio_source_a2dp_1 || source == audio_source_a2dp_2);
}

static bool isAudioSourceMixableWithVoice(audio_sources source)
{
    /* simultaneous voice & a2dp unsupported */
    if(isAudioSourceA2dp(source) || !audioRouteIsMixingOfVoiceAndAudioEnabled()
        || (source == audio_source_USB && usbIsCurrentVoiceSinkUsb()))
    {
        return FALSE;
    }
    return TRUE;
}

static bool isAudioRoutingBlockedByCallIndication(void)
{
    AUD_DEBUG(("isAudioRoutingBlockedByCallIndication = %d\n",
                (isCallIndicationActive() && !audioRouteIsMixingOfVoiceAndAudioEnabled())));

    return (isCallIndicationActive() && !audioRouteIsMixingOfVoiceAndAudioEnabled());
}

static bool isAudioRoutingBlockedByRoutedVoice(audio_sources source)
{
    return (sinkAudioIsVoiceRouted() && !isAudioSourceMixableWithVoice(source));
}

static bool isAudioRoutingPermitted(audio_sources source)
{
    return ((isAudioRoutingBlockedByRoutedVoice(source)
            || isAudioRoutingBlockedByCallIndication()) == FALSE);
}

/****************************************************************************
NAME
    audioRouteSource

DESCRIPTION
    attempt to route the audio for the passed in source

RETURNS
    TRUE if audio routed correctly, FALSE if no audio available yet to route
 */
static bool audioRouteSource(audio_sources source)
{
    bool routing_success = FALSE;

    if(isAudioRoutingPermitted(source))
    {
        switch (source)
        {
            case audio_source_FM:
                routing_success = audioRouteFMIfAvailable();
                break;

            case audio_source_ANALOG:
                routing_success = audioRouteAnalogIfAvailable();
                break;

            case audio_source_SPDIF:
                routing_success = audioRouteSpdifIfAvailable();
                break;

            case audio_source_I2S:
                routing_success = audioRouteI2SIfAvailable();
                break;

            case audio_source_USB:
                routing_success = audioRouteUsbIfAvailable();
                break;

            case audio_source_a2dp_1:
            case audio_source_a2dp_2:
                routing_success = audioRouteA2dpIfAvailable(source);
                break;

            case audio_source_ba:
                routing_success = audioRouteBaIfAvailable();
                break;

            case audio_source_none:
            default:
                break;
        }
        AUD_DEBUG(("AUD: audioRouteSource source = %d routed = %d\n", source, routing_success));
    }

    /* Disconnect any currently routed sources in case requested source didn't make it.*/
    if ((routing_success == FALSE) && (sinkAudioIsAudioRouted()))
    {
        AUD_DEBUG(("AUD: Nothing to route, disconnecting\n"));
        audioSuspendDisconnectAudioSource();
        updateVolumeBasedOnRoutedSources();
    }

    displayUpdateAudioSourceText(sinkAudioGetRoutedAudioSource());

    /* indicate whether audio was successfully routed or not */
    return routing_success;
}


static Sink audioGetActiveScoSink(void)
{
    Sink priority_sink = 0;

    Sink sco_sink_primary = sinkCallManagerGetHfpSink(hfp_primary_link);
    Sink sco_sink_secondary = sinkCallManagerGetHfpSink(hfp_secondary_link);
    hfp_call_state call_state_primary = sinkCallManagerGetHfpCallState(hfp_primary_link);
    hfp_call_state call_state_secondary = sinkCallManagerGetHfpCallState(hfp_secondary_link);

    /* determine number of scos and calls if any */
    if((sco_sink_primary && (call_state_primary > hfp_call_state_idle))&&
           (sco_sink_secondary && (call_state_secondary > hfp_call_state_idle)))
    {
        audio_priority primary_priority = getScoPriorityFromHfpPriority(hfp_primary_link);
        audio_priority secondary_priority = getScoPriorityFromHfpPriority(hfp_secondary_link);

        AUD_DEBUG(("AUD: two SCOs\n"));

        /* two calls and two SCOs exist, determine which sco has the highest priority */
        if(primary_priority == secondary_priority)
        {
            priority_sink = sco_sink_primary;
            /* There are two SCOs and both have the same priority, determine which was first and prioritise that */
            if(HfpGetFirstIncomingCallPriority() == hfp_secondary_link)
            {
                AUD_DEBUG(("AUD: route sec - pri = sec = [%d] [%d] :\n" , primary_priority, secondary_priority)) ;
                priority_sink = sco_sink_secondary;
            }
        }
        else
        {
            priority_sink = sco_sink_primary;
            if(secondary_priority > primary_priority)
            {
                AUD_DEBUG(("AUD: route - sec > pri = [%d] [%d] :\n" , primary_priority, secondary_priority)) ;
                priority_sink = sco_sink_secondary;
            }
        }
    }
    /* primary AG call + sco only? or pri sco and voice dial is active? */
    else if( sco_sink_primary &&
            ((call_state_primary > hfp_call_state_idle)||(sinkHfpDataGetVoiceRecognitionIsActive())))
    {
        AUD_DEBUG(("AUD: AG1 sco\n"));
        /* AG1 (primary) call with sco only */
        priority_sink = sco_sink_primary;
    }
    /* secondary AG call + sco only? or sec sco and voice dial is active? */
    else if( sco_sink_secondary &&
            ((call_state_secondary > hfp_call_state_idle)||(sinkHfpDataGetVoiceRecognitionIsActive())))
    {
        AUD_DEBUG(("AUD: AG2 sco\n"));
        /* AG2 (secondary) call with sco only */
        priority_sink = sco_sink_secondary;
    }
    return priority_sink;
}

static bool callStateIndicatesActiveCall(void)
{
    if(sinkCallManagerGetHfpCallState(hfp_primary_link) == hfp_call_state_idle
            && sinkCallManagerGetHfpCallState(hfp_secondary_link) == hfp_call_state_idle)
    {
        return FALSE;
    }
    return TRUE;
}

static bool isCallIndicationActive(void)
{
    if(callStateIndicatesActiveCall())
    {
        AUD_DEBUG(("hfp_pri_link = %d\n", sinkCallManagerGetHfpCallState(hfp_primary_link)));
        AUD_DEBUG(("hfp_sec_link = %d\n", sinkCallManagerGetHfpCallState(hfp_secondary_link)));
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
NAME
    audioRouteIsScoActive

DESCRIPTION
    Checks for any sco being present

RETURNS
    true if sco routed, false if no sco routable
*/
bool audioRouteIsScoActive(void)
{
    return audioGetActiveScoSink() != 0;
}

/****************************************************************************
NAME
    audioRouteActiveCallIfAvailable

DESCRIPTION
    checks for any sco being present, check whether there is a corresponding
    active call and route it based on its priority. check whether sco is already
    routed or whether a different audio source needs to be suspended/disconnected

RETURNS
    true if sco routed, false if no sco routable
*/
bool audioRouteActiveCallScoIfAvailable(void)
{
    Sink sco_sink_to_route = audioGetActiveScoSink();

    if(sco_sink_to_route)
    {
        AUD_DEBUG(("AUD: Sco Status state p[%d] s[%d] sink p[%p] s[%p]\n",
               sinkCallManagerGetHfpCallState(hfp_primary_link),
               sinkCallManagerGetHfpCallState(hfp_secondary_link),
               (void *) sinkCallManagerGetHfpSink(hfp_primary_link),
               (void *) sinkCallManagerGetHfpSink(hfp_secondary_link))) ;
        
        if(audioRouteScoSink(sco_sink_to_route))
        {
            hfp_link_priority hfp_priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
            uint16 hfp_volume;
            bdaddr ag_addr;

            if( HfpLinkGetBdaddr(hfp_priority, &ag_addr) &&
                    deviceManagerGetAttributeHfpVolume(&ag_addr, &hfp_volume) )
            {
                AUD_DEBUG(("AUD: btAdd : %x %x %x\n",(uint16)ag_addr.nap,(uint16)ag_addr.uap,(uint16)ag_addr.lap ));
                AUD_DEBUG(("AUD: hfp Attribute Volume set: [%d][%d]\n",hfp_priority,hfp_volume));
                sinkHfpdataSetAudioSMVolumeLevel(hfp_volume,PROFILE_INDEX(hfp_priority));
            }
            return TRUE;
        }
    }
    return FALSE;
}

static void audioGetVolumeConfigFromSource(audio_sources source, volume_info *volume)
{
    switch(source)
    {
        case audio_source_FM:
            sinkFmGetFmVolume(volume);
            break;
        case audio_source_ANALOG:
            SinkWiredGetAnalogVolume(volume);
            break;
        case audio_source_I2S:
            SinkWiredGetI2sVolume(volume);
            break;
        case audio_source_SPDIF:
            SinkWiredGetSpdifVolume(volume);
            break;
        case audio_source_USB:
            sinkUsbGetUsbVolume(volume);
            break;
        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            *volume = *sinkA2dpGetA2dpVolumeInfoAtIndex(getA2dpIndexFromSource(source));
            break;
        case audio_source_none:
        default:
            memset(volume, 0, sizeof(*volume));
            break;
    }
}

static void audioSetVolumeConfigFromSource(audio_sources source, const volume_info *volume)
{
    switch(source)
    {
        case audio_source_FM:
            sinkFmSetFmVolume(*volume);
            break;
        case audio_source_ANALOG:
        case audio_source_I2S:
            SinkWiredSetAnalogVolume(*volume);
            break;
        case audio_source_SPDIF:
            SinkWiredSetSpdifVolume(*volume);
            break;
        case audio_source_USB:
            sinkUsbSetUsbVolume(*volume);
            break;
        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            sinkA2dpSetA2dpVolumeInfoAtIndex(getA2dpIndexFromSource(source),*volume);
            break;
        case audio_source_none:
        default:
            AUD_DEBUG(("Non-recognised 'source' in audioSetVolumeConfigFromSource()"));
            break;
    }
}

static bool populateConnectParameters(audio_sources source, audio_connect_parameters *connect_parameters)
{
    switch(source)
    {
        case audio_source_FM:
            return sinkFmRxPopulateConnectParameters(connect_parameters);
        case audio_source_ANALOG:
            return analoguePopulateConnectParameters(connect_parameters);
        case audio_source_SPDIF:
            return spdifPopulateConnectParameters(connect_parameters);
        case audio_source_I2S:
            return i2sPopulateConnectParameters(connect_parameters);
        case audio_source_USB:
            return usbAudioPopulateConnectParameters(connect_parameters);
        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            return A2dpPopulateConnectParameters(getA2dpIndexFromSource(source), connect_parameters);
        case audio_source_ba:
            return baPopulateConnectParameters(connect_parameters);
        case audio_source_none:
        default:
            return FALSE;
    }
}

static void postAudioConnectSourceSpecificConfiguration(audio_sources source)
{
    switch(source)
    {
        case audio_source_FM:
            sinkFmRxPostConnectConfiguration();
            break;
        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            A2dpPostConnectConfiguration(getA2dpIndexFromSource(source));
            break;
        default:
            break;
    }
}

static void postVoiceConnectSpecificConfiguration(void)
{
    if(usbIsCurrentVoiceSinkUsb())
    {
        sinkUsbVoicePostConnectConfiguration();
    }
}

static void preserveInitialAudio(Sink audio_sink)
{
    Source audio_source = StreamSourceFromSink(audio_sink);
    /* Disconnect source from any Dispose sink */
    StreamDisconnect(audio_source, NULL);
    /* Cancel any pending prompts or tones to ensure audio is routed ASAP */
    ToneTerminate();
}

static audio_instance_t ConnectWithParameters(audio_connect_parameters *connect_parameters)
{
    if(!BA_RECEIVER_MODE_ACTIVE && !BA_BROADCASTER_MODE_ACTIVE)
        preserveInitialAudio(connect_parameters->audio_sink);
    
    return AudioConnect(connect_parameters->audio_plugin,
                        connect_parameters->audio_sink,
                        connect_parameters->sink_type,
                        connect_parameters->volume,
                        connect_parameters->rate,
                        connect_parameters->features,
                        connect_parameters->mode,
                        connect_parameters->route,
                        connect_parameters->power,
                        connect_parameters->params,
                        connect_parameters->app_task);
}

static bool populateVoiceConnectParameters(Sink sink, audio_connect_parameters *connect_parameters)
{
    if(sink == usbGetVoiceSink())
    {
        return usbVoicePopulateConnectParameters(connect_parameters);
    }
    else
    {
        return audioHfpPopulateConnectParameters(sink,connect_parameters);
    }

}

static bool connectVoiceSink(Sink sink)
{
    audio_instance_t connected_instance = NULL;
    audio_connect_parameters *connect_parameters = mallocPanic(sizeof(audio_connect_parameters));

    if(populateVoiceConnectParameters(sink,connect_parameters))
    {
        connected_instance = ConnectWithParameters(connect_parameters);
        if(connected_instance)
        {
            sinkAudioSetRoutedVoiceInstance(connected_instance);
        }
    }

    free(connect_parameters);

    return (connected_instance ? TRUE : FALSE);
}


static bool connectAudioSource(audio_sources source)
{
    audio_connect_parameters *connect_parameters = mallocPanic(sizeof(audio_connect_parameters));
    audio_instance_t connected_instance = NULL;

    memset(connect_parameters, 0, sizeof(audio_connect_parameters));

    if(populateConnectParameters(source,connect_parameters))
    {
        connected_instance = ConnectWithParameters(connect_parameters);
        if(connected_instance)
        {
            sinkAudioSetRoutedAudioInstance(connected_instance);
            updateControlPlugin();
        }
    }

    free(connect_parameters);

    return (connected_instance ? TRUE : FALSE);
}

static void updateVolumeBasedOnRoutedSources(void)
{
    if(sinkAudioIsVoiceRouted())
    {
        hfp_link_priority link_priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());
        audioControlLowPowerCodecs(TRUE);

        AudioSetVolume( sinkVolumeGetVolumeMappingforCVC(sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(link_priority))),
                                                            (int16)TonesGetToneVolume());
        VolumeSetHfpMicrophoneGain(link_priority,
                            (sinkHfpDataGetAudioGMuted(PROFILE_INDEX(link_priority)) ? MICROPHONE_MUTE_ON : MICROPHONE_MUTE_OFF));
    }
    else if(sinkAudioIsAudioRouted())
    {
        volume_info volume;
        audioControlLowPowerCodecs(FALSE);
        audioGetVolumeConfigFromSource(sinkAudioGetRoutedAudioSource(), &volume);
        adjustVolumeIfBatteryLowLimitationInplace(&volume);
        VolumeSetupInitialMutesAndVolumes(&volume);
        peerUpdateTWSMuteState();
        updateSwatVolume(volume.main_volume);
    }
}

static void routeVoiceSink(Sink voice_sink)
{
    if(sinkAudioGetRoutedVoiceSink() != voice_sink)
    {
        audioDisconnectRoutedVoice();

        if((!isAudioSourceMixableWithVoice(sinkAudioGetRoutedAudioSource()))
                || (!connectVoiceSink(voice_sink)))
        {
            audioSuspendDisconnectAudioSource();
            PanicFalse(connectVoiceSink(voice_sink));
        }
        postVoiceConnectSpecificConfiguration();
        updateVolumeBasedOnRoutedSources();
    }
}

static bool routeAudioSource(audio_sources source)
{
    bool connected = FALSE;

    subwooferResetSubwooferLinkType();
    audioSuspendDisconnectAudioSource();

    if(connectAudioSource(source))
    {
        postAudioConnectSourceSpecificConfiguration(source);
        sinkAudioSetRoutedAudioSource(source);
        updateVolumeBasedOnRoutedSources();
        connected = TRUE;
    }

    return connected;
}

static audio_sources getA2dpSourceFromIndex(a2dp_index_t index)
{
    audio_sources source = audio_source_none;
    if(index == a2dp_primary)
    {
        source = audio_source_a2dp_1;
    }
    else if(index == a2dp_secondary)
    {
        source = audio_source_a2dp_2;
    }
    return source;
}

/****************************************************************************
NAME
    audioRouteA2dpIfAvailable

DESCRIPTION
    routes specified a2dp source

RETURNS
    TRUE if routed, else FALSE
*/
static bool audioRouteA2dpIfAvailable(audio_sources source)
{
    bool a2dp_routed = FALSE;
    a2dp_index_t index = getA2dpIndexFromSource(source);

    if(sinkA2dpIsA2dpSinkRoutable(index) && (!isAudioGated(audio_gate_a2dp)))
    {
        if(sinkA2dpIsA2dpLinkSuspended(index))
        {
            ResumeA2dpStream(index);
        }
        if(sinkBroadcastIsReadyForRouting() && sinkBroadcastIsA2dpCodecSupported(index))
        {
            a2dp_routed = TRUE;
            if(!a2dpAudioSinkMatch(index, sinkAudioGetRoutedAudioSink()))
            {
                a2dp_routed = routeAudioSource(source);
            }
        }
        AUD_DEBUG(("audioRouteA2dpStreamIfAvailable, index = %d, routed = %d\n", index, a2dp_routed));
    }

    return a2dp_routed;
}

/****************************************************************************
NAME
    audioRouteUsbIfAvailable

DESCRIPTION
    checks for a usb stream being present and routes it if present and allowed
    in the current state

RETURNS
    true if usb routed, false if no usb routable
*/
static bool audioRouteUsbIfAvailable(void)
{
    if((usbGetAudioSink()
            || sinkUsbAudioIsSuspendedLocal())
        && (!isAudioGated(audio_gate_usb)))
    {
        if(sinkBroadcastIsReadyForRouting())
        {
        if(sinkUsbAudioIsSuspendedLocal())
        {
            usbAudioResume();
        }

        if(!usbAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
        {
            return routeAudioSource(audio_source_USB);
        }

        return TRUE;
    }
    }

    return FALSE;
}

/****************************************************************************
NAME
    IsUsbVoiceRoutable

DESCRIPTION
    checks for a usb voice stream being present.

RETURNS
    true if usb routable, false if no usb routable
*/
static bool IsUsbVoiceRoutable(void)
{
    Sink sink_to_route = usbGetVoiceSink();

    return (((!isAudioGated(audio_gate_usb)) && usbIsVoiceSupported()) &&
            (sink_to_route || sinkUsbAudioIsSuspendedLocal()));
}

/****************************************************************************
NAME
    audioRouteUsbVoiceIfAvailable

DESCRIPTION
    checks for a usb voice stream being present and routes it if is present and allowed
    in the current state

RETURNS
    true if usb routed, false if no usb routable
*/
static bool audioRouteUsbVoiceIfAvailable(void)
{
    Sink sink_to_route = usbGetVoiceSink();

    if(IsUsbVoiceRoutable())
    {
        if(sinkUsbAudioIsSuspendedLocal())
        {
            usbAudioResume();
        }

        if(!usbIsCurrentVoiceSinkUsb())
        {
            return audioRouteUsbVoiceSink(sink_to_route);
        }
        return TRUE;
    }

    AUD_DEBUG(("AUD: USB voice NOT routed\n"));
    return FALSE;
}

static bool audioRouteUsbVoiceSink(Sink sink_to_route)
{
    if(sink_to_route && (!isAudioGated(audio_gate_usb)))
    {
        routeVoiceSink(sink_to_route);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
NAME
    isDisconnectRequired

DESCRIPTION
    Checks if disconnect of currently routed wired input is required.

RETURNS
    TRUE if Peer is streaming and wired input sample rate is not as same as Peer sample rate, FALSE otherwise
*/
static bool isDisconnectRequired(void)
{
    WIRED_SAMPLING_RATE_T wired_input_sample_rate = SinkWiredGetAudioInputRate();

    return ((peerGetPeerSampleRateForWiredInput(wired_input_sample_rate) != wired_input_sample_rate) && peerIsPeerStreaming());
}

/****************************************************************************
NAME
    audioRouteAnalogIfAvailable

DESCRIPTION
    checks for an analog audio stream being present and routes it if present
    and allowed in current state

RETURNS
    true if analog audio routed, false if no analog audio routable
*/
static bool audioRouteAnalogIfAvailable(void)
{
    if(analogAudioConnected() && (!isAudioGated(audio_gate_wired)))
    {
        if(sinkBroadcastIsReadyForRouting())
        {
            if(analogAudioSinkMatch(sinkAudioGetRoutedAudioSink()) == FALSE ||
               isDisconnectRequired())
            {
                return routeAudioSource(audio_source_ANALOG);
            }
            return TRUE;
        }
    }
    return FALSE;
}

/****************************************************************************
NAME
    audioRouteSpdifIfAvailable

DESCRIPTION
    checks for an spdif audio stream being present and routes it if present
    and allowed in the current state

RETURNS
    true if spdif audio routed, false if no spdif audio routable
*/
static bool audioRouteSpdifIfAvailable(void)
{
    if(spdifAudioConnected() && (!isAudioGated(audio_gate_wired)) && (!sinkBroadcastAudioIsActive()))
    {
        if(spdifAudioSinkMatch(sinkAudioGetRoutedAudioSink()) == FALSE ||
           isDisconnectRequired())
        {
            return routeAudioSource(audio_source_SPDIF);
        }
        return TRUE;
    }
    return FALSE;
}
/****************************************************************************
NAME
    audioRouteI2SIfAvailable

DESCRIPTION
    checks for an I2S audio stream being present and routes it if present
    and allowed in the current state

RETURNS
    true if I2S audio routed, false if no I2S audio routable
*/
static bool audioRouteI2SIfAvailable(void)
{
    if(i2sAudioConnected() && (!isAudioGated(audio_gate_wired)))
    {
        if(i2sAudioSinkMatch(sinkAudioGetRoutedAudioSink()) == FALSE ||
           isDisconnectRequired())
        {
            return routeAudioSource(audio_source_I2S);
        }
        return TRUE;
    }
    return FALSE;
}

static void adjustVolumeIfBatteryLowLimitationInplace(volume_info *volume)
{
    if(powerManagerGetChargingDeficitAction() == battery_low_output_action_limit_max_volume)
    {
        unsigned cdl_as_step_level = sinkVolumeGetChargeDeficitMaxVolumeLimit();

        /* check whether the volume limit is in effect and prevent it from increasing
                          above limit is output is limited due to low power */
        if(cdl_as_step_level != 0U)
        {
            if(volume->main_volume > (int16)cdl_as_step_level)
            {
                AUD_DEBUG(("Volume adjusted on Routed Source change. Was %d, now %d",volume->main_volume, cdl_as_step_level));
                volume->main_volume = cdl_as_step_level;
                audioSetVolumeConfigFromSource(sinkAudioGetRoutedAudioSource(), (const volume_info *) volume);
            }
        }
    }
}

/****************************************************************************
NAME
    audioRouteFMIfAvailable

DESCRIPTION
    checks for an fm audio stream being present and routes it if present

RETURNS
    true if fm routed, false if no fm routed
*/
bool audioRouteFMIfAvailable(void)
{
    if(sinkFmIsFmRxOn() && (!isAudioGated(audio_gate_fm)))
    {
        if(!sinkFmAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
        {
            return routeAudioSource(audio_source_FM);
        }
        return TRUE;
    }
    return FALSE;
}

bool audioRouteBaIfAvailable(void)
{
    if(sinkReceiverIsRoutable() && (!isAudioGated(audio_gate_ba)) && (!sinkBroadcastAudioIsModeChangeInProgress()))
    {
        if(!baAudioSinkMatch(sinkAudioGetRoutedAudioSink()))
        {
            return routeAudioSource(audio_source_ba);
        }
        return TRUE;
    }
    return FALSE;
}


static bool audioRouteScoSink(Sink sco_sink_to_route)
{
    if(sco_sink_to_route && (!isAudioGated(audio_gate_call)))
    {
        routeVoiceSink(sco_sink_to_route);
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************
NAME
    audioRouteScoIfAvailable

DESCRIPTION
    checks for any sco being present without any call indications

RETURNS
    true if sco routed, false if no sco routable
*/
bool audioRouteScoOutsideOfCallIfAvailable(void)
{
    if(!sinkAudioIsVoiceRouted() && !sinkAudioIsAudioRouted())
    {
        Sink sink_to_route = sinkCallManagerGetHfpSink(hfp_primary_link);
        if(!sink_to_route)
        {
            sink_to_route = sinkCallManagerGetHfpSink(hfp_secondary_link);
        }
        AUD_DEBUG(("AUD: Sco Status state p[%d] s[%d] sink p[%p] s[%p]\n" ,
                   sinkCallManagerGetHfpCallState(hfp_primary_link),
                   sinkCallManagerGetHfpCallState(hfp_secondary_link),
                   (void *) sinkCallManagerGetHfpSink(hfp_primary_link),
                   (void *) sinkCallManagerGetHfpSink(hfp_secondary_link))) ;
        return audioRouteScoSink(sink_to_route);
    }
    return FALSE;
}

static hfp_link_priority getHfpLinkIndexFromA2dpIndex(a2dp_index_t index)
{
    hfp_link_priority hfp_link_index = hfp_invalid_link;
    if(deviceManagerIsSameDevice(index, hfp_primary_link))
    {
        hfp_link_index = hfp_primary_link;
    }
    else if(deviceManagerIsSameDevice(index, hfp_secondary_link))
    {
        hfp_link_index = hfp_secondary_link;
    }
    return hfp_link_index;
}

static bool isActiveCallOnTheSameDevice(a2dp_index_t index)
{
    hfp_link_priority hfp_priority = getHfpLinkIndexFromA2dpIndex(index);

    if(hfp_priority)
    {
        return (sinkCallManagerGetHfpCallState(hfp_priority) > hfp_call_state_idle);
    }
    return FALSE;
}

static void a2dpSuspendIfRequired(a2dp_index_t index)
{
    if(!sinkA2dpIsA2dpLinkSuspended(index))
    {
        if((!isActiveCallOnTheSameDevice(index)) || (!audioRouteIsAutoSuspendOnCallEnabled()))
        {
            AUD_DEBUG(("AUD: suspend a2dp audio \n"));
            SuspendA2dpStream(index);
        }
    }
}

/****************************************************************************
NAME
    audioSuspendDisconnectAudioSource

DESCRIPTION
    determines source of sink passed in and decides whether to issue a suspend or
    not based on source type, an audio disconnect is performed thereafter regardless
    of wether or not the source requires a suspend

RETURNS
    true if audio disconnected, false if no action taken
*/
void audioSuspendDisconnectAudioSource(void)
{
    if(sinkAudioIsAudioRouted())
    {
        audio_route_available current_route = sinkAudioRouteAvailable();
        audioDisconnectRoutedAudio();
        AUD_DEBUG(("audioSuspendDisconnectAudioSource current_route = %d\n", current_route));
        switch(current_route)
        {
            case audio_route_a2dp_primary:
                a2dpSuspendIfRequired(a2dp_primary);
                break;
            case audio_route_a2dp_secondary:
                a2dpSuspendIfRequired(a2dp_secondary);
                break;
            case audio_route_usb:
                usbAudioPostDisconnectConfiguration();
                break;
            case audio_route_fm:
                sinkFmRxAudioPostDisconnectConfiguration();
                break;
            case audio_route_ba:
                baAudioPostDisconnectConfiguration();
                break;
            default:
                break;
        }

        sendMuteToSubwoofer();
        sinkAudioSetRoutedAudioSource(audio_source_none);
    }
}

static void audioSuspendDisconnectVoiceSource(void)
{
    bool isCurrentVoiceSinkUsb = usbIsCurrentVoiceSinkUsb();

    audioDisconnectRoutedVoice();

    if(isCurrentVoiceSinkUsb)
    {
        usbVoicePostDisconnectConfiguration();
    }
}

static int16 audioGetVolumeForSink(Sink sink)
{
    int16 volume = sinkHfpDataGetDefaultVolume();

    if(a2dpAudioSinkMatch(a2dp_primary, sink))
    {
        volume = sinkA2dpGetA2dpVolumeInfoAtIndex(a2dp_primary)->main_volume;
    }
    else if(a2dpAudioSinkMatch(a2dp_secondary, sink))
    {
        volume = sinkA2dpGetA2dpVolumeInfoAtIndex(a2dp_secondary)->main_volume;
    }
    else if (sink == sinkCallManagerGetHfpSink(hfp_primary_link))
    {
        volume = sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(hfp_primary_link));
    }
    else if (sink == sinkCallManagerGetHfpSink(hfp_secondary_link))
    {
        volume = sinkHfpDataGetAudioSMVolumeLevel(PROFILE_INDEX(hfp_secondary_link));
    }
    else if (usbAudioSinkMatch(sink))
    {
        volume = usbGetVolumeLevel();
    }
    else if (analogAudioSinkMatch(sink))
    {
        volume = SinkWiredGetAnalogMainVolume();
    }
    else if (spdifAudioSinkMatch(sink))
    {
        volume = SinkWiredGetSpdifMainVolume();
    }
    else if(i2sAudioSinkMatch(sink))
    {
        volume = SinkWiredGetI2sMainVolume();
    }
    return volume;
}

/****************************************************************************
NAME
    audioDisconnectActiveSink

DESCRIPTION
    Disconnect the active audio sink

RETURNS

*/
static void audioDisconnectInstance(audio_instance_t instance_to_disconnect)
{
    if(instance_to_disconnect)
    {
        AudioDisconnectInstance(instance_to_disconnect);
    }
}

void audioDisconnectRoutedAudio(void)
{
    audioDisconnectInstance(sinkAudioGetRoutedAudioInstance());
    sinkAudioSetRoutedAudioInstance(NULL);
}

void audioDisconnectRoutedVoice(void)
{
    audioDisconnectInstance(sinkAudioGetRoutedVoiceInstance());
    sinkAudioSetRoutedVoiceInstance(NULL);
    updateControlPlugin();
}

/****************************************************************************
NAME
    audioSuspendDisconnectAllA2dpMedia

DESCRIPTION
    called when the SUB link wants to use an ESCO link, there is not enough
    link bandwidth to support a2dp media and esco links so suspend or disconnect
    all a2dp media streams

RETURNS
    true if audio disconnected, false if no action taken
*/
bool audioSuspendDisconnectAllA2dpMedia(void)
{
    bool disconnected = FALSE;

    AUD_DEBUG(("AUD: suspend any a2dp due to esco sub use \n"));

    /* primary a2dp currently routed? */
    if((sinkAudioGetRoutedAudioSource())&&(a2dpAudioSinkMatch(a2dp_primary, sinkAudioGetRoutedAudioSink())))
    {
        AUD_DEBUG(("AUD: suspend a2dp primary audio \n"));
        SuspendA2dpStream(a2dp_primary);
        /* and disconnect the routed sink */
        AUD_DEBUG(("AUD: disconnect a2dp primary audio \n"));
        /* disconnect audio */
        audioDisconnectRoutedAudio();
        /* update currently routed source */
        sinkAudioSetRoutedAudioSource(audio_source_none);
        /* successfully disconnected audio */
        disconnected = TRUE;
    }
    /* secondary a2dp currently routed? */
    else if((sinkAudioGetRoutedAudioSource())&&(a2dpAudioSinkMatch(a2dp_secondary, sinkAudioGetRoutedAudioSink())))
    {
        AUD_DEBUG(("AUD: suspend a2dp secondary audio \n"));
        SuspendA2dpStream(a2dp_secondary);
        /* and disconnect the routed sink */
        AUD_DEBUG(("AUD: disconnect a2dp secondary audio \n"));
        /* disconnect audio */
        audioDisconnectRoutedAudio();
        /* update currently routed source */
        sinkAudioSetRoutedAudioSource(audio_source_none);
        /* successfully disconnected audio */
        disconnected = TRUE;
    }
    /* are there any a2dp media streams not currently routed that need to be suspended? */
    else
    {
        if(sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming)
        {
            AUD_DEBUG(("AUD: suspend pri a2dp\n"));
            /* suspend or disconnect the a2dp media stream */
            SuspendA2dpStream(a2dp_primary);
        }
        if(sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming)
        {
            AUD_DEBUG(("AUD: suspend sec a2dp\n"));
            /* suspend or disconnect the a2dp media stream */
            SuspendA2dpStream(a2dp_secondary);
        }
    }
    return disconnected;
}

/****************************************************************************
NAME
    sinkAudioRouteAvailable

DESCRIPTION
    returns which audio source is routed. Only the route of highest priority is
    returned. The priority starting at the top of the enum audio_route_available

RETURNS
    audio_route_available
*/
audio_route_available sinkAudioRouteAvailable(void)
{
    audio_route_available route = audio_route_none;

    if(sinkAudioIsAudioRouted())
    {
        Sink routed_audio = sinkAudioGetRoutedAudioSink();

        if ( a2dpAudioSinkMatch( a2dp_primary, routed_audio) )
        {
            route = audio_route_a2dp_primary;
        }
        else if ( a2dpAudioSinkMatch( a2dp_secondary, routed_audio) )
        {
            route = audio_route_a2dp_secondary;
        }

        else if (analogAudioSinkMatch(routed_audio))
        {
            route = audio_route_analog;
        }
        else if (spdifAudioSinkMatch(routed_audio))
        {
            route = audio_route_spdif;
        }
        else if(i2sAudioSinkMatch(routed_audio))
        {
            route = audio_route_i2s;
        }
        else if(sinkFmAudioSinkMatch(routed_audio))
        {
            route = audio_route_fm;
        }
        else if(baAudioSinkMatch(routed_audio))
        {
            AUD_DEBUG(("AUD ROUTE: BA\n"));
            route = audio_route_ba;
        }
    }
    else
    {
        hfp_link_priority hfp_priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());

        if( hfp_priority == hfp_primary_link )
        {
            route = audio_route_hfp_primary;
        }
        else if(  hfp_priority == hfp_secondary_link )
        {
            route = audio_route_hfp_secondary;
        }
    }
    return route;
}


static bool a2dpMediaStartIndex(a2dp_index_t index)
{
    if(sinkA2dpGetStreamState(index) == a2dp_stream_open
                    && sinkA2dpIsA2dpLinkSuspended(index) == FALSE
                    && sinkA2dpGetRoleType(index) == a2dp_sink)
    {
        if(A2dpSignallingGetState(getA2dpLinkDataDeviceId(index)) == a2dp_signalling_connected)
        {
            A2dpMediaStartRequest(getA2dpLinkDataDeviceId(index), getA2dpLinkDataStreamId(index));
        }
        return TRUE;
    }
    return FALSE;
}

/****************************************************************************/
void audioA2dpStartStream(void)
{
    if (sinkA2dpIsStreamingAllowed())
    {
        if(!a2dpMediaStartIndex(a2dp_primary))
        {
            a2dpMediaStartIndex(a2dp_secondary);
        }
    }
}

/****************************************************************************/
static bool isAudioSourceEnabled(audio_sources audio_source)
{
    return (audioIsAudioSourceEnabledByUser(audio_source) 
                || audio_source != audio_source_none);
}

/*************************************************************************
NAME
    getAudioSourceFromEventUsrSelectAudioSource

DESCRIPTION
    Maps a user event to the relevant audio source.

INPUTS
    EventUsrSelectAudioSource  User event related to audio sources.

RETURNS
    The matching audio source.

*/
static audio_sources getAudioSourceFromEventUsrSelectAudioSource(const MessageId EventUsrSelectAudioSource)
{
    audio_sources audio_source = audio_source_none;

    switch(EventUsrSelectAudioSource)
    {
        case EventUsrSelectAudioSourceAnalog:
            audio_source = audio_source_ANALOG;
            break;

        case EventUsrSelectAudioSourceSpdif:
            audio_source = audio_source_SPDIF;
            break;

        case EventUsrSelectAudioSourceI2S:
            audio_source = audio_source_I2S;
            break;

        case EventUsrSelectAudioSourceUSB:
            audio_source = audio_source_USB;
            break;

        case EventUsrSelectAudioSourceA2DP1:
            audio_source = audio_source_a2dp_1;
            break;

        case EventUsrSelectAudioSourceA2DP2:
            audio_source = audio_source_a2dp_2;
            break;

        case EventUsrSelectAudioSourceFM:
            audio_source = audio_source_FM;
            break;

        case EventUsrSelectAudioSourceNextRoutable:
            audio_source = getNextRoutableSource(sinkAudioGetRequestedAudioSource());
            break;

        case EventUsrSelectAudioSourceNext:
            audio_source = getNextSourceInSequence(sinkAudioGetRequestedAudioSource());
            break;

        default:
            break;
    }

    if(isAudioSourceEnabled(audio_source) == FALSE)
    {
        audio_source = sinkAudioGetRoutedAudioSource();
    }

    return audio_source;
}

/*************************************************************************
NAME
    processEventUsrSelectAudioSource

DESCRIPTION
    Function to handle the user's source selection. Follow up calls and
    configuration settings status will determine the outcome and proceed
    by indicating the event or not.

INPUTS
    EventUsrSelectAudioSource  Source selection user events.

RETURNS
    TRUE if routing was successful and VM isn't in deviceLimbo state.
    FALSE if routing was not possible or VM is in deviceLimbo state.

*/
bool processEventUsrSelectAudioSource(const MessageId EventUsrSelectAudioSource)
{
    bool result = TRUE;

    /* No need to attempt routing a source while in deviceLimbo state. */
    if(stateManagerGetState() == deviceLimbo)
    {
        result = FALSE;
    }
    else
    {
        if(sinkAudioIsManualSrcSelectionEnabled())
        {
            sinkAudioSetRequestedAudioSource(getAudioSourceFromEventUsrSelectAudioSource(EventUsrSelectAudioSource));

            result = audioIsAudioSourceRoutable(sinkAudioGetRequestedAudioSource());

            peerUpdateRelaySource(sinkAudioGetRequestedAudioSource());
            audioUpdateAudioRouting();

            if(result)
            {
                sinkVolumeResetVolumeAndSourceSaveTimeout();
            }
        }
        else
        {
            result = FALSE;
        }
    }

    return result;
}

/*************************************************************************
NAME
    processEventUsrWiredAudioConnected

DESCRIPTION
    Function to handle the wired audio connection upon user events

INPUTS
    Source selection user events (i.e. EventUsrAnalogAudioConnected).

RETURNS
      void
*/
void processEventUsrWiredAudioConnected(const MessageId id)
{
    switch (id)
    {
        case EventUsrAnalogAudioConnected:
            /* Cancel any disconnect debounce timer */
            MessageCancelAll(&theSink.task, EventSysAnalogueAudioDisconnectTimeout);
            
            /* Start the limbo timer here to turn off the device if this feature is enabled */
            if(SinkWiredIsPowerOffOnWiredAudioConnected())
            {
                /* cancel existing limbo timeout and reschedule another limbo timeout */
                sinkCancelAndSendLater(EventSysLimboTimeout, D_SEC(SinkWiredGetAudioConnectedPowerOffTimeout()));
            }
            else
            {
                peerNotifyWiredSourceConnected(ANALOG_AUDIO);
                audioUpdateAudioRouting();
            }
            break;

        case EventUsrSpdifAudioConnected:
            audioUpdateAudioRouting();
        break;
    }
}

/*************************************************************************
NAME
    processEventWiredAudioDisconnected

DESCRIPTION
    Function to handle the wired audio disconnection upon events

INPUTS
    Event ID (e.g. EventUsrAnalogAudioDisconnected).

RETURNS
      void

*/
void processEventWiredAudioDisconnected(const MessageId id)
{
    switch (id)
    {
        case EventUsrAnalogAudioDisconnected:
            sinkCancelAndSendLater(
                    EventSysAnalogueAudioDisconnectTimeout, 
                    SinkWiredGetAnalogueDisconnectDebouncePeriod());
        break;
        
        case EventSysAnalogueAudioDisconnectTimeout:
            peerNotifyWiredSourceDisconnected(ANALOG_AUDIO);
            audioUpdateAudioRoutingAfterDisconnect();
        break;

        case EventUsrSpdifAudioDisconnected:
            audioUpdateAudioRoutingAfterDisconnect();
        break;
    }
}

static audio_sources getAudioSourceFromConfiguredPriority(unsigned priority)
{
    audio_sources source = (audio_sources)priority;
    if(isAudioSourceA2dp(source))
    {
        if(getA2dpSourcePriority() == a2dp_priority_reversed)
        {
            source = (source == audio_source_a2dp_1 ? audio_source_a2dp_2 : audio_source_a2dp_1);
        }
    }
    return source;
}

static void getSourcePriorities(audio_sources * source_priority_list)
{
    sink_audio_readonly_config_def_t *read_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        source_priority_list[0] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority1);
        source_priority_list[1] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority2);
        source_priority_list[2] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority3);
        source_priority_list[3] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority4);
        source_priority_list[4] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority5);
        source_priority_list[5] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority6);
        source_priority_list[6] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority7);
        source_priority_list[7] = getAudioSourceFromConfiguredPriority(read_data->seqSourcePriority8);
        
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
}

static unsigned getPriorityIndexForAudioSource(audio_sources source, audio_sources * source_priority_list)
{
    unsigned source_index = 0;
    while(source_index < MAXIMUM_NUMBER_OF_SOURCES)
    {
        if(source_priority_list[source_index] == source)
        {
            break;
        }
        source_index++;
    }

    if(source_index == 0 && source_priority_list[source_index] != source)
    {
        Panic();
    }

    return source_index;
}

static unsigned getNextIndexInSequence(unsigned index)
{
    if(++index >= MAXIMUM_NUMBER_OF_SOURCES)
    {
        return 0;
    }
    return index;
}

/*************************************************************************
NAME
    getNextSourceInSequence

DESCRIPTION
    Helper function to retrieve the next available source from the user
    defined sequence.

INPUTS
    source  Initial audio source to start from in order to identify next.

RETURNS
    Audio sources enums.

*/
static audio_sources getNextSourceInSequence(audio_sources source)
{
    unsigned previous_priority_index, next_priority_index;
    
    audio_sources source_priority_list[MAXIMUM_NUMBER_OF_SOURCES];

    getSourcePriorities(source_priority_list);

    previous_priority_index = getPriorityIndexForAudioSource(source, source_priority_list);
    next_priority_index = getNextIndexInSequence(previous_priority_index);

    while(!source_priority_list[next_priority_index])
    {
        next_priority_index = getNextIndexInSequence(next_priority_index);
        if(next_priority_index == previous_priority_index)
        {
            break;
        }
    }

    return source_priority_list[next_priority_index];
}

/*************************************************************************
NAME
    audioIsAudioSourceRoutable

DESCRIPTION
    Helper call to confirm the route-ability of an audio source

INPUTS
    source  Audio source enum

RETURNS
    TRUE when source passed is routable, otherwise FALSE.

*/
static bool audioIsAudioSourceRoutable(audio_sources source)
{
    bool is_routable = FALSE;

    switch(source)
    {
        case audio_source_none:
            is_routable = TRUE;
            break;
        case audio_source_FM:
            is_routable = sinkFmIsFmRxOn();
            break;
        case audio_source_ANALOG:
            is_routable = analogAudioConnected();
            break;
        case audio_source_SPDIF:
            is_routable = spdifAudioConnected();
            break;
        case audio_source_I2S:
            is_routable = i2sAudioConnected();
            break;
        case audio_source_USB:
            is_routable = ( usbGetAudioSink() ||
                    sinkUsbAudioIsSuspendedLocal());
            break;
        case audio_source_a2dp_1:
        case audio_source_a2dp_2:
            is_routable = audioIsA2dpAudioRoutable(getA2dpIndexFromSource(source));
            break;
        case audio_source_ba:
            is_routable = sinkReceiverIsRoutable();
            break;
        case audio_source_end_of_list:
        default:
            break;
    }

    AUD_DEBUG(("AUD: audioIsAudioSourceRoutable source = %d routable = %d\n", source, is_routable));

    return is_routable;

}

/*************************************************************************
NAME
    audioIsAudioSourceEnabledByUser

DESCRIPTION
    Helper call in order to verify requested source is enabled by user.

INPUTS
    source  Audio source enum.

RETURNS
    TRUE if enabled, otherwise FALSE.

*/
static bool audioIsAudioSourceEnabledByUser(audio_sources source)
{
    bool IsUserEnabled = FALSE;
    sink_audio_readonly_config_def_t *read_data = NULL;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        if ((read_data->seqSourcePriority1 == source) ||
            (read_data->seqSourcePriority2 == source) ||
            (read_data->seqSourcePriority3 == source) ||
            (read_data->seqSourcePriority4 == source) ||
            (read_data->seqSourcePriority5 == source) ||
            (read_data->seqSourcePriority6 == source) ||
            (read_data->seqSourcePriority7 == source) ||
            (read_data->seqSourcePriority8 == source) )
        {
            IsUserEnabled = TRUE;
        }

        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }

    return IsUserEnabled;
}

/*************************************************************************
NAME
    audioGetAudioSourceToRoute

DESCRIPTION
    Final call of routing an audio source.

INPUTS
    None

RETURNS
    Audio source to route.

*/
static audio_sources audioGetAudioSourceToRoute(void)
{
    audio_sources source_to_route = sinkAudioGetRequestedAudioSource();

    if(!sinkAudioIsManualSrcSelectionEnabled())
    {
        source_to_route = audioGetHighestPriorityAudioSourceAvailable();
    }

    AUD_DEBUG(("audioGetAudioSourceToRoute()  0x%X\n", source_to_route));
    return source_to_route;
}



static uint16 getAvrcpLinkIndexOfA2dpLink(a2dp_index_t a2dp_link_index)
{
    uint16 avrcp_index = 0;

    while(avrcp_index < sinkAvrcpGetMaximumAvrcpConnections())
    {
        if(BdaddrIsSame((const bdaddr *)getA2dpLinkBdAddr(a2dp_link_index),
                           (const bdaddr *)sinkAvrcpGetLinkBdAddr(avrcp_index)))
        {
            break;
        }
        avrcp_index++;
    }

    return avrcp_index;
}


static bool isA2dpLinkHighestPriorityBasedOnAvrcpState(a2dp_index_t pre_override_index)
{
    a2dp_index_t post_override_index = pre_override_index;

    if(a2dpA2dpAudioIsRouted() && (!a2dpAudioSinkMatch(pre_override_index, sinkAudioGetRoutedAudioSink())))
    {
        if(sinkA2dpIsA2dpSinkPlaying(a2dp_primary) && sinkA2dpIsA2dpSinkPlaying(a2dp_secondary))
        {
            uint16 primary_index = getAvrcpLinkIndexOfA2dpLink(a2dp_primary);
            uint16 secondary_index = getAvrcpLinkIndexOfA2dpLink(a2dp_secondary);

            post_override_index = a2dpGetRoutedInstanceIndex();

            if(sinkAvrcpIsConnected(primary_index)
                    && sinkAvrcpIsConnected(secondary_index))
            {
                if(sinkAvrcpIsAvrcpLinkInPlayingState(primary_index) != sinkAvrcpIsAvrcpLinkInPlayingState(secondary_index))
                {
                    post_override_index = a2dp_primary;
                    if(sinkAvrcpIsAvrcpLinkInPlayingState(primary_index) == FALSE)
                    {
                        post_override_index = a2dp_secondary;
                    }
                }
            }
        }
    }

    return (pre_override_index == post_override_index);
}

static bool routedA2dpShouldConsiderAvrcpState(void)
{
    bool consider_avrcp = FALSE;
    if(sinkAudioIsManualSrcSelectionEnabled() == FALSE
                && sinkAvrcpIsAvrcpAudioSwitchingEnabled() && avrcpAvrcpIsEnabled())
    {
        consider_avrcp = TRUE;
    }
    return consider_avrcp;
}

/*************************************************************************
NAME
    audioIsA2dpAudioRoutable

DESCRIPTION
    Initial check to confirm that A2DP is routable, without actually
    routing at this stage.

INPUTS
    index  a2dp_index_t type.

RETURNS
    TRUE if routable, otherwise FALSE.

*/
static bool audioIsA2dpAudioRoutable(a2dp_index_t index)
{
    bool is_routable = FALSE;

    if(sinkA2dpIsA2dpSinkRoutable(index))
    {
        is_routable = TRUE;

        if(routedA2dpShouldConsiderAvrcpState())
        {
            is_routable = isA2dpLinkHighestPriorityBasedOnAvrcpState(index);
        }
    }

    AUD_DEBUG(("audioIsA2dpAudioRoutable(0x%X) %X\n", index, is_routable));
    return is_routable;
}

/*************************************************************************
NAME
    sinkAudioSilentAnalogueIsRouted

DESCRIPTION
    Determine if analogue audio is routed but silence detection is established

RETURNS
    TRUE if silent analogue audio is routed, otherwise FALSE.

*/
static bool sinkAudioSilentAnalogueIsRouted(void)
{
    return (sinkAudioGetRoutedAudioSource() == audio_source_ANALOG) && sinkAudioIsSilenceDetected();
}

/*************************************************************************
NAME
    getNextRoutableSource

DESCRIPTION
    Gets the next available but also routable audio source.

INPUTS
    current_source  current audio source

RETURNS
    The routable audio source

*/
static audio_sources getNextRoutableSource(audio_sources current_source)
{
    audio_sources next_source;

    if(current_source == audio_source_none)
    {
        next_source = sinkAudioGetHighestPriorityAudioSource();
        current_source = next_source;
    }
    else
    {
        next_source = getNextSourceInSequence(current_source);
    }

    while(!audioIsAudioSourceRoutable(next_source))
    {
        next_source = getNextSourceInSequence(next_source);
        if(next_source == current_source)
        {
            if(!audioIsAudioSourceRoutable(next_source))
            {
                next_source = audio_source_none;
            }
            break;
        }
    }

    return next_source;
}

void audioRouteSpecificA2dpSource(audio_sources a2dp_source)
{
    if(isAudioSourceA2dp(a2dp_source))
    {
        routeAudioSource(a2dp_source);
    }
}

/****************************************************************************
NAME
    audioRouteIsMixingOfVoiceAndAudioEnabled

DESCRIPTION
    Returns the status of HFP/Wired mixing

RETURNS
    TRUE if mixing is enabled, FALSE otherwise
*/
bool audioRouteIsMixingOfVoiceAndAudioEnabled(void)
{
    sink_audio_readonly_config_def_t *read_data = NULL;
    bool enabled = FALSE;

    if (configManagerGetReadOnlyConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID, (const void **)&read_data))
    {
        enabled = read_data->PluginFeatures.enableMixingOfVoiceAndAudio;
        configManagerReleaseConfig(SINK_AUDIO_READONLY_CONFIG_BLK_ID);
    }
    return enabled;
}

static a2dp_index_t getA2dpIndexFromSource(audio_sources source)
{
    PanicFalse(isAudioSourceA2dp(source));

    return (source == audio_source_a2dp_1 ? a2dp_primary : a2dp_secondary);
}

static void setA2dpSourcePriority(a2dp_source_priority_t priority)
{
    AUDIO_GDATA.a2dp_source_priority = priority;
}

static a2dp_source_priority_t getA2dpSourcePriority(void)
{
    return (sinkAudioIsManualSrcSelectionEnabled() ? a2dp_priority_normal : AUDIO_GDATA.a2dp_source_priority);
}

static void sinkAudioConfigurePcmInputs(void)
{
    if(isAudioSourceEnabled(audio_source_I2S))
    {
        sinkWiredConfigureI2sInput();
    }
        
    if(isAudioSourceEnabled(audio_source_SPDIF))
    {
        sinkWiredConfigureSpdifInput();
    }    
}

