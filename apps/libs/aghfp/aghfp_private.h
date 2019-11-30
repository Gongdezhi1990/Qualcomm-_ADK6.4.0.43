/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

*/


#ifndef AGHFP_PRIVATE_H_
#define AGHFP_PRIVATE_H_

#include "aghfp.h"
#include "aghfp_wbs.h"
#include <connection.h>
#include <message.h>
#include <app/message/system_message.h>
#include <stdlib.h>


/* Macros for creating messages */
#define MAKE_AGHFP_MESSAGE(TYPE) TYPE##_T *message = malloc(sizeof(TYPE##_T));
#define MAKE_AGHFP_MESSAGE_WITH_ARRAY(TYPE, len) \
	TYPE##_T *message = (TYPE##_T*)malloc(sizeof(TYPE##_T) + sizeof(uint16) * len);
#define COPY_AGHFP_MESSAGE(src, dst) *dst = *src;


/* Macros used to generate debug lib panics */
#ifdef AGHFP_DEBUG_LIB
 #include <panic.h>
 #include <stdio.h>
 #define DEBUG_PRINT_ENABLED
 #define AGHFP_DEBUG(x)	{ printf x; }
 #define AGHFP_DEBUG_PANIC(x) 	{ printf x; Panic(); }
 #define AGHFP_DEBUG_ASSERT(x, y) 	{ if (!(x)) { printf y; Panic(); } }
#else
 #define AGHFP_DEBUG(x)
 #define AGHFP_DEBUG_PANIC(x)
 #define AGHFP_DEBUG_ASSERT(x, y)
#endif

/* Constants used by the AGHFP profile lib. */
#define AT_RESPONSE_TIMEOUT	(5000)
#define DEFAULT_RING_ALERT_REPEAT (5000)


enum
{
	AGHFP_INTERNAL_MSG_BASE = 0,

    /* Initialisation */
    AGHFP_INTERNAL_INIT_REQ = AGHFP_INTERNAL_MSG_BASE,
	AGHFP_INTERNAL_INIT_CFM,
	AGHFP_INTERNAL_SDP_REGISTER_CFM,

	/* SLC connection */
	AGHFP_INTERNAL_SLC_CONNECT_REQ,
	AGHFP_INTERNAL_SLC_CONNECT_RES,
	AGHFP_INTERNAL_RFCOMM_CONNECT_REQ,
	AGHFP_INTERNAL_SLC_DISCONNECT_REQ,
	
	/* AT cmd receive */
    AGHFP_INTERNAL_AT_CKPD_CMD,					/* AT+CKPD=d		*/
    AGHFP_INTERNAL_AT_BRSF_REQ,                 /* AT+BRSF=f        */
    AGHFP_INTERNAL_AT_CIND_SUPPORTED_REQ,       /* AT+CIND=?        */
    AGHFP_INTERNAL_AT_CIND_STATUS_REQ,          /* AT+CIND?         */
    AGHFP_INTERNAL_AT_CMER_REQ,                 /* AT+CMER=m,k,d,i  */
    AGHFP_INTERNAL_CALL_HOLD_SUPPORT_REQ,		/* AT+CHLD=?        */
    AGHFP_INTERNAL_CALL_HOLD_REQ,				/* AT+CHLD=ci       */
    AGHFP_INTERNAL_ANSWER_REQ,					/* ATA              */
    AGHFP_INTERNAL_CALLER_ID_SETUP_REQ,			/* AT+CLIP          */
	AGHFP_INTERNAL_CALL_WAITING_SETUP_REQ,		/* AT+CCWA          */
	AGHFP_INTERNAL_CALL_HANG_UP_REQ,			/* AT+CHUP          */
	AGHFP_INTERNAL_DIAL_REQ,    				/* ATD              */
	AGHFP_INTERNAL_MEMORY_DIAL_REQ,				/* ATD              */
	AGHFP_INTERNAL_LAST_NUMBER_REDIAL_REQ,		/* AT+BLDN          */
	AGHFP_INTERNAL_NREC_SETUP_REQ,				/* AT+NREC          */
	AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ,	/* AT+BVRA          */
	AGHFP_INTERNAL_PHONE_NUMBER_REQ,			/* AT+BINP          */
	AGHFP_INTERNAL_TRANSMIT_DTMF_CODE,			/* AT+VTS           */
	AGHFP_INTERNAL_RESPONSE_HOLD_STATUS_REQUEST_REQ,/* AT+BTRH?     */
	AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ,/* AT+BTRH=c        */
	AGHFP_INTERNAL_SUBSCRIBER_NUMBER_REQ,       /* AT+CNUM          */
	AGHFP_INTERNAL_CURRENT_CALLS_REQ,           /* AT+CLCC          */
	AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ, /* AT+COPS=m,f      */
	AGHFP_INTERNAL_NETWORK_OPERATOR_REQ,        /* AT+COPS?         */
    AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ,   /* AT+BIA=          */
    AGHFP_INTERNAL_AT_BIND_REQ,                 /* AT+BIND=..       */
    AGHFP_INTERNAL_AT_BIND_SUPPORTED_REQ,       /* AT+BIND=?        */
    AGHFP_INTERNAL_AT_BIND_INDICATOR_STATUS_REQ,/* AT+BIND?         */
    AGHFP_INTERNAL_AT_BIEV_STATUS_REQ,          /* AT+BIEV=<a>,<val>*/

