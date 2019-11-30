/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_analogue_connect.h
 
DESCRIPTION
    Handler for AUDIO_PLUGIN_CONNECT_MSG message for wired analogue source.
*/

#ifndef AUDIO_INPUT_ANALOGUE_CONNECT_H_
#define AUDIO_INPUT_ANALOGUE_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

void AudioInputAnalogueConnectHandler(Task task, Message message, audio_input_context_t* ctx);

void AudioInputAnalogueConnect(Task task, Message message, audio_input_context_t* ctx);

#endif /* AUDIO_INPUT_ANALOGUE_CONNECT_H_ */
