/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_audio_handler.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_audio_handler.h"
#include "hfp_common.h"
#include "hfp_link_manager.h"
#include "hfp_call_handler.h"
#include "hfp_hs_handler.h"

#include "hfp_wbs_handler.h"

#include <bdaddr.h>
#include <panic.h>
#include <print.h>

/* Mask out SCO bits from the original packet type */
#define EDR_ESCO_ONLY(pkt)  ((pkt) & (sync_all_esco | sync_all_edr_esco))

/* Mask out SCO and EDR bits (inverted) from the original packet type */
#define ESCO_ONLY(pkt)      (((pkt) & sync_all_esco) | sync_all_edr_esco)

/* EV3 only */
#define EV3_SAFE            (sync_ev3 | sync_all_edr_esco)

/* Mask out eSCO and EDR bits (inverted) from the original packet type */
#define SCO_ONLY(pkt)       (((pkt) & sync_all_sco) | sync_all_edr_esco)

/* returns > 0 if any packet type bit is set. */
#define IS_SCO(pkt)         ((pkt) & sync_all_sco)
#define IS_ESCO(pkt)        ((pkt) & sync_all_esco)

/* EDR ESCO bits are inverted, so logic checks they are NOT set. */
#define IS_EDR_ESCO(pkt)    (~((pkt) & sync_all_edr_esco) & sync_all_edr_esco)

/* HV1 only */
#define HV1_SAFE            (sync_hv1 | sync_all_edr_esco)

/* Including each EDR eSCO bit disables the options. This bitmap therefore enables EV3 and 2EV3 */
#define WBS_PACKET_TYPE     (sync_ev3 | sync_3ev5 | sync_2ev5 | sync_3ev3 /* | sync_2ev3 */)

/* Default parameters for an eSCO connection.  These are the "S4" settings 
   for a 2-EV3 packet type as defined in the HFP spec 1.7. */
const hfp_audio_params default_s4_esco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x000c,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_link_quality,   /* Retransmission Effort        */
};

/* Default parameters for an eSCO connection when "S4" is not supported.  These are the "S3" settings 
   for a 2-EV3 packet type as defined in the HFP spec. */
const hfp_audio_params default_esco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x000a,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_power_usage,   /* Retransmission Effort        */
};

/* Default parameters for a SCO connection.  The max latency is sufficient to 
   support all SCO packet types - HV1, HV2 and HV3. */
const hfp_audio_params default_sco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x0005,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_disabled,      /* Retransmission Effort        */
};

/* mSBC parameters for an eSCO connection. Configured to allow AG to request either 
"T1" safe settings or "T2" settings */
static const hfp_audio_params msbc_audio_params =
{
    8000,                           /* Bandwidth for both Tx and Rx */
    0x000e,                         /* Max Latency                  */
    sync_air_coding_transparent,    /* Voice Settings               */
    sync_retx_link_quality,         /* Retransmission Effort        */
};

/* eSCO parameter set Q0. Tx/Rx 64 kbps. Codec Mode ID 0 and 4. */
static const hfp_audio_params esco_parameter_set_q0 =
{
    8000,                           /* Tx and Rx Bandwidth */
    0x000E,                         /* Maximum latency */
    sync_air_coding_transparent,    /* Voice Settings - not relevant*/
    sync_retx_link_quality          /* 2 retransmissions */
};

/* eSCO parameter set Q1. Tx/Rx 128 kbps. Codec Mode ID 6. */
static const hfp_audio_params esco_parameter_set_q1 =
{
    16000,                          /* Tx and Rx Bandwidth */
    0x000E,                         /* Maximum latency */
    sync_air_coding_transparent,    /* Voice Settings - not relevant*/
    sync_retx_link_quality          /* 2 retransmissions */
};

