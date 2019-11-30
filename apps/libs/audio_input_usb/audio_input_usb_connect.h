/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_usb_connect.h
 
DESCRIPTION
    Handler for AUDIO_PLUGIN_CONNECT_MSG message for usb source.
*/

#ifndef AUDIO_INPUT_USB_CONNECT_H_
#define AUDIO_INPUT_USB_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

void AudioInputUsbConnectHandler(Task task, Message message, audio_input_context_t* ctx);

void audioInputUsbConnectToMixer(Task task, Message message, audio_input_context_t* ctx);

void audioInputUsbEnableForwardingMonoOutput(audio_input_context_t* ctx);

void audioInputUsbDisableForwardingMonoOutput(audio_input_context_t* ctx);
#endif /* AUDIO_INPUT_USB_CONNECT_H_ */
