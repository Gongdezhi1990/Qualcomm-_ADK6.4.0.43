/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_hfp.c
\brief      HFP State Machine
*/


#include <panic.h>
#include <ps.h>

#ifdef INCLUDE_HFP

#include "av_headset.h"
#include "av_headset_hfp.h"
#include "av_headset_ui.h"
#include "av_headset_link_policy.h"
#include "av_headset_log.h"

/*! Macro for creating a message based on the message name */
#define MAKE_HFP_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);
/*! Macro for creating a variable length message based on the message name */
#define MAKE_HFP_MESSAGE_WITH_LEN(TYPE, LEN) \
    TYPE##_T *message = (TYPE##_T *)PanicUnlessMalloc(sizeof(TYPE##_T) + (LEN-1));

#ifndef HFP_SPEAKER_GAIN
#define HFP_SPEAKER_GAIN    (10)
#endif
#ifndef HFP_MICROPHONE_GAIN
#define HFP_MICROPHONE_GAIN (15)
#endif

#define PSKEY_LOCAL_SUPPORTED_FEATURES (0x00EF)
#define PSKEY_LOCAL_SUPPORTED_FEATURES_SIZE (4)
#define PSKEY_LOCAL_SUPPORTED_FEATURES_DEFAULTS { 0xFEEF, 0xFE8F, 0xFFDB, 0x875B }

static const hfp_audio_params tws_plus_sync_config_params =
{
    8000,                            /* tx_bandwidth   */
    0x0e,                            /* max_latency    */
    sync_air_coding_cvsd,            /* voice_settings */
    sync_retx_link_quality,          /* retx_effort    */
};

/*! \brief Call status state lookup table

    This table is used to convert the call setup and call indicators into 
    the appropriate HFP state machine state
*/
const hfpState hfp_call_state_table[10] =
{
    /* hfp_call_state_idle              */ HFP_STATE_CONNECTED_IDLE,
    /* hfp_call_state_incoming          */ HFP_STATE_CONNECTED_INCOMING,
    /* hfp_call_state_incoming_held     */ HFP_STATE_CONNECTED_INCOMING,
    /* hfp_call_state_outgoing          */ HFP_STATE_CONNECTED_OUTGOING,
    /* hfp_call_state_active            */ HFP_STATE_CONNECTED_ACTIVE,
    /* hfp_call_state_twc_incoming      */ HFP_STATE_CONNECTED_IDLE, /* Not supported */
    /* hfp_call_state_twc_outgoing      */ HFP_STATE_CONNECTED_IDLE, /* Not supported */
    /* hfp_call_state_held_active       */ HFP_STATE_CONNECTED_IDLE, /* Not supported */
    /* hfp_call_state_held_remaining    */ HFP_STATE_CONNECTED_IDLE, /* Not supported */
    /* hfp_call_state_multiparty        */ HFP_STATE_CONNECTED_IDLE, /* Not supported */
};

/* Local Function Prototypes */
static void appHfpHandleInternalConfigWriteRequest(void);
static void appHfpConfigStore(void);
static void appHfpHandleMessage(Task task, MessageId id, Message message);

/*! \brief Set default attributes

    This function populates the supplied attributes with 
    default settings.

    It should be called when a new AG connects and there are no 
    prevously stored attributes for the AG.

    \param  attributes  Pointer to the attributes to be set with default
*/
void appHfpSetDefaultAttributes(appDeviceAttributes *attributes)
{
    PanicNull(attributes);

    attributes->supported_profiles &= ~DEVICE_PROFILE_HFP;
    attributes->hfp_profile = hfp_handsfree_107_profile;
}

/*! \brief Entering `Initialising HFP` state

    This function is called when the HFP state machine enters
    the 'Initialising HFP' state, it calls the HfpInit() function
    to initialise the profile library for HFP.
*/    
static void appHfpEnterInitialisingHfp(void)
{
    hfp_init_params hfp_params = {0};
    uint16 supp_features = (HFP_VOICE_RECOGNITION |
                            HFP_NREC_FUNCTION |
                            HFP_REMOTE_VOL_CONTROL |
                            HFP_CODEC_NEGOTIATION |
                            HFP_HF_INDICATORS |
                            HFP_ESCO_S4_SUPPORTED);

    /* Initialise an HFP profile instance */
    hfp_params.supported_profile = hfp_handsfree_107_profile;
    hfp_params.supported_features = supp_features;
    hfp_params.disable_nrec = TRUE;
    hfp_params.extended_errors = FALSE;
    hfp_params.optional_indicators.service = hfp_indicator_off;
    hfp_params.optional_indicators.signal_strength = hfp_indicator_off;
    hfp_params.optional_indicators.roaming_status = hfp_indicator_off;
    hfp_params.optional_indicators.battery_charge = hfp_indicator_off;
    hfp_params.multipoint = FALSE;
    hfp_params.supported_wbs_codecs = hfp_wbs_codec_mask_cvsd | hfp_wbs_codec_mask_msbc;
    hfp_params.link_loss_time = 1;
    hfp_params.link_loss_interval = 5;
    if (appConfigHfpBatteryIndicatorEnabled())
        hfp_params.hf_indicators = hfp_battery_level_mask;
    else
        hfp_params.hf_indicators = hfp_indicator_mask_none;    

#ifdef INCLUDE_SWB    
    if (appConfigScoSwbEnabled())
        hfp_params.hf_codec_modes = CODEC_64_2_EV3;
    else                                        
        hfp_params.hf_codec_modes = 0;
#endif
    
    HfpInit(appGetHfpTask(), &hfp_params, NULL);
}

/*! \brief Enter 'connecting local' state

    The HFP state machine has entered 'connecting local' state.  Set the
    'connect busy' flag and operation lock to serialise connect attempts,
    reset the page timeout to the default and attempt to connect SLC.
    Make sure AV streaming is suspended.
*/    
static void appHfpEnterConnectingLocal(void)
{
    DEBUG_LOG("appHfpEnterConnectingLocal");

    /* Set operation lock */
    appHfpSetLock(TRUE);

    if (!appGetHfp()->flags & HFP_CONNECT_NO_UI)
        appUiPagingStart();
    
    /* Clear detach pending flag */
    appGetHfp()->detach_pending = FALSE;
        
    /* Start HFP connection */
    if (appGetHfp()->profile == hfp_handsfree_107_profile)
    {
        DEBUG_LOGF("Connecting HFP to AG (%x,%x,%lx)", appGetHfp()->ag_bd_addr.nap, appGetHfp()->ag_bd_addr.uap, appGetHfp()->ag_bd_addr.lap);

        /* Issue connect request for HFP */
        HfpSlcConnectRequest(&appGetHfp()->ag_bd_addr, hfp_handsfree_and_headset, hfp_handsfree_all);
    }
    else
        Panic();
}

/*! \brief Exit 'connecting local' state

    The HFP state machine has exited 'connecting local' state, the connection
    attempt was successful or it failed.  Clear the 'connect busy' flag and
    operation lock to allow pending connection attempts and any pending
    operations on this instance to proceed.  AV streaming can resume now.   
*/    
static void appHfpExitConnectingLocal(void)
{
    DEBUG_LOG("appHfpExitConnectingLocal");
    
    /* Clear operation lock */
    appHfpSetLock(FALSE);

    if (!appGetHfp()->flags & HFP_CONNECT_NO_UI)
        appUiPagingStop();

    /* We have finished (successfully or not) attempting to connect, so
     * we can relinquish our lock on the ACL.  Bluestack will then close
     * the ACL when there are no more L2CAP connections */
    appConManagerReleaseAcl(&appGetHfp()->ag_bd_addr);
}

/*! \brief Enter 'connecting remote' state

    The HFP state machine has entered 'connecting remote' state, this is due
    to receiving a incoming SLC indication. Set operation lock to block any
    pending operations.
*/    
static void appHfpEnterConnectingRemote(void)
{
    DEBUG_LOG("appHfpEnterConnectingRemote");
    
    /* Set operation lock */
    appHfpSetLock(TRUE);

    /* Clear detach pending flag */
    appGetHfp()->detach_pending = FALSE;       
}

/*! \brief Exit 'connecting remote' state

    The HFP state machine has exited 'connecting remote' state.  Clear the
    operation lock to allow pending operations on this instance to proceed.
*/    
static void appHfpExitConnectingRemote(void)
{
    DEBUG_LOG("appHfpExitConnectingRemote");
    
    /* Clear operation lock */
    appHfpSetLock(FALSE);
}

/*! \brief Enter 'connected' state

    The HFP state machine has entered 'connected' state, this means that
    there is a SLC active.  At this point we need to retreive the remote device's
    support features to determine which (e)SCO packets it supports.  Also if there's an
    incoming or active call then answer/transfer the call to headset.
*/    
static void appHfpEnterConnected(void)
{
    DEBUG_LOG("appHfpEnterConnected");

    /* Update most recent connected device */
    appDeviceUpdateMruDevice(&appGetHfp()->ag_bd_addr);

    /* Mark this device as supporting HFP */
    appDeviceSetHfpIsSupported(&appGetHfp()->ag_bd_addr, appGetHfp()->profile);

    /* Read the remote supported features of the AG */
    ConnectionReadRemoteSuppFeatures(appGetHfpTask(), appGetHfp()->slc_sink);
        
    /* Check if connected as HFP 1.5 */
    if (appGetHfp()->profile == hfp_handsfree_107_profile)
    {
        /* Inform AG of the current gain settings */
        /* hfp_primary_link indicates the link that was connected first */
        /* TODO : Handle multipoint support */
        HfpVolumeSyncSpeakerGainRequest((hfp_link_priority)hfp_primary_link, &appGetHfp()->volume);
        HfpVolumeSyncMicrophoneGainRequest(hfp_primary_link, &appGetHfp()->mic_volume);
    }

    /* Set link supervision timeout to 5 seconds */
    //ConnectionSetLinkSupervisionTimeout(appGetHfp()->slc_sink, 0x1F80);
    appLinkPolicyUpdateRoleFromSink(appGetHfp()->slc_sink);

    /* Play connected tone */
    appUiHfpConnected(appGetHfp()->flags & HFP_CONNECT_NO_UI);

    /* Clear silent flags */
    appGetHfp()->flags &= ~(HFP_CONNECT_NO_UI | HFP_CONNECT_NO_ERROR_UI | HFP_DISCONNECT_NO_UI);

    /* Tell clients we have connected */
    MAKE_HFP_MESSAGE(APP_HFP_CONNECTED_IND);
    message->bd_addr = appGetHfp()->ag_bd_addr;
    appTaskListMessageSend(appGetHfp()->status_notify_list, APP_HFP_CONNECTED_IND, message);

#if defined(HFP_CONNECT_AUTO_ANSWER) || defined(HFP_CONNECT_AUTO_TRANSFER)
    if (appGetHfp()->profile != hfp_headset_profile)
    {
#if defined(HFP_CONNECT_AUTO_ANSWER)
        /* Check if incoming call */
        if (appHfpGetState() == HFP_STATE_CONNECTED_INCOMING)
        {
            /* Accept the incoming call */
            appHfpCallAccept();
        }
#endif            
#if defined(HFP_CONNECT_AUTO_TRANSFER)
        /* Check if incoming call */
        if (appHfpGetState() == HFP_STATE_CONNECTED_ACTIVE)
        {
            /* Check SCO is not active */
            if (appGetHfp()->sco_sink == 0)
            {
                /* Attempt to transfer audio */
                HfpAudioTransferConnection(appGetHfp()->hfp, hfp_audio_to_hfp, appGetHfp()->sco_supported_packets ^ sync_all_edr_esco, 0);
            }
        }
#endif            
    }
#endif
}

