/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_i2s_connect.h
 
DESCRIPTION
    Handler for AUDIO_PLUGIN_CONNECT_MSG message for wired i2s source.
*/

#ifndef AUDIO_INPUT_I2S_CONNECT_H_
#define AUDIO_INPUT_I2S_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

void AudioInputI2sConnectHandler(Task task, Message message, audio_input_context_t* ctx);
void AudioInputI2sConnect(Task task, Message message, audio_input_context_t* ctx);
void AudioInputI2sDisconnect(Task task, Message message, audio_input_context_t* ctx);

#endif /* AUDIO_INPUT_I2S_CONNECT_H_ */
