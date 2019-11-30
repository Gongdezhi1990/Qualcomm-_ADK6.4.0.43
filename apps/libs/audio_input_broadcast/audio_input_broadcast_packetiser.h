/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast_packetiser.h

DESCRIPTION
    A minimal prototype implementation of a broadcast_packetiser client.
*/


/*!
@file    audio_input_csb_packetiser.h
@brief   A minimal prototype implementation of a broadcast_packetiser client.

         It does the minimum required to extract the audio frames from an
         incoming CSB stream and send them to the input of the audio graph.
*/

#ifndef AUDIO_INPUT_BROADCAST_PACKETISER_H_
#define AUDIO_INPUT_BROADCAST_PACKETISER_H_


#include <message.h>
#include <sink.h>
#include <source.h>

#include "audio_input_common.h"


/*! @brief Handle Broadcast Packetiser Library messages. 
 *
    @param task [IN] Task to which the message has been sent.
    @param id [IN] Message type identifier.
    @param message [IN] Message contents.
 */
void audioInputBroadcastPacketiserHandleMessage(Task task, MessageId id, Message message, audio_input_context_t *ctx);

/*! @brief Create a new packetiser for CSB receive. 

    @param csb_receive_source [IN] CSB Stream source of broadcast packets.
    @param audio_frame_sink [IN] Kymera Stream sink for audio frames and metadata.
    @param encr [IN] Encryption configuration details.
 */ 
void audioInputBroadcastPacketiserCreate(Source csb_receive_source, Sink audio_frame_sink);

void audioInputBroadcastPacketiserUpdateSink(Sink audio_frame_sink);

/*! @brief Teardown a packetiser.

    @param audio_frame_sink [IN] Kymera Stream sink for audio frames and metadata, for unmapping.
 */ 
void audioInputBroadcastPacketiserDestroy(Sink audio_frame_sink);

/*! @brief Check if a message id should be handled by
           appHandleBroadcastPacketiserMessage.
*/
bool audioInputBroadcastPacketiserIsBroadcastPacketiserMessageId(MessageId id);

#endif /* AUDIO_INPUT_BROADCAST_PACKETISER_H_ */
