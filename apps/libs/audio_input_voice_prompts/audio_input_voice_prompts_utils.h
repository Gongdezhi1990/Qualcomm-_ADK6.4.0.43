/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_voice_prompts_utils.h
 
DESCRIPTION
    Utility functions.
*/

#ifndef LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_UTILS_H_
#define LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_UTILS_H_

#include <csrtypes.h>
#include "audio_plugin_if.h"

/****************************************************************************
DESCRIPTION
    Validates codec type.
*/
bool VoicePromptsIsCodecTypeValid(voice_prompts_codec_t codec_type);

/****************************************************************************
DESCRIPTION
    Check if it is a tone.
*/
bool VoicePromptsIsItTone(voice_prompts_codec_t codec_type);

/****************************************************************************
DESCRIPTION
    Register for messages from given sink.
*/
void VoicePromptsRegisterForMessagesFromSink(Sink sink);

/****************************************************************************
DESCRIPTION
    Deregister from messages from given sink.
*/
void VoicePromptsDeregisterForMessagesFromSink(Sink sink);

#endif /* LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_UTILS_H_ */
