/*
Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
*/
/**
\file
\ingroup sink_app
\brief
    This file handles all Synchronous connection messages

*/


/****************************************************************************
    Header files
*/
#ifdef ENABLE_BISTO
#include <sink2bisto.h>
#endif /* ENABLE_BISTO */
#include "sink_private_data.h"
#include "sink_main_task.h"
#include "sink_audio.h"
#include "sink_hfp_data.h"
#include "sink_statemanager.h"
#include "sink_powermanager.h"
#include "sink_pio.h"
#include "sink_link_policy.h"
#include "sink_tones.h"
#include "sink_volume.h"
#include "sink_speech_recognition.h"
#include "sink_wired.h"
#include "sink_display.h"
#include "sink_audio_routing.h"
#include "sink_peer.h"
#include "sink_swat.h"
#include "sink_ba.h"
#include "sink_ba_broadcaster.h"
#include "sink_ble.h"
#include "sink_callmanager.h"
#include "sink_malloc_debug.h"
#include "sink_usb.h"

#include <audio_plugin_voice_variants.h>
#include <audio_plugin_output_variants.h>
#include <connection.h>
#include <hfp.h>
#include <stdlib.h>
#include <audio.h>
#include <audio_plugin_if.h>
#include <sink.h>
#include <bdaddr.h>
#include <inquiry.h>
#include "sink_sc.h"
#include "sink_gaia.h"

/* Audio params used for second SCO */
const hfp_audio_params multipoint_params = {8000, 7, sync_air_coding_cvsd, sync_retx_power_usage};

#define ALLOWANCE_FOR_TRANSITION_IN_HFP_CALL_STATE        1000

#ifdef DEBUG_AUDIO

#define AUD_DEBUG(x) DEBUG(x)
#else
#define AUD_DEBUG(x)
#endif

/****************************************************************************
NAME
    audioPriorLatency

DESCRIPTION
    Return initial latency estimate for given endpoint codec

RETURNS
    Predicted latency in milliseconds
*/
uint16 audioPriorLatency(uint8 seid)
{
    uint16 latency_ms;

#if defined ENABLE_PEER && defined PEER_TWS
    if (peerIsThisDeviceTwsSlave())
    {
        /* TWS Master will send time-to-play values when it relays, hence target latency on slave should be zero */
        latency_ms = AUDIO_LATENCY_TWS_NULL;
    }
    else
#endif /* defined ENABLE_PEER && defined PEER_TWS */
    {
        switch (seid & BASE_SEID_MASK)
        {
        case FASTSTREAM_SEID:
        case APTX_SPRINT_SEID:
        case APTX_LL_SEID:
            latency_ms = AUDIO_LATENCY_SHORT_MS;
            break;

        case APTXHD_SEID:
            latency_ms = AUDIO_LATENCY_LONG_MS;
            break;

        case AAC_SEID:
            /* Use the same latency for AAC regardless of whether we are in TWS or non-TWS use case
              * This will only work if the AAC decoder is capable of latency this high in non-TWS cases
              */
            latency_ms = AUDIO_LATENCY_AAC_MS;
            break;

        case APTX_SEID:
            latency_ms = AUDIO_LATENCY_APTX_MS;
            break;

        case SBC_SEID:
            latency_ms = AUDIO_LATENCY_SBC_MS;
            break;

        case MP3_SEID:
            latency_ms = AUDIO_LATENCY_MP3_MS;
            break;

        default:
            latency_ms = AUDIO_LATENCY_NORMAL_MS;
            break;
        }
    }

    AUD_DEBUG(("AUD: seid=%u latency=%u\n", seid, latency_ms));
    return latency_ms;
}
/****************************************************************************
NAME
    getScoPriorityFromHfpPriority

DESCRIPTION
    obtain the current sco priority level of the AG priority passed in

RETURNS
    current sco priority level, if any, may not have a sco
*/
audio_priority getScoPriorityFromHfpPriority(hfp_link_priority priority)
{
    AUD_DEBUG(("AUD: GetScoPriority - %d=%d\n",priority,sinkHfpDataGetScoPriority(PROFILE_INDEX(OTHER_PROFILE(priority))))) ;
    return sinkHfpDataGetScoPriority(PROFILE_INDEX(OTHER_PROFILE(priority)));
}

/****************************************************************************
NAME
    setScoPriorityFromHfpPriority

DESCRIPTION
    sets the current sco priority level of the AG priority passed in

RETURNS
    nothing
*/
void setScoPriorityFromHfpPriority(hfp_link_priority priority, audio_priority level)
{
    AUD_DEBUG(("AUD: SetScoPriority - %d=%d\n",priority,level)) ;
    if(priority != hfp_invalid_link)
      sinkHfpDataSetScoPriority(level,PROFILE_INDEX(OTHER_PROFILE(priority)));
}


