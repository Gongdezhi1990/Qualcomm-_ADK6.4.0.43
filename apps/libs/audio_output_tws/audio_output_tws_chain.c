/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_chain.c

DESCRIPTION
    TWS chain functionality
*/

#include <panic.h>
#include <stream.h>

#include <chain.h>
#include <rtime.h>
#include <audio_plugin_forwarding.h>
#include <audio_input_common.h>
#include <sbc_encoder_params.h>
#include <audio_config.h>

#include "audio_output_tws.h"
#include "audio_output_tws_chain_config.h"
#include "audio_output_tws_chain.h"

#define SPLITTER_BUFFER_SIZE                  (4096)
#define SBC_MONO_44_1MHZ_BITPOOL_HIGH_QUALITY (31)
#define SBC_MONO_48MHZ_BITPOOL_HIGH_QUALITY   (29)

/* Core chain instance */
static kymera_chain_handle_t tws_output_chain;

static void configureSplitterIfPresent(void)
{
    Operator splitter = ChainGetOperatorByRole(tws_output_chain, splitter_role);
    if(splitter)
    {
        OperatorsConfigureSplitterWithSram(splitter, SPLITTER_BUFFER_SIZE, TRUE, operator_data_format_encoded, FALSE);
    }
}

static void configureTtpPassthroughIfPresent(const audio_plugin_forwarding_params_t* params)
{
    Operator ttp = ChainGetOperatorByRole(tws_output_chain, ttp_role);
    if(ttp)
    {
        OperatorsStandardSetTimeToPlayLatency(ttp, TTP_LATENCY_IN_US(params->ttp_latency.target_in_ms));
        OperatorsStandardSetLatencyLimits(ttp, TTP_LATENCY_IN_US(params->ttp_latency.min_in_ms),
                                               TTP_LATENCY_IN_US(params->ttp_latency.max_in_ms));
        OperatorsStandardSetSampleRate(ttp, params->sample_rate);
        OperatorsSetPassthroughDataFormat(ttp, operator_data_format_pcm);
    }
}

static bool isMonoMode(void)
{
    return (AudioConfigGetRenderingMode()==single_channel_rendering);
}

static sbc_encoder_params_t getMonoSBCEncoderParams(unsigned sample_rate)
{
    sbc_encoder_params_t sbc_params;
    unsigned bitpool;

    sbc_params.number_of_subbands = 8;
    sbc_params.number_of_blocks = 16;
    sbc_params.sample_rate = sample_rate;
    sbc_params.channel_mode = sbc_encoder_channel_mode_mono;
    sbc_params.allocation_method = sbc_encoder_allocation_method_loudness;

    switch (sample_rate)
    {
        case 44100:
            bitpool = SBC_MONO_44_1MHZ_BITPOOL_HIGH_QUALITY;
        break;

        case 48000:
            bitpool = SBC_MONO_48MHZ_BITPOOL_HIGH_QUALITY;
        break;

        default:
            bitpool = SBC_MONO_44_1MHZ_BITPOOL_HIGH_QUALITY;
    }

    sbc_params.bitpool_size = bitpool;

    return sbc_params;
}

static void configureSbcEncoderParams(const audio_plugin_forwarding_params_t* params, Operator sbc_encoder)
{
    if(params->transcode.codec == audio_codec_sbc)
    {
        unsigned bitpool = params->transcode.params.sbc.bitpool;
        unsigned settings = params->transcode.params.sbc.settings;
        sbc_encoder_params_t sbc_params;

        if(isMonoMode())
        {
            sbc_params = getMonoSBCEncoderParams(params->sample_rate);
        }
        else
        {
            sbc_params = ConvertSbcEncoderParams(bitpool, settings);
        }

        OperatorsSbcEncoderSetEncodingParams(sbc_encoder, &sbc_params);
    }
}

static void configureSbcEncoderIfPresent(const audio_plugin_forwarding_params_t* params)
{
    Operator sbc_encoder = ChainGetOperatorByRole(tws_output_chain, encoder_role);

    if(sbc_encoder)
        configureSbcEncoderParams(params, sbc_encoder);
}

static void configureSecondSbcEncoderIfPresent(const audio_plugin_forwarding_params_t* params)
{
    Operator sbc_encoder = ChainGetOperatorByRole(tws_output_chain, second_encoder_role);

    if(sbc_encoder)
        configureSbcEncoderParams(params, sbc_encoder);
}

static void connectPcmInputs(const audio_plugin_forwarding_params_t* params)
{
    Sink pcm_left_sink = ChainGetInput(tws_output_chain, left_decoded_audio_input);
    Sink pcm_right_sink = ChainGetInput(tws_output_chain, right_decoded_audio_input);

    PanicNull(StreamConnect(params->forwarding.pcm.left_source, pcm_left_sink));
    if(params->forwarding.pcm.right_source)
        PanicNull(StreamConnect(params->forwarding.pcm.right_source, pcm_right_sink));
}

void AudioOutputTwsChainCreate(const audio_plugin_forwarding_params_t* params)
{
    const chain_config_t *chain_configuration = getChainConfig(params->source_codec, params->transcode.codec);

    tws_output_chain = PanicNull(ChainCreate(chain_configuration));

    configureSplitterIfPresent();
    configureTtpPassthroughIfPresent(params);
    configureSbcEncoderIfPresent(params);
    configureSecondSbcEncoderIfPresent(params);

    ChainConnect(tws_output_chain);
    connectPcmInputs(params);
}

void AudioOutputTwsChainStart(void)
{
    if(tws_output_chain)
        ChainStart(tws_output_chain);
}

void AudioOutputTwsChainStop(void)
{
    if(tws_output_chain)
        ChainStop(tws_output_chain);
}

void AudioOutputTwsChainDestroy(void)
{
    if(tws_output_chain)
    {
        ChainStop(tws_output_chain);
        ChainDestroy(tws_output_chain);
        tws_output_chain = NULL;
    }
}

Source AudioOutputTwsChainGetLeftSource(void)
{
    return ChainGetOutput(tws_output_chain, left_decoded_audio_output);
}

Source AudioOutputTwsChainGetRightSource(void)
{
    return ChainGetOutput(tws_output_chain, right_decoded_audio_ouput);
}

Source AudioOutputTwsChainGetForwardingSource(void)
{
    return ChainGetOutput(tws_output_chain, encoded_audio_output);
}