    /* SCO/eSCO */
    AGHFP_INTERNAL_AUDIO_TRANSFER_REQ,
    AGHFP_INTERNAL_AUDIO_CONNECT_REQ,
	AGHFP_INTERNAL_AUDIO_CONNECT_RES,
    AGHFP_INTERNAL_AUDIO_DISCONNECT_REQ,

	/* Indicators */
	AGHFP_INTERNAL_SEND_SERVICE_INDICATOR,
	AGHFP_INTERNAL_SEND_CALL_INDICATOR,
	AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR,
	AGHFP_INTERNAL_SEND_CALL_HELD_INDICATOR,
	AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR,
	AGHFP_INTERNAL_SEND_ROAM_INDICATOR,
	AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR,
	AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION,
	AGHFP_INTERNAL_SET_SERVICE_STATE,

	/* HF Indicator */
	AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE,

	/* Ring */
	AGHFP_INTERNAL_SEND_RING_ALERT,
	AGHFP_INTERNAL_SEND_CALLER_ID,
	AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE,

	/* Call Management */
	AGHFP_INTERNAL_CALL_MGR_CREATE_REQ,
	AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ,
	AGHFP_INTERNAL_CALL_MGR_ANSWER_REQ,
	AGHFP_INTERNAL_CALL_MGR_REMOTE_ANSWERED_REQ,
	AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ,
    AGHFP_INTERNAL_CALL_MGR_TERMINATE_HELD_REQ,
    AGHFP_INTERNAL_CALL_MGR_ACCEPT_HELD_REQ,
	AGHFP_INTERNAL_RING_REPEAT_REQ,
	
	/* Misc */
	AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ,
	AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE,
	AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG,
	AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN,
	AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME,
	AGHFP_INTERNAL_SYNC_MIC_GAIN,				/* AT+VGS */
	AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME,			/* AT+VGM */
    AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE,
    AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE,
    AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER,
    AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBERS_COMPLETE,
    AGHFP_INTERNAL_SEND_CURRENT_CALL,
    AGHFP_INTERNAL_SEND_CURRENT_CALLS_COMPLETE,
    AGHFP_INTERNAL_SEND_NETWORK_OPERATOR,
    AGHFP_INTERNAL_USER_DATA_REQ,
    AGHFP_INTERNAL_SEND_ERROR_REQ,
    AGHFP_INTERNAL_SEND_OK_REQ,

	/* Defined here so its not visible outside this lib */
	AGHFP_COMMON_CFM_MESSAGE,
	
	/* WB-Speech internal messages */
	AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ,
	AGHFP_INTERNAL_WBS_CODEC_CONNECTION_REQ,
	AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ,
	AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ,

        /* Qualcomm Codec Extensions (QCE) internal messages */
        AGHFP_INTERNAL_AT_QAC_REQ,
        AGHFP_INTERNAL_AT_QCS_REQ,
	
	AGHFP_INTERNAL_MSG_TOP
};


typedef struct
{
	aghfp_init_status	status;
	uint8				rfcomm_channel;
} AGHFP_INTERNAL_INIT_CFM_T;


typedef struct
{
	aghfp_lib_status	status;
} AGHFP_INTERNAL_SDP_REGISTER_CFM_T;


typedef struct
{
	bdaddr		addr;
} AGHFP_INTERNAL_SLC_CONNECT_REQ_T;


typedef struct
{
	bool		response;
} AGHFP_INTERNAL_SLC_CONNECT_RES_T;


