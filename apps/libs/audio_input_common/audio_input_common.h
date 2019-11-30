/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common.h

DESCRIPTION
    Default handlers for non input specific messages.
    To be used as common implementation for audio_input_xxx libraries.
*/

#ifndef AUDIO_INPUT_COMMON_H_
#define AUDIO_INPUT_COMMON_H_

#include <message.h>
#include <transform.h>
#include <chain.h>
#include <audio_mixer.h>
#include <audio_plugin_if.h>
#include <audio_plugin_music_params.h>
#include <audio_plugin_forwarding.h>
#include <rtime.h>
#include <ttp_latency.h>

#define TTP_LATENCY_IN_US(latency_in_ms) (latency_in_ms * US_PER_MS)

#define TTP_APTX_LL_ESTIMATED_LATENCY_IN_MS (32)

#define TTP_BA_LATENCY_IN_US (BA_A2DP_LATENCY_MS * US_PER_MS)
#define TTP_BA_MIN_LATENCY_LIMIT_US (120 * US_PER_MS)
#define TTP_BA_MAX_LATENCY_LIMIT_US (TTP_BA_LATENCY_IN_US + (150 * US_PER_MS))

#define TTP_WIRED_BA_LATENCY_IN_MS (430)
#define TTP_WIRED_BA_LATENCY_IN_US (TTP_WIRED_BA_LATENCY_IN_MS * US_PER_MS)

#define BA_KICK_PERIOD (10000)

typedef enum
{
    audio_input_idle,
    audio_input_connecting,
    audio_input_connected,
    audio_input_disconnecting,
    audio_input_forwarding_setup,
    audio_input_forwarding,
    audio_input_forwarding_tear_down,
    audio_input_forwarding_disconnect,
    audio_input_error_state,
    audio_input_number_of_states,
    audio_input_invalid_state
} audio_input_state_t;

typedef enum
{
    audio_input_connect_req,
    audio_input_connect_complete,
    audio_input_disconnect_req,
    audio_input_disconnect_complete,
    audio_input_forward_req,
    audio_input_forward_created,
    audio_input_forward_started,
    audio_input_forward_stop_req,
    audio_input_forward_stopped,
    audio_input_forward_destroyed,
    audio_input_error,
    audio_input_number_of_events
} audio_input_event_t;

struct __audio_input_context;

typedef void (*audio_input_handler_t)(Task task, Message message, struct __audio_input_context* ctx);

typedef struct
{
    audio_input_handler_t handlers[audio_input_number_of_states][audio_input_number_of_events];
} audio_input_state_table_t;

typedef struct
{
    Task plugin;
    unsigned encoder_bitpool;
    unsigned encoder_settings;
    bool mute_until_start_forwarding;
} tws_context_t;

typedef struct
{
    Task plugin;
} ba_context_t;

typedef struct __audio_input_context
{
    kymera_chain_handle_t chain;
    audio_mixer_input_t mixer_input;
    Transform unpacketiser;

    Source codec_source;
    Source left_source;
    Source right_source;
    uint32 sample_rate;

    A2DP_MUSIC_PROCESSING_T music_processing;
    uint16 music_enhancements;

    audio_input_state_t state;
    const audio_input_state_table_t* state_table;

    tws_context_t tws;
    ba_context_t ba;

    Task app_task;
} audio_input_context_t;

