/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

*/

/*!
@file	aghfp.h
@brief	Header file for the Audio Gateway Hands Free Profile library.

		This file documents the Audio Gateway Hands Free Profile library
		API for BlueLab3.
		
*/

#ifndef AGHFP_H_
#define AGHFP_H_

#include <library.h>
#include <bdaddr_.h>
#include <connection.h>
#include <message.h>


/*#define DISPLAY_AT_CMDS 1*/


/*!	@brief The AGHFP profile structure. */
typedef struct __AGHFP AGHFP;

/* The maximum number of codecs the HFP can handle. */
#define AGHFP_MAX_NUM_CODECS	(5)

/* HF indicators as defined by HF Assigned number */
#define AGHFP_MAX_HF_INDICATORS	(2)

/*!  Aghfp Flag Defines

	These flags can be or'd together and used as the supported_features passed
    into AghfpInit. These values correspond to those defined for use with the
    +BRSF AT command in the HFP spec.
*/
typedef enum
{
	aghfp_three_way_calling = 0x1, 		/*!	@brief Device supports three way call control. */
	aghfp_nrec_function = 0x2,			/*!	@brief Device implements noise reduction / echo cancellation. */
	aghfp_voice_recognition = 0x4,		/*!	@brief Device supports voice recognition. */
	aghfp_inband_ring = 0x8,			/*! @brief Device can send inband ring tones. */
	aghfp_attach_phone_number = 0x10,	/*!	@brief Device can attach a phone number to a voice tag. */
	aghfp_incoming_call_reject = 0x20,	/*! @brief Device implements incoming call reject. */
	aghfp_enhanced_call_status = 0x40,  /*! @brief Setting this flag indicates that this device implements enhanced call status. */
	aghfp_enhanced_call_control = 0x80, /*! @brief Setting this flag indicates that this device implements enhanced call control. */
	aghfp_extended_error_codes = 0x100, /*! @brief Setting this flag indicates that this device implements extended error result codes. */
	aghfp_codec_negotiation = 0x200,    /*! @brief Setting this flag indicates that this supports codec negotiation. */
	aghfp_hf_indicators = 0x400,        /*!brief Setting this flag indicates that this supports hf indicators support. */
	aghfp_esco_s4_supported = 0x800     /*! @brief Setting this flag indicates that this supports eSCO S4 audio settings */
} aghfp_supported_features;

/*! @brief This enum defines mask values to represent indicators sent to the HF */
typedef enum
{
    aghfp_service_indicator           = 0x01,
    aghfp_call_indicator              = 0x02,
    aghfp_call_setup_indicator        = 0x04,
    aghfp_call_held_indicator         = 0x08,
    aghfp_signal_strength_indicator   = 0x10,
    aghfp_roaming_status_indicator    = 0x20,
    aghfp_battery_charge_indicator    = 0x40,
    aghfp_all_indicators              = 0x7f
} aghfp_indicator_mask;

/*!	@brief The supported profiles. */
typedef enum
{
	aghfp_headset_profile,						/*!< As defined in part K6 of the Bluetooth specification. */
	aghfp_handsfree_profile,					/*!< As defined in version 1.0 of the handsfree profile specification. */
	aghfp_handsfree_15_profile,					/*!< As defined in version 1.5 of the handsfree profile specification. */
    aghfp_handsfree_16_profile,					/*!< As defined in version 1.6 of the handsfree profile specification. */
    aghfp_handsfree_17_profile					/*!< As defined in version 1.7 of the handsfree profile specification. */
} aghfp_profile;


/*! @brief Generic aghfp status. */
typedef enum
{
	aghfp_success,								/*!< Success. */
	aghfp_fail									/*!< Failure. */
} aghfp_lib_status;


/*!	@brief Return status for the AGHFP_INIT_CFM message */
typedef enum
{
	aghfp_init_success,							/*!< Successful initialisation. */
	aghfp_init_rfc_chan_fail,					/*!< Unsuccessful due to RFCOMM channel registration failure. */
	aghfp_init_sdp_reg_fail						/*!< Unsuccessful due to a service record registration failure. */
} aghfp_init_status;


/*!	@brief Return status for the AGHFP_SLC_CONNECT_CFM message */
typedef enum
{
	aghfp_connect_success,						/*!< Successful connection. */
	aghfp_connect_sdp_fail,						/*!< Unsuccessful due to a service search failure. */
	aghfp_connect_slc_failed,					/*!< Unsuccessful due to a service level connection failure. */
	aghfp_connect_failed_busy,					/*!< Unsuccessful due to service level connection already established. */
    aghfp_connect_failed,						/*!< Unsuccessful due to RFCOMM connection failing to be established. */
    aghfp_connect_server_channel_not_registered,/*!< Unsuccessful due to attempt to connect to unallocated server channel. */
    aghfp_connect_timeout,						/*!< Unsuccessful due to connection attempt timing out. */
    aghfp_connect_rejected,						/*!< Unsuccessful due to remote device rejecting connection. */
    aghfp_connect_normal_disconnect,			/*!< Unsuccessful due to remote device terminating the connection. */
    aghfp_connect_abnormal_disconnect,			/*!< Unsuccessful due to an abnormal disconnect while establishing an rfcomm connection. */
    aghfp_connect_sdp_fail_no_connection,   	/*!< Service search failed as connection to remote device couldn't be made. */
    aghfp_connect_sdp_fail_no_data,       	    /*!< Service search failed as remote device didn't return the requested data. */
    aghfp_connect_security_reject			    /*!< Unsuccessful due to rejected security. */
} aghfp_connect_status;


/*!	@brief Return status for the AGHFP_SLC_DISCONNECT_IND message */
typedef enum
{
	aghfp_disconnect_success,					/*!< Successful disconnection. */
	aghfp_disconnect_link_loss,					/*!< Unsuccessful due to abnormal link loss. */
	aghfp_disconnect_no_slc,					/*!< Unsuccessful due to no current connection. */
	aghfp_disconnect_timeout,					/*!< Unsuccessful due to RFCOMM connection attempt timeout. */
	aghfp_disconnect_error						/*!< Unsuccessful due to RFCOMM connection attempt error. */
} aghfp_disconnect_status;


/*!
	@brief Return status for the AGHFP_CALL_MGR_CREATE_CFM message.
*/
typedef enum
{
    /*! Successful call creation.*/
    aghfp_call_create_success,
    /*! Unsuccessful due to failure indication from firmware.*/
    aghfp_call_create_failure,
    /*! Unsuccessful due to a call create/terminate already being attempted.*/
    aghfp_call_create_in_progress,
    /*! Unsuccessful due to call terminate being issued.*/
    aghfp_call_create_aborted,
    /*! Unsuccessful due to slc being removed.*/
    aghfp_call_create_slc_removed,
    /*! Unsuccessful due to audio already existsing.*/
    aghfp_call_create_have_audio,
    /*! Unsuccessful due to one or more parameters specified being invalid.*/
    aghfp_call_create_invalid_params,
    /*! Unsuccessful due to audio handler currently opening/closing an audio channel.*/
    aghfp_call_create_audio_handler_active,
    /*! Unsuccessful due to library being in incorrect state.*/
    aghfp_call_create_error
} aghfp_call_create_status;


/*!
	@brief Return status for the AGHFP_CALL_MGR_TERMINATE_IND message.
*/
typedef enum
{
    /*! Successful call termination.*/
    aghfp_call_terminate_success,
    /*! Unsuccessful due to failure indication from firmware.*/
    aghfp_call_terminate_failure,
    /*! Unsuccessful due to library being in incorrect state.*/
    aghfp_call_terminate_error
} aghfp_call_terminate_status;


/*!
	@brief Return status for the AGHFP_AUDIO_CONNECT_CFM message.
*/
typedef enum
{
    /*! Successful audio connection.*/
    aghfp_audio_connect_success,
    /*! Unsuccessful due to negotiation failure.*/
    aghfp_audio_connect_failure,
    /*! Unsuccessful due to audio already being with device.*/
    aghfp_audio_connect_have_audio,
    /*! Unsuccessful due to an audio connect already being attempted.*/
    aghfp_audio_connect_in_progress,
    /*! Unsuccessful due to one or more parameters specified being invalid.*/
    aghfp_audio_connect_invalid_params,
    /*! Unsuccessful due to Call Manager setting up/shutting down a call (and hence audio).*/
    aghfp_audio_connect_call_manager_active,
    /*! Unsuccessful due to library being in incorrect state.*/
    aghfp_audio_connect_error,
    /*! Unsuccessful due to a Wide Band Speech Error. */
    aghfp_audio_connect_wbs_fail
} aghfp_audio_connect_status;


