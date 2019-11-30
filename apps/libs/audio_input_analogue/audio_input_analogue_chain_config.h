/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_analogue_chain_config.h

DESCRIPTION
    ANALOGUE audio input chain configuration.
*/

#ifndef AUDIO_INPUT_ANALOGUE_CHAIN_CONFIG_H_
#define AUDIO_INPUT_ANALOGUE_CHAIN_CONFIG_H_

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
const chain_config_t* AudioInputAnalogueGetChainConfig(void);


#endif /* AUDIO_INPUT_ANALOGUE_CHAIN_CONFIG_H_ */
