/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

*/


#ifndef AGHFP_SHIM_LAYER_H
#define AGHFP_SHIM_LAYER_H

#include "aghfp.h"

typedef struct
{
    uint16  task;			
} AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND_TEST_EXTRA_T;

typedef enum
{
    AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND_TEST_EXTRA = AGHFP_MESSAGE_TOP
} AghfpShimMessageId;



void AghfpHandleComplexMessage(Task task, MessageId id, Message message);

void AghfpSlcConnectTestExtra(AGHFP *aghfp, const bdaddr *addr);
void AghfpSlcConnectResponseTestExtra(AGHFP *aghfp, bool response, const bdaddr *addr);

void AghfpAudioConnectTestExtra(AGHFP *aghfp, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs);
void AghfpAudioConnectTestExtraDefaults(AGHFP *aghfp, sync_pkt_type packet_type);
void AghfpAudioConnectResponseTestExtra(AGHFP *aghfp, bool response, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs);
void AghfpAudioConnectResponseTestExtraDefaults(AGHFP *aghfp, bool response, sync_pkt_type packet_type);
void AghfpAudioTransferConnectionTestExtraDefault(AGHFP *aghfp, aghfp_audio_transfer_direction direction, sync_pkt_type packet_type);
void AghfpAudioTransferConnectionTestExtraParams(AGHFP *aghfp, aghfp_audio_transfer_direction direction, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs);
void AghfpSetAudioParamsTestExtra(AGHFP *aghfp, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort, bool override_wbs);
void AghfpSetAudioParamsTestExtraDefault(AGHFP *aghfp, sync_pkt_type packet_type);

void AghfpSendCallerIdTestExtra(AGHFP *aghfp, uint8 type_number, uint16 size_number, uint16 size_string, uint16 size_data, const uint8 *data);
void AghfpSendCallWaitingNotificationTestExtra(AGHFP *aghfp, uint8 type_number, uint16 size_number, uint16 size_string, uint16 size_data, const uint8 *data);

void AghfpSendSubscriberNumberTestExtra(AGHFP *aghfp, uint8 id, uint8 type, uint8 service, uint16 size_number, uint8 *number);
void AghfpSendCurrentCallTestExtra(AGHFP *aghfp, uint8 idx, aghfp_call_dir dir, aghfp_call_state status, aghfp_call_mode mode, aghfp_call_mpty mpty, uint8 type, uint16 size_number, uint8 *number);

void AghfpCallCreateAudioTestExtra (AGHFP *aghfp, aghfp_call_dir direction, bool in_band, sync_pkt_type packet_type, uint32 bandwidth, uint16 max_latency, uint16 voice_settings, sync_retx_effort retx_effort);
void AghfpCallCreateAudioTestExtraDefaults (AGHFP *aghfp, aghfp_call_dir direction, bool in_band, sync_pkt_type packet_type);

void AghfpRouteAudioToPcm(Sink sco_sink);


#endif
