/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    aghfp_audio_handler.c

DESCRIPTION

NOTES

*/


/****************************************************************************
    Header files
*/
#include "aghfp.h"
#include "aghfp_private.h"
#include "aghfp_call_manager.h"
#include "aghfp_audio_handler.h"
#include "aghfp_common.h"

#include "aghfp_wbs_handler.h"
#include "aghfp_qce_handler.h"
#include "aghfp_csr_features.h"

#include <bdaddr.h>
#include <panic.h>
#include <sink.h>
#include <stream.h>


/* Default parameters for an S1 esco connection.  Configured to obtain best ESCO link possible. */
static const sync_config_params default_s1_sync_config_params =
{
    8000,                    /* tx_bandwidth   */
    8000,                    /* rx_bandwidth   */
    0x7,                     /* max_latency    */
    sync_air_coding_cvsd,    /* voice_settings */
    sync_retx_power_usage,   /* retx_effort    */
    (sync_ev3 | sync_all_edr_esco)   /* packet_type    */
};

/* Default parameters for an T1"safe settings" eSCO connection.  These are the "T1" safe settings
   for an EV3 packet type as defined in the HFP spec.  All HFP v1.6 devices should
   support these parameters for an EV3 packet type. */
static const sync_config_params default_t1_sync_config_params =
{
    8000,                            /* tx_bandwidth   */
    8000,                            /* rx_bandwidth   */
    0x8,                             /* max_latency    */
    sync_air_coding_transparent,     /* voice_settings */
    sync_retx_link_quality,          /* retx_effort    */
    (sync_ev3 | sync_all_edr_esco)   /* packet_type    */
};

/* Default parameters for S4 eSCO connection.  These are the "S4" settings
   for a 2-EV3 packet type as defined in the HFP spec 1.7. */
static const sync_config_params default_s4_sync_config_params =
{
    8000,                            /* tx_bandwidth   */
    8000,                            /* rx_bandwidth   */
    0x0c,                            /* max_latency    */
    sync_air_coding_cvsd,            /* voice_settings */
    sync_retx_link_quality,          /* retx_effort    */
    (sync_3ev5 | sync_2ev5 | sync_3ev3)     /* packet_type    */
};

/* Default parameters for an eSCO connection.  These are the "S1" safe settings
   for an EV3 packet type as defined in the HFP spec.  All HFP v1.5 devices should
   support these parameters for an EV3 packet type.                                */
static const aghfp_audio_params default_esco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x0007,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_power_usage,   /* Retransmission Effort        */
    FALSE                    /* Use WB-Speech if available   */
};

/* Default parameters for a SCO connection.  The max latency is sufficient to
   support all SCO packet types - HV1, HV2 and HV3.                           */
static const aghfp_audio_params default_sco_audio_params =
{
    8000,                    /* Bandwidth for both Tx and Rx */
    0x0005,                  /* Max Latency                  */
    sync_air_coding_cvsd,    /* Voice Settings               */
    sync_retx_disabled,      /* Retransmission Effort        */
    FALSE                    /* Use WB-Speech if available   */
};

/* Set aghfp's audio parameters to imply no connection */
static void resetAudioParams (AGHFP *aghfp)
{
    aghfp->audio_sink = 0;
    aghfp->rx_bandwidth = 0;
    aghfp->tx_bandwidth = 0;
    aghfp->link_type = sync_link_unknown;
    aghfp->ag_attempt_settings = aghfp_audio_settings_none;
    aghfp->link_mode_secure = FALSE;
}

/* Attempt S4 if supported by both HF and AG exchanged during SLC */
static bool isS4Supported(AGHFP *aghfp)
{
    return((aghfp->supported_features & aghfp_esco_s4_supported) &&
            (aghfp->hf_supported_features & aghfp_hf_esco_s4_supported ));
}

/* Check if Codec negotiation is supported by both HF ang AG */
static bool isCodecNegotiationSupported(AGHFP *aghfp)
{
    return((aghfp->supported_features & aghfp_codec_negotiation) &&
            (aghfp->hf_supported_features & aghfp_hf_codec_negotiation ));
}

/****************************************************************************
NAME
    aghfpProceedAudioConn

DESCRIPTION
    Proceed audio connection only after checking valid supported features of
    AGHFP/HFP for S4 and WBS while BR/EDR secure link is in use.  If no SC in
    use (default), proceed with the connection.

MESSAGE RETURNED
    None

 RETURNS
    bool
*/
static bool aghfpProceedAudioConn(AGHFP *aghfp)
{
    if (aghfp && aghfp->link_mode_secure)
    {
        /*If both AG and HFP support S4, go ahead with connection. In this case both will be HFP version 1.7 */
        if (
                (isS4Supported(aghfp)) ||
                /* If AG and HFP support WBS, go ahead with connection (SC with T2 is possible here)*/
                (isCodecNegotiationSupported(aghfp))
           )
        {
            return TRUE;
        }
        /* For other profile violations, reject the connection */
        else
        {
            return FALSE;
        }
    }

    /*Default: Proceed with audio conn if no SC*/
    return TRUE;
}

