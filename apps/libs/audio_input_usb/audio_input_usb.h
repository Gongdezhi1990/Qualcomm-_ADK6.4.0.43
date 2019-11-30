/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_usb.h
 
DESCRIPTION
    Audio plugin for usb source.
*/

#ifndef AUDIO_INPUT_USB_H_
#define AUDIO_INPUT_USB_H_

#include <message.h>

void AudioPluginUsbMessageHandler(Task task, MessageId id, Message message);

#endif /* AUDIO_INPUT_USB_H_ */
