/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast_connect.h

DESCRIPTION
    Handlers for when connecting or disconnecting the plugin.
*/


/*!
@file    audio_input_broadcast_connect.h
@brief   Handlers for when connecting or disconnecting the plugin.

         When connecting the plugin create the resources it needs,
         e.g. broadcast_packetiser, audio graph.

         When disconnecting, release the resources whilst avoiding creating
         unwanted audio artefacts.
*/


#ifndef AUDIO_INPUT_BROADCAST_CONNECT_H_
#define AUDIO_INPUT_BROADCAST_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

/*!
    @brief audioInputBroadcastConnectHandler

    @param task
    @param message
    @param ctx
*/
void audioInputBroadcastConnectHandler(Task task, Message message, audio_input_context_t *ctx);

/*!
    @brief audioInputBroadcastFadeOutCompleteHandler
    @param task
    @param message
    @param ctx
*/
void audioInputBroadcastFadeOutCompleteHandler(Task task, Message message, audio_input_context_t *ctx);

/*!
    @brief audioInputBroadcastConnectSetSampleRate
    @param ctx
    @param rate
    @param codec_config
*/
void audioInputBroadcastConnectReconfigureDecoder(audio_input_context_t *ctx, uint32 rate, codec_config_celt *codec_config);

#ifdef HOSTED_TEST_ENVIRONMENT
/* Reset the plugin to disconnected state. For unit test use only */
void audioInputBroadcastTestReset(void);
#endif

#endif /* AUDIO_INPUT_BROADCAST_CONNECT_H_ */