/* Inform the app of the status of the audio (Synchronous) connection */
void sendAudioConnectCfmToApp(AGHFP *aghfp, aghfp_audio_connect_status status, hci_status cl_status)
{
    MAKE_AGHFP_MESSAGE(AGHFP_AUDIO_CONNECT_CFM);

    AGHFP_DEBUG(("sendAudioConnectCfmToApp\n"));

    message->aghfp = aghfp;
    message->status = status;
    message->cl_status = cl_status;
    message->audio_sink = aghfp->audio_sink;
    message->rx_bandwidth = aghfp->rx_bandwidth;
    message->tx_bandwidth = aghfp->tx_bandwidth;
    message->link_type = aghfp->link_type;
    message->qce_codec_mode_id = aghfp->qce_codec_mode_id;

    if ((aghfp->use_wbs) && (aghfp->use_codec > 0))
    {
        message->using_wbs = TRUE;
        message ->wbs_codec = aghfp->use_codec;
    }
    else
        message->using_wbs = FALSE;

    AGHFP_DEBUG(("\tStatus              : %d\n", status));
    AGHFP_DEBUG(("\tusing_wbs           : %d\n", message->using_wbs));
    AGHFP_DEBUG(("\tqce_codec_mode_id   : %d\n",message->qce_codec_mode_id));

    MessageSend(aghfp->client_task, AGHFP_AUDIO_CONNECT_CFM, message);
}


/* Inform the app of the status of the audio (Synchronous) disconnection */
static void sendAudioDisconnectIndToApp(AGHFP *aghfp, aghfp_audio_disconnect_status status)
{
    MAKE_AGHFP_MESSAGE(AGHFP_AUDIO_DISCONNECT_IND);
    message->aghfp = aghfp;
    message->status = status;

    MessageSend(aghfp->client_task, AGHFP_AUDIO_DISCONNECT_IND, message);
}


/* Attempt to create an audio (Synchronous) connection.  Due to firmware operation, eSCO and SCO must be requested
   separately. */
static void startAudioConnectRequest(AGHFP *aghfp)
{
    sync_config_params config_params;

    AGHFP_DEBUG(("startAudioConnectRequest\n"));
    AGHFP_DEBUG(("aghfp->hf_supported_features %d\n",aghfp->hf_supported_features));
    AGHFP_DEBUG(("aghfp_hf_codec_negotiation %d\n",aghfp_hf_codec_negotiation));
    AGHFP_DEBUG(("aghfp->ag_codec_modes %d ->hf_codec_modes %d\n", aghfp->ag_codec_modes, aghfp->hf_codec_modes));

    /* If currently no Audio is connected */
    if (aghfp->audio_connection_state == aghfp_audio_disconnected)
    {
        /* If both devices support Qualcomm Codec Extensions perform Qualcomm Codec
         * Selections (+%QCS) instead of the HFP Codec Connections Setup (+BCS).
         *
         * Logical AND the HF Codec Modes and AG Codec Modes to ensure at least one
         * mode is commonly supported, otherwise go with +BCS: instead.
         *
         * Done for EVERY audio connection.
         */
        if (aghfp->ag_codec_modes & aghfp->hf_codec_modes)
        {
            uint8 idx;
            uint16 codec_mode_bit;
            /* Select the first COMMONLY supported codec mode
             *
             * NOTE: this is currently done by bit order LOW to HIGH - a future
             * refactor may be to use a preferred order set by parameter, but for
             * our AGHFP implementation the simple solution is to only add the
             * Codec Mode ID bit that you want, for testing purposes.
             */
            for (idx=0; idx<16; idx++)
            {
                codec_mode_bit = 1 << idx;
                if (codec_mode_bit & aghfp->ag_codec_modes & aghfp->hf_codec_modes)
                {
                    aghfpQceSendCodecSelection(aghfp, codec_mode_bit);
                    aghfp->audio_connection_state = aghfp_audio_codec_connect;
                    return;
                }
            }
        }

        /* Start Codec Negotiaion if:
           Audio id disconnected AND
           WBS is not being overridden AND
           We wish to use WBS AND
           The AG indicates that it supports Codec Negotiation AND
           No codec has already previously been successfully negotiated
           */
        if ((aghfp->audio_connection_state == aghfp_audio_disconnected) && (!(aghfp->audio_params.override_wbs)) &&
                (aghfp->use_wbs) && (aghfp->hf_supported_features & aghfp_hf_codec_negotiation) &&
                (aghfp->use_codec == 0) && (aghfp->negotiation_type == aghfp_codec_negotiation_type_wbs))
        {
            AGHFP_DEBUG(("aghfpWbsStartCodecNegotiation\n"));
            if (aghfpWbsStartCodecNegotiation(aghfp, aghfp_negotiate_audio_at_ag))
            {
                aghfp->audio_connection_state = aghfp_audio_codec_connect;
                return;
            }
        }

    } /* Endif - no Audio connection */

    /* added to meet wbs test spec
       if we currently use wbs but the HF does not support it then change the audio
       params so that a CVSD SCO can be negotiatiated */
    if((aghfp->use_wbs) && !(aghfp->hf_supported_features & aghfp_hf_codec_negotiation))
    {
        AGHFP_DEBUG(("using default CVSD Sco parameters \n"));
        aghfp->audio_packet_type = 0x2BF;
        aghfp->audio_params.bandwidth = 8000;
        aghfp->audio_params.max_latency = 16;
        aghfp->audio_params.voice_settings = 0;
    }


    if((aghfp->negotiation_type == aghfp_codec_negotiation_type_csr) && (aghfp->app_pending_codec_negotiation))
    {
        AGHFP_DEBUG(("aghfpSendCsrFeaturesNegotiationReqInd\n"));
        /* Prompt the app to negotiate the codec */
        aghfpSendCsrFeatureNegotiationReqInd(aghfp);
        aghfp->audio_connection_state = aghfp_audio_codec_connect;
        return;
    }

    AGHFP_DEBUG(("aghfp->audio_packet_type %d\n", aghfp->audio_packet_type));
    /* Save the packet type for use later if this connection fails. */
    aghfp->audio_packet_type_to_try = aghfp->audio_packet_type;

    /* Set the audio attempt setting to application settings */
    aghfp->ag_attempt_settings = aghfp_audio_settings_app;

    if ( aghfp->audio_packet_type & sync_all_esco )
    {   /* Attempt to open an eSCO connection */
        AGHFP_DEBUG(("attempt to open an eSCO connection\n"));
        aghfp->audio_connection_state = aghfp_audio_connecting_esco;
        config_params.retx_effort = aghfp->audio_params.retx_effort;
    }
    else
    {   /* Attempt to open a SCO connection */
        AGHFP_DEBUG(("attempt to open a SCO connection\n"));
        aghfp->audio_connection_state = aghfp_audio_connecting_sco;
        /* No re-transmissions for SCO */
        config_params.retx_effort = sync_retx_disabled;
    }
    /* set packet type - pass through without checking anything*/
    config_params.packet_type = aghfp->audio_packet_type ;


    config_params.tx_bandwidth = aghfp->audio_params.bandwidth;
    config_params.rx_bandwidth = aghfp->audio_params.bandwidth;
    config_params.max_latency = aghfp->audio_params.max_latency;
    config_params.voice_settings = aghfp->audio_params.voice_settings;

    AGHFP_DEBUG(("config_params.packet_type %d\n", config_params.packet_type));
    AGHFP_DEBUG(("config_params.tx_bandwidth %d\n", (int)config_params.tx_bandwidth));
    AGHFP_DEBUG(("config_params.rx_bandwidth %d\n", (int)config_params.rx_bandwidth));
    AGHFP_DEBUG(("config_params.max_latency %d\n", config_params.max_latency));
    AGHFP_DEBUG(("config_params.voice_settings %d\n", config_params.voice_settings));


    /* Issue a Synchronous connect request to the connection lib */
    ConnectionSyncConnectRequest(&aghfp->task, aghfp->rfcomm_sink, &config_params);
}


