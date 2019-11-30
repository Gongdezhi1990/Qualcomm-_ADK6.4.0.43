/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_plugin_forwarding.c

DESCRIPTION
    Forwarding functionality
*/
#include <stdlib.h>

#include <message.h>
#include <audio_plugin_if.h>
#include <audio_mixer.h>
#include <vmtypes.h>
#include <print.h>

#include "audio_plugin_forwarding.h"
#include "audio.h"

void AudioPluginForwardingCreate(Task input_task, Task output_task, const audio_plugin_forwarding_params_t* params)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_CREATE_REQ\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_CREATE_REQ, message);
    message->input_task = input_task;
    message->params = *params;
    MessageSend(output_task, AUDIO_PLUGIN_FORWARDING_CREATE_REQ, message);
}

void AudioPluginForwardingCreateCfm(Task output_task, Task input_task, const audio_plugin_forwarding_ports_t* ports, audio_plugin_forwarding_status_t status)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_CREATE_CFM\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_CREATE_CFM, message);
    message->output_task = output_task;
    message->ports = *ports;
    message->status = status;
    MessageSend(input_task, AUDIO_PLUGIN_FORWARDING_CREATE_CFM, message);
}

void AudioPluginForwardingStart(Task input_task, Task output_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_START_REQ\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_START_REQ, message);
    message->input_task = input_task;
    MessageSend(output_task, AUDIO_PLUGIN_FORWARDING_START_REQ, message);
}

void AudioPluginForwardingStartCfm(Task output_task, Task input_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_START_CFM\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_START_CFM, message);
    message->output_task = output_task;
    MessageSend(input_task, AUDIO_PLUGIN_FORWARDING_START_CFM, message);
}

void AudioPluginForwardingStop(Task input_task, Task output_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_STOP_REQ\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_STOP_REQ, message);
    message->input_task = input_task;
    MessageSend(output_task, AUDIO_PLUGIN_FORWARDING_STOP_REQ, message);
}

void AudioPluginForwardingStopCfm(Task output_task, Task input_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_STOP_CFM\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_STOP_CFM, message);
    message->output_task = output_task;
    MessageSend(input_task, AUDIO_PLUGIN_FORWARDING_STOP_CFM, message);
}

void AudioPluginForwardingDestroy(Task input_task, Task output_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_DESTROY_REQ\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_DESTROY_REQ, message);
    message->input_task = input_task;
    MessageSend(output_task, AUDIO_PLUGIN_FORWARDING_DESTROY_REQ, message);
}

void AudioPluginForwardingDestroyCfm(Task output_task, Task input_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_DESTROY_CFM\n"));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_DESTROY_CFM, message);
    message->output_task = output_task;
    MessageSend(input_task, AUDIO_PLUGIN_FORWARDING_DESTROY_CFM, message);
}

void AudioPluginForwardingStreamingInd(Task input_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_STREAMING_IND\n"));

    MessageSend(input_task, AUDIO_PLUGIN_FORWARDING_STREAMING_IND, NULL);
}

void AudioPluginForwardingVolumeChangeInd(Task output_task)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_VOLUME_CHANGE_IND\n"));

    MessageSend(output_task, AUDIO_PLUGIN_FORWARDING_VOLUME_CHANGE_IND, NULL);
}

void AudioPluginForwardingSoftMuteInd(Task output_task, bool muted)
{
    PRINT(("AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND %d\n", muted));

    MAKE_AUDIO_MESSAGE(AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND, message);
    message->muted = muted;
    MessageSend(output_task, AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND, message);
}
