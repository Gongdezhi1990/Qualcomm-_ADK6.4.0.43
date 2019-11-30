/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_private.h
    
DESCRIPTION
    
*/

#ifndef HFP_PRIVATE_H_
#define HFP_PRIVATE_H_

#include "hfp.h"

#include <connection.h>
#include <message.h>
#include <app/message/system_message.h>
#include <stdlib.h>
#include <panic.h>

#ifndef MESSAGE_MAKE
/* 
   Normally picked up from message.h, but might not be present pre
   4.1.1h, so fall back to the old behaviour.
*/
#define MESSAGE_MAKE(N,T) T *N = PanicUnlessNew(T)
#endif

/* Macros for creating messages */
#define MAKE_HFP_MESSAGE(TYPE) TYPE##_T *message = (TYPE##_T*)PanicNull(calloc(1,sizeof(TYPE##_T)))
#define MAKE_HFP_MESSAGE_WITH_LEN(TYPE, LEN) TYPE##_T *message = (TYPE##_T *) PanicNull(calloc(1,sizeof(TYPE##_T) + LEN - 1))
#define COPY_HFP_MESSAGE(src, dst) *dst = *src;

/* Macro used to generate debug lib printfs. */
#ifdef HFP_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#define DEBUG_PRINT_ENABLED
#define HFP_DEBUG(x) {printf x;  Panic();}
#define HFP_ASSERT_FAIL(x) {printf x; Panic();}
#else
#define HFP_DEBUG(x)
#define HFP_ASSERT_FAIL(x) {Panic();}
#endif

/* Constants used by the HFP profile lib. */
#define AT_RESPONSE_TIMEOUT_5SEC    (D_SEC(5))
#define AT_RESPONSE_TIMEOUT_10SEC   (D_SEC(10))
#define AT_RESPONSE_TIMEOUT_30SEC   (D_SEC(30))

/* Supported features bit masks - HFP AG */
#define AG_THREE_WAY_CALLING        (1)
#define AG_NREC_FUNCTION            (1<<1)
#define AG_VOICE_RECOGNITION        (1<<2)
#define AG_IN_BAND_RING             (1<<3)
#define AG_VOICE_TAG                (1<<4)
#define AG_REJECT_CALL              (1<<5)
#define AG_ENHANCED_CALL_STATUS     (1<<6)
#define AG_ENHANCED_CALL_CONTROL    (1<<7)
#define AG_EXTENDED_ERROR_CODES     (1<<8)
#define AG_CODEC_NEGOTIATION        (1<<9)
#define AG_HF_INDICATORS            (1<<10)
#define AG_ESCO_S4_SUPPORTED        (1<<11)

/* Handy macros for checking HF/AG feature bits */
#define agFeatureEnabled(link, feature) (((link)->ag_supported_features & (feature)) ? TRUE : FALSE)
#define hfFeatureEnabled(feature)       ((theHfp->hf_supported_features & (feature)) ? TRUE : FALSE)

#define HFP_NO_CFM  0x00
#define MAX_HFP_CONNECTIONS 0x02

typedef enum
{
    hfp_slc_idle,       /* Nothing is happening on this link */
    hfp_slc_disabled,   /* This link is disabled - i.e. multipoint not supported */
                        /* In the following states, there should be a bdaddr */
    hfp_slc_searching,  /* We're searching for the remote server channel on SDP */
    hfp_slc_outgoing,   /* SDP search has completed and we're connecting RFCOMM */
    hfp_slc_incoming,   /* We're handling an incoming RFCOMM connection */
    hfp_slc_connected,  /* RFCOMM has connected, now exchanging AT commands */
    hfp_slc_complete    /* AT commands have been exchanged, channel is established */
} hfp_slc_state;

typedef enum
{
    hfp_no_call,                     /* No active call */
    hfp_active_call                  /* Active call */
} hfp_call;

typedef enum
{
    hfp_no_call_setup,               /* No call currently being established.*/
    hfp_incoming_call_setup,         /* HFP device currently ringing.*/
    hfp_outgoing_call_setup,         /* Call currently being dialed.*/
    hfp_outgoing_call_alerting_setup /* Remote end currently ringing.*/
} hfp_call_setup;