/* Continue with attempt to create an audio (Synchronous) connection.  Due to firmware operation, eSCO and SCO must be requested
   separately. */
static void continueAudioConnectRequest(AGHFP *aghfp)
{
    sync_config_params config_params;
    const sync_config_params* config = (const sync_config_params*) &config_params;

    /* On entry, aghfp->audio_packet_type will contain the packet types last attempted.
       Continue with connection attempt if we tried a packet type > hv1 or esco last time around. */
    if ( aghfp->audio_packet_type_to_try != (sync_hv1 | sync_all_edr_esco) )
    {
        /* If wbs supported and T2 or app params negotiation has failed try T1 first before going for cvsd */
        /* If the link is secure try S4 after T2*/
        if((!aghfp->link_mode_secure) && isCodecNegotiationSupported(aghfp) && (aghfp->ag_attempt_settings < aghfp_audio_settings_t1))
        {
            /* Use T1 audio settings */
            config = &default_t1_sync_config_params;
            /* Set the audio attempt setting to t1 */
            aghfp->ag_attempt_settings = aghfp_audio_settings_t1;
        }
        /* if wbs is not supported or fails or app params for cvsd fails try S4 before trying safe S1 */
        else if(isS4Supported(aghfp) && aghfp->ag_attempt_settings < aghfp_audio_settings_s4)
        {
            /* Use S4 audio settings */
            config = &default_s4_sync_config_params;
            /* Set the audio attempt setting to s4 */
            aghfp->ag_attempt_settings = aghfp_audio_settings_s4;
        }
        /*Check if link is BR/EDR secure connection. If so, disconnect the link, as safe settings (T2/S4) already attempted and failed*/
        else if(aghfp->link_mode_secure)
        {
            /* S4 settings attempted before. Abort hfp connection as Secure connection did not go through*/
            aghfp->audio_connection_state = aghfp_audio_disconnected;
            resetAudioParams(aghfp);

            /* Inform app that connect failed */
            sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_failure, hci_success);
            return;
        }
        /* if EDR bits attempted, try esco only by removing SCO and EDR bits */
        else if((aghfp->audio_packet_type_to_try & sync_all_edr_esco) != sync_all_edr_esco)
        {
            /* mask out SCO and EDR bits (inverted) from the original requested packet type */
            aghfp->audio_packet_type_to_try = ((aghfp->audio_packet_type & sync_all_esco) | sync_all_edr_esco);
        }
        /* if ESCO bits attempted that weren't EV3 only try EV3 with S1 settings */
        else if((aghfp->audio_packet_type_to_try & sync_all_esco)&&
                (aghfp->audio_packet_type_to_try != (sync_ev3 | sync_all_edr_esco)))
        {
            /* set to EV3 only */
            aghfp->audio_packet_type_to_try = (sync_ev3 | sync_all_edr_esco);
            /* Set the audio attempt setting to s1*/
            aghfp->ag_attempt_settings = aghfp_audio_settings_s1;
        }
        /* now down to SCO packet types, try HV1 only before giving up */
        else
        {
            aghfp->audio_packet_type_to_try = (sync_hv1 | sync_all_edr_esco);
            /* Set the audio attempt setting to d0 */
            aghfp->ag_attempt_settings = aghfp_audio_settings_d0;
        }

        aghfp->audio_connection_state = aghfp_audio_connecting_sco;
        config_params.tx_bandwidth = 8000;
        config_params.rx_bandwidth = 8000;
        config_params.retx_effort = sync_retx_disabled;
        config_params.packet_type = aghfp->audio_packet_type_to_try;
        config_params.max_latency = aghfp->audio_params.max_latency;
        config_params.voice_settings = sync_air_coding_cvsd;

        /* when trying EV3 S1 settings, substitute the S1 settings instead of user supplied settings */
        if(aghfp->audio_packet_type_to_try == (sync_ev3 | sync_all_edr_esco))
        {
            /* Issue a Synchronous connect request to the connection lib with s1 safe settings */
            ConnectionSyncConnectRequest(&aghfp->task, aghfp->rfcomm_sink, &default_s1_sync_config_params);
        }
        /* all non S1 EV3 attempts */
        else
        {
            /* Issue a Synchronous connect request to the connection lib */
            ConnectionSyncConnectRequest(&aghfp->task, aghfp->rfcomm_sink, config);
        }
    }
    else
    {
        /* All connection attempts have failed - give up */
        aghfp->audio_connection_state = aghfp_audio_disconnected;
        resetAudioParams(aghfp);

        /* Inform app that connect failed */
        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_failure, hci_success);
    }
}


