/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_tws_connect.c

DESCRIPTION
    Implementation of tws source specific AUDIO_PLUGIN_CONNECT_MSG_T
    message handlers.
    Remaining message handlers are implemented in audio_input_common library.
*/

#include <stdlib.h>
#include <operators.h>
#include <audio_mixer.h>
#include <audio.h>
#include <audio_plugin_music_params.h>
#include <audio_plugin_music_variants.h>
#include <packetiser_helper.h>
#include <tws_packetiser.h>
#include <audio_input_common.h>
#include <audio_config.h>
#include <panic.h>

#include "audio_input_tws.h"
#include "audio_input_tws_connect.h"
#include "audio_input_tws_chain_config.h"

#define SAMPLE_RATE_44100   (44100)
#define SAMPLE_RATE_48000   (48000)

#define CHAIN_INPUT_BUFFER_SIZE 8192

static tws_packetiser_slave_t * tws_unpacketiser = NULL;

static tws_packetiser_codec_t getTwsCodecTypeFromDecoderPluginType(A2DP_DECODER_PLUGIN_TYPE_T plugin_type);
static rtime_sample_rate_t getRtimeSampleRateFromInputRate(uint32 sample_rate);
static packetiser_helper_scmst_t getScmstFromContentProtection(bool content_protection);
static void connectSourceToDecoderChainViaUnpacketiser(Task task, Source source, Sink sink,
                                        A2DP_DECODER_PLUGIN_TYPE_T plugin_type,
                                        uint32 sample_rate, bool content_protection);
static bool pluginTypeIsTws(A2DP_DECODER_PLUGIN_TYPE_T plugin_type);
static kymera_chain_handle_t createDecoderChain(audio_input_tws_decoder_t decoder);
static audio_input_tws_decoder_t getInputDecoderTypeFromPluginType(A2DP_DECODER_PLUGIN_TYPE_T decoder);
static unsigned getTwsKickPeriodFromDecoderPluginType(A2DP_DECODER_PLUGIN_TYPE_T plugin_type);

/****************************************************************************
DESCRIPTION
    AUDIO_PLUGIN_CONNECT_MSG message handler.
    It creates a Kymera decoder chain and connects it to source and
    audio_mixer via the TWS unpacketiser.
*/
void AudioInputTwsConnectHandler(Task task, Message message, audio_input_context_t *ctx)
{
    const AUDIO_PLUGIN_CONNECT_MSG_T *msg = (const AUDIO_PLUGIN_CONNECT_MSG_T*)message;
    A2DP_DECODER_PLUGIN_TYPE_T decoder = ((A2dpPluginTaskdata*)task)->a2dp_plugin_variant;
    A2dpPluginConnectParams* params = (A2dpPluginConnectParams *)msg->params;

    if(pluginTypeIsTws(decoder))
    {
        Sink audio_frame_sink;
        uint32 input_sample_rate = msg->rate;
        bool content_protection = params->content_protection;
        audio_mixer_connect_t connect_data;

        AudioInputCommonDspPowerOn();
        OperatorsFrameworkSetKickPeriod(getTwsKickPeriodFromDecoderPluginType(decoder));

        ctx->left_source = StreamSourceFromSink(msg->audio_sink);

        StreamDisconnect(ctx->left_source, NULL);

        ctx->chain = createDecoderChain(getInputDecoderTypeFromPluginType(decoder));

        audio_frame_sink = ChainGetInput(ctx->chain, path_input);

        connectSourceToDecoderChainViaUnpacketiser(task,
                                        ctx->left_source,
                                        audio_frame_sink,
                                        decoder,
                                        input_sample_rate,
                                        content_protection);

        if(tws_unpacketiser)
        {
            connect_data.left_src = ChainGetOutput(ctx->chain, path_left_output);
            connect_data.right_src = ChainGetOutput(ctx->chain, path_right_output);
            connect_data.connection_type = CONNECTION_TYPE_MUSIC_A2DP;
            connect_data.sample_rate = msg->rate;
            connect_data.channel_mode = AudioConfigGetTwsChannelModeLocal();
            connect_data.variable_rate = FALSE;

            ctx->mixer_input = AudioMixerConnect(&connect_data);
            ctx->tws.mute_until_start_forwarding = TRUE;

            PanicFalse(ctx->mixer_input != audio_mixer_input_error_none);

            AudioInputCommonSetMusicProcessingContext(ctx, params);

            ChainStart(ctx->chain);
            AudioInputCommonConnect(ctx, task);
        }
        else
        {
            /* Enter error state */
            AudioInputCommmonSendError(task);
        }
    }
    else
    {
        Panic();
    }
}