typedef struct
{
	bool		response;
} AGHFP_INTERNAL_SLC_DISCONNECT_RES_T;


typedef struct
{
	bdaddr		addr;
	uint8		rfc_channel;
} AGHFP_INTERNAL_RFCOMM_CONNECT_REQ_T;


typedef struct
{
    aghfp_audio_transfer_direction	direction;
    sync_pkt_type   				packet_type;
	aghfp_audio_params  			audio_params;
} AGHFP_INTERNAL_AUDIO_TRANSFER_REQ_T;


typedef struct
{
    sync_pkt_type   	packet_type;
	aghfp_audio_params  audio_params;
} AGHFP_INTERNAL_AUDIO_CONNECT_REQ_T;


typedef struct
{
	bool                response;
    sync_pkt_type 		packet_type;
    aghfp_audio_params	audio_params;
} AGHFP_INTERNAL_AUDIO_CONNECT_RES_T;


typedef struct
{
	uint8 code;
} AGHFP_INTERNAL_TRANSMIT_DTMF_CODE_T;


typedef struct
{
	uint8 cmd;
} AGHFP_INTERNAL_SET_RESPONSE_HOLD_STATUS_REQ_T;


typedef struct
{
	uint8 mode;
	uint8 format;
} AGHFP_INTERNAL_NETWORK_OPERATOR_FORMAT_REQ_T;

typedef enum 
{
    indicator_off    = 0,
    indicator_on     = 1,
    indicator_ignore = 2
} indicator_status;

typedef struct
{
	indicator_status service:2;
    indicator_status signal:2;
    indicator_status roam:2;
    indicator_status battchg:2;
}AGHFP_INTERNAL_INDICATORS_ACTIVATION_REQ_T;

typedef struct
{
	aghfp_service_availability availability;
} AGHFP_INTERNAL_SEND_SERVICE_INDICATOR_T;

typedef struct
{
	aghfp_hf_indicators_assigned_id assigned_num;
	aghfp_hf_indicator_state  state;
} AGHFP_INTERNAL_SEND_BIND_INDICATOR_STATE_T;

typedef struct
{
	aghfp_call_status status;
} AGHFP_INTERNAL_SEND_CALL_INDICATOR_T;


typedef struct
{
	aghfp_call_setup_status type;
} AGHFP_INTERNAL_SEND_CALL_SETUP_INDICATOR_T;


typedef struct
{
	aghfp_call_held_status status;
} AGHFP_INTERNAL_SEND_CALL_HELD_INDICATOR_T;


typedef struct
{
	uint16 level;
} AGHFP_INTERNAL_SEND_SIGNAL_INDICATOR_T;


typedef struct
{
	aghfp_roam_status status;
} AGHFP_INTERNAL_SEND_ROAM_INDICATOR_T;


typedef struct
{
	uint16 level;
} AGHFP_INTERNAL_SEND_BATT_CHG_INDICATOR_T;


typedef struct
{
	bool enable;
} AGHFP_INTERNAL_INBAND_RING_TONE_ENABLE_T;


typedef struct
{
	aghfp_call_type		call_type;
	sync_pkt_type		packet_type;
	bool				in_band;
	aghfp_audio_params	audio_params;
} AGHFP_INTERNAL_CALL_MGR_CREATE_WITH_AUDIO_REQ_T;


typedef struct
{
	aghfp_call_type		call_type;
	sync_pkt_type		packet_type;
	bool				in_band;
} AGHFP_INTERNAL_CALL_MGR_CREATE_REQ_T;


typedef struct
{
	bool	keep_sink;
} AGHFP_INTERNAL_CALL_MGR_TERMINATE_REQ_T;


typedef struct
{
	uint8 		type_number; 	/* Number type of incoming call */
	uint16		size_number; 	/* Length of number string for incoming calls */
	uint16		size_alpha; 	/* Length of alpha string for incoming calls */
	uint8 	    data[1]; 		/* Array to hold both incoming call number and alpha string */
} AGHFP_INTERNAL_SET_CALLER_ID_DETAILS_REQ_T;


typedef struct
{
	uint16 keycode;
} AGHFP_INTERNAL_AT_CKPD_CMD_T;


typedef struct
{
	uint16 hf_supported_features;
} AGHFP_INTERNAL_AT_BRSF_REQ_T;


typedef struct
{
	uint16 mode;
    uint16 ind;
} AGHFP_INTERNAL_AT_CMER_REQ_T;


