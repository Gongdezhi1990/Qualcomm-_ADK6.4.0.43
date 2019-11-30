/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#ifndef HFP_SHIM_LAYER_H
#define HFP_SHIM_LAYER_H

#include <hfp.h>


#define HFP_SHIM_MESSAGE_BASE
typedef enum
{
    HFP_LINK_GET_BDADDR_CFM = HFP_MESSAGE_TOP,
    HFP_LINK_GET_SLC_SINK_CFM,
    HFP_LINK_GET_AUDIO_SINK_CFM,
    HFP_LINK_GET_CALL_STATE_CFM
}HfpShimMessageId ;

typedef struct
{
    bool   success;
    bdaddr bd_addr;
} HFP_LINK_GET_BDADDR_CFM_T;

typedef struct
{
    bool success;
    Sink sink;
} HFP_LINK_GET_SLC_SINK_CFM_T;

typedef struct
{
    bool success;
    Sink sink;
} HFP_LINK_GET_AUDIO_SINK_CFM_T;

typedef struct
{
    bool           success;
    hfp_call_state call_state;
} HFP_LINK_GET_CALL_STATE_CFM_T;

void HfpHandleComplexMessage(Task task, MessageId id, Message message);


void HfpInitTestExtra(Task theAppTask, hfp_profile supported_profile, uint16 supported_features, hfp_wbs_codec_mask supported_wbs_codecs, 
                      hfp_indicator_status service, hfp_indicator_status signal_strength, hfp_indicator_status roaming_status, hfp_indicator_status battery_charge,
                      bool disable_nrec, bool extended_errors, bool multipoint, uint16 size_extra_indicators, const uint8* extra_indicators);


void HfpInitTestExtraNull(Task theAppTask);


void HfpAudioConnectResponseTestExtraDefault(hfp_link_priority priority, bool response, sync_pkt_type packet_type);


void HfpAudioConnectResponseTestExtraParams(hfp_link_priority priority, bool response, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, sync_air_coding voice_settings, sync_retx_effort retx_effort);


void HfpAudioTransferConnectionTestExtraDefault(hfp_link_priority priority, hfp_audio_transfer_direction direction, sync_pkt_type packet_type );


void HfpAudioTransferConnectionTestExtraParams(hfp_link_priority priority, hfp_audio_transfer_direction direction, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, sync_air_coding voice_settings, sync_retx_effort retx_effort);


void HfpRouteAudioToPcm(Sink sco_sink);


void HfpLinkGetBdaddrTestExtra(hfp_link_priority priority);


void HfpLinkGetSlcSinkTestExtra(hfp_link_priority priority);


void HfpLinkGetAudioSinkTestExtra(hfp_link_priority priority);


void HfpLinkGetCallStateTestExtra(hfp_link_priority priority);

#endif