/****************************************************************************
DESCRIPTION
    Message handler for messages common among all input plugins.
*/
void AudioInputCommonMessageHandler(Task task, MessageId id, Message message, audio_input_context_t *ctx);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_PLUGIN_DISCONNECT_MSG message.
*/
void AudioInputCommonDisconnectHandler(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Stop the ctx chain and disconnect it from the audio mixer
*/
void AudioInputCommonDisconnectChainFromMixer(Task task, Message message, audio_input_context_t *ctx);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_MIXER_FADE_OUT_CFM message.
*/
void AudioInputCommonFadeOutCompleteHandler(Task task, Message message, audio_input_context_t *ctx);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T message.
*/
void AudioInputCommonSetSoftMuteHandler(const AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T *msg, audio_input_context_t *ctx);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T message.
*/
void AudioInputCommonSetGroupVolumeHandler(const AUDIO_PLUGIN_SET_GROUP_VOLUME_MSG_T *msg);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_PLUGIN_PLAY_TONE_MSG_T message.
    It is forwarding message to voice prompt plugin.
*/
void AudioInputCommonPlayToneHandler(const AUDIO_PLUGIN_PLAY_TONE_MSG_T *msg);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG message.
    It is forwarding message to voice prompt plugin.
*/
void AudioInputCommonStopToneHandler(void);

/****************************************************************************
DESCRIPTION
    Common functionality required at the end of a connection request.
    All plugins that handle a connect message should call this function prior to returning.
*/
void AudioInputCommonConnect(audio_input_context_t *ctx, Task task);

/****************************************************************************
DESCRIPTION
    Handler for AUDIO_MIXER_FADE_IN_CFM msg
*/
void AudioInputCommonFadeInCompleteHandler(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Set music processing features
*/
void AudioInputCommonSetMusicProcessingContext(audio_input_context_t *ctx, A2dpPluginConnectParams* params);

/****************************************************************************
DESCRIPTION
    Set music processing PEQ parameters
*/
void AudioInputCommonSetUserEqParameter(const audio_plugin_user_eq_param_t* param);

/****************************************************************************
DESCRIPTION
    Apply stored music processing PEQ parameters
*/
void AudioInputCommonApplyUserEqParameters(void);

/****************************************************************************
DESCRIPTION
    Clear stored music processing PEQ parameters
*/
void AudioInputCommonClearUserEqParameters(void);

/****************************************************************************
DESCRIPTION
    Function to return a music processing PEQ parameter.
*/
void AudioInputCommonGetUserEqParameter(const audio_plugin_user_eq_param_id_t* param_id, Task audio_plugin, Task callback_task);

/****************************************************************************
DESCRIPTION
    Function to return a group of music processing PEQ parameters.
*/
void AudioInputCommonGetUserEqParameters(const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids, Task audio_plugin, Task callback_task);

/****************************************************************************
DESCRIPTION
    Set music processing features
*/
void AudioInputCommonSetMode (const AUDIO_MODE_T mode, const void * params);

/****************************************************************************
DESCRIPTION
    Send response from a get parameter request to sink app
*/
void AudioInputCommonSendGetUserEqParameterResponse(const bool data_valid, const audio_plugin_user_eq_param_t* param);

/****************************************************************************
DESCRIPTION
    Send response from a get parameters request to sink app
*/
void AudioInputCommonSendGetUserEqParametersResponse(const bool data_valid, const unsigned number_of_params, const audio_plugin_user_eq_param_t* params);

/****************************************************************************
DESCRIPTION
    Function to update the Music Channel Mode of an active music input
*/
void AudioInputCommonSetTwsChannelMode(void);

/****************************************************************************
DESCRIPTION
    Handle start forwarding request
*/
void AudioInputCommonForwardStart(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Handle forwarding create confirm
*/
void AudioInputCommonForwardHandleCreateCfm(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Handle forwarding start confirm
*/
void AudioInputCommonForwardHandleStartCfm(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Handle stop forwarding request
*/
void AudioInputCommonForwardStop(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Handle forwarding destroy confirmation
*/
void AudioInputCommonForwardDestroy(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Set state table for context state machine
*/
void AudioInputCommonSetStateTable(const audio_input_state_table_t* table, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Handler used to explicitly ignore an event
*/
void AudioInputCommonIgnoreEvent(Task task, Message message, audio_input_context_t* ctx);

/****************************************************************************
DESCRIPTION
    Send an error event to task
*/
void AudioInputCommmonSendError(Task task);

/****************************************************************************
DESCRIPTION
    Send a connect complete event to task
*/
void AudioInputCommmonSendConnectComplete(Task task);

/****************************************************************************
DESCRIPTION
    Check if a plugin is a broadcast capable.
*/
bool AudioInputCommonTaskIsBroadcaster(Task task);

/****************************************************************************
DESCRIPTION
    Enables the operator framework and set DSP to loaded-idle
*/
void AudioInputCommonDspPowerOn(void);

/****************************************************************************
DESCRIPTION
    Get the kick period.
*/
unsigned AudioInputCommonGetKickPeriod(void);

/****************************************************************************
DESCRIPTION
    Get the kick period for a given codec.
*/
unsigned AudioInputCommonGetKickPeriodFromCodec(audio_codec_t codec);

#endif /* AUDIO_INPUT_COMMON_H_ */
