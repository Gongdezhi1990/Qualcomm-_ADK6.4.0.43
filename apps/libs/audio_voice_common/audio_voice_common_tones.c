/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_common_tones.c

DESCRIPTION
    Implementation of tones operations for audio input HFP.
*/

#include <string.h>
#include <audio.h>
#include <print.h>

#include "audio_voice_common.h"


/******************************************************************************/
void AudioVoiceCommonTonePlay(Task task, Message msg, audio_voice_context_t *ctx)
{
    UNUSED(ctx);
    AUDIO_PLUGIN_PLAY_TONE_MSG_T * tone_message = (AUDIO_PLUGIN_PLAY_TONE_MSG_T*) msg;
    PRINT(("CVC: Tone play\n"));
    AudioSetAudioPromptPlayingTask(task);

    /* Handle tones in the tone plug-in */
    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_PLAY_TONE_MSG, new_message);
    memcpy(new_message, tone_message, sizeof(AUDIO_PLUGIN_PLAY_TONE_MSG_T));
    MessageSend(AudioGetTonePlugin(), AUDIO_PLUGIN_PLAY_TONE_MSG, new_message);

    /* Set audio busy here otherwise other pending tone messages will be sent */
    SetAudioBusy(AudioGetTonePlugin());
}

/******************************************************************************/
void AudioVoiceCommonToneStop(Task task, Message msg, audio_voice_context_t *ctx)
{
    UNUSED(task);
    UNUSED(msg);
    UNUSED(ctx);
    if(IsAudioBusy())
    {
        PRINT(("CVC: Tone stop\n"));
        MessageSend(AudioGetTonePlugin(), AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG, 0);

        /* Update current tone playing status */
        AudioSetAudioPromptPlayingTask((Task)NULL);
    }
}
