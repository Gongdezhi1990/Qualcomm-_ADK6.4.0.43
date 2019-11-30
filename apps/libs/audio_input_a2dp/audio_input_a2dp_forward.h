/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_forward.h

DESCRIPTION
    Handler for AUDIO_PLUGIN_START_FORWARDING_MSG message for a2dp source.
*/

#ifndef AUDIO_INPUT_A2DP_FORWARD_H_
#define AUDIO_INPUT_A2DP_FORWARD_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

void audioInputA2dpForwardCreate(Task task, const AUDIO_PLUGIN_START_FORWARDING_MSG_T *req, audio_input_context_t *ctx);

void audioInputA2dpForwardStart(Task task, audio_input_context_t *ctx, const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T *cfm);

void AudioPluginA2dpForwardEnableForwardingOutput(Task task, audio_input_context_t* ctx);

void audioInputA2dpForwardDestroy(Task task, audio_input_context_t *ctx);

void AudioPluginA2dpForwardHandleDestroyCfm(audio_input_context_t *ctx);

#endif /* AUDIO_INPUT_A2DP_FORWARD_H_ */
