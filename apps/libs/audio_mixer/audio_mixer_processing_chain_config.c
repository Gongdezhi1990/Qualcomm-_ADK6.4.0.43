/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_processing_chain_config.c

DESCRIPTION
    Implementation of functions to return the appropriate chain configuration.
*/

#include <operators.h>
#include <panic.h>

#include "audio_mixer.h"
#include "audio_mixer_processing.h"
#include "audio_mixer_processing_chain_config.h"
#include "audio_mixer_processing_common.h"
#include "audio_mixer_channel_roles.h"
#include "audio_mixer_latency.h"
#include "audio_mixer_connection.h"

#define LEFT_TERMINAL   0
#define RIGHT_TERMINAL  1
#define MIXER_TERM_OUT  0

#define UNUSED_TERMINAL 0xff

#ifdef USE_SECOND_CORE
#define MAKE_OPERATOR_CONFIG_FOR_MIXER(id, role) MAKE_OPERATOR_CONFIG_P1(id, role)
#define MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(id, role, setup_items) MAKE_OPERATOR_CONFIG_P1_WITH_SETUP(id, role, setup_items)
#else
#define MAKE_OPERATOR_CONFIG_FOR_MIXER(id, role) MAKE_OPERATOR_CONFIG(id, role)
#define MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(id, role, setup_items) MAKE_OPERATOR_CONFIG_WITH_SETUP(id, role, setup_items)
#endif

/* Buffer configuration for use with concurrent mixer graph */

static const operator_setup_item_t mixer_pre_processing_setup[] =
{
    OPERATORS_SETUP_STANDARD_BUFFER_SIZE_FROM_LATENCY(CONCURRENT_PRE_PROCESSING_BUFFER_MILLISECONDS)
};

/* Common stream paths */
static const operator_path_node_t left_path[] = 
{
    { pre_processing_role, LEFT_TERMINAL, LEFT_TERMINAL },
};

/* Statics for Voice (Mono) Chain */

static const operator_config_t ops_mono_voice[] = 
{
    MAKE_OPERATOR_CONFIG_WITH_SETUP(capability_id_passthrough, pre_processing_role, mixer_pre_processing_setup)
};

static const operator_path_t paths_mono_voice[] = 
{
    {mixer_voice_channel, path_with_in_and_out, ARRAY_DIM((left_path)),  left_path}
};

/* Statics for Voice Prompt (Mono) Chain */

static const operator_config_t ops_mono_prompt[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, pre_processing_role)
};

static const operator_path_t paths_mono_prompt[] = 
{
    {mixer_prompt_left_channel, path_with_in_and_out, ARRAY_DIM((left_path)),  left_path}
};

/* Statics for Voice Prompt (Stereo Downmix) Chain */

static const operator_config_t ops_stereo_prompt[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_mixer, processing_stereo_to_mono_role),
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, pre_processing_role)
};

static const operator_path_node_t left_path_dmix[] = 
{
    { processing_stereo_to_mono_role, LEFT_TERMINAL, MIXER_TERM_OUT },
    { pre_processing_role,            LEFT_TERMINAL, LEFT_TERMINAL },
};

static const operator_path_node_t right_path_dmix[] = 
{
    { processing_stereo_to_mono_role, RIGHT_TERMINAL, UNUSED_TERMINAL },
};

static const operator_path_t paths_stereo_prompt[] = 
{
    {mixer_prompt_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_dmix)),  left_path_dmix},
    {mixer_prompt_right_channel, path_with_input,      ARRAY_DIM((right_path_dmix)), right_path_dmix}
};

/* Statics for Stereo Music Processing Chain */

static const operator_config_t ops_mono_stereo_music[] =
{
    MAKE_OPERATOR_CONFIG_FOR_MIXER_WITH_SETUP(capability_id_passthrough, pre_processing_role, mixer_pre_processing_setup),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_compander, processing_compander_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_peq, peq_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_vse, vse_role),
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_bass_enhance, dbe_role)
};

