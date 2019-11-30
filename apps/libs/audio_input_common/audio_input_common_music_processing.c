/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common_music_processing.c

DESCRIPTION
    Music processing related handlers.
*/
#include <stdlib.h>
#include <string.h>
#include <print.h>


#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_config.h>
#include <audio_music_processing.h>

#include "audio_input_common.h"
#include "audio_input_common_music_processing.h"

typedef struct
{
    bool music_enhancements_enable_compander;
    bool music_enhancements_enable_user_peq;
    bool music_enhancements_enable_vse;
    bool music_enhancements_enable_dbe;
} music_enhancements_t;

/****************************************************************************
DESCRIPTION
    Decode music_mode_enhancements bitfield to determine which enhancements should be enabled/disabled
*/
static music_enhancements_t get_music_enhancements(const A2DP_MUSIC_PROCESSING_T music_processing_mode, const uint16 music_mode_enhancements)
{
    music_enhancements_t music_enhancements = {0};

    if (music_processing_mode != A2DP_MUSIC_PROCESSING_PASSTHROUGH)
    {
        /* Each bit in music_mode_enhancements is set to 1 to indicate a feature should be bypassed.
         * And a bit value of 0 is used to indicate the feature should be enabled. */
        music_enhancements.music_enhancements_enable_compander = !(music_mode_enhancements & MUSIC_CONFIG_COMPANDER_BYPASS);
        music_enhancements.music_enhancements_enable_user_peq = !(music_mode_enhancements & MUSIC_CONFIG_USER_EQ_BYPASS);
        music_enhancements.music_enhancements_enable_vse = !(music_mode_enhancements & MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS);
        music_enhancements.music_enhancements_enable_dbe = !(music_mode_enhancements & MUSIC_CONFIG_BASS_ENHANCE_BYPASS);
    }
    return music_enhancements;
}

/****************************************************************************
DESCRIPTION
    Determine the new peq config required, or not applicable if disabled or no change required.
*/

