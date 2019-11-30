/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_common_states.h

DESCRIPTION
    State machine API
*/

#ifndef AUDIO_VOICE_COMMON_STATES_H_
#define AUDIO_VOICE_COMMON_STATES_H_

#include <message.h>

#include "audio_voice_common.h"

void AudioVoiceCommonHandleEvent(Task task, audio_voice_event_t event, Message payload, audio_voice_context_t* ctx);

#endif /* AUDIO_VOICE_COMMON_STATES_H_ */