/****************************************************************************
NAME
    audioGetLinkPriority

DESCRIPTION
    Common method of getting the link we want to manipulate audio settings on

RETURNS

*/
hfp_link_priority audioGetLinkPriority ( bool audio )
{
    hfp_link_priority priority;

    /* See if we can get a link from the device audio sink... */
    priority = HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink());

    /* if the call is in outgoing call state */
    if(!priority)
        priority = HfpLinkPriorityFromCallState(hfp_call_state_outgoing);

    /* If that fails see if we have an active call... */
    if(priority == hfp_invalid_link)
        priority = HfpLinkPriorityWithActiveCall(audio);

    /* if there is an incoming call [get first incoming] */
    if(priority == hfp_invalid_link)
        priority = HfpGetFirstIncomingCallPriority();

    /* If we got something return it, otherwise return primary link */
    return ((priority != hfp_invalid_link) ? priority : hfp_primary_link);
}

/****************************************************************************
NAME
    audioSwapMediaChannel

DESCRIPTION
    attempt to swap between media channels if two channels exist and one of them
    is currentyl being routed to the speaker.
RETURNS
    successful or not status
*/
bool audioSwapMediaChannel(void)
{
    a2dp_stream_state a2dpStatePri = a2dp_stream_idle;
    a2dp_stream_state a2dpStateSec = a2dp_stream_idle;
    Sink a2dpSinkPri = 0;
    Sink a2dpSinkSec = 0;

    /* if a2dp connected obtain the current streaming state for primary a2dp connection */
    getA2dpStreamData(a2dp_primary, &a2dpSinkPri, &a2dpStatePri);

    /* if a2dp connected obtain the current streaming state for secondary a2dp connection */
    getA2dpStreamData(a2dp_secondary, &a2dpSinkSec, &a2dpStateSec);

    /* check whether a2dp pri is currently being routed and a2dp sec is available */
    if((sinkAudioGetRoutedAudioSink() == a2dpSinkPri)&&(a2dpSinkSec)&&(a2dpStateSec == a2dp_stream_streaming))
    {
        /* swap to a2dp secondary */
        audioDisconnectRoutedAudio();
        audioRouteSpecificA2dpSource(audio_source_a2dp_2);
        return TRUE;
    }
    /* check whether a2dp sec is currently being routed and swap to a2dp pri */
    else if((sinkAudioGetRoutedAudioSink() == a2dpSinkSec)&&(a2dpSinkPri)&&(a2dpStatePri == a2dp_stream_streaming))
    {
        audioDisconnectRoutedAudio();
        audioRouteSpecificA2dpSource(audio_source_a2dp_1);
        return TRUE;
    }
    /* not possible to swap media channels */
    return FALSE;
}

/****************************************************************************
NAME
    audioShutDownForUpgrade

DESCRIPTION
    Disconnects any routed voice/audio sources and once the audio busy 
    pointer is cleared, messages sink_upgrade to proceed

RETURNS
    void
*/
void audioShutDownForUpgrade(Task task, MessageId id)
{
    audioDisconnectRoutedVoice();
    audioSuspendDisconnectAudioSource();

    MessageSendConditionallyOnTask(task, id, NULL, AudioBusyPtr());
}

/****************************************************************************
NAME
    audioGetSettings

DESCRIPTION
    Get the settings to use for a given incoming/outgoing SCO connection on
    a given AG.

RETURNS
    pointer to hfp_audio_params to use, packets is set to packet types to use.
*/
static const hfp_audio_params* audioGetSettings(hfp_link_priority priority, sync_pkt_type* packets)
{
    Sink sink;

    /* Get the priority of the other link */
    hfp_link_priority other = (priority == hfp_primary_link) ? hfp_secondary_link : hfp_primary_link;

    /* If this is the second sco request limit its choice of packet type to EV3 S1 or HV3 settings only
       this is to prevent to the reserved slot violation problems seen on some phones when trying to use
       2 x 2EV3 links. However if the link mode is secure, we cannot fallback to EV3 S1 settings*/
    if( (sinkDataIsMultipointEnabled()) && (!HfpIsLinkSecure(priority)) && (sinkHfpDataIsForceEV3S1ForSco2()) 
        && (HfpLinkGetAudioSink(other, &sink)) )
    {
        /* request EV3 S1 packet type instead */
        *packets = (sync_hv3 | sync_ev3 | sync_all_edr_esco );
        /* set up for safe EV3 S1 parameters or hv3 should esco not be available */
        return &multipoint_params;
    }

    /* Use configured packet types */
     *packets = sinkHfpDataGetFeaturesSyncType();

       /* If additional params specified use them */
     if(sinkHfpDataGetFeaturesAdditionalAudioParams())
     {
          return sinkHfpDataGetFeaturesTypeAudioParams();
     }
    /* Use HFP library defaults */
    return NULL;
}


