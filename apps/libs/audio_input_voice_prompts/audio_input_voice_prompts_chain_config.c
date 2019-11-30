/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_voice_prompts_chain_config.c

DESCRIPTION
    Implementation of function to return the appropriate chain configuration data.
*/

#include <operators.h>
#include <panic.h>
#include "audio_input_voice_prompts_chain_config.h"

static const operator_config_t tone_op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_tone, tone_role)
};

static const operator_path_node_t tone_output_path[] =
{
    {tone_role, 0, 0}
};

static const operator_path_t tone_paths[] =
{
    {left_output_path_role, path_with_output, ARRAY_DIM((tone_output_path)), tone_output_path}
};

static const chain_config_t tone_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_voice_prompts_tone, audio_ucid_input_tone, tone_op_config, tone_paths);

static const operator_config_t decoder_op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_sbc_decoder, decoder_role)
};

static const operator_path_node_t decoder_input_path[] =
{
    {decoder_role, 0, 0}
};

static const operator_path_node_t decoder_left_output_path[] =
{
    {decoder_role, 0, 0}
};

static const operator_path_node_t decoder_right_output_path[] =
{
    {decoder_role, 0, 1}
};

static const operator_path_t decoder_paths[] =
{
    {input_path_role, path_with_input, ARRAY_DIM((decoder_input_path)), decoder_input_path},
    {left_output_path_role, path_with_output, ARRAY_DIM((decoder_left_output_path)), decoder_left_output_path},
    {right_output_path_role, path_with_output, ARRAY_DIM((decoder_right_output_path)), decoder_right_output_path}
};

static const chain_config_t decoder_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_voice_prompts_decoder, audio_ucid_input_voice_prompts_decoder, decoder_op_config, decoder_paths);

static const operator_config_t dummy_op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, dummy_role)
};

static const operator_path_node_t dummy_input_path[] =
{
    {dummy_role, 0, 0}
};

static const operator_path_node_t dummy_output_path[] =
{
    {dummy_role, 0, 0}
};

static const operator_path_t dummy_paths[] =
{
    {input_path_role, path_with_input, ARRAY_DIM((dummy_input_path)), dummy_input_path},
    {left_output_path_role, path_with_output, ARRAY_DIM((dummy_output_path)), dummy_output_path}
};

static const chain_config_t dummy_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_voice_prompts_decoder, audio_ucid_input_voice_prompts_pcm, dummy_op_config, dummy_paths);

kymera_chain_handle_t audioInputVoicePromptsCreateToneChain(void)
{
    return PanicNull(ChainCreate(&tone_chain_config));
}

kymera_chain_handle_t audioInputVoicePromptsCreatePromptDecoderChain(void)
{
    return PanicNull(ChainCreate(&decoder_chain_config));
}

kymera_chain_handle_t audioInputVoicePromptsCreatePromptDummyChain(void)
{
    return PanicNull(ChainCreate(&dummy_chain_config));
}
