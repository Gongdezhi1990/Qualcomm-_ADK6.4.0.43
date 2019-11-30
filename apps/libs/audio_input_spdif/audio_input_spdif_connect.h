/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_spdif_connect.h

DESCRIPTION
    Handler for AUDIO_PLUGIN_CONNECT_MSG message for spdif source.
*/

#ifndef AUDIO_INPUT_SPDIF_CONNECT_H_
#define AUDIO_INPUT_SPDIF_CONNECT_H_

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_input_common.h>

void AudioInputSpdifConnectHandler(audio_input_context_t *ctx, Task task,
                                   const AUDIO_PLUGIN_CONNECT_MSG_T *msg);

#endif /* AUDIO_INPUT_SPDIF_CONNECT_H_ */
