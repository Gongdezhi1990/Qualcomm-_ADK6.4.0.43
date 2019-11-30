/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_voice_hfp_connect.h

DESCRIPTION
    Audio voice HFP connect functions.
*/

#ifndef _AUDIO_VOICE_HFP_CONNECT_H_
#define _AUDIO_VOICE_HFP_CONNECT_H_

#include <message.h>
#include <audio_voice_common.h>

void AudioVoiceHfpConnectAndFadeIn(Task task, Message msg, audio_voice_context_t* ctx);

#endif /* _AUDIO_VOICE_HFP_CONNECT_H_ */
