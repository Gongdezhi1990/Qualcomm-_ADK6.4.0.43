/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
FILE NAME
    voice_assistant_audio_manager.c
 
DESCRIPTION
    This file contains the implementation details of all the interface functions of the voice assistant
    audio manager, which is responsible for any audio related actions.
*/

#include <string.h>
#include <stdlib.h>
#include <message.h>
#include <panic.h>
#include <vmal.h>

#include "voice_assistant_audio_manager.h"
#include "voice_assistant_audio_manager_private.h"
#include "audio.h"
#include "audio_config.h"
#include "audio_plugin_voice_assistant_variants.h"
#include "audio_plugin_voice_variants.h"

#define TIMER_MAX_COUNT (0xFFFFFFFFUL)

#ifdef DEBUG_PRINT_ENABLED
static const char * const trigger_type[va_audio_mgr_trigger_max] = {
    "BUTTON",
    "VOICE",
    "BUTTON AND VOICE",
};

static const char * const encoder_type[va_audio_mgr_codec_max] = {
    "SBC",
    "mSBC",
    "CELT",
    "INVALID",
};

static const char * const allocation_method[2] = {
    "LOUDNESS",
    "SNR",
};

static const char * channel_mode[4] = {
    "MONO",
    "DUAL MONO",
    "STEREO",
    "JOINT_STEREO",
};
#endif /* DEBUG_PRINT_ENABLED */

/* instance of voice assistant audio manager */
va_audio_manager_t *va_audio_mgr = NULL;

/* Utility Macros for checking conditions */
#define PANIC_IF_NOT_INITIALIZED(audio_mgr)    if(!audio_mgr) \
                                                                 { \
                                                                    PRINT(("VAM: Not Initialized\n")); \
                                                                    Panic(); \
                                                                 }

#define VALIDATE_AUDIO_MANAGER(audio_mgr)  PANIC_IF_NOT_INITIALIZED(audio_mgr); \
                                                                               if(!vaAudioMgrIsConfigured(audio_mgr)) \
                                                                    { \
                                                                        PRINT(("VAM: Not Configured\n")); \
                                                                        return va_audio_mgr_status_not_configured; \
                                                                    }

#define vaAudioMgrGetTask(audio_mgr)             (Task)&audio_mgr->lib_task

#define vaAudioMgrVoiceTriggerConfigured(config) ((config->trigger_type == va_audio_mgr_trigger_voice || \
                                                                             config->trigger_type == va_audio_mgr_trigger_both)) ? \
                                                                             TRUE : FALSE;
#define vaAudioMgrButtontriggerConfigured(config) ((config->trigger_type == va_audio_mgr_trigger_button || \
                                                                             config->trigger_type == va_audio_mgr_trigger_both)) ? \
                                                                             TRUE : FALSE;
                                                                                
/****************************** Utility Functions********************************/

/***************************************************************************/
static Task vaAudioMgrGetPlugin(void)
{
        return (Task)&voice_assistant_plugin;
}

/***************************************************************************/
va_audio_manager_t * vaAudioMgrGetContext(void)
{
    return va_audio_mgr;
}

/***************************************************************************/
static void vaAudioMgrCreateContext(void)
{
    /* just single instance */
    if(!va_audio_mgr)
    {
        va_audio_mgr = PanicNull(calloc(1, sizeof *va_audio_mgr));
    }
}

/***************************************************************************/
static bool vaAudioMgrIsConfigured(va_audio_manager_t *audio_mgr)
{
    return ((audio_mgr->config) ? TRUE : FALSE);
}

/***************************************************************************/
static void vaAudioMgrConfigureEncoder(va_audio_mgr_codec_config_t codec_config)
{
    PRINT(("VAM: Configuring for %s Encoder\n", encoder_type[codec_config.codec_type]));
    switch(codec_config.codec_type)
    {
    case va_audio_mgr_codec_sbc:
        {
            sbc_encoder_params_t param;
            /* update the SBC codec params */
            param.allocation_method = codec_config.allocation_method;
            param.channel_mode = codec_config.channel_mode;
            param.sample_rate = codec_config.sample_rate;
            param.bitpool_size = codec_config.bitpool_size;
            param.number_of_blocks = codec_config.number_of_blocks;
            param.number_of_subbands = codec_config.number_of_subbands;
            PRINT(("VAM: Configuring for SBC Encoder\n"));
            PRINT(("VAM: Allocation Method : %s \n", allocation_method[param.allocation_method]));
            PRINT(("VAM: Channel Mode : %s \n", channel_mode[param.channel_mode]));
            PRINT(("VAM: Sample Rate : %d Hz\n", param.sample_rate));
            PRINT(("VAM: Number of Blocks : %d\n", param.number_of_blocks));
            PRINT(("VAM: Number of Sub-bands : %d\n", param.number_of_subbands));
            PRINT(("VAM: Bitpool Size : %d\n", param.bitpool_size));
            /* Configure the SBC encoder params */
            AudioConfigSetSbcEncoderParams(param);
            AudioConfigSetVaCaptureCodec(va_capture_codec_sbc);
        }
        break;

    case va_audio_mgr_codec_msbc:
        AudioConfigSetMsbcEncoderBitpool(codec_config.bitpool_size);
        AudioConfigSetVaCaptureCodec(va_capture_codec_msbc);
        break;

    case va_audio_mgr_codec_celt:
        {
            /* TODO SS78: Need to know what params to configure of CELT codec?? */
        }
        break;

    default:
            Panic();
        break;
    }
}

