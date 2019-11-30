/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp.h

DESCRIPTION
    Header file for the HFP profile library.
*/

/*!
\file
\defgroup hfp hfp
\ingroup vm_libs

\brief  Header file for the Hands Free Profile library.

\section hfp_intro INTRODUCTION
        This file documents the Hands Free Profile library API.
*/

/** @{ */

#ifndef HFP_H_
#define HFP_H_

#include <library.h>
#include <bdaddr_.h>
#include <connection.h>
#include <message.h>

/*!  Hfp Supported Features Flag Defines

    These flags can be or'd together and used as the supported_features field
    of an hfp_init_params structure.
*/

/*!
    @brief Setting this flag indicates that this device implements noise
    reduction / echo cancellation.
*/
#define HFP_NREC_FUNCTION           (1)
/*!
    @brief Setting this flag indicates that this device supports three way call
    control.
*/
#define HFP_THREE_WAY_CALLING       (1<<1)
/*!
    @brief Setting this flag indicates that this device can display calling
    line identification information.
*/
#define HFP_CLI_PRESENTATION        (1<<2)
/*!
    @brief Setting this flag indicates that this device can enable voice
    recognition on the AG.
*/
#define HFP_VOICE_RECOGNITION       (1<<3)
/*!
    @brief Setting this flag indicates that the AG can control this device's
    local volume.
*/
#define HFP_REMOTE_VOL_CONTROL        (1<<4)
/*!
    @brief Setting this flag indicates that this device can request a list of
    current calls from the AG and also receive call status indicators from the AG.
*/
#define HFP_ENHANCED_CALL_STATUS    (1<<5)
/*!
    @brief Setting this flag indicates that this device can use the extended
    three-way calling features of the AG.
*/
#define HFP_ENHANCED_CALL_CONTROL   (1<<6)
/*!
    @brief Setting this flag indicates that this device supports codec
    negotiation.
*/
#define HFP_CODEC_NEGOTIATION       (1<<7)
/*!
   @brief Setting this flag indicates that this device supports HF indicators.
*/
#define HFP_HF_INDICATORS           (1<<8)
/*!
    @brief Setting this flag indicates that this device supports eSCO S4
    (and T2) Settings. If WBS is disabled this flag is also set if S4 is supported.
*/
#define HFP_ESCO_S4_SUPPORTED       (1<<9)


/*!
    @brief Bitmask of all features supported by the HFP library
*/
#define HFP_SUPPORTED_FEATURES (HFP_NREC_FUNCTION | HFP_THREE_WAY_CALLING | \
                                HFP_CLI_PRESENTATION | HFP_VOICE_RECOGNITION | \
                                HFP_REMOTE_VOL_CONTROL | HFP_ENHANCED_CALL_STATUS | \
                                HFP_CODEC_NEGOTIATION |HFP_HF_INDICATORS | HFP_ESCO_S4_SUPPORTED)

/*! The maximum length of arrays in messages to the application */
#define HFP_MAX_ARRAY_LEN          (32)

/*!
    @brief The supported profiles.
*/
typedef enum
{
    /*! The supported profile has not been set for this profile instance. */
    hfp_no_profile              = 0,
    /*! As defined in part K6 of the Bluetooth specification.*/
    hfp_headset_profile         = 1<<0,
    /*! As defined in version 1.5 of the handsfree profile specification.*/
    hfp_handsfree_profile       = 1<<1,
    /*! As defined in version 1.7 of the handsfree profile specification, S4 mandatory indicating HFP 1.7*/
    hfp_handsfree_107_profile = 1<<2,

    FORCE_ENUM_TO_MIN_16BIT(hfp)
} hfp_profile;

/** Bitmasks for all HSP/HFP **/
#define hfp_headset_all           (hfp_headset_profile)
#define hfp_handsfree_all         (hfp_handsfree_profile | hfp_handsfree_107_profile)
#define hfp_handsfree_and_headset (hfp_headset_all | hfp_handsfree_all)

/*!
    @brief WB-Speech codec bit masks.
*/
typedef enum
{
    /*! No Codec. */
    hfp_wbs_codec_mask_none         = 0x00,
    /*! CVSD Codec. CVSD support is mandatory. */
    hfp_wbs_codec_mask_cvsd         = 0x01,
    /*! SBC Codec. SBC support is mandatory. */
    hfp_wbs_codec_mask_msbc         = 0x02,

    FORCE_ENUM_TO_MIN_16BIT(hfp_wbs_codec_mask)
} hfp_wbs_codec_mask;

/*!
    @brief HF Indicators bit masks.
*/
typedef enum
{
    /*! No HF indicator. */
    hfp_indicator_mask_none           = 0x00,
    /*! Enhanced safety indicator. */
    hfp_enhanced_safety_mask          = 1 << 0,
    /*! Battery level indicator. */
    hfp_battery_level_mask            = 1 << 1,

    FORCE_ENUM_TO_MIN_16BIT(hfp_hf_indicators_mask)
} hfp_hf_indicators_mask;

/*!
    @brief Possible values indicators can take for optional_indicators
*/
typedef enum
{
    hfp_indicator_off    = 0,
    hfp_indicator_on     = 1,
    hfp_indicator_ignore = 2,

    FORCE_ENUM_TO_MIN_16BIT(hfp_indicator_status)
} hfp_indicator_status;

/*!
    @brief Possible values indicators can take for hf_indicators
*/
typedef enum
{
    hfp_hf_indicator_off    = 0,
    hfp_hf_indicator_on     = 1
} hfp_hf_indicator_state;


/*! @brief  Assigned numbers for HF indicators as per
  https://www.bluetooth.org/en-us/specification/assigned-numbers/hands-free-profile
*/
typedef enum
{
    hf_indicators_invalid  = 0x00,
    hf_enhanced_safety     = 0x01,
    hf_battery_level       = 0x02
} hfp_indicators_assigned_id;


/*!
    @brief Possible cvsd audio settings allowed with secure connection in addition to mandatory safe settings 'S1'
*/
typedef enum
{
    hfp_audio_setting_none,
    hfp_audio_setting_s4,
    hfp_audio_setting_s3
} hfp_secure_audio_settings;

/*!
    @brief Structure containing separate values for each supported indicator
*/
typedef struct
{
    unsigned service:2;
    unsigned signal_strength:2;
    unsigned roaming_status:2;
    unsigned battery_charge:2;
    unsigned unused:8;
} hfp_optional_indicators;

/*!
    @brief Possible CSR2CSR codecs
*/
typedef enum
{
    /*! No Codec. */
    hfp_csr_codec_mask_none         = 0x00,
    /*! CVSD Codec. */
    hfp_csr_codec_mask_cvsd         = 0x01,
    /*! Auristream Codec - 2 bits per sample. */
    hfp_csr_codec_mask_adpcm_2bps   = 0x02,
    /*! Auristream Codec - 4 bits per sample. */
    hfp_csr_codec_mask_adpcm_4bps   = 0x04,

    FORCE_ENUM_TO_MIN_16BIT(hfp_csr_codec_mask)
} hfp_csr_codec_mask;

/*!
    @brief Possible CSR2CSR bandwidths (to use with above codecs)
*/
typedef enum
{
    /*! No reported bandwidths supported*/
    hfp_csr_bandwidth_none  = 0x00,
    /*! 8kHz. */
    hfp_csr_bandwidth_8kHz  = 0x01,
    /*! 16kHz. */
    hfp_csr_bandwidth_16kHz = 0x02,

    FORCE_ENUM_TO_MIN_16BIT(hfp_csr_bandwidth_mask)
} hfp_csr_bandwidth_mask;

/*!
    @brief CSR features 
*/
typedef struct
{
    unsigned                caller_name:1;
    unsigned                raw_text:1;
    unsigned                sms:1;
    unsigned                batt_level:1;
    unsigned                pwr_source:1;
    unsigned                codecs:3;
    unsigned                codec_bandwidths:2;
    unsigned                unused:6;
} hfp_csr_features;