/*!
	@brief Return status for the AGHFP_AUDIO_DISCONNECT_IND message.
*/
typedef enum
{
    /*! Successful audio disconnection.*/
    aghfp_audio_disconnect_success,
    /*! Unsuccessful due to failure indication from firmware.*/
    aghfp_audio_disconnect_failure,
    /*! Unsuccessful due to audio being with AG.*/
    aghfp_audio_disconnect_no_audio,
    /*! Unsuccessful due to an audio disconnect already being attempted.*/
    aghfp_audio_disconnect_in_progress,
    /*! Unsuccessful due to Call Manager setting up/shutting down a call (and hence audio).*/
    aghfp_audio_disconnect_call_manager_active,
    /*! Unsuccessful due to library being in incorrect state.*/
    aghfp_audio_disconnect_error
} aghfp_audio_disconnect_status;


/*!
	@brief Transfer direction for audio connection.
*/
typedef enum
{
	/*! Transfer the audio to the HFP device.*/
	aghfp_audio_to_hfp,
	/*! Transfer the audio to the audio gateway.*/
	aghfp_audio_to_ag,
	/*! Toggle direction of current audio.*/
	aghfp_audio_transfer
} aghfp_audio_transfer_direction;


/*! @brief Service indicator parameter. Used in the function AghfpSendServiceIndicator */
typedef enum
{
	aghfp_service_none,							/*!< Implies no service. No Home/Roam network available. */
	aghfp_service_present						/*!< Implies present of service. Home/Roam network available. */
} aghfp_service_availability;


/*! @brief Call indicator parameter. Used in the function AghfpSendCallIndicator */
typedef enum
{
	aghfp_call_none,							/*!< Means no call active. */
	aghfp_call_active							/*!< Means a call is active. */
} aghfp_call_status;


/*! @brief Call setup indicator parameter. Used in the function AghfpSendCallSetupIndicator */
typedef enum
{
	aghfp_call_setup_none,						/*!< Means not currently in call setup. */
	aghfp_call_setup_incoming,					/*!< Means an incoming call process ongoing. */
	aghfp_call_setup_outgoing,					/*!< Means an outgoing call setup is ongoing. */
	aghfp_call_setup_remote_alert				/*!< Means remote part being alerted in an outgoing call. */
} aghfp_call_setup_status;


/*! @brief Call held indicator parameter */
typedef enum
{
    aghfp_call_held_none,
    aghfp_call_held_active,
    aghfp_call_held_remaining
} aghfp_call_held_status;


/*! @brief Roaming status indicator parameter */
typedef enum
{
    aghfp_roam_none,
    aghfp_roam_active
} aghfp_roam_status;


typedef struct
{
    uint16  id;
    uint8   type;
    uint8   service;
    uint16  size_number;
	uint8   *number;
} aghfp_subscriber_info;


typedef enum
{
    aghfp_response_hold_incoming_held,
    aghfp_response_hold_held_accepted,
    aghfp_response_hold_held_rejected
} aghfp_response_hold_state;


typedef enum
{
    aghfp_hold_incoming,
    aghfp_accept_held,
    aghfp_reject_held
} aghfp_response_hold_cmd;


typedef enum
{
    aghfp_call_dir_outgoing,
    aghfp_call_dir_incoming
} aghfp_call_dir;


typedef enum
{
    aghfp_call_state_active,
    aghfp_call_state_held,
    aghfp_call_state_dialling,
    aghfp_call_state_alerting,
    aghfp_call_state_incoming,
    aghfp_call_state_waiting
} aghfp_call_state;


typedef enum
{
    aghfp_call_mode_voice,
    aghfp_call_mode_data,
    aghfp_call_mode_fax
} aghfp_call_mode;


typedef enum
{
    aghfp_call_not_mpty,
    hghfp_call_is_mpty
} aghfp_call_mpty;

/*!
	@brief WB-Speech codec bit masks.
	
	Currently, AMR-WB is unsupported
*/
typedef enum
{
	wbs_codec_invalid	= 0,
	/*! SBC Codec. SBC support is mandatory. */
	wbs_codec_cvsd 	    = (1 << 0),
	/*! SBC Codec. SBC support is mandatory. */
	wbs_codec_msbc 	    = (1 << 1)
} aghfp_wbs_codec;

/*!
    @brief HF Indicators bit masks.
*/
typedef enum
{
    /*! No HF indicator. */
    aghfp_indicator_mask_none           = 0,
    /*! Enhanced safety indicator. */
    aghfp_enhanced_safety_mask       = (1 << 0),
    /*! Battery level indicator. */
    aghfp_battery_level_mask         = (1 << 1)
} aghfp_hf_indicators_mask;

/*! @brief  Assigned numbers for HF indicators as per
  https://www.bluetooth.org/en-us/specification/assigned-numbers/hands-free-profile
*/
typedef enum
{
    aghfp_hf_indicators_invalid  = 0x00,
    aghfp_enhanced_safety        = 0x01,
    aghfp_battery_level          = 0x02
} aghfp_hf_indicators_assigned_id;

/*!
    @brief Possible values indicators can take for hf_indicators
*/
typedef enum
{
    aghfp_hf_indicator_off    = 0,
    aghfp_hf_indicator_on     = 1
} aghfp_hf_indicator_state;

/*!
   @brief HF indicators supported by AG
*/
typedef struct
{
    aghfp_hf_indicators_mask   active_hf_indicators; /* HF Indicators supported by AG*/
    uint16 hf_indicators_state; /* bit mask of hf indicators state */
}aghfp_hf_indicator_info;

typedef enum
{
	aghfp_codec_negotiation_type_none,
	aghfp_codec_negotiation_type_wbs,
	aghfp_codec_negotiation_type_csr
} aghfp_codec_negotiation_type;

typedef struct
{
    uint16              idx;
    aghfp_call_dir      dir;
    aghfp_call_state    status;
    aghfp_call_mode     mode;
    aghfp_call_mpty     mpty;
    uint8               type;
    uint16              size_number;
    uint8              *number;
} aghfp_call_info;


/*! @brief Connection parameters for setting up an eSCO/SCO connection */
typedef struct
{
   uint32            bandwidth;
   uint16            max_latency;
   uint16            voice_settings;
   sync_retx_effort  retx_effort;
   /*! Do not use WB-Speech even if available */
   bool				 override_wbs;
} aghfp_audio_params;


typedef enum
{
	aghfp_call_type_incoming,		/* Create new incoming call to HS/HF */
	aghfp_call_type_outgoing,		/* Create new outgoing call to HS/HF */
	aghfp_call_type_transfer		/* Route existing outgoing call to HS/HF */
} aghfp_call_type;


/*!
	@brief Structure defining user definable information regarding the AG's codecs.

*/
typedef struct
{
	uint8		num_codecs;
	uint16		ag_codecs;
	aghfp_wbs_codec		codec_ids[AGHFP_MAX_NUM_CODECS];
} aghfp_codecs_info;

/* Structure to hold one CSR feature as published in the AT+CSRFN command. */
typedef struct
{
    uint16          indicator;
    uint16          value;
} aghfp_csr_feature_indicator;

/*****************************************************************************/
/* MESSAGES                                                                  */
/*****************************************************************************/


