/****************************************************************************
Copyright (c) 2016-2018 Qualcomm Technologies International, Ltd.

FILE NAME
    audio_input_broadcast_private.h

DESCRIPTION
    Broadcast audio input private definitions.
*/

#ifndef AUDIO_INPUT_BROADCAST_PRIVATE_H
#define AUDIO_INPUT_BROADCAST_PRIVATE_H


#include <audio_input_common.h>
#include <broadcast_packetiser.h>
#include <erasure_coding.h>
#include <rtime.h>


typedef struct __ba_receiver_plugin_context_t
{
    Task app_task;
    audio_input_context_t audio_ctx;
    rtime_sample_rate_t received_sample_rate;
} ba_receiver_plugin_context_t;


ba_receiver_plugin_context_t *audioInputBroadcastGetContext(void);

broadcast_packetiser_t *audioInputBroadcastGetPacketiser(void);

ec_handle_rx_t audioInputBroadcastGetErasureCoding(void);

#endif /* AUDIO_INPUT_BROADCAST_PRIVATE_H */
