/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_assistant.h

DESCRIPTION
    Audio plugin lib for voice assistant .
*/

#ifndef AUDIO_VOICE_ASSISTANT_H_
#define AUDIO_VOICE_ASSISTANT_H_

#include <message.h>

/*the task message handler*/
void audioVaMessageHandler (Task task, MessageId id, Message message);

#endif /* AUDIO_VOICE_ASSISTANT_H_ */