/* eSCO parameter set Q2. Tx/Rx 64 kbps. Codec Mode ID 7. */
static const hfp_audio_params esco_parameter_set_q2 =
{
    8000,                           /* Tx and Rx Bandwidth */
    0x0013,                         /* Maximum latency */
    sync_air_coding_transparent,    /* Voice Settings - not relevant*/
    sync_retx_link_quality          /* 2 retransmissions */
};

/* "S4" settings  for a 2-EV3 packet type as defined in the HFP spec 1.7. */
static const sync_config_params default_s4_sync_config_params =
{
    8000,                            /* tx_bandwidth   */
    8000,                            /* rx_bandwidth   */
    0x0c,                            /* max_latency    */
    sync_air_coding_cvsd,            /* voice_settings */
    sync_retx_link_quality,          /* retx_effort    */
    (sync_3ev5 | sync_2ev5 | sync_3ev3)     /* packet_type    */
};

/* "S1" safe settings for EV3 connection.  Configured to obtain best ESCO link possible. */
static const sync_config_params default_s1_sync_config_params =
{
    8000,                           /* tx_bandwidth   */
    8000,                           /* rx_bandwidth   */
    0x7,                            /* max_latency    */
    sync_air_coding_cvsd,           /* voice_settings */
    sync_retx_power_usage,          /* retx_effort    */
    (sync_ev3 | sync_all_edr_esco)  /* packet_type    */
};


/* Attempt S4 if supported by both HF and AG exchanged during SLC */
bool isS4Supported(const hfp_link_data* link)
{
    return(hfFeatureEnabled(HFP_ESCO_S4_SUPPORTED) &&  agFeatureEnabled(link, AG_ESCO_S4_SUPPORTED));   
}


/* Work out the audio parameters to use */
const hfp_audio_params* hfpAudioGetParams(hfp_link_data* link, sync_pkt_type* packet_type, const hfp_audio_params* audio_params, bool disable_override)
{
    /* If Qualcomm Codec Extensions are defines and a Codec has been selected (AT+%QCS/+%QCS)
     * then HF ang AG are in QCE mode and this takes priority over other codec exchange or
     * link audio params.
     */
    if (link->qce_codec_mode_id != CODEC_MODE_ID_UNSUPPORTED)
    {
        /* Default - quivalent to 0x003F, but for testing force 2-EV3 (0x0380). */
        *packet_type = (sync_3ev5 | sync_2ev5 | sync_3ev3);

        switch (link->qce_codec_mode_id)
        {
            case aptx_adaptive_64_2_EV3:
                /* FALLTHROUGH */
            case aptx_adaptive_64_2_EV3_QHS3:
                return &esco_parameter_set_q0;
            case aptx_adaptive_128_QHS3:
                return &esco_parameter_set_q1;
            case aptx_adaptive_64_QHS3:
                return &esco_parameter_set_q2;
        }
    }

    /* If we're using mSBC */
    if(link->bitfields.wbs_codec_mask == hfp_wbs_codec_mask_msbc && !disable_override)
    {
        *packet_type = WBS_PACKET_TYPE;
        return &msbc_audio_params;
    }
    
    /* If app provided audio params */
    if(audio_params)
        return audio_params;
    /* If we're using eSCO and both HF and AG supports "S4" then use S4 default audio params
         else use "S3" default eSCO audio params.  */
    if(*packet_type & sync_all_esco)
    {
        if(isS4Supported(link))
        {
            link->bitfields.hfp_attempt_settings = hfp_audio_setting_s4;
            /* Both HF and AG supports S4, so try S4 default audio params */
            return &default_s4_esco_audio_params;
        }
        else
        {
            link->bitfields.hfp_attempt_settings = hfp_audio_setting_s3;
            return &default_esco_audio_params;
        }
    }
    /* Must be using SCO */
    return &default_sco_audio_params;
}