typedef enum
{
    hfp_no_held_call,                /* No call on hold */
    hfp_held_active,                 /* Held call and active call */
    hfp_held_remaining               /* Held call and no active call  */
} hfp_call_hold;

typedef enum
{
    hfp_incoming_call_held,          /* An incoming call has been put on hold.*/
    hfp_held_call_accepted,          /* A previously held incoming call has been accepted.*/
    hfp_held_call_rejected           /* A previously held incoming call has been rejected.*/
} hfp_response_hold_state;

typedef enum
{
    hfpNoCmdPending,
    hfpCmdPending,
    hfpCkpdCmdPending,
    hfpBrsfCmdPending,
    hfpQacCmdPending,
    hfpQcsCmdPending,
    hfpCindTestCmdPending,
    hfpCindReadCmdPending,
    hfpCmerCmdPending,
    hfpChldStatusCmdPending,
    hfpAtaCmdPending,
    hfpChupCmdPending,
    hfpBldnCmdPending,
    hfpBvraCmdPending,
    hfpClipCmdPending,
    hfpAtdNumberCmdPending,
    hfpAtdMemoryCmdPending,
    hfpBinpCmdPending,
    hfpNrecCmdPending,
    hfpVtsCmdPending,
    hfpCcwaCmdPending,
    hfpChldZeroCmdPending,
    hfpChldOneCmdPending,
    hfpChldOneIdxCmdPending,
    hfpChldTwoCmdPending,
    hfpChldTwoIdxCmdPending,
    hfpChldThreeCmdPending,
    hfpChldFourCmdPending,
    hfpCnumCmdPending,
    hfpBtrhStatusCmdPending,
    hfpBtrhZeroCmdPending,
    hfpBtrhOneCmdPending,
    hfpBtrhTwoCmdPending,
    hfpCopsFormatCmdPending,
    hfpCopsReqCmdPending,
    hfpCmeeCmdPending,
    hfpBiaCmdPending,
    hfpClccCmdPending,
    hfpBcsCmdPending,
    hfpCsrSfPending,
    hfpCsrBatPending,
    hfpCsrPwrPending,
    hfpCsrModIndPending,
    hfpCsrGetSmsPending,
    hfpBacCmdPending,
    hfpBccCmdPending,
    hfpVgsCmdPending,
    hfpBindCmdPending,
    hfpBindStatusCmdPending,
    hfpBindIndStatusCmdPending,
    hfpBievStatusCmdPending,
    hfpAtCmdPending
} hfp_at_cmd;

typedef struct
{
    uint16  service;
    uint16  call;
    uint16  call_setup;
    uint16  extra_call_setup;
    uint16  signal_strength;
    uint16  roaming_status;
    uint16  battery_charge;
    uint16  call_hold_status;
} hfp_indicators;

typedef struct
{
    /* Indexes of standard indicators in AG commands */
    hfp_indicators      indicator_idxs;
    /* Dynamic array holding indexes of non-standard 
    indicators the app has requested notification of */
    uint16              num_extra_indicator_idxs;
    uint16              *extra_indicator_idxs;
    /* Record of the total number of indicators the 
    AG supports (including extra) */
    uint16              num_indicators;
} hfp_indicator_indexes;

typedef struct
{
    /* Bit mask storing the status of  hf indicators in AG */
    uint16      hf_indicators_status;
} hfp_hf_indicators;


typedef enum
{
    hfp_audio_disconnected,
    hfp_audio_wbs_connect,
    hfp_audio_connecting,
    hfp_audio_accepting,
    hfp_audio_accept_pending,
    hfp_audio_connected,
    hfp_audio_disconnecting
} hfp_audio_connect_state;

typedef enum
{
    hfp_csr_caller_name = 1,
    hfp_csr_raw_text = 2,
    hfp_csr_sms = 3,
    hfp_csr_batt_level = 4,
    hfp_csr_pwr_source = 5,
    hfp_csr_codecs = 6,
    hfp_csr_codec_bandwidths = 7
} hfp_csr_features_idx;

typedef struct
{
    bdaddr        bd_addr;    /* During establishment identify link by bdaddr */
    Sink          sink;       /* Once link is established use the Sink */
} hfp_link_identifier;

