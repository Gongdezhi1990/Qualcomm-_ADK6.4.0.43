/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_chain_config_mono_aac.c

DESCRIPTION
    Get the appropriate AAC mono chain
*/

#include "audio_input_a2dp_chain_config_mono_aac.h"
#include "audio_input_a2dp_chain_config_common.h"
#include <operators.h>
#include <panic.h>


static const operator_setup_item_t switched_passthrough_pcm_setup[] =
{
    OPERATORS_SETUP_SWITCHED_PASSTHROUGH_FORMAT(spc_op_format_pcm)
};

static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_rtp, rtp_role),
    MAKE_OPERATOR_CONFIG(capability_id_splitter, splitter_role),
    MAKE_OPERATOR_CONFIG(capability_id_aac_decoder, decoder_role),
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_switched_passthrough_consumer, switched_passthrough_role, switched_passthrough_pcm_setup)
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

/* Left chain config */

static const operator_path_node_t left_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 0}
};

static const operator_path_node_t consuming_right_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 1},
    {switched_passthrough_role, 0, UNUSED_TERMINAL}
};

static const operator_path_t paths_mono_left[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {path_forwarding_output, path_with_output, ARRAY_DIM((forwarding_output_path)), forwarding_output_path},
    {MONO_OUTPUT, path_with_output, ARRAY_DIM((left_output_path)), left_output_path},
    {MONO_CONSUMED_OUTPUT, path_with_no_in_or_out, ARRAY_DIM((consuming_right_output_path)), consuming_right_output_path}
};

static const chain_config_t left_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp, op_config, paths_mono_left);

/* Right chain config */

static const operator_path_node_t right_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 1}
};

static const operator_path_node_t consuming_left_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 0},
    {switched_passthrough_role, 0, UNUSED_TERMINAL}
};

static const operator_path_t paths_mono_right[] =
{
    {path_input, path_with_input, ARRAY_DIM((input_path)), input_path},
    {path_forwarding_output, path_with_output, ARRAY_DIM((forwarding_output_path)), forwarding_output_path},
    {MONO_OUTPUT, path_with_output, ARRAY_DIM((right_output_path)), right_output_path},
    {MONO_CONSUMED_OUTPUT, path_with_no_in_or_out, ARRAY_DIM((consuming_left_output_path)), consuming_left_output_path}
};

static const chain_config_t right_chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_input_a2dp, op_config, paths_mono_right);


kymera_chain_handle_t audioInputA2dpCreateMonoAacChain(void)
{
    const chain_config_t *chain_config;

    if (audioInputA2dpIsSinkConfiguredToUseLeftChannel())
        chain_config = &left_chain_config;
    else
        chain_config = &right_chain_config;

    return PanicNull(ChainCreate(chain_config));
}