/*!
    @brief Configuration parameters passed into the hfp profile library in
    order for an Hfp profile instance to be created and initialised.

    The library client (usually the application) must specify the profile to be
    supported. In the case of hands free it must also specify the set of
    supported features for that profile instance. Optionally, the client may
    also supply a service record. If the service_record pointer is set to null
    the default service record provided by the hfp library is used. If however
    the pointer is non null, this client supplied service record is used. The
    hfp library will still insert the correct rfcomm channel number into the
    service record but it will perform no other error checking on the record
    provided.  The client provided service record may be fairly large so the
    hfp library will not attempt to take a local copy of the data. For the
    moment the hfp library assumes that the service record pointer is in const
    or that the client will not free the pointer for the lifetime of the
    application.
    
    @param supported_profile This indicates which profiles the library should
    support. This can be headset profile, one of the hansfree profiles, or
    both headset profile and one of the handsfree profiles. 
*/
typedef struct
{
    /*! The profiles supported.                                         */
    hfp_profile               supported_profile;
    /*! See supported features flags.                                   */
    uint16                    supported_features;
    /*! User configurable information regarding the HF's codecs.        */
    hfp_wbs_codec_mask        supported_wbs_codecs;
    /*! Optional indicators the HF may not wish to receive              */
    hfp_optional_indicators   optional_indicators;
    /* pack bool parameters all into one word                           */
    unsigned                  unused:13;
    /*! Disable Noise Reduction/Echo Cancellation on connection         */
    unsigned                  disable_nrec:1;
    /*! Enable sending of extended error codes by AG's supporting them  */
    unsigned                  extended_errors:1;
    /*! Whether multiple connections to one profile are supported       */
    unsigned                  multipoint:1;
    /*! The time in minutes to attempt to reconnect for on link loss    */
    unsigned                  link_loss_time:8;
    /*! The time in seconds between link loss reconnect attempts        */
    unsigned                  link_loss_interval:8;
    /*! The CSR2CSR features to enable if supported by the remote AG    */
    hfp_csr_features          csr_features;
    /*! HF indicators feature HF supports */
    hfp_hf_indicators_mask  hf_indicators;
    /*! Qualcomm Codec Extension Mode IDs supported by HF */
    codec_mode_bit_type       hf_codec_modes;
} hfp_init_params;

/*!
    @brief Connection parameters for setting up an eSCO/SCO connection
*/
typedef struct
{
   uint32            bandwidth;
   uint16            max_latency;
   sync_air_coding   voice_settings;
   sync_retx_effort  retx_effort;
} hfp_audio_params;

/*!
     @brief Generic hfp status.
*/
typedef enum
{
    hfp_success,                        /*!< Success.*/
    hfp_fail,                           /*!< Failure.*/
    hfp_ag_failure,                     /*!< Failure - AG failure.*/
    hfp_no_connection_to_phone,         /*!< Failure - No connection to phone.*/
    hfp_operation_not_allowed,          /*!< Failure - Operation not allowed.*/
    hfp_operation_not_supported,        /*!< Failure - Operation not supported.*/
    hfp_ph_sim_pin_required,            /*!< Failure - PH-SIM PIN required.*/
    hfp_sim_not_inserted,               /*!< Failure - SIM not inserted.*/
    hfp_sim_pin_required,               /*!< Failure - SIM PIN required.*/
    hfp_sim_puk_required,               /*!< Failure - SIM PUK required.*/
    hfp_sim_failure,                    /*!< Failure - SIM failure.*/
    hfp_sim_busy,                       /*!< Failure - SIM busy.*/
    hfp_incorrect_password,             /*!< Failure - Incorrect password.*/
    hfp_sim_pin2_required,              /*!< Failure - SIM PIN2 required.*/
    hfp_sim_puk2_required,              /*!< Failure - SIM PUK2 required.*/
    hfp_memory_full,                    /*!< Failure - Memory full.*/
    hfp_invalid_index,                  /*!< Failure - Invalid index.*/
    hfp_memory_failure,                 /*!< Failure - Memory failure.*/
    hfp_text_string_too_long,           /*!< Failure - Text string too long.*/
    hfp_invalid_chars_in_text_string,   /*!< Failure - Invalid characters in text string.*/
    hfp_dial_string_too_long,           /*!< Failure - Dial string too long.*/
    hfp_invalid_chars_in_dial_string,   /*!< Failure - Invalid characters in dial string.*/
    hfp_no_network_service,             /*!< Failure - No network service.*/
    hfp_network_timeout,                /*!< Failure - Network timeout.*/
    hfp_network_not_allowed,            /*!< Failure - Network not allowed, emergency calls only.*/

    hfp_csr_not_inited,                 /*!< Failure - CSR extensions have not been initialised. */
    hfp_csr_no_slc,                     /*!< Failure - CSR extension request failed due to no SLC being present. */
    hfp_csr_invalid_params,             /*!< Failure - CSR extention request failed due to being supplied with invalid parameters from the application. */
    hfp_csr_invalid_ag_params,          /*!< Failure - CSR extention request failed due to being supplied with invalid parameters from the AG. */
    hfp_csr_mod_ind_no_mem,             /*!< Failure - CSR extension modify indicators failed due to not being able to allocate
                                             memory for AT Command generation.  A request with fewer indictors may succeed */
    hfp_timeout                         /*!< Failure - Timed out waiting for AG response */
} hfp_lib_status;


/*!
    @brief Return status for the HFP_INIT_CFM message
*/
typedef enum
{
    /*! Successful initialisation.*/
    hfp_init_success,
    /*! Unsuccessful as HFP is already initialised. HFP may still be used. */
    hfp_init_reinit_fail,    /* Errors after this are fatal, and hfp is de-initialised */

    /*! Unsuccessful due to RFCOMM channel registration failure. HFP Init failed.*/
    hfp_init_rfc_chan_fail,
    /*! Unsuccessful due to a service record registration failure. HFP Init failed.*/
    hfp_init_sdp_reg_fail
} hfp_init_status;


/*!
    @brief Return status for the HFP_SLC_CONNECT_CFM message
*/
typedef enum
{
    /*! Successful connection.*/
    hfp_connect_success,
    /*! Unsuccessful due to a service search failure.*/
    hfp_connect_sdp_fail,
    /*! Unsuccessful due to a service level connection failure.*/
    hfp_connect_slc_failed,
    /*! Unsuccessful as max supported connections have been reached.*/
    hfp_connect_failed_busy,
    /*! Unsuccessful due to RFCOMM connection failing to be established.*/
    hfp_connect_failed,
    /*! Unsuccessful due to attempt to connect to unallocated server channel.*/
    hfp_connect_server_channel_not_registered,
    /*! Unsuccessful due to connection attempt timing out.*/
    hfp_connect_timeout,
    /*! Unsuccessful due to remote device rejecting connection.*/
    hfp_connect_rejected,
    /*! Unsuccessful due to security failure*/
    hfp_connect_rejected_security,
    /*! Connect attempt aborted in favour of incoming connection from same device */
    hfp_connect_failed_crossover
} hfp_connect_status;


/*!
    @brief Return status for the HFP_SLC_LINK_LOSS_IND message
*/
typedef enum
{
    hfp_link_loss_none,
    hfp_link_loss_recovery,
    hfp_link_loss_timeout,
    hfp_link_loss_abort
} hfp_link_loss_status;


/*!
    @brief Return status for the HFP_SLC_DISCONNECT_IND message
*/
typedef enum
{
    /*! Successful disconnection.*/
    hfp_disconnect_success,
    /*! Unsuccessful due to abnormal link loss.*/
    hfp_disconnect_link_loss,
    /*! Unsuccessful due to no current connection.*/
    hfp_disconnect_no_slc,
    /*! Disconnection has occurred but in an abnormal way */
    hfp_disconnect_abnormally,
    /*! Unsuccessful due to RFCOMM connection attempt error.*/
    hfp_disconnect_error
} hfp_disconnect_status;


/*!
    @brief Link priority is used to identify different links to
    AG devices using the order in which the devices were connected.
*/
typedef enum
{
    /*! Invalid Link. */
    hfp_invalid_link,
    /*! The link that was connected first. */
    hfp_primary_link,
    /*! The link that was connected second. */
    hfp_secondary_link
} hfp_link_priority;