/* Do not document this enum. */
#ifndef DO_NOT_DOCUMENT
typedef enum
{
	AGHFP_INIT_CFM = AGHFP_MESSAGE_BASE,

	/* Service Level Connection messages */
	AGHFP_SLC_CONNECT_CFM,
	AGHFP_SLC_CONNECT_IND,
	AGHFP_SLC_DISCONNECT_IND,

	/* SCO/eSCO messages */
	AGHFP_AUDIO_CONNECT_IND,
	AGHFP_AUDIO_CONNECT_CFM,
	AGHFP_AUDIO_DISCONNECT_IND,

	/* AT command received messages */
    AGHFP_HS_BUTTON_PRESS_IND,			/* AT+CKPD */
	AGHFP_HS_CALL_ANSWER_IND,			/* AT+CKPD */
	AGHFP_HS_CALL_HANG_UP_IND,			/* AT+CKPD */
	AGHFP_HS_AUDIO_REQUEST_IND,			/* AT+CKPD */
	AGHFP_ANSWER_IND,					/* ATA */
	AGHFP_CALL_HANG_UP_IND,				/* AT+CHUP */
	AGHFP_DIAL_IND,						/* ATD */
	AGHFP_MEMORY_DIAL_IND,				/* ATD */
	AGHFP_LAST_NUMBER_REDIAL_IND,		/* AT+BLDN */
	AGHFP_CALLER_ID_SETUP_IND,			/* AT+CLIP */  /* 0xf */
	AGHFP_CALL_WAITING_SETUP_IND,		/* AT+CCWA */	/* 0x10 */
	AGHFP_CALL_HOLD_IND,				/* AT+CHLD */
	AGHFP_NREC_SETUP_IND,				/* AT+NREC */
	AGHFP_VOICE_RECOGNITION_SETUP_IND,	/* AT+BVRA */
	AGHFP_PHONE_NUMBER_REQUEST_IND,		/* AT+BINP */
	AGHFP_TRANSMIT_DTMF_CODE_IND,		/* AT+VTS */
	AGHFP_SYNC_MICROPHONE_GAIN_IND,		/* AT+VGM */
	AGHFP_SYNC_SPEAKER_VOLUME_IND,		/* AT+VGS */
	AGHFP_RESPONSE_HOLD_STATUS_REQUEST_IND, /* AT+BTRH */
	AGHFP_SET_RESPONSE_HOLD_STATUS_IND, /* AT+BTRH */
	AGHFP_SUBSCRIBER_NUMBER_IND,        /* AT+CNUM */
	AGHFP_CURRENT_CALLS_IND,       	    /* AT+CLCC */
	AGHFP_NETWORK_OPERATOR_IND,         /* AT+COPS */
    AGHFP_INDICATORS_ACTIVATION_IND,    /* AT+BIA */
    AGHFP_CALL_INDICATIONS_STATUS_REQUEST_IND, /*AT+CIND?*/
    AGHFP_INDICATOR_EVENTS_REPORTING_IND,      /* AT+CMER */
    AGHFP_HF_INDICATORS_STATUS_REQUEST_IND, /*AT+BIND?*/
    AGHFP_HF_INDICATORS_VALUE_IND,      /*AT+BIEV*/
	AGHFP_UNRECOGNISED_AT_CMD_IND,

	/* Confirms for interface functions */
	AGHFP_SEND_SERVICE_INDICATOR_CFM,
	AGHFP_SEND_CALL_INDICATOR_CFM,		/* 0x1f */
	AGHFP_SEND_CALL_SETUP_INDICATOR_CFM,	/* 0x20 */
	AGHFP_SEND_CALL_HELD_INDICATOR_CFM,
	AGHFP_SEND_SIGNAL_INDICATOR_CFM,
	AGHFP_SEND_ROAM_INDICATOR_CFM,
	AGHFP_SEND_BATT_CHG_INDICATOR_CFM,
	AGHFP_SEND_RING_ALERT_CFM,
	AGHFP_SEND_CALLER_ID_CFM,
	AGHFP_INBAND_RING_ENABLE_CFM,
	AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM,
	AGHFP_VOICE_RECOGNITION_ENABLE_CFM,
	AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM,
	AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM,
	AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM,
	AGHFP_SEND_RESPONSE_HOLD_STATE_CFM,
    AGHFP_CONFIRM_RESPONSE_HOLD_STATE_CFM,
    AGHFP_SUBSCRIBER_NUMBER_CFM,
    AGHFP_CURRENT_CALLS_CFM,			/* 0x30 */
    AGHFP_NETWORK_OPERATOR_CFM,
    AGHFP_USER_DATA_CFM,
    AGHFP_SEND_ERROR_CFM,
    AGHFP_SEND_OK_CFM,
	AGHFP_SET_SERVICE_STATE_CFM,
	AGHFP_SET_CALLER_ID_DETAILS_CFM,
	AGHFP_SEND_BIND_INDICATOR_STATE_CFM,
	
	/* Call Manager Primitives */
    AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND,
    AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND,
    AGHFP_CALL_MGR_CREATE_CFM,
    AGHFP_CALL_MGR_TERMINATE_IND,

    /*csr2csr extensions*/
    AGHFP_CSR_SUPPORTED_FEATURES_IND,
    AGHFP_CSR_FEATURE_NEGOTIATION_IND,
    AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND,

    /* Codec Negotiation Extensions */
    AGHFP_APP_AUDIO_PARAMS_REQUIRED_IND,

    /* HF indicator messages */
    AGHFP_HF_INDICATORS_REPORT_IND,

	AGHFP_MESSAGE_TOP
} AghfpMessageId;

#endif /*end of DO_NOT_DOCUMENT*/


/* The upstream messages from the Aghfp profile library to the application
   are defined below. Each message has an aghfp parameter which is a
   pointer to the profile instance that the message applies to. */


/*!	@brief This message is generated as a result of a call to AghfpInit.

	The application creates a profile instance by calling AghfpInit. This
	function is responsible for creating a task for the profile instance,
	initialising that task and its associated state and performing all
	other initialisation related tasks, e.g. registering a service record.
	Once the initialisation is completed this message will be returned to the
	application.
*/
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance. This will point to a valid Aghfp profile
										 instance if the initialisation succeeded. If however, any part of the initialisation
										 process failed, the aghfp pointer will not be valid. */
	aghfp_init_status	status;		/*!< If the task was created and initialised successfully then this will be set to
									     aghfp_init_success otherwise it will be set to indicate why the initialisation failed. */
} AGHFP_INIT_CFM_T;


/*!	@brief This message returns the result of a call to AghfpSlcConnect.

	This message is sent to the application once a Service Level Connection has been established
	or the connect attempt has failed. It is sent to inform the application that the Service Level
	Connection establishment phase has completed. The application will receive this message regardless of
	whether it initiated the Service Level Connection establishment or whether this was done by
	the HF.
*/
typedef struct
{
	AGHFP					*aghfp;	 /*!< Pointer to aghfp profile instance that is handling the Service Level Connection. */
	aghfp_connect_status	status;  /*!< Indicates whether or not the connection was successfully established. */
    Sink                    rfcomm_sink; /*!< The RFCOMM sink. */
} AGHFP_SLC_CONNECT_CFM_T;


/*!	@brief This message is sent when a remote device wishes to establish a Service Level Connection.

	This message is sent to the application to indicate that a remote device
	(the HF) is requesting to establish a Service Level Connection to this device.
	The application can respond to this message using the function AghfpSlcConnectResponse.
	The application must respond to this message even if it wants to reject
	the incoming Service Level Connection.
*/
typedef struct
{
	AGHFP	*aghfp;					/*!< Pointer to aghfp profile instance that received the incoming Service Level Connection request. */
	bdaddr	bd_addr;      			/*!< The Bluetooth address of the remote device. */
} AGHFP_SLC_CONNECT_IND_T;


/*!	@brief This message is sent when a Service Level Connection has been disconnected.

	This message informs the application that the Service Level Connection has been disconnected.
	This disconnect can be the result of a locally initiated action, a
	disconnect initiated from the HF or an abnormal disconnect e.g. link
	loss.
*/
typedef struct
{
	AGHFP					*aghfp;	 /*!< Pointer to the aghfp profile instance that is handling the Service Level Connection. */
	aghfp_disconnect_status	status;  /*!< Indicates the reason for the disconnect. */
} AGHFP_SLC_DISCONNECT_IND_T;


/*!	@brief This message is sent when a remote device wishes to establish audio (SCO/eSCO) connection.

	The application can respond to the message using the function AghfpAudioConnectResponse.
	The application should respond to this message even if it wants to reject the incoming
	connection request.
*/
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that is handling the Service Level Connection. */
	bdaddr				bd_addr;	/*!< Bluetooth address of remote device. */
} AGHFP_AUDIO_CONNECT_IND_T;


/*!	@brief This message informs the app that an audio (SCO/eSCO) has been connected or that the
	attempt to open an audio connection has failed.
*/
typedef struct
{
	AGHFP				*aghfp;			/*!< Pointer to the aghfp profile instance that is handling the Service Level Connection. */
	aghfp_audio_connect_status	status;	/*!< Error code indicating success or reason for failure. */
	hci_status			cl_status;		/*!< Contains the status returned by the connection library in the CL_DM_SYNC_CONNECT_CFM. */
	sync_link_type		link_type;		/*!< Indicates whether a SCO or eSCO link was created. */
	Sink				audio_sink;		/*!< If a SCO/eSCO connection has been created the audio_sink will be the corresponding sink
										 	 for the SCO/eSCO data, otherwise it will be set to zero. */
	uint32				rx_bandwidth;	/*!< Receive bandwith in bytes per second. */
	uint32				tx_bandwidth;	/*!< Transmit bandwith in bytes per second. */
	bool				using_wbs; 		/*!< Whether the eSCO connection is using Wide Band Speech. */
	uint8				wbs_codec; 		/*!< If using Wide Band Speech, which codec is being used. */
    uint16              qce_codec_mode_id;             /*! QCE Codec Mode ID Selected. */
} AGHFP_AUDIO_CONNECT_CFM_T;


/*!	@brief This message informs the app that the audio connection (SCO/eSCO) has been disconnected. */
typedef struct
{
	AGHFP				*aghfp;		 		/*!< Pointer to the aghfp profile instance that is handling the Service Level Connection. */
	aghfp_audio_disconnect_status	status; /*!< Error code indicating success or reason for failure. */
} AGHFP_AUDIO_DISCONNECT_IND_T;


