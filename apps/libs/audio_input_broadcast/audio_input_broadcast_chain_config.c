/******************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast_chain_connect.c

DESCRIPTION
    Definition of the operator chain for the broadcast audio receiver plugin.
*/

#include <operators.h>
#include <panic.h>
#include "audio_input_broadcast_chain_config.h"

static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, input_buffer_role),
    MAKE_OPERATOR_CONFIG(capability_id_celt_decoder, decoder_role),
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, pre_processing_role)
};

static const operator_path_node_t encoded_audio_path[] =
{
    {input_buffer_role, 0, 0},
    {decoder_role, 0, 0}
};

static const operator_path_node_t left_decoded_audio_path[] =
{
    {decoder_role, 0, 0},
    {pre_processing_role, 0, 0}
};

static const operator_path_node_t right_decoded_audio_path[] =
{
    {decoder_role, 0, 1},
    {pre_processing_role, 1, 1}
};

static const operator_path_t paths[] =
{
    {encoded_audio_input, path_with_input, ARRAY_DIM((encoded_audio_path)), encoded_audio_path},
    {decoded_audio_output_left, path_with_output, ARRAY_DIM((left_decoded_audio_path)), left_decoded_audio_path},
    {decoded_audio_output_right, path_with_output, ARRAY_DIM((right_decoded_audio_path)), right_decoded_audio_path}
};

static const chain_config_t chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_broadcast, audio_ucid_input_broadcast, op_config, paths);

kymera_chain_handle_t audioInputBroadcastCreateChain(void)
{
    return PanicNull(ChainCreate(&chain_config));
}