/*! \brief Exit 'connected' state

    The HFP state machine has exited 'connected' state, this means that
    the SLC has closed.  Make sure any SCO link is disconnected.
*/    
static void appHfpExitConnected(void)
{
    DEBUG_LOG("appHfpExitConnected");

    /* Unregister for battery updates */
    appBatteryUnregister(appGetHfpTask());

    /* Check if SCO is still up */
    if (appGetHfp()->sco_sink)
    {
        /* Disconnect SCO */
        /* TODO: Support multipoint */
        HfpAudioDisconnectRequest(hfp_primary_link);
    }
        
    /* Handle any pending config write */
    if (MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_CONFIG_WRITE_REQ) > 0)
    {
        appHfpHandleInternalConfigWriteRequest();
    }
}

/*! \brief Enter 'connected idle' state

    The HFP state machine has entered 'connected idle' state, this means that
    there is a SLC active but no active call in process.  When entering this
    state we clear the ACL lock that was when entering the 'connecting local'
    state, this allows any other pending connections to proceed.  Any suspended
    AV streaming can now resume.
*/    
static void appHfpEnterConnectedIdle(void)
{
    DEBUG_LOG("appHfpEnterConnectedIdle");
        
#ifdef INCLUDE_AV
    /* Resume AV streaming */
    appAvStreamingResume(AV_SUSPEND_REASON_HFP);
#endif    
}
    
/*! \brief Exit 'connected idle' state

    The HFP state machine has exited 'connected idle' state.
*/    
static void appHfpExitConnectedIdle(void)
{
    DEBUG_LOG("appHfpExitConnectedIdle");
}
    
/*! \brief Enter 'connected outgoing' state

    The HFP state machine has entered 'connected outgoing' state, this means
    that we are in the process of making an outgoing call, just make sure
    that we have suspended AV streaming.
*/    
static void appHfpEnterConnectedOutgoing(void)
{
    DEBUG_LOG("appHfpEnterConnectedOutgoing");
    
#ifdef INCLUDE_AV
    /* We should suspend AV streaming */
    appAvStreamingSuspend(AV_SUSPEND_REASON_HFP);
#endif
}

/*! \brief Exit 'connected outgoing' state

    The HFP state machine has exited 'connected outgoing' state.
*/    
static void appHfpExitConnectedOutgoing(void)
{
    DEBUG_LOG("appHfpExitConnectedOutgoing");
}

/*! \brief Enter 'connected incoming' state

    The HFP state machine has entered 'connected incoming' state, this means
    that there's an incoming call in progress.  Update UI to indicate incoming
    call.
*/    
static void appHfpEnterConnectedIncoming(void)
{
    DEBUG_LOG("appHfpEnterConnectedIncoming");

    appTaskListMessageSendId(appGetHfp()->status_notify_list, APP_HFP_SCO_INCOMING_RING_IND);

    /* Start incoming call indication */
    appUiHfpCallIncomingActive();
}

/*! \brief Exit 'connected incoming' state

    The HFP state machine has exited 'connected incoming' state, this means
    that the incoming call has either been accepted or rejected.  Make sure
    any ring tone is cancelled.
*/    
static void appHfpExitConnectedIncoming(void)
{
    DEBUG_LOG("appHfpExitConnectedIncoming");
    
    /* Clear call accepted flag */
    appGetHfp()->call_accepted = FALSE;    
    
    /* TODO: Cancel any ring-tones */
    /* AudioStopTone();*/

    appTaskListMessageSendId(appGetHfp()->status_notify_list, APP_HFP_SCO_INCOMING_ENDED_IND);

    /* Cancel HSP incoming call timeout */
    MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_HSP_INCOMING_TIMEOUT);

    /* Stop incoming call indication */
    appUiHfpCallIncomingInactive();

    appScoFwdRingCancel();
}

/*! \brief Enter 'connected active' state

    The HFP state machine has entered 'connected active' state, this means
    that a call is in progress, just make sure that we have suspended AV
    streaming.  Update UI to indicate active call.
*/    
static void appHfpEnterConnectedActive(void)
{
    DEBUG_LOG("appHfpEnterConnectedActive");
            
#ifdef INCLUDE_AV
    /* We should suspend AV streaming */
    appAvStreamingSuspend(AV_SUSPEND_REASON_HFP);
#endif    

    /* Show SCO status on LEDs */
    appUiHfpCallActive();
}

/*! \brief Exit 'connected active' state

    The HFP state machine has exited 'connected active' state, this means
    that a call has just finished.  Make sure mute is cancelled.
*/    
static void appHfpExitConnectedActive(void)
{
    DEBUG_LOG("appHfpExitConnectedActive");

    /* Clear any mute reminders and clear mute active flag */
    MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_MUTE_IND);
    appGetHfp()->mute_active = FALSE;

    /* Stop SCO status on LEDs */
    appUiHfpCallInactive();
}    

/*! \brief Enter 'disconnecting' state

    The HFP state machine has entered 'disconnecting' state, this means
    that the SLC should be disconnected.  Set the operation lock to block
    any pending operations.
*/    
static void appHfpEnterDisconnecting(void)
{
    DEBUG_LOG("appHfpEnterDisconnecting");
    
    /* Set operation lock */
    appHfpSetLock(TRUE);

    /* Disconnect SLC */
    /* TODO: Support Multipoint */
    HfpSlcDisconnectRequest(hfp_primary_link);
}

/*! \brief Exit 'disconnecting' state

    The HFP state machine has exited 'disconnecting' state, this means
    that the SLC is now disconnected.  Clear the operation lock to allow
    any pending operations.
*/    
static void appHfpExitDisconnecting(void)
{
    DEBUG_LOG("appHfpExitDisconnecting");
    
    /* Clear operation lock */
    appHfpSetLock(FALSE);
}

/*! \brief Enter 'disconnected' state

    The HFP state machine has entered 'disconnected' state, this means
    that there is now active SLC.  Reset all flags, clear the ACL lock to
    allow pending connections to proceed.  Also make sure AV streaming is
    resumed if previously suspended.
*/    
static void appHfpEnterDisconnected(void)
{
    DEBUG_LOG("appHfpEnterDisconnected");

    /* Tell clients we have disconnected */
    MAKE_HFP_MESSAGE(APP_HFP_DISCONNECTED_IND);
    message->bd_addr = appGetHfp()->ag_bd_addr;
    message->reason =  appGetHfp()->disconnect_reason;
    appTaskListMessageSend(appGetHfp()->status_notify_list, APP_HFP_DISCONNECTED_IND, message);

#ifdef INCLUDE_AV
    /* Resume AV streaming if HFP disconnects for any reason */
    appAvStreamingResume(AV_SUSPEND_REASON_HFP);
#endif
    
    /* Clear status flags */
    appGetHfp()->caller_id_active = FALSE;
    appGetHfp()->voice_recognition_active = FALSE;
    appGetHfp()->voice_recognition_request = FALSE;
    appGetHfp()->mute_active = FALSE;
    appGetHfp()->in_band_ring = FALSE;
    appGetHfp()->call_accepted = FALSE;
    
    /* Clear call state indication */
    appGetHfp()->call_state = 0;
}

static void appHfpExitDisconnected(void)
{
    /* Reset disconnect reason */
    appGetHfp()->disconnect_reason = APP_HFP_CONNECT_FAILED;
}

