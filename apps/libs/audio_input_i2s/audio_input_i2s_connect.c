/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_i2s_connect.c

DESCRIPTION
    Implementation of AUDIO_PLUGIN_CONNECT_MSG message for wired i2s source.
    Remaining message handlers are implemented in audio_input_common library.
*/

#include <stream.h>

#include <audio.h>
#include <audio_config.h>
#include <audio_mixer.h>
#include <audio_ports.h>
#include <audio_i2s_common.h>
#include <audio_plugin_music_params.h>
#include <chain.h>
#include <operators.h>

#include "audio_input_i2s_connect.h"
#include "audio_input_i2s_chain_config.h"

#define TTP_BUFFER_SIZE (4096)

static kymera_chain_handle_t createChain(uint32 sample_rate);
static void createSources(audio_input_context_t *ctx, uint16 sample_size);
static void connectSourcesToChain(audio_input_context_t *ctx);
static void connectChainToMixer(audio_input_context_t *ctx);
static bool isStereo(audio_input_context_t *ctx);

void AudioInputI2sConnectHandler(Task task, Message message, audio_input_context_t* ctx)
{
    const AUDIO_PLUGIN_CONNECT_MSG_T* connect_message = (const AUDIO_PLUGIN_CONNECT_MSG_T*)message;
    A2dpPluginConnectParams *connect_params = (A2dpPluginConnectParams *)connect_message->params;
    uint16 sample_size = AudioI2SIs24BitAudioInputEnabled() ? RESOLUTION_MODE_24BIT : RESOLUTION_MODE_16BIT;
    
    ctx->sample_rate = connect_message->rate;
    ctx->tws.encoder_bitpool = connect_params->bitpool;
    ctx->tws.encoder_settings = connect_params->format;
    ctx->tws.mute_until_start_forwarding = connect_params->peer_is_available;
    AudioInputCommonSetMusicProcessingContext(ctx, connect_params);
    

    AudioInputCommonDspPowerOn();
    OperatorsFrameworkSetKickPeriod(AudioInputCommonGetKickPeriod());
    AudioI2SInitialiseDevice(ctx->sample_rate);
    createSources(ctx, sample_size);
    AudioInputI2sConnect(task, message, ctx);
}

void AudioInputI2sConnect(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(message);
    ctx->chain = createChain(ctx->sample_rate);
	ChainConnect(ctx->chain);
    connectSourcesToChain(ctx);
    connectChainToMixer(ctx);
    ChainStart(ctx->chain);
    AudioInputCommonConnect(ctx, task);
}

void AudioInputI2sDisconnect(Task task, Message message, audio_input_context_t* ctx)
{
    /* I2S sources must be disconnected before MCLK is disabled */
    AudioInputCommonDisconnectChainFromMixer(task, message, ctx);

    StreamDisconnect(ctx->left_source, NULL);
    StreamDisconnect(ctx->right_source, NULL);

    AudioI2SSourceEnableMasterClockIfRequired(ctx->left_source, FALSE);
    AudioInputCommonFadeOutCompleteHandler(task, message, ctx);
}

static kymera_chain_handle_t createChain(uint32 sample_rate)
{
    kymera_chain_handle_t chain;
    Operator ttp_pass_op;
    const chain_config_t* chain_config = AudioInputI2sGetChainConfig();
    ttp_latency_t  ttp_latency = AudioConfigGetWiredTtpLatency();

    chain = ChainCreate(chain_config);

    ttp_pass_op = ChainGetOperatorByRole(chain, ttp_pass_role);
    OperatorsConfigureTtpPassthrough(ttp_pass_op, TTP_LATENCY_IN_US(ttp_latency.target_in_ms), sample_rate, operator_data_format_pcm);
    OperatorsStandardSetLatencyLimits(ttp_pass_op, TTP_LATENCY_IN_US(ttp_latency.min_in_ms),
                                          TTP_LATENCY_IN_US(ttp_latency.max_in_ms));

    OperatorsStandardSetBufferSizeWithFormat(ttp_pass_op, TTP_BUFFER_SIZE, operator_data_format_pcm);

    return chain;
}

static void createSources(audio_input_context_t *ctx, uint16 sample_size)
{
    ctx->left_source = StreamAudioSource(AUDIO_HARDWARE_I2S, AudioConfigGetI2sAudioInstance(), AUDIO_CHANNEL_SLOT_0);
    AudioI2SConfigureSource(ctx->left_source, ctx->sample_rate, sample_size);

    ctx->right_source = StreamAudioSource(AUDIO_HARDWARE_I2S, AudioConfigGetI2sAudioInstance(), AUDIO_CHANNEL_SLOT_1);
    AudioI2SConfigureSource(ctx->right_source, ctx->sample_rate, sample_size);

    SourceSynchronise(ctx->left_source, ctx->right_source);
    
    AudioI2SSourceEnableMasterClockIfRequired(ctx->left_source, TRUE);
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
