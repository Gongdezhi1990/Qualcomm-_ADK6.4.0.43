/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#include <panic.h>
#include <message.h>
#include <source.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h> /* for memset */
#include <vm.h>
#include <sink.h>
#include <source.h>
#include <stream.h>

#include "aghfp_shim.h"


void AghfpHandleComplexMessage(Task task, MessageId id, Message message)
{
	switch (id)
	{
		case AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND:
        {
            AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND_TEST_EXTRA_T *newmsg = malloc(sizeof(AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND_TEST_EXTRA_T)); 
            newmsg->task = (uint16) task;
            MessageSend(task, AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND_TEST_EXTRA, newmsg);
        }
        break;
			
		default:
		printf("Id = %x\n", id);
		Panic();
		break;
	}
}

void AghfpSlcConnectTestExtra(AGHFP *aghfp, const bdaddr *addr)
{
	AghfpSlcConnect(aghfp, addr);
}

void AghfpSlcConnectResponseTestExtra(AGHFP *aghfp, bool response, const bdaddr *addr)
{
	AghfpSlcConnectResponse(aghfp, response);
}

void AghfpAudioConnectTestExtra(AGHFP *aghfp, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs)
{
	aghfp_audio_params params;
	params.bandwidth = bandwidth;
	params.max_latency = max_latency;
	params.voice_settings = voice_settings;
	params.retx_effort = retx_effort;
	params.override_wbs = override_wbs;
	AghfpAudioConnect(aghfp, packet_type, &params);
}


void AghfpAudioConnectTestExtraDefaults(AGHFP *aghfp, sync_pkt_type packet_type)
{
	AghfpAudioConnect(aghfp, packet_type, NULL);
}


void AghfpAudioConnectResponseTestExtra(AGHFP *aghfp, bool response, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs)
{
	aghfp_audio_params params;
	params.bandwidth = bandwidth;
	params.max_latency = max_latency;
	params.voice_settings = voice_settings;
	params.retx_effort = retx_effort;
	params.override_wbs = override_wbs;
	AghfpAudioConnectResponse(aghfp, response, packet_type, &params);
}


void AghfpAudioConnectResponseTestExtraDefaults(AGHFP *aghfp, bool response, sync_pkt_type packet_type)
{
	AghfpAudioConnectResponse(aghfp, response, packet_type, NULL);
}


void AghfpSetAudioParamsTestExtra(AGHFP *aghfp, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs)
{
	aghfp_audio_params params;
	params.bandwidth = bandwidth;
	params.max_latency = max_latency;
	params.voice_settings = voice_settings;
	params.retx_effort = retx_effort;
	params.override_wbs = override_wbs;
	
	AghfpSetAudioParams(aghfp, packet_type, &params);
}


void AghfpSetAudioParamsTestExtraDefault(AGHFP *aghfp, sync_pkt_type packet_type)
{
	AghfpSetAudioParams(aghfp, packet_type, NULL);
}


void AghfpAudioTransferConnectionTestExtraDefault(AGHFP *aghfp, aghfp_audio_transfer_direction direction, sync_pkt_type packet_type)
{
    AghfpAudioTransferConnection(aghfp, direction, packet_type, NULL);
}


void AghfpAudioTransferConnectionTestExtraParams(AGHFP *aghfp, aghfp_audio_transfer_direction direction, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs)
{
    aghfp_audio_params audio_params;
    audio_params.bandwidth = bandwidth;
    audio_params.max_latency = max_latency;
    audio_params.voice_settings = voice_settings;
    audio_params.retx_effort = retx_effort;
	audio_params.override_wbs = override_wbs;
    AghfpAudioTransferConnection(aghfp, direction, packet_type, &audio_params);
}

void AghfpSendCallerIdTestExtra(AGHFP *aghfp, uint8 type_number, uint16 size_number, uint16 size_string, uint16 size_data, const uint8 *data)
{
    const uint8 *string = NULL;

    /* Unused in shim only needed for rfcli */
    size_data = size_data;

	if (size_string > 0) string = data + size_number;

	AghfpSendCallerId(aghfp, type_number, size_number, data, size_string, string);
}


void AghfpSendCallWaitingNotificationTestExtra(AGHFP *aghfp, uint8 type_number, uint16 size_number, uint16 size_string, uint16 size_data, const uint8 *data)
{
	const uint8 *string = NULL;
    /* Unused in shim only needed for rfcli */
    size_data = size_data;

	if (size_string > 0) string = data + size_number;

	AghfpSendCallWaitingNotification(aghfp, type_number, size_number, data, size_string, string);
}


void AghfpSendSubscriberNumberTestExtra(AGHFP *aghfp, uint8 id, uint8 type, uint8 service, uint16 size_number, uint8 *number)
{
    aghfp_subscriber_info sub;
  	sub.type = type;
   	sub.service = service;
   	sub.size_number = size_number;
   	sub.number = number;

   	AghfpSendSubscriberNumber(aghfp, &sub);
}


void AghfpSendCurrentCallTestExtra(AGHFP *aghfp, uint8 idx, aghfp_call_dir dir, aghfp_call_state status, aghfp_call_mode mode, aghfp_call_mpty mpty, uint8 type, uint16 size_number, uint8 *number)
{
    aghfp_call_info call;
   	call.idx = idx;
   	call.dir = dir;
   	call.status = status;
   	call.mode = mode;
   	call.mpty = mpty;
  	call.type = type;
   	call.size_number = size_number;
   	call.number = number;

   	AghfpSendCurrentCall(aghfp, &call);
}


void AghfpCallCreateAudioTestExtra (AGHFP *aghfp, aghfp_call_dir direction, bool in_band, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort)
{
	aghfp_audio_params params;
	params.bandwidth = bandwidth;
	params.max_latency = max_latency;
	params.voice_settings = voice_settings;
	params.retx_effort = retx_effort;
	
	AghfpCallCreateAudio(aghfp, direction, in_band, packet_type, &params);
}


void AghfpCallCreateAudioTestExtraDefaults (AGHFP *aghfp, aghfp_call_dir direction, bool in_band, sync_pkt_type packet_type)
{
	AghfpCallCreateAudio(aghfp, direction, in_band, packet_type, 0);
}


void AghfpRouteAudioToPcm(Sink sco_sink)
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
