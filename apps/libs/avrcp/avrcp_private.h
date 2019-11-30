/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    avrcp_private.h
    
DESCRIPTION
    
*/

#ifndef AVRCP_PRIVATE_H_
#define AVRCP_PRIVATE_H_

#include "avrcp.h"
#include <source_.h>
#include <sink_.h>

/* Macros for creating messages. */
#define MAKE_AVRCP_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);
/* allocate message with data buffer (-1 because first byte is in sizeof already */
#define MAKE_AVRCP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = \
                (TYPE##_T *) PanicUnlessMalloc(sizeof(TYPE##_T) + LEN - 1);


/* L2CAP AVCTP PSM as defined in the assigned numbers section
   of the Bluetooth spec. */
#define AVCTP_PSM                      0x17
#define AVCTP_BROWSING_PSM             0x1B

#define AVRCP_1_3_EXTENSION_MASK        0x2F
#define AVRCP_1_4_EXTENSION_MASK        0x3F

/* L2CAP Channel Configuration values */
#define AVRCP_MTU_MINIMUM               48
#define AVRCP_MTU_DEFAULT               672
#define AVRCP_MIN_BROWSE_MTU            335

/* Packet defines. */
#define AVCTP0_TRANSACTION_SHIFT        4
#define AVCTP0_PACKET_TYPE_MASK        0x0c
#define AVCTP0_PACKET_TYPE_SINGLE      0x00
#define AVCTP0_PACKET_TYPE_START       0x04
#define AVCTP0_PACKET_TYPE_CONTINUE    0x08
#define AVCTP0_PACKET_TYPE_END         0x0C
#define AVCTP0_CR_MASK                 0x02
#define AVCTP0_CR_COMMAND              0x00
#define AVCTP0_CR_RESPONSE             0x02
#define AVCTP0_IPID                    0x01
#define AVCTP1_PROFILE_AVRCP_HIGH      0x11
#define AVCTP2_PROFILE_AVRCP_CONTROLTARGET       0x0c
#define AVCTP2_PROFILE_AVRCP_REMOTECONTROL       0x0e
#define AVCTP2_PROFILE_AVRCP_REMOTECONTROLLER    0x0f


#define AVRCP0_CTYPE_CONTROL              0x00
#define AVRCP0_CTYPE_STATUS               0x01
#define AVRCP0_CTYPE_NOTIFY               0x03
#define AVRCP1_SUBUNIT_TYPE_MASK          0xf8
#define AVRCP1_SUBUNIT_TYPE_SHIFT           3
#define AVRCP1_SUBUNIT_ID_MASK            0x07
#define AVRCP1_UNIT                       0xff

#define AVRCP2_PASSTHROUGH                0x7C
#define AVRCP2_UNITINFO                   0x30
#define AVRCP2_SUBUNITINFO                0x31
#define AVRCP2_VENDORDEPENDENT            0x00
#define AVRCP3_PASSTHROUGH_OP_MASK        0x7f
#define AVRCP3_PASSTHROUGH_STATE_MASK     0x80
#define AVRCP3_SUBUNITINFO_PAGE_SHIFT       4
#define AVRCP3_SUBUNITINFO_EXTEND_MASK      7
#define AVRCP4_UNITINFO_UNIT_TYPE_SHIFT     3
#define AVRCP4_UNITINFO_UNIT_MASK           7

#define AVCTP_HEADER_START_OFFSET       0
#define AVCTP_SINGLE_PKT_HEADER_SIZE    3
#define AVCTP_START_PKT_HEADER_SIZE     4  
#define AVCTP_CONT_PKT_HEADER_SIZE      1 
#define AVCTP_END_PKT_HEADER_SIZE       AVCTP_CONT_PKT_HEADER_SIZE 
#define AVCTP_PID_SIZE                  2
#define AVCTP_SINGLE_PKT_PID_OFFSET     1
#define AVCTP_START_PKT_PID_OFFSET      2
#define AVCTP_NUM_PKT_OFFSET            1
#define AVCTP_TRANSACTION_MASK          0xf0
#define AVCTP_TRANSACTION_RESET         0x0f

   
#define AVRCP_PASSTHROUGH_HEADER_SIZE   5
#define AVRCP_VENDOR_HEADER_SIZE        6
#define AVRCP_GROUP_HEADER_SIZE         10
#define AVRCP_AVC_MAX_PKT_SIZE          512
#define AVRCP_AVC_MAX_DATA_SIZE         502 /* 512- 10 Byte Header size */ 
#define AVRCP_AVC_MAX_PDU_SIZE          506 /* 512 - Vendor Header of 6 */
#define AVRCP_TOTAL_HEADER_SIZE         6
#define AVRCP_START_PKT_HEADER_SIZE     7
#define AVRCP_CTYPE_OFFSET              0
#define AVRCP_SUBUNIT_OFFSET            1
#define AVRCP_OPCODE_OFFSET             2
#define AVRCP_OPID_OFFSET               3 
#define AVRCP_DATA_LEN_OFFSET           4
#define AVRCP_COMPANY_ID_OFFSET         5 
#define AVRCP_VENDOR_UNIQUE_ID_OFFSET   8  
#define AVRCP_VENDOR_COMPANY_ID_OFFSET  3   
#define AVRCP_CAPS_COMPANY_ID_SIZE      3  
#define AVRCP_CAPS_EVENTS_SIZE          2 
#define AVRCP_CAPS_EVENTS_SIZE_CAT_2    1

#define AVRCP_PASSTHROUGH_GROUP_SIZE    10
#define AVRCP_UNITINFO_SIZE             8
#define AVRCP_UNITINFO_PANEL_OFFSET     4
#define AVRCP_SUBUNIT_PAGE_OFFSET       3
#define AVRCP_SUBUNIT_PAGE_DATA_OFFSET  4
#define AVRCP_AVC_LEN_OFFSET            8
#define AVRCP_AVC_LEN_START_OFFSET      12 
#define AVRCP_AVC_PT_OFFSET             7    
#define AVRCP_AVC_PDU_OFFSET            6
#define AVRCP_AVC_PARAM_OFFSET          10
#define AVRCP_GET_ELEMENTS_HDR_SIZE     9
#define AVRCP_GET_PLAY_STATUS_SIZE      9
#define AVRCP_APP_NUM_ATTR_HDR_SIZE     1
#define AVRCP_APP_VAL_TXT_HDR_SIZE      2
#define AVRCP_ERROR_CODE_SIZE           1
#define AVRCP_EVENT_TRACK_HDR_SIZE      9
#define AVRCP_EVENT_POS_HDR_SIZE        5
#define AVRCP_EVENT_DEFAULT_HDR_SIZE    1
#define AVRCP_EVENT_STATUS_HDR_SIZE     2
#define AVRCP_DEFAULT_EVENT_DATA_SIZE   2
#define AVRCP_PLAYER_CHANGED_EVENT_SIZE 5
#define AVRCP_AVC_START_HEADER_OFFSET   10
#define AVRCP_AVC_START_HEADER_SIZE     14
#define AVRCP_PLAY_ITEM_SIZE            11 
#define AVRCP_UID_CHANGED_EVENT_SIZE    3
#define AVRCP_MIN_AVC_PACKET_SIZE       11 /* MetaData (10) + min 1 byte data */

/* Maximum size of Vendor command allowed */
#define AVRCP_MAX_VENDOR_DATA           4                  

/* Value used to identify metadata PDU in Vendor Dependent data. */
#define AVRCP_BT_COMPANY_ID             ((uint32)6488)

/* Mask to retrieve Error Status values from avrcp_response_code */
#define AVRCP_RESPONSE_ERROR_STATUS_MASK           0x7F

/* Frame avrcp_status_code from Error Status values */
#define AVRCP_ERROR_STATUS_BASE                    0x80

/* Number of bytes used to hold metadata header. */
#define METADATA_HEADER_SIZE                        0x04
#define METADATA_LEN_OFFSET                            2

/* Metadata PDU values from the spec.  Metadata PDU Ids supporting 
   Continuation are defined in avrcp.h file. Application required 
   that PDU IDs to use in AvrcpRequestContinuation() and 
   AvrcpAbortContinuation() APIs. */
#define AVRCP_INVALID_PDU_ID                        0x00
#define AVRCP_INFORM_BATTERY_STATUS_PDU_ID          0x18
#define AVRCP_GET_PLAY_STATUS_PDU_ID                0x30
#define AVRCP_REGISTER_NOTIFICATION_PDU_ID          0x31
#define AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID    0x40
#define AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID      0x41
#define AVRCP_SET_ABSOLUTE_VOLUME_PDU_ID            0x50
#define AVRCP_SET_ADDRESSED_PLAYER_PDU_ID           0x60
#define AVRCP_PLAY_ITEM_PDU_ID                      0x74
#define AVRCP_ADD_TO_NOW_PLAYING_PDU_ID             0x90

/* Browsed Player PDU IDs */
#define AVBP_SET_BROWSED_PLAYER_PDU_ID              0x70
#define AVBP_GET_FOLDER_ITEMS_PDU_ID                0x71
#define AVBP_CHANGE_PATH_PDU_ID                     0x72
#define AVBP_GET_ITEM_ATTRIBUTES_PDU_ID             0x73
#define AVBP_GET_NUMBER_OF_ITEM_PDU_ID             0x75
#define AVBP_SEARCH_PDU_ID                          0x80
#define AVBP_GENERAL_REJECT_PDU_ID                  0xA0

/* Event values from the spec. */
#define AVRCP_MAX_NUM_EVENTS                        13
#define EVENT_PLAYBACK_STATUS_CHANGED               0x01
#define EVENT_TRACK_CHANGED                         0x02
#define EVENT_TRACK_END                             0x03
#define EVENT_TRACK_START                           0x04
#define EVENT_PLAYBACK_POS_CHANGED                  0x05
#define EVENT_BATTERY_STATUS_CHANGED                0x06
#define EVENT_SYSTEM_STATUS_CHANGED                 0x07
#define EVENT_PLAYER_SETTING_CHANGED                0x08
#define EVENT_PLAYING_CONTENT_CHANGED               0x09
#define EVENT_AVAILABLE_PLAYERS_CHANGED             0x0A
#define EVENT_ADDRESSED_PLAYER_CHANGED              0x0B
#define EVENT_UIDS_CHANGED                          0x0C
#define EVENT_VOLUME_CHANGED                        0x0D

/* AVRCP ERROR STATUS CODES */
#define AVRCP_STATUS_INTERNAL_ERROR                 0x03
#define AVRCP_STATUS_SUCCESS                        0x04

#define AVRCP_APP_WATCHDOG_TIMEOUT                  D_SEC(30)

#ifdef AVRCP_DEBUG_LIB

/* use large timeout value for debugging purpose */
#define AVCTP_WATCHDOG_TIMEOUT                     D_SEC(4)
#define AVCTP_SEND_RESPONSE_TIMEOUT                D_SEC(4)

#else

/* Use double of response timeout. Trcp=100 Tmtc=200 and Tmtp =1000. 
   Use a largest value instead of using complex logic */
#define AVCTP_WATCHDOG_TIMEOUT                    D_SEC(4)

/* Spec respose timeout. Trcp=100 Tmtc=200 and Tmtp =1000. 
   Use a largest value instead of using complex logic */
#define AVCTP_SEND_RESPONSE_TIMEOUT               D_SEC(2)

#endif

/* A Large timeout value for Browsing channel */
#define AVBP_WATCHDOG_TIMEOUT             ((uint32) 20000)
#define AVBP_SEND_RESPONSE_TIMEOUT        ((uint32) 20000)

/* Browsing channel values */
/* Set Browsed player Response uses 10 bytes Fixed Param + 
   3 Bytes AVRCP header +  3 bytes AVCTP header = 16 bytes */
#define AVBP_MAX_FIXED_PDU_SIZE      16   
#define AVBP_DEFAULT_MAX_TRANSMIT    0x0000
#define AVBP_MAX_TRANSMIT            0xFFFF
#define AVBP_MIN_REMOTE_MTU             335

/* Common Vendor dependent Confirm Messages */
#define AVRCP_INTERNAL_SET_APP_VALUE_RES_T  AVRCP_INTERNAL_VENDORDEPENDENT_RES_T
#define AVRCP_INTERNAL_ABORT_CONTINUING_RES_T \
                                            AVRCP_INTERNAL_VENDORDEPENDENT_RES_T
#define AVRCP_INTERNAL_GROUP_RES_T          AVRCP_INTERNAL_VENDORDEPENDENT_RES_T
  
/* Macro used to generate debug version of this library */
#ifdef AVRCP_DEBUG_LIB
#ifndef DEBUG_PRINT_ENABLED
#define DEBUG_PRINT_ENABLED
#endif

#include <panic.h>
#include <print.h>
#include <stdio.h>

/* Turn ON Extra Debug when required */
/* #define AVRCP_EXTRA_DEBUG 1  */

#define AVRCP_DEBUG(x) {PRINT(x); Panic();}
#ifdef AVRCP_EXTRA_DEBUG
#define AVRCP_INFO(x) PRINT(x)
#else
#define AVRCP_INFO(x)
#endif /* AVRCP_EXTRA_DEBUG */
#else
#define AVRCP_DEBUG(x)
#define AVRCP_INFO(x)
#endif /* AVRCP_DEBUG_LIB */


/* Avrcp Profile Library private messages */
#define AVRCP_MSG_BASE     (0x0)

typedef enum
{
    /* Initialisation */
    AVRCP_INTERNAL_TASK_INIT_REQ = AVRCP_MSG_BASE,
    AVRCP_INTERNAL_TASK_DELETE_REQ,
    AVRCP_INTERNAL_INIT_REQ,

    /* Connect and SDP */
    AVRCP_INTERNAL_SDP_SEARCH_REQ,
    AVRCP_INTERNAL_CONNECT_REQ,
    AVRCP_INTERNAL_CONNECT_RES,
    AVRCP_INTERNAL_DISCONNECT_REQ,
    AVRCP_INTERNAL_GET_FEATURES,
    AVRCP_INTERNAL_GET_EXTENSIONS,

    /* Signalling Request*/
    AVRCP_INTERNAL_PASSTHROUGH_REQ,
    AVRCP_INTERNAL_UNITINFO_REQ,
    AVRCP_INTERNAL_SUBUNITINFO_REQ,
    AVRCP_INTERNAL_VENDORDEPENDENT_REQ,
    AVRCP_INTERNAL_WATCHDOG_TIMEOUT,

    /* Browsing Request */
    AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ,
    AVRCP_INTERNAL_CHANGE_PATH_REQ,
    AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ,
    AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ,
    AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ,
    AVRCP_INTERNAL_SEARCH_REQ,

    /* Signalling Response*/
    AVRCP_INTERNAL_PASSTHROUGH_RES,
    AVRCP_INTERNAL_UNITINFO_RES,
    AVRCP_INTERNAL_SUBUNITINFO_RES,
    AVRCP_INTERNAL_VENDORDEPENDENT_RES,
    AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT,

    /* Metadata transfer Response */
    AVRCP_INTERNAL_GET_CAPS_RES,
    AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES,
    AVRCP_INTERNAL_LIST_APP_VALUE_RES,
    AVRCP_INTERNAL_GET_APP_VALUE_RES,
    AVRCP_INTERNAL_SET_APP_VALUE_RES,
    AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES,
    AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES,
    AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES,
    AVRCP_INTERNAL_GET_PLAY_STATUS_RES,
    AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES,
    AVRCP_INTERNAL_EVENT_COMMON_RES,
    AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES,
    AVRCP_INTERNAL_REJECT_METADATA_RES,
    AVRCP_INTERNAL_ABORT_CONTINUING_RES,
    AVRCP_INTERNAL_GROUP_RES,
    AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET,
    AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES,
    AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES,
    AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES,

    /* Browsing Response */
    AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES,
    AVRCP_INTERNAL_CHANGE_PATH_RES,
    AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES,
    AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES,
    AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES,
    AVRCP_INTERNAL_SEARCH_RES,

    /* Misc */
    AVRCP_INTERNAL_MESSAGE_MORE_DATA,
    AVRCP_COMMON_CFM_MESSAGE,
    AVRCP_MSG_BOTTOM
} AVRCP_INTERNAL_T;


typedef enum 
{
    avrcpReady,
    avrcpConnecting,
    avrcpConnected,
    avrcpDisconnecting
} avrcpState;


typedef enum
{
    avrcp_none = 0,
    avrcp_passthrough,                                                      
    avrcp_unit_info,
    avrcp_subunit_info,
    avrcp_vendor,
    avrcp_next_group,
    avrcp_previous_group,
    avrcp_get_caps              = AVRCP_GET_CAPS_PDU_ID, /* 10 */
    avrcp_list_app_attributes   = AVRCP_LIST_APP_ATTRIBUTES_PDU_ID,
    avrcp_list_app_values       = AVRCP_LIST_APP_VALUE_PDU_ID,
    avrcp_get_app_values        = AVRCP_GET_APP_VALUE_PDU_ID,
    avrcp_set_app_values        = AVRCP_SET_APP_VALUE_PDU_ID,
    avrcp_get_app_attribute_text= AVRCP_GET_APP_ATTRIBUTE_TEXT_PDU_ID,
    avrcp_get_app_value_text    = AVRCP_GET_APP_VALUE_TEXT_PDU_ID,
    avrcp_character_set         = AVRCP_INFORM_CHARACTER_SET_PDU_ID, 
    avrcp_battery_information   = AVRCP_INFORM_BATTERY_STATUS_PDU_ID,
    avrcp_get_element_attributes= AVRCP_GET_ELEMENT_ATTRIBUTES_PDU_ID,
    avrcp_get_play_status       = AVRCP_GET_PLAY_STATUS_PDU_ID, /* 30 */
    avrcp_request_continuation  = AVRCP_REQUEST_CONTINUING_RESPONSE_PDU_ID,
    avrcp_abort_continuation    = AVRCP_ABORT_CONTINUING_RESPONSE_PDU_ID,
    avrcp_absolute_volume       = AVRCP_SET_ABSOLUTE_VOLUME_PDU_ID, /*0x50 */
    avrcp_set_addressed_player  = AVRCP_SET_ADDRESSED_PLAYER_PDU_ID, /*0x60*/
    avrcp_play_item             = AVRCP_PLAY_ITEM_PDU_ID, /* 0x74 */
    avrcp_add_to_now_playing    = AVRCP_ADD_TO_NOW_PLAYING_PDU_ID, /* 0x90*/

    /* The following entries are also used to map into a 16 bit bitfield */
    avrcp_events_start_dummy    = 0xF0, /* Dummy event for start of event */
    avrcp_playback_status,
    avrcp_track_changed,
    avrcp_track_reached_end,
    avrcp_track_reached_start,
    avrcp_playback_pos_changed,
    avrcp_batt_status_changed,
    avrcp_system_status_changed,
    avrcp_player_setting_changed,
    avrcp_playing_content_changed,
    avrcp_available_player_changed,
    avrcp_addressed_player_changed,
    avrcp_uids_changed,
    avrcp_volume_changed,
    /* The above entries are also used to map into a 16 bit bitfield 
     * Having more than 16 is unwise
     */

    avrcp_app_unknown         /* Blocked for an application response */
} avrcpPending;


typedef enum 
{
    avrcp_packet_type_single = 0,
    avrcp_packet_type_start = 1,
    avrcp_packet_type_continue = 2,
    avrcp_packet_type_end = 3
} avrcp_packet_type;


typedef enum 
{
    avrcp_sdp_search_none = 0,
    avrcp_sdp_search_app_features, 
    avrcp_sdp_search_app_profile_version, 
    avrcp_sdp_search_pending,
    avrcp_sdp_search_and_connect_pending,
    avrcp_sdp_search_done_connect_pending
} avrcp_sdp_search;

typedef enum
{
    avrcp_unreg_none,
    avrcp_unreg_signal,
    avrcp_unreg_browse,
    avrcp_unreg_all
} avrcp_unreg_status;

/* Values of this enum must map exactly to avrcp_device_type */
typedef enum
{
    avrcp_tg    = avrcp_target,
    avrcp_ct    = avrcp_controller,
    avrcp_ct_tg = avrcp_target_and_controller,
    avrcp_ct_tg_progress = 0x07 /* Max value - 0111 binary */ 
} avrcp_device_role;
    

typedef struct
{
    Task                        connectionTask;
    avrcp_init_params           init_params;
} AVRCP_INTERNAL_INIT_REQ_T;

typedef struct
{
    bdaddr                bd_addr;
}AVRCP_INTERNAL_SDP_SEARCH_REQ_T;

typedef struct
{
    bdaddr                bd_addr;
} AVRCP_INTERNAL_CONNECT_REQ_T;


typedef struct
{
    uint16                connection_id;
    uint16                signal_id;
    bool                  accept;
} AVRCP_INTERNAL_CONNECT_RES_T;

typedef struct
{
    AVRCP                *avrcp;
    avrcp_status_code    status;
    Sink                sink;    
} AVRCP_COMMON_CFM_MESSAGE_T;

#ifndef AVRCP_TG_ONLY_LIB /* Disable CT for TG only lib */

typedef struct
{
    avc_subunit_type    subunit_type;
    avc_subunit_id      subunit_id;
    bool                state;
    avc_operation_id    opid;
    avrcpPending        pending;
    uint16              operation_data_length;
    Source              operation_data;
} AVRCP_INTERNAL_PASSTHROUGH_REQ_T;

typedef struct
{
    uint8 page;
} AVRCP_INTERNAL_SUBUNITINFO_REQ_T;



typedef struct
{
    AVRCP               *avrcp;
    avrcp_status_code    status;
}AVRCP_COMMON_STATUS_CFM_T;

typedef struct
{
    AVRCP                *avrcp;
    avrcp_status_code    status;

#ifdef AVRCP_ENABLE_DEPRECATED
    uint16              transaction;
#endif

} AVRCP_COMMON_METADATA_CFM_MESSAGE_T;

typedef struct
{
    AVRCP               *avrcp;
    avrcp_status_code   status;
    uint16              metadata_packet_type;    
    uint16              number_of_data_items;    
    uint16              size_data;    
    Source              data;

#ifdef AVRCP_ENABLE_DEPRECATED
    uint16              transaction;
    uint16              no_packets;    
    uint16              ctp_packet_type;    
    uint16              data_offset;   
#endif
} AVRCP_COMMON_FRAGMENTED_METADATA_CFM_T;

typedef struct
{
    avc_subunit_type    subunit_type;
    avc_subunit_id        subunit_id;
    uint8                 ctype;
    uint32                company_id;
    avrcpPending          pending;
    uint16                data_length;
    Source                data;    
    uint16                extra_data_len;
    uint8                 extra_data[1];
} AVRCP_INTERNAL_VENDORDEPENDENT_REQ_T;

typedef struct{
    uint16  player_id;
}AVRCP_INTERNAL_BROWSE_SET_PLAYER_REQ_T;

typedef struct{
   uint16                  uid_counter;
   avrcp_browse_direction  direction;
   avrcp_browse_uid        folder_uid;
}AVRCP_INTERNAL_CHANGE_PATH_REQ_T;

typedef struct{
    avrcp_browse_scope  scope; 
    avrcp_browse_uid    uid;     
    uint16              uid_counter;    
    uint8               num_attr; 
    uint16              attr_list_size;
    Source              attr_list;     
}AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_REQ_T;

typedef struct{
    avrcp_browse_scope  scope;
    uint32              start;
    uint32              end;
    uint8               num_attr;
    uint16              attr_list_size;
    Source              attr_list;
}AVRCP_INTERNAL_GET_FOLDER_ITEMS_REQ_T;

typedef struct{
    avrcp_browse_scope  scope;
}AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_REQ_T;

typedef struct{
    avrcp_char_set      char_set;
    uint16              str_length;
    Source              string;
}AVRCP_INTERNAL_SEARCH_REQ_T;

#endif /* !AVRCP_TG_ONLY_LIB */

#ifndef AVRCP_CT_ONLY_LIB /* Disable TG for CT only lib */
typedef struct
{
    avrcp_response_type    response;
} AVRCP_INTERNAL_VENDORDEPENDENT_RES_T,
  AVRCP_INTERNAL_PASSTHROUGH_RES_T;

typedef struct
{
    avrcp_response_type    response;
    uint8                  pdu_id;
}AVRCP_INTERNAL_COMMON_METADATA_CONTROL_RES_T;

typedef struct
{
    avrcp_response_type     response;
    avrcp_supported_events  event;
}AVRCP_INTERNAL_EVENT_COMMON_RES_T;

typedef struct
{
    bool                accept;
    avc_subunit_type    unit_type;
    uint8                unit;
    uint32                company_id;
} AVRCP_INTERNAL_UNITINFO_RES_T;

typedef struct
{
    bool    accept;
    uint8    page_data[PAGE_DATA_LENGTH];
} AVRCP_INTERNAL_SUBUNITINFO_RES_T;

typedef struct
{
    AVRCP                *avrcp;

#ifdef AVRCP_ENABLE_DEPRECATED
    uint16              transaction;
#endif
} AVRCP_COMMON_METADATA_IND_MESSAGE_T;


typedef struct
{
    AVRCP               *avrcp;
    uint16              number_of_data_items;    
    uint16              size_data;    
    Source              data;

#ifdef AVRCP_ENABLE_DEPRECATED
    uint16              transaction;
    uint16              no_packets;    
    uint16              ctp_packet_type; 
    uint16              data_offset;
    uint16              metadata_packet_type;    
#endif

} AVRCP_COMMON_FRAGMENTED_METADATA_IND_T;


typedef struct
{
    avrcp_response_type response;
    avrcp_capability_id caps_id;
    uint16              size_caps_list;
    Source              caps_list;
} AVRCP_INTERNAL_GET_CAPS_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint8    size_attributes_list; 
    Source attributes_list;
} AVRCP_INTERNAL_LIST_APP_ATTRIBUTE_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint8    size_values_list; 
    Source values_list;
} AVRCP_INTERNAL_LIST_APP_VALUE_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint16    size_values_list; 
    Source values_list;
} AVRCP_INTERNAL_GET_APP_VALUE_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint8 number_of_attributes;
    uint16    size_attributes_list; 
    Source attributes_list;
} AVRCP_INTERNAL_GET_APP_ATTRIBUTES_TEXT_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint8 number_of_values;
    uint16    size_values_list; 
    Source values_list;
} AVRCP_INTERNAL_GET_APP_VALUE_TEXT_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint16 number_of_attributes;
    uint16    size_attributes_list; 
    Source attributes_list;
} AVRCP_INTERNAL_GET_ELEMENT_ATTRIBUTES_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint32 song_length;
    uint32 song_elapsed;
    avrcp_play_status play_status;
} AVRCP_INTERNAL_GET_PLAY_STATUS_RES_T;