/****************************** Interface APIs ********************************/
/***************************************************************************/
bool VaAudioMgrInit(Task apptask)
{
    va_audio_manager_t  *audio_mgr;
    PRINT(("VAM: VaAudioMgrInit() \n"));

    if(apptask)
    {
        vaAudioMgrCreateContext();
        audio_mgr = vaAudioMgrGetContext();

        audio_mgr->lib_task.handler = vaAudioMgrMsgHandler;
        audio_mgr->app_task = apptask;
        audio_mgr->config = NULL;
        audio_mgr->mic_config = NULL;
        audio_mgr->audio_inst = NULL;
        AudioConfigSetAlwaysUseAecRef(TRUE);
        return TRUE;
    }       
    return FALSE;
}

/******************************************************************************************/
va_audio_mgr_status_t VaAudioMgrConfigure(const va_audio_mgr_feature_config_t *feature_config, 
                                                                          const va_audio_mgr_audio_config_t *audio_config)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();
    va_audio_mgr_feature_config_t *config;
    voice_mic_params_t                *mic_config;
    PRINT(("VAM: VaAudioMgrConfigureFeature() \n"));

    PANIC_IF_NOT_INITIALIZED(audio_mgr);

    if(!feature_config || !audio_config)
        return va_audio_mgr_status_invalid_param;

    if(!audio_mgr->config)
    {
        /* allocate the memory to store the feature configuration */
        audio_mgr->config = PanicUnlessMalloc(sizeof(va_audio_mgr_feature_config_t));
    }

    memset(audio_mgr->config, 0, sizeof(va_audio_mgr_feature_config_t));
    config = audio_mgr->config;
    /* update the config structure */
    config->trigger_type = feature_config->trigger_type;
    config->pre_roll_interval = feature_config->pre_roll_interval;
    config->phrase_index = feature_config->phrase_index;

    PRINT(("VAM: Trigger Type : %s\n", trigger_type[config->trigger_type]));
    PRINT(("VAM: Pre-Roll Interval : %lx ms\n", config->pre_roll_interval));

    /* Audio configs */
    if(!audio_mgr->mic_config)
    {
        /* allocate the memory to store the mic configuration */
        audio_mgr->mic_config = (voice_mic_params_t*)PanicUnlessMalloc(sizeof(voice_mic_params_t));
    }

    /* configure the mic parameters for voice assistant */
    memset(audio_mgr->mic_config, 0, sizeof(voice_mic_params_t));
    mic_config = audio_mgr->mic_config;
    /* update the structure */
    mic_config->mic_a = audio_config->mic_config.mic_a;
    mic_config->mic_b = audio_config->mic_config.mic_b;
    mic_config->mic_c = audio_config->mic_config.mic_c;

    /* Update the cvc type */
    audio_mgr->cvc = audio_config->cvc_type;
    
    /* configure encoder parameters */
    vaAudioMgrConfigureEncoder(audio_config->codec_config);

    return va_audio_mgr_status_success;    
}

/***************************************************************************/
va_audio_mgr_status_t VaAudioMgrEnable(void)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();
    audio_va_connect_params_t *va_params;
    AudioPluginFeatures features;
    bool support_button_trigger = FALSE;
    PRINT(("VAM: Enable \n"));
    
    VALIDATE_AUDIO_MANAGER(audio_mgr);   
   
    memset(&features, 0, sizeof(features));
    /* The memory of the va_params shall be freed in the plugin */
    va_params = PanicUnlessMalloc(sizeof(audio_va_connect_params_t));
    va_params->app_task = vaAudioMgrGetTask(audio_mgr);
    va_params->support_voice_trigger = vaAudioMgrVoiceTriggerConfigured(audio_mgr->config);
    va_params->mic_params = audio_mgr->mic_config;
    va_params->cvc_operator = audio_mgr->cvc;
    va_params->cvc_omni_mode = FALSE;
   
    /*Use cVC omni-mode for sending mic data when there is no trigger detected */
    support_button_trigger = vaAudioMgrButtontriggerConfigured(audio_mgr->config);
    if(support_button_trigger && (va_params->cvc_operator == plugin_cvc_spkr_3mic_farfield))
           va_params->cvc_omni_mode = TRUE;


   /* trigger the connect so that plugin is ready */
   audio_mgr->audio_inst =  AudioConnect(vaAudioMgrGetPlugin(),
                                                    NULL,
                                                    AUDIO_SINK_SIDE_GRAPH,
                                                    0/*volume*/,
                                                    0/*rate*/,
                                                    features/*features*/,
                                                    AUDIO_MODE_STANDBY/*mode*/,
                                                    AUDIO_ROUTE_INTERNAL/*route*/,
                                                    POWER_BATT_CRITICAL/*power*/,
                                                    va_params,
                                                    vaAudioMgrGetTask(audio_mgr));
    
   return va_audio_mgr_status_success;
}

