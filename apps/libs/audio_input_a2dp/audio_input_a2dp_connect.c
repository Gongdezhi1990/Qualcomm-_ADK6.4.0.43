/****************************************************************************
Copyright (c) 2016-2019 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_connect.c

DESCRIPTION
    Implementation of AUDIO_PLUGIN_CONNECT_MSG message for a2dp source.
    Remaining message handlers are implemented in audio_input_common library.
*/

#include "audio_input_a2dp_connect.h"
#include "audio_input_a2dp_rtp_config.h"
#include "audio_input_a2dp_task.h"
#include "audio_input_a2dp_broadcast.h"
#include "audio_input_a2dp_chain_config.h"
#include <audio_config.h>
#include <operators.h>
#include <audio_mixer.h>
#include <audio.h>
#include <panic.h>


/* Make sure buffer is large enough to hold number of samples corresponding to the latency */
#define SPLITTER_BUFFER_SIZE (2048)

#define MAX_UINT16 (0xFFFF)

static unsigned getKickPeriodFromTask(Task task)
{
    if(AudioPluginA2dpTaskIsBroadcaster(task))
        return BA_KICK_PERIOD;

    if(AudioConfigGetAlwaysUseAecRef())
        return DEFAULT_KICK_PERIOD;

    return AudioInputCommonGetKickPeriodFromCodec(AudioPluginA2dpTaskGetCodec(task));
}

static void configureOperatorsInChain(Task task, kymera_chain_handle_t chain, audio_codec_t decoder,
                                     uint32 sample_rate, A2dpPluginConnectParams *a2dp_connect_params)
{
    Operator operator;
    bool use_sram = FALSE;

    operator = ChainGetOperatorByRole(chain, rtp_role);
    if(operator != INVALID_OPERATOR)
        audioInputA2dpConfigureRtpOperator(operator, chain, decoder, sample_rate, a2dp_connect_params);

    operator = ChainGetOperatorByRole(chain, splitter_role);
    if(operator != INVALID_OPERATOR)
    {
        if(!AudioPluginA2dpTaskIsBroadcaster(task))
                    use_sram = AudioConfigGetUseSramForA2dp();
        OperatorsConfigureSplitterWithSram(operator, SPLITTER_BUFFER_SIZE, FALSE, operator_data_format_encoded, use_sram);
    }

    operator = ChainGetOperatorByRole(chain, demux_role);
    if(operator != INVALID_OPERATOR)
        OperatorsStandardSetSampleRate(operator, sample_rate);
}

static audio_mixer_input_t getAudioMixerInput(kymera_chain_handle_t chain, uint32 sample_rate)
{
    audio_mixer_input_t audio_mixer_input;
    audio_mixer_connect_t connect_data;

    connect_data.left_src = ChainGetOutput(chain, path_left_output);
    connect_data.right_src = ChainGetOutput(chain, path_right_output);
    connect_data.connection_type = CONNECTION_TYPE_MUSIC_A2DP;
    connect_data.sample_rate = sample_rate;
    connect_data.channel_mode = CHANNEL_MODE_STEREO;
    connect_data.variable_rate = FALSE;

    audio_mixer_input = AudioMixerConnect(&connect_data);
    PanicFalse(audio_mixer_input != audio_mixer_input_error_none);

    return audio_mixer_input;
}

static Transform transformBtClockBasedTimestampToAudioClock(Source source, Sink sink, uint32 sample_rate, bool content_protection_enabled)
{
    if (sample_rate > MAX_UINT16)
        Panic();

    Transform packetiser = PanicNull(TransformPacketise(source, sink));

    /* Using this codec setting the stream is treated like any stream of data, so the content is irrelevant */
    PanicFalse(TransformConfigure(packetiser, VM_TRANSFORM_PACKETISE_CODEC, VM_TRANSFORM_PACKETISE_CODEC_APTX));

    /* This mode is used to get the RTP packet headers and translate the bluetooth clock based timestamps to the local audio clock */
    PanicFalse(TransformConfigure(packetiser, VM_TRANSFORM_PACKETISE_MODE, VM_TRANSFORM_PACKETISE_MODE_TWSPLUS));

    PanicFalse(TransformConfigure(packetiser, VM_TRANSFORM_PACKETISE_SAMPLE_RATE, (uint16) sample_rate));

    PanicFalse(TransformConfigure(packetiser, VM_TRANSFORM_PACKETISE_CPENABLE, (uint16) content_protection_enabled));

    PanicFalse(TransformStart(packetiser));

    return packetiser;
}

static void connectA2dpSourceToChainInput(audio_input_context_t *ctx, audio_codec_t decoder, A2dpPluginConnectParams* a2dp_connect_params)
{
    Source source = ctx->left_source;
    Sink sink = ChainGetInput(ctx->chain, path_input);

    if (audioInputA2dpisTimeToPlayControlledBySource(decoder, a2dp_connect_params))
        ctx->unpacketiser = transformBtClockBasedTimestampToAudioClock(source, sink, ctx->sample_rate, a2dp_connect_params->content_protection);
    else
        StreamConnect(source, sink);
}

static void setDSPKickPeriod(Task task )
{
    unsigned dsp_kick_period = getKickPeriodFromTask(task);

    OperatorsFrameworkSetKickPeriod(dsp_kick_period);
}

void audioInputA2dpConnectHandler(audio_input_context_t *ctx, Task task, const AUDIO_PLUGIN_CONNECT_MSG_T *msg)
{
    A2dpPluginConnectParams* a2dpParams = (A2dpPluginConnectParams *)msg->params;
    audio_codec_t codec = AudioPluginA2dpTaskGetCodec(task);

    AudioInputCommonDspPowerOn();
    setDSPKickPeriod(task);

    ctx->left_source = StreamSourceFromSink(msg->audio_sink);
    AudioInputCommonSetMusicProcessingContext(ctx, a2dpParams);

    ctx->tws.mute_until_start_forwarding = a2dpParams->peer_is_available;
    ctx->sample_rate = msg->rate;
    ctx->app_task = msg->app_task;

    if(AudioPluginA2dpTaskIsBroadcaster(task))
    {
        ctx->chain = audioInputA2dpCreateBroadcasterChain(codec);
        configureOperatorsInChain(task, ctx->chain, codec, ctx->sample_rate, a2dpParams);
        ChainConnect(ctx->chain);

        ctx->ba.plugin = a2dpParams->ba_output_plugin;
        audioInputA2dpBroadcastCreate(task, ctx->ba.plugin, ctx);

        SetAudioBusy(task);
    }
    else
    {
        StreamDisconnect(ctx->left_source, NULL);
        
        ctx->chain = audioInputA2dpCreateChain(codec);
        configureOperatorsInChain(task, ctx->chain, codec, ctx->sample_rate, a2dpParams);
        ChainConnect(ctx->chain);

        ctx->mixer_input = getAudioMixerInput(ctx->chain, ctx->sample_rate);

        connectA2dpSourceToChainInput(ctx, codec, a2dpParams);
        ChainStart(ctx->chain);

        AudioInputCommonConnect(ctx, task);
    }
}
