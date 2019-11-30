/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_common_.c

DESCRIPTION
    Default handlers for non input specific voice messages.
    To be used as common implementation for audio_voice_xxx libraries.
*/

#include <print.h>
#include <stdlib.h>

#include <audio_plugin_if.h>
#include <audio_mixer.h>
#include <audio.h>

#include "audio_voice_common.h"

#include "../audio_voice_hfp/audio_voice_hfp.h"
#include "audio_voice_common_states.h"
#include "audio_voice_common_messages.h"



static void audioVoiceCommonControlMessageHandler(Task task, MessageId id, Message message, audio_voice_context_t *ctx)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);
    UNUSED(ctx);
}


audio_voice_context_t* AudioVoiceCommonCreateContext(void)
{
    return PanicUnlessNew(audio_voice_context_t);
}

audio_voice_context_t* AudioVoiceCommonDestroyContext(audio_voice_context_t* ctx)
{
    free(ctx);
    return NULL;
}


void AudioVoiceCommonMessageHandler(Task task, MessageId id, Message message, audio_voice_context_t *ctx)
{
    PRINT(("Audio Voice Common Handler: id 0x%x\n", (unsigned)id));

    switch(id)
    {
        case AUDIO_VOICE_CONNECT_ERROR:
            AudioVoiceCommonHandleEvent(task, audio_voice_error, message, ctx);
        break;

        case AUDIO_PLUGIN_CONNECT_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_connect_req, message, ctx);
        break;

        case AUDIO_MIXER_FADE_IN_CFM:
            AudioVoiceCommonHandleEvent(task, audio_voice_connect_complete, message, ctx);
        break;

        case AUDIO_PLUGIN_PLAY_TONE_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_play_tone, message, ctx);
        break;

        case AUDIO_PLUGIN_STOP_TONE_AND_PROMPT_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_stop_tone, message, ctx);
        break;

        case AUDIO_PLUGIN_SET_VOLUME_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_set_volume, message, ctx);
        break;

        case AUDIO_PLUGIN_RESET_VOLUME_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_reset_volume, message, ctx);
        break;

        case AUDIO_PLUGIN_SET_SOFT_MUTE_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_set_soft_mute, message, ctx);
        break;

        case AUDIO_PLUGIN_DISCONNECT_MSG:
            AudioVoiceCommonHandleEvent(task, audio_voice_disconnect_req, message, ctx);
        break;

        case AUDIO_MIXER_FADE_OUT_CFM:
            AudioVoiceCommonHandleEvent(task, audio_voice_disconnect_complete, message, ctx);
        break;

        default:
            if(ctx->state != audio_voice_error_state)
                audioVoiceCommonControlMessageHandler(task, id, message, ctx);
            break;
    }
}


void AudioVoiceCommonDummyStateHander(Task task, Message msg, audio_voice_context_t *ctx)
{
    UNUSED(task);
    UNUSED(msg);
    UNUSED(ctx);
}


void AudioVoiceCommonDspPowerOn(void)
{
    OperatorsFrameworkEnable();
    SetCurrentDspStatus(DSP_LOADED_IDLE);
}