typedef struct
{
	uint16 action;
	uint16 index;
} AGHFP_INTERNAL_CALL_HOLD_REQ_T;


typedef struct
{
    uint16 state;
} AGHFP_INTERNAL_CALLER_ID_SETUP_REQ_T;


typedef struct
{
    uint16 state;
} AGHFP_INTERNAL_CALL_WAITING_SETUP_REQ_T;


typedef struct
{
    uint16 size_number;
	uint8 number[1];
} AGHFP_INTERNAL_DIAL_REQ_T;


typedef struct
{
    uint16 size_number;
	uint8 number[1];
} AGHFP_INTERNAL_MEMORY_DIAL_REQ_T;


typedef struct
{
	bool state;
} AGHFP_INTERNAL_NREC_SETUP_REQ_T;


typedef struct
{
	bool state;
} AGHFP_INTERNAL_VOICE_RECOGNITION_SETUP_REQ_T;


typedef struct
{
    uint8   type;
	uint16  size_number;
	uint16  size_string;
	uint8   data[1];
} AGHFP_INTERNAL_SEND_CALLER_ID_T;


typedef struct
{
    uint8   type;
	uint16  size_string;		/*    When this message gets destructed, this member needs to be free'd. */
	uint8  *string; 			/* <- The profile handler needs to free it if it rejects the message,    */
	uint16  size_number;		/*    otherwise it becomes the responsibility of the message handler.    */
	uint8   number[1];
} AGHFP_INTERNAL_SEND_CALL_WAITING_NOTIFICATION_T;


typedef struct
{
	bool service_state;
} AGHFP_INTERNAL_SET_SERVICE_STATE_T;


typedef struct
{
	bool enable;
} AGHFP_INTERNAL_VOICE_RECOGNITION_ENABLE_T;


typedef struct
{
	uint16 size_number;
	uint8 number[1];
} AGHFP_INTERNAL_SEND_PHONE_NUMBER_FOR_VOICE_TAG_T;


typedef struct
{
	uint8 gain;
} AGHFP_INTERNAL_SET_REMOTE_MICROPHONE_GAIN_T;


typedef struct
{
	uint8 volume;
} AGHFP_INTERNAL_SET_REMOTE_SPEAKER_VOLUME_T;


typedef struct
{
	uint8 gain;
} AGHFP_INTERNAL_SYNC_MIC_GAIN_T;


typedef struct
{
	uint8 volume;
} AGHFP_INTERNAL_SYNC_SPEAKER_VOLUME_T;


typedef struct
{
   	aghfp_response_hold_state state;
} AGHFP_INTERNAL_SEND_RESPONSE_HOLD_STATE_T;

typedef struct
{
    aghfp_response_hold_state state;
} AGHFP_INTERNAL_CONFIRM_RESPONSE_HOLD_STATE_T;

typedef struct
{
    uint8   id;
    uint8   type;
    uint8   service;
    uint16  size_number;
	uint8   number[1];
} AGHFP_INTERNAL_SEND_SUBSCRIBER_NUMBER_T;

typedef struct
{
    uint8   idx;
    uint8   dir;
    uint8   status;
    uint8   mode;
    uint8   mpty;
    uint8   type;
    uint16  size_number;
    uint8   number[1];

} AGHFP_INTERNAL_SEND_CURRENT_CALL_T;

typedef struct
{
    uint8 mode;
	uint16 size_operator;
	uint8 operator[1];
} AGHFP_INTERNAL_SEND_NETWORK_OPERATOR_T;


typedef struct
{
    uint16 size_data;
	uint8 data[1];
} AGHFP_INTERNAL_USER_DATA_REQ_T;


/* Many messages returned from the AGHFP lib to the app have
   the form of this message. By defining it here its not visible
   outside this lib. This way we can use a common function to
   allocate this message and just set the type when sending the
   message to the app. */
typedef struct
{
	AGHFP				*aghfp;
	aghfp_lib_status	status;
} AGHFP_COMMON_CFM_MESSAGE_T;

typedef struct
{
	aghfp_wbs_codec codec_id;
} AGHFP_INTERNAL_WBS_CODEC_NEGOTIATE_REQ_T;

typedef struct
{
	uint16  num_codecs;
	uint16	codec_uuids[AGHFP_MAX_NUM_CODECS];
} AGHFP_INTERNAL_CODEC_NEGOTIATION_REQ_T;