/****************************************************************************
NAME
    audioHandleSyncConnectInd

DESCRIPTION
    Handle HFP_AUDIO_CONNECT_IND.  This indicates that an incoming sychronous
    connection is being requested

RETURNS

*/
void audioHandleSyncConnectInd ( const HFP_AUDIO_CONNECT_IND_T *pInd )
{
    sync_pkt_type     packet_types;
    const hfp_audio_params* audio_params;
    bool              disable_wbs_override = FALSE;

    AUD_DEBUG(("AUD: Synchronous Connect Ind [%d] from [%x]:\n" , pInd->codec, PROFILE_INDEX(pInd->priority))) ;

    audio_params = audioGetSettings(pInd->priority, &packet_types);

    AUD_DEBUG(("AUD : [%lx][%x][%x][%x]\n" , audio_params->bandwidth ,audio_params->max_latency   ,
                                            audio_params->voice_settings ,audio_params->retx_effort)) ;
  
#ifdef WBS_TEST
    /* WBS test case - use bad WBS params */
    if(sinkHfpDataIsFailAudioNegotiation())
    {
        packet_types = sync_all_sco;
        disable_wbs_override = TRUE;
    }
#endif
    HfpAudioConnectResponse(pInd->priority, TRUE, packet_types, audio_params, disable_wbs_override);
}

static void updateScoAudioRouting(hfp_call_state call_state)
{
    if(call_state == hfp_call_state_idle)
    {
        sinkCancelAndSendLater(EventSysCheckAudioRouting, ALLOWANCE_FOR_TRANSITION_IN_HFP_CALL_STATE);
    }
    else
    {
        audioUpdateAudioRouting();
    }
}

/****************************************************************************
NAME
    audioHandleSyncConnectCfm

DESCRIPTION
    Handle HFP_AUDIO_CONNECT_CFM.  This indicates that an incoming sychronous
    connection has been established

RETURNS

*/
void audioHandleSyncConnectCfm ( const HFP_AUDIO_CONNECT_CFM_T * pCfm )
{
    uint8 index = PROFILE_INDEX(pCfm->priority);
    hfp_call_state CallState;

    /* Get the priority of the other link */
    hfp_link_priority other = (pCfm->priority == hfp_primary_link) ? hfp_secondary_link : hfp_primary_link;


    AUD_DEBUG(("Synchronous Connect Cfm from [%x]:\n", (uint16)pCfm->priority)) ;

    /* if successful */
    if ( pCfm->status == hfp_success)
    {
        Sink sink;

        /*Revert OTAU settings on SCO connect*/
        sinkGaiaUpgradeScoConnectInd();

        SinkVaCancelSession();
		
#ifdef ENABLE_BISTO
        BistoAudioInClose();
#endif
        /* obtain sink for this audio connection */
        if(HfpLinkGetSlcSink(pCfm->priority, &sink))
        {
            /* Send our link policy settings for active SCO role */
            linkPolicyUseHfpSettings(pCfm->priority, sink);
        }

        /* store in individual hfp struct as it may be necessary to disconnect and reconnect
           audio on a per hfp basis for multipoint multiparty calling */
        sinkHfpDataSetAudioTxBandwidth(pCfm->tx_bandwidth, index);
        sinkHfpDataSetAudioSynclinkType(pCfm->link_type, index);
        sinkHfpDataSetAudioCodec(pCfm->codec, index);

        sinkHfpDataSetAudioValid(index, TRUE);

        if(isLinkSecure(pCfm->priority))
        {
           /* Send an event to indicate that a SCO has been opened on Secure Link */
           /* this indicates that an audio connection has been successfully created to the AG*/
           MessageSend ( &theSink.task , EventSysSCOSecureLinkOpen , 0 ) ;
        }
        else
        {
           /* Send an event to indicate that a SCO has been opened */
           /* this indicates that an audio connection has been successfully created to the AG*/
           MessageSend ( &theSink.task , EventSysSCOLinkOpen , 0 ) ;
        }

        /* update the audio priority state
           is this sco a streaming audio sco?, check call state for this ag */
        if(HfpLinkGetCallState(pCfm->priority, &CallState))
        {
            /* determine sco priority based on call status */
            switch(CallState)
            {
                /* no call so this is a steaming audio connection */
                case hfp_call_state_idle:
                    setScoPriorityFromHfpPriority(pCfm->priority, sco_streaming_audio);
                break;

                /* incoming call so this is an inband ring sco */
                case hfp_call_state_incoming:
                case hfp_call_state_incoming_held:
                case hfp_call_state_twc_incoming:
                    setScoPriorityFromHfpPriority(pCfm->priority, sco_inband_ring);
                break;

                /* active call states so this sco has highest priority */
                case hfp_call_state_active:
                case hfp_call_state_twc_outgoing:
                    /* this audio connection may have been the result of the an audio transfer
                       from the AG and there is already an active call on the other AG, check for this
                       and make this new audio connection held leaving the other routed audio connection
                       intact */
                    if(getScoPriorityFromHfpPriority(other)==sco_active_call)
                    {
                        if (CallState == hfp_call_state_active)
                        {
                            setScoPriorityFromHfpPriority(pCfm->priority, sco_active_call);
                            setScoPriorityFromHfpPriority(other, sco_held_call);
                        }
                        else
                        {
                            setScoPriorityFromHfpPriority(pCfm->priority, sco_held_call);
                        }
                    }
                    else
                        setScoPriorityFromHfpPriority(pCfm->priority, sco_active_call);
                break;

                /* an outgoing call sco has highest priority, if there is a call on other AG
                   it needs to be put on hold whilst this outgoing call is made */
                case hfp_call_state_outgoing:
                    /* does other AG have active call on it?, if so hold the audio */
                    if(getScoPriorityFromHfpPriority(other)==sco_active_call)
                        setScoPriorityFromHfpPriority(other, sco_held_call);
                    /* make the outgoing call audio the one that gets routed */
                    setScoPriorityFromHfpPriority(pCfm->priority, sco_active_call);
                break;

                /* this call is held so the sco is put to on hold priority which is lower than
                   active call but higher than streaming */
                case hfp_call_state_held_active:
                case hfp_call_state_held_remaining:
                    if(sinkAudioIsVoiceRouted())
                        setScoPriorityFromHfpPriority(pCfm->priority, sco_held_call);
                break;

                /* non covered states treat as highest priority sco connection */
                default:
                    setScoPriorityFromHfpPriority(pCfm->priority, sco_active_call);
                break;
            }
        }

        updateScoAudioRouting(CallState);

#ifdef ENABLE_FAST_PAIR
        /* On establishing a new SCO send the indication to BLE SM to avoid FP Data */        
        sinkBleIndicateSCOStatusChanged();
#endif
        /*change the active call state if necessary*/
        if ((stateManagerGetState() == deviceActiveCallNoSCO) )
        {
            stateManagerEnterActiveCallState();
        }

#ifdef DEBUG_AUDIO
        switch (pCfm->link_type)
        {
            case (sync_link_unknown):
                AUD_DEBUG(("AUD: Link = ?\n")) ;
            break ;
            case (sync_link_sco) :
                AUD_DEBUG(("AUD: Link = SCO\n")) ;
            break;
            case sync_link_esco:
                AUD_DEBUG(("AUD: Link = eSCO\n")) ;
            break ;
        }
#endif

    }
    else
    {
        AUD_DEBUG(("Synchronous Connect Cfm: FAILED\n")) ;
    }
    AUD_DEBUG(("AUD : Sco->\n")) ;
}


