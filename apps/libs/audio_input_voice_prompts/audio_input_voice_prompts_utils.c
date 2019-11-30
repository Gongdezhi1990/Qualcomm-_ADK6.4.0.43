/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_voice_prompts_utils.c
 
DESCRIPTION
    Utility functions.
*/
#include <stdlib.h>

#include <vmal.h>
#include <message.h>
#include <print.h>

#include "audio_input_voice_prompts_utils.h"

extern const TaskData csr_voice_prompts_plugin;

/****************************************************************************
DESCRIPTION
    Validates codec type.
*/
bool VoicePromptsIsCodecTypeValid(voice_prompts_codec_t codec_type)
{
    bool is_codec_type_valid = FALSE;
    switch(codec_type)
    {
        case voice_prompts_codec_pcm:
        case voice_prompts_codec_pcm_8khz:
        case voice_prompts_codec_tone:
        case voice_prompts_codec_sbc:
            is_codec_type_valid = TRUE;
            break;
        case voice_prompts_codec_ima_adpcm:
        case voice_prompts_codec_mp3:
        case voice_prompts_codec_aac:
            is_codec_type_valid = FALSE;
            break;
        default:
        break;
    }

    return is_codec_type_valid;
}

/****************************************************************************
DESCRIPTION
    Check if it is a tone.
*/
bool VoicePromptsIsItTone(voice_prompts_codec_t codec_type)
{
    if(voice_prompts_codec_tone == codec_type)
    {
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

/****************************************************************************
DESCRIPTION
    Register for messages from given sink.
*/
void VoicePromptsRegisterForMessagesFromSink(Sink sink)
{
#ifndef DEBUG_PRINT_ENABLED
    MessageStreamTaskFromSink(sink , (TaskData*) &csr_voice_prompts_plugin);
#else
    Task taskdata = MessageStreamTaskFromSink(sink , (TaskData*) &csr_voice_prompts_plugin);
    PRINT(("VP: 0x%p sink task now 0x%p was 0x%p.\n",(void*)sink, (void*)&csr_voice_prompts_plugin, (void*)taskdata));
#endif
}

/****************************************************************************
DESCRIPTION
    Deregister from messages from given sink.
*/
void VoicePromptsDeregisterForMessagesFromSink(Sink sink)
{
#ifndef DEBUG_PRINT_ENABLED
    MessageStreamTaskFromSink(sink, NULL);
#else
    Task taskdata = MessageStreamTaskFromSink(sink, NULL);
    PRINT(("VP: 0x%p sink task now NULL was 0x%p\n",(void*)sink, (void*)taskdata));
    UNUSED(taskdata);
#endif
}
