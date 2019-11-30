/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_voice_prompts_file.h
 
DESCRIPTION
    Voice prompts file related functions.
*/

#ifndef LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_FILE_H_
#define LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_FILE_H_

#include <csrtypes.h>
#include "audio_plugin_if.h"
#include "audio_input_voice_prompts_defs.h"

typedef struct
{
    voice_prompts_codec_t codec_type;
    uint16              playback_rate;
    bool                stereo;
} voice_prompt_t;

/****************************************************************************
DESCRIPTION
    Set up prompt structure content using voice prompt header file pointed by
    file_index.
*/
void VoicePromptsFileSetProperties(FILE_INDEX file_index, voice_prompt_t* prompt);

/****************************************************************************
DESCRIPTION
    Get tone or voice prompt source and set up prompt structure.
*/
Source VoicePromptsFileGetPrompt(const vp_context_t *context, voice_prompt_t* prompt);

#endif /* LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_FILE_H_ */