/* Set hfp's audio parameters to imply no connection */
static void resetAudioParams (hfp_link_data* link)
{
    link->bitfields.audio_state = hfp_audio_disconnected;
    link->audio_sink  = 0;
    /* Reset the audio settings to none */
    link->bitfields.hfp_attempt_settings = hfp_audio_setting_none;
}

/****************************************************************************
NAME    
    hfpProceedAudioConn

DESCRIPTION
    Proceed audio connection only after checking valid supported features of AGHFP/HFP for S4 and WBS
    while BR/EDR secure link is in use.
    If no SC in use (default), proceed with the connection.

MESSAGE RETURNED
    None
    
RETURNS
    bool
*/
static bool hfpProceedAudioConn(hfp_link_data* link)
{          
    if (link && link->bitfields.link_mode_secure)
    {
        /*If both AG and HFP support S4, go ahead with connection. In this case both will be HFP version 1.7 */
        if ((isS4Supported(link)) || 
            /* If AG and HFP support WBS, go ahead with connection (SC with T2 is possible here)*/
            (hfFeatureEnabled(HFP_CODEC_NEGOTIATION) && agFeatureEnabled(link, AG_CODEC_NEGOTIATION)))
        {
            return TRUE;
        }
        /*For other profile violations, reject the connection*/
        else
        {   
            return FALSE;
        }
    }

    /*Default: Proceed with audio conn if no SC*/
    return TRUE;
}


/* Inform the app of the status of the audio (Synchronous) connection */
void sendAudioConnectCfmFailToApp(const hfp_link_data* link, hfp_audio_connect_status status)
{
    if(status != hfp_audio_connect_success)
    {
        /* Send a cfm message to the application. */
        MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_CFM);
        message->priority           = hfpGetLinkPriority(link);
        message->status             = status;
        message->audio_sink         = link ? link->audio_sink : 0;
        message->rx_bandwidth       = 0;
        message->tx_bandwidth       = 0;
        message->link_type          = sync_link_unknown;
        message->codec              = hfp_wbs_codec_mask_none;
        message->wesco              = 0;
        message->qce_codec_mode_id = CODEC_MODE_ID_UNSUPPORTED;
        MessageSend(theHfp->clientTask, HFP_AUDIO_CONNECT_CFM, message);
    }
}


/* Inform the app of the status of the audio (Synchronous) disconnection */
void sendAudioDisconnectIndToApp(const hfp_link_data* link, hfp_audio_disconnect_status status)
{
    /* Send a cfm message to the application. */
    hfpSendCommonCfmMessageToApp(HFP_AUDIO_DISCONNECT_IND, link, status);
}


/* Attempt to create an audio (Synchronous) connection.  Due to firmware operation, eSCO and SCO must be requested
   separately. */
static void startAudioConnectRequest(hfp_link_data* link, sync_pkt_type packet_type, const hfp_audio_params *audio_params)
{
    sync_config_params config_params;

    /* If both sides support WBS kick off codec negotiation */
    if (hfpWbsStartCodecNegotiation(link))
    {
        link->bitfields.audio_state = hfp_audio_wbs_connect;
        return;
    }
    
    /* Store connection parameters for later use */
    link->bitfields.audio_state              = hfp_audio_connecting;
    link->audio_packet_type        = packet_type;
    link->audio_packet_type_to_try = packet_type;
    link->audio_params             = *audio_params;
    
    /* determine if esco or edr (inverted logic) packet types requested */
    if ( IS_ESCO(packet_type) |  IS_EDR_ESCO(packet_type) )
    {
        /* set packet type as the passed in esco and edr bits */
        config_params.packet_type = EDR_ESCO_ONLY(packet_type);
        config_params.retx_effort = audio_params->retx_effort;
    }
    else
    {
        /* mask out esco and add edr bits */
        config_params.packet_type = SCO_ONLY(packet_type);
        /* No re-transmissions for SCO */            
        config_params.retx_effort = sync_retx_disabled;
    }
    
    config_params.tx_bandwidth   = audio_params->bandwidth;
    config_params.rx_bandwidth   = audio_params->bandwidth;
    config_params.max_latency    = audio_params->max_latency;
    config_params.voice_settings = audio_params->voice_settings;
        
    /* Issue a Synchronous connect request to the connection lib */
    ConnectionSyncConnectRequest(&theHfp->task, hfpGetLinkSink(link), &config_params);
}


