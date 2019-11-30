/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_ttp_latency.c

DESCRIPTION
    Latency related messages for a2dp source.
*/

#ifndef AUDIO_INPUT_A2DP_TTP_LATENCY_H_
#define AUDIO_INPUT_A2DP_TTP_LATENCY_H_

#include <audio_input_common.h>

void audioInputA2dpSendLatencyUpdateToAppTask(audio_input_context_t* ctx, uint16 target_ttp_in_tenths_of_ms);

#endif /* AUDIO_INPUT_A2DP_TTP_LATENCY_H_ */
