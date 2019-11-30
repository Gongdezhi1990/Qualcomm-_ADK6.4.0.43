/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_input_common_forward.c

DESCRIPTION
    Handler for AUDIO_PLUGIN_START_FORWARDING_MSG message
*/

#include <audio.h>
#include <audio_config.h>
#include <audio_plugin_forwarding.h>
#include <audio_mixer.h>
#include <operators.h>

#include "audio_config.h"
#include "audio_input_common.h"
#include "audio_input_common_music_processing.h"

void AudioInputCommonForwardStart(Task task, Message message, audio_input_context_t* ctx)
{
    const AUDIO_PLUGIN_START_FORWARDING_MSG_T* req = (const AUDIO_PLUGIN_START_FORWARDING_MSG_T*)message;
    
    audio_plugin_forwarding_params_t params;
    params.source_codec = audio_codec_pcm;
    params.forwarding.codec_source = NULL;
    params.forwarding.pcm.left_source = ctx->left_source;
    params.forwarding.pcm.right_source = ctx->right_source;
    params.forwarding_sink = req->forwarding_sink;
    params.content_protection = req->content_protection;
    params.sample_rate = ctx->sample_rate;
    params.transcode.codec = audio_codec_sbc;
    params.transcode.params.sbc.bitpool = ctx->tws.encoder_bitpool;
    params.transcode.params.sbc.settings = ctx->tws.encoder_settings;
    params.ttp_latency = AudioConfigGetTWSTtpLatency();
    AudioPluginForwardingCreate(task, req->output_plugin, &params);
    
    ChainStop(ctx->chain);
    
    if(AudioMixerDisconnect(ctx->mixer_input) == AUDIO_MIXER_DISCONNECT_ERROR)
        Panic();
    
    ctx->tws.plugin = req->output_plugin;
    
    SetAudioBusy(task);
}

void AudioInputCommonForwardHandleCreateCfm(Task task, Message message, audio_input_context_t* ctx)
{
    audio_mixer_connect_t connect_data;
    const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T* cfm = (const AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T*)message;

    if(cfm->status != audio_output_success)
    {
        AudioInputCommmonSendError(task);
        return;
    }
    
    connect_data.left_src = cfm->ports.left_source;
    connect_data.right_src = cfm->ports.right_source;
    connect_data.connection_type = CONNECTION_TYPE_MUSIC;
    connect_data.sample_rate = ctx->sample_rate;
    connect_data.channel_mode = AudioConfigGetTwsChannelModeLocal();

    ctx->mixer_input = AudioMixerConnect(&connect_data);
    AudioPluginForwardingStart(task, cfm->output_task);
    audioInputCommonSetMusicProcessing(ctx, ctx->music_processing, ctx->music_enhancements);
}

void AudioInputCommonForwardHandleStartCfm(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(task);
    UNUSED(message);
    ctx->tws.mute_until_start_forwarding = FALSE;
    AudioMixerFadeIn(ctx->mixer_input, NULL);
    SetAudioBusy(NULL);
}

void AudioInputCommonForwardStop(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(message);
    
    AudioPluginForwardingStop(task, ctx->tws.plugin);
    SetAudioBusy(task);
}

void AudioInputCommonForwardDestroy(Task task, Message message, audio_input_context_t* ctx)
{
    UNUSED(message);
    
    if(ctx->chain)
        ChainStop(ctx->chain);
    StreamDisconnect(ctx->left_source, NULL);
    StreamDisconnect(ctx->right_source, NULL);
    
    if(AudioMixerDisconnect(ctx->mixer_input) == AUDIO_MIXER_DISCONNECT_ERROR)
        Panic();
    
    AudioPluginForwardingDestroy(task, ctx->tws.plugin);
}