/* Continue with attempt to create an audio (Synchronous) connection.  Due to firmware operation, eSCO and SCO must be requested
   separately. */
static void continueAudioConnectRequest(hfp_link_data* link)
{
    /* On entry, link->audio_packet_type will contain the packet types last attempted.
       Continue with connection attempt if we tried a packet type > hv1 last time around. */
    if ( link->audio_packet_type_to_try != HV1_SAFE )
    {
        sync_config_params  config_params;
        const sync_config_params* config = (const sync_config_params*) &config_params;
        
        if((link->audio_packet_type_to_try & sync_all_edr_esco) != sync_all_edr_esco)
        {
            /* If EDR bits attempted, try esco only by removing SCO and EDR bits */
            link->audio_packet_type_to_try = ESCO_ONLY(link->audio_packet_type);
        }
        else if((link->audio_packet_type_to_try & sync_all_esco) && (link->audio_packet_type_to_try != EV3_SAFE))
        {
            /* If ESCO bits attempted that weren't EV3 only try EV3 with S1 settings */
            link->audio_packet_type_to_try = EV3_SAFE;
            /* Substitute the S1 settings instead of user supplied settings */
            config = &default_s1_sync_config_params;
        }
        else if(link->bitfields.link_mode_secure)
        {
            /* S1 safe settings attempted. Abort hfp connection as Secure connection on sco is unsupported */
            resetAudioParams(link);
    
            /* Inform app that connect failed */
            sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
            return;
        }
        else if(link->audio_packet_type_to_try == EV3_SAFE)
        {
            /* Now try all enabled sco packet bits */
            link->audio_packet_type_to_try = SCO_ONLY(link->audio_packet_type);
            /* Disable retransmits for SCO */
            link->audio_params.retx_effort = sync_retx_disabled;
        }
        else 
        {
            /* Now down to SCO packet types, try HV1 only before giving up */
            link->audio_packet_type_to_try = HV1_SAFE;
        }
        
        config_params.packet_type    = link->audio_packet_type_to_try;
        config_params.tx_bandwidth   = link->audio_params.bandwidth;
        config_params.rx_bandwidth   = link->audio_params.bandwidth;
        config_params.retx_effort    = link->audio_params.retx_effort;
        config_params.max_latency    = link->audio_params.max_latency;
        config_params.voice_settings = link->audio_params.voice_settings;
        
        ConnectionSyncConnectRequest(&theHfp->task, hfpGetLinkSink(link), config);
    }
    else
    {   /* All connection attempts have failed - give up */
        resetAudioParams(link);
    
        /* Inform app that connect failed */
        sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
    }
}


/****************************************************************************
NAME    
    continueSecureAudioConnectRequest

DESCRIPTION
    For BR/EDR Secure Connection, if S4 attempt has failed, disconnect the link. 
    If T2 attempt has failed, try S4. If that fails, disconnect the Link
    This function is called when either T2/S4 has failed for BR/EDR SC.

RETURNS
    void
*/
static void continueSecureAudioConnectRequest(hfp_link_data* link)
{
    bool reset = TRUE;
 
    if(link->bitfields.hfp_attempt_settings < hfp_audio_setting_s4) /*If T2 was attempted and failed*/
    {
        const sync_config_params* config = NULL;
        
        if(isS4Supported(link))
        {
            reset = FALSE;
            link->bitfields.hfp_attempt_settings = hfp_audio_setting_s4;
            
            /* Substitute the S4 settings */
            config = &default_s4_sync_config_params;
            
            ConnectionSyncConnectRequest(&theHfp->task, hfpGetLinkSink(link), config);
        }      
    }

    if (reset)
    {   
        /* All connection attempts have failed - give up */
        resetAudioParams(link);
    
        /* Inform app that connect failed */
        sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
    }   
}