/****************************************************************************
NAME
    audioHandleSyncDisconnectInd

DESCRIPTION
    Handle HFP_AUDIO_DISCONNECT_IND.  This indicates that an incoming sychronous
    connection has been disconnected

RETURNS

*/
void audioHandleSyncDisconnectInd ( const HFP_AUDIO_DISCONNECT_IND_T * pInd )
{
    Sink sink;

    /* Get the priority of the other link */
    hfp_link_priority other = (pInd->priority == hfp_primary_link) ? hfp_secondary_link : hfp_primary_link;
    AUD_DEBUG(("AUD: Synchronous Disconnect Ind [%x]:\n",pInd->priority)) ;

    /* ensure disconnection was succesfull */
    if(pInd->status == hfp_audio_disconnect_success)
    {
        if(isLinkSecure(pInd->priority))
        {
           MessageSend ( &theSink.task , EventSysSCOSecureLinkClose , 0 ) ;
        }
        else
        {
           MessageSend ( &theSink.task , EventSysSCOLinkClose , 0 ) ;
        }

        sinkHfpDataSetAudioValid(PROFILE_INDEX(pInd->priority), FALSE);

        /* update sco priority */
        setScoPriorityFromHfpPriority(pInd->priority, sco_none);

        /* SCO has been disconnected, check for the prescence of another sco in hold state, this occurs
           when the AG has performed an audio transfer, promote the held call to active */
        if(getScoPriorityFromHfpPriority(other) == sco_held_call)
            setScoPriorityFromHfpPriority(other, sco_active_call);

        AUD_DEBUG(("AUD: Synchronous Disconnect Ind [%x]: sco_pri = %d\n",pInd->priority, HfpLinkPriorityFromAudioSink(sinkAudioGetRoutedVoiceSink()) )) ;

        /* deroute the audio */
        audioUpdateAudioRoutingAfterDisconnect();

#ifdef ENABLE_FAST_PAIR
        /* On exiting SCO send the indication to BLE SM to restart FP Data */        
        sinkBleIndicateSCOStatusChanged();
#endif

        /*change the active call state if necessary*/
        if ((stateManagerGetState() == deviceActiveCallSCO))
            stateManagerEnterActiveCallState();

        /* Send our link policy settings for normal role  now SCO is dropped */
        if(HfpLinkGetSlcSink(pInd->priority, &sink))
            linkPolicyUseHfpSettings(pInd->priority, sink);

        /*Apply Upgrade Configurations in case of ongoing Upgrade and SCO disconnect*/
        sinkGaiaUpgradeScoDisconnectInd();
    }
}

static plugin_type_t getQ2QHfpPluginType(uint16 q2q_mode_id)
{
    plugin_type_t type = super_wide_band;
    switch(q2q_mode_id)
    {
        case 4:
            type = ultra_wide_band;
            break;
        case 0:
        case 1:
        case 6:
        case 7:
        default:
            type = super_wide_band;
            break;
    }
    return type;
}

static plugin_type_t getStandardHfpPluginType(hfp_wbs_codec_mask codec)
{
    plugin_type_t type = narrow_band;
    switch(codec)
    {
        case hfp_wbs_codec_mask_cvsd:
            type = narrow_band;
            break;
        case hfp_wbs_codec_mask_msbc:
            type = wide_band;
            break;
        case hfp_wbs_codec_mask_none:
        default:
            Panic();
            break;
    }
    return type;
}

