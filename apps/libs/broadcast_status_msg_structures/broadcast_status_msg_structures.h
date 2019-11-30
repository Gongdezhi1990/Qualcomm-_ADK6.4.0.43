/****************************************************************************
//  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 
****************************************************************************/

#ifndef BROADCAST_STATUS_MSG_STRUCTURES_H
#define BROADCAST_STATUS_MSG_STRUCTURES_H

#include <erasure_code_input_stats.h>

typedef struct _broadcast_status_broadcaster
{
    /* Audio output */
    unsigned int audio_output_status;
    /* TTP generator */
    signed  int latency_error_ms;
    /* Current TTP extension value (16-bit) */
    unsigned int ttp_extension;
} broadcast_status_broadcaster_t;

typedef struct _broadcast_status_receiver
{
    /* Audio output */
    unsigned int audio_output_status;
    /* CSB input */
    unsigned int csb_input_packets_received;
    unsigned int csb_input_mac_failures;
    unsigned int csb_input_sample_rate;
    unsigned int csb_input_volume;
    unsigned int csb_input_invalid_packets;
    /*EC input */
    ec_input_stats_t ec_input_stats;
} broadcast_status_receiver_t;

#endif