/****************************************************************************
NAME    
    hfpHandleAudioConnectReq

DESCRIPTION
    Transfer the audio from the AG to the HF or vice versa depending on which
    device currently has it.

RETURNS
    void
*/
void hfpHandleAudioConnectReq(const HFP_INTERNAL_AUDIO_CONNECT_REQ_T *req)
{
    hfp_link_data*           link   = req->link;
    hfp_audio_connect_status status = hfp_audio_connect_success;
    
    switch ( link->bitfields.audio_state )
    {
        case hfp_audio_disconnected:
            startAudioConnectRequest(link, req->packet_type, &req->audio_params);
        break;
        
        case hfp_audio_connecting:
        case hfp_audio_accepting:
        case hfp_audio_accept_pending:
        case hfp_audio_wbs_connect:
            status = hfp_audio_connect_in_progress;
        break;
        
        case hfp_audio_disconnecting:
        case hfp_audio_connected:
            status = hfp_audio_connect_have_audio;
        break;
        
        default:
            HFP_DEBUG(("hfpHandleAudioConnectReq invalid state %d\n",link->bitfields.audio_state));
        break;
    }
    /* Send fail message if status is not success */
    sendAudioConnectCfmFailToApp(link, status);
}


/****************************************************************************
NAME    
    hfpHandleSyncConnectInd

DESCRIPTION
    Incoming audio notification, accept if we recognise the sink reject
    otherwise.

RETURNS
    void
*/
void hfpHandleSyncConnectInd(const CL_DM_SYNC_CONNECT_IND_T *ind)
{
    /* Get the link (if any) for this device */
    hfp_link_data* link = hfpGetLinkFromBdaddr(&ind->bd_addr);
    
    /* Make sure its our device and audio is disconnected or WBS connecting */
    if(link && (link->bitfields.audio_state == hfp_audio_disconnected || link->bitfields.audio_state == hfp_audio_wbs_connect))
    {
        /* Tell the application */
        MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_IND);
        message->priority = hfpGetLinkPriority(link);
        message->codec    = link->bitfields.wbs_codec_mask ? link->bitfields.wbs_codec_mask : hfp_wbs_codec_mask_cvsd;
        MessageSend(theHfp->clientTask, HFP_AUDIO_CONNECT_IND, message);
        
        link->bitfields.audio_state = hfp_audio_accepting;
    }
    else if(link && link->bitfields.audio_state == hfp_audio_connecting)
    {
        link->bitfields.audio_state = hfp_audio_accept_pending;
    }
    else
    {
        /* Not interested in this device, reject */
        ConnectionSyncConnectResponse(&theHfp->task, &ind->bd_addr, FALSE, 0);
    }
}


/* Accept/reject an incoming audio connect request */
static void audioConnectResponse(hfp_link_data* link, bool response, sync_pkt_type packet_type, const hfp_audio_params *audio_params)
{
    bdaddr bd_addr;
    bool   have_addr = hfpGetLinkBdaddr(link, &bd_addr);
    
    if(response && have_addr)
    {
        /* Set up audio config */
        sync_config_params           config_params;
        memset(&config_params, 0 ,sizeof(sync_config_params));
        config_params.tx_bandwidth   = audio_params->bandwidth;
        config_params.rx_bandwidth   = audio_params->bandwidth;
        config_params.max_latency    = audio_params->max_latency;
        config_params.voice_settings = audio_params->voice_settings;
        config_params.packet_type    = packet_type;
        config_params.retx_effort    = audio_params->retx_effort;
        /* Accept the request */
        ConnectionSyncConnectResponse(&theHfp->task, &bd_addr, TRUE, &config_params);
    }
    else
    {
        /* Reject the request */
        if(have_addr)
            ConnectionSyncConnectResponse(&theHfp->task, &bd_addr, FALSE, 0);
        
        /* App rejected or we have no bdaddr, fail and tidy up the link */
        sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
        resetAudioParams(link);
    }
}


