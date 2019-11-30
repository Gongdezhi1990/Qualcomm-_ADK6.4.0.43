/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
FILE NAME
    voice_assistant_audio_manager_handler.c
 
DESCRIPTION
    This file implements the data/control message handler for voice assistant audio manager
*/

#include <message.h>
#include <panic.h>
#include <vmal.h>

#include "voice_assistant_audio_manager_private.h"
#include "voice_assistant_audio_manager.h"
#include "audio.h"


/****************************** Utility Functions********************************************/
static void vaAudioMgrSendTriggerPhraseInd(AUDIO_VA_MESSAGE_TRIGGERED_IND_T *ind)
{
    va_audio_manager_t *audio_mgr = vaAudioMgrGetContext();

    MAKE_MESSAGE(VA_AUDIO_MGR_WAKEWORD_IND);
    message->start_timestamp = ind->start_timestamp;
    message->end_timestamp = ind->end_timestamp;
    message->qva_instance = 0; /*TODO SS78: once plugin supports this parameter, update it. For now default */
    message->trigger_phrase_index = 0;/*TODO SS78: once plugin supports this parameter, update it. For now default */

    PRINT(("VAM: Trigger Detected\n"));
    MessageSend(audio_mgr->app_task, VA_AUDIO_MGR_WAKEWORD_IND, message);
}

static void vaAudioMgrSendMicSourceInd(AUDIO_VA_MESSAGE_START_SEND_CFM_T *cfm)
{
    va_audio_manager_t *audio_mgr = vaAudioMgrGetContext();
    
    MAKE_MESSAGE(VA_AUDIO_MGR_CAPTURE_IND);
    message->src = cfm->source;

    PRINT(("VAM: Got the mic source, started mic capture\n"));
    MessageSend(audio_mgr->app_task, VA_AUDIO_MGR_CAPTURE_IND, message);
}


/***************************************************************************/
void vaAudioMgrMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch(id)
    {
        case AUDIO_VA_MESSAGE_TRIGGERED_IND:
        {
            vaAudioMgrSendTriggerPhraseInd((AUDIO_VA_MESSAGE_TRIGGERED_IND_T*)(message));            
        }
        break;
        /* In both the cases we need to send source indication msg to application. Eventually this could
            be just one message coming from va plugin */
        case AUDIO_VA_INDICATE_DATA_SOURCE:
        case AUDIO_VA_MESSAGE_START_SEND_CFM:
        {
            vaAudioMgrSendMicSourceInd((AUDIO_VA_MESSAGE_START_SEND_CFM_T*)(message));
        }
        break;
    }
}