static plugin_type_t getHfpPluginType(hfp_link_priority priority_index)
{
    plugin_type_t type = narrow_band;
    uint16 q2q_mode_id;

    if(HfpGetQceSelectedCodecModeId(priority_index, &q2q_mode_id))
    {
        type = getQ2QHfpPluginType(q2q_mode_id);
    }
    else
    {
        type = getStandardHfpPluginType(sinkHfpDataGetAudioCodec(PROFILE_INDEX(priority_index)));
    }
    return type;
}

bool audioHfpPopulateConnectParameters(Sink sink, audio_connect_parameters *connect_parameters)
{
    uint8 index = PROFILE_INDEX(HfpLinkPriorityFromAudioSink(sink));

    /* ensure a valid codec is negotiated, should be at least cvsd */
    if(sinkHfpDataGetAudioCodec(index))
    {
        AUD_DEBUG(("AUD: plugin [%d] [%d], sink [%p]\n" , sinkHfpDataGetAudioPlugin()
                                                        , sinkHfpDataGetAudioCodec(index)
                                                        , (void *) sinkAudioGetRoutedVoiceSink())) ;

        connect_parameters->audio_plugin = AudioPluginVoiceVariantsGetHfpPlugin(getHfpPluginType(HfpLinkPriorityFromAudioSink(sink)), sinkHfpDataGetAudioPlugin());
        connect_parameters->audio_sink = sink;
        connect_parameters->sink_type = sinkHfpDataGetAudioSynclinkType(index);
        connect_parameters->volume = (int16)sinkVolumeGetVolumeMappingforCVC(sinkHfpDataGetAudioSMVolumeLevel(index));
        connect_parameters->rate = sinkHfpDataGetAudioTxBandwidth(index);
        connect_parameters->features = sinkAudioGetPluginFeatures();
        connect_parameters->mode = AUDIO_MODE_CONNECTED;
        connect_parameters->route = AUDIO_ROUTE_INTERNAL;
        connect_parameters->power = powerManagerGetLBIPM();
        connect_parameters->params = sinkHfpDataGetHfpPluginParams();
        connect_parameters->app_task = NULL;

        AUD_DEBUG(("Audio Connect %p %u\n",
                   (void *) connect_parameters->audio_plugin,
                   connect_parameters->volume));

        return TRUE;

    }

    return FALSE;;
}

void A2dpPostConnectConfiguration(a2dp_index_t Index)
{
    a2dp_codec_settings * codec_settings;
    codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(Index), getA2dpLinkDataStreamId(Index));

    if(codec_settings)
    {
        if(peerIsA2dpAudioConnected(Index))
        {
            sinkAvrcpSetActiveConnectionFromBdaddr(getA2dpLinkBdAddr(Index));
        }
    }
    freePanic(codec_settings);
}