typedef struct
{
    unsigned                    rfc_server_channel:8;
    hfp_profile                 profile:3;
    unsigned                    busy:1;
    unsigned                    unused:4;
} hfp_service_bitfields;

typedef struct
{
    uint32                      sdp_record_handle;
    hfp_service_bitfields       bitfields;
} hfp_service_data;

typedef struct
{
    hfp_link_identifier         identifier;
    hfp_service_data*           service;
    
    struct
    {
        unsigned                    owns_service:1;
        hfp_link_priority           priority:2;
        hfp_slc_state               ag_slc_state:3;
        hfp_call_state              ag_call_state:4;
        hfp_link_loss_status        ag_link_loss_state:2;
        hfp_profile                 ag_profiles_to_try:3;
        unsigned                    ag_csr_features_enabled:1;
           
        hfp_audio_connect_state     audio_state:3;
        hfp_at_cmd                  at_cmd_resp_pending:8;
        unsigned                    at_vol_pending:1;
        unsigned                    at_vol_setting:4;
        unsigned                    at_cops_format_set:1;
        unsigned                    at_chup_retried:1;
        
        unsigned                    wbs_started_by_hf:1;
        hfp_wbs_codec_mask          wbs_codec_mask:2;
        unsigned                    manage_linkloss:1;     /* When set, the linkloss recovery mechanism for this link is managed by HFP lib */	
        unsigned                    link_mode_secure:1;
        hfp_secure_audio_settings   hfp_attempt_settings:2;
        unsigned                    unused:9;
    } bitfields;
    
    hfp_csr_features            ag_csr_features;
    uint16                      ag_supported_features;
    hfp_indicator_indexes       ag_supported_indicators;
    hfp_hf_indicators           ag_supported_hf_indicators;  	
    
    Sink                        audio_sink;
    hfp_audio_params            audio_params;
    sync_pkt_type               audio_packet_type;
    sync_pkt_type               audio_packet_type_to_try;
    uint16                      ag_codec_modes;             /* Bitfield of QCE Mode IDS supported by AG             */
    uint16                      qce_codec_mode_id;          /* Selected Codec Mode Id                               */
} hfp_link_data;


typedef struct
{
    TaskData                    task;                       /* The HFP profile handler                              */
    Task                        clientTask;                 /* The application/higher layer task                    */
    uint16                      hf_supported_features;      /* The features supported by the headset                */
    hfp_csr_features            csr_features;               /* CSR2CSR features supported locally                   */
    const char *                extra_indicators;           /* Any extra indicators the client wants to know about  */
    hfp_optional_indicators     optional_indicators;        /* Optional indicators the HF may not want to receive   */    

    struct
    {
        unsigned                    link_loss_time:8;           /* The time in minutes to try and reconnect for         */
        unsigned                    link_loss_interval:8;       /* The time in seconds between reconnect attempts       */
        
        unsigned                    initialised:1;              /* Initialisation flag (set when init is complete)      */
        unsigned                    disable_nrec:1;             /* Disable Noise Reduction/Echo cancellation on AG      */
        unsigned                    extended_errors:1;          /* Enable extended error codes                          */
        unsigned                    services_hidden:1;          /* Hide the services during sdp                         */
        hfp_wbs_codec_mask          wbs_codec_mask:2;           /* The HF supported codecs                              */
        unsigned                    first_incoming_call:2;      /* Holds the priority information for first call        */
        unsigned                    unused:8;
    } bitfields;
  
    uint16                      busy_channel;               /* Channel being registered with SDP                    */
    hfp_hf_indicators_mask      hf_indicators;              /* HF indicators supported by HF */
    uint8                       num_links;
    uint8                       num_services;
    hfp_link_data *             links;                      /* Pointer to array of HFP links                        */
    hfp_service_data *          services;                   /* Pointer to array of HFP services                     */
    uint16                      hf_codec_modes;             /* Bitfield of QCE Mode IDS supported by HF             */
} hfp_task_data;

/* External link to the HFP data struct (instantiated in init.c) */
extern hfp_task_data* theHfp;


/* Hfp Profile Library private messages */
#define HFP_MSG_BASE     (0x0)

#define AT_COMMAND_LENGTH         20

