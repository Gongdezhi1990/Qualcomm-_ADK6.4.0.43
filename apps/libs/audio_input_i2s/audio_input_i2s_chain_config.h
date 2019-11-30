/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_i2s_chain_config.h

DESCRIPTION
    I2S audio input chain configuration.
*/

#ifndef AUDIO_INPUT_I2S_CHAIN_CONFIG_H_
#define AUDIO_INPUT_I2S_CHAIN_CONFIG_H_

#include <chain.h>

enum
{
    ttp_pass_role,
	switched_passthrough_role
};

typedef enum
{
    path_pcm_input_left,
    path_pcm_input_right,
    path_pcm_output_left,
    path_pcm_output_right,
    path_pcm_forwarding
} path_channel_t;

/****************************************************************************
DESCRIPTION
    Return the chain configuration.
*/
const chain_config_t* AudioInputI2sGetChainConfig(void);


#endif /* AUDIO_INPUT_I2S_CHAIN_CONFIG_H_ */