bool A2dpPopulateConnectParameters(a2dp_index_t Index, audio_connect_parameters *connect_parameters)
{
    Sink sink = sinkA2dpGetAudioSink(Index);
    AUD_DEBUG(("AudioA2dpRoute Index %u Sink %p\n", Index, (void *) sink)) ;

    if(sink)
    {
        a2dp_codec_settings * codec_settings;

        AUD_DEBUG(("AudioA2dpRoute Index[%d] DevId[%x]\n", Index , getA2dpLinkDataDeviceId(Index))) ;

        codec_settings = A2dpCodecGetSettings(getA2dpLinkDataDeviceId(Index), getA2dpLinkDataStreamId(Index));

        if(codec_settings)
        {
        	A2dpPluginConnectParams *audioConnectParams = getAudioPluginConnectParams();

            /* initialise the AudioConnect extra parameters required to pass in additional codec information */
            if (codec_settings->codecData.latency_reporting) /* latency reporting retrieved from a2dp library */
            {
                uint16 latency_ms = audioPriorLatency(codec_settings->seid);

                audioConnectParams->latency.last = 0;
                audioConnectParams->latency.target = audioPriorLatency(codec_settings->seid) / LATENCY_TARGET_MULTIPLIER;
                audioConnectParams->latency.change = 10 / LATENCY_CHANGE_MULTIPLIER;
                audioConnectParams->latency.period = 500 / LATENCY_PERIOD_MULTIPLIER;

                setA2dpLinkDataLatency(Index, 10 * latency_ms);
            }
            else
            {
                audioConnectParams->latency.period = 0;
            }

            audioConnectParams->packet_size =
                    codec_settings->codecData.packet_size; /* Packet size retrieved from a2dp library */
            audioConnectParams->content_protection =
                    codec_settings->codecData.content_protection; /* content protection retrieved from a2dp library */
            audioConnectParams->clock_mismatch =
                    getA2dpLinkDataClockMismatchRate(Index); /* clock mismatch rate for this device */
            audioConnectParams->mode_params =
                    getAudioPluginModeParams(); /* EQ mode and Audio enhancements */

#ifdef INCLUDE_A2DP_EXTRA_CODECS

#ifdef INCLUDE_FASTSTREAM
            audioConnectParams->voice_rate = codec_settings->codecData.voice_rate; /* voice rate retrieved from a2dp library */
            audioConnectParams->bitpool = codec_settings->codecData.bitpool; /* bitpool retrieved from a2dp library */
            audioConnectParams->format = codec_settings->codecData.format; /* format retrieved from a2dp library */
#endif

#ifdef INCLUDE_APTX
            audioConnectParams->channel_mode = codec_settings->channel_mode; /* aptX channel mode */
#endif

#ifdef INCLUDE_APTX_ACL_SPRINT
            audioConnectParams->aptx_sprint_params = codec_settings->codecData.aptx_sprint_params; /* aptX LL params */
#endif

            audioConnectParams->aptx_ad_params = codec_settings->codecData.aptx_ad_params; /* Codec parameters specific to aptX Adaptive */
#endif /* INCLUDE_A2DP_EXTRA_CODECS */

            audioConnectParams->peer_is_available = peerCanThisTwsDeviceRelayRequestedAudio();

            connect_parameters->audio_plugin = getA2dpPlugin(codec_settings->seid);
            connect_parameters->audio_sink = sink;
            connect_parameters->sink_type = AUDIO_SINK_AV;
            connect_parameters->volume = TonesGetToneVolumeInDb(audio_output_group_main);
            connect_parameters->rate = codec_settings->rate;
            connect_parameters->features = sinkAudioGetPluginFeatures();
            connect_parameters->mode = AUDIO_MODE_CONNECTED;
            connect_parameters->route = AUDIO_ROUTE_INTERNAL;
            connect_parameters->power = powerManagerGetLBIPM();
            connect_parameters->params = audioConnectParams;
            connect_parameters->app_task = &theSink.task;
            /* When using BA plugin update the BA specific parameters */
            if(useBroadcastPlugin())
            {
                audio_sources a2dp_audio_source = (Index == a2dp_primary) ? audio_source_a2dp_1 : audio_source_a2dp_2;
                connect_parameters->volume = sinkBroadcastAudioGetVolume(a2dp_audio_source);
                audioConnectParams->ba_output_plugin = AudioPluginOutputGetBroadcastPluginTask();
            }
            else
            {
                audioConnectParams->ba_output_plugin = (Task)NULL;
            }
            freePanic(codec_settings);

            return TRUE;
        }
    }
    return FALSE;
}


/****************************************************************************
NAME
    audioControlLowPowerCodecs

DESCRIPTION
    Enables / disables the low power codecs.
    will only use low power codecs is the flag UseLowPowerCodecs is set and
    enable == TRUE

    This fn is called whenever an audioConnect occurs in order to select the
    right codec ouptut stage power .

RETURNS
    none
*/


/****************************************************************************
NAME
    audioHandleMicSwitch

DESCRIPTION
    Handle AT+MICTEST AT command from TestAg.
    This command swaps between the two microphones to test 2nd mic in production.

RETURNS
    returns true if mic switch command issued to plugin
*/
bool audioHandleMicSwitch( void )
{
    AUD_DEBUG(("audioHandleMicSwitch\n"));

    /*only attempt to swap between the microphones if we have a sco connection present*/
    if (sinkAudioIsAudioRouted())
    {
        AudioMicSwitch() ;
        return TRUE;
    }
    else
    {
        AUD_DEBUG(("audioHandleMicSwitch - no sco present\n"));
        return FALSE;
    }
}

bool audioHandleOutputSwitch(void)
{
    AUD_DEBUG(("audioHandleOutputSwitch\n"));

    /*only attempt to swap between the speakers if we have a sco connection present*/
    if (sinkAudioIsAudioRouted())
    {
        AudioOutputSwitch();
        return TRUE;
    }
    else
    {
        AUD_DEBUG(("audioHandleOutputSwitch - no sco present\n"));
        return FALSE;
    }
}

/****************************************************************************
NAME
    sinkTransferToggle

DESCRIPTION
    If the audio is at the device end transfer it back to the AG and
    vice versa.

RETURNS
    void
*/
void sinkTransferToggle(void)
{
    /* Get the link to toggle audio on */
    hfp_link_priority priority = audioGetLinkPriority(FALSE);
    sync_pkt_type packet_type;
    const hfp_audio_params* audio_params;

    audio_params = audioGetSettings(priority, &packet_type);

    AUD_DEBUG(("AUD: Audio Transfer\n")) ;

    /* Perform audio transfer */
    HfpAudioTransferRequest(priority, hfp_audio_transfer, packet_type, audio_params );
}


/****************************************************************************
NAME
    sinkCheckForAudioTransfer

DESCRIPTION
    checks on connection for an audio connction and performs a transfer if not present

RETURNS
    void
*/
#define OTHER_HFP_LINK(priority) ((priority == hfp_primary_link) ? (hfp_secondary_link) : (hfp_primary_link))
#define IS_CALL_ACTIVE_STATE(cas) ((cas == hfp_call_state_active) || (cas == hfp_call_state_twc_incoming) \
                                    || (cas == hfp_call_state_twc_outgoing) || (cas == hfp_call_state_held_active) \
                                        || (cas == hfp_call_state_multiparty))