typedef enum
{
/* SDP */
/*0x00*/    HFP_INTERNAL_SDP_REGISTER_REQ = HFP_MSG_BASE,

/* SLC connection */
/*0x01*/    HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_0_IND,
/*0x02*/    HFP_RFCOMM_LINK_LOSS_RECONNECT_LINK_1_IND,
/*0x03*/    HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_0_IND,
/*0x04*/    HFP_RFCOMM_LINK_LOSS_TIMEOUT_LINK_1_IND,
/*0x05*/    HFP_INTERNAL_SLC_DISCONNECT_REQ,
/*0x06*/    HFP_INTERNAL_GET_SINK_REQ,

/* AT cmd receive */
/*0x07*/    HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_0_IND,
/*0x08*/    HFP_INTERNAL_WAIT_AT_TIMEOUT_LINK_1_IND,

/* AT cmd send */
/*0x09*/    HFP_INTERNAL_AT_ANSWER_REQ,
/*0x0A*/    HFP_INTERNAL_AT_TERMINATE_REQ,
/*0x0B*/    HFP_INTERNAL_AT_BLDN_REQ,
/*0x0C*/    HFP_INTERNAL_AT_ATD_NUMBER_REQ,
/*0x0D*/    HFP_INTERNAL_AT_ATD_MEMORY_REQ,
/*0x0E*/    HFP_INTERNAL_AT_BVRA_REQ,
/*0x0F*/    HFP_INTERNAL_AT_VGS_REQ,
/*0x10*/    HFP_INTERNAL_AT_VGM_REQ,
/*0x11*/    HFP_INTERNAL_AT_CLIP_REQ,
/*0x12*/    HFP_INTERNAL_AT_BINP_REQ,
/*0x13*/    HFP_INTERNAL_AT_NREC_REQ,
/*0x14*/    HFP_INTERNAL_AT_VTS_REQ,
/*0x15*/    HFP_INTERNAL_AT_CCWA_REQ,
/*0x16*/    HFP_INTERNAL_AT_CHLD_REQ,
/*0x17*/    HFP_INTERNAL_AT_CNUM_REQ,
/*0x18*/    HFP_INTERNAL_AT_BTRH_STATUS_REQ,
/*0x19*/    HFP_INTERNAL_AT_BTRH_REQ,
/*0x1A*/    HFP_INTERNAL_AT_CLCC_REQ,
/*0x1B*/    HFP_INTERNAL_AT_COPS_REQ,
/*0x1C*/    HFP_INTERNAL_AT_CMEE_REQ,
/*0x1D*/    HFP_INTERNAL_AT_BIA_REQ,
/*0x1E*/    HFP_INTERNAL_AT_CMD_REQ,
    
/* Audio connection */
/*0x1F*/    HFP_INTERNAL_AUDIO_TRANSFER_REQ,
/*0x20*/    HFP_INTERNAL_AUDIO_CONNECT_REQ,
/*0x21*/    HFP_INTERNAL_AUDIO_CONNECT_RES,
/*0x22*/    HFP_INTERNAL_AUDIO_DISCONNECT_REQ,
    
/* CSR Extensions */
/*0x23*/    HFP_INTERNAL_CSR_SUPPORTED_FEATURES_REQ,
/*0x24*/    HFP_INTERNAL_CSR_BATTERY_LEVEL_REQ,
/*0x25*/    HFP_INTERNAL_CSR_POWER_SOURCE_REQ,
/*0x26*/    HFP_INTERNAL_CSR_MOD_INDS_REQ,
/*0x27*/    HFP_INTERNAL_CSR_MOD_INDS_DISABLE_REQ,
/*0x28*/    HFP_INTERNAL_CSR_GET_SMS_REQ,
/*0x29*/    HFP_INTERNAL_CSR_AG_BAT_REQ,
        
/* WB-Speech Messages */
/*0x2A*/    HFP_INTERNAL_AT_BAC_REQ,

/* HSP */
/*0x2B*/    HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_0_IND,
/*0x2C*/    HFP_INTERNAL_HS_INCOMING_TIMEOUT_LINK_1_IND,
/*0x2D*/    HFP_INTERNAL_AT_CKPD_REQ,
    
/* Top internal message */
/*0x2E*/    HFP_INTERNAL_TOP

} HFP_INTERNAL_T;

