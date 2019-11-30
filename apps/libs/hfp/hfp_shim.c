/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include <panic.h>
#include <message.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <vm.h>
#include <sink.h>
#include <source.h>
#include <stream.h>

#include "hfp_shim.h"
#include "hfp.h"
#include "hfp_private.h"


void HfpHandleComplexMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);

    Panic();
}


void HfpInitTestExtra(Task theAppTask, hfp_profile supported_profile, uint16 supported_features, hfp_wbs_codec_mask supported_wbs_codecs, 
                      hfp_indicator_status service, hfp_indicator_status signal_strength, hfp_indicator_status roaming_status, hfp_indicator_status battery_charge,
                      bool disable_nrec, bool extended_errors, bool multipoint, uint16 size_extra_indicators, const uint8* extra_indicators)
{
    hfp_init_params config;
    char* l_extra_indicators;
    
    config.supported_profile                   = supported_profile;
    config.supported_features                  = supported_features;
    config.supported_wbs_codecs                = supported_wbs_codecs;
    config.optional_indicators.service         = service;
    config.optional_indicators.signal_strength = signal_strength;
    config.optional_indicators.roaming_status  = roaming_status;
    config.optional_indicators.battery_charge  = battery_charge;
    config.disable_nrec                        = disable_nrec;
    config.extended_errors                     = extended_errors;
    config.multipoint                          = multipoint;
    
    if(size_extra_indicators)
    {
        l_extra_indicators = PanicUnlessMalloc(size_extra_indicators);
        memmove(l_extra_indicators, extra_indicators, size_extra_indicators);
    }
    else
    {
        l_extra_indicators = NULL;
    }
    
    HfpInit(theAppTask, &config, l_extra_indicators);
}


void HfpInitTestExtraNull(Task theAppTask)
{
    HfpInit(theAppTask, NULL, NULL);
}


void HfpAudioConnectResponseTestExtraDefault(hfp_link_priority priority, bool response, sync_pkt_type packet_type)
{
    HfpAudioConnectResponse(priority, response, packet_type, NULL, FALSE);
}


void HfpAudioConnectResponseTestExtraParams(hfp_link_priority priority, bool response, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, sync_air_coding voice_settings, sync_retx_effort retx_effort)
{
    hfp_audio_params audio_params;
    audio_params.bandwidth      = bandwidth;
    audio_params.max_latency    = max_latency;
    audio_params.voice_settings = voice_settings;
    audio_params.retx_effort    = retx_effort;
    HfpAudioConnectResponse(priority, response, packet_type, &audio_params, FALSE);
}


void HfpAudioTransferConnectionTestExtraDefault(hfp_link_priority priority, hfp_audio_transfer_direction direction, sync_pkt_type packet_type )
{
    HfpAudioTransferRequest(priority, direction, packet_type, NULL);
}


void HfpAudioTransferConnectionTestExtraParams(hfp_link_priority priority, hfp_audio_transfer_direction direction, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, sync_air_coding voice_settings, sync_retx_effort retx_effort)
{
    hfp_audio_params audio_params;
    audio_params.bandwidth      = bandwidth;
    audio_params.max_latency    = max_latency;
    audio_params.voice_settings = voice_settings;
    audio_params.retx_effort    = retx_effort;
    HfpAudioTransferRequest(priority, direction, packet_type, &audio_params);
}


void HfpRouteAudioToPcm(Sink sco_sink)
{
    Sink audio_sink_0 = StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_0, AUDIO_CHANNEL_A_AND_B);
    Sink audio_sink_1 = StreamAudioSink(AUDIO_HARDWARE_CODEC, AUDIO_INSTANCE_1, AUDIO_CHANNEL_A_AND_B);
    Source audio_source_0 = StreamSourceFromSink(audio_sink_0);
    Source audio_source_1 = StreamSourceFromSink(audio_sink_1);
    Source sco_source     = StreamSourceFromSink(sco_sink);
    
    if (!SinkIsValid(sco_sink) || !SourceIsValid(sco_source))
        Panic();

    StreamDisconnect(audio_source_0, audio_sink_0); 
    StreamDisconnect(audio_source_1, audio_sink_1);

    (void) PanicFalse(SinkConfigure(audio_sink_0, STREAM_CODEC_OUTPUT_RATE, 8000));
    (void) PanicFalse(SourceConfigure(audio_source_0, STREAM_CODEC_INPUT_RATE, 8000));
    
    (void) PanicFalse(StreamConnect(sco_source, audio_sink_0));
    (void) PanicFalse(StreamConnect(audio_source_0, sco_sink));
}


void HfpLinkGetBdaddrTestExtra(hfp_link_priority priority)
{
    MAKE_HFP_MESSAGE(HFP_LINK_GET_BDADDR_CFM);
    message->success = HfpLinkGetBdaddr(priority, &message->bd_addr);
    MessageSend(theHfp->clientTask, HFP_LINK_GET_BDADDR_CFM, message);
}


void HfpLinkGetSlcSinkTestExtra(hfp_link_priority priority)
{
    MAKE_HFP_MESSAGE(HFP_LINK_GET_SLC_SINK_CFM);
    message->success = HfpLinkGetSlcSink(priority, &message->sink);
    MessageSend(theHfp->clientTask, HFP_LINK_GET_SLC_SINK_CFM, message);
}


void HfpLinkGetAudioSinkTestExtra(hfp_link_priority priority)
{
    MAKE_HFP_MESSAGE(HFP_LINK_GET_AUDIO_SINK_CFM);
    message->success = HfpLinkGetAudioSink(priority, &message->sink);
    MessageSend(theHfp->clientTask, HFP_LINK_GET_AUDIO_SINK_CFM, message);
}


void HfpLinkGetCallStateTestExtra(hfp_link_priority priority)
{
    MAKE_HFP_MESSAGE(HFP_LINK_GET_CALL_STATE_CFM);
    message->success = HfpLinkGetCallState(priority, &message->call_state);
    MessageSend(theHfp->clientTask, HFP_LINK_GET_CALL_STATE_CFM, message);
}
