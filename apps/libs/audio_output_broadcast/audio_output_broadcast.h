/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_output_broadcast.h

DESCRIPTION
    Broadcast implementation of audio_plugin_forwarding interface.
*/


/*!
@file    audio_output_broadcast.h
@brief   Broadcast implementation of audio_plugin_forwarding interface.
         It handles broadcast_packetiser.
*/


#ifndef AUDIO_OUTPUT_BROADCAST_
#define AUDIO_OUTPUT_BROADCAST_

#include <message.h>

void AudioOutputBroadcastMessageHandler(Task task, MessageId id, Message message);


#endif /* AUDIO_OUTPUT_BROADCAST_ */