static void audioInputTwsResetPacketiser(void)
{
    if(tws_unpacketiser)
    {
        TwsPacketiserSlaveDestroy(tws_unpacketiser);
        tws_unpacketiser = NULL;
    }
}

/****************************************************************************
DESCRIPTION
    AUDIO_MIXER_FADE_OUT_CFM message handler.
    Destroys the TWS unpacketiser and disconnects the chain.
*/
void AudioInputTwsFadeOutCompleteHandler(Task task, Message message, audio_input_context_t *ctx)
{
    if(ctx->chain)
        ChainStop(ctx->chain);

    audioInputTwsResetPacketiser();
    AudioInputCommonFadeOutCompleteHandler(task, message, ctx);
}

#ifdef HOSTED_TEST_ENVIRONMENT
/* Reset TWS input */
void AudioInputTwsTestReset(void)
{
    audioInputTwsResetPacketiser();
    AudioMixerTestReset();
}
#endif

/****************************************************************************
DESCRIPTION
    Convert A2DP_DECODER_PLUGIN_TYPE_T type to one understood by the
    tws_packetiser library.
*/
static tws_packetiser_codec_t getTwsCodecTypeFromDecoderPluginType(A2DP_DECODER_PLUGIN_TYPE_T plugin_type)
{
    tws_packetiser_codec_t tws_codec_type = TWS_PACKETISER_CODEC_SBC;

    switch(plugin_type)
    {
        case TWS_SBC_DECODER:
            tws_codec_type = TWS_PACKETISER_CODEC_SBC;
            break;
        case TWS_APTX_DECODER:
            tws_codec_type = TWS_PACKETISER_CODEC_APTX;
            break;
        case TWS_AAC_DECODER:
            tws_codec_type = TWS_PACKETISER_CODEC_AAC;
            break;
        case TWS_APTX_AD_DECODER:
            tws_codec_type = TWS_PACKETISER_CODEC_APTX_AD;
            break;
        default:
            Panic();
            break;
    }
    return tws_codec_type;
}

/****************************************************************************
DESCRIPTION
    Get the kick period, depending on the decoder plugin type
*/
static unsigned getTwsKickPeriodFromDecoderPluginType(A2DP_DECODER_PLUGIN_TYPE_T plugin_type)
{
    switch(plugin_type)
    {
        case TWS_SBC_DECODER:
            return AudioInputCommonGetKickPeriodFromCodec(audio_codec_sbc);
        case TWS_APTX_DECODER:
            return AudioInputCommonGetKickPeriodFromCodec(audio_codec_aptx);
        case TWS_AAC_DECODER:
            return AudioInputCommonGetKickPeriodFromCodec(audio_codec_aac);
        default:
            return DEFAULT_KICK_PERIOD;
    }
}

/****************************************************************************
DESCRIPTION
    Convert sample rate to sample rate type understood by the
    tws_packetiser library.
*/
static rtime_sample_rate_t getRtimeSampleRateFromInputRate(uint32 sample_rate)
{
    rtime_sample_rate_t rtime_sample_rate = rtime_sample_rate_44100;

    if(sample_rate == SAMPLE_RATE_44100)
    {
        rtime_sample_rate = rtime_sample_rate_44100;
    }
    else if(sample_rate == SAMPLE_RATE_48000)
    {
        rtime_sample_rate = rtime_sample_rate_48000;
    }
    else
    {
        Panic();
    }
    return rtime_sample_rate;
}