/* Accept/reject an incoming audio connect request */
static void audioConnectResponse(AGHFP *aghfp, bool response, sync_pkt_type packet_type, const aghfp_audio_params *audio_params)
{
    tp_bdaddr tpaddr;
    sync_config_params config_params;

    AGHFP_DEBUG(("audioConnectResponse\n"));

    /* If connection request is being rejected, don't worry about validating params being returned */
    if ( !response )
    {
        AGHFP_DEBUG(("if !response\n"));

        /* To send the response we need the bd addr of the underlying connection. */
        if(SinkGetBdAddr(aghfp->rfcomm_sink, &tpaddr))
        {
            AGHFP_DEBUG(("SinkGetBdAddr\n"));

            ConnectionSyncConnectResponse(&aghfp->task, &tpaddr.taddr.addr, FALSE, 0);
        }
        /*
           If we can't get the addr from the sink then quietly don't respond.
           If the underlying ACL has gone down there's not much we can do.
           */
    }
    else
    {
        AGHFP_DEBUG(("setting audio params\n"));

        if((aghfp->use_wbs) && !(aghfp->hf_supported_features & aghfp_hf_codec_negotiation))
        {
            AGHFP_DEBUG(("using default CVSD Sco parameters \n"));
            config_params.tx_bandwidth = 8000;
            config_params.rx_bandwidth = 8000;
            config_params.max_latency = 16;
            config_params.voice_settings = 0;
            config_params.retx_effort = sync_retx_power_usage;
            config_params.packet_type = 0x2BF;
        }

        else
        {
            config_params.tx_bandwidth = audio_params->bandwidth;
            config_params.rx_bandwidth = audio_params->bandwidth;
            config_params.max_latency = audio_params->max_latency;
            config_params.voice_settings = audio_params->voice_settings;
            config_params.retx_effort = audio_params->retx_effort;
            config_params.packet_type = packet_type;
        }

        /*
           To send the response we need the bd addr of the underlying connection.
           If we can't get the addr from the sink then quietly don't respond.
           If the underlying ACL has gone down there's not much we can do.
           */
        if(SinkGetBdAddr(aghfp->rfcomm_sink, &tpaddr))
        {
            AGHFP_DEBUG(("ConnectionSyncConnectResponse\n"));
            ConnectionSyncConnectResponse(&aghfp->task, &tpaddr.taddr.addr, TRUE, &config_params);
        }
    }
}


/* Attempt to create a new audio (Synchronous) connection */
static void audioConnectRequest(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params)
{
    if ( !aghfpCallManagerActiveNotComplete(aghfp) )
    {
        if ( aghfpStoreAudioParams(aghfp, packet_type, audio_params) )
        {
            startAudioConnectRequest(aghfp);
        }
        else
        {
            /* Inform app that one or more parameters were invalid */
            sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_invalid_params, hci_success);
        }
    }
    else
    {
        /* Inform app that call manager is active, setting up or shutting down a call */
        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_call_manager_active, hci_success);
    }
}

