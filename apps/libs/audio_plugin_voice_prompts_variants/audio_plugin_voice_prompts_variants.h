/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_plugin_voice_prompts_variants.h

DESCRIPTION
    Definitions of voice prompts.
*/

#ifndef AUDIO_INPUT_VOICE_PROMPTS_H_
#define AUDIO_INPUT_VOICE_PROMPTS_H_

#include <csrtypes.h>
#include <message.h>
#include <file.h>

extern const TaskData csr_voice_prompts_plugin;

bool CsrVoicePromptsIsMixable(FILE_INDEX prompt_header_index);
void CsrVoicePromptsPluginCleanup(void);
void freePhraseData(void);

#endif /* AUDIO_INPUT_VOICE_PROMPTS_H_ */
