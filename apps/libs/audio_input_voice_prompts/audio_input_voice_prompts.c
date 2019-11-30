/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_voice_prompts.c

DESCRIPTION
    Message dispatcher.
*/
#include <stdlib.h>

#include <message.h>
#include <audio.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>
#include <vmtypes.h>
#include <print.h>

#include "audio_input_voice_prompts.h"
#include "audio_input_voice_prompts_chain.h"
#include "audio_input_voice_prompts_play.h"
#include "audio_plugin_voice_prompts_variants.h"

/*the local message handling functions*/
static void handleAudioMessage (Task task , MessageId id, Message message);
static void handleInternalMessage (Task task , MessageId id, Message message);

const TaskData csr_voice_prompts_plugin = { AudioInputVoicePromptsMessageHandler };

/****************************************************************************
DESCRIPTION
    The main task message handler
*/
void AudioInputVoicePromptsMessageHandler ( Task task, MessageId id, Message message )
{
    if ( (id >= AUDIO_DOWNSTREAM_MESSAGE_BASE ) && (id < AUDIO_DOWNSTREAM_MESSAGE_TOP) )
    {
        handleAudioMessage (task , id, message ) ;
    }
    else if (VoicePromptsIsItDspMessage(id))
    {
        VoicePromptsDspMessageHandler(VoicePromptsGetContext(), task , message) ;
    }
    else if (VoicePromptsIsChainMessage(id))
    {
        VoicePromptsChainMessageHandler(task, id, message);
    }
    else
    {
        handleInternalMessage (task , id , message ) ;
    }
}

/****************************************************************************
DESCRIPTION

    messages from the audio library are received here.
    and converted into function calls to be implemented in the
    plugin module
*/
static void handleAudioMessage ( Task task , MessageId id, Message message )
{
    UNUSED(task);
    switch (id)
    {
        case (AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG ):
        {
            const AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG_T * prompt_message = (const AUDIO_PLUGIN_PLAY_AUDIO_PROMPT_MSG_T *)message ;

            AudioInputVoicePromptsPluginPlayPhrase(prompt_message->prompt_index,
                                                   prompt_message->prompt_header_index,
                                                   prompt_message->ap_volume, /* Here its used for playing audio without DSP */
                                                   prompt_message->features,
                                                   prompt_message->app_task);

        }
        break ;

        case (AUDIO_PLUGIN_PLAY_TONE_MSG ):
        {
            const AUDIO_PLUGIN_PLAY_TONE_MSG_T * tone_message = (const AUDIO_PLUGIN_PLAY_TONE_MSG_T *)message ;

            AudioInputVoicePromptsPluginPlayTone(tone_message->tone,
                tone_message->tone_volume,
                tone_message->features) ;
        }
        break ;

        case (AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG ):
            AudioInputVoicePromptsPluginStopPhrase() ;
        break ;

        #ifdef HOSTED_TEST_ENVIRONMENT
        case (AUDIO_PLUGIN_TEST_RESET_MSG):
            AudioInputVoicePromptsPluginTestReset();
        break;
        #endif

        default:
            /*other messages do not need to be handled by the voice prompts plugin*/
        break;
    }
}

/****************************************************************************
DESCRIPTION
    Internal messages to the task are handled here
*/
static void handleInternalMessage ( Task task , MessageId id, Message message )
{
    UNUSED(task);
    UNUSED(message);
    switch (id)
    {
        case MESSAGE_STREAM_DISCONNECT:
            AudioInputVoicePromptsPluginHandleStreamDisconnect();
        break ;

        case MESSAGE_MORE_SPACE:
        case MESSAGE_SOURCE_EMPTY:
        break;

        default:
            Panic();
    }
}

bool CsrVoicePromptsIsMixable(FILE_INDEX prompt_header_index)
{
    UNUSED(prompt_header_index);
    return TRUE;
}

#ifdef HOSTED_TEST_ENVIRONMENT
/* To cleanup the Voice Prommpt Context Data */
void AudioInputVoicePromptsPluginTestReset(void)
{
    AudioMixerTestReset();
    freePhraseData();
}
#endif
