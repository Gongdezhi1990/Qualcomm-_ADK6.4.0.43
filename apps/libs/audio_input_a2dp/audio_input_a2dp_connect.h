/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_connect.h

DESCRIPTION
    Handler for AUDIO_PLUGIN_CONNECT_MSG message for a2dp source.
*/

#ifndef AUDIO_INPUT_A2DP_CONNECT_H_
#define AUDIO_INPUT_A2DP_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

/****************************************************************************
DESCRIPTION
    AUDIO_PLUGIN_CONNECT_MSG message handler.
    It creates Kymera chain and connect it to source and audio_mixer.
*/
void audioInputA2dpConnectHandler(audio_input_context_t *ctx, Task task, const AUDIO_PLUGIN_CONNECT_MSG_T *msg);

#endif /* AUDIO_INPUT_A2DP_CONNECT_H_ */
