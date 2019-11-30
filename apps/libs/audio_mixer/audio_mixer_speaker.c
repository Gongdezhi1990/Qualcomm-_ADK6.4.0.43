/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_mixer_speaker.c

DESCRIPTION
       Implementation of Mixer Speaker chain management
*/

#include <string.h>

#include <panic.h>

#include <operators.h>
#include <chain.h>
#include <audio_hardware.h>
#include <audio_mixer.h>
#include <audio_config.h>
#include <audio_plugin_ucid.h>


#include "audio_mixer_speaker_chain_configs.h"
#include "audio_mixer_speaker.h"
#include "audio_mixer_gain.h"
#include "audio_mixer_channel_roles.h"
#include "audio_mixer_connection.h"

typedef struct mixer_speaker_data
{
    kymera_chain_handle_t chain;
    audio_mixer_speaker_configuration_t mode;
} mixer_speaker_data_t;

static mixer_speaker_data_t speaker_data =
{
    .chain = NULL,
    .mode = audio_mixer_speaker_invalid
};

static bool mixerSpeakerModeIsPermittedWithVoice(audio_mixer_speaker_configuration_t speaker_mode)
{
    if(speaker_mode == audio_mixer_speaker_stereo_xover)
        return FALSE;

    if(speaker_mode == audio_mixer_speaker_stereo_bass)
        return FALSE;

    return TRUE;
}

/******************************************************************************/
static bool mixerSpeakerModeIsPermitted(audio_mixer_speaker_configuration_t speaker_mode, connection_type_t connection_type)
{
    switch(connection_type)
    {
        case CONNECTION_TYPE_VOICE:
        {
            /* Voice must limit number of speaker channels to ensure echo
               cancellation is effective */
            return mixerSpeakerModeIsPermittedWithVoice(speaker_mode);
        }

        case CONNECTION_TYPE_MUSIC:        
		case CONNECTION_TYPE_MUSIC_A2DP:
        {
            /* If simultaneous voice and wired music are supported we must
               consider voice when setting up wired music */
            if(AudioConfigGetMaximumConcurrentInputs() > 1)
            {
                return mixerSpeakerModeIsPermittedWithVoice(speaker_mode);
            }
            return TRUE;
        }

        default:
            return TRUE;
    }
}

/******************************************************************************/
static audio_mixer_speaker_configuration_t mixerSpeakerGetOutputMode(connection_type_t connection_type)
{
    audio_mixer_speaker_configuration_t speaker_mode = audio_mixer_speaker_stereo;

    audio_hardware_speaker_config_t audio_hardware_speaker_config = AudioHardwareGetSpeakerConfig();

    if(AudioConfigGetMaximumConcurrentInputs() == 1 && connection_type == CONNECTION_TYPE_VOICE)
    {
        speaker_mode = audio_mixer_speaker_voice_only;
    }
    else if (audio_hardware_speaker_config == speaker_stereo_xover)
    {
        if(mixerSpeakerModeIsPermitted(audio_mixer_speaker_stereo_xover, connection_type))
            speaker_mode = audio_mixer_speaker_stereo_xover;
    }
    else if (audio_hardware_speaker_config == speaker_stereo_bass)
    {
        if(mixerSpeakerModeIsPermitted(audio_mixer_speaker_stereo_bass, connection_type))
            speaker_mode = audio_mixer_speaker_stereo_bass;
    }
    else if (audio_hardware_speaker_config == speaker_stereo)
    {
        speaker_mode = audio_mixer_speaker_stereo;
    }
    else if (audio_hardware_speaker_config == speaker_mono)
    {
        speaker_mode = audio_mixer_speaker_mono;
    }
    else
    {
        Panic();
    }

    return speaker_mode;
}

/******************************************************************************/
static ucid_crossover_t mixerSpeakerGetCrossoverUcid(audio_mixer_speaker_configuration_t chain_type)
{
    ucid_crossover_t ucid_crossover = ucid_crossover_speaker_stereo_bass;
    switch(chain_type)
    {
        case audio_mixer_speaker_stereo_bass:
        {
            ucid_crossover = ucid_crossover_speaker_stereo_bass;
            break;
        }
        case audio_mixer_speaker_stereo_xover:
        {
            ucid_crossover = ucid_crossover_speaker_stereo_xover;
            break;
        }
        default:
        {
            Panic();
            break;
        }
    }
    return ucid_crossover;
}