void sinkCheckForAudioTransfer ( hfp_link_priority link )
{
    sinkState lState = stateManagerGetState() ;

    AUD_DEBUG(("AUD: Tx[%d] [%p]\n", lState , (void *) sinkAudioGetRoutedAudioSink())) ;

    switch (lState)
    {
        case deviceIncomingCallEstablish :
        case deviceThreeWayCallWaiting :
        case deviceThreeWayCallOnHold :
        case deviceThreeWayMulticall :
        case deviceIncomingCallOnHold :
        case deviceActiveCallNoSCO :
        {
            Sink sink;
            hfp_call_state state;

            /* check if there is an active call to be transferred on "link"? */
            if((HfpLinkGetCallState(link, &state)) && (IS_CALL_ACTIVE_STATE(state)) && (!HfpLinkGetAudioSink(link, &sink)))
            {
                /* check if there is already an active call with audio on other link, if yes, do not do audioTransfer */
                if(!((HfpLinkGetCallState(OTHER_HFP_LINK(link), &state)) && (IS_CALL_ACTIVE_STATE(state)) && (HfpLinkGetAudioSink(OTHER_HFP_LINK(link), &sink))))
                {
                    hfp_audio_params *audio_params = NULL;

                    AUD_DEBUG(("AUD: Transfering the call"));
                    HfpAudioTransferRequest(link, hfp_audio_to_hfp, sinkHfpDataGetFeaturesSyncType(), audio_params);
                }
            }
        }
        break ;
        default:
        break;
    }
}

void CreateAudioConnection(void)
{
    hfp_audio_params * audio_params = NULL;
    bdaddr hfp_addr;

    if(HfpLinkGetBdaddr(hfp_primary_link, &hfp_addr))
    {
        HfpAudioTransferRequest(hfp_primary_link,
                                hfp_audio_to_hfp ,
                                sinkHfpDataGetFeaturesSyncType(),
                                audio_params );
    }
}

/****************************************************************************
NAME
    sinkAudioSetEnhancement
DESCRIPTION
    updates the current audio enhancement settings and updates the dsp if it
    is currently running and streaming a2dp music.

RETURNS
    void
*/
void sinkAudioSetEnhancement(uint16 enhancement, bool enable)
{
    uint16 bypassed_enhancements;
    Sink routedAudio = sinkAudioGetRoutedAudioSink();

    /* user has changed an enhancement default, set flag to indicate user values should be used and not
       dsp default values */
    setMusicProcessingBypassFlags(getMusicProcessingBypassFlags() | MUSIC_CONFIG_DATA_VALID);

    bypassed_enhancements = getMusicProcessingBypassFlags();

    if(enable)
        bypassed_enhancements &= ~enhancement;
    else
        bypassed_enhancements |= enhancement;

    if(getMusicProcessingBypassFlags() != bypassed_enhancements)
    {
        setMusicProcessingBypassFlags(bypassed_enhancements);
    }

    /* now decide whether the music manager is currently running and an update of
       configuration is required */

    /* Determine which a2dp source this is for */
    if(((sinkA2dpGetStreamState(a2dp_primary) == a2dp_stream_streaming) && (a2dpAudioSinkMatch(a2dp_primary, routedAudio)))||
       ((sinkA2dpGetStreamState(a2dp_secondary) == a2dp_stream_streaming) && (a2dpAudioSinkMatch(a2dp_secondary, routedAudio)))
       ||((usbAudioSinkMatch(routedAudio) && sinkUsbIsUsbPluginTypeStereo()))
       ||(analogAudioSinkMatch(routedAudio))||
            (spdifAudioSinkMatch(routedAudio) ||
            (i2sAudioSinkMatch(routedAudio)))
      )
    {
        AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());
        AudioSetMusicProcessingEnhancements (getMusicProcessingMode(), getMusicProcessingBypassFlags());
    }
}

/****************************************************************************
NAME    
    disconnectScoIfConnected

DESCRIPTION
    Disconnects the Sco link if its no more required.

RETURNS
    void
*/
void disconnectScoIfConnected(bdaddr bd_addr)
{
    hfp_link_priority hfp_priority = hfp_invalid_link;

    hfp_priority = HfpLinkPriorityFromBdaddr(&bd_addr);

    /*Apply Upgrade Configurations in case of ongoing Upgrade and SCO disconnect*/
    sinkGaiaUpgradeScoDisconnectInd();

    if((hfp_priority) && (getScoPriorityFromHfpPriority(hfp_priority)))
    {
        HfpAudioDisconnectRequest(hfp_priority);
    }
}

/*************************************************************************
NAME
    toggleEnhancement

DESCRIPTION
    The parameter passed is a bit mask, with a single bit set indicating the bit within musicModeEnhancement
    to be toggled.

RETURNS
    The toggled value of the enhancement.

*/
static bool toggleEnhancement(uint16 enhancement)
{
    return ((getMusicProcessingBypassFlags() & enhancement) == enhancement);
}

