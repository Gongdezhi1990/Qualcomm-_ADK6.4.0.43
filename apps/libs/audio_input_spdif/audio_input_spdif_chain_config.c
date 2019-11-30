/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_spdif_chain_config.c

DESCRIPTION
    Implementation of functions to return the appropriate chain configuration
*/

#include <operators.h>
#include "audio_input_spdif_chain_config.h"

static const operator_config_t spdif_ops[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_spdif_decoder, spdif_decoder_role),
    MAKE_OPERATOR_CONFIG(capability_id_ttp_passthrough, spdif_ttp_passthrough_role)
};

static const operator_endpoint_t spdif_inputs[] =
{
#ifdef INTERLEAVED_ENABLED
    {spdif_decoder_role, spdif_input_interleaved, 0}
#else
    {spdif_decoder_role, spdif_input_left, 0},
    {spdif_decoder_role, spdif_input_right, 1}
#endif
};

static const operator_endpoint_t spdif_outputs[] =
{
    {spdif_ttp_passthrough_role, spdif_output_left, 0},
    {spdif_ttp_passthrough_role, spdif_output_right, 1}
};

static const operator_connection_t spdif_connections[] =
{
    {spdif_decoder_role, 1, spdif_ttp_passthrough_role, 0, 2}
};

static const chain_config_t spdif_chain_config =
    MAKE_CHAIN_CONFIG(chain_id_audio_input_spdif,
        audio_ucid_input_digital, spdif_ops, spdif_inputs, spdif_outputs, spdif_connections);

const chain_config_t* AudioInputSpdifGetChainConfig(void)
{
    return (const chain_config_t* )&spdif_chain_config;
}