/*!
    @brief Return status for the HFP_AUDIO_CONNECT_CFM message.
*/
typedef enum
{
    /*! Successful audio connection.*/
    hfp_audio_connect_success,
    /*! Unsuccessful due to negotiation failure.*/
    hfp_audio_connect_failure,
    /*! Unsuccessful due to audio already being with device.*/
    hfp_audio_connect_have_audio,
    /*! Unsuccessful due to an audio connect already being attempted.*/
    hfp_audio_connect_in_progress,
    /*! Unsuccessful due to one or more parameters specified being invalid.*/
    hfp_audio_connect_invalid_params,
    /*! Unsuccessful due to library being in incorrect state.*/
    hfp_audio_connect_error,
    /*! Unsuccessful due to AG being unable to begin codec negotiation.*/
    hfp_audio_connect_codec_neg_fail,
    /*! Unsuccessful due to unexpected error during codec negotiation.*/
    hfp_audio_connect_codec_neg_error,
    /*! Successful audio connection but no HFP link is present*/
    hfp_audio_connect_no_hfp_link
} hfp_audio_connect_status;


/*!
    @brief Return status for the HFP_AUDIO_DISCONNECT_IND message.
*/
typedef enum
{
    /*! Successful audio disconnection.*/
    hfp_audio_disconnect_success,
    /*! Unsuccessful due to failure indication from firmware.*/
    hfp_audio_disconnect_failure,
    /*! Unsuccessful due to audio being with AG.*/
    hfp_audio_disconnect_no_audio,
    /*! Unsuccessful due to an audio disconnect already being attempted.*/
    hfp_audio_disconnect_in_progress,
    /*! Unsuccessful due to library being in incorrect state.*/
    hfp_audio_disconnect_error
} hfp_audio_disconnect_status;


/*!
    @brief Transfer direction for audio connection.
*/
typedef enum
{
    /*! Transfer the audio to the HFP device.*/
    hfp_audio_to_hfp,
    /*! Transfer the audio to the audio gateway.*/
    hfp_audio_to_ag,
    /*! Toggle direction of current audio.*/
    hfp_audio_transfer
} hfp_audio_transfer_direction;


/*!
    @brief Actions that can be carried out using HfpCallHoldActionRequest.
*/
typedef enum
{
    /*! CHLD=0 */
    hfp_chld_release_held_reject_waiting = 0,
    /*! CHLD=1 */
    hfp_chld_release_active_accept_other = 1,
    /*! CHLD=2 */
    hfp_chld_hold_active_accept_other    = 2,
    /*! CHLD=3 */
    hfp_chld_add_held_to_multiparty      = 3,
    /*! CHLD=4 */
    hfp_chld_join_calls_and_hang_up      = 4
} hfp_chld_action;


/*!
    @brief The call state of a connection. Maps directly onto the call SM.
*/
typedef enum
{
    hfp_call_state_idle,
    hfp_call_state_incoming,
    hfp_call_state_incoming_held,
    hfp_call_state_outgoing,
    hfp_call_state_active,
    hfp_call_state_twc_incoming,    /* TWC: Three way call */
    hfp_call_state_twc_outgoing,
    hfp_call_state_held_active,
    hfp_call_state_held_remaining,
    hfp_call_state_multiparty
} hfp_call_state;


/*!
    @brief Values of the subscriber number service.
*/
typedef enum
{
    /*! Asynchronous modem.*/
    hfp_service_async_modem,
    /*! Synchronous modem.*/
    hfp_service_sync_modem,
    /*! PAD access (synchronous).*/
    hfp_service_pad_access,
    /*! Packet access (asynchronous).*/
    hfp_service_packet_access,
    /*! Voice.*/
    hfp_service_voice,
    /*! Fax.*/
    hfp_service_fax
} hfp_subscriber_service;


/*!
    @brief Call direction used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Call from AG to network.*/
    hfp_call_mobile_originated,
    /*! Call from network to AG.*/
    hfp_call_mobile_terminated
} hfp_call_direction;


/*!
    @brief Call status used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Call is currently active.*/
    hfp_call_active,
    /*! Call is currently held.*/
    hfp_call_held,
    /*! Call is being dialled - mobile originated only.*/
    hfp_call_dialling,
    /*! Call is alerting - mobile originated only.*/
    hfp_call_alerting,
    /*! Call is incoming - mobile terminated only.*/
    hfp_call_incoming,
    /*! Call is waiting - mobile terminated only.*/
    hfp_call_waiting
} hfp_call_status;


/*!
    @brief Call mode used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Voice call.*/
    hfp_call_voice,
    /*! Data call.*/
    hfp_call_data,
    /*! FAX call.*/
    hfp_call_fax
} hfp_call_mode;


/*!
    @brief Call multiparty status used in HFP_CURRENT_CALLS_IND message.
*/
typedef enum
{
    /*! Call is not multiparty.*/
    hfp_not_multiparty_call,
    /*! Call is multiparty.*/
    hfp_multiparty_call
} hfp_call_multiparty;


/*!
    @brief Response and hold actions that can be used in HfpResponseHoldActionRequest.
*/
typedef enum
{
    /*! Place an incoming call on hold.*/
    hfp_hold_incoming_call,
    /*! Accept a previously held incoming call.*/
    hfp_accept_held_incoming_call,
    /*! Reject a previously held incoming call.*/
    hfp_reject_held_incoming_call
} hfp_btrh_action;


/*!
    @brief Used to identify type of number specified in HFP_SUBSCRIBER_NUMBER_IND,
           HFP_CALLER_ID_IND, HFP_CALL_WAITING_IND and HFP_CURRENT_CALLS_IND.
*/
typedef enum
{
    /*! Type of number is unknown.*/
    hfp_number_unknown,
    /*! Number is an international number.*/
    hfp_number_international,
    /*! Number is a national number.*/
    hfp_number_national,
    /*! Number is a network specific number.*/
    hfp_number_network,
    /*! Number is a dedicated access, short code.*/
    hfp_number_dedicated
} hfp_number_type;

/*!
    @brief Power source status.
*/
typedef enum
{
    /*! Device in using battery power. */
    hfp_csr_pwr_rep_battery = 1,
    /*! Device is using an external power source. */
    hfp_csr_pwr_rep_external = 2
} hfp_csr_power_status_report;

/*!
    @brief Define the types for the upstream messages sent from the HFP profile
    library to the application.
*/

/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
typedef enum
{
/* 0x5A00 */    HFP_INIT_CFM = HFP_MESSAGE_BASE,
/* Service Level Connection messages */
/* 0x5A01 */    HFP_SLC_CONNECT_CFM,
/* 0x5A02 */    HFP_SLC_CONNECT_IND,
/* 0x5A03 */    HFP_SLC_LINK_LOSS_IND,
/* 0x5A04 */    HFP_SLC_DISCONNECT_IND,
/* Audio messages */
/* 0x5A05 */    HFP_AUDIO_CONNECT_CFM,
/* 0x5A06 */    HFP_AUDIO_CONNECT_IND,
/* 0x5A07 */    HFP_AUDIO_DISCONNECT_IND,
/* Indicators */
/* 0x5A08 */    HFP_SERVICE_IND,
/* 0x5A09 */    HFP_SIGNAL_IND,
/* 0x5A0A */    HFP_ROAM_IND,
/* 0x5A0B */    HFP_BATTCHG_IND,
/* 0x5A0C */    HFP_RING_IND,
/* 0x5A0D */    HFP_CALLER_ID_IND,
/* Call control */
/* 0x5A0E */    HFP_CALL_STATE_IND,
/* 0x5A0F */    HFP_CALL_WAITING_IND,
/* Dialling */
/* 0x5A10 */    HFP_DIAL_LAST_NUMBER_CFM,
/* 0x5A11 */    HFP_DIAL_NUMBER_CFM,
/* 0x5A12 */    HFP_DIAL_MEMORY_CFM,
/* 0x5A13 */    HFP_VOICE_RECOGNITION_ENABLE_CFM,
/* 0x5A14 */    HFP_VOICE_RECOGNITION_IND,
/* Volume controls */
/* 0x5A15 */    HFP_VOLUME_SYNC_SPEAKER_GAIN_IND,
/* 0x5A16 */    HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND,
/* Other */
/* 0x5A17 */    HFP_VOICE_TAG_NUMBER_IND,
/* 0x5A18 */    HFP_UNRECOGNISED_AT_CMD_IND,
/* 0x5A19 */    HFP_EXTRA_INDICATOR_INDEX_IND,
/* 0x5A1A */    HFP_EXTRA_INDICATOR_UPDATE_IND,
/* 0x5A1B */    HFP_SUBSCRIBER_NUMBER_IND,
/* 0x5A1C */    HFP_CURRENT_CALLS_IND,
/* 0x5A1D */    HFP_NETWORK_OPERATOR_IND,
/* CSR Extended Features */
/* 0x5A1E */    HFP_CSR_FEATURES_TEXT_IND,
/* 0x5A1F */    HFP_CSR_FEATURES_NEW_SMS_IND,
/* 0x5A20 */    HFP_CSR_FEATURES_GET_SMS_CFM,
/* 0x5A21 */    HFP_CSR_FEATURES_BATTERY_LEVEL_REQUEST_IND,
/* Optional confirmation messages */
/* 0x5A22 */    HFP_CALLER_ID_ENABLE_CFM,
/* 0x5A23 */    HFP_CALL_ANSWER_CFM,
/* 0x5A24 */    HFP_CALL_TERMINATE_CFM,
/* 0x5A25 */    HFP_CALL_WAITING_ENABLE_CFM,
/* 0x5A26 */    HFP_CALL_HOLD_ACTION_CFM,
/* 0x5A27 */    HFP_RESPONSE_HOLD_ACTION_CFM,
/* 0x5A28 */    HFP_HS_BUTTON_PRESS_CFM,
/* 0x5A29 */    HFP_DTMF_CFM,
/* 0x5A2A */    HFP_SUBSCRIBER_NUMBERS_CFM,
/* 0x5A2B */    HFP_CURRENT_CALLS_CFM,
/* 0x5A2C */    HFP_NETWORK_OPERATOR_CFM,
/* 0x5A2D */    HFP_VOICE_TAG_NUMBER_CFM,
/* 0x5A2E */    HFP_AT_CMD_CFM,

/* Library message limit */
/* HF Indicators Feature */
/* 0x5A2F */    HFP_HF_INDICATORS_REPORT_IND,
/* 0x5A30 */    HFP_HF_INDICATORS_IND,

/* Library message limit */
/* 0x5A31 */    HFP_MESSAGE_TOP
}HfpMessageId ;
#endif /*end of DO_NOT_DOCUMENT*/


