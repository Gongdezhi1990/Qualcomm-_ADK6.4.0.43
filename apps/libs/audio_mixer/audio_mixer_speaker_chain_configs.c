/****************************************************************************
Copyright (c) 2016 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_spaker_configs.c

DESCRIPTION
    Implementation of functions to return the appropriate chain configuration.
*/

#include <stddef.h>

#include <panic.h>
#include <operators.h>

#include "audio_mixer.h"
#include "audio_mixer_speaker_chain_configs.h"
#include "audio_mixer_speaker.h"
#include "audio_mixer_channel_roles.h"
#include "audio_mixer_connection.h"

#define VOLUME_INPUT_MAIN1          (0)
#define VOLUME_INPUT_AUX1           (1)
#define VOLUME_INPUT_MAIN2          (2)
#define VOLUME_INPUT_MAIN3          (4)
#define VOLUME_INPUT_MAIN4          (6)

#define VOLUME_OUTPUT1              (0)
#define VOLUME_OUTPUT2              (1)
#define VOLUME_OUTPUT3              (2)
#define VOLUME_OUTPUT4              (3)

#define LEFT_INPUT                  (0)
#define RIGHT_INPUT                 (1)
#define LEFT_OUTPUT                 (0)
#define RIGHT_OUTPUT                (1)

#define PRIMARY_LEFT_INPUT          (0)
#define PRIMARY_RIGHT_INPUT         (1)
#define SECONDARY_LEFT_INPUT        (2)
#define SECONDARY_RIGHT_INPUT       (3)

#define PRIMARY_LEFT_OUTPUT         (0)
#define PRIMARY_RIGHT_OUTPUT        (1)
#define SECONDARY_LEFT_OUTPUT       (2)
#define SECONDARY_RIGHT_OUTPUT      (3)
#define SUBWOOFER_OUTPUT            (SECONDARY_LEFT_OUTPUT)

#define CROSSOVER_LEFT_LOW          (0)
#define CROSSOVER_LEFT_HIGH         (1)
#define CROSSOVER_RIGHT_LOW         (2)
#define CROSSOVER_RIGHT_HIGH        (3)

#define UNUSED_TERMINAL             (0xff)

#define MAKE_SPEAKER_OPS_COMMON() \
    MAKE_OPERATOR_CONFIG(capability_id_volume,      media_volume_role), \
    MAKE_OPERATOR_CONFIG(capability_id_peq,         speaker_peq_role), \
    MAKE_OPERATOR_CONFIG(capability_id_passthrough, post_processing_role)

/* Mono output single channel */
static const operator_config_t speaker_ops_downmix_mono[] =
{
    MAKE_SPEAKER_OPS_COMMON(), 
    MAKE_OPERATOR_CONFIG(capability_id_volume, master_volume_role),
    MAKE_OPERATOR_CONFIG(capability_id_mixer,  stereo_to_mono_role)
};

static const operator_path_node_t left_downmix_primary_left[] =
{
    {media_volume_role,    VOLUME_INPUT_MAIN1, VOLUME_OUTPUT1},
    {speaker_peq_role,     LEFT_INPUT,         LEFT_OUTPUT},
    {stereo_to_mono_role,  LEFT_INPUT,         LEFT_OUTPUT},
    {master_volume_role,   VOLUME_INPUT_MAIN1, VOLUME_OUTPUT1},
    {post_processing_role, PRIMARY_LEFT_INPUT, PRIMARY_LEFT_OUTPUT}
};

static const operator_path_node_t right_downmix_primary_left[] =
{
    {media_volume_role,   VOLUME_INPUT_MAIN2, VOLUME_OUTPUT2},
    {speaker_peq_role,    RIGHT_INPUT,        RIGHT_OUTPUT},
    {stereo_to_mono_role, RIGHT_INPUT,        UNUSED_TERMINAL}
};

static const operator_path_node_t prompt_mix[] = 
{
    { media_volume_role, VOLUME_INPUT_AUX1, UNUSED_TERMINAL}
};

static const operator_path_t paths_downmix_mono[] =
{
    {mixer_left_channel,        path_with_in_and_out, ARRAY_DIM((left_downmix_primary_left)),  left_downmix_primary_left},
    {mixer_right_channel,       path_with_input,      ARRAY_DIM((right_downmix_primary_left)), right_downmix_primary_left},
    {mixer_prompt_left_channel, path_with_input,      ARRAY_DIM((prompt_mix)),              prompt_mix}
};

static const chain_config_t speaker_config_downmix_mono =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_speaker,
                                   audio_ucid_mixer_speaker_mono,
                                   speaker_ops_downmix_mono,
                                   paths_downmix_mono);

