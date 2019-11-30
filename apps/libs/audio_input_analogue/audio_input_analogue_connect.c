/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_analogue_connect.c

DESCRIPTION
    Implementation of AUDIO_PLUGIN_CONNECT_MSG message for wired analogue source.
    Remaining message handlers are implemented in audio_input_common library.
*/
#include <stream.h>
#include <source.h>
#include <panic.h>
#include <print.h>

#include <audio.h>
#include <audio_config.h>
#include <audio_mixer.h>
#include <audio_plugin_music_params.h>
#include <audio_plugin_common.h>
#include <audio_ports.h>
#include <chain.h>
#include <operators.h>

#include "audio_input_analogue_broadcast.h"
#include "audio_input_analogue_connect.h"
#include "audio_input_analogue_chain_config.h"
#include "audio_input_common.h"

#define TTP_BUFFER_SIZE (4096)
#define TTP_BUFFER_SIZE_BROADCASTER (2048)

static A2dpPluginConnectParams *getConnectParamsFromMessage(Message message);
static kymera_chain_handle_t createChain(Task task, uint32 sample_rate);
static void createSources(audio_input_context_t *ctx, const analogue_input_params* settings, uint32 sample_size);
static Source createLeftSource(const analogue_input_params* settings, uint32 sample_rate, uint32 sample_size);
static Source createRightSource(const analogue_input_params* settings, uint32 sample_rate, uint32 sample_size);
static Source createSource(audio_channel channel, const analogue_input_params params, uint32 sample_rate, uint32 sample_size);
static void connectSourcesToChain(audio_input_context_t *ctx);
static void connectChainToMixer(audio_input_context_t *ctx);
static bool isStereo(audio_input_context_t *ctx);

/****************************************************************************
DESCRIPTION
    AUDIO_PLUGIN_CONNECT_MSG message handler.
    It configures ADC and connects it the audio mixer.
    In case of analogue plugin source is not passed by application.
    The source is obtained in here based on instance parameter in
    connect_params->mic_params->line_in.
*/
void AudioInputAnalogueConnectHandler(Task task, Message message, audio_input_context_t* ctx)
{
    const AUDIO_PLUGIN_CONNECT_MSG_T* connect_message = (const AUDIO_PLUGIN_CONNECT_MSG_T*)message;
    A2dpPluginConnectParams *connect_params = getConnectParamsFromMessage(message);
    bool samples_24_bit = connect_params->analogue_in_params->enable_24_bit_resolution;
    
    unsigned sample_size = samples_24_bit ? RESOLUTION_MODE_24BIT : RESOLUTION_MODE_16BIT;

    PRINT(("analogue connected sample rate %d, sample_size %d, stereo %d\n",
            connect_message->rate, sample_size, is_stereo));

    AudioInputCommonDspPowerOn();
    OperatorsFrameworkSetKickPeriod(AudioInputCommonGetKickPeriod());
    ctx->sample_rate = connect_message->rate;
    ctx->tws.encoder_bitpool = connect_params->bitpool;
    ctx->tws.encoder_settings = connect_params->format;
    ctx->tws.mute_until_start_forwarding = connect_params->peer_is_available;

    AudioInputCommonSetMusicProcessingContext(ctx, connect_params);
    
    createSources(ctx, connect_params->analogue_in_params, sample_size);
    AudioInputAnalogueConnect(task, message, ctx);
}

void AudioInputAnalogueConnect(Task task, Message message, audio_input_context_t* ctx)
{   
    if (!AudioInputCommonTaskIsBroadcaster(task))
    {
        ctx->chain = createChain(task, ctx->sample_rate);
        connectSourcesToChain(ctx);

        ChainConnect(ctx->chain);

        connectChainToMixer(ctx);
    
        ChainStart(ctx->chain);

        AudioInputCommonConnect(ctx, task);
    }
    else
    {
        PRINT(("analogue broadcast\n"));

        ctx->ba.plugin = getConnectParamsFromMessage(message)->ba_output_plugin;

        OperatorsFrameworkSetKickPeriod(BA_KICK_PERIOD);

        ctx->chain = createChain(task, ctx->sample_rate);
        connectSourcesToChain(ctx);

        audioInputAnalogueBroadcastCreate(task, ctx->ba.plugin, ctx);
        SetAudioBusy(task);
    }
}

