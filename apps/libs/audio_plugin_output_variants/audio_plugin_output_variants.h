/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_plugin_output_variants.h
 
DESCRIPTION
    Definitions of output plug-ins
*/

#ifndef AUDIO_PLUGIN_OUTPUT_VARIANTS_H_
#define AUDIO_PLUGIN_OUTPUT_VARIANTS_H_

#include <csrtypes.h>
#include <message.h>

typedef enum
{
    audio_plugin_output_mode_tws,
    audio_plugin_output_mode_share_me
} audio_plugin_output_mode_t;

Task AudioPluginOutputGetTwsPluginTask(audio_plugin_output_mode_t mode);

Task AudioPluginOutputGetBroadcastPluginTask(void);

#endif /* AUDIO_PLUGIN_OUTPUT_VARIANTS_H_ */
