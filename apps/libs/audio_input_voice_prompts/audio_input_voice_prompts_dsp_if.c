/****************************************************************************
Copyright (c) 2015-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_voice_prompts_dsp_if.c

DESCRIPTION
    Kymera DSP implementation of voice prompt plug-in.
*/

#include <stdlib.h>
#include <string.h>

#include <sink.h>
#include <print.h>

#include "audio.h"
#include <chain.h>

#include "audio_input_common.h"

#include "audio_input_voice_prompts_dsp_if.h"
#include "audio_input_voice_prompts_utils.h"
#include "audio_input_voice_prompts_play.h"
#include "audio_input_voice_prompts_chain.h"


#define FORCED_TONE_PLAYBACK_RATE   (48000)

#define TONE_ID_MSG_ID 1

static bool isItToneEndMessage(const MessageFromOperator *op_msg);


/*******************************************************************************
DESCRIPTION
    Entry point for DSP specific playback of tones and voice prompts.
*/
void VoicePromptsDspPlay(vp_context_t *context)
{
    PRINT(("VoicePromptsDspPlay() : "));

    AudioInputCommonDspPowerOn();

    context->chain = VoicePromptsChainConnectAndStart(context);

}

/*******************************************************************************
DESCRIPTION
    Stop of tone or voice prompt. It is used for both forced stop and
    for handling end of voice prompt message.
    Triggers CsrVoicePromptsPluginCleanup().
*/
void VoicePromptsDspStop(void)
{
    SetAudioBusy(NULL);
}

/*******************************************************************************
DESCRIPTION
    DSP specific. Stop operators before disconnecting and closing them down.
*/
void VoicePromptsDspPrepareForClose(vp_context_t *context)
{
    VoicePromptsChainStop(context->chain);
}

/*******************************************************************************
DESCRIPTION
    Tells upper layer if closing down procedure should continue.
*/
bool VoicePromptsDspShallHandleStreamDisconnect(vp_context_t *context)
{
    return context ? TRUE : FALSE;
}

/*******************************************************************************
DESCRIPTION
    Actual clean up of DSP resources.
*/
void VoicePromptsDspCleanup(vp_context_t *context)
{
    UNUSED(context);

    PRINT(("VoicePromptsDspCleanup\n"));

    VoicePromptsChainDisconnect(context->chain);
    context->chain = NULL;

    OperatorsFrameworkDisable();

    /* Notify other plugin that tone has finished */
    if(AudioIsAudioPromptPlaying())
    {
        MessageSend(AudioGetAudioPromptPlayingTask(), AUDIO_PLUGIN_TONE_END_NOTIFICATION_MSG, NULL);
    }
}

/*******************************************************************************
DESCRIPTION
    Helper function to determine if messages was sent by DSP.
*/
bool VoicePromptsIsItDspMessage(MessageId id)
{
    return (id == MESSAGE_FROM_OPERATOR) ? TRUE : FALSE;
}

/*******************************************************************************
DESCRIPTION
    Message handler for DSP messages.
*/
void VoicePromptsDspMessageHandler(vp_context_t *context, Task task, Message message)
{
    UNUSED(context);
    UNUSED(task);

    if(isItToneEndMessage((const MessageFromOperator *)message))
    {
        AudioInputVoicePromptsPluginStopPhrase();
    }
}

/*******************************************************************************
DESCRIPTION
    Determine if received message is a message from DSP.
*/
static bool isItToneEndMessage(const MessageFromOperator *op_msg)
{
    /* TONE_END message have different format on Bluecore and Hydra.
       On Bluecore message length is 1 and data are in the 1st word.
       On Hydra message length is 3 and data are in the 2nd word.
     */
    if(op_msg->len == 1)
    {
        if(op_msg->message[0] == TONE_ID_MSG_ID)
        {
            return TRUE;
        }
    }
    else if(op_msg->len == 3)
    {
        if(op_msg->message[1] == TONE_ID_MSG_ID)
        {
            return TRUE;
        }
    }

    return FALSE;
}