/* Mono channel or Stereo channels. For mono channel, only mixer_left_channel path is connected during chain connection */
static const operator_config_t speaker_ops_mono_or_stereo[] =
{
    MAKE_SPEAKER_OPS_COMMON()
};

static const operator_path_node_t left_primary_left[] =
{
    {media_volume_role,    VOLUME_INPUT_MAIN1, VOLUME_OUTPUT1},
    {speaker_peq_role,     LEFT_INPUT,         LEFT_OUTPUT},
    {post_processing_role, PRIMARY_LEFT_INPUT, PRIMARY_LEFT_OUTPUT}
};

static const operator_path_node_t right_primary_right[] =
{
    {media_volume_role,    VOLUME_INPUT_MAIN2,  VOLUME_OUTPUT2},
    {speaker_peq_role,     RIGHT_INPUT,         RIGHT_OUTPUT},
    {post_processing_role, PRIMARY_RIGHT_INPUT, PRIMARY_RIGHT_OUTPUT}
};

static const operator_path_t paths_mono[] = 
{
    {mixer_left_channel,        path_with_in_and_out, ARRAY_DIM((left_primary_left)),   left_primary_left},
    {mixer_prompt_left_channel, path_with_input,      ARRAY_DIM((prompt_mix)),          prompt_mix}
};

static const operator_path_t paths_stereo[] = 
{
    {mixer_left_channel,        path_with_in_and_out, ARRAY_DIM((left_primary_left)),   left_primary_left},
    {mixer_right_channel,       path_with_in_and_out, ARRAY_DIM((right_primary_right)), right_primary_right},
    {mixer_prompt_left_channel, path_with_input,      ARRAY_DIM((prompt_mix)),          prompt_mix}
};

static const chain_config_t speaker_config_mono =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_speaker,
                                   audio_ucid_mixer_speaker_stereo,
                                   speaker_ops_mono_or_stereo,
                                   paths_mono);

static const chain_config_t speaker_config_stereo =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_speaker,
                                   audio_ucid_mixer_speaker_stereo,
                                   speaker_ops_mono_or_stereo,
                                   paths_stereo);

/* Stereo output two channels + bass channel */
static const operator_config_t speaker_ops_stereo_bass[] =
{
    MAKE_SPEAKER_OPS_COMMON(), 
    MAKE_OPERATOR_CONFIG(capability_id_volume,          master_volume_role),
    MAKE_OPERATOR_CONFIG(capability_id_crossover_2band, crossover_role),
    MAKE_OPERATOR_CONFIG(capability_id_mixer,           stereo_to_mono_role),
    MAKE_OPERATOR_CONFIG(capability_id_compander,       compander_role)
};

static const operator_path_node_t xover_high_freq_pri_left[] =
{
    {media_volume_role,    VOLUME_INPUT_MAIN1, VOLUME_OUTPUT1},
    {speaker_peq_role,     LEFT_INPUT,         LEFT_OUTPUT},
    {crossover_role,       LEFT_INPUT,         CROSSOVER_LEFT_HIGH},
    {master_volume_role,   VOLUME_INPUT_MAIN1, VOLUME_OUTPUT1},
    {post_processing_role, PRIMARY_LEFT_INPUT, PRIMARY_LEFT_OUTPUT}
};

static const operator_path_node_t xover_high_freq_pri_right[] =
{
    {media_volume_role,    VOLUME_INPUT_MAIN2,  VOLUME_OUTPUT2},
    {speaker_peq_role,     RIGHT_INPUT,         RIGHT_OUTPUT},
    {crossover_role,       RIGHT_INPUT,         CROSSOVER_RIGHT_HIGH},
    {master_volume_role,   VOLUME_INPUT_MAIN2,  VOLUME_OUTPUT2},
    {post_processing_role, PRIMARY_RIGHT_INPUT, PRIMARY_RIGHT_OUTPUT}
};

static const operator_path_node_t xover_low_freq_left_downmix_sec_left[] =
{
    {crossover_role,        UNUSED_TERMINAL,        CROSSOVER_LEFT_LOW},
    {stereo_to_mono_role,   LEFT_INPUT,             LEFT_OUTPUT},
    {master_volume_role,    VOLUME_INPUT_MAIN3,     VOLUME_OUTPUT3},
    {compander_role,        LEFT_INPUT,             LEFT_OUTPUT},
    {post_processing_role,  SECONDARY_LEFT_INPUT,   SUBWOOFER_OUTPUT}
};

