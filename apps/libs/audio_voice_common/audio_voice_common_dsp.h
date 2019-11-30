/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_common_dsp.h

DESCRIPTION
    Interface header for DSP operations.
*/

#ifndef _AUDIO_VOICE_COMMON_DSP_H_
#define _AUDIO_VOICE_COMMON_DSP_H_

#include <audio_plugin_if.h>
#include <audio_plugin_voice_variants.h>
#include <audio_voice_common.h>
#include <message.h>

/*******************************************************************************
DESCRIPTION
    Power off the DSP.
*/
void AudioVoiceCommonDspPowerOff(void);

/*******************************************************************************
DESCRIPTION
    Create the voice chain and fade in.
*/
void AudioVoiceCommonDspCreateChainAndFadeIn(Task task, audio_voice_context_t* ctx, operator_filters_t* filters);

/*******************************************************************************
DESCRIPTION
    Send fade out request to the audio mixer
*/
void AudioVoiceCommonDspFadeOut(Task task);

/*******************************************************************************
DESCRIPTION
    Stop all DSP activity.
*/
void AudioVoiceCommonDspDestroyChain(audio_voice_context_t* ctx);


#endif /* _AUDIO_VOICE_COMMON_DSP_H_ */
