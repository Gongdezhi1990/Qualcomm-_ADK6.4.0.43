/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_task.h

DESCRIPTION
    Helper to get data from the plugin task
*/

#ifndef AUDIO_INPUT_A2DP_TASK_H_
#define AUDIO_INPUT_A2DP_TASK_H_

#include <message.h>
#include <audio_data_types.h>

audio_codec_t AudioPluginA2dpTaskGetCodec(Task task);
bool AudioPluginA2dpTaskIsBroadcaster(Task task);

#endif /* AUDIO_INPUT_A2DP_TASK_H_ */
