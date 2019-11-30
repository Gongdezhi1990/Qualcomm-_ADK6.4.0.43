/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_usb_broadcast.c

DESCRIPTION
    Implementation of handlers for broadcast specific messages.
*/

#include <audio.h>
#include <audio_plugin_forwarding.h>
#include <audio_plugin_output_variants.h>

#include "audio_input_usb_broadcast.h"
#include "audio_input_usb_chain_config.h"


void audioInputUsbBroadcastCreate(Task input_task, Task output_task, audio_input_context_t *ctx)
{
    audio_plugin_forwarding_params_t params;
    params.sample_rate = ctx->sample_rate;
    params.forwarding.pcm.left_source = ChainGetOutput(ctx->chain, path_pcm_left);
    params.forwarding.pcm.right_source = ChainGetOutput(ctx->chain, path_pcm_right);
    AudioPluginForwardingCreate(input_task, output_task, &params);
}

void audioInputUsbBroadcastStart(Task input_task, Message message, audio_input_context_t *ctx)
{
    const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T *cfm = (const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T *)message;

    if(cfm->status == audio_output_success)
    {
        audio_mixer_connect_t mixer_config;

        mixer_config.left_src = cfm->ports.left_source;
        mixer_config.right_src = cfm->ports.right_source;
        mixer_config.connection_type = CONNECTION_TYPE_MUSIC_BA_TX;
        mixer_config.sample_rate = ctx->sample_rate;
        mixer_config.channel_mode = CHANNEL_MODE_STEREO;
        mixer_config.variable_rate = FALSE;

        ctx->mixer_input = AudioMixerConnect(&mixer_config);
        PanicFalse(ctx->mixer_input != audio_mixer_input_error_none);

        AudioPluginForwardingStart(input_task, cfm->output_task);
    }
}

void audioInputUsbBroadcastStartChain(Task input_task, Message message, audio_input_context_t* ctx)
{
    UNUSED(message);
    ChainStart(ctx->chain);
    AudioInputCommonConnect(ctx, input_task);
}

void audioInputUsbBroadcastStop(Task input_task, audio_input_context_t *ctx)
{
    SetAudioBusy(input_task);
    AudioPluginForwardingStop(input_task, ctx->ba.plugin);
}

void audioInputUsbBroadcastDestroy(Task input_task, Task output_task, audio_input_context_t *ctx)
{
    if(ctx->chain)
        ChainStop(ctx->chain);
    StreamDisconnect(ctx->left_source, NULL);
    StreamDisconnect(ctx->right_source, NULL);

    if(AudioMixerDisconnect(ctx->mixer_input) == AUDIO_MIXER_DISCONNECT_ERROR)
        Panic();

    AudioPluginForwardingDestroy(input_task, output_task);
}