/*! \brief Set HFP state

    Called to change state.  Handles calling the state entry and exit functions.
    Note: The entry and exit functions will be called regardless of whether or not
    the state actually changes value.
*/
static void appHfpSetState(hfpState state)
{
    hfpState old_state;
    
    DEBUG_LOGF("appHfpSetState(%d)", state);

    /* Handle state exit functions */
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
            appHfpExitConnectingLocal();
            break;
        case HFP_STATE_CONNECTING_REMOTE:
            appHfpExitConnectingRemote();
            break;
        case HFP_STATE_CONNECTED_IDLE:
            appHfpExitConnectedIdle();
            if (state < HFP_STATE_CONNECTED_IDLE || state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpExitConnected();
            break;            
        case HFP_STATE_CONNECTED_ACTIVE:
            appHfpExitConnectedActive();
            if (state < HFP_STATE_CONNECTED_IDLE || state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpExitConnected();
            break;
        case HFP_STATE_CONNECTED_INCOMING:
            appHfpExitConnectedIncoming();
            if (state < HFP_STATE_CONNECTED_IDLE || state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpExitConnected();
            break;
        case HFP_STATE_CONNECTED_OUTGOING:
            appHfpExitConnectedOutgoing();
            if (state < HFP_STATE_CONNECTED_IDLE || state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpExitConnected();
            break;
        case HFP_STATE_DISCONNECTING:
            appHfpExitDisconnecting();
            break;
        case HFP_STATE_DISCONNECTED:
            appHfpExitDisconnected();
            break;
        default:
            break;
    }               
    
    /* Set new state, copy old state */
    old_state = appHfpGetState();
    appGetHfp()->state = state;
    
    /* Handle state entry functions */
    switch (state)
    {
        case HFP_STATE_INITIALISING_HFP:
            appHfpEnterInitialisingHfp();
            break;
        case HFP_STATE_CONNECTING_LOCAL:
            appHfpEnterConnectingLocal();
            break;
        case HFP_STATE_CONNECTING_REMOTE:
            appHfpEnterConnectingRemote();
            break;
        case HFP_STATE_CONNECTED_IDLE:
            if (old_state < HFP_STATE_CONNECTED_IDLE || old_state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpEnterConnected();
            appHfpEnterConnectedIdle();
            break;
        case HFP_STATE_CONNECTED_ACTIVE:
            if (old_state < HFP_STATE_CONNECTED_IDLE || old_state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpEnterConnected();
            appHfpEnterConnectedActive();
            break;
        case HFP_STATE_CONNECTED_INCOMING:
            if (old_state < HFP_STATE_CONNECTED_IDLE || old_state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpEnterConnected();
            appHfpEnterConnectedIncoming();
            break;
        case HFP_STATE_CONNECTED_OUTGOING:
            if (old_state < HFP_STATE_CONNECTED_IDLE || old_state > HFP_STATE_CONNECTED_ACTIVE)
                appHfpEnterConnected();
            appHfpEnterConnectedOutgoing();
            break;
        case HFP_STATE_DISCONNECTING:
            appHfpEnterDisconnecting();
            break;
        case HFP_STATE_DISCONNECTED:
            appHfpEnterDisconnected();
            break;
        default:
            break;
    }               
    
    /* Reflect state change in UI */
    appUiHfpState(state);

    /* Update link policy following change in state */
    appLinkPolicyUpdatePowerTable(appHfpGetAgBdAddr());
}

/*! \brief Handle HFP error

    Some error occurred in the HFP state machine, to avoid the state machine
    getting stuck, drop connection and move to 'disconnected' state.
*/	
static void appHfpError(MessageId id, Message message)
{
    UNUSED(message); UNUSED(id);

    DEBUG_LOGF("appHfpError, state=%u, id=%x", appGetHfp()->state, id);

    /* Check if we are connected */
    if (appHfpIsConnected())
    {
        /* Move to 'disconnecting' state */
        appHfpSetState(HFP_STATE_DISCONNECTING); 
    }
}

/*! \brief Check SCO encryption

    This functions is called to check if SCO is encrypted or not.  If there is
    a SCO link active, a call is in progress and the link becomes unencrypted, start
    a indication tone to the user.
*/	
static void appHfpCheckEncryptedSco(void)
{
    DEBUG_LOGF("appHfpCheckEncryptedSco(%d, %x)", appGetHfp()->encrypted, (int)appGetHfp()->sco_sink);
    
    /* Check SCO is active */
    if (appHfpIsScoActive() && appHfpIsCall())
    {
        /* Check if link is encrypted */
        if (appHfpIsEncrypted())
        {
            /* SCO is encrypted */
            MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_SCO_UNENCRYPTED_IND);
        }
        else
        {
            /* SCO is not encrypted, play warning tone */
            appUiHfpScoUnencryptedReminder();
            
            /* TODO: Mute the MIC to prevent eavesdropping */
            
            /* Check again later */
            MessageSendLater(appGetHfpTask(), HFP_INTERNAL_SCO_UNENCRYPTED_IND, 0, D_SEC(APP_UI_SCO_UNENCRYPTED_REMINDER_TIME));
        }
    }
}

/*! \brief Handle HFP Library initialisation confirmation 
*/    
static void appHfpHandleHfpInitConfirm(const HFP_INIT_CFM_T *cfm)
{
    DEBUG_LOG("appHfpHandleHfpInitConfirm");

    switch (appHfpGetState())
    {
        case HFP_STATE_INITIALISING_HFP:
        {
            /* Check HFP initialisation was successful */
            if (cfm->status == hfp_init_success)
            {
                /* Move to disconnected state */
                appHfpSetState(HFP_STATE_DISCONNECTED);

                /* Tell main application task we have initialised */
                MessageSend(appGetAppTask(), APP_HFP_INIT_CFM, 0);
            }
            else
                Panic();
        }
        return;
        
        default:
            appHfpError(HFP_INIT_CFM, cfm);
            return;
    }
}

/*! \brief Handle SLC connect indication
*/    
static void appHfpHandleHfpSlcConnectIndication(const HFP_SLC_CONNECT_IND_T *ind)
{
    DEBUG_LOG("appHfpHandleHfpSlcConnectIndication");

    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Check which profile we have connecting */
            /* TODO: Probably need to accept the RFCOMM connection first */
            if (ind->accepted)
            {
                /* Store address of AG */
                appGetHfp()->ag_bd_addr = ind->addr;
            }
            
            /* TODO: Probably need to accept the RFCOMM connection first */
            appHfpSetState(HFP_STATE_CONNECTING_REMOTE);
        }
        return;
        
        default:
        /* TODO: What should we do here? Go back to DISCONNECTED ?*/
        return;
    }
}

/*! \brief Send SLC status indication to all clients on the list.
 */
static void appHfpSendSlcStatus(bool connected, hfp_link_priority priority, bdaddr* bd_addr)
{
    Task next_client = 0;

    while (appTaskListIterate(appGetHfp()->slc_status_notify_list, &next_client))
    {
        MAKE_HFP_MESSAGE(APP_HFP_SLC_STATUS_IND);
        message->slc_connected = connected;
        message->priority = priority;
        message->bd_addr = *bd_addr;
        MessageSend(next_client, APP_HFP_SLC_STATUS_IND, message);
    }
}

/*! \brief Handle SLC connect confirmation
*/    
static void appHfpHandleHfpSlcConnectConfirm(const HFP_SLC_CONNECT_CFM_T *cfm)
{
    DEBUG_LOGF("appHfpHandleHfpSlcConnectConfirm(%d)", cfm->status);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        {
            /* Check if SLC connection was successful */
            if (cfm->status == hfp_connect_success)
            {
                /* Inform the hfp library if the link is secure */
                if (appDeviceIsSecureConnection(&cfm->bd_addr))
                    HfpLinkSetLinkMode(cfm->priority, TRUE);

                /* Store SLC sink */
                appGetHfp()->slc_sink = cfm->sink;
        
                /* Update profile used */
                appGetHfp()->profile = cfm->profile;

                /* Turn off link-loss management */
                HfpManageLinkLoss(cfm->priority, FALSE);

                /* Ensure the underlying ACL is encrypted*/
                ConnectionSmEncrypt(appGetHfpTask(), appGetHfp()->slc_sink, TRUE);
        
                /* Move to new connected state */
                appHfpSetState(hfp_call_state_table[appGetHfp()->call_state]);
                
                /* inform clients */
                appHfpSendSlcStatus(TRUE, cfm->priority, &cfm->bd_addr);

                return;        
            }

            /* Play error tone */
            appUiHfpError(appGetHfp()->flags & HFP_CONNECT_NO_ERROR_UI);
    
            /* Set disconnect reason */
            appGetHfp()->disconnect_reason = APP_HFP_CONNECT_FAILED;

            /* Move to disconnected state */
            appHfpSetState(HFP_STATE_DISCONNECTED);
        }
        return;
                    
        default:
            appHfpError(HFP_SLC_CONNECT_CFM, cfm);
            return;
    }
}

/*! \brief Handle SLC disconnect indication
*/    
static void appHfpHandleHfpSlcDisconnectIndication(const HFP_SLC_DISCONNECT_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpSlcDisconnectIndication(%d)", ind->status);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            /* Check if SCO is still up */
            if (appGetHfp()->sco_sink)
            {
                /* Disconnect SCO */
                HfpAudioDisconnectRequest(hfp_primary_link);
            }
    
            /* Reconnect on link loss */
            if (ind->status == hfp_disconnect_link_loss && !appGetHfp()->detach_pending)
            {
                 /* Play link loss tone */
                 appUiHfpLinkLoss();

                 /* Set disconnect reason */
                 appGetHfp()->disconnect_reason = APP_HFP_DISCONNECT_LINKLOSS;
            }
            else
            {
                /* Play disconnected tone */
                appUiHfpDisconnected();

                /* Set disconnect reason */
                appGetHfp()->disconnect_reason = APP_HFP_DISCONNECT_NORMAL;
            }

            /* inform clients */
            appHfpSendSlcStatus(FALSE, hfp_primary_link, appHfpGetAgBdAddr());

            /* Move to disconnected state */
            appHfpSetState(HFP_STATE_DISCONNECTED);
        }
        break;

        case HFP_STATE_DISCONNECTING:
        case HFP_STATE_DISCONNECTED:
        {
            /* Play disconnected tone */
            appUiHfpDisconnected();

            /* Set disconnect reason */
            appGetHfp()->disconnect_reason = APP_HFP_DISCONNECT_NORMAL;

            /* Move to disconnected state */
            appHfpSetState(HFP_STATE_DISCONNECTED);
        }
        break;
                        
        default:
            appHfpError(HFP_SLC_DISCONNECT_IND, ind);
            return;
    }
}

/*! \brief Handle SCO Audio connect indication
*/    
static void appHfpHandleHfpAudioConnectIndication(const HFP_AUDIO_CONNECT_IND_T *ind)
{
    DEBUG_LOG("appHfpHandleHfpAudioConnectIndication");
    const hfp_audio_params *hfp_sync_config_params = NULL;

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            /* Check if audio connection is for us */
            /* TODO: Support for multipoint */
            /* we need to accept the connection only if:
             *  - we are in the TWS+ case and the local earbud is in ear
             *  - we are not in the TWS+ case and the local earbud or the peer is in ear
            */

            /* TWS+ or not */
            bool is_tws_plus = appDeviceIsTwsPlusHandset(appHfpGetAgBdAddr());
            /* check if this earbud is in ear */
            bool local_in_ear = appSmIsInEar();
            /* check if the peer earbud is in ear (peer sync must be completed) */
            bool peer_in_ear = appPeerSyncIsComplete() && appPeerSyncIsPeerInEar();

            /* accept or not with the rule described above */
            /* We can't reject SCO for TWS+ if we are out of ear as this can cause issues with some phone apps.
             * The TWS+ the phone will also only send the call audio to the earbuds that are 'inEar' */
            bool accept = (is_tws_plus) || (!is_tws_plus && (local_in_ear || peer_in_ear));

            if(accept)
            {
                if(is_tws_plus && ind->codec == hfp_wbs_codec_mask_cvsd)
                {
                    /* Accept SCO connection */
                    hfp_sync_config_params = &tws_plus_sync_config_params;
                }

                /* Accept SCO connection */
                HfpAudioConnectResponse(ind->priority, TRUE, appGetHfp()->sco_supported_packets ^ sync_all_edr_esco, hfp_sync_config_params, FALSE);

        
#ifdef INCLUDE_AV
                /* Suspend AV streaming */
                appAvStreamingSuspend(AV_SUSPEND_REASON_SCO);
#endif
                return;
            }
        }
        /* Fall through */
        
        default:
        {
            /* Reject SCO connection */
            HfpAudioConnectResponse(ind->priority, FALSE, 0, NULL, FALSE);
        }
        return;
    }
}

/*! \brief Handle SCO Audio connect confirmation
*/    
static void appHfpHandleHfpAudioConnectConfirmation(const HFP_AUDIO_CONNECT_CFM_T *cfm)
{
    DEBUG_LOGF("appHfpHandleHfpAudioConnectConfirmation(%d)", cfm->status);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            /* Check if audio connection was successful */
            if (cfm->status == hfp_audio_connect_success)
            {
                /* Inform client tasks SCO is active */
                appTaskListMessageSendId(appGetHfp()->status_notify_list, APP_HFP_SCO_CONNECTED_IND);

                /* Store sink associated with SCO */
                appGetHfp()->sco_sink = cfm->audio_sink;

                /* Check if SCO is now encrypted (or not) */
                appHfpCheckEncryptedSco();
        
                /* Update link policy now SCO is active */
                appLinkPolicyUpdatePowerTable(appHfpGetAgBdAddr());
        
#ifdef INCLUDE_AV
                /* Check if AG only supports HV1 SCO packets, if so then disconnect A2DP & AVRCP */
                if (appAvHasAConnection() && (appGetHfp()->sco_supported_packets == sync_hv1))
                {
                    /* Disconnect AV */
                    appAvDisconnectHandset();
            
                    /* Set flag for AV re-connect */
                    appGetHfp()->sco_av_reconnect = TRUE;
                }
                else
                {
                    /* Clear flag for AV re-connect */
                    appGetHfp()->sco_av_reconnect = FALSE;
                }
#endif

                appScoFwdHandleHfpAudioConnectConfirmation(cfm);

                /* Check if in HSP mode, use audio connection as indication of active call */
                if (appGetHfp()->profile == hfp_headset_profile)
                {
                    /* Move to active call state */
                    appHfpSetState(HFP_STATE_CONNECTED_ACTIVE);
                }

                /* Play SCO connected tone, only play if state is ConnectedIncoming,
                   ConnectedOutgoing or ConnectedActive and not voice recognition */
                if (appHfpIsCall() && !appHfpIsVoiceRecognitionActive())
                {
                    /* Set flag indicating we need UI tone when SCO disconnects */
                    appGetHfp()->sco_ui_indication = TRUE;
            
                    /* Play UI tone */
                    appUiHfpScoConnected();
                }
                else
                {
                    /* Clear flag indicating we don't need UI tone when SCO disconnects */
                    appGetHfp()->sco_ui_indication = FALSE;
                }
            }
            else
            {
#ifdef INCLUDE_AV
                /* Resume AV streaming */
                appAvStreamingResume(AV_SUSPEND_REASON_SCO);
#endif
            }
        }
        return;
                    
        default:
            appHfpError(HFP_AUDIO_CONNECT_CFM, cfm);
            return;
    }
}