/****************************************************************************
NAME
    sinkAudioProcessEvent

DESCRIPTION
    Event handler for usr/sys events related to audio.

RETURNS
    TRUE if the event was handled, otherwise FALSE
*/
bool sinkAudioProcessEvent(MessageId id)
{
    if(peerProcessEvent(id))
    {
        return FALSE;
    }
    else
    {
        switch(id)
        {
            case EventUsrBassEnhanceEnableDisableToggle:
            {
                sinkAudioSetEnhancement(MUSIC_CONFIG_BASS_ENHANCE_BYPASS, toggleEnhancement(MUSIC_CONFIG_BASS_ENHANCE_BYPASS));
            }
            break;

            case EventUsrBassEnhanceOn:
                sinkAudioSetEnhancement(MUSIC_CONFIG_BASS_ENHANCE_BYPASS,TRUE);
                break;

            case EventUsrBassEnhanceOff:
                sinkAudioSetEnhancement(MUSIC_CONFIG_BASS_ENHANCE_BYPASS,FALSE);
                break;

            case EventUsr3DEnhancementEnableDisableToggle:
                sinkAudioSetEnhancement(MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS, toggleEnhancement(MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS));
                break;

            case EventUsr3DEnhancementOn:
                sinkAudioSetEnhancement(MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS,TRUE);
                break;

            case EventUsr3DEnhancementOff:
                sinkAudioSetEnhancement(MUSIC_CONFIG_SPATIAL_ENHANCE_BYPASS,FALSE);
                break;

            case EventUsrCompanderOn:
                sinkAudioSetEnhancement(MUSIC_CONFIG_COMPANDER_BYPASS,TRUE);
                break;

            case EventUsrCompanderOff:
                sinkAudioSetEnhancement(MUSIC_CONFIG_COMPANDER_BYPASS,FALSE);
                break;

            case EventUsrCompanderOnOffToggle:
                sinkAudioSetEnhancement(MUSIC_CONFIG_COMPANDER_BYPASS, toggleEnhancement(MUSIC_CONFIG_COMPANDER_BYPASS));
                break;

            case EventUsrUserEqOnOffToggle:
                sinkAudioSetEnhancement(MUSIC_CONFIG_USER_EQ_BYPASS, toggleEnhancement(MUSIC_CONFIG_USER_EQ_BYPASS));
                /* generate event to drive EQ indicator */
                sinkAudioEQOperatingStateIndication();
            break;

            case EventUsrUserEqOn:
                sinkAudioSetEnhancement(MUSIC_CONFIG_USER_EQ_BYPASS, TRUE);
                /* generate event to drive EQ indicator */
                sinkAudioEQOperatingStateIndication();
                break;

            case EventUsrUserEqOff:
                sinkAudioSetEnhancement(MUSIC_CONFIG_USER_EQ_BYPASS, FALSE);
                /* generate event to drive EQ indicator */
                sinkAudioEQOperatingStateIndication();
                break;
            case EventUsrSelectNextUserEQBank:
                {
                    /* If A2DP in use and muted set mute */
                    AUDIO_MODE_T mode = AUDIO_MODE_CONNECTED;
                    if(id == EventUsrSelectNextUserEQBank && VolumeCheckA2dpMute())
                    {
                        mode = AUDIO_MODE_MUTE_SPEAKER;
                    }
                    /* If USB in use set current USB mode */
                    usbAudioGetMode(&mode);
                    /* cycle through EQ modes */
                    setMusicProcessingMode(A2DP_MUSIC_PROCESSING_FULL_NEXT_EQ_BANK);
                    AUD_DEBUG(("HS : EventUsrSelectNextUserEQBank %x\n", getMusicProcessingMode() ));

                    AudioSetMode(mode, getAudioPluginModeParams());
                    AudioSetMusicProcessingEnhancements(getMusicProcessingMode(), getMusicProcessingBypassFlags());
                }
                break;

            case EventSysUpdateUserEq:
            case EventSysUpdateAudioEnhancements:
            case EventSysPeerGeneratedUpdateUserEq:
            case EventSysPeerGeneratedUpdateAudioEnhancements:
                AudioSetMode(AUDIO_MODE_CONNECTED, getAudioPluginModeParams());
                AudioSetMusicProcessingEnhancements(getMusicProcessingMode(), getMusicProcessingBypassFlags());
                break;
            default:
                break;
        }
    }
    return TRUE;
}


/****************************************************************************
NAME
    sinkAudioEQOperatingStateIndication

DESCRIPTION
    generates an event that can be used to drive an LED to indicate whether the EQ
    funcionality is enabled or not

RETURNS
    void
*/
void sinkAudioEQOperatingStateIndication(void)
{
    /* ensure device not in limbo state */
    if(stateManagerGetState() != deviceLimbo)
    {
        if(getAudioPluginModeParams()->music_mode_enhancements & MUSIC_CONFIG_USER_EQ_BYPASS)
        {
            /* EQ is currently disabled */
            MessageSend(&theSink.task, EventSysEQOff, 0); 
        }
        else
        {   
            /* EQ is currently enabled */
            MessageSend(&theSink.task, EventSysEQOn, 0);
        }
    }
    /* device in limbo operating state so turn off EQ indicator */
    else
    {
        MessageSend(&theSink.task, EventSysEQOff, 0);
    }
}