/*
    The upstream messages from the HFP profile library to the application are
    defined below. 
*/


/*!
    @brief This message is generated as a result of a call to HfpInit.

    The application creates a profile instance by calling HfpInit. This
    function is responsible for creating a task for the profile instance,
    initialising that task and its associated state and performing all other
    initialisation related tasks, e.g. registering a service record.  Once the
    initialisation is completed this message will be returned to the
    application.
*/
typedef struct
{
    /*! If the task was created and initialised successfully then this will be
      set to hfp_init_success otherwise it will be set to indicate why the
      initialisation failed. */
    hfp_init_status    status;
} HFP_INIT_CFM_T;


/*!
    @brief This message returns the result of an attempt to establish a Service
    Level Connection.

    This message is sent to the application once a Service Level Connection has
    been established or the connect attempt has failed. It is sent to inform
    the application that the Service Level Connection establishment phase has
    completed. The application will receive this message regardless of whether
    it initiated the Service Level Connection establishment or whether this was
    done by the AG.
*/
typedef struct
{
    /*! The priority of the connection. */
    hfp_link_priority   priority;
    /*! The bluetooth address for the AG */
    bdaddr              bd_addr;
    /*! The sink associated with this HFP Service Level Connection. */
    Sink                sink;
    /*! The profile connected. */
    hfp_profile         profile;
    /*! Indicates whether or not the connection was successfully established. */
    hfp_connect_status  status;
} HFP_SLC_CONNECT_CFM_T;


/*!
    @brief This message is sent when a remote device wishes to establish a
    Service Level Connection.

    This message is sent to the application to indicate that a remote device
    (the AG) is requesting to establish a Service Level Connection to this
    device.  No response is required to this message. If the request has
    been rejected then no HFP_SLC_CONNECT_CFM will be sent to the application
    for this indication.
*/
typedef struct
{
    /*! The Bluetooth address of the remote device.*/
    bdaddr    addr;
    /*! Whether the connection request was accepted or rejected */
    bool      accepted;
} HFP_SLC_CONNECT_IND_T;


/*!
    @brief This message is sent when a Service Level Connection has been
    lost and the library is attempting to automatically reconnect. This
    can be configured in hfp_init_params. If all reconnection attempts 
    fail then HFP_SLC_DISCONNECT_IND will be sent with status 
    hfp_disconnect_link_loss.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority     priority;
    /*! The link loss status */
    hfp_link_loss_status  status;
} HFP_SLC_LINK_LOSS_IND_T;


/*!
    @brief This message is sent when a Service Level Connection has been
    disconnected.

    This message informs the application that the Service Level Connection has
    been disconnected.  This disconnect can be the result of a locally
    initiated action, a disconnect initiated from the AG or an abnormal
    disconnect e.g. link loss.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority     priority;
    /*! The bluetooth address for the AG */
    bdaddr                bd_addr;
    /*! Indicates the reason for the disconnect.*/
    hfp_disconnect_status status;
} HFP_SLC_DISCONNECT_IND_T;


/*!
    @brief This message informs the app that an audio (SCO/eSCO) connection is being requested by the AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! The WBS codec negotiated (CVSD if WBS not used) */
    hfp_wbs_codec_mask     codec;
} HFP_AUDIO_CONNECT_IND_T;


/*!
    @brief This message informs the app that the audio (SCO/eSCO) has been connected or that the
    attempt to open a SCO/eSCO connection has failed.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority           priority;
    /*! If the return status from the connection library is hci_success, then status will be 
        set to hfp_success. Otherwise it will be set to hfp_fail. */    
    hfp_audio_connect_status    status;
    /*! Indicates whether a SCO or eSCO link was created. */
    sync_link_type              link_type;
    /*! If a SCO/eSCO connection has been created the audio_sink will be the corresponding sink
        for the SCO/eSCO data, otherwise it will be set to zero. */
    Sink                        audio_sink;
    /*! Receive bandwith in bytes per second. */
    uint32                      rx_bandwidth;
    /*!< Transmit bandwith in bytes per second. */
    uint32                      tx_bandwidth;
    /*! The WBS codec negotiated (CVSD if WBS not used) */
    hfp_wbs_codec_mask          codec;
    /*! Wesco reported for the link. */
    uint8                       wesco;
    /*! Qualcomm Codec Mode ID Selected - If in Qualcomm Codec Extensions mode. */
    uint16                      qce_codec_mode_id;
} HFP_AUDIO_CONNECT_CFM_T;


/*!
    @brief This message informs the app that the audio (SCO/eSCO) has been disconnected.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority           priority;
    /*! If the return status from the connection library is hci_success then status
        is set to hfp_success. Otherwise the status is set to hfp_fail. */
    hfp_audio_disconnect_status status;
} HFP_AUDIO_DISCONNECT_IND_T;


/*!
    @brief This message informs the application of a change in the service
    indicator's status.

    This message is sent on Service Level Connection establishment and
    subsequently whenever a service indicator update is received.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! The new value of the service indicator.*/
    uint16    service;
} HFP_SERVICE_IND_T;


/*!
    @brief This message is sent to the application whenever the call state
    of an AG changes.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! The call state of the AG. */
    hfp_call_state call_state;
} HFP_CALL_STATE_IND_T;


/*!
    @brief This message is sent to the application whenever a signal indication
    is received from the AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*!< The new value of the signal indicator.*/
    uint16   signal;    
} HFP_SIGNAL_IND_T;


/*!
    @brief This message is sent to the application whenever a roam indication
    is received from the AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*!< The new value of the roam indicator.*/
    uint16   roam;    
} HFP_ROAM_IND_T;


/*!
    @brief This message is sent to the application whenever a battery indication
    is received from the AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*!< The new value of the battchg indicator.*/
    uint16   battchg;    
} HFP_BATTCHG_IND_T;

/*!
    @brief This message is sent to the application to report HF indicators supported
    by AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! Number of HF indicators supported by AG */
    uint16 num_hf_indicators;
    /*! Bit mask of HF indicators supported */
    uint16 hf_indicators_mask;
} HFP_HF_INDICATORS_REPORT_IND_T;

