/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_i2s.h
 
DESCRIPTION
    Audio plugin for wired i2s source.
*/

#ifndef AUDIO_INPUT_I2S_H_
#define AUDIO_INPUT_I2S_H_

#include <message.h>

void AudioPluginI2sMessageHandler(Task task, MessageId id, Message message);

#endif /* AUDIO_INPUT_I2S_H_ */
