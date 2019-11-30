/****************************************************************************
//  Copyright (c) 2017 Qualcomm Technologies International, Ltd.
 Part of BroadcastADK 4.0
****************************************************************************/

#ifndef BROADCAST_MSG_INTERFACE_H
#define BROADCAST_MSG_INTERFACE_H

/*! VM->DSP message to set the sample rate of the broadcast */
#define KALIMBA_MSG_AUDIO_SAMPLE_RATE       0x3000
/*! DSP->VM message to indicate the received sample rate has changed */
#define KALIMBA_MSG_CSB_SAMPLE_RATE_CHANGED 0x3001
/*! VM->DSP message */
#define KALIMBA_MSG_LED_COLOUR              0x3002
/*! DSP->VM message to provide status information to the VM */
#define KALIMBA_MSG_AUDIO_STATUS            0x3003
/*! DSP->VM message to indicate change in volume received from broadcaster */
#define KALIMBA_MSG_VOLUME_IND              0x3009
/*! VM->DSP message to set the latency */
#define KALIMBA_MSG_SET_LATENCY             0x3010
/*! VM->DSP message to set the volume level */
#define KALIMBA_MSG_SET_VOLUME              0x3011
/*! VM->DSP message to request some random bits from firmware */
#define KALIMBA_MSG_RANDOM_BITS_REQ         0x3012
/*! DSP->VM message supplying the requested random bits */
#define KALIMBA_MSG_RANDOM_BITS_RESP        0x3013
/*! VM->DSP message to set a new CSB encryption key */
#define KALIMBA_MSG_SET_KEY                 0x3014
/*! VM->DSP message to set a new CSB variant IV */
#define KALIMBA_MSG_SET_IV                  0x3015
/*! VM->DSP message to set a new CSB invariant IV */
#define KALIMBA_MSG_SET_FIXED_IV            0x3016
/*! DSP->VM A long message whose data is of type broadcast_status_broadcaster_t */
#define KALIMBA_MSG_BROADCAST_STATUS        0x3017
/*! VM->DSP message to set the TTP extension value. */
#define KALIMBA_MSG_SET_TTP_EXTENSION       0x3018
/*! VM->DSP message */
#define KALIMBA_MSG_BROADCAST_CONFIG        0x3019
/*! VM->DSP message to set CSB parameters */
#define KALIMBA_MSG_SET_CSB_TIMING          0x301A
/*! VM->DSP message to set the stream ID. */
#define KALIMBA_MSG_SET_STREAM_ID           0x3020
/*! VM->DSP message to set the CELT codec configuration. */
#define KALIMBA_MSG_SET_CELT_CONFIG         0x3021
/*! VM<->DSP message to indicate a AFH channel map change is pending. */
#define KALIMBA_MSG_AFH_CHANNEL_MAP_CHANGE_PENDING 0x3022
/*! VM->DSP message to set the content protection header. */
#define KALIMBA_MSG_SET_CONTENT_PROTECTION  0x3023 

/*! VM->DSP message */
#define KALIMBA_MSG_SET_SCM_SEGMENT_REQ     0x3100
/*! DSP->VM message */
#define KALIMBA_MSG_SET_SCM_SEGMENT_CFM     0x3101
/*! DSP->VM message */
#define KALIMBA_MSG_SET_SCM_SEGMENT_IND     0x3102
/*! DSP->VM message */
#define KALIMBA_MSG_SCM_SEGMENT_IND         0x3103
/*! DSP->VM message */
#define KALIMBA_MSG_SCM_SEGMENT_EXPIRED     0x3104
/*! VM->DSP message */
#define KALIMBA_MSG_SCM_SHUTDOWN_REQ        0x3110
/*! DSP->VM message */
#define KALIMBA_MSG_SCM_SHUTDOWN_CFM        0x3111

#endif /* BROADCAST_MSG_INTERFACE_H */
