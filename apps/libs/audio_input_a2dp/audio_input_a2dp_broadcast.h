/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_input_a2dp_broadcast.h
 
DESCRIPTION
    Handlers for broadcast specific messages.
*/

#ifndef AUDIO_INPUT_A2DP_BROADCAST_H_
#define AUDIO_INPUT_A2DP_BROADCAST_H_

#include <audio_input_common.h>
#include <audio_plugin_forwarding.h>

void audioInputA2dpBroadcastCreate(Task input_task, Task output_task, audio_input_context_t *ctx);
void audioInputA2dpBroadcastStart(Task input_task, audio_input_context_t *ctx, const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T *cfm);
void audioInputA2dpBroadcastDestroy(Task input_task, Task output_task, audio_input_context_t *ctx);

#endif /* AUDIO_INPUT_A2DP_BROADCAST_H_ */