/******************************************************************************
 Start of AT command indicators
*/

/*!	@brief This message informs the app that the HS has pressed a button. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the AT+CKPD command. */
} AGHFP_HS_BUTTON_PRESS_IND_T;

/*!	@brief This message informs the app that the HS has answered the call. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the AT+CKPD command. */
} AGHFP_HS_CALL_ANSWER_IND_T;


/*!	@brief This message informs the app that the HS wants to end the call. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the AT+CKPD command. */
} AGHFP_HS_CALL_HANG_UP_IND_T;


/*!	@brief This message informs the app that the HS has requested an audio channel. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the AT+CKPD command. */
} AGHFP_HS_AUDIO_REQUEST_IND_T;


/*!	@brief This message informs the app that the HF has answered the call. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the ATA indicator. */
} AGHFP_ANSWER_IND_T;


/*! @brief This message informs the app that the HF wants to perform some action on the waiting call. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the AT+CHLD message. */
	uint16				action;		/*!< The action code specified in the AT+CHLD command that was received. */
	uint16				index;		/*!< The call index specified in the AT+CHLD command that was received. */
} AGHFP_CALL_HOLD_IND_T;


/*! @brief This message informs the app that the HF wants to enable or disable indicator event reporting. */
typedef struct
{
    AGHFP *aghfp;   /*!< Pointer to the aghfp profile instance that received the AT+CMER message. */
    uint16 mode;    /*!< The mode contained in the AT+CMER command. */
    uint16 ind;     /*!< The ind contained in the AT+CMER command. */
} AGHFP_INDICATOR_EVENTS_REPORTING_IND_T;


/*!	@brief This message contains the strings that the AT command parser cannot parse.

	The strings that the auto generated AT command parser supports are
	specified in aghfp_parse.parse along with the handler function to be
	called once a command has been parsed. However, if the parser cannot
	parse a particular AT string it will pass it to the application for
	further processing.
*/
typedef struct
{
	AGHFP	*aghfp;					/*!< Pointer to the aghfp profile instance that is handling the Service Level Connection. */
	uint16	size_data;				/*!< The number of bytes pointed to by data. */
	uint8	data[1];				/*!< The data that could not be parsed. Although the array size is specified a 1, the actual size can be much larger - ie this structure has a variable size, depending on the amount of data. If the client needs access to this data after the message has been destroyed it is the client's responsibility to copy it. */
} AGHFP_UNRECOGNISED_AT_CMD_IND_T;


/*! @brief This message informs the app that the HF has rejected the call */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the call reject message. */
} AGHFP_CALL_HANG_UP_IND_T;


/*! @brief This message informs the app that the HF has dialed a number */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the dial message. */
	uint16				size_number;/*!< The number of digits (and special characters like '+') in the number. */
	uint8 				number[1];	/*!< Pointer to the number to dial. Although the array size is specified a 1, the actual size can be much larger - ie this structure has a variable size, depending on the amount of data. If the client needs access to this data after the message has been destroyed it is the client's responsibility to copy it. */
} AGHFP_DIAL_IND_T;


/*! @brief This message informs the app that the HF has dialed a number */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the memory dial message. */
	uint16				size_number;/*!< The number of digits in the slot number (and special characters like '+') in the number. */
	uint8 				number[1];	/*!< Pointer to the slot number to dial. Although the array size is specified a 1, the actual size can be much larger - ie this structure has a variable size, depending on the amount of data. If the client needs access to this data after the message has been destroyed it is the client's responsibility to copy it. */
} AGHFP_MEMORY_DIAL_IND_T;


/*! @brief This message informs the app that the HF has request a last number redial */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the last number redial request. */
} AGHFP_LAST_NUMBER_REDIAL_IND_T;


/*! @brief This message informs the app that the HF wants to enable/disable caller ID notifications */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the setup request. */
	bool				enable;		/*!< Whether to enable or disable caller ID notifications. */
} AGHFP_CALLER_ID_SETUP_IND_T;


/*! @brief This message informs the app that the HF wants to enable/disable call waiting notifications */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the setup request. */
	bool				enable;		/*!< Whether to enable or disable call waiting notifications. */
} AGHFP_CALL_WAITING_SETUP_IND_T;


/*! @brief This message informs the app that the HF wants to enable/disable noise reduction and echo canceling in the AG. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the setup request. */
	bool				enable;		/*!< Whether to enable or disable noise reduction and echo canceling. */
} AGHFP_NREC_SETUP_IND_T;


/*! @brief This message informs the app that the HF wants to enable/disable voice recognition in AG. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the setup request. */
	bool				enable;		/*!< Whether to enable or disable voice recognition. */
} AGHFP_VOICE_RECOGNITION_SETUP_IND_T;


/*! @brief This message informs the app that the HF has requested a phone number corresponding to a voice print it has just recorded. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the setup request. */
} AGHFP_PHONE_NUMBER_REQUEST_IND_T;


/*! @brief This message informs the app that the HF has requested that a DTMF code be transmitted by the AG. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the DTMF transmit request. */
	uint8				code;		/*!< The single character DTMF code to transmit, may be 0-9, A-D, # or * */
} AGHFP_TRANSMIT_DTMF_CODE_IND_T;


/*! @brief This message informs the app that the HF has sent a microphone gain synchronisation message. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronistaion message. */
	uint8				gain;		/*!< The gain value received from the HF. */
} AGHFP_SYNC_MICROPHONE_GAIN_IND_T;


/*! @brief This message informs the app that the HF has sent a speaker volume synchronisation message. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
	uint8				volume;		/*!< The gain value received from the HF. */
} AGHFP_SYNC_SPEAKER_VOLUME_IND_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
} AGHFP_RESPONSE_HOLD_STATUS_REQUEST_IND_T;


typedef struct
{
	AGHFP				    *aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
	aghfp_response_hold_cmd cmd;        /*!< Response Hold command HF wants performed. */
} AGHFP_SET_RESPONSE_HOLD_STATUS_IND_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
	uint16              last_id;    /*!< Application specific id of last subscriber information sent - will be zero on initial request. */
} AGHFP_SUBSCRIBER_NUMBER_IND_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
	uint16              last_idx;   /*!< Index of last call information sent - will be zero on initial request. */
} AGHFP_CURRENT_CALLS_IND_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
} AGHFP_NETWORK_OPERATOR_IND_T;


typedef struct
{
    AGHFP                *aghfp;		    /*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
    aghfp_indicator_mask active_indicators; /*!< Mask indicating which indicators are active. */
} AGHFP_INDICATORS_ACTIVATION_IND_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the volume synchronisation message. */
}AGHFP_CALL_INDICATIONS_STATUS_REQUEST_IND_T;

typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the hf indicator request. */
    aghfp_hf_indicators_mask   hf_indicators; /*!< Mask indicating which indicators HF supports */
}AGHFP_HF_INDICATORS_REPORT_IND_T;

typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the hf indicator request. */
}AGHFP_HF_INDICATORS_STATUS_REQUEST_IND_T;

typedef struct
{
    AGHFP           *aghfp ;      /*!< Pointer to the aghfp profile instance that received the hf indicator request. */
    uint16          hf_indicator_assigned_num;
    uint16          value;
}AGHFP_HF_INDICATORS_VALUE_IND_T ;
/*
 End of AT command indicators
*******************************************************************************/

/*! @brief This message is sent in response to a call of AghfpSendServiceIndicator. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send service indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a service indicator then the
										 status is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_SERVICE_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a call of AghfpSendCallIndicator. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call indicator then the status
										 is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_CALL_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a call of AghfpSendCallSetupIndicator. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call setup indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call setup indicator then the
										 status is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_CALL_SETUP_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a request from the application to
    send a call held indicator. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call setup indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call setup indicator then the
										 status is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_CALL_HELD_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a call of AghfpSendRingAlert. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call setup indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call setup indicator then the
										 status is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_SIGNAL_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a request from the application to
    send a roaming status indicator. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call setup indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call setup indicator then the
										 status is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_ROAM_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a request from the application to
    send a battery charge indicator. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call setup indicator
										 request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call setup indicator then the
										 status is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_BATT_CHG_INDICATOR_CFM_T;


/*! @brief This message is sent in response to a request from the application to
    send a ring alert. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send ring alert request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a ring alert then the status
										 is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_RING_ALERT_CFM_T;


/*! @brief This message is sent in response to a call of AghfpSendCallerId. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send caller ID request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send the caller ID then the status
										 is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_SEND_CALLER_ID_CFM_T;


/*! @brief This message is sent in response to a call of AghfpInbandRingToneEnable. */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the enable/disable request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a ring alert then the status
										 is set to aghfp_success. Otherwise the status is set to aghfp_fail. */
} AGHFP_INBAND_RING_ENABLE_CFM_T;