typedef struct
{
	uint16  num_hf_indicators;
	uint16	hf_indicators_uuids[AGHFP_MAX_HF_INDICATORS];
} AGHFP_INTERNAL_AT_BIND_REQ_T;

typedef struct
{
    uint16          hf_indicator_assigned_num;
    uint16          value;
} AGHFP_INTERNAL_AT_BIEV_STATUS_REQ_T ;

typedef struct
{
    sync_pkt_type   	packet_type;
	aghfp_audio_params  audio_params;
} AGHFP_INTERNAL_SET_AUDIO_PARAMS_REQ_T;

typedef struct
{
    uint16          codec_mode_ids;
} AGHFP_INTERNAL_AT_QAC_REQ_T;

typedef struct 
{
    uint16          codec_mode_id;
} AGHFP_INTERNAL_AT_QCS_REQ_T;

typedef enum
{
	aghfp_hf_noise_reduction				= 0x1,
	aghfp_hf_3_way_calling					= 0x2,
	aghfp_hf_cli_presentation				= 0x4,
	aghfp_hf_voice_recognition				= 0x8,
	aghfp_hf_remote_vol_control				= 0x10,
	aghfp_hf_enhanced_call_status           = 0x20,
	aghfp_hf_enhanced_call_control          = 0x40,
	aghfp_hf_codec_negotiation				= 0x80,
	aghfp_hf_hfindicators                   = 0x100,
	aghfp_hf_esco_s4_supported              = 0x200
} aghfp_hf_supported_features;


/* This enum defines the bit flags in the AGHFP session data's
   features_status variable. */
typedef enum
{
    aghfp_feature_call_waiting_notification	= 0x1,
	aghfp_feature_cli_presentation			= 0x2,
	aghfp_feature_noise_reduction			= 0x4,
    aghfp_feature_inband_ring				= 0x8
} aghfp_features_status;


/* This enum defines all the possible state machine states. Used by the AGHFP
   sessions data's "state" variable. */
typedef enum
{
	aghfp_initialising				= 0,
	aghfp_ready						= 1,
	aghfp_slc_connecting			= 2,
	aghfp_slc_connected				= 3,
	aghfp_incoming_call_establish	= 4,
	aghfp_outgoing_call_establish	= 5,
	aghfp_active_call				= 6,
    aghfp_held_call                 = 7
} aghfp_state;


typedef enum
{
	CallProgressIdle,				/* Call Manager not processing a call request */
	CallProgressWaitInBandSetup,	/* Setting up audio channel for inband ringing (now) and voice (later) */
	CallProgressWaitResponse,		/* Waiting for answer from HF/HS */
	CallProgressWaitAudioOpen,		/* Opening audio channel for voice */
	CallProgressWaitAudioCancel,	/* Closing audio channel (call create aborted) */ 
	CallProgressComplete,			/* Call creation has completed */
	CallProgressWaitAudioClose		/* Closing audio channel (after successful call setup) */
} aghfp_call_progress;


typedef enum
{
	CallEventCreate,
	CallEventAnswer,
	CallEventRemoteAnswered,
	CallEventTerminate,
	CallEventAudioConnected,
	CallEventAudioDisconnected,
	CallEventAudioTransfer,
	CallEventRing,
	CallEventSlcRemoved,
    CallEventTerminateHeld,
    CallEventAcceptHeld
} aghfp_call_event;


typedef enum
{
	CallFlagsNone               = 0x0000,
	CallFlagTerminate			= 0x0001,
	CallFlagOpenAudio			= 0x0002,
	CallFlagCloseAudio			= 0x0004,
	CallFlagResponseReqd		= 0x0008,
	CallFlagResponseAfterAudio	= 0x0010,
	CallFlagRingAlertsReqd      = 0x0020,
	CallFlagResponded           = 0x0040,
	CallFlagFail				= 0x0800,
	CallFlagTimeOut             = 0x1000,
	CallFlagLinkLoss            = 0x2000,
	CallFlagSuccess				= 0x4000,
	CallFlagsAll				= 0x7FFF
} aghfp_call_flags;


typedef enum
{
	aghfp_call_transfer_to_ag,
	aghfp_call_transfer_to_hs
} aghfp_call_transfer;


typedef struct
{
	uint8 		type_number; 	/* Number type of incoming call */
	uint16		size_number; 	/* Length of number string for incoming calls */
	uint16		size_alpha; 	/* Length of alpha string for incoming calls */
	uint8 		*data;	 		/* Array holding both number and alpha string */
} aghfp_call_details;


