/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

 
FILE NAME
    audio_output_tws_ctx.c
 
DESCRIPTION
    Set and get the input task.
*/

#include "audio_output_tws_ctx.h"

static Task input_task;

void AudioOutputTwsSetInputTask(Task task)
{
    input_task = task;
}

Task AudioOutputTwsGetInputTask(void)
{
    return input_task;
}