typedef struct
{
    avrcp_response_type response;
    avrcp_play_status play_status;
} AVRCP_INTERNAL_EVENT_PLAYBACK_STATUS_CHANGED_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint32 track_index_high;
    uint32 track_index_low;
} AVRCP_INTERNAL_EVENT_TRACK_CHANGED_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint32 playback_pos;
} AVRCP_INTERNAL_EVENT_PLAYBACK_POS_CHANGED_RES_T;


typedef struct
{
    avrcp_response_type response;
    avrcp_battery_status battery_status;
} AVRCP_INTERNAL_EVENT_BATT_STATUS_CHANGED_RES_T;


typedef struct
{
    avrcp_response_type response;
    avrcp_system_status system_status;
} AVRCP_INTERNAL_EVENT_SYSTEM_STATUS_CHANGED_RES_T;


typedef struct
{
    avrcp_response_type response;
    uint16 size_attributes;
    Source attributes;
} AVRCP_INTERNAL_EVENT_PLAYER_APP_SETTING_CHANGED_RES_T;

typedef struct
{
    avrcp_response_type response;
    uint8 pdu_id;
}AVRCP_INTERNAL_REJECT_METADATA_RES_T; 


typedef struct
{
    avrcp_response_type response;
    uint8               volume;
}AVRCP_INTERNAL_SET_ABSOLUTE_VOL_RES_T, 
 AVRCP_INTERNAL_EVENT_VOLUME_CHANGED_RES_T;


