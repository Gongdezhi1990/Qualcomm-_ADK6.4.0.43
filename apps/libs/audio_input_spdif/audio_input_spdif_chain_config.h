/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_spdif_chain_config.h

DESCRIPTION
    Spdif audio input chain configuration.
*/

#ifndef AUDIO_INPUT_SPDIF_CHAIN_CONFIG_H_
#define AUDIO_INPUT_SPDIF_CHAIN_CONFIG_H_


#include <chain.h>


typedef enum
{
    spdif_input_left,
    spdif_input_right,
    spdif_input_interleaved,
    spdif_output_left,
    spdif_output_right
} chain_endpoint_role_t;

typedef enum
{
    spdif_decoder_role,
    spdif_ttp_passthrough_role
}spdif_operator_role_t;


/****************************************************************************
NAME
    AudioInputSpdifGetChainConfig
DESCRIPTION
    Get the chain configuration for the SPDIF input chain.
RETURNS
    SPDIF chain configuration.
*/
const chain_config_t* AudioInputSpdifGetChainConfig(void);


#endif /* AUDIO_INPUT_SPDIF_CHAIN_CONFIG_H_ */