/* Disconnect an existing audio (Synchronous) connection */
static void audioDisconnectRequest(AGHFP *aghfp)
{
    if ( !aghfpCallManagerActiveNotComplete(aghfp) )
    {
        /* Send a disconnect request to the connection lib */
        aghfp->audio_connection_state = aghfp_audio_disconnecting;
        ConnectionSyncDisconnect(aghfp->audio_sink, hci_error_oetc_user);
    }
    else
    {
        /* Inform app that call manager is active, setting up or shutting down a call */
        sendAudioConnectCfmToApp(aghfp, aghfp_audio_disconnect_call_manager_active, hci_success);
    }
}


/****************************************************************************
NAME
    aghfpGetDefaultAudioParams

DESCRIPTION
    Returns the most appropriate default audio parameters dependant on the
    specified packet type.

RETURNS
    Pointer to audio parameters.
*/
const aghfp_audio_params * aghfpGetDefaultAudioParams (sync_pkt_type packet_type)
{
    const aghfp_audio_params *audio_params;

    if ( packet_type & sync_all_esco )
    {
        audio_params = &default_esco_audio_params;
    }
    else
    {
        audio_params = &default_sco_audio_params;
    }

    return audio_params;
}


/****************************************************************************
NAME
    aghfpStoreAudioParams

DESCRIPTION
    Checks that the specified parameters are valid for the packet type being
    requested and stores the information for later use.

RETURNS
    TRUE on success, FALSE otherwise.
*/
bool aghfpStoreAudioParams(AGHFP *aghfp, sync_pkt_type packet_type, const aghfp_audio_params *audio_params)
{
    if (aghfp->audio_connection_state == aghfp_audio_disconnected || aghfp->audio_connection_state == aghfp_audio_codec_connect)
    {   /* Store connection parameters for later use */
        aghfp->audio_packet_type = packet_type;
        aghfp->audio_params = *audio_params;

        return TRUE;
    }

    return FALSE;
}


/****************************************************************************
  NAME
  aghfpAudioConnectRequest

  DESCRIPTION
  Kicks off an audio (synchronous) connection attempt with previously stored parameters,
  via aghfpStoreAudioParams.

  RETURNS
  TRUE if connection request is issued, FALSE otherwise.
  */
bool aghfpAudioConnectRequest(AGHFP *aghfp)
{
    /* If we don't have a valid audio handle, open a Synchronous connection */
    if (( aghfp->audio_connection_state==aghfp_audio_disconnected ) || ( aghfp->audio_connection_state==aghfp_audio_codec_connect ))
    {
        startAudioConnectRequest(aghfp);
        return TRUE;
    }

    return FALSE;
}


/****************************************************************************
NAME
    aghfpAudioDisconnectRequest

DESCRIPTION
    Kicks off an audio (synchronous) disconnection attempt.

RETURNS
    TRUE if disconnection request is issued, FALSE otherwise.
*/
bool aghfpAudioDisconnectRequest(AGHFP *aghfp)
{
    if ( aghfp->audio_connection_state==aghfp_audio_connected )
    {
        /* Send a disconnect request to the connection lib */
        aghfp->audio_connection_state = aghfp_audio_disconnecting;
        ConnectionSyncDisconnect(aghfp->audio_sink, hci_error_oetc_user);
        return TRUE;
    }

    return FALSE;
}


/****************************************************************************
NAME
    aghfpHandleSyncConnectInd

DESCRIPTION
    Incoming audio notification, accept if we recognise the sink reject
    otherwise.

RETURNS
    void
*/
void aghfpHandleSyncConnectInd(AGHFP *aghfp, const CL_DM_SYNC_CONNECT_IND_T *ind)
{
    uint16 i=0;
    uint16 my_audio = 0;
    uint16 num_sinks = 5;
    tp_bdaddr tp_addr = { {TYPED_BDADDR_PUBLIC, { 0, 0, 0 }}, TRANSPORT_BREDR_ACL};

    /* Sink array to store the sinks on the acl. */
    Sink *all_sinks = (Sink *)PanicNull(calloc(num_sinks, sizeof(Sink)));

    tp_addr.taddr.addr = ind->bd_addr;

    if(StreamSinksFromBdAddr(&num_sinks, all_sinks, &tp_addr))
    {
        for (i=0; i<num_sinks; i++)
        {
            /* Make sure this profile instance owns the unlerlying sink */
            if (all_sinks[i] && all_sinks[i] == aghfp->rfcomm_sink)
            {
                my_audio = 1;
                break;
            }
        }
    }

    free(all_sinks);

    /* If this is our audio connection then ask the app, otherwise reject outright */
    if (my_audio)
    {
        MAKE_AGHFP_MESSAGE(AGHFP_AUDIO_CONNECT_IND);
        message->aghfp = aghfp;
        message->bd_addr = ind->bd_addr;
        MessageSend(aghfp->client_task, AGHFP_AUDIO_CONNECT_IND, message);

        aghfp->audio_connection_state = aghfp_audio_accepting;
    }
    else
    {
        ConnectionSyncConnectResponse(&aghfp->task, &ind->bd_addr, FALSE, 0);
    }
}


