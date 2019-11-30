/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_chain_config.c

DESCRIPTION
    TWS chain config
*/

#include <panic.h>
#include <stream.h>

#include <operators.h>
#include <chain.h>
#include <audio_config.h>

#include "audio_output_tws_chain_config.h"

#define UNUSED_TERMINAL       (0xff)
#define BUFFER_SIZE           (8000)
/* Configuration */

static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_ttp_passthrough, ttp_role),
    MAKE_OPERATOR_CONFIG(capability_id_sbc_encoder, encoder_role),
    MAKE_OPERATOR_CONFIG(capability_id_splitter, splitter_role),
    MAKE_OPERATOR_CONFIG(capability_id_sbc_decoder, decoder_role)
};

static const operator_path_node_t left_decoded_audio_input_path[] =
{
    {ttp_role, 0, 0},
    {encoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t right_decoded_audio_input_path[] =
{
    {ttp_role, 1, 1},
    {encoder_role, 1, UNUSED_TERMINAL}
};

static const operator_path_node_t split_encoded_audio_path[] =
{
    {encoder_role, UNUSED_TERMINAL, 0},
    {splitter_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t encoded_audio_output_path[] =
{
    {splitter_role, UNUSED_TERMINAL, 1}
};

static const operator_path_node_t decode_audio_path[] =
{
    {splitter_role, UNUSED_TERMINAL, 0},
    {decoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t left_decoded_audio_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 0}
};

static const operator_path_node_t right_decoded_audio_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 1}
};

static const operator_path_t paths[] =
{
    {left_decoded_audio_input, path_with_input, ARRAY_DIM((left_decoded_audio_input_path)), left_decoded_audio_input_path},
    {right_decoded_audio_input, path_with_input, ARRAY_DIM((right_decoded_audio_input_path)), right_decoded_audio_input_path},
    {split_encoded_audio, path_with_no_in_or_out, ARRAY_DIM((split_encoded_audio_path)), split_encoded_audio_path},
    {encoded_audio_output, path_with_output, ARRAY_DIM((encoded_audio_output_path)), encoded_audio_output_path},
    {decode_audio, path_with_no_in_or_out, ARRAY_DIM((decode_audio_path)), decode_audio_path},
    {left_decoded_audio_output, path_with_output, ARRAY_DIM((left_decoded_audio_output_path)), left_decoded_audio_output_path},
    {right_decoded_audio_ouput, path_with_output, ARRAY_DIM((right_decoded_audio_output_path)), right_decoded_audio_output_path}
};

static const chain_config_t chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_output_tws, audio_ucid_output_tws, op_config, paths);

/* wired mono chain */
static const operator_setup_item_t passthrough_setup[] =
{
    OPERATORS_SETUP_STANDARD_BUFFER_SIZE(BUFFER_SIZE)
};

static const operator_config_t wired_mono_op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_ttp_passthrough, ttp_role),
    MAKE_OPERATOR_CONFIG(capability_id_sbc_encoder, encoder_role),
    MAKE_OPERATOR_CONFIG(capability_id_sbc_encoder, second_encoder_role),
    MAKE_OPERATOR_CONFIG(capability_id_sbc_decoder, decoder_role),
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_passthrough, passthrough_role, passthrough_setup),
};

static const operator_path_node_t wired_mono_left_input_channel[] =
{
    {ttp_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t wired_mono_right_input_channel[] =
{
    {ttp_role, 1, UNUSED_TERMINAL}
};

/* Left input is mapped with local output */
static const operator_path_node_t wired_mono_pcm_channel[] =
{
    {ttp_role, UNUSED_TERMINAL, 0},
    {encoder_role, 0, 0},
    {decoder_role, 0, 0},
    {passthrough_role, 0, 0}
};

/* Right input is mapped with forwarding output */
static const operator_path_node_t wired_mono_forwarding_channel[] =
{
    {ttp_role, UNUSED_TERMINAL, 1},
    {second_encoder_role, 0, 0}
};

static const operator_path_t paths_wired_mono[] =
{
    {left_decoded_audio_input, path_with_input, ARRAY_DIM((wired_mono_left_input_channel)), wired_mono_left_input_channel},
    {right_decoded_audio_input, path_with_input, ARRAY_DIM((wired_mono_right_input_channel)), wired_mono_right_input_channel},
    {left_decoded_audio_output, path_with_output, ARRAY_DIM((wired_mono_pcm_channel)), wired_mono_pcm_channel},
    {encoded_audio_output, path_with_output, ARRAY_DIM((wired_mono_forwarding_channel)), wired_mono_forwarding_channel},
};

static const chain_config_t wired_mono_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_output_tws, audio_ucid_output_tws, wired_mono_op_config, paths_wired_mono);

/* SBC Mono Chain */

static const operator_config_t op_config_mono_sbc[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_sbc_encoder, encoder_role)
};

static const operator_path_node_t mono_encoder_input_path[] =
{
    {encoder_role, 0, 0}
};

static const operator_path_node_t mono_encoder_output_path[] =
{
    {encoder_role, 0, 0}
};

static const operator_path_t paths_mono_sbc[] =
{
    {left_decoded_audio_input, path_with_input, ARRAY_DIM((mono_encoder_input_path)), mono_encoder_input_path},
    {encoded_audio_output, path_with_output, ARRAY_DIM((mono_encoder_output_path)), mono_encoder_output_path},
};

static const chain_config_t mono_encoder_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_output_tws, audio_ucid_output_tws, op_config_mono_sbc, paths_mono_sbc);

/*
 * return a bool to indicate if the device is configured for mono operation only
 */
static bool isMonoMode(void)
{
    return (AudioConfigGetRenderingMode()==single_channel_rendering);
}

const chain_config_t* getChainConfig(audio_codec_t source_codec, audio_codec_t dest_codec)
{
    if (isMonoMode())
    {
        if(source_codec == audio_codec_pcm && dest_codec == audio_codec_sbc)
            return &wired_mono_chain_config;

        return &mono_encoder_chain_config;
    }
    else
    {
        if(source_codec == audio_codec_pcm && dest_codec == audio_codec_sbc)
            return &chain_config;
    }

    Panic();
    return NULL;
}
