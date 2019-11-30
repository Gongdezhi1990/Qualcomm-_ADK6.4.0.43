/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_voice_prompts_defs.h
 
DESCRIPTION
    Definitaion of Voice Prompts Context Structure 
*/

#ifndef LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_DEFS_H_
#define LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_DEFS_H_

#include "audio_plugin_if.h"

/* Opaque handle to a tone/prompt chain instance. */
typedef struct __vp_data_t* vp_handle_t;

/*!
    @brief Context data for this plugin.
 */
typedef struct
{
    const ringtone_note*    tone;
    FILE_INDEX              prompt_index;
    FILE_INDEX              prompt_header_index;
    voice_prompts_codec_t   codec_type;
    bool                    stereo;
    uint16                  playback_rate;
    int16                   prompt_volume;
    AudioPluginFeatures     features;
    Source                  source;
    Task                    app_task;
    vp_handle_t             chain;
} vp_context_t ;

#define TONE_SAMPLE_RATE 8000


#endif /* LIBS_AUDIO_INPUT_VOICE_PROMPTS_AUDIO_INPUT_VOICE_PROMPTS_DEFS_H_ */