/*! \brief Handle SCO Audio disconnect indication
*/    
static void appHfpHandleHfpAudioDisconnectIndication(const HFP_AUDIO_DISCONNECT_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpAudioDisconnectIndication(%d)", ind->status);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        case HFP_STATE_DISCONNECTED:
        {
            /* Inform client tasks SCO is inactive */
            appTaskListMessageSendId(appGetHfp()->status_notify_list, APP_HFP_SCO_DISCONNECTED_IND);

            /* Check if have SCO link */
            if (appGetHfp()->sco_sink)
            {
#ifdef INCLUDE_AV
                /* Check if we need to reconnect AV */
                if (appGetHfp()->sco_av_reconnect)
                    appAvConnectHandset(FALSE);
#endif
                /* Play SCO disconnected tone if required */
                if (appGetHfp()->sco_ui_indication)
                    appUiHfpScoDisconnected();

                appScoFwdHandleHfpAudioDisconnectIndication(ind);

                /* Check if in HSP mode, if so then end the call */
                if (appGetHfp()->profile == hfp_headset_profile && appHfpIsConnected())
                {
                    /* Move to connected state */
                    appHfpSetState(HFP_STATE_CONNECTED_IDLE);
                }

                /* Clear SCO sink */
                appGetHfp()->sco_sink = 0;
        
                /* Clear any SCO unencrypted reminders */
                appHfpCheckEncryptedSco();

                /* Update link policy now SCO is inactive */
                appLinkPolicyUpdatePowerTable(appHfpGetAgBdAddr());

#ifdef INCLUDE_AV
                /* Resume AV streaming */
                appAvStreamingResume(AV_SUSPEND_REASON_SCO);
#endif
            }
        }
        return;
                    
        default:
            appHfpError(HFP_AUDIO_DISCONNECT_IND, ind);
            return;
    }
}

/* TODO: Support for HFP encryption change ? */

/*! \brief Handle Ring indication
*/    
static void appHfpHandleHfpRingIndication(const HFP_RING_IND_T *ind)
{
    DEBUG_LOG("appHfpHandleHfpRingIndication");

    /* forward the ring if both of the following are true:
     * - the peer is supporting RING FWD
     * - the peer is connected
    */
    bool ring_fwd = appConfigScoForwardingEnabled() && appDeviceIsPeerRingForwardSupported() && appDeviceIsPeerConnected();

    /* TODO: Support multipoint */
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:             
        {
            /* Check if in HSP mode, use rings as indication of incoming call */
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Move to incoming call establishment */
                appHfpSetState(HFP_STATE_CONNECTED_INCOMING);

                /* Start HSP incoming call timeout */
                MessageSendLater(appGetHfpTask(), HFP_INTERNAL_HSP_INCOMING_TIMEOUT, 0, D_SEC(5));
            }

            /* Play ring tone if AG doesn't support in band ringing */
            if (!ind->in_band && !appGetHfp()->call_accepted)
            {                
                if (ring_fwd)
                {
                    DEBUG_LOG("appHfpHandleHfpRingIndication forwarded");
                    /* The SCO FWD module will both do forwarding and playing it locally */
                    appScoFwdRing();
                }
                else
                {
                    /* play the ring locally */
                    DEBUG_LOG("appHfpHandleHfpRingIndication local");
                    appUiHfpRing(appGetHfp()->caller_id_active);
                }
            }
        }
        return;

        case HFP_STATE_CONNECTED_INCOMING:
        {
            /* Check if in HSP mode, use rings as indication of incoming call */
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Reset incoming call timeout */
                MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_HSP_INCOMING_TIMEOUT);
                MessageSendLater(appGetHfpTask(), HFP_INTERNAL_HSP_INCOMING_TIMEOUT, 0, D_SEC(5));
            }
        }        
        /* Fallthrough */            
        
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            /* Play ring tone if AG doesn't support in band ringing */
            if (!ind->in_band && !appGetHfp()->call_accepted)
            {
                if (ring_fwd)
                {
                    DEBUG_LOGF("appHfpHandleHfpRingIndication forwarded");
                    /* if we have to forward the ring let ask this to the SCO FWD module.
                       it will both do forwarding and playing it locally */
                    appScoFwdRing();
                }
                else
                {
                    /* play the ring locally */
                    DEBUG_LOG("appHfpHandleHfpRingIndication local");
                    appUiHfpRing(appGetHfp()->caller_id_active);
                }
            }
        }
        return;
        
        case HFP_STATE_DISCONNECTING:
            return;
            
        default:
            appHfpError(HFP_RING_IND, ind);
            return;
    }
}

/*! \brief Handle service indication
*/    
static void appHfpHandleHfpServiceIndication(const HFP_SERVICE_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpServiceIndication(%d)", ind->service);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            /* TODO: Handle service/no service */
        }
        return;
            
        default:
            appHfpError(HFP_SERVICE_IND, ind);
            return;
    }    
}

/*! \brief Handle call state indication
*/    
static void appHfpHandleHfpCallStateIndication(const HFP_CALL_STATE_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpCallStateIndication(%d)", ind->call_state);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_DISCONNECTING:
        {
            /* Store call setup indication */
            appGetHfp()->call_state = ind->call_state;
        }
        return;
            
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            hfpState state;

            /* Store call setup indication */
            appGetHfp()->call_state = ind->call_state;

            /* Move to new state, depending on call state */
            state = hfp_call_state_table[appGetHfp()->call_state];
            if (appHfpGetState() != state)
                appHfpSetState(state);
        }
        return;
            
        default:
            appHfpError(HFP_CALL_STATE_IND, ind);
            return;
    }
}

/*! \brief Handle voice recognition indication
*/    
static void appHfpHandleHfpVoiceRecognitionIndication(const HFP_VOICE_RECOGNITION_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpVoiceRecognitionIndication(%d)", ind->enable);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            appGetHfp()->voice_recognition_active = ind->enable;
    
#ifdef INCLUDE_AV
            if (appHfpIsVoiceRecognitionActive())
                appAvStreamingSuspend(AV_SUSPEND_REASON_HFP);
            else
                appAvStreamingResume(AV_SUSPEND_REASON_HFP);
#endif
        }
        return;
            
        default:
            appHfpError(HFP_VOICE_RECOGNITION_IND, ind);
            return;
    }
}

/*! \brief Handle voice recognition enable confirmation
*/    
static void appHfpHandleHfpVoiceRecognitionEnableConfirmation(const HFP_VOICE_RECOGNITION_ENABLE_CFM_T *cfm)
{
    DEBUG_LOGF("appHfpHandleHfpVoiceRecognitionEnableConfirmation(%d)", cfm->status);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            if (cfm->status == hfp_success)
                appGetHfp()->voice_recognition_active = appGetHfp()->voice_recognition_request;
            else
                appGetHfp()->voice_recognition_request = appGetHfp()->voice_recognition_active;

#ifdef INCLUDE_AV
            if (appHfpIsVoiceRecognitionActive())
                appAvStreamingSuspend(AV_SUSPEND_REASON_HFP);
            else
                appAvStreamingResume(AV_SUSPEND_REASON_HFP);
#endif
        }
        return;
            
        default:
            appHfpError(HFP_VOICE_RECOGNITION_ENABLE_CFM, cfm);
            return;
    }
}

/*! \brief Handle caller ID indication
*/    
static void appHfpHandleHfpCallerIdIndication(const HFP_CALLER_ID_IND_T *ind)
{
    DEBUG_LOG("appHfpHandleHfpCallerIdIndication");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_INCOMING:
        {
            /* Check we haven't already accepted the call */
            if (!appGetHfp()->call_accepted)                
            {
                /* Queue prompt & number */
                appUiHfpCallerId(ind->caller_number, ind->size_caller_number, NULL, 0);
            }
        }
        return;

        case HFP_STATE_DISCONNECTING:
            return;
                   
        default:
            appHfpError(HFP_CALLER_ID_IND, ind);
            return;
    }
}

/*! \brief Handle caller ID enable confirmation
*/    
static void appHfpHandleHfpCallerIdEnableConfirmation(const HFP_CALLER_ID_ENABLE_CFM_T *cfm)
{
    DEBUG_LOGF("appHfpHandleHfpCallerIdEnableConfirmation(%d)", cfm->status);
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            if (cfm->status == hfp_success)
                appGetHfp()->caller_id_active = TRUE;
        }
        return;
        
        default:
            appHfpError(HFP_CALLER_ID_ENABLE_CFM, cfm);
            return;
    }
}

/*! \brief Notify all registered clients of new HFP volume. */
static void appHfpVolumeNotifyClients(void)
{
    MAKE_HFP_MESSAGE(APP_HFP_VOLUME_IND);
    message->volume = appGetHfp()->volume;
    appTaskListMessageSend(appGetHfp()->status_notify_list, APP_HFP_VOLUME_IND, message);
}

/*! \brief Handle volume indication
*/    
static void appHfpHandleHfpVolumeSyncSpeakerGainIndication(const HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpVolumeSyncSpeakerGainIndication(%d)", ind->volume_gain);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            /* Set output gain */
            appGetHfp()->volume = ind->volume_gain;
   
            /* Update audio plug-in if SCO active */
            if (appGetHfp()->sco_sink)
            {
                appHfpVolumeNotifyClients();
                /* only set volume now, if SCOFWD hasn't registered to handle it */
                if (!appGetHfp()->sco_forward_handling_volume)
                {
                    appKymeraScoSetVolume(appGetHfp()->volume);
                }
            }

            /* Store new configuration */        
            appHfpConfigStore();   
        }
        return;
                    
        default:
            appHfpError(HFP_VOLUME_SYNC_SPEAKER_GAIN_IND, ind);
            return;
    }
}

/*! \brief Handle microphone volume indication
*/    
static void appHfpHandleHfpVolumeSyncMicrophoneGainIndication(const HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpVolumeSyncMicrophoneGainIndication(%d)", ind->mic_gain);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            /* Set input gain */
            appGetHfp()->mic_volume = ind->mic_gain;
    
            /* Store new configuration */        
            appHfpConfigStore();
        }
        return;
            
        default:
            appHfpError(HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND, ind);
            return;
    }
}