/****************************************************************************
NAME
    aghfpHandleSyncConnectIndReject

DESCRIPTION
    Incoming audio notification, reject outright, profile is in the wrong
    state.  This is probably a audio ind for a different task.

RETURNS
    void
*/
void aghfpHandleSyncConnectIndReject(AGHFP *aghfp, const CL_DM_SYNC_CONNECT_IND_T *ind)
{
    /* Reject the Synchronous connect ind outright, we're in the wrong state */
    ConnectionSyncConnectResponse(&aghfp->task, &ind->bd_addr, FALSE, 0);
}


/****************************************************************************
NAME
    aghfpHandleSyncConnectCfm

DESCRIPTION
    Confirmation in response to an audio (SCO/eSCO) open request indicating the
    outcome of the Synchronous connect attempt.

RETURNS
    void
*/
void aghfpHandleSyncConnectCfm(AGHFP *aghfp, const CL_DM_SYNC_CONNECT_CFM_T *cfm)
{
    if ( aghfp->audio_connection_state==aghfp_audio_connecting_esco ||
            aghfp->audio_connection_state==aghfp_audio_connecting_sco ||
            aghfp->audio_connection_state==aghfp_audio_accepting )
    {
        /* Informs us of the outcome of the Synchronous connect attempt */
        if (cfm->status == hci_success)
        {
            /* store the audio parameters */
            aghfp->audio_sink = cfm->audio_sink;
            aghfp->rx_bandwidth = cfm->rx_bandwidth;
            aghfp->tx_bandwidth = cfm->tx_bandwidth;
            aghfp->link_type = cfm->link_type;
            aghfp->ag_attempt_settings = aghfp_audio_settings_none;
            aghfp->audio_connection_state = aghfp_audio_connected;

            /* Tell the app about this */
            if ( aghfpCallManagerActiveNotComplete(aghfp) )
            {       /* Audio connection request will have come from call manager */
                aghfpManageCall(aghfp, CallEventAudioConnected, CallFlagSuccess);
            }
            else
            {       /* Audio connection request will have come fom app */
                sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_success, cfm->status);
            }
        }
        else
        {
            /* Give up if we are either attempting to accept in incoming connection or error code
               indicates it is pointless to continue. */
            if ( cfm->status<=hci_error_host_timeout ||
                    (cfm->status>=hci_error_oetc_user && cfm->status<=hci_error_unknown_lmp_pdu) ||
                    aghfp->audio_connection_state==aghfp_audio_accepting )
            {
                aghfp->audio_connection_state = aghfp_audio_disconnected;

                resetAudioParams(aghfp);

                /* SCO/eSCO connect failed */
                if ( aghfpCallManagerActiveNotComplete(aghfp) )
                {       /* Audio connection request will have come from call manager */
                    aghfpManageCall(aghfp, CallEventAudioConnected, CallFlagFail);
                }
                else
                {       /* Audio connection request will have come fom app */
                    sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_failure, cfm->status);
                }
            }
            else
            {
                /* This step failed, move onto next stage of connection attempt */
                continueAudioConnectRequest(aghfp);
            }
        }
    }
    else
    {
        /* Should never get here */
        AGHFP_DEBUG(("aghfpHandleSyncConnectCfm invalid state %d",aghfp->audio_connection_state));
    }
}


/****************************************************************************
NAME
    aghfpHandleSyncDisconnectInd

DESCRIPTION
    Audio (Synchronous) connection has been disconnected

RETURNS
    void
*/
void aghfpHandleSyncDisconnectInd(AGHFP *aghfp, const CL_DM_SYNC_DISCONNECT_IND_T *ind)
{
    /* If it's not our sink, silently ignore this indication */
    if ( ind->audio_sink == aghfp->audio_sink )
    {
        if ( aghfp->audio_connection_state==aghfp_audio_connected ||
                aghfp->audio_connection_state==aghfp_audio_disconnecting )
        {
            /* Inform the app */
            if (ind->status == hci_success)
            {
                resetAudioParams(aghfp);
                aghfp->audio_connection_state = aghfp_audio_disconnected;

                if ( aghfpCallManagerActiveNotComplete(aghfp) )
                {
                    aghfpManageCall(aghfp, CallEventAudioDisconnected, CallFlagSuccess);
                }
                else
                {
                    sendAudioDisconnectIndToApp(aghfp, aghfp_success);
                }
            }
            else
            {
                /* Disconnect has failed, we are still connected - inform the app */
                if ( aghfpCallManagerActiveNotComplete(aghfp) )
                {
                    aghfpManageCall(aghfp, CallEventAudioDisconnected, CallFlagFail);
                }
                else
                {
                    sendAudioDisconnectIndToApp(aghfp, aghfp_fail);
                }
            }

            /* Update the local state. Check current state in case SLC disconnect has beaten the SCO/eSCO disconnect */
            if (!aghfpCallManagerActive(aghfp) && supportedProfileIsHsp(aghfp->supported_profile) && (aghfp->state != aghfp_ready))
            {
                aghfpSetState(aghfp, aghfp_slc_connected);
            }
        }
        else
        {
            /* Should never get here */
            AGHFP_DEBUG(("aghfpHandleSyncDisconnectInd invalid state %d\n",aghfp->audio_connection_state));
        }
    }
}