/*!
    @brief This message is sent to the application whenever a hf indicator state
     change message is received  from the AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    bool hf_indicator_status;
    uint16 hf_indicator_assigned_num;
} HFP_HF_INDICATORS_IND_T;


/*!
    @brief This message is sent to the application whenever a RING indication
    is received from the AG.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! Whether the AG supports in band ring, or in the case of HSP whether
        SCO is open on the AG sending the ring indication. */
    bool                in_band;
} HFP_RING_IND_T;


/*!
    @brief If the application has enabled caller id notifications at the AG,
    this message will be sent to the application whenever a caller id
    notification is received from the AG.
    Note: These messages can own a 2nd heap cell through the member:
    caller_name.  Thus they should never be delete using the
    MessageCancelXXX API's as this will lead to a memory leak.

*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! Type of number. */
    hfp_number_type     number_type;
    /*! The size of the caller name (in bytes).*/
    uint16              size_name;
    /*! A pointer to the callers name if there is one. If this is not NULL
      then the client is responsibile for freeing this buffer.*/
    uint8*              caller_name;
    /*! The size of the caller number (in bytes).*/
    uint16              size_number;
    /*! An array containing the callers number. The client
      should not attempt to free this pointer, the memory will be freed when
      the message is destroyed. If the client needs access to this data after
      the message has been destroyed it is the client's responsibility to copy
      it. */
    uint8               caller_number[1];
} HFP_CALLER_ID_IND_T;


/*!
    @brief This message is sent from the Hfp profile library to the application
    whenever a call waiting notification is received from the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! Type of number. */
    hfp_number_type     number_type;
    /*! The offset of the caller number from caller_info (in bytes). */
    uint16              offset_number;
    /*! The size of the caller number (in bytes).*/
    uint16              size_number;
    /*! The offset of the caller name from caller_info (in bytes). */
    uint16              offset_name;
    /*! The size of the caller name (in bytes).*/
    uint16              size_name;
    /*! Pointer to the phone number and name of the waiting call. The
      client should not attempt to free this pointer, the memory will be freed
      when the message is destroyed. If the client needs access to this data
      after the message has been destroyed it is the client's responsibility to
      copy it.*/
    uint8               caller_info[1];
} HFP_CALL_WAITING_IND_T;


/*!
    @brief This message informs the application of the outcome of this dial
    request.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_DIAL_LAST_NUMBER_CFM_T;


/*!
    @brief This message informs the application of the outcome of a request to
    dial a specific number.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_DIAL_NUMBER_CFM_T;


/*!
    @brief This message is sent in response to a request to the AG to dial a
    number from a particular memory location.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_DIAL_MEMORY_CFM_T;


/*!
    @brief This message is sent in response to the voice recognition enable/
    disable command being sent to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_VOICE_RECOGNITION_ENABLE_CFM_T;


/*!
    @brief This is an unsolicited message sent in response to a voice
    recognition indication received from the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! The current status of the AG's voice recognition function. Enabled
      (TRUE) or disabled (FALSE).*/
    uint16    enable;
} HFP_VOICE_RECOGNITION_IND_T;


/*!
    @brief This message is sent in response to a speaker gain indication being
    received from the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! The value from the AT indication sent by the AG.*/
    uint16    volume_gain;
} HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T;


/*!
    @brief This message is sent in response to a microphone gain indication
    being received from the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! The value from the AT indication sent by the AG.*/
    uint16    mic_gain;
} HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND_T;


/*!
    @brief This message is sent in response to the AT command requesting the AG
    supplies a number to attach to a voice tag.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! The number of bytes pointed to by the number pointer.*/
    uint16            size_phone_number;
    /*! Pointer to the phone number. The client should not attempt to free this
      pointer, the memory will be freed when the message is destroyed. If the
      client needs access to this data after the message has been destroyed it
      is the client's responsibility to copy it.*/
    uint8            phone_number[1];
} HFP_VOICE_TAG_NUMBER_IND_T;


/*!
    @brief This message contains the strings that the AT command parser cannot
    parse.

    The strings that the auto generated AT command parser supports are
    specified in hfp_parse.parse along with the handler function to be called
    once a command has been parsed. However, if the parser cannot parse a
    particular AT string it will pass it to the application for further
    processing.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! The number of bytes pointed to by data.*/
    uint16    size_data;
    /*! The data that could not be parsed. The client should not attempt to
      free this pointer, the memory will be freed when the message is
      destroyed. If the client needs access to this data after the message has
      been destroyed it is the client's responsibility to copy it. */
    uint8    data[1];
} HFP_UNRECOGNISED_AT_CMD_IND_T;


/*!
    @brief This message returns an indicator supported by the AG.

    The application may request to be notified if the AG supports a particular
    indicator when initiating an Service Level Connection or accepting an
    incoming connection request. It does this by passing a configuration string
    to the Hfp profile library. If the +CIND response received from the AG does
    contain one of the indicators in the configuration string this message will
    be sent to the application. One message will be sent for each indicator
    found.
*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! The index of the indicator in the configuration string the application
      passed to the Hfp library on a connect request response.*/
    uint16    indicator_register_index;
    /*! The index of the indicator in the CIND string returned by the AG.*/
    uint16    indicator_index;
    /*! Minimum value allowed for this indicitor.*/
    uint16    min_range;
    /*! Maximum value allowed for this indicitor.*/
    uint16    max_range;
} HFP_EXTRA_INDICATOR_INDEX_IND_T;


/*!
    @brief This message returns the value of an indicator the AG supports.

    If the application registers that it is interested in a particular
    indicator (other than the ones defined by the HFP specification) it will be
    sent this message whenever an indicator status update (+CIEV) is received
    from the AG. All indicator updates will be sent to the application even if
    they are not for the indicator the application initially registered an
    interest in. It is the application's responsibility to determine whather
    the indicator update is of interest.

*/
typedef struct
{
    /*! The priority of the AG. */
    hfp_link_priority   priority;
    /*! The index of the indicator as initially reported in the CIND string
      (i.e. indicator_index in the HFP_EXTRA_INDICATOR_INDEX_IND message).*/
    uint16    index;
    /*! The new value for this indicator. */
    uint16    value;
} HFP_EXTRA_INDICATOR_UPDATE_IND_T;


/*!
    @brief The application will receive this message for each subscriber number sent by the AG in response to a
    request command.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority       priority;
    /*! Service relating to the phone number.*/
    hfp_subscriber_service  service;
    /*! Type of number. */
    hfp_number_type         number_type;
    /*! The number of bytes in the array containing the subscriber number.*/
    uint16                  size_number;
    /*! Array containing the subscriber number.*/
    uint8                   number[1];
} HFP_SUBSCRIBER_NUMBER_IND_T;


/*!
    @brief The application will receive one of these messages for each active call on the AG in response
    to a request to obtain the list of current calls.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority       priority;
    /*! Index number of call e.g. for referencing with AT+CHLD commands.*/
    uint16                  call_idx;
    /*! Indicates if the call is AG originated or not.*/
    hfp_call_direction      direction;
    /*! State of the call.*/
    hfp_call_status         status;
    /*! Indicates the mode of the call - bearer/teleservice.*/
    hfp_call_mode           mode;
    /*! Indicates if the call is a multi-party call or not.*/
    hfp_call_multiparty     multiparty;
    /*! Type of number. */
    hfp_number_type         number_type;
    /*! Length of array, in bytes, containing the phone number.*/
    uint16                  size_number;
    /*! Array containing the phone number. */
    uint8                   number[1];
} HFP_CURRENT_CALLS_IND_T;


/*!
    @brief This message is sent to the application to inform it of the network
    operator sent by the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! Network operator selection mode.*/
    uint8   mode;
    /*! Length of operator_name, in bytes.*/
    uint16  size_operator_name;
    /*! Operator name string.*/
    uint8   operator_name[1];
} HFP_NETWORK_OPERATOR_IND_T;


/*!
    @brief Unsolicited Text 2 Speech text has been received.
    
    The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! The number of bytes in the text string. */
    uint16  size_text;
    /*! The text that needs to be presented to the user. The client
      should not attempt to free this pointer, the memory will be freed when
      the message is destroyed. If the client needs access to this data after
      the message has been destroyed it is the client's responsibility to copy
      it. */
    uint8   text[1];
} HFP_CSR_FEATURES_TEXT_IND_T;


