/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_common_volume.h

DESCRIPTION
    Implementation of volume operations for audio input HFP.
*/

#include <operators.h>

#include "audio_voice_common.h"
#include "audio_voice_common_config.h"
#include <audio_hardware.h>

/*******************************************************************************
DESCRIPTION
    Send the DSP a message to mute or unmute microphone and/or speakers.
*/
static void audioVoiceCommonConfigureMute(bool mute_microphone, bool mute_speaker, audio_voice_context_t* ctx)
{
    if(mute_speaker != ctx->speaker_muted)
        AudioMixerMuteOutput(mute_speaker);

    if(mute_microphone != ctx->mic_muted)
        AudioHardwareMuteMicOutput(mute_microphone);
}

/*******************************************************************************
DESCRIPTION
    Send the DSP a message to set the speaker volume.
*/
static void audioVoiceCommonConfigureVolume(void)
{
    AudioMixerSetVolume();
}

/******************************************************************************/
void AudioVoiceCommonVolumeSet(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(task);
    UNUSED(msg);

    if(!(ctx->speaker_muted))
    {
        audioVoiceCommonConfigureVolume();
    }
}

/******************************************************************************/
void AudioVoiceCommonVolumeSetSoftMute(Task task, Message msg, audio_voice_context_t* ctx)
{
    UNUSED(task);
    AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T* message = (AUDIO_PLUGIN_SET_SOFT_MUTE_MSG_T*) msg;
    bool mute_mic = FALSE;
    bool mute_speaker = FALSE;

    if(message->mute_states & AUDIO_MUTE_MASK(audio_mute_group_main))
        mute_speaker = TRUE;

    if(message->mute_states & AUDIO_MUTE_MASK(audio_mute_group_mic))
        mute_mic = TRUE;

    /* Notify DSP if mute mode needs updating */
    audioVoiceCommonConfigureMute(mute_mic, mute_speaker, ctx);

    /* Update HFP state(s). This must always be after mute is configured */
    ctx->mic_muted = mute_mic;
    ctx->speaker_muted = mute_speaker;
}
