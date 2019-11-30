/****************************************************************************
Copyright (c) 2017-2018 Qualcomm Technologies International, Ltd.


FILE NAME
    audio_output_tws_connect.c

DESCRIPTION
    TWS functionality
*/
#include <audio_plugin_forwarding.h>
#include <tws_packetiser.h>

#include "audio_output_tws.h"
#include "audio_output_tws_connect.h"
#include "audio_output_tws_chain.h"
#include "audio_output_tws_packetiser.h"
#include "audio_output_tws_ctx.h"

void AudioOutputTwsConnect(Task output_task, const AUDIO_PLUGIN_FORWARDING_CREATE_REQ_T* req)
{
    audio_plugin_forwarding_ports_t ports;
    Source forwarding_source = req->params.forwarding.codec_source;
    audio_plugin_forwarding_status_t status = audio_output_success;
    
    AudioOutputTwsSetInputTask(req->input_task);

    if(req->params.transcode.codec != audio_codec_none)
    {
        AudioOutputTwsChainCreate(&req->params); 
        forwarding_source = AudioOutputTwsChainGetForwardingSource();
    }
    
    if(!AudioOutputTwsPacketiserInit(output_task, forwarding_source, &req->params))
    {
        status = audio_output_failed_packetiser;
        AudioOutputTwsChainDestroy();
    }
    
    ports.left_source = AudioOutputTwsChainGetLeftSource();
    ports.right_source = AudioOutputTwsChainGetRightSource();
    
    AudioPluginForwardingCreateCfm(output_task, req->input_task, &ports, status);
}

void AudioOutputTwsStart(Task output_task, const AUDIO_PLUGIN_FORWARDING_START_REQ_T* req)
{
    AudioOutputTwsChainStart();
    AudioPluginForwardingStartCfm(output_task, req->input_task);
}

void AudioOutputTwsStop(Task output_task, const AUDIO_PLUGIN_FORWARDING_STOP_REQ_T* req)
{
    AudioOutputTwsChainStop();
    AudioPluginForwardingStopCfm(output_task, req->input_task);
}

void AudioOutputTwsDestroy(Task output_task, const AUDIO_PLUGIN_FORWARDING_DESTROY_REQ_T* req)
{
    AudioOutputTwsChainDestroy();
    AudioOutputTwsPacketiserDestroy();
    AudioPluginForwardingDestroyCfm(output_task, req->input_task);
    AudioOutputTwsSetInputTask(NULL);
}

#ifdef HOSTED_TEST_ENVIRONMENT
void AudioOutputTwsConnectTestReset(void)
{
    AudioOutputTwsChainDestroy();
    AudioOutputTwsPacketiserDestroy();
}
#endif