/*! 
    @brief A new SMS has arrived.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! SMS Index value. */
    uint16 index;
    /*! Offset of sender number (in bytes) from sender_info.*/
    uint16 offset_number;
    /*! Size of sender number (in bytes) */
    uint16 size_number;
    /*! Offset of sender name (in bytes) from sender_info.*/
    uint16 offset_name;
    /*! Size of sender name (in bytes) */
    uint16 size_name;
    /*! Number and Name of the sender of the SMS message. The client should not 
        attempt to free this pointer, the memory will be freed when the message 
        is destroyed. If the client needs access to this data after the message 
        has been destroyed it is the client's responsibility to copy it. */
    uint8 sender_info[1];
} HFP_CSR_FEATURES_NEW_SMS_IND_T;


/*!
    @brief Body of requested text message
    
    The string will be truncated to 32 characters in length (including the NULL).
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! Command status. */
    hfp_lib_status status;
    /*! The number of bytes in the text string. */
    uint16  size_sms;
    /*! Body of the SMS message. The client should not attempt to free this
        pointer, the memory will be freed when the message is destroyed. If
        the client needs access to this data after the message has been 
        destroyed it is the client's responsibility to copy it. */
    uint8   sms[1];
} HFP_CSR_FEATURES_GET_SMS_CFM_T;


/*!
    @brief AG has requested that a Battery report be sent.
    
    Use HfpCsrFeaturesBatteryLevelRequest to send the report.
*/
typedef struct
{
    hfp_link_priority priority;
} HFP_CSR_FEATURES_BATTERY_LEVEL_REQUEST_IND_T;


/*!
    @brief This message informs the application of the outcome of this enable/
    disable command sent to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_CALLER_ID_ENABLE_CFM_T;


/*!
    @brief This message is sent in response to a request from the application
    to accept an incoming call.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_CALL_ANSWER_CFM_T;


/*!
    @brief This message is sent in response to a request from the application
    to terminate an outgoing call process or hang up an active call.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_CALL_TERMINATE_CFM_T;


/*!
    @brief This message is sent in response to a request from the application
    to the AG to enable /disable call waiting notifications.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_CALL_WAITING_ENABLE_CFM_T;


/*!
    @brief This message informs the application of the outcome of a call to
    HfpCallHoldActionRequest
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! The action that was requested */
    hfp_chld_action     action;
    /*! The status of the attempt */
    hfp_lib_status      status;
} HFP_CALL_HOLD_ACTION_CFM_T;


/*!
    @brief This message informs the application of the outcome of this request
    command sent to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! The response and hold action that was requested. */
    hfp_btrh_action     action;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. */
    hfp_lib_status      status;
} HFP_RESPONSE_HOLD_ACTION_CFM_T;


/*!
    @brief This message is sent in response to the AT button press command (as
    defined in the HSP specification) being sent to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_HS_BUTTON_PRESS_CFM_T;


/*!
    @brief This message is sent to the application to inform it of the outcome
    of sending a DTMF command to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*!<If the AG responds with OK to the AT command, status is set to
      hfp_success. If the AG responds with ERROR or the profile instance is in
      the wrong state to send out the command the status will be set to
      hfp_fail. */
    hfp_lib_status    status;
} HFP_DTMF_CFM_T;


/*!
    @brief This message informs the application of the outcome of this request
    command sent to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. */
    hfp_lib_status    status;
} HFP_SUBSCRIBER_NUMBERS_CFM_T;


/*!
    @brief This message informs the application of the outcome of this request
    command sent to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
      responds with ERROR or the profile instance is in the wrong state to send out the
      command the status will be set to hfp_fail. */
    hfp_lib_status      status;
} HFP_CURRENT_CALLS_CFM_T;


/*!
    @brief This message is sent to the application to inform it of the outcome
    of sending a network operator request to the AG.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority priority;
    /*! If the AG responds with OK to the AT command, status is set to hfp_success. If the AG
        responds with ERROR or the profile instance is in the wrong state to send out the
        command the status will be set to hfp_fail. */
    hfp_lib_status    status;
} HFP_NETWORK_OPERATOR_CFM_T;


/*!
    @brief This message is sent in response to the AT command requesting the AG
    supplies a number to attach to a voice tag.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! Command status. */
    hfp_lib_status status;
} HFP_VOICE_TAG_NUMBER_CFM_T;


/*!
    @brief Response from the AG to an application AT command.
*/
typedef struct
{
    /*! The priority of the link. */
    hfp_link_priority   priority;
    /*! Command status. */
    hfp_lib_status status;
} HFP_AT_CMD_CFM_T;


/*!
    @brief Initialise an instance of the HFP library.

    @param theAppTask The current application task.

    @param config Profile supported (HSP or HFP) plus any supported features.

    The application registers its own task, theAppTask, with the HFP library so
    that return messages can be routed to the correct task.

    The config parameter is used to configure the profile this HFP instance
    will support (HSP or HFP). If the HFP is being supported the supported
    features should also be supplied. These are passed in as a bit mask, as
    defined in the HFP specification. If the profile instance is being
    configured as HSP the supported features should be set to zero as they are
    not used.
    
    WARNING - Use of an HFP version other than HFP1.6 will require that a
    dynamic SDP record is created to reflect the correct version. This 
    increases baseline memory usage and should be avoided if possible.

    @param extra_indicators Additional indicators the AG may support.

    The HFP defines three indicators, (service, call and call_setup) that the
    AG must send to the HFP device. However, the GSM specification defines a
    number of other indicators the AG may support. The application can use
    extra_indicators to specify that it wants to be notified whether the AG
    supports a particular indicator and of its index in the CIND response.
    
    The extra_indicators should be specified as a string with carriage returns
    (@\r) separating each indicator. For example, the extra_indicators string
    might looks as follows, "battchg\rroam\rsounder". It is important that
    spaces are not inserted into the string. The application will be notified
    of the indicator's index (if it is supported by the AG) using a
    HFP_EXTRA_INDICATOR_INDEX_IND message.

    If extra indicator reporting is enabled then all indicator updates will be
    passed to the application (using a HFP_EXTRA_INDICATOR_UPDATE_IND message)
    and not just the indicators the application has registered an interest
    in. It is left up to the application to determine whether the indicator
    update is of interest. On SLC establishment the HFP lib will send AT+BIA 
    to the AG and will attempt to disable any extra indicators not included in 
    extra_indicators, in addition to those disabled in hfp_init_params, as HFP 
    assumes the application is not interested in them. Most applications will 
    not want to enable extra indicators, in which case extra_indicators should 
    be set to null.
    
    
    The application will receive an HFP_INIT_CFM message from the library to
    indicate the success or otherwise of the initialisation.
*/
void HfpInit(Task theAppTask, const hfp_init_params *config, const char* extra_indicators);


/*!
    @brief  Service records are visible during sdp
    
    Enables the service record corresponding to the specified service
*/
void hfpShowServiceRecords(void);


/*!
    @brief  Service records are not visible during sdp
    
    Disables the service record corresponding to the specified service
*/
void hfpHideServiceRecords(void);


/*!
    @brief Initate the creation of a Service Level Connection.

    @param bd_addr The address of the remote device (AG) to which the
    connection will be created.

    @param profiles A bitmask indicating all profiles to attempt to
    connect to for this device. If this bitmask contains HFP and HSP, 
    HFP will be attempted first.
    
    @param first_attempt The profile from profiles to try first.
    
    The application will receive an HFP_SLC_CONNECT_CFM message from the
    library to indicate the success or otherwise of the connection attempt.
*/
void HfpSlcConnectRequest(const bdaddr *bd_addr, hfp_profile profiles, hfp_profile first_attempt);


/*!
    @brief Initiate the disconnection of an Service Level Connection for a
    particular AG.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    The application will receive an HFP_SLC_DISCONNECT_IND message from the
    library to indicate the success or otherwise of the diconnection attempt(s).
*/
void HfpSlcDisconnectRequest(hfp_link_priority priority);