/****************************************************************************
NAME
    aghfpHandleAudioConnectReq

DESCRIPTION
    Transfer the audio from the AG to the HF or vice versa depending on which
    device currently has it.

RETURNS
    void
*/
void aghfpHandleAudioConnectReq(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_REQ_T *req)
{
    AGHFP_DEBUG(("aghfpHandleAudioConnectReq\n"));
    /* If we don't have a valid audio handle, open a Synchronous connection */
    switch ( aghfp->audio_connection_state )
    {
        case aghfp_audio_disconnected:
        case aghfp_audio_codec_connect:
            audioConnectRequest(aghfp, req->packet_type, &req->audio_params);
            break;
        case aghfp_audio_connecting_esco:
        case aghfp_audio_connecting_sco:
        case aghfp_audio_accepting:
            /* Already attempting to create an audio connection - indicate a fail for this attempt */
            sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_in_progress, hci_success);
            break;
        case aghfp_audio_disconnecting:   /* Until disconnect is complete, assume we have a connection */
        case aghfp_audio_connected:
            /* Already have an audio connection - indicate a fail for this attempt */
            sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_have_audio, hci_success);
            break;
        default:
            AGHFP_DEBUG(("aghfpHandleAudioConnectReq invalid state %d\n",aghfp->audio_connection_state));
            break;
    }
}


/****************************************************************************
NAME
    aghfpHandleAudioConnectReqError

DESCRIPTION
    The app has requested that the audio be transferred (either to or from the
    AG).  If the profile instance was in the wrong state for this operation to
    be performed we need to send an immediate response to the app indicating an
    error has ocurred. We send the a audio status message with the status code
    set to hfp_fail. We determine which message to send by looking at the
    action the app has requested and the current state of the HFP instance.

RETURNS
    void
*/
void aghfpHandleAudioConnectReqError(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_REQ_T *req)
{
    /* Send error message to inform the app we didn't open a Synchronous connection */
    sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_error, hci_success);
}


/****************************************************************************
NAME
    aghfpHandleAudioConnectRes

DESCRIPTION
    Accept or reject to an incoming audio connection request from remote device.

RETURNS
    void
*/
void aghfpHandleAudioConnectRes(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_RES_T *res)
{
    /*Check for valid AG/HFP supported features in use for BR/EDR secure connections*/
    if (aghfpProceedAudioConn(aghfp))
    {
        switch ( aghfp->audio_connection_state )
        {
            case aghfp_audio_disconnected:
            case aghfp_audio_accepting:
                audioConnectResponse(aghfp, res->response, res->packet_type, &res->audio_params);
                break;
            case aghfp_audio_connecting_esco:
            case aghfp_audio_connecting_sco:
                /* Already attempting to create an audio connection - indicate a fail for this attempt */
                sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_in_progress, hci_success);
                break;
            case aghfp_audio_disconnecting:   /* Until disconnect is complete, assume we have a connection */
            case aghfp_audio_connected:
                /* Already have an audio connection - indicate a fail for this attempt */
                sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_have_audio, hci_success);
                break;
            default:
                AGHFP_DEBUG(("aghfpHandleAudioConnectRes invalid state %d\n",aghfp->audio_connection_state));
                break;
        }
    }
    else /*Profile violations in case of SC*/
    {
        /* Reset audio params*/
        resetAudioParams(aghfp);

        /* Inform app that connect failed */
        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_failure, hci_success);
    }
}


/****************************************************************************
NAME
    aghfpHandleAudioConnectResError

DESCRIPTION
    Attempt has been made to accept/reject an incoming audio connection
    request.  However, HFP library is not in the correct state to process the
    response.

RETURNS
    void
*/
void aghfpHandleAudioConnectResError(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_CONNECT_RES_T *res)
{
    /* Send error message to inform the app we didn't respond to an incoming Synchronous connect request */
    sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_error, hci_success);
}


/****************************************************************************
NAME
    aghfpHandleAudioDisconnectReq

DESCRIPTION
    Attempt to disconnect the audio (Synchronous) connection.

RETURNS
    void
*/
void aghfpHandleAudioDisconnectReq(AGHFP *aghfp)
{
    switch ( aghfp->audio_connection_state )
    {
        case aghfp_audio_disconnected:
        case aghfp_audio_connecting_esco:  /* Until connect is complete, assume we don't have a connection */
        case aghfp_audio_connecting_sco:
        case aghfp_audio_accepting:
            /* Audio already with AG - indicate a fail for this attempt */
            sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_no_audio);
            break;
        case aghfp_audio_disconnecting:
            /* Already attempting to close an audio connection - indicate a fail for this attempt */
            sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_in_progress);
            break;
        case aghfp_audio_connected:
            audioDisconnectRequest(aghfp);
            break;
        default:
            AGHFP_DEBUG(("aghfpHandleAudioDisconnectReq invalid state %d\n",aghfp->audio_connection_state));
            break;
    }
}


