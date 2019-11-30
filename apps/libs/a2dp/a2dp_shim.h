/* Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd. */
/*  */
#ifndef A2DP_SHIM_LAYER_H
#define A2DP_SHIM_LAYER_H


#include <a2dp.h>


typedef enum
{
    A2DP_CODEC_SETTINGS_IND_TEST_EXTRA  = A2DP_MESSAGE_TOP
} A2dpShimMessageId;


typedef struct
{
    A2DP                   *a2dp;
    uint32                  rate;
    a2dp_channel_mode       channel_mode;
    uint8                   seid;
    uint8                   content_protection;
    uint32                  voice_rate;
    uint16                  bitpool;
    uint16                  format;
    uint16                  packet_size;
} A2DP_CODEC_SETTINGS_IND_TEST_EXTRA_T;


void A2dpHandleComplexMessage(Task task, MessageId id, Message message);

void A2dpInitTestExtraAppselect(Task theAppTask, uint16 linkloss_timeout);

void A2dpStartKalimbaStreaming(const A2DP* a2dp, uint16 media_sink);
void A2dpStopKalimbaStreaming(void);
void A2dpInitTestExtraDefault( Task theAppTask, uint8 role, bool enable_mp3 );

void A2dpConnectTestExtra(bdaddr *addr);
void A2dpConnectResponseTestExtra(uint16 device_id, bool accept);
void A2dpDisconnectTestExtra(uint16 device_id);

void A2dpOpenTestExtra(uint16 device_id);
void A2dpOpenResponseTestExtra(uint16 device_id, bool accept);
void A2dpCloseTestExtra(uint16 device_id, uint16 stream_id);
void A2dpMediaStartTestExtra(uint16 device_id, uint16 stream_id);
void A2dpMediaStartResponseTestExtra(uint16 device_id, uint16 stream_id, bool accept);
void A2dpMediaSuspendTestExtra(uint16 device_id, uint16 stream_id);
void A2dpMediaAvSyncDelayResponseTestExtra(uint16 device_id, uint8 stream_id, uint16 delay);

void A2dpReconfigureTestExtra(uint16 device_id, uint16 stream_id, uint16 size_sep_caps, uint8 *sep_caps);

void A2dpSendMediaPacketTestExtra(uint16 device_id, uint16 stream_id);

#endif