typedef struct{
    avrcp_response_type response;  
    uint16              player_id;
    uint16              uid_counter;
}AVRCP_INTERNAL_EVENT_ADDRESSED_PLAYER_CHANGED_RES_T;

typedef struct{
    avrcp_response_type response;  
    uint16              uid_counter;
}AVRCP_INTERNAL_EVENT_UIDS_CHANGED_RES_T;


typedef struct{
    avrcp_response_type response;  
    uint16              uid_counter;
    uint32              num_items;
    avrcp_char_set      char_type;
    uint8               folder_depth;
    uint16              size_folder_path;
    Source              folder_path;
}AVRCP_INTERNAL_BROWSE_SET_PLAYER_RES_T;


typedef struct{
    avrcp_response_type  response;
    uint32               num_items;
}AVRCP_INTERNAL_CHANGE_PATH_RES_T;


typedef struct{
    avrcp_response_type response;
    uint8               num_attributes;
    uint16              size_attr_list;
    Source              attr_value_list;
}AVRCP_INTERNAL_GET_ITEM_ATTRIBUTES_RES_T;


typedef struct{
    avrcp_response_type response;
    uint16              uid_counter;
    uint16              num_items;
    uint16              item_list_size;
    Source              item_list;
}AVRCP_INTERNAL_GET_FOLDER_ITEMS_RES_T;