/****************************************************************************
NAME
    aghfpHandleAudioDisconnectReqError

DESCRIPTION
    Attempt has been made to disconnect an audio connection request.  However,
    HFP library is not in the correct state to process the request.

RETURNS
    void
*/
void aghfpHandleAudioDisconnectReqError(AGHFP *aghfp)
{
    sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_error);
}


/****************************************************************************
NAME
    aghfpHandleAudioTransferReq

DESCRIPTION
    Transfer the audio from the AG to the HF or vice versa depending on which
    device currently has it.

RETURNS
    void
*/
void aghfpHandleAudioTransferReq(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req)
{
    /*Check for valid AG/HFP supported features in use for BR/EDR secure connections*/
    if (aghfpProceedAudioConn(aghfp))
    {
        switch (req->direction)
        {
            case aghfp_audio_to_hfp:
                switch ( aghfp->audio_connection_state )
                {
                    case aghfp_audio_disconnected:
                    case aghfp_audio_codec_connect:

                        audioConnectRequest(aghfp, req->packet_type, &req->audio_params);
                        break;
                    case aghfp_audio_connecting_esco:
                    case aghfp_audio_connecting_sco:
                    case aghfp_audio_accepting:
                        /* Already attempting to create an audio connection - indicate a fail for this attempt */
                        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_in_progress, hci_success);
                        break;
                    case aghfp_audio_disconnecting:   /* Until disconnect is complete, assume we have a connection */
                    case aghfp_audio_connected:
                        /* Already have an audio connection - indicate a fail for this attempt */
                        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_have_audio, hci_success);
                        break;
                    default:
                        AGHFP_DEBUG(("aghfpHandleAudioTransferReq invalid state %d\n",aghfp->audio_connection_state));
                        break;
                }
                break;

            case aghfp_audio_to_ag:
                switch ( aghfp->audio_connection_state )
                {
                    case aghfp_audio_disconnected:
                    case aghfp_audio_connecting_esco:  /* Until connect is complete, assume we don't have a connection */
                    case aghfp_audio_connecting_sco:
                    case aghfp_audio_accepting:
                        /* Audio already with AG - indicate a fail for this attempt */
                        sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_no_audio);
                        break;
                    case aghfp_audio_disconnecting:
                        /* Already attempting to close an audio connection - indicate a fail for this attempt */
                        sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_in_progress);
                        break;
                    case aghfp_audio_connected:
                        audioDisconnectRequest(aghfp);
                        break;
                    default:
                        AGHFP_DEBUG(("aghfpHandleAudioTransferReq invalid state %d\n",aghfp->audio_connection_state));
                        break;
                }
                break;

            case aghfp_audio_transfer:
                switch ( aghfp->audio_connection_state )
                {
                    case aghfp_audio_disconnected:
                    case aghfp_audio_codec_connect:
                        audioConnectRequest(aghfp, req->packet_type, &req->audio_params);
                        break;
                    case aghfp_audio_connecting_esco:
                    case aghfp_audio_connecting_sco:
                    case aghfp_audio_accepting:
                        /* Already attempting to create an audio connection - indicate a fail for this attempt */
                        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_in_progress, hci_success);
                        break;
                    case aghfp_audio_disconnecting:
                        /* Already attempting to close an audio connection - indicate a fail for this attempt */
                        sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_in_progress);
                        break;
                    case aghfp_audio_connected:
                        audioDisconnectRequest(aghfp);
                        break;
                    default:
                        AGHFP_DEBUG(("aghfpHandleAudioTransferReq invalid state %d\n",aghfp->audio_connection_state));
                        break;
                }
                break;

            default:
                AGHFP_DEBUG(("Unknown audio transfer direction\n"));
        }
    }
    else /*Profile violations in case of SC*/
    {
        /* Reset audio params*/
        resetAudioParams(aghfp);

        /* Inform app that connect failed */
        sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_failure, hci_success);
    }
}


/****************************************************************************
NAME
    aghfpHandleAudioTransferReqError

DESCRIPTION
    The app has requested that the audio be transferred (either to or from the
    AG).  If the profile instance was in the wrong state for this operation to
    be performed we need to send an immediate response to the app indicating an
    error has ocurred. We send the a audio status message with the status code
    set to hfp_fail. We determine which message to send by looking at the
    action the app has requested and the current state of the HFP instance.

RETURNS
    void
*/
void aghfpHandleAudioTransferReqError(AGHFP *aghfp, const AGHFP_INTERNAL_AUDIO_TRANSFER_REQ_T *req)
{
    switch (req->direction)
    {
        case aghfp_audio_to_hfp:
            /* Send error message to inform the app we didn't open a Synchronous connection */
            sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_error, hci_success);
            break;

        case aghfp_audio_to_ag:
            /* Send error message to inform the app we didn't close the Synchronous connection */
            sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_error);
            break;

        case aghfp_audio_transfer:
            /* Need to inform the app that the request failed. */
            if (aghfp->audio_sink)
            {
                sendAudioDisconnectIndToApp(aghfp, aghfp_audio_disconnect_error);
            }
            else
            {
                sendAudioConnectCfmToApp(aghfp, aghfp_audio_connect_error, hci_success);
            }
            break;

        default:
            AGHFP_DEBUG(("Unknown audio transfer direction\n"));
    }
}