/*!
    @brief Transfer audio to/from HF from/to the AG

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param direction The direction in which to transfer audio. Setting to
    hfp_audio_transfer will transfer audio to whichever end of the link does 
    not currently have it. It is strongly recommended that hfp_audio_transfer
    not be used with hfp_any_link or hfp_all_links as results could be highly
    unpredictable.
    
    @param packet_type The packet type to use if connecting audio
    
    @param audio_params The audio parameters to use if connecting audio
    
    The application will receive either an HFP_AUDIO_CONNECT_CFM if audio was
    transferred to the HF, or an HFP_AUDIO_DISCONNECT_IND if audio was
    trasnferred to the AG
*/
void HfpAudioTransferRequest(hfp_link_priority priority, hfp_audio_transfer_direction direction, sync_pkt_type packet_type, const hfp_audio_params *audio_params);


/*!
    @brief Respond to HFP_AUDIO_CONNECT_IND

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param response Set to TRUE to accept, FALSE to reject
    
    @param packet_type The packet type to use
    
    @param audio_params The audio parameters to use
    
    @param disable_override Stop the lib setting WBS compliant eSCO 
    params/packet type (for test only)
    
    The application will receive an HFP_AUDIO_CONNECT_CFM message from the
    library to indicate the success or otherwise of the connection attempt(s).
*/
void HfpAudioConnectResponse(hfp_link_priority priority, bool response, sync_pkt_type packet_type, const hfp_audio_params *audio_params, bool disable_override);


/*!
    @brief Disconnect audio

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    The application will receive an HFP_AUDIO_DISCONNECT_IND message from the
    library to indicate the success or otherwise of the diconnection attempt(s).
*/
void HfpAudioDisconnectRequest(hfp_link_priority priority);


/*!
    @brief Answer an incoming call

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param accept Set TRUE to accept the call, FALSE to reject the call.
    
    The application will receive an HFP_CALL_ANSWER_CFM message from the
    library to indicate the success or otherwise of the answer attempt(s).
*/
void HfpCallAnswerRequest(hfp_link_priority priority, bool accept);


/*!
    @brief Terminate an incoming/outgoing/active call.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    The application will receive an HFP_CALL_TERMINATE_CFM message from the
    library to indicate the success or otherwise of the terminate attempt(s).
*/
void HfpCallTerminateRequest(hfp_link_priority priority);


/*!
    @brief Send HSP button press command if AG is connected using HSP

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    The application will receive an HFP_HS_BUTTON_PRESS_CFM message from the
    library to indicate the success or otherwise of the terminate attempt.
*/
void HfpHsButtonPressRequest(hfp_link_priority priority);


/*!
    @brief Enable call waiting notifications on a link. These are enabled
    by default.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param enable Set to TRUE to enable call waiting notifications, FALSE to
    disable.
    
    The application will receive an HFP_CALL_WAITING_ENABLE_CFM message from 
    the library to indicate the success or otherwise of this function.
*/
void HfpCallWaitingEnableRequest(hfp_link_priority priority, bool enable);


/*!
    @brief Request Current Calls from the AG (AT+CLCC)

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    The application will receive HFP_CURRENT_CALLS_IND for each call on the
    AG if the enhanced call status feature is supported by both HF and AG, 
    and HFP_CURRENT_CALLS_CFM indicating the success or failure of the
    request.
*/
void HfpCurrentCallsRequest(hfp_link_priority priority);


/*!
    @brief Send a CHLD command to the AG to perform a given three way calling
    command.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param action The action to perform
    
    @param index The index obtained from the current calls indication of the
    call on which to perform the action. Enhanced call control must be enabled
    on both the HF and AG to use this feature. To use normal CHLD commands
    set index to zero.
    
    The application will receive HFP_CALL_HOLD_ACTION_CFM indicating the success
    or failure of the request
*/
void HfpCallHoldActionRequest(hfp_link_priority priority, hfp_chld_action action, uint16 index);


/*!
    @brief Send a BTRH command to the AG to perform a given response and hold
    command.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param action The action to perform
    
    The application will receive HFP_RESPONSE_HOLD_ACTION_CFM indicating the success
    or failure of the request
*/
void HfpResponseHoldActionRequest(hfp_link_priority priority, hfp_btrh_action action);


/*!
    @brief Update the supported codec mask. If any optional WBS codec is included
    then the mandatory mSBC codec must also be included. CVSD must always be
    included. If these bits are not set correctly by the application they 
    will be overwritten by HFP. An AT+BAC command will be sent to any connected
    AG's to notify them of the change to the supported codecs. 

    @param codec_mask A mask indicating the codecs supported.
    
    @param send_notification Set to TRUE to notify any connected AG's of the change
    in supported codecs, FALSE to only notify when an AG attempts to negotiate a
    new codec.
    
    No message is returned by this function.
*/
void HfpWbsSetSupportedCodecs(hfp_wbs_codec_mask codec_mask, bool send_notification);


/*!
    @brief Send a new speaker volume to the AG

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param volume A pointer to a uint8 containing the volume to send. The
    volume is limited to 0-15, values above 15 will result in 15 being sent
    to the AG. 
    
    This will update volume to the actual value sent and return TRUE if 
    successful, otherwise volume will be unchanged and it will return FALSE
*/
bool HfpVolumeSyncSpeakerGainRequest(hfp_link_priority priority, uint8* volume);


/*!
    @brief Send a new microphone volume to the AG

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param volume A pointer to a uint8 containing the volume to send. The
    volume is limited to 0-15, values above 15 will result in 15 being sent
    to the AG. 
    
    This will update volume to the actual value sent and return TRUE if 
    successful, otherwise volume will be unchanged and it will return FALSE
*/
bool HfpVolumeSyncMicrophoneGainRequest(hfp_link_priority priority, uint8* volume);


/*!
    @brief Enable or disable caller ID notifications

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param enable TRUE to enable caller ID unsolicited notifications from the
    AG, FALSE to disable them.
*/
void HfpCallerIdEnableRequest(hfp_link_priority priority, bool enable);


/*!
    @brief Perform last number redial

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
*/
void HfpDialLastNumberRequest(hfp_link_priority priority);


/*!
    @brief Dial a specified number

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param length The length of the number string
    
    @param number An ASCII string containing the character representation of
    the number to call. This can include valid dial characters such as '+'.
*/
void HfpDialNumberRequest(hfp_link_priority priority, uint16 length, const uint8 *number);


/*!
    @brief Dial the number in a specific memory location

    @param priority The priority of the AG to perform this command on.
    
    @param length The length of the memory location string
    
    @param memory_location  An ASCII string character representation of location

    Only hfp_primary_link and hfp_secondary_link are valid settings for the
    priority parameter.
    Interpretation of the memory_location parameter is AG implementation specific.

*/
void HfpDialMemoryRequest(hfp_link_priority priority, uint16 length, const uint8 *memory_location);


/*!
    @brief Enable/Disable the voice recognition function on the AG

    @param priority The priority of the AG to perform this command on.
    
    @param enable TRUE to enable voice recognition, FALSE to disable.

    Only hfp_primary_link and hfp_secondary_link are valid settings for the
    priority parameter.
*/
void HfpVoiceRecognitionEnableRequest(hfp_link_priority priority, bool enable);


/*!
    @brief Request a number to associate with a voice tag. A headset supporting
    voice recognition may use this to request the AG user to select a number to
    associate with a particular voice tag. The number to associate with the voice
    tag is returned in a HFP_VOICE_TAG_NUMBER_IND message.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
*/
void HfpVoiceTagNumberRequest(hfp_link_priority priority);

/*!
    @brief Check if a the AG on a hfp link supports voice recognition.
    
    @param priority The priority of the AG to perform this command on.
    
    @return TRUE is hfp link supports voice recognition, FALSE otherwise.
*/
bool HfpVoiceRecognitionIsSupported(hfp_link_priority priority);

/*!
    @brief Request subscriber number(s) from the AG. Calling this function 
    will result in one or more HFP_SUBSCRIBER_NUMBER_IND being sent to the 
    application, each containing one of the subscriber's numbers.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
*/
void HfpSubscriberNumbersRequest(hfp_link_priority priority);


/*!
    @brief Send a DTMF code to the AG.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param dtmf The code to send, this must be a valid DTMF code ('0'-'9', 
    'A'-'D', '#' or '*')
*/
void HfpDtmfRequest(hfp_link_priority priority, uint8 dtmf);


/*!
    @brief Get the network operator for the AG.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
*/
void HfpNetworkOperatorRequest(hfp_link_priority priority);

