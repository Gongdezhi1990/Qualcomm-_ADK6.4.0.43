/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_common.h

DESCRIPTION
    Default handlers for non input specific voice messages.
    To be used as common implementation for audio_voice_xxx libraries.
*/

#ifndef AUDIO_VOICE_COMMON_H_
#define AUDIO_VOICE_COMMON_H_

#include <message.h>
#include <chain.h>
#include <audio_plugin_if.h>
#include <audio_mixer.h>
#include <audio_plugin_voice_variants.h>
#include <audio_plugin_music_params.h>

#define AUDIO_RX_TTP_LATENCY               20
#define AUDIO_SWB_AND_UWB_RX_TTP_LATENCY   28
#define AUDIO_RX_MIN_LATENCY               15
#define AUDIO_RX_MAX_LATENCY               25
#define AUDIO_RX_TTP_BUFFER_SIZE           1024

typedef enum
{
    audio_voice_not_loaded,
    audio_voice_connecting,
    audio_voice_connected,
    audio_voice_disconnecting,
    audio_voice_forwarding_disconnect,
    audio_voice_error_state,
    audio_voice_number_of_states,
    audio_voice_invalid_state
} audio_voice_state_t;

typedef enum
{
    audio_voice_connect_req,
    audio_voice_connect_complete,
    audio_voice_set_soft_mute,
    audio_voice_set_volume,
    audio_voice_reset_volume,
    audio_voice_play_tone,
    audio_voice_stop_tone,
    audio_voice_disconnect_req,
    audio_voice_disconnect_complete,
    audio_voice_error,
    audio_voice_number_of_events
} audio_voice_event_t;


typedef enum
{
    hfp_mode_hands_free_kit,        /* HFP enabled */
    hfp_mode_stand_by               /* Stand by (no output, low power) */
} hfp_mode_t;


struct __audio_voice_context;

typedef void (*audio_voice_handler_t)(Task task, Message message, struct __audio_voice_context * ctx);

typedef struct
{
    audio_voice_handler_t handler;
    audio_voice_state_t   next_state;
} state_machine_data_t;


typedef struct
{
    state_machine_data_t handlers[audio_voice_number_of_states][audio_voice_number_of_events];
} audio_voice_state_table_t;

typedef struct __audio_voice_context
{
    Source audio_source;
    Sink audio_sink;

    unsigned variant;
    voice_mics_t voice_mics;
    link_encoding_t encoder;
    mic_input_rate_t mic_input_rate;

    uint32 incoming_sample_rate;

    kymera_chain_handle_t chain;
    audio_mixer_input_t mixer_input;

    const voice_mic_params_t* mic_params;
    AUDIO_MODE_T mode;
    bool mic_muted;
    bool speaker_muted;
    ttp_latency_t ttp_latency;

    audio_voice_state_t state;
    const audio_voice_state_table_t* state_table;
} audio_voice_context_t;

typedef enum
{
    audio_voice_receive_role,
    audio_voice_send_role,
    audio_voice_rate_adjustment_send_role
} audio_voice_common_roles;

/****************************************************************************
DESCRIPTION
    Set state table for context state machine
*/
void AudioVoiceCommonSetStateTable(const audio_voice_state_table_t* table, audio_voice_context_t* ctx);


/****************************************************************************
DESCRIPTION
    Enables the operator framework and set DSP to loaded-idle
*/
void AudioVoiceCommonDspPowerOn(void);


/****************************************************************************
DESCRIPTION
    Create the audio_voice_common context
*/
audio_voice_context_t* AudioVoiceCommonCreateContext(void);

/****************************************************************************
DESCRIPTION
    Destroy the caudio_voice_common context
*/
audio_voice_context_t* AudioVoiceCommonDestroyContext(audio_voice_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Message handler for messages common among all input plugins.
*/
void AudioVoiceCommonMessageHandler(Task task, MessageId id, Message message, audio_voice_context_t *ctx);

/****************************************************************************
DESCRIPTION
    Dummy Message handler for state machine transitions that explicitly do nothing, but should not result in a panic.
*/
void AudioVoiceCommonDummyStateHander(Task task, Message msg, audio_voice_context_t *ctx);

/*******************************************************************************
DESCRIPTION
    Handle a connection request
*/
void AudioVoiceCommonConnectAndFadeIn(Task task, Message msg, audio_voice_context_t* ctx, operator_filters_t* filters);

/*******************************************************************************
DESCRIPTION
    Handle a connection error caused by an invalid SCO source/sink
*/
void AudioVoiceCommonConnectAndFadeInError(Task task, Message msg, audio_voice_context_t* ctx);

/*******************************************************************************
DESCRIPTION
    Handle a successful connection and fade in
*/
void AudioVoiceCommonConnectAndFadeInSuccess(Task task, Message msg, audio_voice_context_t* ctx);

/*******************************************************************************
DESCRIPTION
    Begin the disconnect procedure by fading out
*/
void AudioVoiceCommonDisconnectFadeOut(Task task, Message msg, audio_voice_context_t* ctx);

/*******************************************************************************
DESCRIPTION
    Destroy the CVC chain and destroy the plug-in context
*/
void AudioVoiceCommonDisconnectDestroyChainAndPlugin(Task task, Message msg, audio_voice_context_t* ctx);

/*******************************************************************************
DESCRIPTION
    Destroy the plug-in context
*/
void AudioVoiceCommonDestroyPlugin(Task task, Message msg, audio_voice_context_t* ctx);

/*******************************************************************************
DESCRIPTION
    Play a tone, either by connecting the a tone source directly to the DAC
    if the plug-in is not using the DSP OR by connecting the tone source to the
    DSP tone mixing input.
*/
void AudioVoiceCommonTonePlay(Task task, Message msg, audio_voice_context_t *ctx);

/*******************************************************************************
DESCRIPTION
    Stop a tone from playing.
*/
void AudioVoiceCommonToneStop(Task task, Message msg, audio_voice_context_t *ctx);

/*******************************************************************************
DESCRIPTION
    Tell HFP to update the volume.
*/
void AudioVoiceCommonVolumeSet(Task task, Message msg, audio_voice_context_t *ctx);

/*******************************************************************************
DESCRIPTION
    Set the mute states.
*/
void AudioVoiceCommonVolumeSetSoftMute(Task task, Message msg, audio_voice_context_t* ctx);


/*******************************************************************************
DESCRIPTION
    Returns true if encoder is wideband type.
*/
bool AudioVoiceCommonIsWideband(cvc_plugin_type_t cvc_plugin_type);

/*******************************************************************************
DESCRIPTION
    Returns plugin type
*/
plugin_type_t AudioVoiceCommonGetPluginType(cvc_plugin_type_t cvc_plugin_type);

/*******************************************************************************
DESCRIPTION
    Returns plugin type
*/
bool AudioVoiceCommonIs2Mic(cvc_plugin_type_t cvc_plugin_type);

#endif /* AUDIO_VOICE_COMMON_H_ */