/*! @brief This message informs the app whether the AghfpSendCallWaitingNotification was successful */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send call waiting notification request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a call waiting notification,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM_T;


/*! @brief This message informs the app whether the AghfpVoiceRecognitionEnable was successful */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the voice recognition enable request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a voice recognition enable message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_VOICE_RECOGNITION_ENABLE_CFM_T;

/*! @brief This message informs the app whether the AghfpSendPhoneNumberForVoiceTag was successful */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the send phone number request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a phone number for voice tag,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM_T;

/*! @brief This message informs the app whether the AghfpSetRemoteMicrophoneGain was successful */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote microphone gain request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set microphone gain message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM_T;


/*! @brief This message informs the app whether the AghfpSetRemoteSpeakerVolume was successful */
typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote speaker volume request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set speaker volume message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote speaker volume request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set speaker volume message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SEND_RESPONSE_HOLD_STATE_CFM_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote speaker volume request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set speaker volume message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_CONFIRM_RESPONSE_HOLD_STATE_CFM_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote speaker volume request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set speaker volume message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SUBSCRIBER_NUMBER_CFM_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote speaker volume request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set speaker volume message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_CURRENT_CALLS_CFM_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that received the set remote speaker volume request. */
	aghfp_lib_status	status;		/*!< If the library was in a suitable state to send a set speaker volume message,
										 then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_NETWORK_OPERATOR_CFM_T;


/*! @brief This message is sent in response to the app calling the AghfpSendUserSpecificData function */
typedef struct
{
    AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_lib_status	status;		/*!< If data was sent successfully to the remote device, then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_USER_DATA_CFM_T;


/*! @brief This message is sent in response to the app calling the AghfpSendError function */
typedef struct
{
    AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_lib_status	status;		/*!< If data was sent successfully to the remote device, then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SEND_ERROR_CFM_T;


/*! @brief This message is sent in response to the app calling the AghfpSendOk function */
typedef struct
{
    AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_lib_status	status;		/*!< If data was sent successfully to the remote device, then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SEND_OK_CFM_T;


/*! @brief This message is sent in response to the app calling the AghfpSetServiceState function */
typedef struct
{
    AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_lib_status	status;		/*!< If the state was set successfully then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SET_SERVICE_STATE_CFM_T;


/*! @brief This message is sent in response to the app calling the AghfpSetCallerIdDetails function */
typedef struct
{
    AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_lib_status	status;		/*!< If the state was set successfully then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
} AGHFP_SET_CALLER_ID_DETAILS_CFM_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance. */
	Sink				audio_sink;	/*!< Sink of SCO/eSCO connection being used for the in-band ring tone. Will be set to NULL if no audio channel is used. */
} AGHFP_CALL_MGR_CREATE_WAITING_RESPONSE_IND_T;


typedef struct
{
	AGHFP				*aghfp;		/*!< Pointer to the aghfp profile instance. */
	Sink				audio_sink;	/*!< Sink of SCO/eSCO connection being used for the remote alert indications. Will be set to NULL if no audio channel is used. */
} AGHFP_CALL_MGR_CREATE_ALERTING_REMOTE_IND_T;


typedef struct
{
    AGHFP				*aghfp;			/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_call_create_status	status;	/*!< If data was sent successfully to the remote device, then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
    sync_link_type		link_type;		/*!< Indicates whether a SCO or eSCO link was created. */
	Sink				audio_sink;		/*!< Sink of SCO/eSCO connection being used for the call. Will be set to NULL if no audio channel is used. */
	uint32				rx_bandwidth;	/*!< Receive bandwith in bytes per second. */
	uint32				tx_bandwidth;	/*!< Transmit bandwith in bytes per second. */
	bool				using_wbs; 		/*!< Whether the eSCO connection is using Wide Band Speech. */
	uint8				wbs_codec; 		/*!< If using Wide Band Speech, which codec is being used. */
} AGHFP_CALL_MGR_CREATE_CFM_T;


typedef struct
{
    AGHFP				*aghfp;			/*!< Pointer to the aghfp profile instance that handled the request. */
	aghfp_call_terminate_status	status;	/*!< If data was sent successfully to the remote device, then status is set to aghfp_success. Otherwise status is set to aghfp_fail. */
	Sink				audio_sink;		/*!< If a SCO/eSCO connection remains after the call has ended then audio_sink will indicate this. */
} AGHFP_CALL_MGR_TERMINATE_IND_T;

typedef struct 
{
    AGHFP * aghfp;
	/*! Caller name supported by the HF. */
	bool callerName;
	/*! Unsolicited raw text supported by the HF. */
	bool rawText;
	/*! SMS Indications supported by the HF. */
	bool smsInd;
	/*! Battery level reporting supported by the HF. */
	bool battLevel;
	/*! Power source reporting supported by the HF. */
	bool pwrSource;
	/*! Bit mask of codecs supported by the HF */
	uint16 codecs;    
	/*! Indicates if the HF published the codecs bandwidth indicator in its AT+CSRSF command. */
	bool codec_bandwidths_present;
	/*! Bit mask of bandwidths supported by the HF */
	uint16 codec_bandwidths;    
}AGHFP_CSR_SUPPORTED_FEATURES_IND_T ;


typedef struct
{
    AGHFP * 					aghfp ;
    uint16                      num_csr_features;
    aghfp_csr_feature_indicator csr_features[2];
}AGHFP_CSR_FEATURE_NEGOTIATION_IND_T ;

typedef struct
{
    AGHFP * aghfp ;
}AGHFP_CSR_FEATURE_NEGOTIATION_REQ_IND_T ;

/*! @brief Inform the AG of Supported Features

	@param hfp A pointer to the profile instance.
	@param callerName HF Device supports the presentation of Caller Names in CLIP and CCWA
    @param rawText HF Device supports the presentation of raw text from a CSRTXT
    @param smsInd HF Device supports the receipt of SMS indications
    @param battLevel HF Device will send battery level indications (see HfpCsrPowerLevel)
    @param pwrSource HF Device will send power source indications (see HfpCsrPowerSource)
    @param codecs bitmask of the curpported codecs supported by the HF device 
    This function informs the AG which supported features that the HF supports. A HFP_CSR_SUPPORTED_FEATURES_CFM 
    message, containing the AG's supported features, will be received on completion.
    
    An SLC must be fully established before calling this function.
*/
void aghfpCsrSupportedFeaturesResponse (AGHFP *aghfp, bool callerName, bool rawText, bool smsInd, bool battLevel, bool pwrSource, uint16 codecs, bool codec_bandwidths_present, uint16 codec_bandwidths);


/*todo*/
void aghfpFeatureNegotiate ( AGHFP * aghfp , uint16 num_csr_features, uint16 indicator0, uint16 value0, uint16 indicator1, uint16 value1, bool sendLeadingOK );
 
/*****************************************************************************/
/* FUNCTIONS                                                                 */
/*****************************************************************************/


/*!	@brief Initialise an instance of the AGHFP library.

	@param theAppTask The current application task.
	@param aghfp_supported_profile Profile supported. Choose from the members
	       of the aghfp_profile enum.
	@param supported_features A bit mask containing the supported Audio Gateway
	       features (See the aghfp_supported_features enum).

	The application registers its own task, theAppTask, with the AGHFP library so
	that return messages can be routed to the correct task.

	The supported_features parameter is used to configure the profile this AGHFP
	instance will support. If HFP is being supported the supported
	features should also be supplied. These are passed in as a bit mask, as
	defined in the HFP specification. If the profile instance is being
	configured as HSP the supported features can be set to zero as they
	are not used.

	The application will receive an AGHFP_INIT_CFM message from the library
	to indicate the success or otherwise of the initialisation. */
void AghfpInit(Task theAppTask, aghfp_profile aghfp_supported_profile, uint16 supported_features);


/*!	@brief Initialise an instance of the AGHFP library with Qualcomm Codec
               Extensions (QCE) enabled.

	@param theAppTask The current application task.
	@param aghfp_supported_profile Profile supported. Choose from the members
	       of the aghfp_profile enum.
	@param supported_features A bit mask containing the supported Audio Gateway
	       features (See the aghfp_supported_features enum).
        @param codec_mode_bitmask A bit mask indicating which Qualcomm Codecs 
               the AGHFP support (see the codec_mode_bit_type enum). Setting this 
               parameter to 0 will disable QCE mode. 

	Otherwise, this function is the same as the AghfpInit() function.

	The application will receive an AGHFP_INIT_CFM message from the library
	to indicate the success or otherwise of the initialisation. */
void AghfpInitQCE(
        Task theAppTask, 
        aghfp_profile aghfp_supported_profile, 
        uint16 supported_features,
        uint16 codec_modes
        );

/*!	@brief Initiate the creation of a Service Level Connection.

	@param aghfp The profile instance which will be used to create the connection.
	@param bd_addr The address of the remote device (HF) to which the connection will be created.

	The application will receive an AGHFP_SLC_CONNECT_CFM message from the library
	to indicate the success or otherwise of the connection attempt. */
void AghfpSlcConnect(AGHFP *aghfp, const bdaddr *bd_addr);


/*!	@brief Respond to an AGHFP_SLC_CONNECT_IND message from the HF.

	@param aghfp The profile instance which will be used to create the connection.
	@param response Accept (TRUE) or reject (FALSE) the incoming connection attempt.

	If the HF initiates a Service Level Connection to the AG device the application
	will be notified about the incoming connection using a AGHFP_SLC_CONNECT_IND
	message. The application must respond to the message using this function.
	The profile instance to which the connection will be created is specified
	by aghfp. This must be the same as the profile instance in the
	AGHFP_SLC_CONNECT_IND message as the AG will be attempting to connect to a
	particular service on the local device.

	The application will receive an AGHFP_SLC_CONNECT_CFM message from the library
	to indicate the success or otherwise of the connection attempt. */
void AghfpSlcConnectResponse(AGHFP *aghfp, bool response);


/*!	@brief Initiate the disconnection of an Service Level Connection for a particular
	profile instance (aghfp).

	@param aghfp The profile instance which is to be disconnected.

	The application will receive an AGHFP_SLC_DISCONNECT_IND message from the library
	to indicate the success or otherwise of the disconnection attempt. */
void AghfpSlcDisconnect(AGHFP *aghfp);


/*!	@brief Transfer the audio connection either from the AG to the HF/HS or vice versa.

	@param hfp The profile instance we want the audio paths transfer performed on.
	@param direction The direction we want the audio connection transferred to.
	@param packet_type The packet type to be used on the audio connection.
	@param audio_params Used to specify bandwith, latency, voice setting and retransmission effort for eSCO packet types.

	The direction in which to attempt the audio transfer is specified by
	direction. This can be either from the AG to the headset (aghfp_audio_to_hf), from
	the headset to the AG (aghfp_audio_to_ag) or a request to transfer the audio to the
	other device to where it currently is (aghfp_audio_transfer) i.e. if the audio
	paths are currently routed to the headset then transfer the audio to the AG
	or vice versa.

	It is the application's responsibility to ensure that the remote device
	supports the requested packet type.

	The application will receive an AGHFP_AUDIO_CONNECT_CFM or
	AGHFP_AUDIO_DISCONNECT_IND message from the library depending on the direction
	of audio transfer.
*/
void AghfpAudioTransferConnection(AGHFP *aghfp, aghfp_audio_transfer_direction direction, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);


/*! @brief Request a SCO/eSCO connection.

    @param aghfp The profile instance which is to be connected.
	@param packet_type Bit mask containing all the packet types you wish to allow (See the sync_pkt_type enum in the Connection Library).
	@param audio_params Configuration parameters. Passing NULL will cause defaults to be used. If
	 non-NULL is passed the data will be copied. Therefore the caller may free it immediately
     after the function call returns.

    The application will receive an AGHFP_AUDIO_CONNECT_CFM from the library to indicate
    success or otherwise of the connection attempt. */
void AghfpAudioConnect(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);


/*! @brief Respond to a request for a SCO/eSCO connection.

    @param aghfp The profile instance which is to be connected.
	@param response TRUE to accept the connection, FALSE to reject.
	@param packet_type Bit mask containing all the packet types you wish to allow.
	@param audio_params Configuration parameters. Passing NULL will cause defaults to be used. If
	 non-NULL is passed the data will be copied. Therefore the caller may free it immediately
     after the function call returns.

    The application will receive an AGHFP_AUDIO_CONNECT_CFM from the library to indicate
    success or otherwise of the connection attempt. */
void AghfpAudioConnectResponse(AGHFP *aghfp, bool response, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);


/*! @brief Shutdown a SCO/eSCO connection.

    @param aghfp A pointer to the profile instance which is to be disconnected.

	The application will receive an AGHFP_AUDIO_DISCONNECT_IND from the library to indicate
    success or otherwise of the disconnection attempt. */
void AghfpAudioDisconnect(AGHFP *aghfp);

/*! @brief Set the audio params in the AG.

    @param aghfp A pointer to the profile instance which is to be disconnected.
	@param packet_type Bit mask containing all the packet types you wish to allow.
	@param audio_params Configuration parameters. Passing NULL will cause defaults to be used. If
	 non-NULL is passed the data will be copied. Therefore the caller may free it immediately
     after the function call returns.

	No message will be passed back to the app as a result of this function. */
void AghfpSetAudioParams(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);


/*! @brief Tell AGHFP to request CIND status information from the client application rather than responding automatically 

    @param aghfp A pointer to the profile instance which is to be disconnected.
    @param enable TRUE to turn polling on, FALSE to turn it off
    
    No message will be passed back to the app as a result of this function. 
    Any device which enables this functionality must handle AGHFP_CALL_INDICATIONS_STATUS_REQUEST_IND 
    messages or SLC connections will fail.
*/
void AghfpCindStatusPollClientEnable(AGHFP *aghfp, bool enable);


/*! @brief Respond to AGHFP_CALL_INDICATIONS_STATUS_REQUEST_IND 

    @param aghfp A pointer to the profile instance which is to be disconnected.
    @param availability Indicates whether network is available or not
    @param call_status Indicates whether there is an active call in place or not
    @param call_setup_status Indicates if a call setup is in progress or not (and in what direction)
    @param call_held_status Indicates whether a call is on hold or not
    @param signal Indicates the network signal strength
    @param roam_status Indicates the roam status
    @param batt Indicates the AG battery level
    
    No message will be passed back to the app as a result of this function. 
*/
void AghfpCallIndicatorsStatusResponse(AGHFP *aghfp, aghfp_service_availability availability, \
                                                     aghfp_call_status call_status, \
                                                     aghfp_call_setup_status call_setup_status, \
                                                     aghfp_call_held_status call_held_status, \
                                                     uint16 signal, \
                                                     aghfp_roam_status roam_status, \
                                                     uint16 batt);


/*! @brief Send a HF indicator state to the HF.

	@param aghfp A pointer to the profile instance.
	@param assigned_num HF indicator as per HF Assigned Number.
	@param state Whether enabled or disabled.

	The application will receive an AGHFP_SEND_BIND_INDICATOR_STATE_CFM from the library to
	indicate success or otherwise of the indicator state send attempt. */
void AghfpSendHfIndicatorState(AGHFP *aghfp, aghfp_hf_indicators_assigned_id assigned_num, aghfp_hf_indicator_state state);


/*! @brief Tell AGHFP to enable HF indicators by host app.

	@param aghfp A pointer to the profile instance.
	@param aghfp_hf_indicators HF indicators enabled .
	
    No message will be passed back to the app as a result of this function. */
void AghfpBindIndicatorEnable(AGHFP *aghfp, aghfp_hf_indicators_mask ag_hf_indicators);

/*! @brief Send default HF indicators state mask to the AGHFP library to be sent to HF.

	@param aghfp A pointer to the profile instance.
	@param hf_indicators_state_mask HF indicators values as per HF Assigned Number each
           bit indicating whether HF indicator is enabled/disabled.

	The application will receive an AGHFP_SLC_CONNECT_CFM from the library to
	indicate success or otherwise of the indicators state send attempt. */
void AghfpHfIndicatorsInitStatusResponse(AGHFP *aghfp, uint16 hf_indicators_state_mask);


/*! @brief Send a service indicator to the HF.

	@param aghfp A pointer to the profile instance.
	@param availability Whether or not a service is present.

	The application will receive an AGHFP_SEND_SERVICE_INDICATOR_CFM from the library to
	indicate success or otherwise of the indicator send attempt. */
void AghfpSendServiceIndicator(AGHFP *aghfp, aghfp_service_availability availability);


/*! @brief Send a call indicator to the HF.

	@param aghfp A pointer to the profile instance.
	@param status Either aghfp_call_none or aghfp_call_active (see the aghfp_call_status enum)

	The application will receive an AGHFP_SEND_CALL_INDICATOR_CFM from the library to
	indicate success or otherwise of the indicator send attempt. */
void AghfpSendCallIndicator(AGHFP *aghfp, aghfp_call_status status);


/*! @brief Send a call setup indicator to the HF.

	@param aghfp A pointer to the profile instance.
	@param status The call setup status (see the aghfp_call_setup_status enum)

	The application will receive an AGHFP_SEND_CALL_SETUP_INDICATOR_CFM from the library to
	indicate success or otherwise of the indicator send attempt. */
void AghfpSendCallSetupIndicator(AGHFP *aghfp, aghfp_call_setup_status status);


/*! @brief Send a call held indicator to the HF.
	@param aghfp A pointer to the profile instance. */
void AghfpSendCallHeldIndicator(AGHFP *aghfp, aghfp_call_held_status status);


/*! @brief Send a signal strength indicator to the HF.
	@param aghfp A pointer to the profile instance. */
void AghfpSendSignalIndicator(AGHFP *aghfp, uint16 level);


/*! @brief Send a roaming status indicator to the HF.
	@param aghfp A pointer to the profile instance. */
void AghfpSendRoamIndicator(AGHFP *aghfp, aghfp_roam_status status);


/*! @brief Send a battery charge indicator to the HF.
	@param aghfp A pointer to the profile instance. */
void AghfpSendBattChgIndicator(AGHFP *aghfp, uint16 level);


/*! @brief Send a ring alert to the HF.

	@param aghfp A pointer to the profile instance.

	The application will receive an AGHFP_SEND_RING_ALERT_CFM message from
	the library to indicate the success or otherwise of the ring send attempt. */
void AghfpSendRingAlert(AGHFP *aghfp);


/*! @brief Send caller ID to the HF.

	@param aghfp A pointer to the profile instance.
	@param type_number This parameter specifies the format of the phone number provided and can be one of the following values:
					   128-143=Unknown format. 144-159=International number. 160-175=National number.
					   See the description of "+CLIP" in HFP 1.5 spec for more details.
	@param size_number The number of characters in the phone number. (Not including any null terminator)
	@param number A uint8 array containing one digit of the phone number
	              per array element. The digits are in ASCII format and
	              may include special characters such as '+'. The number
	              should *NOT* be NULL terminated.
	@param size_string The number of characters in the string. (Not including any null terminator). Should be zero if string is NULL.
	@param string A friendly name to go with the phone number. Maybe NULL.

	Send the phone number of the remote party. This action is only valid
	immediately after sending a ring alert.

	The application will receive an AGHFP_SEND_CALLER_ID_CFM message from
	the library to indicate the success or otherwise of the callerID send attempt. */
void AghfpSendCallerId(AGHFP *aghfp, uint8 type_number, uint16 size_number, const uint8 *number, uint16 size_string, const uint8 *string);


/*! @brief Tell the HF that inband ring tones have been enabled/disabled.

	@param aghfp A pointer to the profile instance.
	@param enable Enable(TRUE) or Disable(FALSE)

    The application will receive an AGHFP_INBAND_RING_ENABLE_CFM message
    from the library to indicate the success or otherwise of the
    enable/disable attempt. */
void AghfpInBandRingToneEnable(AGHFP *aghfp, bool enable);


/*! @brief Tell the HF that there is a call waiting, while this is already
    an incoming call.

	@param aghfp A pointer to the profile instance.
	@param type_number The type of number being sent. Permitted values are 128 to 175 - see HFP 1.5 spec.
	@param size_number The number of characters in the phone number
	@param number The phone number of the waiting call. This is a uint8
				  array containing one digit of the phone number
	              per array element. The digits are in ASCII format and
	              may include special characters such as '+'. The number
	              should *NOT* be NULL terminated.
	@param size_string The number of characters in the string. (Not including any null terminator). Should be zero if string is NULL.
	@param string A friendly name to go with the phone number. Maybe NULL.

    The application will receive an AGHFP_SEND_CALL_WAITING_NOTIFICATION_CFM
    message from the library to indicate the success or otherwise of the
    notification send attempt. */
void AghfpSendCallWaitingNotification(AGHFP *aghfp, uint8 type_number, uint16 size_number, const uint8 *number, uint16 size_string, const uint8 *string);


/*! @brief Tell the HF to activate/deactivate its voice recognition function.

	@param aghfp A pointer to the profile instance.
	@param enable Whether to enable or disable voice recognition.

	The application will receive an AGHFP_VOICE_RECOGNITION_ENABLE_CFM to
	indicate the success of otherwise of sending this enable/disable. */
void AghfpVoiceRecognitionEnable(AGHFP *aghfp, bool enable);


/*! @brief Send a phone number to the HF in response to a AT+BNIP request.

	@param aghfp A pointer to the profile instance.
	@param size_number The number of characters in the phone number.
	@param number A uint8 array containing one digit of the phone number
	              per array element. The digits are in ASCII format and
	              may include special characters such as '+'. The number
	              should not be NULL terminated.

	The application will receive an AGHFP_SEND_PHONE_NUMBER_FOR_VOICE_TAG_CFM
	to indicate the success of otherwise of sending phone number. */
void AghfpSendPhoneNumberForVoiceTag(AGHFP *aghfp, uint16 size_number, const uint8 *number);


/*! @brief Set microphone gain on the HF.

	@param aghfp A pointer to the profile instance.
	@param gain The desired microphone gain. In the range 0-15 inclusive.

	The application will receive an AGHFP_SET_REMOTE_MICROPHONE_GAIN_CFM
	to indicate the success of otherwise of sending the microphone gain. */
void AghfpSetRemoteMicrophoneGain(AGHFP *aghfp, uint8 gain);


/*! @brief Set speaker gain on the HF.

	@param aghfp A pointer to the profile instance.
	@param volume The desired speaker volume. In the range 0-15 inclusive.

	The application will receive an AGHFP_SET_REMOTE_SPEAKER_VOLUME_CFM
	to indicate the success of otherwise of sending the speaker volume. */
void AghfpSetRemoteSpeakerVolume(AGHFP *aghfp, uint8 volume);


/*! @brief Respond to a response and hold read current status from the HF.

	@param aghfp A pointer to the profile instance.
	@param state 

	*/
void AghfpSendResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state);


/*! @brief Respond to a response and hold set command from the HF.

	@param aghfp A pointer to the profile instance.
	@param state 
	
	*/
void AghfpConfirmResponseHoldState(AGHFP *aghfp, aghfp_response_hold_state state);


/*! @brief

	@param aghfp A pointer to the profile instance.
	@param
	
	*/
void AghfpSendSubscriberNumber(AGHFP *aghfp, aghfp_subscriber_info *sub);


/*! @brief

	@param aghfp A pointer to the profile instance.
	@param
	
	*/
void AghfpSendSubscriberNumbersComplete(AGHFP *aghfp);


/*! @brief

	@param aghfp A pointer to the profile instance.
	@param
	
	*/
void AghfpSendCurrentCall(AGHFP *aghfp, aghfp_call_info *call);


/*! @brief

	@param aghfp A pointer to the profile instance.
	@param
	
	*/
void AghfpSendCurrentCallsComplete(AGHFP *aghfp);


/*! @brief

	@param aghfp A pointer to the profile instance.
	@param
	
	*/
void AghfpSendNetworkOperator(AGHFP *aghfp, uint8 mode, uint16 size_operator, uint8 *operator);


/*! @brief Allow the client to send an arbitrary string to the remote end.

	@param aghfp A pointer to the profile instance.
	@param size_data Length of the data pointed to by data.
    @param data The user specific data to send to the remote end.

    This function allows the client of the aghfp library to send user
    specific data to the remote end. It is the client's responsibility
    to ensure the data is formatted correctly. The aghfp library will
    just send out over the rfcomm stream any data received from the
    client as long as it has an SLC fully established.

    Returns a AGHFP_USER_DATA_CFM message to the client to indicate
    whether the data was sent successfully or not.
*/
void AghfpSendUserSpecificData(AGHFP *aghfp, uint16 size_data, uint8 *data);


/*! @brief Send \r\nERROR\r\n to the remote end.

    This function allows the client of the aghfp library to send an AT
    error response to the remote device. This command is valid as soon
    as the RFCOMM link is made - ie before a full SLC is established.

    Returns a AGHFP_SEND_ERROR_CFM message to the client to indicate
    whether the data was sent successfully or not.
*/
void AghfpSendError(AGHFP *aghfp);


/*! @brief Send \r\nOK\r\n to the remote end.

    This function allows the client of the aghfp library to send an AT
    OK response to the remote device. This command is valid as soon
    as the RFCOMM link is made - ie before a full SLC is established.

    Returns a AGHFP_SEND_OK_CFM message to the client to indicate
    whether the data was sent successfully or not.
*/
void AghfpSendOk(AGHFP *aghfp);


/*! @brief Set the current network service availability.

	@param aghfp A pointer to the profile instance.
	@param service_state The current state of network availability.
*/
void AghfpSetServiceState(AGHFP *aghfp, bool service_state);


/*! @brief Sets caller id details for +CLIP notifications.

	@param aghfp A pointer to the profile instance.
	@param type_number This parameter specifies the format of the phone number provided and can be one of the following values:
					   128-143=Unknown format. 144-159=International number. 160-175=National number.
					   See the description of "+CLIP" in HFP 1.5 spec for more details.
	@param size_number The number of characters in the phone number. (Not including any null terminator)
	@param number A uint8 array containing one digit of the phone number
	              per array element. The digits are in ASCII format and
	              may include special characters such as '+'. The number
	              should *NOT* be NULL terminated.
	@param size_string The number of characters in the string. (Not including any null terminator). Should be zero if string is NULL.
	@param string A friendly name to go with the phone number. Maybe NULL.
*/
void AghfpSetCallerIdDetails(AGHFP *aghfp, uint8 type_number, uint16 size_number, const uint8 *number, uint16 size_string, const uint8 *string);


/*! @brief Configure the time interval between repeating RING indications.

	@param aghfp A pointer to the profile instance.
	@param interval The time interval in milliseconds. Setting 0 disables ring indications. The default is 5000ms.
*/
void AghfpSetRingRepeatInterval(AGHFP *aghfp, uint16 interval);


/*! @brief Initiate/transfer a call to a HS/HF device using an existing audio connection.

	@param aghfp A pointer to the profile instance.
	@param call_type Whether call is incoming, outgoing or being transfered to HS/HF.
	@param in_band Indicates whether in band ringing is required for an incoming call.
*/
void AghfpCallCreate(AGHFP *aghfp, aghfp_call_type call_type, bool in_band);


/*! @brief Initiate/transfer a call to a HS/HF device and setup an audio connection.

	@param aghfp A pointer to the profile instance.
	@param call_type Whether call is incoming, outgoing or being transfered to HS/HF.
	@param in_band Indicates whether in band ringing is required.
	@param packet_type Packet type(s) for synchronous connection.
	@param audio_params Optional parameters for synchronous connection.
*/
void AghfpCallCreateAudio(AGHFP *aghfp, aghfp_call_type call_type, bool in_band, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);


/*! @brief Enables AG to answer a call.

	@param aghfp A pointer to the profile instance.
*/
void AghfpCallAnswer(AGHFP *aghfp);


/*! @brief Informs Call Manager that the remote party has answered the outgoing call.

	@param aghfp A pointer to the profile instance.
*/
void AghfpCallRemoteAnswered(AGHFP *aghfp);


/*! @brief Rejects a call being setup (incoming/outgoing) or terminates an existing call.

	@param aghfp A pointer to the profile instance.
	@param keep_sink Indicates whether any existing audio channel used for the call should be kept open or closed.
*/
void AghfpCallTerminate(AGHFP *aghfp, bool keep_sink);

/*! @brief Rejects a held incoming call.

	@param aghfp A pointer to the profile instance.
*/
void AghfpHeldCallTerminate(AGHFP *aghfp);

/*! @brief Accepts a held incoming call.

	@param aghfp A pointer to the profile instance.
*/
void AghfpHeldCallAccept(AGHFP *aghfp);

/*! @brief Sets the codec to negotiate and the negotiation type to use.

	@param aghfp A pointer to the profile instance.
	@param codec The codec to negotiate.
*/
void AghfpSetCodecType(AGHFP *aghfp, aghfp_wbs_codec codec);

/*! @brief Sets the codec negotiation type to use.

	@param aghfp A pointer to the profile instance.
	@param negotiation_type The negotiation type to use for the negotiation.
*/
void AghfpSetNegotiationType(AGHFP *aghfp, aghfp_codec_negotiation_type negotiation_type);

/*! @brief Dynamically enables WBS codec negotiation. This function can only has an affect
	if the AGHFP's state is 'aghfp_ready', i.e. no SLC connection.

	@param aghfp A pointer to the profile instance.
*/
void AghfpEnableWbsCodecNegotiation(AGHFP *aghfp);

/*! @brief Dynamically disables WBS codec negotiation. This function can only has an affect
	if the AGHFP's state is 'aghfp_ready', i.e. no SLC connection.

@param aghfp A pointer to the profile instance.
*/
void AghfpDisableWbsCodecNegotiation(AGHFP *aghfp);

/*! @brief Start WBS negotiation if appropriate.

@param aghfp A pointer to the profile instance.
*/
void AghfpStartWbsCodecNegotiation(AGHFP *aghfp);

/*! @brief Lets the AGHFP know that an app level codec negotiation is no longer pending.

@param aghfp A pointer to the profile instance.
*/
void AghfpClearAppCodecNegotiationPending(AGHFP * aghfp);

/*! @brief Kicks off audio after an app level codec negotiation.

@param aghfp A pointer to the profile instance.
*/
void AghfpStartAudioAfterAppCodecNegotiation(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params);

/*! @brief Gets the WBS codecs supported by the connected HF.
*/
uint8 AghfpGetHfWbsCodecsSupported(AGHFP * aghfp);

/*! @brief Indicates whether a Wbs has been negotiated.
*/
bool AghfpCodecHasBeenNegotiated(AGHFP * aghfp, uint8 *wbs_codec);

/*! @brief Returns the codecs supported by this instantiation of the AGHFP.
	Return value is only valid AFTER a call to AghfpInit().
*/
uint16 AghfpSupportedWbsCodecs(AGHFP * aghfp);

/*! @brief Request AudioParams once the codec negotiation setup is successful.

    @param aghfp A pointer to the profile instance.
    @param packet_type Bit mask containing all the packet types that is negotiated and filled back(See the sync_pkt_type enum in the Connection Library).
    @param audio_params Configuration parameters that is negotiated and is filled back
    
    Return value is only TRUE if the audio params are returned else FALSE.

 */
bool AghfpGetNegotiatedAudioParams(AGHFP *aghfp, sync_pkt_type *packet_type, aghfp_audio_params *audio_params);

/*! @brief   The link whether SC or non-SC is decided as a result of link encrypted after pairing.
                  This function will set the link mode accordingly in AGHFP library.

	@param aghfp A pointer to the profile instance.
	@param secure_link Whether the link is secure or not.
*/
void AghfpLinkSetLinkMode(AGHFP *aghfp, bool secure_link);


/*! @brief Returns the Qualcomm Codec Extensions Codec Mode IDs the AG is 
           configured to support in bitfield format. Only valid AFTER a call to
           AghfpInit().

    If the AG is not configured for QCE mode, the #CODEC_MODE_ID_UNSUPPORTED
    value is returned.

    @param aghfp A pointer to the profile instance.

    @Returns uint16 bitfield mask of all AG supported Codec Mode IDs (see 
    codec_mode_bit_type in library.h).
*/
uint16 AghfpGetSupportedQceCodecModeIds(AGHFP *aghfp);


/*! @brief Set the Qualcomm Codec Extensions Codec Mode IDs the AG is 
           configured to support in bitfield fromat. Only valid AFTER a call to
           AghfpInit().

    NOTE: 
    - The AG selects the first Codec that is supported in both HF and AG
      lowest to highest bit, according the to the Codec Mode IDs bitmask.
    - Only valid Codec Mode ID bits will be set (see codec_mode_bit_type in 
      library.h).

    @param aghfp A pointer to the profile instance.
    @param ag_codecs_bitmask Bitmask of all required Codec Mode ID bits.

    @Returns TRUE if successfully set, FALSE in all other cases (such as, if
    the AG already has an SLC with an HF or the bit mask is invalid).
*/
bool AghfpSetSupportedQceCodecModeIds(AGHFP *aghfp, uint16 ag_codecs_bitmask);


/*! @brief Gets the Qualcom Codec Extensions Codec Mode IDs the HF indicates
           it supports, after QCE supported feature exchange when SLC is
           established.

    If the AG is not configured for QCE mode or the SLC is not connected, the
    #CODEC_MODE_ID_UNSUPPORTED value is returned.

    @param aghfp A pointer to the profile instance.

    @Returns uint16 bitfield mask of all AG supported Codec Mode IDs (see 
    codec_mode_bit_type in library.h).
*/
uint16 AghfpGetSupportedQceCodecModeIdsInHf(AGHFP *aghfp);


/*! @brief Returns the selected Codec Mode Id, after Codec Selection during 
           Audio connection when in QCE mode. 
    If 
    - QCE mode is not enabled (no QCE Codec Mode IDs set for the AG).
    - There is no SCO/eSCO Audio connection.
    - Codec could not be selected e.g. not supported on both AG and HF.
    Returns the mode_id as #CODEC_MODE_ID_UNSUPPORTED.

    NOTE: The SCO/eSCO Audio parameters configured can be discovered using the 
    AghfpGetNegotiatedAudioParams() function.

    @param aghfp A pointer to the profile instance.
    @Param mode_id A pointer to return the Codec Mode ID value in, if
    this is successful. 

    @Return TRUE if successful, otherwise FALSE.

*/
bool AghfpGetQceSelectedCodecModeId(AGHFP *aghfp, uint16 *mode_id);

#endif /* AGHFP_H_ */