static const operator_path_node_t xover_low_freq_right_downmix_sec_left[] =
{
    {crossover_role,       UNUSED_TERMINAL, CROSSOVER_RIGHT_LOW},
    {stereo_to_mono_role,  RIGHT_INPUT,     UNUSED_TERMINAL}
};

static const operator_path_t paths_stereo_bass[] = 
{
    {mixer_left_channel,              path_with_in_and_out,   ARRAY_DIM((xover_high_freq_pri_left)),               xover_high_freq_pri_left},
    {mixer_right_channel,             path_with_in_and_out,   ARRAY_DIM((xover_high_freq_pri_right)),              xover_high_freq_pri_right},
    {mixer_wired_sub_channel,         path_with_output,       ARRAY_DIM((xover_low_freq_left_downmix_sec_left)),   xover_low_freq_left_downmix_sec_left},
    {mixer_secondary_right_channel,   path_with_no_in_or_out, ARRAY_DIM((xover_low_freq_right_downmix_sec_left)),  xover_low_freq_right_downmix_sec_left},
    {mixer_prompt_left_channel,       path_with_input,        ARRAY_DIM((prompt_mix)),                             prompt_mix}
};

static const chain_config_t speaker_config_stereo_bass =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_speaker,
                                   audio_ucid_mixer_speaker_stereo_bass,
                                   speaker_ops_stereo_bass, 
                                   paths_stereo_bass);

/* Stereo output two channels with separate LF and HF outputs */
static const operator_config_t speaker_ops_stereo_xover[] =
{
    MAKE_SPEAKER_OPS_COMMON(), 
    MAKE_OPERATOR_CONFIG(capability_id_volume,          master_volume_role),
    MAKE_OPERATOR_CONFIG(capability_id_crossover_2band, crossover_role),
    MAKE_OPERATOR_CONFIG(capability_id_compander,       compander_role)
};

static const operator_path_node_t xover_low_freq_pri_left[] =
{
    {media_volume_role,     VOLUME_INPUT_MAIN1,     VOLUME_OUTPUT1},
    {speaker_peq_role,      LEFT_INPUT,             LEFT_OUTPUT},
    {crossover_role,        LEFT_INPUT,             CROSSOVER_LEFT_LOW},
    {master_volume_role,    VOLUME_INPUT_MAIN3,     VOLUME_OUTPUT3},
    {compander_role,        LEFT_INPUT,             LEFT_OUTPUT},
    {post_processing_role,  PRIMARY_LEFT_INPUT,     PRIMARY_LEFT_OUTPUT}
};

static const operator_path_node_t xover_low_freq_pri_right[] =
{
    {media_volume_role,     VOLUME_INPUT_MAIN2,     VOLUME_OUTPUT2},
    {speaker_peq_role,      RIGHT_INPUT,            RIGHT_OUTPUT},
    {crossover_role,        RIGHT_INPUT,            CROSSOVER_RIGHT_LOW},
    {master_volume_role,    VOLUME_INPUT_MAIN4,     VOLUME_OUTPUT4},
    {compander_role,        RIGHT_INPUT,            RIGHT_OUTPUT},
    {post_processing_role,  PRIMARY_RIGHT_INPUT,    PRIMARY_RIGHT_OUTPUT}
};

static const operator_path_node_t xover_high_freq_sec_left[] =
{
    {crossover_role,       UNUSED_TERMINAL,         CROSSOVER_LEFT_HIGH},
    {master_volume_role,   VOLUME_INPUT_MAIN1,      VOLUME_OUTPUT1},
    {post_processing_role, SECONDARY_LEFT_INPUT,    SECONDARY_LEFT_OUTPUT}
};

static const operator_path_node_t xover_high_freq_sec_right[] =
{
    {crossover_role,       UNUSED_TERMINAL,         CROSSOVER_RIGHT_HIGH},
    {master_volume_role,   VOLUME_INPUT_MAIN2,      VOLUME_OUTPUT2},
    {post_processing_role, SECONDARY_RIGHT_INPUT,   SECONDARY_RIGHT_OUTPUT}
};

static const operator_path_t paths_stereo_xover[] = 
{
    {mixer_left_channel,              path_with_in_and_out,   ARRAY_DIM((xover_low_freq_pri_left)),     xover_low_freq_pri_left},
    {mixer_right_channel,             path_with_in_and_out,   ARRAY_DIM((xover_low_freq_pri_right)),    xover_low_freq_pri_right},
    {mixer_secondary_left_channel,    path_with_output,       ARRAY_DIM((xover_high_freq_sec_left)),    xover_high_freq_sec_left},
    {mixer_secondary_right_channel,   path_with_output,       ARRAY_DIM((xover_high_freq_sec_right)),   xover_high_freq_sec_right},
    {mixer_prompt_left_channel,       path_with_input,        ARRAY_DIM((prompt_mix)),                  prompt_mix}
};