A2dpPluginConnectParams *getConnectParamsFromMessage(Message message)
{
    const AUDIO_PLUGIN_CONNECT_MSG_T* connect_message = (const AUDIO_PLUGIN_CONNECT_MSG_T*)message;

    return (A2dpPluginConnectParams *)connect_message->params;
}

static kymera_chain_handle_t createChain(Task task, uint32 sample_rate)
{
    kymera_chain_handle_t chain;
    Operator ttp_pass_op;
    const chain_config_t *config = AudioInputAnalogueGetChainConfig();
    chain = ChainCreate(config);
    ttp_latency_t ttp_latency;
    uint16 ttp_buffer_size = TTP_BUFFER_SIZE;

    ttp_pass_op = ChainGetOperatorByRole(chain, ttp_pass_role);

    if (!AudioInputCommonTaskIsBroadcaster(task))
    {        
        ttp_latency = AudioConfigGetWiredTtpLatency();
        OperatorsStandardSetLatencyLimits(ttp_pass_op,
                                          TTP_LATENCY_IN_US(ttp_latency.min_in_ms),
                                          TTP_LATENCY_IN_US(ttp_latency.max_in_ms));
    }
    else
    {       
        ttp_latency.target_in_ms = TTP_WIRED_BA_LATENCY_IN_MS;
        OperatorsStandardSetLatencyLimits(ttp_pass_op,
                                          TTP_BA_MIN_LATENCY_LIMIT_US,
                                          TTP_BA_MAX_LATENCY_LIMIT_US);
        ttp_buffer_size = TTP_BUFFER_SIZE_BROADCASTER;
    }
    
    OperatorsConfigureTtpPassthrough(ttp_pass_op, TTP_LATENCY_IN_US(ttp_latency.target_in_ms), sample_rate, operator_data_format_pcm);
    OperatorsStandardSetBufferSizeWithFormat(ttp_pass_op, ttp_buffer_size, operator_data_format_pcm);
    
    return chain;
}

static void createSources(audio_input_context_t *ctx, const analogue_input_params* settings, uint32 sample_size)
{
    ctx->left_source = createLeftSource(settings, ctx->sample_rate, sample_size);
    ctx->right_source = createRightSource(settings, ctx->sample_rate, sample_size);

    SourceSynchronise(ctx->left_source, ctx->right_source);
}

static Source createLeftSource(const analogue_input_params* settings, uint32 sample_rate, uint32 sample_size)
{
    return createSource(AUDIO_CHANNEL_A, *settings, sample_rate, sample_size);
}

static Source createRightSource(const analogue_input_params* settings, uint32 sample_rate, uint32 sample_size)
{
    return createSource(AUDIO_CHANNEL_B, *settings, sample_rate, sample_size);
}

static Source createSource(audio_channel channel, const analogue_input_params params, uint32 sample_rate, uint32 sample_size)
{
    Source source;

    source = AudioPluginAnalogueInputSetup(channel, params, sample_rate);
    PanicFalse(SourceConfigure(source, STREAM_AUDIO_SAMPLE_SIZE, sample_size));

    return source;
}

static void connectSourcesToChain(audio_input_context_t *ctx)
{
    StreamConnect(ctx->left_source, ChainGetInput(ctx->chain, path_pcm_input_left));

    if(isStereo(ctx))
    {
        StreamConnect(ctx->right_source, ChainGetInput(ctx->chain, path_pcm_input_right));
    }
}

static void connectChainToMixer(audio_input_context_t *ctx)
{
    audio_mixer_connect_t connect_data;

    connect_data.left_src = ChainGetOutput(ctx->chain, path_pcm_output_left);
    connect_data.right_src = isStereo(ctx) ? ChainGetOutput(ctx->chain, path_pcm_output_right) : NULL;
    connect_data.connection_type = CONNECTION_TYPE_MUSIC;
    connect_data.sample_rate = ctx->sample_rate;
    connect_data.channel_mode = CHANNEL_MODE_STEREO;
    connect_data.variable_rate = FALSE;

    ctx->mixer_input = AudioMixerConnect(&connect_data);

    PanicFalse(ctx->mixer_input != audio_mixer_input_error_none);
}

static bool isStereo(audio_input_context_t *ctx)
{
    return (ctx->right_source) ? TRUE : FALSE;
}
