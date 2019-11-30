/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_mixer_resampler_chain_config.c

DESCRIPTION
    Implementation of functions to return the appropriate chain configuration.
*/

#include <operators.h>

#include "audio_mixer.h"
#include "audio_mixer_resampler.h"
#include "audio_mixer_resampler_chain_config.h"
#include "audio_mixer_channel_roles.h"

#include <panic.h>

#define RESAMPLER_TERM_IN_LEFT  0
#define RESAMPLER_TERM_IN_RIGHT 1

#define RESAMPLER_TERM_OUT_LEFT   0
#define RESAMPLER_TERM_OUT_RIGHT  1

#ifdef USE_SECOND_CORE
#define MAKE_OPERATOR_CONFIG_FOR_MIXER(id, role) MAKE_OPERATOR_CONFIG_P1(id, role)
#else
#define MAKE_OPERATOR_CONFIG_FOR_MIXER(id, role) MAKE_OPERATOR_CONFIG(id, role)
#endif

static const operator_config_t ops_resampler[] = 
{
    MAKE_OPERATOR_CONFIG(capability_id_iir_resampler, resampler_role)
};

static const operator_config_t ops_resampler_music[] = 
{
    MAKE_OPERATOR_CONFIG_FOR_MIXER(capability_id_iir_resampler, resampler_role)
};

static const operator_config_t ops_resampler_music_ba[] =
{
    MAKE_OPERATOR_CONFIG_PRIORITY_LOW(capability_id_iir_resampler, resampler_role)
};

static const operator_endpoint_t inputs_resampler_voice[] = 
{
        {resampler_role, mixer_voice_channel,  RESAMPLER_TERM_IN_LEFT}
};

static const operator_endpoint_t inputs_resampler_prompt[] = 
{
        {resampler_role, mixer_prompt_left_channel,  RESAMPLER_TERM_IN_LEFT},
        {resampler_role, mixer_prompt_right_channel, RESAMPLER_TERM_IN_RIGHT}
};

static const operator_endpoint_t inputs_resampler_music[] = 
{
        {resampler_role, mixer_music_left_channel,  RESAMPLER_TERM_IN_LEFT},
        {resampler_role, mixer_music_right_channel, RESAMPLER_TERM_IN_RIGHT}
};

static const operator_endpoint_t outputs_resampler_voice[] = 
{
    {resampler_role, mixer_voice_channel,  RESAMPLER_TERM_OUT_LEFT}
};

static const operator_endpoint_t outputs_resampler_prompt[] = 
{
    {resampler_role, mixer_prompt_left_channel,  RESAMPLER_TERM_OUT_LEFT},
    {resampler_role, mixer_prompt_right_channel, RESAMPLER_TERM_OUT_RIGHT}
};

static const operator_endpoint_t outputs_resampler_music[] = 
{
    {resampler_role, mixer_music_left_channel,  RESAMPLER_TERM_OUT_LEFT},
    {resampler_role, mixer_music_right_channel, RESAMPLER_TERM_OUT_RIGHT}
};

/* Chain Config Statics */

static const chain_config_t config_voice =
    MAKE_CHAIN_CONFIG_NO_CONNECTIONS(chain_id_audio_mixer_voice_resampler,
                                     audio_ucid_mixer_voice_resampler,
                                     ops_resampler, 
                                     inputs_resampler_voice, 
                                     outputs_resampler_voice);

static const chain_config_t config_prompt =
    MAKE_CHAIN_CONFIG_NO_CONNECTIONS(chain_id_audio_mixer_prompt_resampler,
                                     audio_ucid_mixer_prompt_resampler,
                                     ops_resampler, 
                                     inputs_resampler_prompt, 
                                     outputs_resampler_prompt);

static const chain_config_t config_music =
    MAKE_CHAIN_CONFIG_NO_CONNECTIONS(chain_id_audio_mixer_music_resampler,
                                     audio_ucid_mixer_music_resampler,
                                     ops_resampler_music, 
                                     inputs_resampler_music, 
                                     outputs_resampler_music);

static const chain_config_t config_music_ba_tx =
    MAKE_CHAIN_CONFIG_NO_CONNECTIONS(chain_id_audio_mixer_music_resampler,
                                     audio_ucid_mixer_music_resampler,
                                     ops_resampler_music_ba,
                                     inputs_resampler_music,
                                     outputs_resampler_music);

/******************************************************************************/
resampler_chain_t mixerResamplerGetChainType(connection_type_t connection_type)
{
    switch (connection_type)
    {
        case CONNECTION_TYPE_MUSIC_A2DP:
        case CONNECTION_TYPE_MUSIC:
        case CONNECTION_TYPE_MUSIC_BA_RX:
            return music_resampler;

        case CONNECTION_TYPE_MUSIC_BA_TX:
            return music_ba_tx_resampler;

        case CONNECTION_TYPE_TONES:
            return prompt_resampler;

        case CONNECTION_TYPE_VOICE:
            return voice_resampler;

        default:
            Panic();
            return no_resampler;
    }
}

/******************************************************************************/
const chain_config_t* mixerResamplerGetChainConfig(resampler_chain_t chain_type)
{
    switch (chain_type)
    {
        case music_resampler:
            return &config_music;

        case music_ba_tx_resampler:
            return &config_music_ba_tx;

        case prompt_resampler:
            return &config_prompt;
        
        case voice_resampler:
            return &config_voice;

        default:
            return NULL;
    }
}
