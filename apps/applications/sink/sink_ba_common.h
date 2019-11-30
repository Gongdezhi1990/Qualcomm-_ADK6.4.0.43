/*******************************************************************************
Copyright (c) 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_ba_common.h

DESCRIPTION
    Common Header File used in broadcast Audio. This header file is used by
    sink_ba/sink_boradcaster/sink_receiver and sink_ba_plugin. It is used to 
    have common defines and other data which is shared between these sub modules
    This file is not used to expose any interfaces.
*/

#ifndef _SINK_BA_COMMON_H_
#define _SINK_BA_COMMON_H_

#include <scm.h>
#include <broadcast.h>
#include <ps.h>


/*! Size in words of the encryption key data stored in PS. */
#define ENCR_KEY_SIZE       9
/*! Size in words of the fixed IV data stored in PS. */
#define ENCR_FIXED_IV_SIZE  3
/*! Maximum volume step in BA mode */
#define BA_MAX_STEPS 31

/******************************************************************************
 * Config Usage, all stored in a single PS key. With the following format:
 *  | Fixed IV (3 words) | Seckey (9 words) | Role (1 word)
 ******************************************************************************/
#define CSB_RECEIVER_PSKEY   198
#define CSB_BROADCASTER_PSKEY   199

#define CONFIG_RECEIVER_SECKEY_PSKEY     CSB_RECEIVER_PSKEY
#define CONFIG_RECEIVER_FIXED_IV_PSKEY   CSB_RECEIVER_PSKEY

#define CONFIG_BROADCASTER_SECKEY_PSKEY     CSB_BROADCASTER_PSKEY
#define CONFIG_BROADCASTER_FIXED_IV_PSKEY   CSB_BROADCASTER_PSKEY

/*! Time in seconds to wait for association to complete.
    Association will be cancelled if not completed within
    this time. */
#define ASSOCIATION_TIMEOUT         (30)

/*--------------BLE Connection Specific Defines-------------*/
/*! Advertising scan parameters for fast and slow scan modes. */
#define ADV_SCAN_INTERVAL_FAST  96
#define ADV_SCAN_WINDOW_FAST    48
#define ADV_SCAN_INTERVAL_SLOW  512
#define ADV_SCAN_WINDOW_SLOW    16

/* AFH Update message defines */
/* Length of the header in an AFH Update SCM message, i.e. the csb_scm_msg_id. */
#define SCM_AFH_UPDATE_MESSAGE_HEADER_LEN   1
/* Length of the channel map data in bytes. */
#define SCM_AFH_UPDATE_MESSAGE_MAP_LEN      10
/* Length of clock instant data in the message, in bytes. */
#define SCM_AFH_UPDATE_MESSAGE_INSTANT_LEN  4

/* Total size of an AFH Update SCM message. */
#define AFH_UPDATE_MESSAGE_SIZE             ((SCM_AFH_UPDATE_MESSAGE_HEADER_LEN) + \
                                             (SCM_AFH_UPDATE_MESSAGE_MAP_LEN) + \
                                             (SCM_AFH_UPDATE_MESSAGE_INSTANT_LEN))

/*-----------------SCM Defines----------------------*/
/* Base of the range of system reserved SCM message type IDs. */
#define SCM_MESSAGE_ID_SYSTEM_BASE      0x00
/* Top of the range of system reserved SCM message type IDs. */
#define SCM_MESSAGE_ID_SYSTEM_TOP       0x7f
/* Base of the range of vendor SCM message type IDs. */
#define SCM_MESSAGE_ID_VENDOR_BASE      0x80
/* Top of the range of vendor SCM message type IDs. */
#define SCM_MESSAGE_ID_VENDOR_TOP       0xff

/*! Identifier of a Broadcast stream using the CELT codec. */
#define CELT_STREAM_ID_EC_2_5              0x01
#define CELT_STREAM_ID_EC_3_9              0x02

/* Message IDs for SCM message types used by CSB. */
typedef enum
{
    /* Reserved for system use. */
    csb_scm_msg_id_reserved         = SCM_MESSAGE_ID_SYSTEM_BASE,
    /* Volume Reset SCM message. */
    csb_scm_msg_id_vol_reset        = 0x01,
    /* Power Off SCM Message. */
    csb_scm_msg_id_power_off        = 0x02,
    /* AFH Update full channel map SCM message. */
    csb_scm_msg_id_afh_update_full  = 0x03,
    /* AFH Update short channel map SCM message. */
    csb_scm_msg_id_afh_update_short = 0x04
} csb_scm_msg_id;

#define BA_INTERNAL_MESSAGE_BASE    (0x00)
typedef enum _ba_message_t
{
    BA_INTERNAL_MSG_INIT  =  BA_INTERNAL_MESSAGE_BASE,
    BA_INTERNAL_MSG_DE_INIT,
    BA_INTERNAL_MSG_START,
    BA_INTERNAL_MSG_STOP,
    BA_INTERNAL_MSG_SWITCH_MODE,
    BA_INTERNAL_MSG_START_ASSOCIATION,
    BA_INTERNAL_MSG_MSG_TOP
}broadcastAudio_message_t;
/* Message definitions for internal use within the application. */
typedef enum _broadcaster_message_t
{
    BROADCASTER_INTERNAL_MSG_START_BROADCAST =  BA_INTERNAL_MSG_MSG_TOP,
    BROADCASTER_INTERNAL_MSG_STOP_BLE_BROADCAST,
    BROADCASTER_INTERNAL_MSG_STOP_BROADCASTER,
    BROADCASTER_INTERNAL_MSG_DESTROY_BROADCASTER,
    BROADCASTER_MSG_TOP
}broadcaster_message_t;

typedef enum _receiver_message_t
{
    RECEIVER_MSG_WRITE_VOLUME_PSKEY  =  BA_INTERNAL_MSG_MSG_TOP+0x20,
    RECEIVER_INTERNAL_MSG_STOP_BLE_BROADCAST,
    RECEIVER_INTERNAL_MSG_START_RECEIVER,
    RECEIVER_INTERNAL_MSG_STOP_RECEIVER,
    RECEIVER_INTERNAL_MSG_DESTROY_RECEIVER,
    RECEIVER_INTERNAL_MSG_AUDIO_DISCONNECT,
    RECEIVER_MSG_MSG_TOP
}receiver_message_t;

#endif /* _SINK_BA_COMMON_H_ */
