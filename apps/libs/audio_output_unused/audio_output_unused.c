/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_unused.c

DESCRIPTION
    Dummy implementation for cases where an output plug-in is not required
*/

#include <vmtypes.h>
#include <stdlib.h>
#include <audio_output_unused.h>

Task AudioPluginOutputGetTwsPluginTask(audio_plugin_output_mode_t mode)
{
    UNUSED(mode);
    return NULL;
}

Task AudioPluginOutputGetBroadcastPluginTask(void)
{
    return (Task)NULL;
}
