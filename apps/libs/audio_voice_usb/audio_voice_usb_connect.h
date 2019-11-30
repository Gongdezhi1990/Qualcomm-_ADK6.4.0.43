/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_usb_connect.h

DESCRIPTION
    Audio input USB connect functions.
*/

#ifndef _AUDIO_INPUT_USB_CONNECT_H_
#define _AUDIO_INPUT_USB_CONNECT_H_

#include <message.h>
#include <audio_voice_common.h>

void AudioInputUsbConnectAndFadeIn(Task task, Message msg, audio_voice_context_t* ctx);

#endif /* _AUDIO_INPUT_USB_CONNECT_H_ */
