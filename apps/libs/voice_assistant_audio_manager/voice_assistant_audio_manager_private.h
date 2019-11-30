/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
FILE NAME
    voice_assistant_audio_manager_private.h
 
DESCRIPTION
    This file defines all the functions and data structure which is local to audio manager and which
    should not be exposed
*/

#ifndef _VA_AUDIO_MANAGER_PRIVATE_H_
#define _VA_AUDIO_MANAGER_PRIVATE_H_

#include <print.h>

#include "voice_assistant_audio_manager.h"
#include "audio_instance.h"

/* Message macros */
#define MAKE_MESSAGE(TYPE) MESSAGE_MAKE(message,TYPE##_T);

/* data structure defining the audio manager */
typedef struct __va_audio_manager
{
    TaskData lib_task;
    Task app_task;
    va_audio_mgr_feature_config_t *config;
    voice_mic_params_t  *mic_config;
    audio_instance_t    audio_inst;
    uint8 cvc;
} va_audio_manager_t;

/***************************************************************************
DESCRIPTION
    Data/Control message handler for VA audio manager
 
PARAMS
    task - task associated with the message
    id - message id which needs to be handled
    message - data payload which needs to be processed
 
RETURNS
    None
*/
void vaAudioMgrMsgHandler(Task task, MessageId id, Message message);

/***************************************************************************
DESCRIPTION
    Returns the VA Audio Manager Instance
 
PARAMS
    None
 
RETURNS
    pointer to the audio manager instance
*/
va_audio_manager_t * vaAudioMgrGetContext(void);


#endif /* ifdef _VA_AUDIO_MANAGER_PRIVATE_H_ */

