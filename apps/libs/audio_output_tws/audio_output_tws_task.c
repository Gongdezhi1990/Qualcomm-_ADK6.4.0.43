/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_task.c

DESCRIPTION
    Task implementation
*/

#include <audio_plugin_output_variants.h>
#include "audio_output_tws_task.h"
#include "audio_output_tws.h"

static TaskData tws_master_plugin = {AudioOutputTwsMessageHandler};
static TaskData share_me_master_plugin = {AudioOutputTwsMessageHandler};

Task AudioPluginOutputGetTwsPluginTask(audio_plugin_output_mode_t mode)
{
    if(mode == audio_plugin_output_mode_share_me)
        return &share_me_master_plugin;
    
    return &tws_master_plugin;
}

bool AudioOutputTwsTaskIsTws(Task task)
{
    return (task == &tws_master_plugin);
}

bool AudioOutputTwsTaskIsShareMe(Task task)
{
    return (task == &share_me_master_plugin);
}

Task AudioPluginOutputGetBroadcastPluginTask(void)
{
    return (Task)NULL;
}