/* TODO: Make use of audio_params and packet_type already in __AGHFP */
typedef struct
{
	aghfp_call_type		call_type;		/* Type of call - incoming, outgoing or transfer */
	bool				in_band;		/* Indicates if incoming call is to use in-band ring tones */
	aghfp_call_details  call_details;
} aghfp_call_params;


typedef enum
{
    aghfp_audio_disconnected,
	aghfp_audio_codec_connect,
    aghfp_audio_connecting_esco,
    aghfp_audio_connecting_sco,
    aghfp_audio_accepting,
    aghfp_audio_disconnecting,
    aghfp_audio_connected
} aghfp_audio_connect_state;

typedef enum
{
	aghfp_negotiate_undefined,
	aghfp_negotiate_audio_at_hf,
	aghfp_negotiate_audio_at_ag,
	aghfp_negotiate_no_audio
} aghfp_wbs_negotiate_action;

typedef enum
{
    aghfp_audio_settings_none,
    aghfp_audio_settings_app,
    aghfp_audio_settings_t1,
    aghfp_audio_settings_s4,
    aghfp_audio_settings_s1,
    aghfp_audio_settings_d0	 
} aghfp_attempt_audio_settings;

struct __AGHFP
{
	TaskData					task;
	Task						client_task;
	aghfp_state					state;
	aghfp_call_setup_status		call_setup_status;
	aghfp_profile				supported_profile;
	aghfp_supported_features	supported_features;		/* A record of what was passed into AghfpInit */
	aghfp_hf_supported_features	hf_supported_features;	/* A record of what the HF sent in its AT+BRSF */
	aghfp_features_status		features_status;		/* The current status of features that can be enabled/disabled at run time */
    aghfp_indicator_mask        active_indicators;      /* Mask giving the indicators currently active (all enabled or whatever was in the last AT+BIA from HF) */
	uint8						local_rfc_server_channel;
	unsigned int				wbs_lm_sco_handle:8;
    unsigned int				indicators_enable:1;
    unsigned int                link_mode_secure:1;
	uint32						sdp_record_handle;
	uint32						rx_bandwidth;
	uint32						tx_bandwidth;
	sync_link_type				link_type;
	Sink						audio_sink;
    aghfp_audio_connect_state 	audio_connection_state;
    sync_pkt_type           	audio_packet_type;
    aghfp_audio_params			audio_params;
    sync_pkt_type               audio_packet_type_to_try;
	Sink						rfcomm_sink;
	uint8						*mapped_rfcomm_sink;	/* Used by aghfp_send_data.c */
	
	bool						service_state;			/* Stores whether or not there currently is network service (ie GSM) */

	aghfp_codecs_info			codecs_info;
	unsigned int				use_wbs:1;
	unsigned int				wbs_negotiate_action:2; /* AG Started WB-Speech connection process */
	unsigned int				hf_codecs:3; /* WB-Speech codecs supported by the HF (remote).  Only 2 codecs in the spec. */
    unsigned int				use_codec:3; /* Codecs being requested for WB-Speech.  Only 2 codecs in the spec. */
	unsigned int				codec_to_negotiate:3;
	unsigned int				negotiation_type:3;
	unsigned int				app_pending_codec_negotiation:1;
	
	
	aghfp_call_progress			call_progress;
	aghfp_call_flags			call_flags;
	aghfp_call_params			call_params;
	uint16						ring_repeat_interval;	/* In milliseconds. Default 5000. */
    uint16                      rfcomm_lock;
    
    bool                        cind_poll_client ;
    uint16                      rfc_channel;
    aghfp_attempt_audio_settings  ag_attempt_settings;
    aghfp_hf_indicator_info        ag_hf_indicators_info; /* HF Indicators supported by AG */
    aghfp_hf_indicators_mask    hf_indicators; /* HF Indicators supported by the HF (remote). */

    uint16 ag_codec_modes;      /* Qualcomm Codec Extensions - AG supported modes. */
    uint16 hf_codec_modes;      /* Qualcomm Codec Extensions - HF supported modes. */
    uint16 qce_codec_mode_id;   /* Qualcomm Codec Extensions - agreed Codec Mode Id (otherwise 0xFFFF) */
};


#endif /* AGHFP_PRIVATE_H_ */
