/******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast.h

DESCRIPTION
    Broadcast audio plugin when the device is in the receiver role.
*/


/*!
@file    audio_input_broadcast.h
@brief   Broadcast audio plugin when the device is in the receiver role.

         When this plugin is connected it creates a broadcast_packetiser and
         an audio graph. The packetiser is connected to the incoming CSB stream
         from the broadcaster, extracts the audio packets and forwards them
         onto the input of the audio graph.

         The incoming CSB stream may also contain control data which the
         packetiser will extract and forward to this plugin. The control data
         is handled by this plugin and acted on as required.

         This plugin is only supported in the kymera audio framework.

         This plugin implements the audio plugin interface defined in
         audio_plugin_if.h. It should only ever be used via the audio library,
         i.e. the functions in this plugin should never be called directly
         from application code.
*/


#ifndef AUDIO_INPUT_BROADCAST_
#define AUDIO_INPUT_BROADCAST_

/*!
    @brief Top-level message handler for the plugin.
*/
void AudioInputBroadcastMessageHandler(Task task, MessageId id, Message message);

#endif /* AUDIO_INPUT_BROADCAST_ */
