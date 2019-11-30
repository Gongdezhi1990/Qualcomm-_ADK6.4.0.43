/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_usb.h

DESCRIPTION
    Interface file for the audio input usb plug-in.
*/

#ifndef _AUDIO_INPUT_USB_H_
#define _AUDIO_INPUT_USB_H_

#include <message.h>
#include <audio_plugin_if.h>

/*******************************************************************************
DESCRIPTION
    Audio input HFP plug-in message handler.
*/
void AudioVoiceUsbMessageHandler(Task task, MessageId id, Message message);


#endif /* _AUDIO_INPUT_USB_H_ */
