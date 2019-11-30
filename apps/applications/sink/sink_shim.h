/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
#ifndef SINK_SHIM_LAYER_H
#define SINK_SHIM_LAYER_H

#include <headset.h>
#include <headset_private.h>

#define SINK_SHIM_MESSAGE_BASE
typedef enum
{
    SINK_DUMMY_MESSAGE_TEST_EXTRA = SINK_MESSAGE_TOP
}SinkShimMessageId;

void HeadsetHandleComplexMessage(Task task, MessageId id, Message message);


#endif /* SINK_SHIM_LAYER */
