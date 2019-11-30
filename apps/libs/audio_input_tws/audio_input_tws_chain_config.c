/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_tws_chain_config.c

DESCRIPTION
    Create appropriate chain for audio_input_tws
*/

#include "audio_input_tws_chain_config.h"
#include "audio_input_tws_chain_config_common.h"
#include "audio_input_tws_chain_config_mono_aac.h"
#include <operators.h>
#include <panic.h>
#include <audio_config.h>


static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, passthrough_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, decoder_role)
};

static const operator_path_node_t input_path[] =
{
    {passthrough_role, 0, 0},
    {decoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t left_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 0}
};

static const operator_path_node_t right_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 1}
};

static const operator_path_t stereo_paths[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {path_left_output, path_with_output, ARRAY_DIM((left_output_path)), left_output_path},
    {path_right_output, path_with_output, ARRAY_DIM((right_output_path)), right_output_path},
};

static const operator_path_t mono_paths[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {MONO_OUTPUT, path_with_output, ARRAY_DIM((left_output_path)), left_output_path}
};

static const chain_config_t stereo_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_tws, audio_ucid_input_a2dp, op_config, stereo_paths);

static const chain_config_t hq_stereo_chain_config_aptx =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_tws, audio_ucid_input_a2dp_hq_codec, op_config, stereo_paths);

static const chain_config_t mono_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_tws, audio_ucid_input_a2dp, op_config, mono_paths);

static const chain_config_t hq_mono_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_tws, audio_ucid_input_a2dp_hq_codec, op_config, mono_paths);


static bool isMonoChannel(void)
{
    return (AudioConfigGetRenderingMode() == single_channel_rendering);
}

static bool isMonoAacDecoder(audio_input_tws_decoder_t decoder)
{
    return ((isMonoChannel()) && (decoder == audio_input_tws_aac));
}

static const chain_config_t* getChainConfig(audio_input_tws_decoder_t decoder)
{
    switch(decoder)
    {
        case audio_input_tws_aptx:
        case audio_input_tws_aptx_adaptive:
            return (isMonoChannel())? &hq_mono_chain_config : &hq_stereo_chain_config_aptx;
        default:
            return (isMonoChannel())? &mono_chain_config : &stereo_chain_config;
    }
}

static capability_id_t getDecoderCapabilityId(audio_input_tws_decoder_t decoder)
{
    switch(decoder)
    {
        case audio_input_tws_sbc:
            return capability_id_sbc_decoder;
        case audio_input_tws_aac:
            return capability_id_aac_decoder;
        case audio_input_tws_aptx:
            return (isMonoChannel())? capability_id_aptx_mono_decoder_no_autosync : capability_id_aptx_decoder;
        case audio_input_tws_aptx_adaptive:
            return capability_id_aptx_ad_decoder;
        default:
            Panic();
            return capability_id_none;
    }
}

static const operator_config_t * getOperatorConfigBasedOnRole(unsigned role)
{
    unsigned i;

    for (i = 0; i < ARRAY_DIM(op_config); ++i)
    {
        if (op_config[i].role == role)
            return &op_config[i];
    }

    Panic();
    return NULL;
}

static operator_config_t getChainConfigFilter(const operator_config_t *base_config, capability_id_t cap_id)
{
    operator_config_t filter;

    /* Copy all fields form base configuration then replace capability ID */
    memcpy(&filter, base_config, sizeof(filter));
    filter.capability_id = cap_id;

    return filter;
}

static operator_config_t getChainConfigFilterForDecoder(audio_input_tws_decoder_t decoder)
{
    return getChainConfigFilter(getOperatorConfigBasedOnRole(decoder_role), getDecoderCapabilityId(decoder));
}

static kymera_chain_handle_t CreateChain(audio_input_tws_decoder_t decoder)
{
    operator_config_t filter[] = {getChainConfigFilterForDecoder(decoder)};
    operator_filters_t filters = {.num_operator_filters = ARRAY_DIM(filter), .operator_filters = filter};

    return PanicNull(ChainCreateWithFilter(getChainConfig(decoder), &filters));
}

kymera_chain_handle_t audioInputTWSCreateChain(audio_input_tws_decoder_t decoder)
{
    if (isMonoAacDecoder(decoder))
        return audioInputTWSCreateMonoAacChain();
    else
        return CreateChain(decoder);
}
