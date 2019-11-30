/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
*/
 
 
/*!
@file    voice_assistant_audio_manager.h
@brief  Interface file to expose the voice assistant audio manager functionality to the application
          This handles all the audio related functionality of voice assistant

*/

#ifndef _VOICE_ASSISTANT_AUDIO_MANAGER_H_
#define _VOICE_ASSISTANT_AUDIO_MANAGER_H_

#include "library.h"
#include "audio_plugin_if.h"

/*!
    @brief Upstream messages from voice assistant audio manager to client applications
 
 */
typedef enum
{
    VA_AUDIO_MGR_CAPTURE_IND = VA_AUDIO_MGR_MESSAGE_BASE,
    VA_AUDIO_MGR_WAKEWORD_IND,
    /* Don't add anything after this */
    VA_AUDIO_MGR_MESSAGE_TOP
} va_audio_mgr_msg_t;

/*!
    @brief Defines the different trigger types of voice assistant
 
 */
typedef enum
{
    va_audio_mgr_trigger_button = 0,
    va_audio_mgr_trigger_voice,
    va_audio_mgr_trigger_both,
    va_audio_mgr_trigger_max
}va_audio_mgr_trigger_type_t;

/*!
    @brief Defines the different codecs of voice assistant
 
 */
typedef enum
{
    va_audio_mgr_codec_sbc = 0,
    va_audio_mgr_codec_msbc,
    va_audio_mgr_codec_celt,
    va_audio_mgr_codec_invalid,
    va_audio_mgr_codec_max
}va_audio_mgr_codec_type_t;

/*!
    @brief voice assistant audio manager status codes.

    Defines the return status codes for voice assistant audio manager upstream messages.
*/
typedef enum __va_audio_mgr_status
{
    va_audio_mgr_status_success = 0,                    
    va_audio_mgr_status_not_configured,
    va_audio_mgr_status_invalid_param,
    va_audio_mgr_status_failed                  /*! Generic failure */
} va_audio_mgr_status_t;

/*!
    @brief voice assistant audio manager feature configuration

    Defines the different feature configuration for voice assistant
*/
typedef struct __va_audio_mgr_feature_config
{
    va_audio_mgr_trigger_type_t   trigger_type;
    uint32                                  pre_roll_interval;
    FILE_INDEX                           phrase_index;
} va_audio_mgr_feature_config_t;

/*!
    @brief voice assistant audio manager codec configuration

*/
typedef struct __va_audio_mgr_codec_config
{
    unsigned number_of_subbands;
    unsigned number_of_blocks;
    unsigned bitpool_size;
    unsigned sample_rate;
    uint8 channel_mode;
    uint8 allocation_method;
    va_audio_mgr_codec_type_t codec_type;
}va_audio_mgr_codec_config_t;

/*!
    @brief voice assistant audio manager configurations related to audio

    Defines the different audio configuration for voice assistant
*/
typedef struct __va_audio_mgr_audio_config
{
    va_audio_mgr_codec_config_t codec_config;
    voice_mic_params_t               mic_config;
    uint8                       cvc_type;
} va_audio_mgr_audio_config_t;

/*!
    @brief This is an unsolicitated message which is to indicate that a trigger phrase (wake word) 
              was dectected.

    @param start_timestamp : timestamp of the trigger start sample index. Unit of time is in micro seconds
    @param end_index : timestamp of the trigger end sample index. Unit of time is in micro seconds
    @param qva_instance : points to the QVA instance which detected the wake-word. Useful for FFV
    @param trigger_phrase_index : indicates which phrase got triggered (if multiple trigger phrase 
                                                is enabled)
*/
typedef struct
{
    uint32 start_timestamp;
    uint32 end_timestamp;
    uint8 qva_instance;
    uint8 trigger_phrase_index;
} VA_AUDIO_MGR_WAKEWORD_IND_T;

/*!
    @brief This is an unsolicitated message which is to indicate that mic voice data captured 
              has started

    @param src : Pointer to the source stream of mic data
*/
typedef struct
{
   Source src;
}VA_AUDIO_MGR_CAPTURE_IND_T;

/******************************************************************************
    @brief This function initializes the voice assistant audio manager library. 

    @param appTask - application task that needs to be registered with the library to send the upstream
                              messages

    @return TRUE is successfully initialized, else FALSE
*/
bool VaAudioMgrInit(Task apptask);

/******************************************************************************
    @brief This interfaces configures the voice assistant

    @param feature_config - Pointer to va_audio_mgr_feature_config_t structure having the configuration
    @param audio_config - Pointer to va_audio_mgr_audio_config_t structure having the configuration for audio

    @return can expect va_audio_mgr_status_t 
*/
va_audio_mgr_status_t VaAudioMgrConfigure(const va_audio_mgr_feature_config_t *feature_config, 
                                                                           const va_audio_mgr_audio_config_t *audio_config);

/******************************************************************************
    @brief This interfaces enable's the voice assistant audio manager functionality. 

    @param None

    @return can expect va_audio_mgr_status_t 
*/
va_audio_mgr_status_t VaAudioMgrEnable(void);


/******************************************************************************
    @brief This interfaces disable's the voice assistant audio manager functionality. 

    @param None

    @return can expect va_audio_mgr_status_t 

    Note: To avoid unexpected behavior, its recomended to call VaAudioMgrDeactivateTrigger() 
    [if voice-trigger enabled] before calling VaAudioMgrDisable()
*/
va_audio_mgr_status_t VaAudioMgrDisable(void);


/***********************************************************************************
    @brief This interfaces activates the trigger phase so that once the trigger phrase is said it 
              shall be notified to application 

    @param none

    @return expect va_audio_mgr_status_t as the status of executing this interface.
*/
va_audio_mgr_status_t VaAudioMgrActivateTrigger(void);

/*************************************************************************************
    @brief This interfaces deactivates the trigger phase so that trigger phrase utterance is not captured 

    @param none

    @return expect va_audio_mgr_status_t as the status of executing this interface.
*/
va_audio_mgr_status_t VaAudioMgrDeactivateTrigger(void);


/*************************************************************************************
    @brief This is generic interfaces to inform start capturing mic data which needs to sent

    @param start_ts <optional> : start timestamp from where the captured voice data needs to sent.
                                                   optional because it may not be valid for button triggered VA

    @return expect va_audio_mgr_status_t as the status of executing this interface. An unsolicitated 
                message VA_AUDIO_MGR_CAPTURE_IND shall be received once mic source is avialable,
               /ref VA_AUDIO_MGR_CAPTURE_IND_T
*/
va_audio_mgr_status_t VaAudioMgrStartCapture(uint32 start_ts);

/*************************************************************************************
    @brief This interfaces informs to stop capturing mic data

    @param none

    @return expect va_audio_mgr_status_t as the status of executing this interface. 
*/
va_audio_mgr_status_t VaAudioMgrStopCapture(void);

#ifdef VA_AUDIOMGR_TEST_BUILD
/*************************************************************************************
    @brief This interfaces deinitialize Audio manager. This is for unit test purpose only

    @param none

    @return none
*/
void VaAudioMgrDeInit(void);
#endif /* VA_AUDIOMGR_TEST_BUILD */

#endif /* ifdef _VOICE_ASSISTANT_AUDIO_MANAGER_H_ */