/****************************************************************************
DESCRIPTION
    Convert content protection flag to content protection type understood by the
    tws_packetiser library.
*/
static packetiser_helper_scmst_t getScmstFromContentProtection(bool content_protection)
{
    if(content_protection)
    {
        return packetiser_helper_scmst_copy_prohibited;
    }
    else
    {
        return packetiser_helper_scmst_copy_allowed;
    }
}

/****************************************************************************
DESCRIPTION
    Create Kymera chain for decoder.
*/
static kymera_chain_handle_t createDecoderChain(audio_input_tws_decoder_t decoder)
{
    Operator op;
    kymera_chain_handle_t chain = audioInputTWSCreateChain(decoder);

    op = ChainGetOperatorByRole(chain, passthrough_role);
    OperatorsStandardSetBufferSizeWithFormat(op, CHAIN_INPUT_BUFFER_SIZE, operator_data_format_encoded);
    OperatorsSetPassthroughDataFormat(op, operator_data_format_encoded);

    ChainConnect(chain);

    return chain;
}

/****************************************************************************
DESCRIPTION
    Convert global decoder type to local one.
    Result is limited to actually supported decoders.
*/
static audio_input_tws_decoder_t getInputDecoderTypeFromPluginType(A2DP_DECODER_PLUGIN_TYPE_T decoder)
{
    audio_input_tws_decoder_t supported_decoder = audio_input_tws_sbc;

    switch(decoder)
    {
        case TWS_SBC_DECODER:
            supported_decoder = audio_input_tws_sbc;
            break;
        case TWS_APTX_DECODER:
            supported_decoder = audio_input_tws_aptx;
            break;
        case TWS_AAC_DECODER:
            supported_decoder = audio_input_tws_aac;
            break;
        case TWS_APTX_AD_DECODER:
            supported_decoder = audio_input_tws_aptx_adaptive;
            break;
        default:
            Panic();
            break;
    }

    return supported_decoder;
}

/****************************************************************************
DESCRIPTION
    Uses the tws unpacketiser to unpacketise the incoming tws input and connect
    into the decoder chain.
*/
static void connectSourceToDecoderChainViaUnpacketiser(Task task, Source source, Sink sink,
                                        A2DP_DECODER_PLUGIN_TYPE_T plugin_type,
                                        uint32 sample_rate, bool content_protection)
{
    tws_packetiser_slave_config_t tws_unpacketiser_config;

    PanicNotNull(tws_unpacketiser);

    tws_unpacketiser_config.client = task;
    tws_unpacketiser_config.source = source;
    tws_unpacketiser_config.sink = sink;
    tws_unpacketiser_config.codec = getTwsCodecTypeFromDecoderPluginType(plugin_type);
    tws_unpacketiser_config.sample_rate = getRtimeSampleRateFromInputRate(sample_rate);
    tws_unpacketiser_config.scmst = getScmstFromContentProtection(content_protection);
    tws_unpacketiser_config.mode = TWS_PACKETISER_SLAVE_MODE_TWS;

    tws_unpacketiser = TwsPacketiserSlaveInit(&tws_unpacketiser_config);
}

/****************************************************************************
DESCRIPTION
    Checks whether decoder type is supported by this tws input plugin.
*/
static bool pluginTypeIsTws(A2DP_DECODER_PLUGIN_TYPE_T plugin_type)
{
    switch(plugin_type)
    {
        case TWS_SBC_DECODER:
        case TWS_APTX_DECODER:
        case TWS_APTX_AD_DECODER:
        case TWS_MP3_DECODER:
        case TWS_AAC_DECODER:
            return TRUE;
        default:
            return FALSE;
    }
}