/****************************************************************************
NAME    
    hfpHandleAudioConnectRes

DESCRIPTION
    Accept or reject to an incoming audio connection request from remote device.

RETURNS
    void
*/
void hfpHandleAudioConnectRes(const HFP_INTERNAL_AUDIO_CONNECT_RES_T *res)
{
    hfp_link_data*           link   = res->link;
    hfp_audio_connect_status status = hfp_audio_connect_success;

    /*Check for valid AG/HFP supported features in use for BR/EDR secure connections*/
    if (hfpProceedAudioConn(link))
    {    
        /* Fail if not in hfp_audio_accepting state */
        switch ( link->bitfields.audio_state )
        {
            case hfp_audio_accept_pending:
                link->bitfields.audio_state = hfp_audio_connecting;
                /* Fall through */
            case hfp_audio_accepting:
                audioConnectResponse(link, res->response, res->packet_type, &res->audio_params);
            break;
            
            case hfp_audio_connecting:
                status = hfp_audio_connect_in_progress;
            break;
            
            case hfp_audio_disconnecting:   
            case hfp_audio_connected:
                status = hfp_audio_connect_have_audio;
            break;
            
            case hfp_audio_disconnected:
                status = hfp_audio_connect_failure;
            break;
            
            default:
                HFP_DEBUG(("hfpHandleAudioConnectRes invalid state %d\n",link->bitfields.audio_state));
            break;
        }
    }
    else /*Profile violations in case of SC*/
    {   
        /* Reset audio params*/
        resetAudioParams(link);

        /* Inform app that connect failed */
        status = hfp_audio_connect_failure;
    }
    
    /* Send fail message if status is not success */
    sendAudioConnectCfmFailToApp(link, status);
}