typedef struct{
    avrcp_response_type response;
    uint16              uid_counter;
    uint32              num_items;
}AVRCP_INTERNAL_GET_NUMBER_OF_ITEMS_RES_T;

typedef struct{
    avrcp_response_type response;
    uint16              uid_counter;
    uint32              num_items;   
}AVRCP_INTERNAL_SEARCH_RES_T; 

typedef struct
{
    Source data;
    uint16 param_length;
    uint16 pdu_id;
    uint16 response;
} AVRCP_INTERNAL_NEXT_CONTINUATION_PACKET_T;

#endif /* !AVRCP_CT_ONLY_LIB*/

typedef struct
{
    avrcpPending pending_command;
    uint16 data;
} AVRCP_INTERNAL_SEND_RESPONSE_TIMEOUT_T;

typedef struct 
{
    TaskData            cleanUpTask;
    uint8               *sent_data;
} AvrcpCleanUpTask;

typedef struct
{
    Task       app_task;
    struct
    {
        unsigned   local_target_features:8;
        unsigned   local_controller_features:8;
        unsigned   local_extensions:8;
        avrcp_device_role    device_type:3;
        unsigned   unused:5;
    } bitfields;
} AvrcpDeviceTask;


typedef struct __AVBP AVBP;

struct __AVRCP
{
    TaskData                task;
    Task                    clientTask;
    Task                    avbp_task;
    AvrcpCleanUpTask        dataFreeTask;
    Sink                    sink;
    Source                  continuation_data;
    Source                  data_app_ind;
    uint16                  continuation_pdu;
    uint16                  l2cap_mtu;
    uint16                  tg_registered_events; /* Events registered on TG, upto 15 events*/
    uint8                   tg_notify_transaction_label[AVRCP_MAX_NUM_EVENTS]; 
    uint16                  ct_registered_events; /* Events registered by CT, upto 15 events */
    uint8                   ct_notify_transaction_label[AVRCP_MAX_NUM_EVENTS]; 
    uint8*                  av_msg;
    uint16                  av_msg_len;
    uint16                  sdp_search_mode; /* Type: avrcp_sdp_search. uint16 as used in MessageSendConditionally */
    uint16                  pending;         /* Type: avrcpPending. uint16 as used in MessageSendConditionally */
    struct 
    {
        avrcpPending            block_received_data:8;
        unsigned                remote_features:8;
        unsigned                remote_extensions:8;
        unsigned                cmd_transaction_label:4;
        unsigned                rsp_transaction_label:4;
        avrcpState              state:3;
        unsigned                sdp_search_repeat:1;
        unsigned                fragment:2;
        unsigned                unused:2;
    } bitfields;
    uint16                  av_max_data_size;
    uint8                   avctp_packets_remaining; /* packets of fragmented
                                                        message still to come */
    bdaddr                bd_addr;
};