/*! \brief Handle answer call confirmation
*/    
static void appHfpHandleHfpCallAnswerConfirmation(const HFP_CALL_ANSWER_CFM_T *cfm)
{
    DEBUG_LOG("appHfpHandleHfpCallAnswerConfirmation");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_INCOMING:
        {
            if (cfm->status == hfp_success)
            {
#ifdef INCLUDE_AV
                /* Tell main task we should suspend AV streaming */
                appAvStreamingSuspend(AV_SUSPEND_REASON_HFP);
#endif  
                /* Flag call as accepted, so we ignore any ring indications or caller ID */
                appGetHfp()->call_accepted = TRUE;
                
                /* TODO: Cancel any ring-tones */
                /* AudioStopTone();*/
            }
        }
        return;                

        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
            return;

        default:
            appHfpError(HFP_CALL_ANSWER_CFM, cfm);
            return;
    }
}

/*! \brief Handle terminate call confirmation
*/    
static void appHfpHandleHfpCallTerminateConfirmation(const HFP_CALL_TERMINATE_CFM_T *cfm)
{
    DEBUG_LOG("appHfpHandleHfpCallTerminateConfirmation");

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
            return;                

        default:
            appHfpError(HFP_CALL_TERMINATE_CFM, cfm);
            return;
    }
}

/*! \brief Handle unrecognised AT commands as TWS+ custom commands.
 */
static void appHfpHandleHfpUnrecognisedAtCmdInd(HFP_UNRECOGNISED_AT_CMD_IND_T* ind)
{
    hfpTaskData* hfp = appGetHfp();

    DEBUG_LOG("appHfpHandleHfpUnrecognisedAtCmdInd");

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            /* copy the message and send to register AT client */
            MAKE_HFP_MESSAGE_WITH_LEN(APP_HFP_AT_CMD_IND, ind->size_data);
            message->priority = ind->priority;
            message->size_data = ind->size_data;
            memcpy(message->data, ind->data, ind->size_data);
            MessageSend(hfp->at_cmd_task, APP_HFP_AT_CMD_IND, message);
        }
            return;                

        default:
            appHfpError(HFP_UNRECOGNISED_AT_CMD_IND, ind);
            return;
    }

}


static void appHfpHandleHfpHfIndicatorsReportInd(const HFP_HF_INDICATORS_REPORT_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpHfIndicatorsReportInd, num %u, mask %04x", ind->num_hf_indicators, ind->hf_indicators_mask);
}

static void appHfpHandleHfpHfIndicatorsInd(const HFP_HF_INDICATORS_IND_T *ind)
{
    DEBUG_LOGF("appHfpHandleHfpHfIndicatorsInd, num %u, status %u", ind->hf_indicator_assigned_num, ind->hf_indicator_status);
    if (ind->hf_indicator_assigned_num == hf_battery_level)
    {
        if (ind->hf_indicator_status)
        {
            hfpTaskData *hfp = appGetHfp();

            hfp->battery_form.task = appGetHfpTask();
            hfp->battery_form.hysteresis = 1;
            hfp->battery_form.representation = battery_level_repres_percent;

            appBatteryRegister(&hfp->battery_form);
            HfpBievIndStatusRequest(hfp_primary_link, hf_battery_level, appBatteryGetPercent());
        }
        else
        {
            appBatteryUnregister(appGetHfpTask());
        }
    }
}


static void appHfpHandleBatteryLevelUpdatePercent(MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT_T *msg)
{
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
            HfpBievIndStatusRequest(hfp_primary_link, hf_battery_level, msg->percent);
            break;

        default:
            break;
    }
}


/*! \brief Send handset signalling AT command to handset. 

    \param  priority    Which HFP link to send the command over
    \param  cmd         The NULL terminated AT command to send
*/
void appHfpSendAtCmdReq(hfp_link_priority priority, char* cmd)
{
    HfpAtCmdRequest(priority, cmd);
}

/*! \brief Handle confirmation result of attempt to send AT command to handset. */
static void appHfpHandleHfpAtCmdConfirm(HFP_AT_CMD_CFM_T *cfm)
{
    hfpTaskData* hfp = appGetHfp();
    DEBUG_LOGF("appHfpHandleHfpAtCmdConfirm %d", cfm->status);
    MAKE_HFP_MESSAGE(APP_HFP_AT_CMD_CFM);
    message->status = cfm->status == hfp_success ? TRUE : FALSE;
    MessageSend(hfp->at_cmd_task, APP_HFP_AT_CMD_CFM, message);
}

/*! \brief Handle remote support features confirmation
*/    
static void appHfpHandleClDmRemoteFeaturesConfirm(const CL_DM_REMOTE_FEATURES_CFM_T *cfm)
{
    DEBUG_LOG("appHfpHandleClDmRemoteFeaturesConfirm");

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        case HFP_STATE_DISCONNECTED:
        {
            if (cfm->status == hci_success)
            {
                uint16 features[PSKEY_LOCAL_SUPPORTED_FEATURES_SIZE] = PSKEY_LOCAL_SUPPORTED_FEATURES_DEFAULTS;
                uint16 packets;
                uint16 index;

                /* Read local supported features to determine SCO packet types */
                PsFullRetrieve(PSKEY_LOCAL_SUPPORTED_FEATURES, &features, PSKEY_LOCAL_SUPPORTED_FEATURES_SIZE);

                /* Get supported features that both HS & AG support */
                for (index = 0; index < PSKEY_LOCAL_SUPPORTED_FEATURES_SIZE; index++)
                {
                    printf("%04x ", features[index]);
                    features[index] &= cfm->features[index];
                }
                printf("");

                /* Calculate SCO packets we should use */
                packets = sync_hv1;
                if (features[0] & 0x2000)
                    packets |= sync_hv3;
                if (features[0] & 0x1000)
                    packets |= sync_hv2;
    
                /* Only use eSCO for HFP 1.5 */
                if (appGetHfp()->profile == hfp_handsfree_107_profile)
                {
                    if (features[1] & 0x8000)
                        packets |= sync_ev3;
                    if (features[2] & 0x0001)
                        packets |= sync_ev4;
                    if (features[2] & 0x0002)
                        packets |= sync_ev5;
                    if (features[2] & 0x2000)
                    {
                        packets |= sync_2ev3;
                        if (features[2] & 0x8000)
                            packets |= sync_2ev5;
                    }
                    if (features[2] & 0x4000)
                    {
                        packets |= sync_3ev3;
                        if (features[2] & 0x8000)
                            packets |= sync_3ev5;
                    }
                }

                /* Update supported SCO packet types */
                appGetHfp()->sco_supported_packets = packets;

                DEBUG_LOGF("appHfpHandleClDmRemoteFeaturesConfirm, SCO packets %x", packets);
            }	
        }
        return;

        default:
            appHfpError(CL_DM_REMOTE_FEATURES_CFM, cfm);
            return;
    }
}

/*! \brief Handle encrypt confirmation
*/    
static void appHfpHandleClDmEncryptConfirmation(const CL_SM_ENCRYPT_CFM_T *cfm)
{
    DEBUG_LOGF("appHfpHandleClDmEncryptConfirmation(%d)", cfm->encrypted);

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_DISCONNECTING:
        {
            /* Store encrypted status */
            appGetHfp()->encrypted = cfm->encrypted;
    
            /* Check if SCO is now encrypted (or not) */
            appHfpCheckEncryptedSco();
        }
        return;
            
        default:
            appHfpError(CL_SM_ENCRYPT_CFM, cfm);
            return;
    }
}

/*! \brief Handle connect HFP SLC request
*/    
static void appHfpHandleInternalHfpConnectRequest(const HFP_INTERNAL_HFP_CONNECT_REQ_T *req)
{
    DEBUG_LOGF("appHfpHandleInternalHfpConnectRequest, %x,%x,%lx",
               req->addr.nap, req->addr.uap, req->addr.lap);

    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Check ACL is connected */
            if (appConManagerIsConnected(&req->addr))
            {
                /* Store connection flags */
                appGetHfp()->flags = req->flags;

                /* Store AG Bluetooth Address and profile type */
                appGetHfp()->ag_bd_addr = req->addr;
                appGetHfp()->profile = req->profile;

                /* Move to connecting local state */
                appHfpSetState(HFP_STATE_CONNECTING_LOCAL);
            }
            else
            {
                DEBUG_LOGF("appHfpHandleInternalHfpConnectRequest, no ACL %x,%x,%lx",
                           req->addr.nap, req->addr.uap, req->addr.lap);

                /* Set disconnect reason */
                appGetHfp()->disconnect_reason = APP_HFP_CONNECT_FAILED;

                /* Move to 'disconnected' state */
                appHfpSetState(HFP_STATE_DISCONNECTED);
            }
        }
        return;
        
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTING_LOCAL:
            return;
                    
        case HFP_STATE_DISCONNECTING:
        {
            MAKE_HFP_MESSAGE(HFP_INTERNAL_HFP_CONNECT_REQ);

            /* repost the connect message pending final disconnection of the profile
             * via the lock */
            message->addr = req->addr;
            message->profile = req->profile;
            message->flags = req->flags;
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_CONNECT_REQ, message,
                                     &appHfpGetLock());
        }
        return;

        default:
            appHfpError(HFP_INTERNAL_HFP_CONNECT_REQ, req);
            return;
    }    
}

/*! \brief Handle disconnect HFP SLC request
*/    
static void appHfpHandleInternalHfpDisconnectRequest(const HFP_INTERNAL_HFP_DISCONNECT_REQ_T *req)
{
    DEBUG_LOG("appHfpHandleInternalHfpDisconnectRequest");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:   
        {
            /* Move to disconnecting state */
            appHfpSetState(HFP_STATE_DISCONNECTING);
        }
        return;
        
        case HFP_STATE_DISCONNECTED:
            return;
        
        default:
            appHfpError(HFP_INTERNAL_HFP_DISCONNECT_REQ, req);
            return;
    }		
}

/*! \brief Handle last number redial request
*/    
static void appHfpHandleInternalHfpLastNumberRedialRequest(void)
{
    DEBUG_LOG("appHfpHandleInternalHfpLastNumberRedialRequest");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        {
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Send button press */
                /* TODO: Support Multilink */
                HfpHsButtonPressRequest(hfp_primary_link);
            }
            else
            {
                /* Request last number redial */
                /* TODO: Support Mulitilink */
                HfpDialLastNumberRequest(hfp_primary_link);
            }
        }
        return;

        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:   
        case HFP_STATE_DISCONNECTED:
            /* Ignore last number redial request as it doesn't make sense in these states */
            return;
            
        default:
            appHfpError(HFP_INTERNAL_HFP_LAST_NUMBER_REDIAL_REQ, NULL);
            return;
    }
}                    
        