/****************************************************************************
NAME    
    hfpHandleSyncConnectCfm

DESCRIPTION
    Confirmation in response to an audio (SCO/eSCO) open request indicating 
    the outcome of the Synchronous connect attempt.

RETURNS
    void
*/
void hfpHandleSyncConnectCfm(const CL_DM_SYNC_CONNECT_CFM_T *cfm)
{
    /* Get the link */
    hfp_link_data* link = hfpGetLinkFromBdaddr(&cfm->bd_addr);
    
    /* If this isn't for us ignore it silently */
    if(link)
    {
        if ( link->bitfields.audio_state==hfp_audio_connecting || link->bitfields.audio_state==hfp_audio_accepting || link->bitfields.audio_state==hfp_audio_accept_pending )
        {
            /* Informs us of the outcome of the Synchronous connect attempt */
            if (cfm->status == hci_success)
            {
                /* Construct cfm message for the application. */
                MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_CFM);
                message->priority     = hfpGetLinkPriority(link);
                message->status       = hfp_audio_connect_success;
                message->audio_sink   = cfm->audio_sink;
                message->rx_bandwidth = cfm->rx_bandwidth;
                message->tx_bandwidth = cfm->tx_bandwidth;
                message->link_type    = cfm->link_type;
                message->codec        = link->bitfields.wbs_codec_mask ? link->bitfields.wbs_codec_mask : hfp_wbs_codec_mask_cvsd;
                message->wesco        = cfm->wesco;
                message->qce_codec_mode_id  = link->qce_codec_mode_id;
                MessageSend(theHfp->clientTask, HFP_AUDIO_CONNECT_CFM, message);
            
                /* store the audio sink */
                link->audio_sink  = cfm->audio_sink;
                link->bitfields.audio_state = hfp_audio_connected;
                
                /* Update the link call state if required*/
                hfpHandleCallAudio(link, TRUE);
                
                return;
            }
            else if( link->bitfields.link_mode_secure && link->bitfields.audio_state == hfp_audio_connecting)
            {
                /* either wbs attempt or audio params settings attempt(in case of cvsd) or s4 audio settings attempt failed */
                continueSecureAudioConnectRequest(link);
                return;
            }
            else if( link->bitfields.audio_state == hfp_audio_connecting )
            {
                /* This step failed, move onto next stage of connection attempt */
                continueAudioConnectRequest(link);
                return;
            }
            else if( link->bitfields.audio_state == hfp_audio_accept_pending )
            {
                /* Notify the app of pending SYNC connect ind */
                MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_IND);
                message->priority = hfpGetLinkPriority(link);
                MessageSend(theHfp->clientTask, HFP_AUDIO_CONNECT_IND, message);
                return;
            }
        }
        
        /* Either got here in odd state or incoming sync failed, tidy up the link */
        resetAudioParams(link);
        
        /* Inform app that connect failed */
        sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
    }
    /* sco connection made with HFP link not connected */
    else
    {
        /* Inform app that connection was made with no link */
        if (cfm->status == hci_success)
        {
            MAKE_HFP_MESSAGE(HFP_AUDIO_CONNECT_CFM);
            message->priority           = hfp_invalid_link;
            message->status             = hfp_audio_connect_no_hfp_link;
            message->audio_sink         = cfm->audio_sink;
            message->rx_bandwidth       = cfm->rx_bandwidth;
            message->tx_bandwidth       = cfm->tx_bandwidth;
            message->link_type          = cfm->link_type;
            message->wesco              = cfm->wesco;
            message->qce_codec_mode_id  = link->qce_codec_mode_id;

            /* Codec info is not available as connection made without link */
            message->codec        = hfp_wbs_codec_mask_none;
            MessageSend(theHfp->clientTask, HFP_AUDIO_CONNECT_CFM, message);
        }
    }
}


/* Disconnect an existing audio (Synchronous) connection */
static void audioDisconnectRequest(hfp_link_data* link)
{
    /* Send a disconnect request to the connection lib */
    link->bitfields.audio_state = hfp_audio_disconnecting;
    ConnectionSyncDisconnect(link->audio_sink, hci_error_oetc_user);
}


/****************************************************************************
NAME
    hfpHandleAudioDisconnectReq

DESCRIPTION
    Attempt to disconnect the audio (Synchronous) connection.

RETURNS
    void
*/
void hfpHandleAudioDisconnectReq(hfp_link_data* link)
{
    switch ( link->bitfields.audio_state )
    {
        case hfp_audio_disconnected:
        case hfp_audio_connecting:
        case hfp_audio_accepting:
        case hfp_audio_accept_pending:
            /* Audio already with AG - indicate a fail for this attempt */
            sendAudioDisconnectIndToApp(link, hfp_audio_disconnect_no_audio);
        break;
        case hfp_audio_disconnecting:   
            /* Already attempting to close an audio connection - indicate a fail for this attempt */
            sendAudioDisconnectIndToApp(link, hfp_audio_disconnect_in_progress);
        break;
        case hfp_audio_connected:
            audioDisconnectRequest(link);
        break;
        default:
            HFP_DEBUG(("hfpHandleAudioDisconnectReq invalid state %d\n",link->bitfields.audio_state));
        break;
    }
}


/****************************************************************************
NAME    
    hfpManageSyncDisconnect

DESCRIPTION
    Used to inform hfp of a synchronous (audio) disconnection.
    
RETURNS
    void
*/
void hfpManageSyncDisconnect(hfp_link_data* link)
{
    if ( link->bitfields.audio_state!=hfp_audio_disconnected )
    {
        /* Reset the audio handle */
        resetAudioParams(link);
        sendAudioDisconnectIndToApp(link, hfp_audio_disconnect_success);
    }
}