static const operator_path_node_t left_path_music[] = 
{
    { pre_processing_role,          LEFT_TERMINAL, LEFT_TERMINAL },
    { processing_compander_role,    LEFT_TERMINAL, LEFT_TERMINAL },
    { peq_role,                     LEFT_TERMINAL, LEFT_TERMINAL },
    { vse_role,                     LEFT_TERMINAL, LEFT_TERMINAL },
    { dbe_role,                     LEFT_TERMINAL, LEFT_TERMINAL },
};

static const operator_path_node_t right_path_music[] = 
{
    { pre_processing_role,          RIGHT_TERMINAL, RIGHT_TERMINAL },
    { processing_compander_role,    RIGHT_TERMINAL, RIGHT_TERMINAL },
    { peq_role,                     RIGHT_TERMINAL, RIGHT_TERMINAL },
    { vse_role,                     RIGHT_TERMINAL, RIGHT_TERMINAL },
    { dbe_role,                     RIGHT_TERMINAL, RIGHT_TERMINAL },
};

static const operator_path_t paths_mono_music[] =
{
    {mixer_music_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_music)),  left_path_music}
};

static const operator_path_t paths_stereo_music[] = 
{
    {mixer_music_left_channel,  path_with_in_and_out, ARRAY_DIM((left_path_music)),  left_path_music},
    {mixer_music_right_channel, path_with_in_and_out, ARRAY_DIM((right_path_music)), right_path_music}
};

/* Chain Config Statics */

static const chain_config_t config_voice =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_voice_processing,
                                   audio_ucid_mixer_processing_mono_voice,
                                   ops_mono_voice, 
                                   paths_mono_voice);

static const chain_config_t config_mono_prompt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_prompt_processing,
                                   audio_ucid_mixer_processing_prompt,
                                   ops_mono_prompt, 
                                   paths_mono_prompt);

static const chain_config_t config_stereo_prompt =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_prompt_processing,
                                   audio_ucid_mixer_processing_prompt,
                                   ops_stereo_prompt, 
                                   paths_stereo_prompt);

static const chain_config_t config_mono_music =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_music_processing,
                                   audio_ucid_mixer_processing_stereo_music,
                                   ops_mono_stereo_music,
                                   paths_mono_music);

static const chain_config_t config_stereo_music =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_music_processing,
                                   audio_ucid_mixer_processing_stereo_music,
                                   ops_mono_stereo_music,
                                   paths_stereo_music);

/* Filter out all music processing operators except PEQ */
static const operator_config_t processing_ops_low_power_filter[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_none, pre_processing_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, processing_compander_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, vse_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, dbe_role)
};

static const operator_filters_t processing_low_power_filter = 
{
    ARRAY_DIM((processing_ops_low_power_filter)),  processing_ops_low_power_filter
};

/* Filter out vse operator for mono input */
static const operator_config_t processing_ops_mono_music_input_filter[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_none, vse_role)
};

static const operator_filters_t processing_mono_music_input_filter =
{
    ARRAY_DIM((processing_ops_mono_music_input_filter)),  processing_ops_mono_music_input_filter
};

/******************************************************************************/
const chain_config_t* mixerProcessingGetChainConfig(processing_chain_t chain_type)
{
    switch (chain_type)
    {
        case mono_music_processing:
            return &config_mono_music;

        case stereo_music_processing:
            return &config_stereo_music;

        case mono_prompt_processing:
            return &config_mono_prompt;

        case stereo_prompt_processing:
            return &config_stereo_prompt;
        
        case voice_processing:
            return &config_voice;

        default:
            return NULL;
    }
}

/******************************************************************************/
const operator_filters_t *mixerProcessingGetChainFilter(processing_chain_t chain_type,
                                                        bool attenuate_volume_pre_processing)
{
    if(!attenuate_volume_pre_processing)
        return &processing_low_power_filter;

    if(chain_type == mono_music_processing)
        return &processing_mono_music_input_filter;

    return NULL;
}
