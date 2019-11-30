/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_broadcast_chain_config.c

DESCRIPTION
    Kymera chain configuration for audio_output_broadcast.
*/

#include <operators.h>
#include <panic.h>
#include "audio_output_broadcast_chain_config.h"

#define UNUSED_TERMINAL (0xff)

static const operator_config_t op_config[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, pre_processing_role),
    MAKE_OPERATOR_CONFIG(capability_id_compander, compander_role),
    MAKE_OPERATOR_CONFIG(capability_id_peq, peq_role),
    MAKE_OPERATOR_CONFIG(capability_id_vse, vse_role),
    MAKE_OPERATOR_CONFIG(capability_id_bass_enhance, dbe_role),
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, cancel_headroom_role),
    MAKE_OPERATOR_CONFIG(capability_id_celt_encoder, encoder_role),
    MAKE_OPERATOR_CONFIG(capability_id_splitter, splitter_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_celt_decoder, decoder_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_MEDIUM(capability_id_passthrough, broadcast_buffer_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_passthrough, local_buffer_role)
};

static const operator_path_node_t left_decoded_audio_input_path[] =
{
    {pre_processing_role, 0, 0},
    {compander_role, 0, 0},
    {peq_role, 0, 0},
    {vse_role, 0, 0},
    {dbe_role, 0, 0},
    {cancel_headroom_role, 0, 0},
    {encoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t right_decoded_audio_input_path[] =
{
    {pre_processing_role, 1, 1},
    {compander_role, 1, 1},
    {peq_role, 1, 1},
    {vse_role, 1, 1},
    {dbe_role, 1, 1},
    {cancel_headroom_role, 1, 1},
    {encoder_role, 1, UNUSED_TERMINAL}
};

static const operator_path_node_t split_encoded_audio_path[] =
{
    {encoder_role, UNUSED_TERMINAL, 0},
    {splitter_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t broadcast_encoded_audio_output_path[] =
{
    {splitter_role, UNUSED_TERMINAL, 1},
    {broadcast_buffer_role, 0, 0}
};

static const operator_path_node_t decode_encoded_audio_path[] =
{
    {splitter_role, UNUSED_TERMINAL, 0},
    {decoder_role, 0, UNUSED_TERMINAL}
};

static const operator_path_node_t left_decoded_audio_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 0},
    {local_buffer_role, 0, 0}
};

static const operator_path_node_t right_decoded_audio_output_path[] =
{
    {decoder_role, UNUSED_TERMINAL, 1},
    {local_buffer_role, 1, 1}
};

static const operator_path_t paths[] =
{
    {left_decoded_audio_input, path_with_input, ARRAY_DIM((left_decoded_audio_input_path)), left_decoded_audio_input_path},
    {right_decoded_audio_input, path_with_input, ARRAY_DIM((right_decoded_audio_input_path)), right_decoded_audio_input_path},
    {split_encoded_audio, path_with_no_in_or_out, ARRAY_DIM((split_encoded_audio_path)), split_encoded_audio_path},
    {broadcast_encoded_audio_output, path_with_output, ARRAY_DIM((broadcast_encoded_audio_output_path)), broadcast_encoded_audio_output_path},
    {decode_encoded_audio, path_with_no_in_or_out, ARRAY_DIM((decode_encoded_audio_path)), decode_encoded_audio_path},
    {left_decoded_audio_output, path_with_output, ARRAY_DIM((left_decoded_audio_output_path)), left_decoded_audio_output_path},
    {right_decoded_audio_ouput, path_with_output, ARRAY_DIM((right_decoded_audio_output_path)), right_decoded_audio_output_path}
};

static const chain_config_t chain_config =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_input_a2dp, audio_ucid_output_broadcast, op_config, paths);

kymera_chain_handle_t audioOutputBroadcastCreateChain(void)
{
    return PanicNull(ChainCreate(&chain_config));
}
