/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_mono_aptx.c

DESCRIPTION
    Get the appropriate aptx mono chain
*/

#include "audio_input_a2dp_chain_config_mono_aptx.h"
#include "audio_input_a2dp_chain_config_common.h"
#include <operators.h>
#include <panic.h>


static const operator_setup_item_t passthrough_setup[] =
{
    OPERATORS_SETUP_STANDARD_BUFFER_SIZE(PASSTHROUGH_BUFFER_SIZE_FOR_MONO)
};

static const operator_setup_item_t switched_passthrough_setup[] =
{
    OPERATORS_SETUP_SWITCHED_PASSTHROUGH_FORMAT(spc_op_format_encoded)
};

static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_rtp, rtp_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, demux_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, decoder_role),
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_switched_passthrough_consumer, switched_passthrough_role, switched_passthrough_setup),
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_passthrough, passthrough_role, passthrough_setup)
};

static const operator_path_node_t input_path[] =
{
    {rtp_role, 0, 0},
    {demux_role, 0, UNUSED_TERMINAL}
};

/* Left chain config */

static const operator_path_node_t left_output_path[] =
{
    {demux_role, UNUSED_TERMINAL, 0},
    {decoder_role, 0, 0},
    {passthrough_role, 0, 0}
};

static const operator_path_node_t forwarding_right_output_path[] =
{
    {demux_role, UNUSED_TERMINAL, 1},
    {switched_passthrough_role, 0, 0}
};

static const operator_path_t paths_mono_left[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {MONO_OUTPUT, path_with_output, ARRAY_DIM((left_output_path)), left_output_path},
    {path_forwarding_output, path_with_output, ARRAY_DIM((forwarding_right_output_path)), forwarding_right_output_path}
};

static const chain_config_t left_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp_hq_codec, op_config, paths_mono_left);

/* Right chain config */

static const operator_path_node_t right_output_path[] =
{
    {demux_role, UNUSED_TERMINAL, 1},
    {decoder_role, 0, 0},
    {passthrough_role, 0, 0}
};

static const operator_path_node_t forwarding_left_output_path[] =
{
    {demux_role, UNUSED_TERMINAL, 0},
    {switched_passthrough_role, 0, 0}
};

static const operator_path_t paths_mono_right[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {MONO_OUTPUT, path_with_output, ARRAY_DIM((right_output_path)), right_output_path},
    {path_forwarding_output, path_with_output, ARRAY_DIM((forwarding_left_output_path)), forwarding_left_output_path}
};

static const chain_config_t right_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp_hq_codec, op_config, paths_mono_right);


static capability_id_t getDecoderCapabilityId(audio_codec_t decoder)
{
    switch(decoder)
    {
        case audio_codec_aptx:
            return capability_id_aptx_mono_decoder_no_autosync;
        case audio_codec_aptx_adaptive:
            return capability_id_aptx_ad_decoder;
        default:
            Panic();
            return capability_id_none;
    }
}

static capability_id_t getDemuxCapabilityId(audio_codec_t decoder)
{
    switch(decoder)
    {
        case audio_codec_aptx:
            return capability_id_aptx_demux;
        case audio_codec_aptx_adaptive:
            return capability_id_aptx_ad_demux;
        default:
            Panic();
            return capability_id_none;
    }
}

static const operator_config_t * getOperatorConfigBasedOnRole(unsigned role)
{
    return audioInputA2dpGetOpConfigWithThisRole(role, op_config, ARRAY_DIM(op_config));
}

static operator_config_t getDemuxFilter(audio_codec_t decoder)
{
    return audioInputA2dpGetOpConfigFilter(getDemuxCapabilityId(decoder), getOperatorConfigBasedOnRole(demux_role));
}

static operator_config_t getDecoderFilter(audio_codec_t decoder)
{
    return audioInputA2dpGetOpConfigFilter(getDecoderCapabilityId(decoder), getOperatorConfigBasedOnRole(decoder_role));
}

static const chain_config_t * getChainConfig(void)
{
    if (audioInputA2dpIsSinkConfiguredToUseLeftChannel())
        return &left_chain_config;
    else
        return &right_chain_config;
}

kymera_chain_handle_t audioInputA2dpCreateMonoAptxChain(audio_codec_t decoder)
{
    operator_config_t filter[] = {getDecoderFilter(decoder), getDemuxFilter(decoder)};
    operator_filters_t filters = {.operator_filters = filter, .num_operator_filters = ARRAY_DIM(filter)};

    return PanicNull(ChainCreateWithFilter(getChainConfig(), &filters));
}
