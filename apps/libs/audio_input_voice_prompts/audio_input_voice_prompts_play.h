/****************************************************************************
Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_audio_prompts_play.h

DESCRIPTION
    Handler functions for voice prompts and tones source messages.
*/

#ifndef LIBS_AUDIO_INPUT_AUDIO_PROMPTS_AUDIO_INPUT_AUDIO_PROMPTS_PLAY_H_
#define LIBS_AUDIO_INPUT_AUDIO_PROMPTS_AUDIO_INPUT_AUDIO_PROMPTS_PLAY_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

#include "audio_input_voice_prompts_dsp_if.h"
#include "audio_input_voice_prompts_defs.h"

vp_context_t *VoicePromptsGetContext(void);

void AudioInputVoicePromptsPluginPlayPhrase(FILE_INDEX prompt_index, FILE_INDEX prompt_header_index, int16 no_dsp_prompt_volume ,
    AudioPluginFeatures features, Task app_task);

void AudioInputVoicePromptsPluginPlayTone (const ringtone_note * tone, int16 tone_volume, AudioPluginFeatures features);

void AudioInputVoicePromptsPluginStopPhrase (void);

void AudioInputVoicePromptsPluginHandleStreamDisconnect(void);

#endif /* LIBS_AUDIO_INPUT_AUDIO_PROMPTS_AUDIO_INPUT_AUDIO_PROMPTS_PLAY_H_ */