/***************************************************************************/
va_audio_mgr_status_t VaAudioMgrDisable(void)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();

    PANIC_IF_NOT_INITIALIZED(audio_mgr);

    PRINT(("VAM: Disabled \n"));
    /* flush all the task in the audio manager queue */
    MessageFlushTask(vaAudioMgrGetTask(audio_mgr));
    AudioDisconnectInstance(audio_mgr->audio_inst);
    
    return va_audio_mgr_status_success;
}

/********************************************************/
va_audio_mgr_status_t VaAudioMgrActivateTrigger(void)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();
    bool support_voice_trigger = FALSE;

    PRINT(("VAM: VaAudioMgrActivateTrigger() \n"));

    VALIDATE_AUDIO_MANAGER(audio_mgr);
    support_voice_trigger = vaAudioMgrVoiceTriggerConfigured(audio_mgr->config);

    /* allow trigger olny if voice assistant was configured for voice trigger */
    if(support_voice_trigger)
    {
        /* Call the audio library to activate the trigger plugin */
        AudioVoiceAssistantEnableTrigger(audio_mgr->config->phrase_index);
    }
    /* not configured for voice trigger, just ignore it */
    return va_audio_mgr_status_success;
}

/********************************************************/
va_audio_mgr_status_t VaAudioMgrDeactivateTrigger(void)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();
    bool support_voice_trigger = FALSE;
    PRINT(("VAM: VaAudioMgrDeactivateTrigger() \n"));

    VALIDATE_AUDIO_MANAGER(audio_mgr);
    support_voice_trigger = vaAudioMgrVoiceTriggerConfigured(audio_mgr->config);

    /* allow trigger olny if voice assistant was configured for voice trigger */
    if(support_voice_trigger)
    {
        /* flush out all the trigger indication msgs as client is no more interested in
            trigger detection */
        MessageCancelAll(vaAudioMgrGetTask(audio_mgr), AUDIO_VA_MESSAGE_TRIGGERED_IND);
        /* Call the audio library to deactivate the trigger plugin */
        AudioVoiceAssistantDisableTrigger();
    }
    /* not configured for voice trigger, just ignore it */
    return va_audio_mgr_status_success;
}

/******************************************************/
va_audio_mgr_status_t VaAudioMgrStartCapture(uint32 start_ts)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();
    bool support_voice_trigger = FALSE;
    va_audio_mgr_feature_config_t *config;

    PRINT(("VAM: VaAudioMgrStartCapture() \n"));

    VALIDATE_AUDIO_MANAGER(audio_mgr);

    config = audio_mgr->config;
    support_voice_trigger = vaAudioMgrVoiceTriggerConfigured(config);
    if(start_ts && support_voice_trigger)
    {
        uint32 start_timestamp = 0;
        if(start_ts < config->pre_roll_interval)
            start_timestamp = (uint32)(start_ts + ((TIMER_MAX_COUNT - config->pre_roll_interval)+1));
        else
            start_timestamp = start_ts - config->pre_roll_interval;

         AudioVoiceAssistantStartSend(TRUE, start_timestamp);
    }
    else
    {
        /* Either this is button press or voice trigger without pre-roll */
         AudioVoiceAssistantStartSend(FALSE, 0);
    }

    return va_audio_mgr_status_success;
}

/******************************************************/
va_audio_mgr_status_t VaAudioMgrStopCapture(void)
{
    va_audio_manager_t  *audio_mgr = vaAudioMgrGetContext();
    PRINT(("VAM: VaAudioMgrStopCapture() \n"));

    VALIDATE_AUDIO_MANAGER(audio_mgr);

    /* flush the mic source indication msg, as application is no more interested in it */
    MessageCancelAll((Task)&(audio_mgr->lib_task), AUDIO_VA_INDICATE_DATA_SOURCE);

    /* Call the audio library to stop capturing mic data */
    AudioVoiceAssistantStopSend();

    return va_audio_mgr_status_success;
}

#ifdef VA_AUDIOMGR_TEST_BUILD
/******************************************************/
void VaAudioMgrDeInit(void)
{
     if(va_audio_mgr)
     {
        if(va_audio_mgr->config)
            free(va_audio_mgr->config);
        if(va_audio_mgr->mic_config)
            free(va_audio_mgr->mic_config);

        free(va_audio_mgr);
        va_audio_mgr = NULL;
    }
}

#endif/* VA_AUDIOMGR_TEST_BUILD */