/******************************************************************************/
static Operator getVolumeOperatorForHardwareSettings(void)
{
    Operator volume_op = ChainGetOperatorByRole(speaker_data.chain, master_volume_role);

    if(volume_op == INVALID_OPERATOR)
        volume_op = ChainGetOperatorByRole(speaker_data.chain, media_volume_role);

    return volume_op;
}

/******************************************************************************/
static Operator getVolumeOperatorForToneMixing(void)
{
    Operator volume_op = ChainGetOperatorByRole(speaker_data.chain, media_volume_role);
    
    if(volume_op == INVALID_OPERATOR)
        volume_op = ChainGetOperatorByRole(speaker_data.chain, master_volume_role);
    
    return volume_op;
}

/******************************************************************************/
static void mixerSpeakerSetUnusedVolume(void)
{
    /* In most cases the master_volume_role is only used for post gain and trims
       so we set the main gain to unity and don't touch it again. */
    Operator volume_op = ChainGetOperatorByRole(speaker_data.chain, master_volume_role);

    if(volume_op != INVALID_OPERATOR)
        OperatorsVolumeSetMainGain(volume_op, GAIN_UNITY);
}

/******************************************************************************/
static void mixerSpeakerSetPostGain(void)
{
    int post_gain = AudioOutputGainGetFixedHardwareLevel();
    Operator volume_op = getVolumeOperatorForHardwareSettings();

    if(volume_op != INVALID_OPERATOR)
        OperatorsVolumeSetPostGain(volume_op, post_gain);
}

/******************************************************************************/
static void mixerSpeakerSetTrimsDefault(void)
{
    Operator volume_op = getVolumeOperatorForHardwareSettings();

    if(volume_op != INVALID_OPERATOR)
    {
        volume_trims_t trims;
        audio_output_gain_t gain_info;

        AudioOutputGainGetDigital(audio_output_group_main, 0, 0, &gain_info);

        trims.primary_left = gain_info.trim.main.primary_left;
        trims.primary_right = gain_info.trim.main.primary_right;
        trims.secondary_left = gain_info.trim.main.secondary_left;
        trims.secondary_right = gain_info.trim.main.secondary_right;

        OperatorsVolumeSetTrims(volume_op, &trims);
    }
}

/******************************************************************************/
static void mixerSpeakerSetFixedToneVolume(void)
{
    Operator volume_op = getVolumeOperatorForToneMixing();

    if(AudioConfigGetTonesAreFixedVolume() && volume_op != INVALID_OPERATOR)
        OperatorsVolumeSetAuxGain(volume_op, AudioConfigGetToneVolume());
}

/****************************************************************************
DESCRIPTION
    Configure default post and master gain settings in the volume operators
*/
static void mixerConfigureVolumeDefaults(void)
{
    mixerSpeakerSetTrimsDefault();
    mixerSpeakerSetPostGain();
    mixerSpeakerSetUnusedVolume();
    mixerSpeakerSetFixedToneVolume();

    mixerSpeakerSetVolume();
    mixerSpeakerMuteOutput(AudioConfigGetMasterMuteState());
}

