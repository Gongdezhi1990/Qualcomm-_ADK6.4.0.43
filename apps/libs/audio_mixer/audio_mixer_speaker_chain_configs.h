/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_speaker_chain_configs.h

DESCRIPTION
       Declarations for Mixer Speaker Chain configurations
*/

#ifndef _AUDIO_MIXER_SPEAKER_CHAIN_CONFIGS_H_
#define _AUDIO_MIXER_SPEAKER_CHAIN_CONFIGS_H_

#include <chain.h>

#include "audio_mixer.h"
#include "audio_mixer_speaker.h"

typedef enum _audio_mixer_speaker_roles
{
    media_volume_role,
    speaker_peq_role,
    stereo_to_mono_role,
    crossover_role,
    master_volume_role,
    compander_role,
    post_processing_role
} audio_mixer_speaker_roles_t;

/****************************************************************************
DESCRIPTION
    Return the speaker chain configuration for the specified speaker set-up
*/
const chain_config_t *mixerSpeakerGetChainConfig(audio_mixer_speaker_configuration_t chain_type, bool mono_path_enabled);

/****************************************************************************
DESCRIPTION
    Return the speaker chain filter for the specified speaker set-up
*/
const operator_filters_t *mixerSpeakerGetChainFilter(connection_type_t connection_type);

#endif /* _AUDIO_MIXER_SPEAKER_CHAIN_CONFIGS_H_ */
