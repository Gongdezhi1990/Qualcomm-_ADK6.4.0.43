/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_spdif_connect.c

DESCRIPTION
    Implementation of AUDIO_PLUGIN_CONNECT_MSG message for spdif source.
    Remaining message handlers are implemented in audio_input_common library.
*/

#include <operators.h>
#include <audio.h>
#include <audio_config.h>
#include <audio_mixer.h>
#include <audio_ports.h>
#include <print.h>

#include "audio_input_spdif_private.h"
#include "audio_input_spdif_connect.h"
#include "audio_input_spdif_driver.h"
#include "audio_input_spdif_chain_config.h"


#define SPDIF_SAMPLE_SIZE       (24)
#define TTP_BUFFER_SIZE         (4096)


typedef enum
{
    spdif_source_left,
    spdif_source_right,
    spdif_source_interleaved

} spdif_source_t;


static kymera_chain_handle_t createChain(void);
static void createSources(audio_input_context_t *ctx);
static Source createSpdifSource(spdif_source_t source_type);
static void configureSpdifSource(Source source);
static void synchroniseSpdifSources(Source left_source, Source right_source);
static void connectSourcesToChain(audio_input_context_t *ctx);
static void connectChainToMixer(audio_input_context_t *ctx);


/****************************************************************************
DESCRIPTION
    AUDIO_PLUGIN_CONNECT_MSG message handler.
    It creates Kymera chain and connects it to audio mixer.
*/
void AudioInputSpdifConnectHandler(audio_input_context_t *ctx, Task task,
                                   const AUDIO_PLUGIN_CONNECT_MSG_T *connect_message)
{
    A2dpPluginConnectParams *connect_params = (A2dpPluginConnectParams *)connect_message->params;

    PRINT(("AudioInputSpdifConnect\n"));

    AudioInputCommonDspPowerOn();
    OperatorsFrameworkSetKickPeriod(AudioInputCommonGetKickPeriod());

    ctx->chain = createChain();

    /* The Spdif is a variable sample rate input, dynamically changing the 
    buffer size each time the sample rate changes is not possible with current 
    architecture. Hence the configuration item max sample rate is used which 
    indicates the maximum sample rate that will be used when using the variable
    rate input, the buffer allocation for the operators are done only once
    using this configuration item.*/

    ctx->sample_rate = AudioConfigGetMaxSampleRate();
    ctx->tws.mute_until_start_forwarding = connect_params->peer_is_available;

    createSources(ctx);

    connectSourcesToChain(ctx);

    connectChainToMixer(ctx);

    AudioInputCommonSetMusicProcessingContext(ctx, connect_params);

    AudioInputSpdifSetTargetLatency();

    ChainStart(ctx->chain);

    AudioInputCommonConnect(ctx, task);
}

/****************************************************************************
DESCRIPTION
    Create Kymera chain for spdif decoder.
*/
static kymera_chain_handle_t createChain(void)
{
    kymera_chain_handle_t chain;
    const chain_config_t* chain_config = AudioInputSpdifGetChainConfig();
    Operator spdif_op;
    Operator ttp_op;

    chain = PanicNull(ChainCreate(chain_config));

    spdif_op = ChainGetOperatorByRole(chain, spdif_decoder_role);
    MessageOperatorTask(spdif_op, AudioInputSpdifGetDriverHandlerTask());

    ttp_op = ChainGetOperatorByRole(chain, spdif_ttp_passthrough_role);
    OperatorsStandardSetBufferSizeWithFormat(ttp_op, TTP_BUFFER_SIZE, operator_data_format_pcm);

    ChainConnect(chain);

    return chain;
}

/****************************************************************************
DESCRIPTION
    Helper function to create sources.
*/
static void createSources(audio_input_context_t *ctx)
{
    Sink interleaved_sink = ChainGetInput(ctx->chain, spdif_input_interleaved);

    if(interleaved_sink)
    {
        PRINT(("Spdif input source: interleaved\n"));
        ctx->left_source = createSpdifSource(spdif_source_interleaved);
    }
    else
    {
        PRINT(("Spdif input source: two channel\n"));
        ctx->left_source = createSpdifSource(spdif_source_left);
        ctx->right_source = createSpdifSource(spdif_source_right);

        synchroniseSpdifSources(ctx->left_source, ctx->right_source);
    }
}

/****************************************************************************
DESCRIPTION
    Creates and configures selected spdif source.
*/
static Source createSpdifSource(spdif_source_t source_type)
{
    Source source = 0;
    audio_instance spdif_audio_instance = AudioConfigGetSpdifAudioInstance();

    switch(source_type)
    {
        case spdif_source_left:
        {
            source = StreamAudioSource(AUDIO_HARDWARE_SPDIF, spdif_audio_instance, SPDIF_CHANNEL_A);
            break;
        }

        case spdif_source_right:
        {
            source = StreamAudioSource(AUDIO_HARDWARE_SPDIF, spdif_audio_instance, SPDIF_CHANNEL_B);
            break;
        }

        case spdif_source_interleaved:
        {
            source = StreamAudioSource(AUDIO_HARDWARE_SPDIF, spdif_audio_instance, SPDIF_CHANNEL_A_B_INTERLEAVED);
            break;
        }

        default:
            Panic();
    }

    PanicNull(source);
    configureSpdifSource(source);

    return source;
}

/****************************************************************************
DESCRIPTION
    Configures spdif source.
*/
static void configureSpdifSource(Source source)
{
    PanicFalse(SourceConfigure(source, STREAM_AUDIO_SAMPLE_SIZE, SPDIF_SAMPLE_SIZE));
    PanicFalse(SourceConfigure(source, STREAM_SPDIF_AUTO_RATE_DETECT, TRUE));
    PanicFalse(SourceConfigure(source, STREAM_SPDIF_CHNL_STS_REPORT_MODE, TRUE));
}

/****************************************************************************
DESCRIPTION
    Helper function for synchronising spdif sources.
*/
static void synchroniseSpdifSources(Source left_source, Source right_source)
{
    PRINT(("Spdif input: synchronise sources\n"));
    PanicFalse(SourceSynchronise(left_source, right_source));
}

/****************************************************************************
DESCRIPTION
    Connect inputs to the operators chain.
*/
static void connectSourcesToChain(audio_input_context_t *ctx)
{
    Sink interleaved_sink = ChainGetInput(ctx->chain, spdif_input_interleaved);

    if(interleaved_sink)
    {
        PanicNull(StreamConnect(ctx->left_source, interleaved_sink));
    }
    else
    {
        PanicNull(StreamConnect(ctx->left_source, ChainGetInput(ctx->chain, spdif_input_left)));
        PanicNull(StreamConnect(ctx->right_source, ChainGetInput(ctx->chain, spdif_input_right)));
    }
}

/****************************************************************************
DESCRIPTION
    Connect the operators chain to the mixer.
*/
static void connectChainToMixer(audio_input_context_t *ctx)
{
    audio_mixer_connect_t connect_data;

    connect_data.left_src = ChainGetOutput(ctx->chain, spdif_output_left);
    connect_data.right_src = ChainGetOutput(ctx->chain, spdif_output_right);
    connect_data.connection_type = CONNECTION_TYPE_MUSIC;
    connect_data.sample_rate = ctx->sample_rate;
    connect_data.channel_mode = CHANNEL_MODE_STEREO;
    connect_data.variable_rate = TRUE;

    ctx->mixer_input = AudioMixerConnect(&connect_data);

    PanicFalse(ctx->mixer_input != audio_mixer_input_error_none);
}