/*! \brief Handle voice dial request
*/    
static void appHfpHandleInternalHfpVoiceDialRequest(void)
{
    DEBUG_LOG("appHfpHandleInternalHfpVoiceDialRequest");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        {
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Send button press */
                /* TODO: Support Multilink */
                HfpHsButtonPressRequest(hfp_primary_link);
            }
            else
            {
                /* Send the CMD to the AG */
                /* TODO: Support Multipoint */
                HfpVoiceRecognitionEnableRequest(hfp_primary_link, appGetHfp()->voice_recognition_request = TRUE);
            }
        }
        return;

        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:   
        case HFP_STATE_DISCONNECTED:
            /* Ignore voice dial request as it doesn't make sense in these states */
            return;
            
        default:
            appHfpError(HFP_INTERNAL_HFP_VOICE_DIAL_REQ, NULL);
            return;
    }
}                    

/*! \brief Handle voice dial disable request
*/    
static void appHfpHandleInternalHfpVoiceDialDisableRequest(void)
{
    DEBUG_LOG("appHfpHandleInternalHfpVoiceDialDisableRequest");
       
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_IDLE:
        {
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Send button press */
                /* TODO: Support Multilink */
                HfpHsButtonPressRequest(hfp_primary_link);
            }
            else
            {
                /* Send the CMD to the AG */
                /* TODO: Support Multipoint */
                HfpVoiceRecognitionEnableRequest(hfp_primary_link, appGetHfp()->voice_recognition_request = FALSE);
            }
        }
        return;

        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:   
        case HFP_STATE_DISCONNECTED:
            /* Ignore voice dial request as it doesn't make sense in these states */
            return;
            
        default:
            appHfpError(HFP_INTERNAL_HFP_VOICE_DIAL_DISABLE_REQ, NULL);
            return;
    }
}                    

/*! \brief Handle accept call request
*/    
static void appHfpHandleInternalHfpCallAcceptRequest(void)
{
    DEBUG_LOG("appHfpHandleInternalHfpCallAcceptRequest");
       
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_INCOMING:
        {
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Send button press */
                /* TODO: Support Multilink */
                HfpHsButtonPressRequest(hfp_primary_link);
            }
            else
            {
                /* Answer the incoming call */
                /* TODO: Support Multipoint */
                HfpCallAnswerRequest(hfp_primary_link, TRUE);
            }
        }
        return;
        
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:   
        case HFP_STATE_DISCONNECTED:
            /* Ignore call accept request as it doesn't make sense in these states */
            return;
            
        default:
            appHfpError(HFP_INTERNAL_HFP_CALL_ACCEPT_REQ, NULL);
            return;
    }
}                    

/*! \brief Handle reject call request
*/    
static void appHfpHandleInternalHfpCallRejectRequest(void)
{
    DEBUG_LOG("appHfpHandleInternalHfpCallRejectRequest");
       
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_INCOMING:
        {
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Play error tone, we can't reject call in headset profile */
                appUiHfpError(FALSE);
            }
            else
            {
                /* Reject the incoming call */
                /* TODO: Support Multipoint */
                HfpCallAnswerRequest(hfp_primary_link, FALSE);
            }
        }
        return;
        
        case HFP_STATE_CONNECTED_IDLE:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:   
        case HFP_STATE_DISCONNECTED:
            /* Ignore call accept request as it doesn't make sense in these states */
            return;
            
        default:
            appHfpError(HFP_INTERNAL_HFP_CALL_REJECT_REQ, NULL);
            return;
    }
}                    

/*! \brief Handle hangup call request
*/    
static void appHfpHandleInternalHfpCallHangupRequest(void)
{
    DEBUG_LOG("appHfpHandleInternalHfpCallHangupRequest");
       
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_CONNECTED_OUTGOING:
        {
            if (appGetHfp()->profile == hfp_headset_profile)
            {
                /* Send an HSP button press */
                /* TODO: Support Multipoint */
                HfpHsButtonPressRequest(hfp_primary_link);
            }
            else
            {
                /* Terminate the call */
                /* TODO: Support Multiponit */
                HfpCallTerminateRequest(hfp_primary_link);
            }
        }
        return;
        
        case HFP_STATE_CONNECTED_IDLE:   
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_DISCONNECTED:
            /* Ignore call accept request as it doesn't make sense in these states */
            return;
            
        default:
            appHfpError(HFP_INTERNAL_HFP_CALL_HANGUP_REQ, NULL);
            return;
    }
}                    

/*! \brief Handle mute/unmute request
*/    
static void appHfpHandleInternalHfpMuteRequest(const HFP_INTERNAL_HFP_MUTE_REQ_T *req)
{
    DEBUG_LOG("appHfpHandleInternalHfpMuteRequest");
       
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            /* Cancel mute reminder tone */
            MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_MUTE_IND);
            
            /* If muted then start mute reminder timer */
            if (req->mute)
                MessageSendLater(appGetHfpTask(), HFP_INTERNAL_MUTE_IND, 0, D_SEC(APP_UI_MUTE_REMINDER_TIME));
            
            /* Set mute flag */
            appGetHfp()->mute_active = req->mute;
    
            /* Re-configure audio chain */
            appKymeraScoMicMute(req->mute);
        }
        return;
        
        case HFP_STATE_CONNECTED_IDLE:   
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_DISCONNECTED:
            /* Ignore call accept request as it doesn't make sense in these states */
            return;
        
        default:
            appHfpError(HFP_INTERNAL_HFP_CALL_HANGUP_REQ, NULL);
            return;
    }
}                    

/*! \brief Handle audio transfer request
*/    
static void appHfpHandleInternalHfpTransferRequest(const HFP_INTERNAL_HFP_TRANSFER_REQ_T *req)
{
    DEBUG_LOG("appHfpHandleInternalHfpTransferRequest");

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_ACTIVE:
        case HFP_STATE_CONNECTED_OUTGOING:
        {
            /* Attempt to transfer audio */
            /* TODO: Support Multipoint */
            HfpAudioTransferRequest(hfp_primary_link,
                                    req->transfer_to_ag ? hfp_audio_to_ag : hfp_audio_to_hfp,
                                    appGetHfp()->sco_supported_packets  ^ sync_all_edr_esco,
                                    NULL);
        }
        return;
        
        case HFP_STATE_CONNECTED_IDLE:   
        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_DISCONNECTED:
            /* Ignore call accept request as it doesn't make sense in these states */
            return;
        
        default:
            appHfpError(HFP_INTERNAL_HFP_CALL_HANGUP_REQ, NULL);
            return;
    }
}
                                              
/*! \brief Handle config write request 

    This functions is called to write the current HFP configuration
    to persistent store.
*/
static void appHfpHandleInternalConfigWriteRequest(void)
{
    hfpPsConfigData ps_config;
    
    DEBUG_LOG("appHfpHandleInternalConfigWriteRequest");
    
    /* Initialise PS config structure */
    ps_config.volume = appGetHfp()->volume;
    ps_config.mic_volume = appGetHfp()->mic_volume;

    /* Write to persistent store */
    PsStore(PS_HFP_CONFIG, &ps_config, sizeof(ps_config)); 
}

/*! \brief Handle mute reminder indication 

    Time to remind user that mute is enabled, play tone and reschedule
    another reminder.
*/
static void appHfpHandleInternalMuteReminderIndication(void)
{
    DEBUG_LOG("appHfpHandleInternalMuteReminderIndication");
    
    /* Queue up mute reminder message */
    MessageSendLater(appGetHfpTask(), HFP_INTERNAL_MUTE_IND, 0, D_SEC(APP_UI_MUTE_REMINDER_TIME));
    
    /* Play reminder tone */
    appUiHfpMuteReminder();    
}

/*! \brief Handle HSP incoming call timeout

    We have had a ring indication for a while, so move back to 'connected
    idle' state.
*/
static void appHfpHandleHfpHspIncomingTimeout(void)
{
    DEBUG_LOG("appHfpHandleHfpHspIncomingTimeout");

    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_INCOMING:
        {
            /* Move back to connected idle state */
            appHfpSetState(HFP_STATE_CONNECTED_IDLE);
        }
        return;
        
        default:
            appHfpError(HFP_INTERNAL_HSP_INCOMING_TIMEOUT, NULL);
            return;
    }
}

/*! \brief Handle indication of change in a connection status.
    
    Some phones will disconnect the ACL without closing any L2CAP/RFCOMM
    connections, so we check the ACL close reason code to determine if this
    has happened.

    If the close reason code was not link-loss and we have an HFP profile
    on that link, mark it as detach pending, so that we can gracefully handle
    the L2CAP or RFCOMM disconnection that will follow shortly.
 */
static void appHfpHandleConManagerConnectionInd(CON_MANAGER_CONNECTION_IND_T *ind)
{
    /* if disconnection and not an connection timeout, see if we need to mark
     * the HFP profile at having a pending detach */
    if (!ind->connected && ind->reason != hci_error_conn_timeout)
    {
        if (!appHfpIsDisconnected() && BdaddrIsSame(&ind->bd_addr, &appGetHfp()->ag_bd_addr))
        {
            DEBUG_LOG("appHfpHandleConManagerConnectionInd, detach pending");
            appGetHfp()->detach_pending = TRUE;
        }
    }
}

/*! \brief Initialise HFP instance

    This function initialises the HFP instance, all state variables are
    set to defaults and volume defaults read from persistent store.
*/
void appHfpInit(void)
{
    hfpPsConfigData ps_config;

    /* Set up task handler */
    appGetHfp()->task.handler = appHfpHandleMessage;

    /* Set defaults */
    ps_config.volume = HFP_SPEAKER_GAIN;
    ps_config.mic_volume = HFP_MICROPHONE_GAIN;

    /* Read config from persistent store */
    PsRetrieve(PS_HFP_CONFIG, &ps_config, sizeof(ps_config));
    
    /* Update configuration */
    appGetHfp()->volume = ps_config.volume;
    appGetHfp()->mic_volume = ps_config.mic_volume;

    /* Store default SCO packet types */
    appGetHfp()->sco_supported_packets = sync_all_sco;

    /* create list for SLC notification clients */
    appGetHfp()->slc_status_notify_list = appTaskListInit();

    /* create list for general status notification clients */
    appGetHfp()->status_notify_list = appTaskListInit();

    /* Initialise state */
    appGetHfp()->state = HFP_STATE_NULL;
    appGetHfp()->sco_sink = 0;
    appGetHfp()->hfp_lock = 0;
    appGetHfp()->disconnect_reason = APP_HFP_CONNECT_FAILED;
    appHfpSetState(HFP_STATE_INITIALISING_HFP);

    /* by default this module handles setting volume in kymera */
    appGetHfp()->sco_forward_handling_volume = FALSE;

    /* Register to receive notifications of (dis)connections */
    appConManagerRegisterConnectionsClient(appGetHfpTask());
}

/*! \brief Initiate HFP connection to default

    Attempt to connect to the previously connected HFP AG.

    \return TRUE if a connection was requested. FALSE is returned
        in the case of an error such as HFP not being supported by
        the handset or there already being an HFP connection. The
        error will apply even if the existing HFP connection is
        to the requested handset.
*/
bool appHfpConnectHandset(void)
{
    bdaddr bd_addr;
    appDeviceAttributes attributes;

    /* Get handset device address */
    if (appDeviceGetHandsetAttributes(&bd_addr, &attributes, NULL) && appDeviceIsHfpSupported(&bd_addr))
        return appHfpConnectWithBdAddr(&bd_addr, attributes.hfp_profile);

    return FALSE;
}
    
