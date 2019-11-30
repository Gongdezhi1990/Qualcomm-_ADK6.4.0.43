/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_voice_common_messages.h

DESCRIPTION
    Definitions of internal messages
*/

#ifndef _AUDIO_VOICE_COMMON_MESSAGES_H_
#define _AUDIO_VOICE_COMMON_MESSAGES_H_

#include <message.h>

#define AUDIO_VOICE_COMMON_MESSAGE_BASE    1

typedef enum
{
    AUDIO_VOICE_CONNECT_ERROR = AUDIO_VOICE_COMMON_MESSAGE_BASE,
    AUDIO_VOICE_COMMON_MESSAGE_TOP
} audio_voice_common_message_t;

#endif /* _AUDIO_VOICE_COMMON_MESSAGES_H_ */