/****************************************************************************
NAME    
    hfpHandleSyncDisconnectInd

DESCRIPTION
    Audio (Synchronous) connection has been disconnected 

RETURNS
    void
*/
void hfpHandleSyncDisconnectInd(const CL_DM_SYNC_DISCONNECT_IND_T *ind)
{
    hfp_link_data* link = hfpGetLinkFromAudioSink(ind->audio_sink);
    
    /* If it's not our sink, silently ignore this indication */
    if ( link )
    {
        if ( link->bitfields.audio_state==hfp_audio_connected || link->bitfields.audio_state==hfp_audio_disconnecting )
        { 
            /* Inform the app */ 
            if (ind->status == hci_success)
            {
                hfpManageSyncDisconnect(link);
                
                /* Update the link call state if required*/
                hfpHandleCallAudio(link, FALSE);
            }
            else
            {
                /* Disconnect has failed, we are still connected - inform the app */
                sendAudioDisconnectIndToApp(link, hfp_audio_disconnect_failure);
            }
        }
        else
        {
            /* Should never get here */
            HFP_DEBUG(("hfpHandleSyncDisconnectInd invalid state %d\n",link->bitfields.audio_state));
            
            resetAudioParams(link);
    
            /* Inform app that connect failed */
            sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
        }
    }
}


/****************************************************************************
NAME    
    hfpHandleAudioTransferReq

DESCRIPTION
    Transfer the audio from the AG to the HF or vice versa depending on which
    device currently has it.

RETURNS
    void
*/
void hfpHandleAudioTransferReq(const HFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req)
{
    hfp_link_data* link = req->link;
    hfp_audio_transfer_direction direction = req->direction;

    /*Check for valid AG/HFP supported features in use for BR/EDR secure connections*/
    if (hfpProceedAudioConn(link))
    {    
        /* Work out direction if the application wasn't specific */
        if(direction == hfp_audio_transfer)
        {
            switch ( link->bitfields.audio_state )
            {
                case hfp_audio_disconnected:
                    direction = hfp_audio_to_hfp;
                    break;
                case hfp_audio_connecting:
                case hfp_audio_accepting:
                case hfp_audio_accept_pending:
                case hfp_audio_wbs_connect:
                    /* Already attempting to create an audio connection - indicate a fail for this attempt */
                    sendAudioConnectCfmFailToApp(link, hfp_audio_connect_in_progress);
                    break;
                case hfp_audio_disconnecting:   
                    /* Already attempting to close an audio connection - indicate a fail for this attempt */
                    sendAudioDisconnectIndToApp(link, hfp_audio_disconnect_in_progress);
                    break;
                case hfp_audio_connected:
                    direction = hfp_audio_to_ag;
                    break;
                default:
                    HFP_DEBUG(("hfpHandleAudioTransferReq invalid state %d\n",link->bitfields.audio_state));
                    break;
            }
        }
        
        if(direction == hfp_audio_to_hfp)
        {
            /* Connect Audio */
            MAKE_HFP_MESSAGE(HFP_INTERNAL_AUDIO_CONNECT_REQ);
            message->link         = link;
            message->packet_type  = req->packet_type;
            message->audio_params = req->audio_params;
            MessageSend(&theHfp->task, HFP_INTERNAL_AUDIO_CONNECT_REQ, message);
        }
        else if(direction == hfp_audio_to_ag)
        {
            /* Disconnect Audio */
            hfpSendCommonInternalMessage(HFP_INTERNAL_AUDIO_DISCONNECT_REQ, link);
        }
    }
    else /*Profile violations in case of SC*/
    {   
        /* Reset audio params*/
        resetAudioParams(link);

        /* Inform app that connect failed */
        sendAudioConnectCfmFailToApp(link, hfp_audio_connect_failure);
    }
}