/*! \brief Initiate HFP connection to device

    Attempt to connect to the specified HFP AG.

    \param  bd_addr Bluetooth address of the HFP AG to connect to
    \param  profile The version of hfp profile to use when connectinhg

    \return TRUE if a connection was requested. FALSE is returned
        if there is already an HFP connection. The error will apply 
        even if the existing HFP connection is to the requested handset.

*/
bool appHfpConnectWithBdAddr(const bdaddr *bd_addr, hfp_profile profile)
{
    DEBUG_LOG("appHfpConnectWithBdAddr");

    /* Check if not already connected */
    if (!appHfpIsConnected())
    {
        MAKE_HFP_MESSAGE(HFP_INTERNAL_HFP_CONNECT_REQ);

        /* Send message to HFP task */
        message->addr = *bd_addr;
        message->profile = profile;
        message->flags = 0;
        MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_CONNECT_REQ, message,
                                 appConManagerCreateAcl(bd_addr));
            
        /* Connect will now be handled by HFP task */
        return TRUE;
    }
    
    /* Error occured */
    return FALSE;
}

/*! \brief Initiate HFP disconnect
*/	
void appHfpDisconnectInternal(void)
{
    DEBUG_LOG("appHfpDisconnect");
    if (!appHfpIsDisconnected())
    {
        MAKE_HFP_MESSAGE(HFP_INTERNAL_HFP_DISCONNECT_REQ);

        /* Send message to HFP task */
        message->silent = FALSE;
        MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_DISCONNECT_REQ,
                                 message, &appHfpGetLock());		
    }
}

/*! \brief Attempt last number redial

    Initiate last number redial, attempt to connect SLC first if not currently
    connected.
*/
void appHfpCallLastDialed(void)
{
    DEBUG_LOG("appHfpCallLastDialed");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Connect SLC */
            if (!appHfpConnectHandset())
            {
                /* Play error tone to indicate we don't have a valid address */
                appUiHfpError(FALSE);
                break;
            }
        }
        /* Fall through */
            
        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:            
        {    
            /* Play tone */
            appUiHfpLastNumberRedial();            

            /* Send message into HFP state machine */
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_LAST_NUMBER_REDIAL_REQ,
                                     NULL, &appHfpGetLock());		
        }
        break;        
            
        default:
            break;
    }    
}

/*! \brief Attempt voice dial

    Initiate voice dial, attempt to connect SLC first if not currently
    connected.
*/
void appHfpCallVoice(void)
{
    DEBUG_LOG("appHfpCallVoice");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* if we don't have a HFP connection but SCO FWD is connected, it means that we are the slave */
            if(appScoFwdIsConnected())
            {
                /* we send the command across the SCO FWD OTA channel asking the master to
                    trigger the CALL VOICE */
                appScoFwdCallVoice();
                break;
            }
            /* if SCO FWD is not connected we can try to enstablish SLC and proceed */
            else if(!appHfpConnectHandset())
            {
                /* Play error tone to indicate we don't have a valid address */
                appUiHfpError(FALSE);
                break;
            }
        }

        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        {    
            /* Play tone */
            appUiHfpVoiceDial();            

            /* Send message into HFP state machine */
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_VOICE_DIAL_REQ,
                                     NULL, &appHfpGetLock());		
        }
        break;        

        default:
            break;
    }
}

/*! \brief Disable voice dial

    Disable voice dial, attempt to connect SLC first if not currently
    connected.
*/
void appHfpCallVoiceDisable(void)
{
    DEBUG_LOG("appHfpCallVoiceDisable");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Connect SLC */
            if (!appHfpConnectHandset())
            {
                /* Play error tone to indicate we don't have a valid address */
                appUiHfpError(FALSE);
                break;
            }
        }
        /* Fall through */

        case HFP_STATE_CONNECTING_LOCAL:
        case HFP_STATE_CONNECTING_REMOTE:
        case HFP_STATE_CONNECTED_IDLE:
        {    
            /* Play tone */
            appUiHfpVoiceDialDisable();            

            /* Send message into HFP state machine */
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_VOICE_DIAL_DISABLE_REQ,
                                     NULL, &appHfpGetLock());		
        }
        break;        

        default:
            break;
    }
}

/*! \brief Accept call

    Accept incoming call, attempt to connect SLC first if not currently
    connected.
*/
void appHfpCallAccept(void)
{
    DEBUG_LOG("appHfpCallAccept");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Connect SLC */
            if (!appHfpConnectHandset())
            {
                /* Play error tone to indicate we don't have a valid address */
                appUiHfpError(FALSE);
                break;
            }
        }
        /* Fall through */

        case HFP_STATE_CONNECTED_INCOMING:
        {    
            /* Play tone */
            appUiHfpAnswer();

            /* Send message into HFP state machine */
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_CALL_ACCEPT_REQ,
                                     NULL, &appHfpGetLock());		
        }
        break;        

        default:
            break;
    }
}

/*! \brief Reject call

    Reject incoming call, attempt to connect SLC first if not currently
    connected.
*/
void appHfpCallReject(void)
{
    DEBUG_LOG("appHfpCallReject");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Connect SLC */
            if (!appHfpConnectHandset())
            {
                /* Play error tone to indicate we don't have a valid address */
                appUiHfpError(FALSE);
                break;
            }
        }
        /* Fall through */

        case HFP_STATE_CONNECTED_INCOMING:
        {    
            /* Play tone */
            appUiHfpReject();

            /* Send message into HFP state machine */
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_CALL_REJECT_REQ,
                                     NULL, &appHfpGetLock());		
        }
        break;        

        default:
            break;
    }
}

/*! \brief Hangup call

    Hangup active call, attempt to connect SLC first if not currently
    connected.
*/
void appHfpCallHangup(void)
{
    DEBUG_LOG("appHfpCallHangup");

    switch (appHfpGetState())
    {
        case HFP_STATE_DISCONNECTED:
        {
            /* Connect SLC */
            if (!appHfpConnectHandset())
            {
                /* Play error tone to indicate we don't have a valid address */
                appUiHfpError(FALSE);
                break;
            }
        }
        /* Fall through */

        case HFP_STATE_CONNECTED_INCOMING:
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:
        {    
            /* Play tone */
            appUiHfpHangup();

            /* Send message into HFP state machine */
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_CALL_HANGUP_REQ,
                                     NULL, &appHfpGetLock());		
        }
        break;        

        default:
            break;
    }
}

/*! \brief Store AV configuration

    This function is called to store the current HFP configuration.
    
    The configuration isn't store immediately, instead a timer is started, any
    currently running timer is cancel.  On timer expiration the configuration
    is written to Persistent Store, (see \ref appHfpHandleInternalConfigWriteRequest).
    This is to avoid multiple writes when the user adjusts the playback volume.
*/		
static void appHfpConfigStore(void)
{
    /* Cancel any pending messages */
    MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_CONFIG_WRITE_REQ);
    
    /* Store configuration after a 5 seconds */
    MessageSendLater(appGetHfpTask(), HFP_INTERNAL_CONFIG_WRITE_REQ, 0, D_SEC(5));
}

/*! \brief Make volume change
*/
static bool appHfpVolumeChange(int16 step)
{
    int8 volume = appGetHfp()->volume;
    DEBUG_LOG("appHfpVolumeChange");

    /* Check if increasing volume */    
    if (step > 0)
    {
        /* Adjust volume if not at limit */
        if (volume < 15)
            appGetHfp()->volume = ((volume + step) <= 15) ? volume + step : 15;
        else
            return FALSE;
    }
    else
    {
        /* Adjust volume if not at limit */
        if (volume > 0)
            appGetHfp()->volume = (volume >= -step) ? volume + step : 0;
        else
            return FALSE;
    }
    
    /* Update audio plug-in if SCO active */
    if (appGetHfp()->sco_sink)
    {
        appHfpVolumeNotifyClients();
        /* only set volume now, if SCOFWD hasn't registered to handle it */
        if (!appGetHfp()->sco_forward_handling_volume)
        {
            appKymeraScoSetVolume(appGetHfp()->volume);
        }
    }
           
    /* Tell AG new volume */
    if (appHfpIsConnected())
    {
        /* TODO: Support mulltipoint */
        HfpVolumeSyncSpeakerGainRequest(hfp_primary_link, &appGetHfp()->volume);
    }
            
    /* Store new configuration */        
    appHfpConfigStore();

    /* Return indicating volume changed successfully */
    return TRUE;
}

/*! \brief Continue volume change
*/
static bool appHfpVolumeRepeat(int16 step)
{
    DEBUG_LOGF("appHfpVolumeRepeat(%d)", step);
    
    if (appHfpIsConnected())
    {
        /* Handle volume change locally */
        if (appHfpVolumeChange(step))
        {
            /* Send repeat message later */
            MessageSendLater(appGetHfpTask(), step > 0 ? HFP_INTERNAL_VOLUME_UP : HFP_INTERNAL_VOLUME_DOWN, NULL, APP_UI_VOLUME_REPEAT_TIME);   
            appGetHfp()->volume_repeat = 1;

            /* Return indicating volume changed */
            return TRUE;
        }
        else
        {
            /* Play volume limit tone */
            appUiHfpVolumeLimit();
        }    
    }
    
    /* Return indicating volume not changed */
    return FALSE;
}

/*! \brief Start volume change

    Start a repeating volume change

    \param  step    change to be applied to volume, +ve or -ve
*/
void appHfpVolumeStart(int16 step)
{
    DEBUG_LOGF("appHfpVolumeStart(%d)", step);

    if (appHfpVolumeRepeat(step))
    {
        appGetHfp()->volume_repeat = 0;
        if (step > 0)
            appUiHfpVolumeUp();
        else
            appUiHfpVolumeDown();
    }
}

/*! \brief Stop volume change

    Cancel a repeating volume change. 

    \todo We don't need to supply the step for our HFP implementation of
    repeating volume. Remove the parameter ?

    \param  step    The volume step that was being used
*/
void appHfpVolumeStop(int16 step)
{
    UNUSED(step);
    DEBUG_LOGF("appHfpVolumeStop(%d)", step);

    /* Cancel any pending volume messages, play tone if message actually cancelled */
    if (MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_VOLUME_UP))
    {
        if (appGetHfp()->volume_repeat)
            appUiHfpVolumeUp();
    }
    if (MessageCancelFirst(appGetHfpTask(), HFP_INTERNAL_VOLUME_DOWN))
    {
        if (appGetHfp()->volume_repeat)
            appUiHfpVolumeDown();
    }
}

/*! \brief Toggle mute
*/
void appHfpMuteToggle(void)
{
    DEBUG_LOG("appHfpMuteToggle");
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            MAKE_HFP_MESSAGE(HFP_INTERNAL_HFP_MUTE_REQ);

            /* Send message into HFP state machine */
            message->mute = !appHfpIsMuted();
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_MUTE_REQ,
                                     message, &appHfpGetLock());	

            /* Play appropriate tone */                                     
            if (appHfpIsMuted())
                appUiHfpMuteInactive();        
            else
                appUiHfpMuteActive();        
        }
        break;
        
        default:
            break;
    }           
}

