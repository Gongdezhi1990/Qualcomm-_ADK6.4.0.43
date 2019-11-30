/****************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_instance.c

DESCRIPTION
    Audio instance library
*/

#include "audio_instance.h"
#include <panic.h>
#include <stdlib.h>
#include <string.h>

struct audio_instance_tag
{
    Task task;
    Sink sink;
    AUDIO_SINK_T sink_type;
};

typedef struct audio_instance_tag audio_instance_private_t;

#define MAXIMUM_AUDIO_INSTANCES     3

static audio_instance_private_t audio_instances[MAXIMUM_AUDIO_INSTANCES];

static bool instanceIsEmpty(uint8 index)
{
    return (audio_instances[index].sink == NULL
                && audio_instances[index].task == NULL);
}

static audio_instance_t storeInstanceAtIndex(uint8 index, Task task, Sink sink, AUDIO_SINK_T sink_type)
{
    audio_instances[index].task = task;
    audio_instances[index].sink = sink;
    audio_instances[index].sink_type = sink_type;
    return &audio_instances[index];
}

audio_instance_t AudioInstanceCreate(Task task, Sink sink, AUDIO_SINK_T sink_type)
{
    uint8 instance_index = 0;
    PanicNull(task);

    while(instance_index < MAXIMUM_AUDIO_INSTANCES)
    {
        if(instanceIsEmpty(instance_index))
        {
            return storeInstanceAtIndex(instance_index, task, sink, sink_type);
        }
        instance_index++;
    }
    return (audio_instance_t)NULL;
}

void AudioInstanceDestroy(audio_instance_t instance)
{
    PanicNull(instance);
    instance->sink = (Sink)NULL;
    instance->task = (Task)NULL;
    instance->sink_type = AUDIO_SINK_INVALID;
}

Sink AudioInstanceGetSink(audio_instance_t instance)
{
    PanicNull(instance);
    return (instance ? instance->sink : (Sink)NULL);
}

Task AudioInstanceGetPluginTask(audio_instance_t instance)
{
    PanicNull(instance);
    return (instance ? instance->task : (Task)NULL);
}

AUDIO_SINK_T AudioInstanceGetSinkType(audio_instance_t instance)
{
    PanicNull(instance);
    return instance->sink_type;
}

audio_instance_t AudioInstanceGetInstanceFromTask(Task task)
{
    if(task)
    {
        unsigned i;

        for(i = 0; i < MAXIMUM_AUDIO_INSTANCES; i++)
        {
            if(audio_instances[i].task == task)
            {
                return &audio_instances[i];
            }
        }
    }
    return (audio_instance_t)NULL;
}

#ifdef HOSTED_TEST_ENVIRONMENT
void AudioInstanceTestReset(void)
{
    memset(audio_instances, 0, sizeof(audio_instances));
}
#endif
