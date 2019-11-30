/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_voice_prompt_file.c
 
DESCRIPTION
     Voice prompts file related functions.
*/
#include <stdio.h>
#include <string.h>

#include <source.h>
#include <file.h>

#include "print.h"

#include "audio_input_voice_prompts_file.h"
#include "audio_input_voice_prompts_utils.h"

#define SIZE_PROMPT_DATA   (12)

/****************************************************************************
DESCRIPTION
    Get voice prompt source.
*/
static Source getVoicePromptSource(FILE_INDEX file_index)
{
    return StreamFileSource(file_index);
}

/****************************************************************************
DESCRIPTION
    Set up prompt structure content using voice prompt header file pointed by
    file_index.
*/
void VoicePromptsFileSetProperties(FILE_INDEX file_index, voice_prompt_t* prompt)
{
    Source source = NULL;
    const uint8* rx_array;

    source = StreamFileSource(file_index);

    /* Check source created successfully */
    if(SourceSize(source) < SIZE_PROMPT_DATA)
    {
        /* Finished with header source, close it */
        SourceClose(source);
        Panic();
    }

    rx_array = SourceMap(source);

    prompt->stereo        = rx_array[4];
    prompt->codec_type    = rx_array[9];
    prompt->playback_rate = (uint16)(((uint16)rx_array[10] << 8) | (rx_array[11]));

    if(!VoicePromptsIsCodecTypeValid(prompt->codec_type))
    {
        Panic();
    }

    /* Finished with header source, close it */
    SourceClose(source);
}

/****************************************************************************
DESCRIPTION
    Get tone or voice prompt source and set up prompt structure.
*/
Source VoicePromptsFileGetPrompt(const vp_context_t * pData, voice_prompt_t* prompt)
{
    Source audio_source;

    if(!pData || !prompt )
        return NULL;

    VoicePromptsFileSetProperties(pData->prompt_header_index, prompt);
    audio_source = getVoicePromptSource(pData->prompt_index);
    
    return audio_source;
}


