/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_plugin_forwarding.h

DESCRIPTION
    Audio plugin interface between audio input and forwarding output
*/

#ifndef AUDIO_PLUGIN_FORWARDING_H_
#define AUDIO_PLUGIN_FORWARDING_H_

#include <library.h>
#include <audio_data_types.h>
#include <audio_plugin_music_params.h>

typedef enum
{
    AUDIO_PLUGIN_FORWARDING_CREATE_REQ = AUDIO_PLUGIN_FORWARDING_BASE,
    AUDIO_PLUGIN_FORWARDING_CREATE_CFM,
    AUDIO_PLUGIN_FORWARDING_START_REQ,
    AUDIO_PLUGIN_FORWARDING_START_CFM,
    AUDIO_PLUGIN_FORWARDING_STOP_REQ,
    AUDIO_PLUGIN_FORWARDING_STOP_CFM,
    AUDIO_PLUGIN_FORWARDING_DESTROY_REQ,
    AUDIO_PLUGIN_FORWARDING_DESTROY_CFM,
    AUDIO_PLUGIN_FORWARDING_STREAMING_IND,
    AUDIO_PLUGIN_FORWARDING_VOLUME_CHANGE_IND,
    AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND,
    AUDIO_PLUGIN_FORWARDING_TOP
} audio_plugin_forwarding_message_t;

typedef enum
{
    audio_output_success,
    audio_output_failed_packetiser
} audio_plugin_forwarding_status_t;

typedef struct
{
    Source left_source;
    Source right_source;
} audio_plugin_forwarding_ports_t;

typedef struct
{
    Source codec_source;
    audio_plugin_forwarding_ports_t pcm;
} audio_plugin_forwarding_streams_t;

typedef struct
{
    unsigned bitpool;
    unsigned settings;
} audio_plugin_sbc_encode_params_t;

typedef struct
{
    audio_codec_t codec;
    union
    {
        audio_plugin_sbc_encode_params_t sbc;
    } params;
} audio_plugin_forwarding_transcode_t;

typedef struct
{
    audio_codec_t source_codec;
    bool content_protection;
    uint32 sample_rate;
    Sink forwarding_sink;
    audio_plugin_forwarding_streams_t forwarding;
    audio_plugin_forwarding_transcode_t transcode;
    ttp_latency_t ttp_latency;
} audio_plugin_forwarding_params_t;

typedef struct
{
    Task input_task;
    audio_plugin_forwarding_params_t params;
} AUDIO_PLUGIN_FORWARDING_CREATE_REQ_T;

typedef struct
{
    Task output_task;
    audio_plugin_forwarding_status_t status;
    audio_plugin_forwarding_ports_t ports;
} AUDIO_PLUGIN_FORWARDING_CREATE_CFM_T;

typedef struct
{
    Task input_task;
} AUDIO_PLUGIN_FORWARDING_START_REQ_T;

typedef struct
{
    Task output_task;
} AUDIO_PLUGIN_FORWARDING_START_CFM_T;

typedef struct
{
    Task input_task;
} AUDIO_PLUGIN_FORWARDING_STOP_REQ_T;

typedef struct
{
    Task output_task;
} AUDIO_PLUGIN_FORWARDING_STOP_CFM_T;

typedef struct
{
    Task input_task;
} AUDIO_PLUGIN_FORWARDING_DESTROY_REQ_T;

typedef struct
{
    Task output_task;
} AUDIO_PLUGIN_FORWARDING_DESTROY_CFM_T;

typedef struct
{
    bool muted;
} AUDIO_PLUGIN_FORWARDING_SOFT_MUTE_IND_T;

/*!
    @brief Send message to the output plugin to prepare for forwarding
*/
void AudioPluginForwardingCreate(Task input_task, Task output_task, const audio_plugin_forwarding_params_t* params);

/*!
    @brief Send message to the input plugin with ports to connect for forwarding
*/
void AudioPluginForwardingCreateCfm(Task output_task, Task input_task, const audio_plugin_forwarding_ports_t* ports, audio_plugin_forwarding_status_t status);

/*!
    @brief Send message to the output plugin to start forwarding
*/
void AudioPluginForwardingStart(Task input_task, Task output_task);

/*!
    @brief Send message to the input plugin to confirm forwarding has started
*/
void AudioPluginForwardingStartCfm(Task output_task, Task input_task);

/*!
    @brief Send message to the output plugin to stop forwarding
*/
void AudioPluginForwardingStop(Task input_task, Task output_task);

/*!
    @brief Send message to the input plugin to confirm forwarding has stopped
*/
void AudioPluginForwardingStopCfm(Task output_task, Task input_task);

/*!
    @brief Send message to the output plugin to release all resources
*/
void AudioPluginForwardingDestroy(Task input_task, Task output_task);

/*!
    @brief Send message to the input plugin to confirm all resources have been
    released.
*/
void AudioPluginForwardingDestroyCfm(Task output_task, Task input_task);

/*!
    @brief Send message to the input plugin to indicate that streaming has started.
*/
void AudioPluginForwardingStreamingInd(Task input_task);

/*!
    @brief Send message to the output plugin to indicate that volume has changed.
*/
void AudioPluginForwardingVolumeChangeInd(Task output_task);

/*!
    @brief Send message to the output plugin to indicate that the stream has been muted.
*/
void AudioPluginForwardingSoftMuteInd(Task output_task, bool muted);


#endif /* AUDIO_PLUGIN_FORWARDING_H_ */
