/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_tws_connect.h

DESCRIPTION
    Handler for AUDIO_PLUGIN_CONNECT_MSG message for tws source.
*/

#ifndef AUDIO_INPUT_TWS_CONNECT_H_
#define AUDIO_INPUT_TWS_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

void AudioInputTwsConnectHandler(Task task, Message message, audio_input_context_t *ctx);

void AudioInputTwsFadeOutCompleteHandler(Task task, Message message, audio_input_context_t *ctx);

#ifdef HOSTED_TEST_ENVIRONMENT
void AudioInputTwsTestReset(void);
#endif

#endif /* AUDIO_INPUT_TWS_CONNECT_H_ */