/*! \brief Transfer call to headset

    Transfer call to headset, attempt to connect SLC first if not currently
    connected.
*/
void appHfpTransferToHeadset(void)
{
    DEBUG_LOG("appHfpTransferToHeadset");
    
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            MAKE_HFP_MESSAGE(HFP_INTERNAL_HFP_TRANSFER_REQ);

            /* Send message into HFP state machine */
            message->transfer_to_ag = FALSE;
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_TRANSFER_REQ,
                                     message, &appHfpGetLock());	

            /* Play tone */
            appUiHfpTransfer();
        }
        break;
        
        default:
            break;
    }           
}

/*! \brief Transfer call to AG

    Transfer call to AG, attempt to connect SLC first if not currently
    connected.
*/
void appHfpTransferToAg(void)
{
    DEBUG_LOG("appHfpTransferToAg");
        
    switch (appHfpGetState())
    {
        case HFP_STATE_CONNECTED_OUTGOING:
        case HFP_STATE_CONNECTED_ACTIVE:
        {
            MAKE_HFP_MESSAGE(HFP_INTERNAL_HFP_TRANSFER_REQ);

            /* Send message into HFP state machine */
            message->transfer_to_ag = TRUE;
            MessageSendConditionally(appGetHfpTask(), HFP_INTERNAL_HFP_TRANSFER_REQ,
                                     message, &appHfpGetLock());	

            /* Play tone */
            appUiHfpTransfer();
        }
        break;
        
        default:
            break;
    }           
}

/*! \brief Register with HFP to receive notifications of SLC connect/disconnect.

    \param  task    The task being registered to receive notifications.
 */
void appHfpClientRegister(Task task)
{
    hfpTaskData* hfp = appGetHfp();
    appTaskListAddTask(hfp->slc_status_notify_list, task);
}

/*! \brief Register with HFP to receive notifications of state changes.

    \param  task    The task being registered to receive notifications.
 */
void appHfpStatusClientRegister(Task task)
{
    hfpTaskData* hfp = appGetHfp();
    appTaskListAddTask(hfp->status_notify_list, task);
}

/*! \brief Register task to handle custom AT commands.

    \param  task    The task being registered to receive the commands
 */
void appHfpRegisterAtCmdTask(Task task)
{
    hfpTaskData* hfp = appGetHfp();
    hfp->at_cmd_task = task;
}

extern uint8 appHfpGetVolume(void)
{
    return appGetHfp()->volume;
}

void appHfpScoFwdHandlingVolume(bool enabled)
{
    appGetHfp()->sco_forward_handling_volume = enabled;
}

/*! \brief Message Handler

    This function is the main message handler for the HFP instance, every
    message is handled in it's own seperate handler function.  The switch
    statement is broken into seperate blocks to reduce code size, if execution
    reaches the end of the function then it is assumed that the message is
    unhandled.
*/    
static void appHfpHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

    /* Handle internal messages */
    switch (id)
    {
        case HFP_INTERNAL_CONFIG_WRITE_REQ:
            appHfpHandleInternalConfigWriteRequest();
            return;

        case HFP_INTERNAL_MUTE_IND:
            appHfpHandleInternalMuteReminderIndication();
            return;

        case HFP_INTERNAL_SCO_UNENCRYPTED_IND:
            appHfpCheckEncryptedSco();
            return;

        case HFP_INTERNAL_HSP_INCOMING_TIMEOUT:
            appHfpHandleHfpHspIncomingTimeout();
            return;                

        case HFP_INTERNAL_HFP_CONNECT_REQ:
            appHfpHandleInternalHfpConnectRequest((HFP_INTERNAL_HFP_CONNECT_REQ_T *)message);
            return;
        
        case HFP_INTERNAL_HFP_DISCONNECT_REQ:
            appHfpHandleInternalHfpDisconnectRequest((HFP_INTERNAL_HFP_DISCONNECT_REQ_T *)message);
            return;
        
        case HFP_INTERNAL_HFP_LAST_NUMBER_REDIAL_REQ:
            appHfpHandleInternalHfpLastNumberRedialRequest();
            return;        

        case HFP_INTERNAL_HFP_VOICE_DIAL_REQ:
            appHfpHandleInternalHfpVoiceDialRequest();
            return;        

        case HFP_INTERNAL_HFP_VOICE_DIAL_DISABLE_REQ:
            appHfpHandleInternalHfpVoiceDialDisableRequest();
            return;        

        case HFP_INTERNAL_HFP_CALL_ACCEPT_REQ:
            appHfpHandleInternalHfpCallAcceptRequest();
            return;        

        case HFP_INTERNAL_HFP_CALL_REJECT_REQ:
            appHfpHandleInternalHfpCallRejectRequest();
            return;        

        case HFP_INTERNAL_HFP_CALL_HANGUP_REQ:
            appHfpHandleInternalHfpCallHangupRequest();
            return;        

        case HFP_INTERNAL_HFP_MUTE_REQ:
            appHfpHandleInternalHfpMuteRequest((HFP_INTERNAL_HFP_MUTE_REQ_T *)message);
            return;

        case HFP_INTERNAL_HFP_TRANSFER_REQ:
            appHfpHandleInternalHfpTransferRequest((HFP_INTERNAL_HFP_TRANSFER_REQ_T *)message);
            return;            

        case HFP_INTERNAL_VOLUME_UP:
            appHfpVolumeRepeat(1);
            return;

        case HFP_INTERNAL_VOLUME_DOWN:
            appHfpVolumeRepeat(-1);
            return;
    }
    
    /* HFP profile library messages */
    switch (id)
    {
        case HFP_INIT_CFM:
            appHfpHandleHfpInitConfirm((HFP_INIT_CFM_T *)message);
            return;

        case HFP_SLC_CONNECT_IND:
            appHfpHandleHfpSlcConnectIndication((HFP_SLC_CONNECT_IND_T *)message);
            return;

        case HFP_SLC_CONNECT_CFM:
            appHfpHandleHfpSlcConnectConfirm((HFP_SLC_CONNECT_CFM_T *)message);
            return;
 
        case HFP_SLC_DISCONNECT_IND:
            appHfpHandleHfpSlcDisconnectIndication((HFP_SLC_DISCONNECT_IND_T *)message);
            return;

        case HFP_AUDIO_CONNECT_IND:
             appHfpHandleHfpAudioConnectIndication((HFP_AUDIO_CONNECT_IND_T *)message);
             return;

        case HFP_AUDIO_CONNECT_CFM:
             appHfpHandleHfpAudioConnectConfirmation((HFP_AUDIO_CONNECT_CFM_T *)message);
             return;

        case HFP_AUDIO_DISCONNECT_IND:
             appHfpHandleHfpAudioDisconnectIndication((HFP_AUDIO_DISCONNECT_IND_T *)message);
             return;

        case HFP_RING_IND:
             appHfpHandleHfpRingIndication((HFP_RING_IND_T *)message);
             return;

        case HFP_SERVICE_IND:
             appHfpHandleHfpServiceIndication((HFP_SERVICE_IND_T *)message);
             return;

        case HFP_CALL_STATE_IND:
             appHfpHandleHfpCallStateIndication((HFP_CALL_STATE_IND_T *)message);
             return;

        case HFP_VOICE_RECOGNITION_IND:
             appHfpHandleHfpVoiceRecognitionIndication((HFP_VOICE_RECOGNITION_IND_T *)message);
             return;

        case HFP_VOICE_RECOGNITION_ENABLE_CFM:
             appHfpHandleHfpVoiceRecognitionEnableConfirmation((HFP_VOICE_RECOGNITION_ENABLE_CFM_T *)message);
             return;

        case HFP_CALLER_ID_IND:
             appHfpHandleHfpCallerIdIndication((HFP_CALLER_ID_IND_T *)message);
             return;
        
        case HFP_CALLER_ID_ENABLE_CFM:
             appHfpHandleHfpCallerIdEnableConfirmation((HFP_CALLER_ID_ENABLE_CFM_T *)message);
             return;
        
        case HFP_VOLUME_SYNC_SPEAKER_GAIN_IND:
             appHfpHandleHfpVolumeSyncSpeakerGainIndication((HFP_VOLUME_SYNC_SPEAKER_GAIN_IND_T *)message);
             return;

        case HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND:
             appHfpHandleHfpVolumeSyncMicrophoneGainIndication((HFP_VOLUME_SYNC_MICROPHONE_GAIN_IND_T *)message);
             return;

        case HFP_CALL_ANSWER_CFM:
             appHfpHandleHfpCallAnswerConfirmation((HFP_CALL_ANSWER_CFM_T *)message); 
             return;                
        
        case HFP_CALL_TERMINATE_CFM:
             appHfpHandleHfpCallTerminateConfirmation((HFP_CALL_TERMINATE_CFM_T *)message); 
             return;                

        case HFP_AT_CMD_CFM:
             appHfpHandleHfpAtCmdConfirm((HFP_AT_CMD_CFM_T*)message);
             return;

        case HFP_UNRECOGNISED_AT_CMD_IND:
             appHfpHandleHfpUnrecognisedAtCmdInd((HFP_UNRECOGNISED_AT_CMD_IND_T*)message);
             return;

        /* Handle additional messages */
        case HFP_HS_BUTTON_PRESS_CFM:
        case HFP_DIAL_LAST_NUMBER_CFM:
        case HFP_SIGNAL_IND:
        case HFP_ROAM_IND:
        case HFP_BATTCHG_IND:
        case HFP_CALL_WAITING_IND:
        case HFP_EXTRA_INDICATOR_INDEX_IND:
        case HFP_EXTRA_INDICATOR_UPDATE_IND:
        case HFP_NETWORK_OPERATOR_IND:
        case HFP_CURRENT_CALLS_CFM:
            return;

        case HFP_HF_INDICATORS_REPORT_IND:
            appHfpHandleHfpHfIndicatorsReportInd((HFP_HF_INDICATORS_REPORT_IND_T *)message);
            return;

        case HFP_HF_INDICATORS_IND:
            appHfpHandleHfpHfIndicatorsInd((HFP_HF_INDICATORS_IND_T *)message);
            return;
    }
    
    /* Handle connection library messages */
    switch (id)
    {
        case CL_DM_REMOTE_FEATURES_CFM:
            appHfpHandleClDmRemoteFeaturesConfirm((CL_DM_REMOTE_FEATURES_CFM_T *)message);
            return;
        
        case CL_SM_ENCRYPT_CFM:
            appHfpHandleClDmEncryptConfirmation((CL_SM_ENCRYPT_CFM_T *)message);
            return;
   }

    /* Handle other messages */
    switch (id)
    {
        case CON_MANAGER_CONNECTION_IND:
            appHfpHandleConManagerConnectionInd((CON_MANAGER_CONNECTION_IND_T *)message);
            return;

        case MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT:
            appHfpHandleBatteryLevelUpdatePercent((MESSAGE_BATTERY_LEVEL_UPDATE_PERCENT_T *)message);
            return;
    }

   /* Unhandled message */
   appHfpError(id, message);
}
#endif