typedef struct
{
    hfp_service_data* service;
} HFP_INTERNAL_SDP_REGISTER_REQ_T;

typedef struct
{
    hfp_link_data* link;
} HFP_INTERNAL_SLC_DISCONNECT_REQ_T;

typedef struct
{
    hfp_link_data* link;
    bool           accept;
} HFP_INTERNAL_AT_ANSWER_REQ_T;

typedef struct 
{
    hfp_link_data* link;
    uint16         length;
    uint8          number[1];
} HFP_INTERNAL_AT_ATD_NUMBER_REQ_T;

typedef struct
{
    hfp_link_data* link;
    uint16         length;
    uint8          memory[1];
} HFP_INTERNAL_AT_ATD_MEMORY_REQ_T;

typedef struct
{
    hfp_link_data* link;
    uint16         enable;
} HFP_INTERNAL_AT_BVRA_REQ_T;

typedef struct
{
    hfp_link_data* link;
    bool           enable;
} HFP_INTERNAL_AT_CLIP_REQ_T;

typedef struct
{
    hfp_link_data* link;
    uint8          dtmf;
} HFP_INTERNAL_AT_VTS_REQ_T;

typedef struct
{
    hfp_link_data* link;
    bool           enable;
} HFP_INTERNAL_AT_CCWA_REQ_T;

typedef struct
{
    hfp_link_data*  link;
    hfp_chld_action action;
    uint16          index;
} HFP_INTERNAL_AT_CHLD_REQ_T;

typedef struct
{
    hfp_link_data*  link;
    hfp_btrh_action action;
} HFP_INTERNAL_AT_BTRH_REQ_T;

typedef struct
{
    hfp_link_data*  link;
    uint8           size_cmd;
    char            cmd[1];
} HFP_INTERNAL_AT_CMD_REQ_T;

typedef struct
{
    hfp_link_data*               link;
    hfp_audio_transfer_direction direction;
    sync_pkt_type                packet_type;
    hfp_audio_params             audio_params;
} HFP_INTERNAL_AUDIO_TRANSFER_REQ_T;

typedef struct
{
    hfp_link_data*   link;
    sync_pkt_type    packet_type;
    hfp_audio_params audio_params;
} HFP_INTERNAL_AUDIO_CONNECT_REQ_T;

typedef struct
{
    hfp_link_data*   link;
    bool             response;
    sync_pkt_type    packet_type;
    hfp_audio_params audio_params;
} HFP_INTERNAL_AUDIO_CONNECT_RES_T;

typedef struct
{
    hfp_link_data* link;
    uint16         length;
    uint8          data[1];
} HFP_INTERNAL_USER_DATA_REQ_T;

typedef struct
{
    hfp_link_data* link;
    uint16         batt_level;
} HFP_INTERNAL_CSR_BATTERY_LEVEL_REQ_T;

typedef struct
{
    hfp_link_data*              link;
    hfp_csr_power_status_report pwr_status;
} HFP_INTERNAL_CSR_POWER_SOURCE_REQ_T;

typedef struct
{
    hfp_link_data*         link;
    /* TODO */
} HFP_INTERNAL_CSR_MOD_INDS_REQ_T;

typedef struct
{
    hfp_link_data* link;
    uint16         index;
} HFP_INTERNAL_CSR_GET_SMS_REQ_T;

/* 
    Many messages returned from the HFP lib to the app have 
    the form of this message. By defining it here its not visible 
    outside this lib. This way we can use a common function to 
    allocate this message and just set the type when sending the
    message to the app. 
*/
typedef struct
{
    hfp_link_priority priority;
    hfp_lib_status    status;
} HFP_COMMON_CFM_MESSAGE_T;

/*
    All internal messages except HFP_INTERNAL_SDP_REGISTER_REQ
    will have hfp_link_data* as the first parameter, we use the
    following message as a cast in order to perform checks on 
    the link in profile_handler.
*/
typedef struct
{
    hfp_link_data* link;
} HFP_COMMON_INTERNAL_MESSAGE_T;

#endif /* HFP_PRIVATE_H_ */
