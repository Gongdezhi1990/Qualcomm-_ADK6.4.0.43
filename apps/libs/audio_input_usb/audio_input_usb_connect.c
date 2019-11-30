/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_usb_connect.c

DESCRIPTION
    Implementation of AUDIO_PLUGIN_CONNECT_MSG message for usb source.
    Remaining message handlers are implemented in audio_input_common library.
*/

#include <operators.h>
#include <audio_mixer.h>
#include <audio.h>
#include <audio_plugin_music_params.h>
#include <audio_plugin_music_variants.h>
#include <audio_ports.h>
#include <panic.h>
#include <print.h>

#include "audio_input_usb.h"
#include "audio_input_usb_broadcast.h"
#include "audio_input_usb_connect.h"
#include "audio_input_usb_chain_config.h"
#include "audio_config.h"

#define CHANNELS_NUMBER_MONO    1
#define CHANNELS_NUMBER_STEREO  2

#define SAMPLE_SIZE_16_BIT      2
#define SAMPLE_SIZE_24_BIT      3

#define TTP_BUFFER_SIZE (4096)
#define TTP_BUFFER_SIZE_BROADCASTER (2048)


static kymera_chain_handle_t createChain(Task task, unsigned sample_rate, unsigned sample_size, unsigned number_of_channels);

/****************************************************************************/
static void audioInputUsbSetSwitchedPassthroughMode(audio_input_context_t* ctx, spc_mode_t mode)
{
    Operator switched_passthrough = ChainGetOperatorByRole(ctx->chain, switched_passthrough_role);
    if (switched_passthrough)
        OperatorsSetSwitchedPassthruMode(switched_passthrough, mode);
}

/****************************************************************************
DESCRIPTION
    AUDIO_PLUGIN_CONNECT_MSG message handler.
    It creates Kymera chain and connects it to source and audio mixer.
*/
void AudioInputUsbConnectHandler(Task task, Message message, audio_input_context_t* ctx)
{
    const AUDIO_PLUGIN_CONNECT_MSG_T* connect_message = (const AUDIO_PLUGIN_CONNECT_MSG_T*)message;
    A2dpPluginConnectParams *connect_params = (A2dpPluginConnectParams *)connect_message->params;

    /* For now all USB audio endpoints only support 16 bit resolution */
    unsigned sample_size = SAMPLE_SIZE_16_BIT;
    unsigned number_of_channels = CHANNELS_NUMBER_STEREO;

    PRINT(("AudioInputUsbConnect: sample rate:%d, sample size:%d, number of channels:%d\n", connect_message->rate, sample_size, number_of_channels));

    AudioInputCommonDspPowerOn();
    OperatorsFrameworkSetKickPeriod(AudioInputCommonGetKickPeriod());

    ctx->sample_rate = connect_message->rate;

    ctx->chain = createChain(task, ctx->sample_rate, sample_size, number_of_channels);
    ctx->codec_source = StreamSourceFromSink(connect_message->audio_sink);
    ctx->left_source = ChainGetOutput(ctx->chain, path_pcm_left);
    ctx->right_source = isMonoMode()? ChainGetOutput(ctx->chain, path_pcm_forwarding): ChainGetOutput(ctx->chain, path_pcm_right);
    ctx->tws.encoder_bitpool = connect_params->bitpool;
    ctx->tws.encoder_settings = connect_params->format;
    ctx->tws.mute_until_start_forwarding = connect_params->peer_is_available;

    StreamDisconnect(ctx->codec_source, NULL);
    ChainConnect(ctx->chain);
    StreamConnect(ctx->codec_source, ChainGetInput(ctx->chain, path_encoded));

    AudioInputCommonSetMusicProcessingContext(ctx, connect_params);

    if (AudioInputCommonTaskIsBroadcaster(task))
    {
        ctx->ba.plugin = connect_params->ba_output_plugin;

        OperatorsFrameworkSetKickPeriod(BA_KICK_PERIOD);

        audioInputUsbBroadcastCreate(task, ctx->ba.plugin, ctx);
        SetAudioBusy(task);
    }
    else
    {
        audioInputUsbConnectToMixer(task, message, ctx);
    }
}


/****************************************************************************
DESCRIPTION
    Create Kymera chain for usb audio rx.
*/
static kymera_chain_handle_t createChain(Task task, unsigned sample_rate, unsigned sample_size,
        unsigned number_of_channels)
{
    kymera_chain_handle_t chain;
    Operator usb_audio_rx_op;
    usb_config_t config;
    ttp_latency_t ttp_latency;
    uint16 ttp_buffer_size = TTP_BUFFER_SIZE;


    const chain_config_t* chain_config = AudioInputUsbGetChainConfig();
    config.sample_rate = sample_rate;
    config.sample_size = sample_size;
    config.number_of_channels = number_of_channels;
    chain = PanicNull(ChainCreate(chain_config));

    usb_audio_rx_op = ChainGetOperatorByRole(chain, usb_rx_role);
    OperatorsConfigureUsbAudio(usb_audio_rx_op, config);

    if (!AudioInputCommonTaskIsBroadcaster(task))
    {
        ttp_latency = AudioConfigGetWiredTtpLatency();
        OperatorsStandardSetLatencyLimits(usb_audio_rx_op,
                                          TTP_LATENCY_IN_US(ttp_latency.min_in_ms),
                                          TTP_LATENCY_IN_US(ttp_latency.max_in_ms));
    }
    else
    {
        ttp_latency.target_in_ms = TTP_WIRED_BA_LATENCY_IN_MS;
        OperatorsStandardSetLatencyLimits(usb_audio_rx_op,
                                          TTP_BA_MIN_LATENCY_LIMIT_US,
                                          TTP_WIRED_BA_LATENCY_IN_US + (75 * US_PER_MS));
        ttp_buffer_size = TTP_BUFFER_SIZE_BROADCASTER;
    }

    OperatorsStandardSetTimeToPlayLatency(usb_audio_rx_op, TTP_LATENCY_IN_US(ttp_latency.target_in_ms));
    OperatorsStandardSetBufferSizeWithFormat(usb_audio_rx_op, ttp_buffer_size, operator_data_format_pcm);
    return chain;
}

void audioInputUsbConnectToMixer(Task task, Message message, audio_input_context_t* ctx)
{

    audio_mixer_connect_t connect_data;
    UNUSED(message);

    connect_data.left_src = ctx->left_source;
    connect_data.right_src = isMonoMode()? NULL: ctx->right_source;
    connect_data.connection_type = CONNECTION_TYPE_MUSIC;
    connect_data.sample_rate = ctx->sample_rate;
    connect_data.channel_mode = CHANNEL_MODE_STEREO;
    connect_data.variable_rate = FALSE;

    ctx->mixer_input = AudioMixerConnect(&connect_data);

    PanicFalse(ctx->mixer_input != audio_mixer_input_error_none);

    ChainStart(ctx->chain);

    AudioInputCommonConnect(ctx, task);
}

void audioInputUsbEnableForwardingMonoOutput(audio_input_context_t* ctx)
{
    audioInputUsbSetSwitchedPassthroughMode(ctx, spc_op_mode_passthrough);
}

void audioInputUsbDisableForwardingMonoOutput(audio_input_context_t* ctx)
{
    audioInputUsbSetSwitchedPassthroughMode(ctx, spc_op_mode_consumer);
}
