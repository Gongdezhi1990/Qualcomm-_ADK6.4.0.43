/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_a2dp_ttp_latency.c

DESCRIPTION
    Latency related messages for a2dp source.
*/

#include "audio_input_a2dp_ttp_latency.h"
#include <audio.h>

void audioInputA2dpSendLatencyUpdateToAppTask(audio_input_context_t* ctx, uint16 target_ttp_in_tenths_of_ms)
{
    MAKE_AUDIO_MESSAGE(AUDIO_LATENCY_REPORT, message);
    message->latency = target_ttp_in_tenths_of_ms;
    message->estimated = TRUE;
    message->sink = StreamSinkFromSource(ctx->left_source);
    MessageSend(ctx->app_task, AUDIO_LATENCY_REPORT, message);
}
