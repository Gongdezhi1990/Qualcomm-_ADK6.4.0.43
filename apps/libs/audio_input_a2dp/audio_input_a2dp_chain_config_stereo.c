/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_stereo.c

DESCRIPTION
    Stereo chain configuration
*/

#include "audio_input_a2dp_chain_config_stereo.h"
#include "audio_input_a2dp_chain_config_common.h"
#include <operators.h>
#include <panic.h>


static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_rtp, rtp_role),
    MAKE_OPERATOR_CONFIG(capability_id_splitter, splitter_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, decoder_role)
};

static const operator_path_node_t input_path[] =
{
    {rtp_role, 0, 0},
    {splitter_role, 0, 0},
    {decoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t forwarding_output_path[] =
{
    {splitter_role, UNUSED_TERMINAL, 1}
};

static const operator_path_node_t left_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 0}
};

static const operator_path_node_t right_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 1}
};

static const operator_path_t paths[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {path_forwarding_output, path_with_output, ARRAY_DIM((forwarding_output_path)), forwarding_output_path},
    {path_left_output, path_with_output, ARRAY_DIM((left_output_path)), left_output_path},
    {path_right_output, path_with_output, ARRAY_DIM((right_output_path)), right_output_path},
};

static const chain_config_t a2dp_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp, op_config, paths);

static const chain_config_t a2dp_hq_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp_hq_codec, op_config, paths);


static capability_id_t getDecoderCapabilityId(audio_codec_t decoder)
{
    switch(decoder)
    {
        case audio_codec_sbc:
            return capability_id_sbc_decoder;
        case audio_codec_aac:
            return capability_id_aac_decoder;
        case audio_codec_aptx:
            return capability_id_aptx_decoder;
        case audio_codec_aptx_ll:
            return capability_id_aptx_ll_decoder;
        case audio_codec_aptx_hd:
            return capability_id_aptx_hd_decoder;
        case audio_codec_aptx_adaptive:
            return capability_id_aptx_ad_decoder;
        default:
            Panic();
            return capability_id_none;
    }
}

static const operator_config_t * getOperatorConfigBasedOnRole(unsigned role)
{
    return audioInputA2dpGetOpConfigWithThisRole(role, op_config, ARRAY_DIM(op_config));
}

static operator_config_t getDecoderFilter(audio_codec_t decoder)
{
    return audioInputA2dpGetOpConfigFilter(getDecoderCapabilityId(decoder), getOperatorConfigBasedOnRole(decoder_role));
}

static operator_config_t getRemoveSplitterFilter(void)
{
    return audioInputA2dpGetOpConfigFilter(capability_id_none, getOperatorConfigBasedOnRole(splitter_role));
}

static const chain_config_t * getChainConfig(audio_codec_t decoder)
{
    switch(decoder)
    {
        case audio_codec_aptx:
        case audio_codec_aptx_ll:
        case audio_codec_aptx_hd:
        case audio_codec_aptx_adaptive:
            return &a2dp_hq_chain_config;
        default:
            return &a2dp_chain_config;
    }
}

kymera_chain_handle_t audioInputA2dpCreateStereoChainWithoutSplitter(audio_codec_t decoder)
{
    operator_config_t filter[] = {getDecoderFilter(decoder), getRemoveSplitterFilter()};
    operator_filters_t filters = {.operator_filters = filter, .num_operator_filters = ARRAY_DIM(filter)};

    return PanicNull(ChainCreateWithFilter(getChainConfig(decoder), &filters));
}

kymera_chain_handle_t audioInputA2dpCreateStereoChain(audio_codec_t decoder)
{
    operator_config_t filter[] = {getDecoderFilter(decoder)};
    operator_filters_t filters = {.operator_filters = filter, .num_operator_filters = ARRAY_DIM(filter)};

    return PanicNull(ChainCreateWithFilter(getChainConfig(decoder), &filters));
}
