/****************************************************************************
Copyright (c) 2017-2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing_chain_config.h

DESCRIPTION
    Implementation of chain construction functions
*/

#ifndef _AUDIO_MIXER_PROCESSING_CHAIN_CONFIG_H_
#define _AUDIO_MIXER_PROCESSING_CHAIN_CONFIG_H_

#include <chain.h>

#include "audio_mixer.h"
#include "audio_mixer_processing_common.h"


/****************************************************************************
DESCRIPTION
    returns the required chain config for a given chain type
*/
const chain_config_t* mixerProcessingGetChainConfig(processing_chain_t chain_type);

/****************************************************************************
DESCRIPTION
    returns the required filter config for volume attenuation / for mono music */
const operator_filters_t *mixerProcessingGetChainFilter(processing_chain_t chain_type,
                                                        bool attenuate_volume_pre_processing);
#endif /* _AUDIO_MIXER_PROCESSING_CHAIN_CONFIG_H_ */