static peq_config_t getPeqConfig(const A2DP_MUSIC_PROCESSING_T music_processing_mode)
{
    peq_config_t config = peq_config_default;
    switch (music_processing_mode)
    {
        case A2DP_MUSIC_PROCESSING_FULL:
        case A2DP_MUSIC_PROCESSING_PASSTHROUGH:
        {
            config = AudioMusicProcessingGetPeqConfig();
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_NEXT_EQ_BANK:
        {
            config = AudioMusicProcessingGetPeqConfig();
            if ((config == peq_config_6) || (config == peq_config_default))
                config = peq_config_0;
            else
                config++;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK0:
        {
            config = peq_config_0;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK1:
        {
            config = peq_config_1;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK2:
        {
            config = peq_config_2;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK3:
        {
            config = peq_config_3;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK4:
        {
            config = peq_config_4;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK5:
        {
            config = peq_config_5;
            break;
        }
        case A2DP_MUSIC_PROCESSING_FULL_SET_EQ_BANK6:
        {
            config = peq_config_6;
            break;
        }
        default:
            Panic();
    }
    return config;
}

/****************************************************************************
DESCRIPTION
    Select a new PEQ configuration
*/
static void selectPeqConfig(const A2DP_MUSIC_PROCESSING_T music_processing_mode)
{
        peq_config_t peq_config = getPeqConfig(music_processing_mode);
        AudioMusicProcessingSelectUserPeqConfig(peq_config);
}

void AudioInputCommonSetUserEqParameter(const audio_plugin_user_eq_param_t* param)
{
    AudioMusicProcessingSetUserEqParameter(param);
}

void AudioInputCommonApplyUserEqParameters(void)
{
    AudioMusicProcessingApplyUserEqParameters();
}

void AudioInputCommonClearUserEqParameters(void)
{
    AudioMusicProcessingClearUserEqParameters();
}

void AudioInputCommonGetUserEqParameter(const audio_plugin_user_eq_param_id_t* param_id, Task audio_plugin, Task callback_task)
{
    SetAudioBusy(audio_plugin);
    AudioConfigSetAppTask(callback_task);
    AudioMusicProcessingGetUserEqParameter(param_id, audio_plugin);
    SetAudioBusy(NULL);
}

void AudioInputCommonGetUserEqParameters(const unsigned number_of_params, const audio_plugin_user_eq_param_id_t* param_ids, Task audio_plugin, Task callback_task)
{
    SetAudioBusy(audio_plugin);
    AudioConfigSetAppTask(callback_task);
    AudioMusicProcessingGetUserEqParameters(number_of_params, param_ids, audio_plugin);
    SetAudioBusy(NULL);
}

void AudioInputCommonSendGetUserEqParameterResponse(const bool data_valid, const audio_plugin_user_eq_param_t* param)
{
    unsigned message_size = sizeof(AUDIO_GET_USER_EQ_PARAMETER_CFM_T);
    AUDIO_GET_USER_EQ_PARAMETER_CFM_T* message = PanicUnlessMalloc(message_size);
    memset(message, 0, message_size);
    Task app_task = AudioConfigGetAppTask();

    message->data_valid = data_valid;
    if (data_valid)
    {
        message->param[0].id.bank = param->id.bank;
        message->param[0].id.band = param->id.band;
        message->param[0].id.param_type = param->id.param_type;

        message->param[0].value = param->value;
    }
    MessageSend(app_task, AUDIO_GET_USER_EQ_PARAMETER_CFM, message);
}

void AudioInputCommonSendGetUserEqParametersResponse(const bool data_valid, const unsigned number_of_params, const audio_plugin_user_eq_param_t* params)
{
    unsigned i;
    unsigned message_size = CALC_MESSAGE_LENGTH_WITH_VARIABLE_PARAMS(AUDIO_GET_USER_EQ_PARAMETERS_CFM_T, number_of_params, user_eq_param_t);
    AUDIO_GET_USER_EQ_PARAMETERS_CFM_T* message = PanicUnlessMalloc(message_size);
    memset(message, 0, message_size);

    Task app_task = AudioConfigGetAppTask();

    message->data_valid = data_valid;
    message->number_of_params = (uint16)number_of_params;

    for(i = 0; i < number_of_params; i++)
    {
        message->params[i].id.bank = params[i].id.bank;
        message->params[i].id.band = params[i].id.band;
        message->params[i].id.param_type = params[i].id.param_type;
        message->params[i].value = params[i].value;
    }

    MessageSend(app_task, AUDIO_GET_USER_EQ_PARAMETERS_CFM, message);
}

void audioInputCommonSetMusicProcessing(audio_input_context_t *ctx, const A2DP_MUSIC_PROCESSING_T music_processing_mode, const uint16 music_mode_enhancements)
 {
    selectPeqConfig(music_processing_mode);

    if (music_mode_enhancements & MUSIC_CONFIG_DATA_VALID)
    {
        music_enhancements_t music_enhancements = get_music_enhancements(music_processing_mode, music_mode_enhancements);

        AudioMusicProcessingEnableProcessing(audio_music_processing_compander_role, music_enhancements.music_enhancements_enable_compander);
        AudioMusicProcessingEnableProcessing(audio_music_processing_user_peq_role, music_enhancements.music_enhancements_enable_user_peq);
        AudioMusicProcessingEnableProcessing(audio_music_processing_volume_spatial_enhancement_role, music_enhancements.music_enhancements_enable_vse);
        AudioMusicProcessingEnableProcessing(audio_music_processing_dynamic_bass_enhancement_role, music_enhancements.music_enhancements_enable_dbe);
    }

    ctx->music_processing = music_processing_mode;
    ctx->music_enhancements = music_mode_enhancements;
}

void AudioInputCommonSetMusicProcessingContext(audio_input_context_t *ctx, A2dpPluginConnectParams* params)
{
    if (params)
    {
        A2dpPluginModeParams* mode_params = (A2dpPluginModeParams*) params->mode_params;
        if (mode_params)
        {
            ctx->music_processing = mode_params->music_mode_processing;
            ctx->music_enhancements = mode_params->music_mode_enhancements;
        }
    }
}