typedef struct _AVRCP_List
{
    AVRCP *avrcp; /* stores avrcp related data information. */
    struct _AVRCP_List *next; /* Pointer to next node. */
}avrcpList;

/* Provide a reference for all AVRCP lib modules - should not be exposed at API level */
extern avrcpList *avrcpListHead;

/******************************************************************
 * Browsing Channel Data Structure definitions. 
 */

typedef enum 
{
    avbpInitialising = 0,
    avbpReady,
    avbpConnecting,
    avbpConnected
} avbpState;


/* Browsing channel Task */
struct __AVBP
{
    TaskData                task;          /* AVBP Task */
    Task                    avrcp_task;    /* Associated AVRCP Task */
    Sink                    avbp_sink;     /* L2CAP sink for Browse Data */
    uint16                  avbp_sink_data; /* Application Data on L2CAP Sink */
    uint16                  avbp_mtu;      /* Browsing channel Outgoing MTU */
    uint16                  blocking_cmd;  /* Outstanding Command pending */
    struct
    {
        unsigned                trans_id:4;    /* Outstanding/Last Transaction ID*/
        avbpState               avbp_state:2;  /* State of Browsing Channel */ 
        unsigned                reserved:2;    /* reserved for Role while
                                                  supporting multiple instances */
    } bitfields;
};

#endif /* AVRCP_PRIVATE_H_ */