static const chain_config_t speaker_config_stereo_xover =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_speaker,
                                   audio_ucid_mixer_speaker_stereo_xover,
                                   speaker_ops_stereo_xover, 
                                   paths_stereo_xover);

/* Speaker chain voice only */
static const operator_config_t speaker_ops_low_power[] =
{
    MAKE_OPERATOR_CONFIG(capability_id_volume, media_volume_role)
};

static const operator_path_node_t left_low_power[] =
{
    {media_volume_role,    VOLUME_INPUT_MAIN1, VOLUME_OUTPUT1}
};

static const operator_path_t paths_voice_only[] = 
{
    {mixer_left_channel,              path_with_in_and_out,   ARRAY_DIM((left_low_power)),  left_low_power},
    {mixer_prompt_left_channel,       path_with_input,        ARRAY_DIM((prompt_mix)),      prompt_mix}
};

static const chain_config_t speaker_config_voice_only =
    MAKE_CHAIN_CONFIG_WITH_PATHS(chain_id_audio_mixer_speaker,
                                   audio_ucid_mixer_speaker_voice_only,
                                   speaker_ops_low_power, 
                                   paths_voice_only);

/* Stereo output two channels */
static const operator_config_t speaker_ops_low_power_filter[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_none, speaker_peq_role),
    MAKE_OPERATOR_CONFIG(capability_id_none, post_processing_role)
};

static const operator_filters_t speaker_low_power_filter = 
{
    ARRAY_DIM((speaker_ops_low_power_filter)),  speaker_ops_low_power_filter
};

/* Broadcaster role requires running the mixer chain at a higher than default priority. */
static const operator_config_t speaker_ops_ba_tx_filter[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_volume,             master_volume_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_crossover_2band,    crossover_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_mixer,              stereo_to_mono_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_compander,          compander_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_volume,             media_volume_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_peq,                speaker_peq_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_passthrough,        post_processing_role)
};

static const operator_filters_t speaker_ba_tx_filter =
{
    ARRAY_DIM((speaker_ops_ba_tx_filter)),  speaker_ops_ba_tx_filter
};

/* Combination of low power and broadcast filters */
static const operator_config_t speaker_ops_ba_tx_low_power_filter[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_volume,             master_volume_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_crossover_2band,    crossover_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_mixer,              stereo_to_mono_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_compander,          compander_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_volume,             media_volume_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_none,               speaker_peq_role),
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_none,               post_processing_role)
};

static const operator_filters_t speaker_ba_tx_low_power_filter =
{
    ARRAY_DIM((speaker_ops_ba_tx_low_power_filter)),  speaker_ops_ba_tx_low_power_filter
};

/******************************************************************************/
const chain_config_t *mixerSpeakerGetChainConfig(audio_mixer_speaker_configuration_t chain_type, bool mono_path_enabled)
{
    switch(chain_type)
    {
        case audio_mixer_speaker_mono:
            if(mono_path_enabled)
                return &speaker_config_mono;
            else
                return &speaker_config_downmix_mono;
            
        case audio_mixer_speaker_stereo:
            return &speaker_config_stereo;
            
        case audio_mixer_speaker_stereo_bass:
            return &speaker_config_stereo_bass;
            
        case audio_mixer_speaker_stereo_xover:
            return &speaker_config_stereo_xover;
            
        case audio_mixer_speaker_voice_only:
            return &speaker_config_voice_only;
        
        default:
            Panic();
            return NULL;
    }
}

const operator_filters_t *mixerSpeakerGetChainFilter(connection_type_t connection_type)
{
    switch(connection_type)
    {
        case CONNECTION_TYPE_VOICE:
            /* Voice chain is independent and never needs filtering */
            return NULL;
        
        case CONNECTION_TYPE_TONES:
            /* Stand alone tones never need passthrough. If tones are being
               mixed the speaker chain will already have been created */
            return &speaker_low_power_filter;

        case CONNECTION_TYPE_MUSIC_BA_TX:
            /* Broadcast mode + broadcaster role requires the mixer chain
               to be run at a higher priority than default. */
            if(audioMixerConnectionOfLowPowerMusicEnabled())
                return &speaker_ba_tx_low_power_filter;

            return &speaker_ba_tx_filter;
        
        default:
            /* Apply filters for low power music */
            if(audioMixerConnectionOfLowPowerMusicEnabled())
                return &speaker_low_power_filter;
    }
    
    return NULL;
}