/*!
    @brief Send the HF indicator value to the AG.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    @param assigned_id HF Indicator Assigned Number
    @param value - HF Indicator value as per valid values
*/
void HfpBievIndStatusRequest(hfp_link_priority priority, uint16 assigned_id, uint16 value);

/*!
    @brief Send an application specific AT command to the AG.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param cmd The AT command to send. String terminated with "@\r@\0" and
    with a maximum string length of HFP_MAX_ARRAY_LEN.
*/
void HfpAtCmdRequest(hfp_link_priority priority, const char* cmd);


/*!
    @brief Send CSR2CSR battery level reading to the AG. This will fail if
    either the HF or AG does not support CSR2CSR battery level readings.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param batt_level The battery level (0-9)
*/
void HfpCsrFeaturesBatteryLevelRequest(hfp_link_priority priority, uint16 batt_level);


/*!
    @brief Send CSR2CSR power source indication to the AG. This will fail if
    either the HF or AG does not support CSR2CSR power source indications.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param pwr_status The power source (battery or charger)
*/
void HfpCsrFeaturesPowerSourceRequest(hfp_link_priority priority, hfp_csr_power_status_report pwr_status);


/*!
    @brief Send CSR2CSR Get SMS request to the AG. This will fail if
    either the HF or AG does not support CSR2CSR SMS exchange.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param index The SMS index (indicated in HFP_CSR_FEATURES_NEW_SMS_IND)
*/
void HfpCsrFeaturesGetSmsRequest(hfp_link_priority priority, uint16 index);


/*!
    @brief Get the bluetooth address of a given link

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param bd_addr A pointer to a bdaddr to hold the result, this must be
    instantiated before calling this function
    
    This will return TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetBdaddr(hfp_link_priority priority, bdaddr* bd_addr);


/*!
    @brief Get the SLC Sink of a given link

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param sink A pointer to a Sink to hold the result, this must be
    instantiated before calling this function
    
    This will return TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetSlcSink(hfp_link_priority priority, Sink* sink);


/*!
    @brief Get the audio Sink of a given link

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function
    
    @param sink A pointer to a Sink to hold the result, this must be
    instantiated before calling this function
    
    This will return TRUE if successful, FALSE otherwise
*/
bool HfpLinkGetAudioSink(hfp_link_priority priority, Sink* sink);


/*!
    @brief Get the call state of a given link

    @param priority The priority of the AG to perform this command on.
    
    @param state Pointer to previously instantiated an hfp_call_state for resul

    @returns bool TRUE if successful, FALSE otherwise

    Only hfp_primary_link and hfp_secondary_link are valid settings for the
    priority parameter
*/
bool HfpLinkGetCallState(hfp_link_priority priority, hfp_call_state* state);


/*!
    @brief Set the maximum number of allowed connections. It is not possible
    to set max connections greater than the configured number, so if 
    multipoint support is not configured any setting other than one will fail.
    If reducing the number of connections this function will fail if an idle
    link is not available to be disabled.
    
    @param max_connections  The maximum allowed connections.
    
    @returns bool TRUE if successful, FALSE otherwise
*/
bool HfpLinkSetMaxConnections(uint8 max_connections);


/*!
    @brief Determine if any of the connected AG's are in the passed in state, 
    if a state match is found return the link priority

    @param  call_state The call state to match
    
    This will return link priority if match is found, invalid link if none 
    found
*/ 
hfp_link_priority HfpLinkPriorityFromCallState(hfp_call_state call_state);


/*!
    @brief Find a link with at least one active call
    
    @param link_with_audio_wanted If set to TRUE then this function will look for 
    the highest priority link with audio and at least one active call. If no call is 
    found it will return the highest priority link with audio. If set to FALSE
    then function will return the highest priority link with at least one active
    call.
    
    This will return link priority if match is found, invalid link if none 
    found
*/ 
hfp_link_priority HfpLinkPriorityWithActiveCall(bool link_with_audio_wanted);


/*!
    @brief Try to find a match for an audio sink
    
    @param sco_sink The sink to match
    
    This will return link priority if match is found, invalid link if none 
    found
*/ 
hfp_link_priority HfpLinkPriorityFromAudioSink(Sink sco_sink);


/*!
    @brief Try to find a match for a Bluetooth address
    
    @param bd_addr The Bluetooth address to match
    
    This will return link priority if match is found, invalid link if none 
    found
*/ 
hfp_link_priority HfpLinkPriorityFromBdaddr(const bdaddr* bd_addr);


/*!
    @brief Query if HFP is currently attempting to reconnect any
    lost links
    
    This will return TRUE if HFP is recovering from link loss, FALSE 
    otherwise
*/
bool HfpLinkLoss(void);


/*!
    @brief Query if HFP is currently attempting to reconnect any lost links
    and return the link priority if a match is found, invalid link if none found.
*/ 
hfp_link_priority HfpLinkLossLinkPriority(void);


/*!
    @brief   The link whether SC or non-SC is decided as a result of link encrypted after pairing.
    This function will set the link mode accordingly in HFP library.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function

    @param secure_link Whether the link is secure or not.

*/
void HfpLinkSetLinkMode(hfp_link_priority priority , bool secure_link);

/*!
    @brief   This function will return the link mode, whether SC or non-SC,  set in HFP library.

    @param priority The priority of the AG to perform this command on. Only
    hfp_primary_link and hfp_secondary_link are valid settings for this
    function

*/
bool HfpIsLinkSecure(hfp_link_priority priority);

/*!
    @brief Enables/disabled HFP library managed link loss recovery mechanism for the given link.

*/
void HfpManageLinkLoss(hfp_link_priority link_priority_to_manage , bool enable);


/*!
    @brief Get the bitmask of Qualcomm Codec Extensions mode supported Codec
           Mode IDs.

    @returns uint16 bitfield mask of all HF supported QCE Mode IDs (see
            codec_mode_bit_type, in library.h).

*/
uint16 HfpGetSupportedQceCodecModeIds(void);

/*!
    @brief Set the bitmask of Qualcomm Codec Extensions supported Codec Mode
           IDs.

    This can only be done when there is no existing SLC for any link. Only
    valid bit masks will be set (see codec_mode_bit_type in Library.h).

    @param hf_codecs_bitmask Bitmask of all required Codec Mode ID bits.

    @returns TRUE if successfully set, FALSE if the bitmask had no valid
    bits set OR the HF library has an active SLC link.
*/
bool HfpSetSupportedQceCodecModeIds(uint16 hf_codec_bitmask);


/*!
    @brief Get the bitmask of supported QCE Codec Mode IDs at the AG for
           the indicated link.


    Only hfp_primary_link and hfp_secondary_link are valid settings for the
    priority parameter

    @param priority The priority of the AG to perform this command on.

    @returns returns the AG Codec Mode IDs bitmask if successfull or
             #CODEC_MODE_ID_UNSUPPORTED (see library.h) if not connected or
            the AG is not in QCE mode.
*/
uint16 HfpGetSupportedQceCodecModeIdsInAg(hfp_link_priority priority);

/*!
    @brief Get the selected Codec Mode ID, if it has been selected by the AG
           and both HF and AG are in Qualcomm Codec Extensions (QCE) mode.

    @param priority The priority of the AG to perform this command on.
    @param mode_id Pointer to return the selected Codec Mode ID in, if
           successful.

    @returns TRUE if a Codec Mode ID has been selected, otherwise false.
*/
bool HfpGetQceSelectedCodecModeId(hfp_link_priority priority, uint16 *mode_id);


/****************************************************************************
NAME    
    HfpPriorityIsHsp
    
DESCRIPTION
    Query if the connection for the profile passed in is HSP profile

RETURNS
    TRUE if HSP profile otherwise FALSE
*/
bool HfpPriorityIsHsp(hfp_link_priority priority);

/****************************************************************************
NAME    
    HfpGetFirstIncomingCallPriority
    
DESCRIPTION
    This function will return link priority of the first incoming call

RETURNS
    hfp_link_priority
*/
hfp_link_priority HfpGetFirstIncomingCallPriority(void);

extern const hfp_audio_params default_s4_esco_audio_params;
extern const hfp_audio_params default_esco_audio_params;
extern const hfp_audio_params default_sco_audio_params;

#endif /* HFP_H_ */

/** @} */
