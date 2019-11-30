/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_hfp.h

DESCRIPTION
    Interface file for the audio voice hfp plug-in.
*/

#ifndef _AUDIO_VOICE_HFP_H_
#define _AUDIO_VOICE_HFP_H_

#include <message.h>
#include <audio_plugin_if.h>

/*******************************************************************************
DESCRIPTION
    Audio voice HFP plug-in message handler.
*/
void AudioVoiceHfpMessageHandler(Task task, MessageId id, Message message);


#endif /* _AUDIO_VOICE_HFP_H_ */
