/****************************************************************************
Copyright (c) 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing_common.h

DESCRIPTION
    Common definitions used in audio_mixer_processing chain configs
*/

#ifndef _AUDIO_MIXER_PROCESSING_COMMON_H_
#define _AUDIO_MIXER_PROCESSING_COMMON_H_

#include <chain.h>

#include "audio_mixer.h"

enum
{
    processing_compander_role,
    peq_role,
    vse_role,
    dbe_role,
    processing_stereo_to_mono_role,
    pre_processing_role,
    crossover_2band_role,
    compander_2band_low_freq_role,
    compander_2band_high_freq_role,
    mixer_2band_role,
    first_crossover_3band_role,
    peq_3band_role,
    second_crossover_3band_role,
    compander_3band_low_freq_role,
    compander_3band_mid_freq_role,
    compander_3band_high_freq_role,
    mixer_3band_role
};

typedef enum
{
    no_processing,
    voice_processing,
    mono_prompt_processing,
    stereo_prompt_processing,
    mono_music_processing,
    stereo_music_processing,
    mono_music_2band_mbc_processing,
    stereo_music_2band_mbc_processing,
    mono_music_3band_mbc_processing,
    stereo_music_3band_mbc_processing
} processing_chain_t;

#endif /* _AUDIO_MIXER_PROCESSING_COMMON_H_ */