/****************************************************************************
DESCRIPTION
    Apply configuration settings
*/
static void mixerSpeakerConfigureChain(unsigned output_sample_rate)
{
    Operator speaker_peq_op;
    Operator media_volume_op;
    Operator mixer_op;
    Operator compander_op;
    Operator master_volume_op;
    Operator crossover_op;
    Operator passthrough_op;

    speaker_peq_op = ChainGetOperatorByRole(speaker_data.chain, speaker_peq_role);
    if(speaker_peq_op)
    {
        music_processing_mode_t music_processing_mode;

        music_processing_mode = AudioConfigGetSpeakerPeqBypass() ? music_processing_mode_passthrough
                                                                 : music_processing_mode_full_processing;

        OperatorsStandardSetUCID(speaker_peq_op, ucid_peq_speaker);
        OperatorsSetMusicProcessingMode(speaker_peq_op, music_processing_mode);
    }

    media_volume_op = ChainGetOperatorByRole(speaker_data.chain, media_volume_role);
    if(media_volume_op)
    {
        OperatorsStandardSetUCID(media_volume_op, ucid_volume_core_media);
    }

    mixer_op = ChainGetOperatorByRole(speaker_data.chain, stereo_to_mono_role);
    if(mixer_op)
    {
        OperatorsMixerSetChannelsPerStream(mixer_op, 1, 1, 0);
    }

    master_volume_op = ChainGetOperatorByRole(speaker_data.chain, master_volume_role);
    if (master_volume_op)
    {
        OperatorsStandardSetUCID(master_volume_op, ucid_volume_speaker_master);
    }

    compander_op = ChainGetOperatorByRole(speaker_data.chain, compander_role);
    if (compander_op)
    {
        OperatorsStandardSetUCID(compander_op, ucid_compander_speaker);
    }

    crossover_op = ChainGetOperatorByRole(speaker_data.chain, crossover_role);
    if (crossover_op)
    {
        OperatorsStandardSetUCID(crossover_op, mixerSpeakerGetCrossoverUcid(speaker_data.mode));
    }

    passthrough_op = ChainGetOperatorByRole(speaker_data.chain, post_processing_role);
    if (passthrough_op)
    {
        OperatorsStandardSetUCID(passthrough_op, ucid_passthrough_speaker);
    }

    ChainConfigureSampleRate(speaker_data.chain, output_sample_rate, NULL, 0);

    mixerConfigureVolumeDefaults();
}

static void mixerSpeakerChainConnect(kymera_chain_handle_t chain, bool mono_path_enabled)
{
    if(mono_path_enabled)
    {
        ChainConnectWithPath(chain, mixer_left_channel);
        ChainConnectWithPath(chain, mixer_prompt_left_channel);
    }
    else
    {
        ChainConnect(chain);
    }
}
/******************************************************************************/
void mixerSpeakerCreate(connection_type_t connection_type, unsigned output_sample_rate)
{
    const chain_config_t *speaker_chain_config;
    const operator_filters_t* speaker_chain_filter;
    bool mono_path_enabled = (AudioConfigGetRenderingMode()==single_channel_rendering);

    speaker_data.mode = mixerSpeakerGetOutputMode(connection_type);
    speaker_chain_config = mixerSpeakerGetChainConfig(speaker_data.mode, mono_path_enabled);
    speaker_chain_filter = mixerSpeakerGetChainFilter(connection_type);
    speaker_data.chain = PanicNull(ChainCreateWithFilter(speaker_chain_config, speaker_chain_filter));

    mixerSpeakerConfigureChain(output_sample_rate);

    mixerSpeakerChainConnect(speaker_data.chain, mono_path_enabled);
}

/******************************************************************************/
kymera_chain_handle_t mixerSpeakerGetChain(void)
{
    return speaker_data.chain;
}

/******************************************************************************/
void mixerSpeakerStart(void)
{
    ChainStart(speaker_data.chain);
}

/******************************************************************************/
void mixerSpeakerStop(void)
{
    ChainStop(speaker_data.chain);
}

/******************************************************************************/
void mixerSpeakerDestroy(void)
{
    if (speaker_data.chain != NULL)
    {
        ChainDestroy(speaker_data.chain);
        speaker_data.chain = NULL;
        speaker_data.mode = audio_mixer_speaker_invalid;
    }
}

/******************************************************************************/
void mixerSpeakerSetVolume(void)
{
    int master_volume = AudioConfigGetMasterVolume();
    Operator volume_op = ChainGetOperatorByRole(speaker_data.chain, media_volume_role);

    if(AudioConfigGetTonesAreFixedVolume())
        OperatorsVolumeSetMainGain(volume_op, master_volume);
    else
        OperatorsVolumeSetMainAndAuxGain(volume_op, master_volume);
}

/******************************************************************************/
void mixerSpeakerMuteOutput(bool enable)
{
    Operator volume_op = getVolumeOperatorForHardwareSettings();
    if(volume_op != INVALID_OPERATOR)
        OperatorsVolumeMute(volume_op, enable);
}

/******************************************************************************/
bool mixerSpeakerIncreasesVolumePostProcessing(void)
{
    if(ChainGetOperatorByRole(speaker_data.chain, post_processing_role))
        return TRUE;
    return FALSE;
}
