/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_resamper_chain_config.h

DESCRIPTION
    Implementation of chain construction functions
*/

#ifndef _AUDIO_MIXER_RESAMPLER_CHAIN_CONFIG_H_
#define _AUDIO_MIXER_RESAMPLER_CHAIN_CONFIG_H_

#include <chain.h>

#include "audio_mixer.h"

enum 
{
    resampler_role
};

typedef enum
{
    no_resampler,
    voice_resampler,
    prompt_resampler,
    music_resampler,
    music_ba_tx_resampler
} resampler_chain_t;

/****************************************************************************
DESCRIPTION
    Function to return the required chain type given key input parameters
*/
resampler_chain_t mixerResamplerGetChainType(connection_type_t connection_type);

/****************************************************************************
DESCRIPTION
    Returns the required chain config for a given chain type
*/
const chain_config_t* mixerResamplerGetChainConfig(resampler_chain_t chain_type);

#endif /* _AUDIO_MIXER_RESAMPLER_CHAIN_CONFIG_H_ */

