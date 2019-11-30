/*!
\copyright  Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_sm.c
\brief	    Application state machine
*/

#include "av_headset.h"
#include "av_headset_sm.h"
#include "av_headset_sm_private.h"
#include "av_headset_peer_sync.h"
#include "av_headset_ui.h"
#include "av_headset_gatt.h"
#include "av_headset_scan_manager.h"
#include "av_headset_con_manager.h"
#include "av_headset_log.h"
#include "av_headset_conn_rules.h"
#include "av_headset_config.h"
#include "av_headset_handset_signalling.h"
#include "av_headset_scofwd.h"

#include <panic.h>
#include <connection.h>
#include <ps.h>
#include <boot.h>
#include <message.h>

static void appSmHandleInternalDeleteHandsets(void);

#ifdef INCLUDE_DFU
static void appSmEnterDfuOnStartup(bool upgrade_reboot);
static void appSmNotifyUpgradeStarted(void);
static void appSmStartDfuTimer(void);
#endif /* INCLUDE_DFU */

/*****************************************************************************
 * SM utility functions
 *****************************************************************************/
static void appSmCancelDfuTimers(void)
{
    DEBUG_LOG("appSmCancelDfuTimers Cancelled all SM_INTERNAL_TIMEOUT_DFU_... timers");

    MessageCancelAll(appGetSmTask(),SM_INTERNAL_TIMEOUT_DFU_ENTRY);
    MessageCancelAll(appGetSmTask(),SM_INTERNAL_TIMEOUT_DFU_MODE_START);
    MessageCancelAll(appGetSmTask(),SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT);
}

/*! \brief Delete all peer and handset pairing and reboot device. */
static void appSmDeletePairingAndReset(void)
{
    bdaddr bd_addr;

    DEBUG_LOG("appSmDeletePairingAndReset");

    /* cancel the link disconnection, may already be gone if it fired to get us here */
    MessageCancelFirst(appGetSmTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION);

    /* delete all paired handsets */
    appSmHandleInternalDeleteHandsets();

    /* delete paired peer earbud */
    if (appDeviceGetPeerBdAddr(&bd_addr))
        appDeviceDelete(&bd_addr);

    /* Flood fill PS to force a defrag on reboot */
    PsFlood();
    
    appPowerReboot();
}


/*! \brief Determine which state to transition to based on physical state.
    \return appState One of the states that correspond to a physical earbud state.
*/
static appState appSmCalcCoreState(void)
{
    bool busy = appAvIsStreaming() || appHfpIsScoActive();

    switch (appSmGetPhyState())
    {
        case PHY_STATE_IN_CASE:
            return APP_STATE_IN_CASE_IDLE;

        case PHY_STATE_OUT_OF_EAR:
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            return busy ? APP_STATE_OUT_OF_CASE_BUSY :
                          APP_STATE_OUT_OF_CASE_IDLE;

        case PHY_STATE_IN_EAR:
            return busy ? APP_STATE_IN_EAR_BUSY :
                          APP_STATE_IN_EAR_IDLE;

        /* Physical state is unknown, default to in-case state */
        case PHY_STATE_UNKNOWN:
            return APP_STATE_IN_CASE_IDLE;

        default:
            Panic();
    }

    return APP_STATE_IN_EAR_IDLE;
}

static void appSmSetCoreState(void)
{
    appState state = appSmCalcCoreState();
    if (state != appGetState())
        appSetState(state);
}

/*! \brief Set the core app state for the first time. */
static void appSmSetInitialCoreState(void)
{
    smTaskData *sm = appGetSm();

    /* Register with physical state manager to get notification of state
     * changes such as 'in-case', 'in-ear' etc */
    appPhyStateRegisterClient(&sm->task);

    /* Get latest physical state */
    sm->phy_state = appPhyStateGetState();

    /* Generate physical state events */
    switch (sm->phy_state)
    {
        case PHY_STATE_IN_CASE:
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_IN_CASE);
            break;

        case PHY_STATE_OUT_OF_EAR_AT_REST:
        case PHY_STATE_OUT_OF_EAR:
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_OUT_CASE);
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_OUT_EAR);
            break;

        case PHY_STATE_IN_EAR:
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_OUT_CASE);
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_IN_EAR);
            break;

        default:
            Panic();
    }

    /* Update core state */
    appSmSetCoreState();
}

/*! \brief Initiate disconnect of all links */
static void appSmInitiateLinkDisconnection(smDisconnectBits links_to_disconnect, uint16 timeout_ms,
                                           smPostDisconnectAction post_disconnect_action)
{
    smDisconnectBits disconnecting_links = 0;

    DEBUG_LOGF("appSmInitiateLinkDisconnection links %d, timeout %d, action %d",
               links_to_disconnect,
               timeout_ms,
               post_disconnect_action);

    if (links_to_disconnect & SM_DISCONNECT_PEER)
    {
        if (appDeviceIsPeerConnected())
        {
            DEBUG_LOG("appSmInitiateLinkDisconnection PEER IS CONNECTED");
            if (timeout_ms != 0)
            {
                appPeerSigForceDisconnect();
                appAvDisconnectPeer();
            }
            disconnecting_links |= SM_DISCONNECT_PEER;
        }
    }
    if (links_to_disconnect & SM_DISCONNECT_HANDSET)
    {
        if (!appDeviceIsHandsetHfpDisconnected())
        {
            DEBUG_LOG("appSmInitiateLinkDisconnection HANDSET HFP IS CONNECTED");
            if (timeout_ms != 0)
                appHfpDisconnectInternal();

            disconnecting_links |= SM_DISCONNECT_HANDSET;
        }
        if (!appDeviceIsHandsetA2dpDisconnected() || !appDeviceIsHandsetAvrcpDisconnected())
        {
            DEBUG_LOG("appSmInitiateLinkDisconnection HANDSET AV IS CONNECTED");
            if (timeout_ms != 0)
                appAvDisconnectHandset();

            disconnecting_links |= SM_DISCONNECT_HANDSET;
        }
    }


    MESSAGE_MAKE(msg, SM_INTERNAL_LINK_DISCONNECTION_COMPLETE_T);
    if (!disconnecting_links)
    {
        appSmDisconnectLockClearLinks(SM_DISCONNECT_ALL);
    }
    else
    {
        appSmDisconnectLockSetLinks(disconnecting_links);
    }
    msg->post_disconnect_action = post_disconnect_action;
    MessageSendConditionally(appGetSmTask(), SM_INTERNAL_LINK_DISCONNECTION_COMPLETE,
                             msg, &appSmDisconnectLockGet());

    /* Start a timer to force reset if we fail to complete disconnection */
    MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION, NULL, timeout_ms);
}


static bool appSmStateNewBleConnectionChange(appState last,appState next)
{
    bool ble_was_allowed = appSmStateAreNewBleConnectionsAllowed(last);
    bool ble_now_allowed = appSmStateAreNewBleConnectionsAllowed(next);
    bool state_has_changed = last != next;

    /* If we are BLE connectable and state has changed may need to re-advertise */
    if (state_has_changed && ble_now_allowed)
        return TRUE;

    if (ble_was_allowed != ble_now_allowed)
        return TRUE;

    return FALSE;
}


/*****************************************************************************
 * SM state transition handler functions
 *****************************************************************************/
/*! \brief Enter initialising state

    This function is called whenever the state changes to
    APP_STATE_INITIALISING.
    It is reponsible for initialising global aspects of the application,
    i.e. non application module related state.
*/
static void appEnterInitialising(void)
{
    DEBUG_LOG("appEnterInitialising");
}

/*! \brief Exit initialising state.
 */
static void appExitInitialising(void)
{
    DEBUG_LOG("appExitInitialising");
}

/*! \brief Enter
 */
static void appEnterDfuCheck(void)
{
    DEBUG_LOG("appEnterDfuCheck");

    /* We only get into DFU check, if there wasn't a pending DFU
       So send a message to go to startup. */
    MessageSend(appGetSmTask(), SM_INTERNAL_NO_DFU, NULL);
}

/*! \brief Exit
 */
static void appExitDfuCheck(void)
{
    DEBUG_LOG("appExitDfuCheck");
}

/*! \brief Enter actions when we enter the factory reset state.
    The application will drop all links, delete all pairing and reboot.
 */
static void appEnterFactoryReset(void)
{
    DEBUG_LOG("appEnterFactoryReset");
    appSmInitiateLinkDisconnection(SM_DISCONNECT_ALL, appConfigFactoryResetTimeoutMs(),
                                   POST_DISCONNECT_ACTION_NONE);
}

/*! \brief Exit factory reset. */
static void appExitFactoryReset(void)
{
    /* Should never happen */
    Panic();
}

/*! \brief Actions to take on entering startup state
 */
static void appEnterStartup(void)
{
    DEBUG_LOG("appEnterStartup");

    /* Initial kick to the connection rules engine */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_STARTUP);

    /* As we have entered startup state we know there is no upgrade in
       progress. If needed block upgrades until we know we are in an 
       appropriate state */
#ifdef INCLUDE_DFU
    if (   (   !appConfigDfuAllowBleUpgradeOutOfCase()
            && !appConfigDfuAllowBredrUpgradeOutOfCase())
        || appConfigDfuOnlyFromUiInCase())
    {
        appUpgradeAllowUpgrades(FALSE);
    }
#endif
}

static void appExitStartup(void)
{
    DEBUG_LOG("appExitStartup");

    /* Clear event now we're exiting startup state */
    appConnRulesResetEvent(RULE_EVENT_STARTUP);
}

/*! \brief Start process to pair with peer earbud.
 */
static void appEnterPeerPairing(void)
{
    DEBUG_LOG("appEnterPeerPairing");

    appPairingPeerPair(appGetSmTask(), FALSE);
}

/*! \brief Exit actions when peer pairing completed.
 */
static void appExitPeerPairing(void)
{
    DEBUG_LOG("appExitPeerPairing");

    /* No actions currently required. */
    appPairingPeerPairCancel();
}


/*! \brief Start process to pairing with handset.
 */
static void appEnterHandsetPairing(void)
{
    DEBUG_LOG("appEnterHandsetPairing");

    appSmInitiateLinkDisconnection(SM_DISCONNECT_HANDSET,
                                   appConfigLinkPairingDisconnectionTimeoutTerminatingMs(),
                                   POST_DISCONNECT_ACTION_HANDSET_PAIRING);
    appPeerSyncSend(FALSE);

    appGattSetAdvertisingMode(APP_ADVERT_RATE_FAST);
}

/*! \brief Exit actions when handset pairing completed.
 */
static void appExitHandsetPairing(void)
{
    DEBUG_LOG("appExitHandsetPairing");

    appConnRulesSetRuleComplete(CONN_RULES_HANDSET_PAIR);
    appPairingHandsetPairCancel();
    appPeerSyncSend(FALSE);

    appGattSetAdvertisingMode(APP_ADVERT_RATE_SLOW);
}

/*! \brief Enter
 */
static void appEnterInCaseIdle(void)
{
    DEBUG_LOG("appEnterInCaseIdle");
}

/*! \brief Exit
 */
static void appExitInCaseIdle(void)
{
    DEBUG_LOG("appExitInCaseIdle");
}

/*! \brief Enter
 */
static void appEnterInCaseDfu(void)
{
    DEBUG_LOG("appEnterInCaseDfu");

    /* send peer sync to inform peer that DFU is in progress. */
    appPeerSyncSend(FALSE);

    appGetSm()->enter_dfu_in_case = FALSE;

    appGattSetAdvertisingMode(APP_ADVERT_RATE_SLOW);
}

/*! \brief Exit
 */
static void appExitInCaseDfu(void)
{
    DEBUG_LOG("appExitInCaseDfu");
    appSmCancelDfuTimers();

    appGattSetAdvertisingMode(APP_ADVERT_RATE_SLOW);

}

/*! \brief Enter
 */
static void appEnterOutOfCaseIdle(void)
{
    DEBUG_LOG("appEnterOutOfCaseIdle");

    /* Show idle on LEDs */
    appUiIdleActive();

    if (appConfigIdleTimeoutMs())
    {
        MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_IDLE, NULL, appConfigIdleTimeoutMs());
    }
}

/*! \brief Exit
 */
static void appExitOutOfCaseIdle(void)
{
    DEBUG_LOG("appExitOutOfCaseIdle");

    /* Stop idle on LEDs */
    appUiIdleInactive();

    MessageCancelFirst(appGetSmTask(), SM_INTERNAL_TIMEOUT_IDLE);
}

/*! \brief Enter
 */
static void appEnterOutOfCaseBusy(void)
{
    DEBUG_LOG("appEnterOutOfCaseBusy");
}

/*! \brief Cancel the audio pause timer.

    May have already fired, which is causing us to exit this state, but
    safe to do so.
 */
static void appExitOutOfCaseBusy(void)
{
    DEBUG_LOG("appExitOutOfCaseBusy");
}

/*! \brief Enter. */
static void appEnterOutOfCaseSoporific(void)
{
    DEBUG_LOG("appEnterOutOfCaseSoporific");
}

/*! \brief Exit. */
static void appExitOutOfCaseSoporific(void)
{
    DEBUG_LOG("appExitOutOfCaseSoporific");
}

/*! \brief Exit. */
static void appExitOutOfCaseSoporificTerminating(void)
{
    DEBUG_LOG("appExitOutOfCaseSoporificTerminating");
}

/*! \brief Enter. */
static void appEnterOutOfCaseSoporificTerminating(void)
{
    DEBUG_LOG("appEnterOutOfCaseSoporificTerminating");
}

/*! \brief Common actions when one of the terminating substates is entered.
 */
static void appEnterSubStateTerminating(void)
{
    DEBUG_LOG("appEnterSubStateTerminating");
    appSmInitiateLinkDisconnection(SM_DISCONNECT_ALL, appConfigLinkDisconnectionTimeoutTerminatingMs(),
                                   POST_DISCONNECT_ACTION_NONE);
}

/*! \brief Common actions when one of the terminating substates is exited.
 */
static void appExitSubStateTerminating(void)
{
    DEBUG_LOG("appExitSubStateTerminating");
}

/*! \brief Enter
 */
static void appEnterInEarIdle(void)
{
    DEBUG_LOG("appEnterInEarIdle");
}

/*! \brief Exit
 */
static void appExitInEarIdle(void)
{
    DEBUG_LOG("appExitInEarIdle");
}

/*! \brief Enter
 */
static void appEnterInEarBusy(void)
{
    DEBUG_LOG("appEnterInEarBusy");
}

/*! \brief Exit
 */
static void appExitInEarBusy(void)
{
    DEBUG_LOG("appExitInEarBusy");
}

/*! \brief Exit APP_STATE_IN_CASE parent function actions. */
static void appExitInCase(void)
{
    DEBUG_LOG("appExitInCase");

    /* run rules for being taken out of the case */
    appConnRulesResetEvent(RULE_EVENT_IN_CASE);
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_OUT_CASE);
}

/*! \brief Exit APP_STATE_OUT_OF_CASE parent function actions. */
static void appExitOutOfCase(void)
{
    DEBUG_LOG("appExitOutOfCase");

    MessageCancelFirst(appGetSmTask(), SM_INTERNAL_TIMEOUT_OUT_OF_EAR_A2DP);
    MessageCancelFirst(appGetSmTask(), SM_INTERNAL_TIMEOUT_OUT_OF_EAR_SCO);
}

/*! \brief Exit APP_STATE_IN_EAR parent function actions. */
static void appExitInEar(void)
{
    DEBUG_LOG("appExitInEar");

    /* Run rules for out of ear event */
    appConnRulesResetEvent(RULE_EVENT_IN_EAR);
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_OUT_EAR);
}

/*! \brief Enter APP_STATE_IN_CASE parent function actions. */
static void appEnterInCase(void)
{
    DEBUG_LOG("appEnterInCase");

    /* request handset signalling module send current state to handset. */
    appHandsetSigSendEarbudStateReq(PHY_STATE_IN_CASE);

    /* Run rules for in case event */
    appConnRulesResetEvent(RULE_EVENT_OUT_CASE);
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_IN_CASE);
}

/*! \brief Enter APP_STATE_OUT_OF_CASE parent function actions. */
static void appEnterOutOfCase(void)
{
    DEBUG_LOG("appEnterOutOfCase");

    /* request handset signalling module send current state to handset. */
    appHandsetSigSendEarbudStateReq(PHY_STATE_OUT_OF_EAR);
}

/*! \brief Enter APP_STATE_IN_EAR parent function actions. */
static void appEnterInEar(void)
{
    DEBUG_LOG("appEnterInEar");

    /* request handset signalling module send current state to handset. */
    appHandsetSigSendEarbudStateReq(PHY_STATE_IN_EAR);

    /* Run rules for in ear event */
    appConnRulesResetEvent(RULE_EVENT_OUT_EAR);
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_IN_EAR);

    /* try and cancel the A2DP auto restart timer, if there was one
     * then we're inside the time required to automatically restart
     * audio due to earbud being put back in the ear, so we need to
     * send a play request to the handset */
    if (MessageCancelFirst(appGetSmTask(), SM_INTERNAL_TIMEOUT_IN_EAR_A2DP_START))
    {
        appAvPlay(FALSE);
    }

    /* \todo this should move to a rule at some point */
    if (appHfpIsCallIncoming())
    {
        DEBUG_LOG("appEnterInEar accepting call HFP");
        appHfpCallAccept();
    }
    else if(appScoFwdIsCallIncoming())
    {
        DEBUG_LOG("appEnterInEar accepting call SCOFWD");
        appScoFwdCallAccept();
    }
}

/*****************************************************************************
 * End of SM state transition handler functions
 *****************************************************************************/

/* This function is called to change the applications state, it automatically
   calls the entry and exit functions for the new and old states.
*/
void appSetState(appState new_state)
{
    appState previous_state = appGetSm()->state;

    DEBUG_LOGF("appSetState, state 0x%02x to 0x%02x", previous_state, new_state);

    /* Handle state exit functions */
    switch (previous_state)
    {
        case APP_STATE_NULL:
            /* This can occur when DFU is entered during INIT. */
            break;

        case APP_STATE_INITIALISING:
            appExitInitialising();
            break;

        case APP_STATE_DFU_CHECK:
            appExitDfuCheck();
            break;

        case APP_STATE_FACTORY_RESET:
            appExitFactoryReset();
            break;

        case APP_STATE_STARTUP:
            appExitStartup();
            break;

        case APP_STATE_PEER_PAIRING:
            appExitPeerPairing();
            break;

        case APP_STATE_HANDSET_PAIRING:
            appExitHandsetPairing();
            break;

        case APP_STATE_IN_CASE_IDLE:
            appExitInCaseIdle();
            break;

        case APP_STATE_IN_CASE_DFU:
            appExitInCaseDfu();
            break;

        case APP_STATE_OUT_OF_CASE_IDLE:
            appExitOutOfCaseIdle();
            break;

        case APP_STATE_OUT_OF_CASE_BUSY:
            appExitOutOfCaseBusy();
            break;

        case APP_STATE_OUT_OF_CASE_SOPORIFIC:
            appExitOutOfCaseSoporific();
            break;

        case APP_STATE_OUT_OF_CASE_SOPORIFIC_TERMINATING:
            appExitOutOfCaseSoporificTerminating();
            break;

        case APP_STATE_TERMINATING:
            DEBUG_LOG("appExitTerminating");
            break;

        case APP_STATE_IN_CASE_TERMINATING:
            DEBUG_LOG("appExitInCaseTerminating");
            break;

        case APP_STATE_OUT_OF_CASE_TERMINATING:
            DEBUG_LOG("appExitOutOfCaseTerminating");
            break;

        case APP_STATE_IN_EAR_TERMINATING:
            DEBUG_LOG("appExitInEarTerminating");
            break;

        case APP_STATE_IN_CASE_DISCONNECTING:
            DEBUG_LOG("appExitInCaseDisconnecting");
            break;

        case APP_STATE_OUT_OF_CASE_DISCONNECTING:
            DEBUG_LOG("appExitOutOfCaseDisconnecting");
            break;

        case APP_STATE_IN_EAR_DISCONNECTING:
            DEBUG_LOG("appExitInEarDisconnecting");
            break;

        case APP_STATE_IN_EAR_IDLE:
            appExitInEarIdle();
            break;

        case APP_STATE_IN_EAR_BUSY:
            appExitInEarBusy();
            break;

        default:
            DEBUG_LOGF("Attempted to exit unsupported state 0x%x", appGetSm()->state);
            Panic();
            break;
    }

    /* if exiting a sleepy state */
    if (appSmStateIsSleepy(previous_state) && !appSmStateIsSleepy(new_state))
        appPowerClientProhibitSleep(appGetSmTask());

    /* if exiting a terminating substate */
    if (appSmSubStateIsTerminating(previous_state) && !appSmSubStateIsTerminating(new_state))
        appExitSubStateTerminating();

    /* if exiting the APP_STATE_IN_CASE parent state */
    if (appSmStateInCase(previous_state) && !appSmStateInCase(new_state))
        appExitInCase();

    /* if exiting the APP_STATE_OUT_OF_CASE parent state */
    if (appSmStateOutOfCase(previous_state) && !appSmStateOutOfCase(new_state))
        appExitOutOfCase();

    /* if exiting the APP_STATE_IN_EAR parent state */
    if (appSmStateInEar(previous_state) && !appSmStateInEar(new_state))
        appExitInEar();

    /* Set new state */
    appGetSm()->state = new_state;

    /* transitioning from connectable to not connectable, or vice versa */
    if (appSmStateIsConnectable(previous_state) != appSmStateIsConnectable(new_state))
    {
        /* Kick the rules engine. The rule for RULE_EVENT_PAGE_SCAN_UPDATE is 
           an always evaluated rule, so the event does not need to be reset */
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PAGE_SCAN_UPDATE);
    }

    /* transitioning from BLE connectable to not connectable, or vice versa */
    if (appSmStateNewBleConnectionChange(previous_state,new_state))
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);
    }

    /* if entering the APP_STATE_IN_CASE parent state */
    if (!appSmStateInCase(previous_state) && appSmStateInCase(new_state))
        appEnterInCase();

    /* if entering the APP_STATE_OUT_OF_CASE parent state */
    if (!appSmStateOutOfCase(previous_state) && appSmStateOutOfCase(new_state))
        appEnterOutOfCase();

    /* if entering the APP_STATE_IN_EAR parent state */
    if (!appSmStateInEar(previous_state) && appSmStateInEar(new_state))
        appEnterInEar();

    /* if entering a terminating substate */
    if (!appSmSubStateIsTerminating(previous_state) && appSmSubStateIsTerminating(new_state))
        appEnterSubStateTerminating();

    /* if entering a sleepy state */
    if (!appSmStateIsSleepy(previous_state) && appSmStateIsSleepy(new_state))
        appPowerClientAllowSleep(appGetSmTask());

    /* Handle state entry functions */
    switch (new_state)
    {
        case APP_STATE_INITIALISING:
            appEnterInitialising();
            break;

        case APP_STATE_DFU_CHECK:
            appEnterDfuCheck();
            break;

        case APP_STATE_FACTORY_RESET:
            appEnterFactoryReset();
            break;

        case APP_STATE_STARTUP:
            appEnterStartup();
            break;

        case APP_STATE_PEER_PAIRING:
            appEnterPeerPairing();
            break;

        case APP_STATE_HANDSET_PAIRING:
            appEnterHandsetPairing();
            break;

        case APP_STATE_IN_CASE_IDLE:
            appEnterInCaseIdle();
            break;

        case APP_STATE_IN_CASE_DFU:
            appEnterInCaseDfu();
            break;

        case APP_STATE_OUT_OF_CASE_IDLE:
            appEnterOutOfCaseIdle();
            break;

        case APP_STATE_OUT_OF_CASE_BUSY:
            appEnterOutOfCaseBusy();
            break;

        case APP_STATE_OUT_OF_CASE_SOPORIFIC:
            appEnterOutOfCaseSoporific();
            break;

        case APP_STATE_OUT_OF_CASE_SOPORIFIC_TERMINATING:
            appEnterOutOfCaseSoporificTerminating();
            break;

        case APP_STATE_TERMINATING:
            DEBUG_LOG("appEnterTerminating");
            break;

        case APP_STATE_IN_CASE_TERMINATING:
            DEBUG_LOG("appEnterInCaseTerminating");
            break;

        case APP_STATE_OUT_OF_CASE_TERMINATING:
            DEBUG_LOG("appEnterOutOfCaseTerminating");
            break;

        case APP_STATE_IN_EAR_TERMINATING:
            DEBUG_LOG("appEnterInEarTerminating");
            break;

        case APP_STATE_IN_CASE_DISCONNECTING:
            DEBUG_LOG("appEnterInCaseDisconnecting");
            break;

        case APP_STATE_OUT_OF_CASE_DISCONNECTING:
            DEBUG_LOG("appEnterOutOfCaseDisconnecting");
            break;

        case APP_STATE_IN_EAR_DISCONNECTING:
            DEBUG_LOG("appEnterInEarDisconnecting");
            break;

        case APP_STATE_IN_EAR_IDLE:
            appEnterInEarIdle();
            break;

        case APP_STATE_IN_EAR_BUSY:
            appEnterInEarBusy();
            break;

        default:
            DEBUG_LOGF("Attempted to enter unsupported state %d", new_state);
            Panic();
            break;
    }
}

appState appGetState(void)
{
    return appGetSm()->state;
}

/*! \brief Modify the substate of the current parent appState and return the new state. */
static appState appSetSubState(appSubState substate)
{
    appState state = appGetState();
    state &= ~APP_SUBSTATE_MASK;
    state |= substate;
    return state;
}

static appSubState appGetSubState(void)
{
    return appGetState() & APP_SUBSTATE_MASK;
}

/*! \brief Update the disconnecting links lock status */
static void appSmUpdateDisconnectingLinks(void)
{
    bdaddr remote_bdaddr;
    if (appSmDisconnectLockPeerIsDisconnecting())
    {
        DEBUG_LOG("appSmUpdateDisconnectingLinks disconnecting peer");
        if (!appDeviceGetPeerBdAddr(&remote_bdaddr) || !appConManagerIsConnected(&remote_bdaddr))
        {
            DEBUG_LOG("appSmUpdateDisconnectingLinks peer disconnected");
            appSmDisconnectLockClearLinks(SM_DISCONNECT_PEER);
        }
    }
    if (appSmDisconnectLockHandsetIsDisconnecting())
    {
        DEBUG_LOG("appSmUpdateDisconnectingLinks disconnecting handset");
        if (!appDeviceGetHandsetBdAddr(&remote_bdaddr) || !appConManagerIsConnected(&remote_bdaddr))
        {
            DEBUG_LOG("appSmUpdateDisconnectingLinks handset disconnected");
            appSmDisconnectLockClearLinks(SM_DISCONNECT_HANDSET);
        }
    }
}

/*! \brief Handle notification of (dis)connections. */
static void appSmHandleConManagerConnectionInd(CON_MANAGER_CONNECTION_IND_T* ind)
{
    DEBUG_LOGF("appSmHandleConManagerConnectionInd connected:%d", ind->connected);

    if (ind->ble)
    {
        /*! Although we get this, indicating that the BLE link has changed 
            state, our booleans that track this are not yet updated, so
            there is no point kicking the peer synchronisation. */
    }

    if (ind->connected)
    {
        if (!ind->ble)
        {
            /* A BREDR connection won't neccessarily evaluate rules.
               And if we evaluate them now, the conditions needed for Upgrade control 
               will not have changed.  Send a message instead. */
            MessageSend(appGetSmTask(), SM_INTERNAL_BREDR_CONNECTED, NULL);
        }
        return;
    }

    appSmUpdateDisconnectingLinks();

    if (ind->ble)
    {
        DEBUG_LOGF("appSmHandleConManagerConnectionInd BLE link");
        return;
    }

    switch (appGetState())
    {
        default:
            /* Check if we've disconnected to peer */
            if (appDeviceIsPeer(&ind->bd_addr))
            {
                DEBUG_LOG("appSmHandleConManagerConnectionInd, disconnected from peer");

                /* Peer sync information is now out of date */
                appPeerSyncResetSync();
            }
            break;
    }

    /* Check if message was sent due to link-loss */
    if (ind->reason == hci_error_conn_timeout)
    {
        /* Generate link-loss events */
        if (appDeviceIsPeer(&ind->bd_addr))
        {
            DEBUG_LOG("appSmHandleConManagerConnectionInd, peer link loss");
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_LINK_LOSS);
        }
        else if (appDeviceIsHandset(&ind->bd_addr))
        {
            DEBUG_LOG("appSmHandleConManagerConnectionInd, handset link loss");
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_LINK_LOSS);
        }
    }

}

/*! \brief Handle completion of application module initialisation. */
static void appSmHandleInitConfirm(void)
{
    DEBUG_LOG("appSmHandleInitConfirm");

    /* Now we are fully initialised, enable messages from the always on
       event for DFU checking */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_CHECK_DFU);

    switch (appGetState())
    {
        case APP_STATE_INITIALISING:
            appSetState(APP_STATE_DFU_CHECK);
            appPowerOn();
            break;

        case APP_STATE_IN_CASE_DFU:
            appPowerOn();
            break;

        default:
            Panic();
    }
}

/*! \brief Handle completion of peer pairing. */
static void appSmHandlePairingPeerPairConfirm(PAIRING_PEER_PAIR_CFM_T *cfm)
{
    DEBUG_LOGF("appSmHandlePairingPeerPairConfirm, status %d", cfm->status);

    /* only mark as complete if succeeds as we want the rules to
     * still consider this task outstanding and request a retry */
    if (cfm->status == pairingSuccess)
    {
        appConnRulesSetRuleComplete(CONN_RULES_PEER_PAIR);
    }

    switch (appGetState())
    {
        case APP_STATE_PEER_PAIRING:
        {
            /* peer pairing operation completed, always moved
             * back to STARTUP, if not successful the rules
             * will try again */
            appSetState(APP_STATE_STARTUP);
        }
        break;

        case APP_STATE_FACTORY_RESET:
            /* Nothing to do, even if peer pairing succeeded, the final act of
               factory reset is to delete the peer pairing */
        break;

        default:
            switch (appGetSubState())
            {
                case APP_SUBSTATE_TERMINATING:
                    /* Nothing to do. If pairing failed, the user can try again
                    next power on. If pairing succeeded, the user can connect
                    to their phone next power on. */
                    break;
                default:
                    Panic();
                    break;
            }
    }
}

/*! \brief Handle completion of handset pairing. */
static void appSmHandlePairingHandsetPairConfirm(PAIRING_HANDSET_PAIR_CFM_T *cfm)
{
    UNUSED(cfm);
    DEBUG_LOGF("appSmHandlePairingHandsetPairConfirm, status %d", cfm->status);

    switch (appGetState())
    {
        case APP_STATE_HANDSET_PAIRING:
            appSmSetCoreState();
            break;

        case APP_STATE_FACTORY_RESET:
            /* Nothing to do, even if pairing with handset succeeded, the final
            act of factory reset is to delete handset pairing */
            break;

        default:
            /* Ignore, paired with handset with known address as requested by peer */
            break;
    }
}

/*! \brief Handle state machine transitions when earbud removed from the ear,
           or transitioning motionless out of the ear to just out of the ear. */
static void appSmHandlePhyStateOutOfEarEvent(void)
{
    DEBUG_LOGF("appSmHandlePhyStateOutOfEarEvent state=0x%x", appGetState());

    if (appSmIsCoreState())
        appSmSetCoreState();
}

/*! \brief Handle state machine transitions when earbud motionless out of the ear. */
static void appSmHandlePhyStateOutOfEarAtRestEvent(void)
{
    DEBUG_LOGF("appSmHandlePhyStateOutOfEarAtRestEvent state=0x%x", appGetState());

    if (appSmIsCoreState())
        appSmSetCoreState();
}

/*! \brief Handle state machine transitions when earbud is put in the ear. */
static void appSmHandlePhyStateInEarEvent(void)
{
    DEBUG_LOGF("appSmHandlePhyStateInEarEvent state=0x%x", appGetState());
    if (appSmIsCoreState())
        appSmSetCoreState();
}

/*! \brief Handle state machine transitions when earbud is put in the case. */
static void appSmHandlePhyStateInCaseEvent(void)
{
    DEBUG_LOGF("appSmHandlePhyStateInCaseEvent state=0x%x", appGetState());

    /*! \todo Need to add other non-core states to this conditional from which we'll
     * permit a transition back to a core state, such as...peer pairing? sleeping? */
    if (appSmIsCoreState() ||
        (appGetState() == APP_STATE_HANDSET_PAIRING))
    {
        appSmSetCoreState();
    }
}

/*! \brief Handle changes in physical state of the earbud. */
static void appSmHandlePhyStateChangedInd(smTaskData* sm, PHY_STATE_CHANGED_IND_T *ind)
{
    UNUSED(sm);

    DEBUG_LOGF("appSmHandlePhyStateChangedInd new phy state %d", ind->new_state);

    sm->phy_state = ind->new_state;

    switch (ind->new_state)
    {
        case PHY_STATE_IN_CASE:
            appSmHandlePhyStateInCaseEvent();
            return;
        case PHY_STATE_OUT_OF_EAR:
            appSmHandlePhyStateOutOfEarEvent();
            return;
        case PHY_STATE_OUT_OF_EAR_AT_REST:
            appSmHandlePhyStateOutOfEarAtRestEvent();
            return;
        case PHY_STATE_IN_EAR:
            appSmHandlePhyStateInEarEvent();
            return;
        default:
            break;
    }
}

/*! \brief Take action following power's indication of imminent sleep.
    SM only permits sleep when soporific. */
static void appSmHandlePowerSleepPrepareInd(void)
{
    DEBUG_LOG("appSmHandlePowerSleepPrepareInd");

    PanicFalse(APP_STATE_OUT_OF_CASE_SOPORIFIC == appGetState());

    appSetState(APP_STATE_OUT_OF_CASE_SOPORIFIC_TERMINATING);
}

/*! \brief Handle sleep cancellation. */
static void appSmHandlePowerSleepCancelledInd(void)
{
    DEBUG_LOG("appSmHandlePowerSleepCancelledInd");
    /* Sleep can only be entered from a core state, so return to the current core state */
    appSmSetCoreState();
}

/*! \brief Take action following power's indication of imminent shutdown.
    Can be received in any state. */
static void appSmHandlePowerShutdownPrepareInd(void)
{
    DEBUG_LOG("appSmHandlePowerShutdownPrepareInd");

    appSetState(appSetSubState(APP_SUBSTATE_TERMINATING));
}

/*! \brief Handle shutdown cancelled. */
static void appSmHandlePowerShutdownCancelledInd(void)
{
    DEBUG_LOG("appSmHandlePowerShutdownCancelledInd");
    /* Shutdown can be entered from any state (including non core states, e.g.
     peer pairing etc), so startup again, to determine the correct state to enter
     */
    appSetState(APP_STATE_STARTUP);
}


/*! \brief Start pairing with peer earbud. */
static void appSmHandleConnRulesPeerPair(void)
{
    DEBUG_LOG("appSmHandleConnRulesPeerPair");

    switch (appGetState())
    {
        case APP_STATE_STARTUP:
            DEBUG_LOG("appSmHandleConnRulesPeerPair, rule said pair with peer");
            appSetState(APP_STATE_PEER_PAIRING);
            break;

    default:
            break;
    }
}

static void appSmHandleConnRulesHandsetPair(void)
{
    DEBUG_LOG("appSmHandleConnRulesHandsetPair");

    switch (appGetState())
    {
        case APP_STATE_OUT_OF_CASE_IDLE:
        case APP_STATE_IN_EAR_IDLE:
            DEBUG_LOG("appSmHandleConnRulesHandsetPair, rule said pair with handset");
            appSmClearUserPairing();
            appSetState(APP_STATE_HANDSET_PAIRING);
            break;
        default:
            break;
    }
}

static void appSmHandleConnRulesEnterDfu(void)
{
    DEBUG_LOG("appSmHandleConnRulesEnterDfu");

    switch (appGetState())
    {
        case APP_STATE_IN_CASE_IDLE:
        case APP_STATE_IN_CASE_DFU:
            DEBUG_LOG("appSmHandleConnRulesEnterDfu, rule said enter DFU");
#ifdef INCLUDE_DFU
            appSmEnterDfuMode();
#endif
            break;

        default:
            break;
    }
    appConnRulesSetRuleComplete(CONN_RULES_ENTER_DFU);
}

static void appSmHandleConnRulesAllowHandsetConnect(void)
{
    DEBUG_LOG("appSmHandleConnRulesAllowHandsetConnect");

    appConManagerAllowHandsetConnect(TRUE);
    appConnRulesSetRuleComplete(CONN_RULES_ALLOW_HANDSET_CONNECT);
}

static void appSmHandleConnRulesRejectHandsetConnect(void)
{
    DEBUG_LOG("appSmHandleConnRulesRejectHandsetConnect");

    appConManagerAllowHandsetConnect(FALSE);
    appConnRulesSetRuleComplete(CONN_RULES_REJECT_HANDSET_CONNECT);
}

/*! \brief Handle rules engine action message to send peer sync.
 */
static void appSmHandleConnRulesPeerSync(void)
{
    DEBUG_LOG("appSmHandleConnRulesPeerSync");

    /* Send peer sync */
    appPeerSyncSend(FALSE);

    /* In all cases mark rule as done */
    appConnRulesSetRuleComplete(CONN_RULES_SEND_PEER_SYNC);
}

static void appSmHandleConnRulesForwardLinkKeys(void)
{
    bdaddr peer_addr;

    DEBUG_LOG("appSmHandleConnRulesForwardLinkKeys");

    /* Can only send this if we have a peer earbud */
    if (appDeviceGetPeerBdAddr(&peer_addr))
    {
        /* Attempt to send handset link keys to peer device */
        appPairingTransmitHandsetLinkKeysReq();
    }
    else
    {
        /* We should never attempt to send link keys message if there's no peer device */
    }

    /* In all cases mark rule as done */
    appConnRulesSetRuleComplete(CONN_RULES_PEER_SEND_LINK_KEYS);
}


/*! \brief Connect HFP, A2DP and AVRCP to last connected handset. */
static void appSmHandleConnRulesConnectHandset(CONN_RULES_CONNECT_HANDSET_T* crch)
{
    DEBUG_LOGF("appSmHandleConnRulesConnectHandset profiles %u PeerRIP %u", crch->profiles, appPeerSyncPeerRulesInProgress());
    /*! need to send a peer sync to notify peer that we're executing a rule action
     * for which in-progress matters */
    appPeerSyncSend(FALSE);

    if (crch->profiles & DEVICE_PROFILE_HFP)
    {
        /* Connect HFP to handset */
        appHfpConnectHandset();
    }

    if (crch->profiles & DEVICE_PROFILE_A2DP)
    {
        /* Connect AVRCP and A2DP to handset */
        appAvConnectHandset(crch->action == RULE_POST_HANDSET_CONNECT_ACTION_PLAY_MEDIA);
    }
}


/*! \brief Connect A2DP and AVRCP to peer. */
static void appSmHandleConnRulesConnectPeer(const CONN_RULES_CONNECT_PEER_T* crcp)
{
    DEBUG_LOGF("appSmHandleConnRulesConnectPeer profiles %u", crcp->profiles);

    if ((crcp->profiles & DEVICE_PROFILE_SCOFWD))
    {
        /* Connect SCO Forwarding link to peer */
        appScoFwdConnectPeer();
    }

    if (crcp->profiles & DEVICE_PROFILE_A2DP)
    {
        /* Connect AVRCP and A2DP to peer */
        appAvConnectPeer();
    }
    /* Mark rule as done */
    appConnRulesSetRuleComplete(CONN_RULES_CONNECT_PEER);
}


/*! \brief Connect HFP, A2DP and AVRCP to peer's connected handset. */
static void appSmHandleConnRulesConnectPeerHandset(CONN_RULES_CONNECT_PEER_HANDSET_T* crcph)
{
    bdaddr peer_handset_addr;

    DEBUG_LOGF("appSmHandleConnRulesConnectPeerHandset profiles:%u", crcph->profiles);

    if (crcph->profiles & DEVICE_PROFILE_A2DP)
    {
        /* Connect to peer's handset for A2DP (and AVRCP) */
        appPeerSyncGetPeerHandsetAddr(&peer_handset_addr);
        appAvConnectWithBdAddr(&peer_handset_addr);
    }
    if (crcph->profiles & DEVICE_PROFILE_HFP)
    {
        /* connecto to peer's handset for HFP */
        appPeerSyncGetPeerHandsetAddr(&peer_handset_addr);
        appHfpConnectWithBdAddr(&peer_handset_addr, hfp_handsfree_107_profile);
    }

    /* Mark rule as done */
    appConnRulesSetRuleComplete(CONN_RULES_CONNECT_PEER_HANDSET);
}

/*! \brief Connect HFP, A2DP and AVRCP to peer's connected handset. */
static void appSmHandleConnRulesUpdateMruPeerHandset(void)
{
    bdaddr peer_handset_addr;
    DEBUG_LOG("appSmHandleConnRulesUpdateMruPeerHandset");

    /* Update most recent connected device */
    appPeerSyncGetPeerHandsetAddr(&peer_handset_addr);
    appDeviceUpdateMruDevice(&peer_handset_addr);

    /* Mark rule as done */
    appConnRulesSetRuleComplete(CONN_RULES_UPDATE_MRU_PEER_HANDSET);

}

/*! \brief Send Earbud state and role messages to handset. */
static void appSmHandleConnRulesSendStatusToHandset(void)
{
    DEBUG_LOG("appSmHandleConnRulesSendStatusToHandset");

    /* request handset signalling module send current state to handset. */
    appHandsetSigSendEarbudStateReq(appSmGetPhyState());

    /* request handset signalling module sends current role to handset. */
    appHandsetSigSendEarbudRoleReq(appConfigIsLeft() ? EARBUD_ROLE_LEFT : EARBUD_ROLE_RIGHT);

    /* Mark rule as done */
    appConnRulesSetRuleComplete(CONN_RULES_SEND_STATE_TO_HANDSET);
}

/*! \brief Start timer to stop A2DP if earbud stays out of the ear. */
static void appSmHandleConnRulesA2dpTimeout(void)
{
    DEBUG_LOG("appSmHandleConnRulesA2dpTimeout");

    if (appSmIsOutOfCase() && appConfigOutOfEarA2dpTimeoutSecs())
    {
        DEBUG_LOGF("appSmHandleConnRulesA2dpTimeout, out of case/ear, so starting %u second timer", appConfigOutOfEarA2dpTimeoutSecs());
        MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_OUT_OF_EAR_A2DP,
                         NULL, D_SEC(appConfigOutOfEarA2dpTimeoutSecs()));
    }

    appConnRulesSetRuleComplete(CONN_RULES_A2DP_TIMEOUT);
}

/*! \brief Start timer to transfer SCO to AG if earbud stays out of the ear. */
static void appSmHandleConnRulesScoTimeout(void)
{
    DEBUG_LOG("appSmHandleConnRulesA2dpTimeout");

    if (appSmIsOutOfCase() && appConfigOutOfEarScoTimeoutSecs())
    {
        DEBUG_LOGF("appSmHandleConnRulesScoTimeout, out of case/ear, so starting %u second timer", appConfigOutOfEarScoTimeoutSecs());
        MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_OUT_OF_EAR_SCO,
                         NULL, D_SEC(appConfigOutOfEarScoTimeoutSecs()));
    }

    appConnRulesSetRuleComplete(CONN_RULES_SCO_TIMEOUT);
}

/*! \brief Transfer SCO from AG as earbud is in the ear. */
static void appSmHandleConnRulesScoTransferToEarbud(void)
{
    DEBUG_LOGF("appSmHandleConnRulesScoTransferToEarbud, in ear transferring audio this %u peer %u", appSmIsInEar(), appPeerSyncIsPeerInEar());
    appHfpTransferToHeadset();
    appConnRulesSetRuleComplete(CONN_RULES_SCO_TRANSFER_TO_EARBUD);
}

static void appSmHandleConnRulesScoTransferToHandset(void)
{
    DEBUG_LOG("appSmHandleConnRulesScoTransferToHandset");
    appHfpTransferToAg();
    appConnRulesSetRuleComplete(CONN_RULES_SCO_TRANSFER_TO_HANDSET);
}

/*! \brief Turns LEDs on */
static void appSmHandleConnRulesLedEnable(void)
{
    DEBUG_LOG("appSmHandleConnRulesLedEnable");

    appLedEnable(TRUE);
    appConnRulesSetRuleComplete(CONN_RULES_LED_ENABLE);
}

/*! \brief Turns LEDs off */
static void appSmHandleConnRulesLedDisable(void)
{
    DEBUG_LOG("appSmHandleConnRulesLedDisable");

    appLedEnable(FALSE);
    appConnRulesSetRuleComplete(CONN_RULES_LED_DISABLE);
}

/*! \brief Pause A2DP as earbud is out of the ear for too long. */
static void appSmHandleInternalTimeoutOutOfEarA2dp(void)
{
    DEBUG_LOG("appSmHandleInternalTimeoutOutOfEarA2dp out of ear pausing audio");

    /* Double check we're still out of case when the timeout occurs */
    if (appSmIsOutOfCase())
    {
        /* request the handset pauses the AV, indicate not a UI initiated
         * request with FALSE */
        appAvPause(FALSE);

        /* start the timer to autorestart the audio if earbud is placed
         * back in the ear. */
        if (appConfigInEarA2dpStartTimeoutSecs())
        {
            MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_IN_EAR_A2DP_START,
                             NULL, D_SEC(appConfigInEarA2dpStartTimeoutSecs()));
        }
    }
}

/*! \brief Nothing to do. */
static void appSmHandleInternalTimeoutInEarA2dpStart(void)
{
    DEBUG_LOG("appSmHandleInternalTimeoutInEarA2dpStart");

    /* no action needed, we're just using the presence/absence of an active
     * timer when going into InEar to determine if we need to restart audio */
}

/*! \brief Transfer SCO to AG as earbud is out of the ear for too long. */
static void appSmHandleInternalTimeoutOutOfEarSco(void)
{
    DEBUG_LOG("appSmHandleInternalTimeoutOutOfEarSco out of ear transferring audio");

    /* If we're forwarding SCO, check peer state as transferring SCO on master will
     * also remove SCO on slave */
    if (appScoFwdIsSending())
    {
        /* Double check we're still out of case and peer hasn't gone back in the
         * ear when the timeout occurs */
        if (appSmIsOutOfCase() && !appPeerSyncIsPeerInEar())
        {
            /* Transfer SCO to handset */
            appHfpTransferToAg();
        }
    }
    else
    {
        /* Double check we're still out of case */
        if (appSmIsOutOfCase())
        {
            /* Transfer SCO to handset */
            appHfpTransferToAg();
        }
    }
}


/*! \brief Earbud put in case disconnect link to handset */
static void appSmHandleConnRulesHandsetDisconnect(CONN_RULES_DISCONNECT_HANDSET_T *action)
{
    DEBUG_LOG("appSmHandleConnRulesHandsetDisconnect");

    smPostDisconnectAction post_disconnect_action = POST_DISCONNECT_ACTION_NONE;
    if (action && action->handover)
    {
        bool play_media;
        switch (appAvPlayStatus())
        {
            case avrcp_play_status_playing:
            case avrcp_play_status_fwd_seek:
            case avrcp_play_status_rev_seek:
                play_media = TRUE;
                break;
            default:
                play_media = FALSE;
                break;
        }
        post_disconnect_action = play_media ? POST_DISCONNECT_ACTION_HANDOVER_AND_PLAY_MEDIA
                                            : POST_DISCONNECT_ACTION_HANDOVER;
    }
    appSmInitiateLinkDisconnection(SM_DISCONNECT_HANDSET,
                                   appConfigLinkDisconnectionTimeoutTerminatingMs(),
                                   post_disconnect_action);
    appSetState(appSetSubState(APP_SUBSTATE_DISCONNECTING));
    appConnRulesSetRuleComplete(CONN_RULES_DISCONNECT_HANDSET);
}


/*! \brief Earbud put in case disconnect link to peer */
static void appSmHandleConnRulesPeerDisconnect(void)
{
    DEBUG_LOG("appSmHandleConnRulesPeerDisconnect");

    if (appDeviceIsPeerScoFwdConnected())
    {
        DEBUG_LOG("appSmHandleConnRulesPeerDisconnect, disconnecting peer SCOFWD");
        appScoFwdDisconnectPeer();
    }

    if (appDeviceIsPeerA2dpConnected() || appDeviceIsPeerAvrcpConnectedForAv())
    {
        DEBUG_LOG("appSmHandleConnRulesPeerDisconnect, disconnecting peer AV");
        appAvDisconnectPeer();
    }

    appConnRulesSetRuleComplete(CONN_RULES_DISCONNECT_PEER);
}

/*! \brief Let the SCO forwarding module know that we want to bring
        up the SCO forwarding link. */
static void appSmHandleConnRulesSendPeerScofwdConnect(void)
{
    bdaddr peer;

    DEBUG_LOG("appSmHandleConnRulesSendPeerScofwdConnect");

    if (appDeviceGetPeerBdAddr(&peer) && appDeviceIsHandsetHfpConnected() &&
        !appDeviceIsTwsPlusHandset(appHfpGetAgBdAddr()))
    {
        appScoFwdConnectPeer();
        /*! \todo Decide whether to mark the rule complete when we
            don't know if the link is up yet */
    }
    else
    {
        appConnRulesSetRuleComplete(CONN_RULES_SEND_PEER_SCOFWD_CONNECT);
    }
}

/*! \brief Switch which microphone is used during MIC forwarding. */
static void appSmHandleConnRulesSelectMic(CONN_RULES_SELECT_MIC_T* crsm)
{
    DEBUG_LOGF("appSmHandleConnRulesSelectMic selected mic %u", crsm->selected_mic);

    switch (crsm->selected_mic)
    {
        case MIC_SELECTION_LOCAL:
            appKymeraScoUseLocalMic();
            break;
        case MIC_SELECTION_REMOTE:
            appKymeraScoUseRemoteMic();
            break;
        default:
            break;
    }

    appConnRulesSetRuleComplete(CONN_RULES_SELECT_MIC);
}

static void appSmHandleConnRulesScoForwardingControl(CONN_RULES_SCO_FORWARDING_CONTROL_T* crsfc)
{
    DEBUG_LOGF("appSmHandleConnRulesScoForwardingControl forwarding %u", crsfc->forwarding_control);
    
    if (crsfc->forwarding_control)
    {
        /* enabled forwarding */
        appScoFwdEnableForwarding();
    }
    else
    {
        /* disable forwarding */
        appScoFwdDisableForwarding();
    }

    appConnRulesSetRuleComplete(CONN_RULES_SCO_FORWARDING_CONTROL);
}


/*! \brief Handle rule action to update page scan settings. */
static void appSmHandleBleConnectionUpdate(const CONN_RULES_BLE_CONNECTION_UPDATE_T* update)
{

    if (appGattAllowBleConnections(update->enable))
    {
        DEBUG_LOGF("appSmHandleBleConnectionUpdate enable:%u Kicking peer sync", update->enable);
        appPeerSyncSend(FALSE);
    }
    else
    {
        DEBUG_LOGF("appSmHandleBleConnectionUpdate enable:%u Unchanged, not kicking peer sync", update->enable);
    }
}


/*! \brief Idle timeout */
static void appSmHandleInternalTimeoutIdle(void)
{
    DEBUG_LOG("appSmHandleInternalTimeoutIdle");
    PanicFalse(APP_STATE_OUT_OF_CASE_IDLE == appGetState());
    appSetState(APP_STATE_OUT_OF_CASE_SOPORIFIC);
}

/*! \brief Handle rule action to update page scan settings. */
static void appSmHandleConnRulesPageScanUpdate(const CONN_RULES_PAGE_SCAN_UPDATE_T* crpsu)
{
    DEBUG_LOGF("appSmHandleConnRulesPageScanUpdate enable:%u", crpsu->enable);

    if (crpsu->enable)
        appScanManagerEnablePageScan(SCAN_MAN_USER_SM, SCAN_MAN_PARAMS_TYPE_SLOW);
    else
        appScanManagerDisablePageScan(SCAN_MAN_USER_SM);
}

/*! \brief Handle command to pair with a handset. */
static void appSmHandlePeerSigPairHandsetIndication(PEER_SIG_PAIR_HANDSET_IND_T *ind)
{
    smTaskData *sm = appGetSm();
    appDeviceAttributes attr;

    DEBUG_LOGF("appSmHandlePeerSigPairHandsetIndication, bdaddr %04x,%02x,%06lx",
               ind->handset_addr.nap, ind->handset_addr.uap, ind->handset_addr.lap);

    /* Check if we already know about this handset */
    if (appDeviceFindBdAddrAttributes(&ind->handset_addr, &attr))
    {
        /* Attempt to delete it, this will fail if we're connected */
        if (appDeviceDelete(&ind->handset_addr))
        {
            DEBUG_LOG("appSmHandlePeerSigPairHandsetIndication, known handset deleting and re-pairing");
            appPairingHandsetPairAddress(&sm->task, &ind->handset_addr, appConfigMaxHandsetPairingAttempts());
        }
        else
            DEBUG_LOG("appSmHandlePeerSigPairHandsetIndication, known handset, current connected so ignore pairing request");
    }
    else
    {
        DEBUG_LOG("appSmHandlePeerSigPairHandsetIndication, pairing with handset");
        appPairingHandsetPairAddress(&sm->task, &ind->handset_addr, appConfigMaxHandsetPairingAttempts());
    }
}

/*! \brief Handle command to connect to handset. */
static void appSmHandlePeerSigConnectHandsetIndication(PEER_SIG_CONNECT_HANDSET_IND_T *ind)
{
    /* Generate event that will run rules to connect to handset */
    connRulesEvents event = ind->play_media ? RULE_EVENT_HANDOVER_RECONNECT_AND_PLAY
                                            : RULE_EVENT_HANDOVER_RECONNECT;
    appConnRulesSetEvent(appGetSmTask(), event);
}

static void appSmHandleAvA2dpConnectedInd(const AV_A2DP_CONNECTED_IND_T *ind)
{
    DEBUG_LOG("appSmHandleAvA2dpConnectedInd");

    if (appDeviceIsHandset(&ind->bd_addr))
    {
        appConnRulesSetRuleComplete(CONN_RULES_CONNECT_HANDSET);
        appConnRulesResetEvent(RULE_EVENT_HANDSET_A2DP_DISCONNECTED);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_A2DP_CONNECTED);

        /* Record that we're connected with A2DP to handset */
        appDeviceSetA2dpWasConnected(&ind->bd_addr, TRUE);
    }
}

static void appSmHandleAvA2dpDisconnectedInd(const AV_A2DP_DISCONNECTED_IND_T *ind)
{
    DEBUG_LOGF("appSmHandleAvA2dpDisconnectedInd, reason %u inst %x", ind->reason, ind->av_instance);

    switch (appGetState())
    {
        default:
        {
            if (appDeviceIsHandset(&ind->bd_addr))
            {
                /* Clear connected and set disconnected events */
                appConnRulesResetEvent(RULE_EVENT_HANDSET_A2DP_CONNECTED);
                appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_A2DP_DISCONNECTED);

                /* If it was a normal disconnect and we're intentionally disconnecting
                   links, record that we're not connected with A2DP to handset */
                if (ind->reason == AV_A2DP_DISCONNECT_NORMAL && !appSmIsDisconnectingLinks())
                    appDeviceSetA2dpWasConnected(&ind->bd_addr, FALSE);
            }
        }
        break;
    }
}

static void appSmHandleAvAvrcpConnectedInd(const AV_AVRCP_CONNECTED_IND_T *ind)
{
    DEBUG_LOG("appSmHandleAvAvrcpConnectedInd");

    if (appDeviceIsHandset(&ind->bd_addr))
    {
        appConnRulesResetEvent(RULE_EVENT_HANDSET_AVRCP_DISCONNECTED);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_AVRCP_CONNECTED);
    }

    /* Check if connected to TWS+ device (handset or earbud) */
    if (appDeviceIsTwsPlusHandset(&ind->bd_addr) || appDeviceIsPeer(&ind->bd_addr))
    {
        DEBUG_LOG("appSmHandleAvAvrcpConnectedInd, using wallclock for UI synchronisation");

        /* Use wallclock on this connection to synchronise LEDs */
        appLedSetWallclock(ind->sink);
    }

    if (appDeviceIsPeer(&ind->bd_addr))
    {
        /* Use wallclock on this connection to synchronise RINGs */
        appScoFwdSetWallclock(ind->sink);
        appConnRulesResetEvent(RULE_EVENT_PEER_DISCONNECTED);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_CONNECTED);
    }
}

static void appSmHandleAvAvrcpDisconnectedInd(const AV_AVRCP_DISCONNECTED_IND_T *ind)
{
    DEBUG_LOG("appSmHandleAvAvrcpDisconnectedInd");

    switch (appGetState())
    {
        default:
        {
            if (appDeviceIsHandset(&ind->bd_addr))
            {
                appConnRulesResetEvent(RULE_EVENT_HANDSET_AVRCP_CONNECTED);
                appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_AVRCP_DISCONNECTED);
            }

            /* Check if disconnected from TWS+ device (handset or earbud) */
            if (appDeviceIsTwsPlusHandset(&ind->bd_addr) || appDeviceIsPeer(&ind->bd_addr))
            {
                /* Don't use wallclock */
                appLedSetWallclock(0);

            }

            if (appDeviceIsPeer(&ind->bd_addr))
            {
                appScoFwdSetWallclock(0);
                appConnRulesResetEvent(RULE_EVENT_PEER_CONNECTED);
                appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_DISCONNECTED);
            }
        }
        break;
    }
}

static void appSmHandleAvStreamingActiveInd(void)
{
    bdaddr handset_bd_addr;
    DEBUG_LOGF("appSmHandleAvStreamingActiveInd state=0x%x", appGetState());

    /* Have an active A2DP streaming, send a peer sync to inform
     * peer earbud. */
    if  (appDeviceGetHandsetBdAddr(&handset_bd_addr) && !appDeviceIsTwsPlusHandset(&handset_bd_addr))
        appPeerSyncSend(FALSE);

    /* We only care about this event if we're in a core state,
     * and it could be dangerous if we're not */
    if (appSmIsCoreState())
        appSmSetCoreState();
}

static void appSmHandleAvStreamingInactiveInd(void)
{
    DEBUG_LOGF("appSmHandleAvStreamingInactiveInd state=0x%x", appGetState());

    /* A2DP stream now inactive, send a peer sync to inform
     * peer earbud. */
    appPeerSyncSend(FALSE);

    /* We only care about this event if we're in a core state,
     * and it could be dangerous if we're not */
    if (appSmIsCoreState())
        appSmSetCoreState();
}

static void appSmHandleHfpConnectedInd(APP_HFP_CONNECTED_IND_T *ind)
{
    DEBUG_LOG("appSmHandleHfpConnectedInd");

    if (appDeviceIsHandset(&ind->bd_addr))
    {
        appConnRulesSetRuleComplete(CONN_RULES_CONNECT_HANDSET);
        appConnRulesResetEvent(RULE_EVENT_HANDSET_HFP_DISCONNECTED);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_HFP_CONNECTED);

        /* Record that we're connected with HFP to handset */
        appDeviceSetHfpWasConnected(&ind->bd_addr, TRUE);
    }
}

static void appSmHandleHfpDisconnectedInd(APP_HFP_DISCONNECTED_IND_T *ind)
{
    DEBUG_LOGF("appSmHandleHfpDisconnectedInd, reason %u", ind->reason);

    switch (appGetState())
    {
        default:
        {
            if (appDeviceIsHandset(&ind->bd_addr))
            {
                appConnRulesResetEvent(RULE_EVENT_HANDSET_HFP_CONNECTED);
                appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDSET_HFP_DISCONNECTED);

                /* If it was a normal disconnect and we're intentionally disconnecting
                   links, record that we're not connected with HFP to handset */
                if (ind->reason == APP_HFP_DISCONNECT_NORMAL && !appSmIsDisconnectingLinks())
                    appDeviceSetHfpWasConnected(&ind->bd_addr, FALSE);
            }
        }
        break;
    }
}

static void appSmHandleHfpScoConnectedInd(void)
{
    DEBUG_LOGF("appSmHandleHfpScoConnectedInd state=0x%x", appGetState());

    /* We only care about this event if we're in a core state,
     * and it could be dangerous if we're not */
    if (appSmIsCoreState())
        appSmSetCoreState();
}

static void appSmHandleHfpScoDisconnectedInd(void)
{
    DEBUG_LOGF("appSmHandleHfpScoDisconnectedInd state=0x%x", appGetState());

    /* We only care about this event if we're in a core state,
     * and it could be dangerous if we're not */
    if (appSmIsCoreState())
        appSmSetCoreState();
}

static void appSmHandleInternalPairHandset(void)
{
    if (appSmStateIsIdle(appGetState()))
    {
        appSmSetUserPairing();
        appSetState(APP_STATE_HANDSET_PAIRING);
    }
    else
        DEBUG_LOG("appSmHandleInternalPairHandset can only pair in IDLE state");
}

/*! \brief Delete pairing for all handsets.
    \note There must be no connections to a handset for this to succeed. */
static void appSmHandleInternalDeleteHandsets(void)
{
    DEBUG_LOG("appSmHandleInternalDeleteHandsets");

    switch (appGetState())
    {
        case APP_STATE_IN_CASE_IDLE:
        case APP_STATE_OUT_OF_CASE_IDLE:
        case APP_STATE_IN_EAR_IDLE:
        case APP_STATE_FACTORY_RESET:
        {
            bdaddr bd_addr;
            while (appDeviceGetHandsetBdAddr(&bd_addr))
            {
                if (!appDeviceDelete(&bd_addr))
                {
                    return;
                }
            }
            break;
        }
        default:
            DEBUG_LOGF("appSmHandleInternalDeleteHandsets bad state %u", 
                                                        appGetState());
            break;
    }
}

/*! \brief Handle request to start factory reset. */
static void appSmHandleInternalFactoryReset(void)
{
    if (appSmStateIsIdle(appGetState()))
        appSetState(APP_STATE_FACTORY_RESET);
    else
        DEBUG_LOG("appSmHandleInternalFactoryReset can only factory reset in IDLE state");
}

/*! \brief Handle failure to successfully disconnect links within timeout.
    Manually tear down the ACL and wait for #SM_INTERNAL_LINK_DISCONNECTION_COMPLETE.
*/
static void appSmHandleInternalLinkDisconnectionTimeout(void)
{
    bdaddr addr;

    DEBUG_LOG("appSmHandleInternalLinkDisconnectionTimeout");
    if (appSmDisconnectLockPeerIsDisconnecting())
    {
        if (appDeviceGetPeerBdAddr(&addr) && appConManagerIsConnected(&addr))
        {
            DEBUG_LOG("appSmHandleInternalLinkDisconnectionTimeout PEER IS STILL CONNECTED");
            appConManagerSendCloseAclRequest(&addr, TRUE);
        }
        else
        {
            appSmDisconnectLockClearLinks(SM_DISCONNECT_PEER);
        }
    }
    if (appSmDisconnectLockHandsetIsDisconnecting())
    {
        if (appDeviceGetHandsetBdAddr(&addr) && appConManagerIsConnected(&addr))
        {
            DEBUG_LOG("appSmHandleInternalLinkDisconnectionTimeout HANDSET IS STILL CONNECTED");
            appConManagerSendCloseAclRequest(&addr, TRUE);
        }
        else
        {
            appSmDisconnectLockClearLinks(SM_DISCONNECT_HANDSET);
        }
    }
}

#ifdef INCLUDE_DFU
/*! Change the state to DFU mode with an optional timeout

    \param timeoutMs Timeout (in ms) after which the DFU state
    may be exited
 */
static void appSmHandleEnterDfuWithTimeout(uint32 timeoutMs)
{
    MessageCancelAll(appGetSmTask(), SM_INTERNAL_TIMEOUT_DFU_ENTRY);
    if (timeoutMs)
    {
        MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_DFU_ENTRY, NULL, timeoutMs);
    }

    if (APP_STATE_IN_CASE_DFU != appGetState())
    {
        DEBUG_LOGF("appSmHandleEnterDfuWithTimeout. restarted SM_INTERNAL_TIMEOUT_DFU_ENTRY - %dms",timeoutMs);

        appSetState(APP_STATE_IN_CASE_DFU);
    }
    else
    {
        DEBUG_LOGF("appSmHandleEnterDfuWithTimeout. restarted SM_INTERNAL_TIMEOUT_DFU_ENTRY - %dms. Already in APP_STATE_IN_CASE_DFU",timeoutMs);
    }

    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_DFU_CONNECT);
}


static void appSmHandleDfuEnded(bool error)
{
    DEBUG_LOGF("appSmHandleDfuEnded(%d)",error);

    if (appGetState() == APP_STATE_IN_CASE_DFU)
    {
        appSetState(APP_STATE_STARTUP);
        if (error)
        {
            appGaiaDisconnect();
        }
    }
}


static void appSmHandleUpgradeConnected(void)
{
    switch (appGetState())
    {
        case APP_STATE_IN_CASE_DFU:
            /* If we are in the special DFU case mode then start a fresh time
               for dfu mode as well as a timer to trap for an application that 
               opens then closes the upgrade connection. */
            DEBUG_LOG("appSmHandleStartUpgrade, valid state to enter DFU");
            appSmStartDfuTimer();

            DEBUG_LOG("appSmHandleUpgradeConnected Start SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT %dms",
                                appConfigDfuTimeoutCheckForSupportMs());
            MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT, 
                                NULL, appConfigDfuTimeoutCheckForSupportMs());
            break;

        default:
            /* In all other states and modes we don't need to do anything.
               Start of an actual upgrade will be blocked if needed, 
               see appSmHandleDfuAllow() */
            break;
    }
}


static void appSmHandleUpgradeDisconnected(void)
{
    DEBUG_LOG("appSmHandleUpgradeDisconnected");

    if (MessageCancelAll(appGetSmTask(), SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT))
    {
        DEBUG_LOG("appSmHandleUpgradeDisconnected Cancelled SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT timer");

        appSmStartDfuTimer();
    }

}

#endif


static void appSmHandleNoDfu(void)
{

    if (appGetState() != APP_STATE_IN_CASE_DFU)
    {
        DEBUG_LOG("appSmHandleNoDfu. Not gone into DFU, so safe to assume we can continue");

        appSetState(APP_STATE_STARTUP);
    }
    else
    {
        DEBUG_LOG("appSmHandleNoDfu. We are in DFU mode !");
    }
}

/*! \brief Reboot the earbud, no questions asked. */
static void appSmHandleInternalReboot(void)
{
    BootSetMode(BootGetMode());
}

/*! \brief Handle indication all requested links are now disconnected. */
static void appSmHandleInternalAllRequestedLinksDisconnected(SM_INTERNAL_LINK_DISCONNECTION_COMPLETE_T *msg)
{
    DEBUG_LOGF("appSmHandleInternalAllRequestedLinksDisconnected 0x%x", appGetState());
    MessageCancelFirst(appGetSmTask(), SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION);
    bool play_media = FALSE;

    switch (msg->post_disconnect_action)
    {
        case POST_DISCONNECT_ACTION_HANDOVER_AND_PLAY_MEDIA:
            play_media = TRUE;
            // fallthrough
        case POST_DISCONNECT_ACTION_HANDOVER:
        {
            bdaddr peer_addr;
            if (appDeviceGetPeerBdAddr(&peer_addr))
            {
                appPeerSigTxConnectHandsetRequest(appGetSmTask(), &peer_addr, play_media);
            }
        }
        break;
        case POST_DISCONNECT_ACTION_HANDSET_PAIRING:
            appPairingHandsetPair(appGetSmTask(), appSmIsUserPairing());
        break;

        case POST_DISCONNECT_ACTION_NONE:
        default:
        break;
    }

    switch (appGetState())
    {
        case APP_STATE_OUT_OF_CASE_SOPORIFIC_TERMINATING:
            appPowerSleepPrepareResponse(appGetSmTask());
        break;
        case APP_STATE_FACTORY_RESET:
            appSmDeletePairingAndReset();
        break;
        default:
            switch (appGetSubState())
            {
                case APP_SUBSTATE_TERMINATING:
                    appPowerShutdownPrepareResponse(appGetSmTask());
                    break;
                case APP_SUBSTATE_DISCONNECTING:
                    appSmSetCoreState();
                    break;
                default:
                    break;
            }
        break;
    }
}


static void appSmHandlePeerSyncStatus(const PEER_SYNC_STATUS_T* status)
{
    UNUSED(status);
    DEBUG_LOGF("appSmHandlePeerSyncStatus %u", status->peer_sync_complete);

    if (appGetState() == APP_STATE_STARTUP)
    {
        appSmSetInitialCoreState();
    }
}


static void appSmHandleGattConnectable(void)
{
    appPeerSyncSend(FALSE);
}


static void appSmHandleGattConnection(void)
{
    appPeerSyncSend(FALSE);
}


static void appSmHandleDfuAllow(const CONN_RULES_DFU_ALLOW_T *dfu)
{
#ifdef INCLUDE_DFU
    appUpgradeAllowUpgrades(dfu->enable);
#else
    UNUSED(dfu);
#endif
}


static void appSmHandleInternalBredrConnected(void)
{
    DEBUG_LOG("appSmHandleInternalBredrConnected");

    /* Kick the state machine to see if this changes DFU state.
       A BREDR connection for upgrade can try an upgrade before we
       allow it. Always evaluate events do not need resetting. */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_CHECK_DFU);
}


/*! \brief Application state machine message handler. */
void appSmHandleMessage(Task task, MessageId id, Message message)
{
    smTaskData* sm = (smTaskData*)task;

    switch (id)
    {
        case INIT_CFM:
            appSmHandleInitConfirm();
            break;

        /* Pairing completion confirmations */
        case PAIRING_PEER_PAIR_CFM:
            appSmHandlePairingPeerPairConfirm((PAIRING_PEER_PAIR_CFM_T *)message);
            break;
        case PAIRING_HANDSET_PAIR_CFM:
            appSmHandlePairingHandsetPairConfirm((PAIRING_HANDSET_PAIR_CFM_T *)message);
            break;

        /* Connection manager status indications */
        case CON_MANAGER_CONNECTION_IND:
            appSmHandleConManagerConnectionInd((CON_MANAGER_CONNECTION_IND_T*)message);
            break;

        /* AV status change indications */
        case AV_A2DP_CONNECTED_IND:
            appSmHandleAvA2dpConnectedInd((AV_A2DP_CONNECTED_IND_T *)message);
            break;
        case AV_A2DP_DISCONNECTED_IND:
            appSmHandleAvA2dpDisconnectedInd((AV_A2DP_DISCONNECTED_IND_T *)message);
            break;
        case AV_AVRCP_CONNECTED_IND:
            appSmHandleAvAvrcpConnectedInd((AV_AVRCP_CONNECTED_IND_T *)message);
            break;
        case AV_AVRCP_DISCONNECTED_IND:
            appSmHandleAvAvrcpDisconnectedInd((AV_AVRCP_DISCONNECTED_IND_T *)message);
            break;
        case AV_STREAMING_ACTIVE_IND:
            appSmHandleAvStreamingActiveInd();
            break;
        case AV_STREAMING_INACTIVE_IND:
            appSmHandleAvStreamingInactiveInd();
            break;

        /* HFP status change indications */
        case APP_HFP_CONNECTED_IND:
            appSmHandleHfpConnectedInd((APP_HFP_CONNECTED_IND_T *)message);
            break;
        case APP_HFP_DISCONNECTED_IND:
            appSmHandleHfpDisconnectedInd((APP_HFP_DISCONNECTED_IND_T *)message);
            break;
        case APP_HFP_SCO_CONNECTED_IND:
            appSmHandleHfpScoConnectedInd();
            break;
        case APP_HFP_SCO_DISCONNECTED_IND:
            appSmHandleHfpScoDisconnectedInd();
            break;

        /* Physical state changes */
        case PHY_STATE_CHANGED_IND:
            appSmHandlePhyStateChangedInd(sm, (PHY_STATE_CHANGED_IND_T*)message);
            break;

        /* Power indications */
        case APP_POWER_SLEEP_PREPARE_IND:
            appSmHandlePowerSleepPrepareInd();
            break;
        case APP_POWER_SLEEP_CANCELLED_IND:
            appSmHandlePowerSleepCancelledInd();
            break;
        case APP_POWER_SHUTDOWN_PREPARE_IND:
            appSmHandlePowerShutdownPrepareInd();
            break;
        case APP_POWER_SHUTDOWN_CANCELLED_IND:
            appSmHandlePowerShutdownCancelledInd();
            break;

        /* Connection rules */
        case CONN_RULES_PEER_PAIR:
            appSmHandleConnRulesPeerPair();
            break;
        case CONN_RULES_SEND_PEER_SYNC:
            appSmHandleConnRulesPeerSync();
            break;
        case CONN_RULES_PEER_SEND_LINK_KEYS:
            appSmHandleConnRulesForwardLinkKeys();
            break;
        case CONN_RULES_CONNECT_HANDSET:
            appSmHandleConnRulesConnectHandset((CONN_RULES_CONNECT_HANDSET_T*)message);
            break;
        case CONN_RULES_CONNECT_PEER:
            appSmHandleConnRulesConnectPeer((CONN_RULES_CONNECT_PEER_T*)message);
            break;
        case CONN_RULES_CONNECT_PEER_HANDSET:
            appSmHandleConnRulesConnectPeerHandset((CONN_RULES_CONNECT_PEER_HANDSET_T*)message);
            break;
        case CONN_RULES_UPDATE_MRU_PEER_HANDSET:
            appSmHandleConnRulesUpdateMruPeerHandset();
            break;
        case CONN_RULES_SEND_STATE_TO_HANDSET:
            appSmHandleConnRulesSendStatusToHandset();
            break;
        case CONN_RULES_A2DP_TIMEOUT:
            appSmHandleConnRulesA2dpTimeout();
            break;
        case CONN_RULES_SCO_TIMEOUT:
            appSmHandleConnRulesScoTimeout();
            break;
        case CONN_RULES_SCO_TRANSFER_TO_EARBUD:
            appSmHandleConnRulesScoTransferToEarbud();
            break;
        case CONN_RULES_SCO_TRANSFER_TO_HANDSET:
            appSmHandleConnRulesScoTransferToHandset();
            break;
        case CONN_RULES_LED_ENABLE:
            appSmHandleConnRulesLedEnable();
            break;
        case CONN_RULES_LED_DISABLE:
            appSmHandleConnRulesLedDisable();
            break;
        case CONN_RULES_DISCONNECT_HANDSET:
            appSmHandleConnRulesHandsetDisconnect((CONN_RULES_DISCONNECT_HANDSET_T*)message);
            break;
        case CONN_RULES_DISCONNECT_PEER:
            appSmHandleConnRulesPeerDisconnect();
            break;
        case CONN_RULES_HANDSET_PAIR:
            appSmHandleConnRulesHandsetPair();
            break;
        case CONN_RULES_ENTER_DFU:
            appSmHandleConnRulesEnterDfu();
            break;
        case CONN_RULES_ALLOW_HANDSET_CONNECT:
            appSmHandleConnRulesAllowHandsetConnect();
            break;
        case CONN_RULES_REJECT_HANDSET_CONNECT:
            appSmHandleConnRulesRejectHandsetConnect();
            break;
        case CONN_RULES_PAGE_SCAN_UPDATE:
            appSmHandleConnRulesPageScanUpdate((CONN_RULES_PAGE_SCAN_UPDATE_T*)message);
            break;
        case CONN_RULES_SEND_PEER_SCOFWD_CONNECT:
            appSmHandleConnRulesSendPeerScofwdConnect();
            break;
        case CONN_RULES_SELECT_MIC:
            appSmHandleConnRulesSelectMic((CONN_RULES_SELECT_MIC_T*)message);
            break;
        case CONN_RULES_SCO_FORWARDING_CONTROL:
            appSmHandleConnRulesScoForwardingControl((CONN_RULES_SCO_FORWARDING_CONTROL_T*)message);
            break;            
        case CONN_RULES_ANC_TUNING_START:
            appAncTuningEnable(1);
            break;
        case CONN_RULES_ANC_TUNING_STOP:
            appAncTuningEnable(0);
            break;


        case CONN_RULES_BLE_CONNECTION_UPDATE:
            appSmHandleBleConnectionUpdate((const CONN_RULES_BLE_CONNECTION_UPDATE_T *)message);
            break;

        case CONN_RULES_DFU_ALLOW:
            appSmHandleDfuAllow((const CONN_RULES_DFU_ALLOW_T*)message);
            break;

        /* Peer signalling messages */
        case PEER_SIG_PAIR_HANDSET_IND:
            appSmHandlePeerSigPairHandsetIndication((PEER_SIG_PAIR_HANDSET_IND_T*)message);
            break;
        case PEER_SIG_CONNECT_HANDSET_IND:
            appSmHandlePeerSigConnectHandsetIndication((PEER_SIG_CONNECT_HANDSET_IND_T*)message);
            break;
        case PEER_SIG_CONNECT_HANDSET_CFM:
            break;

        /* Peer sync messages */
        case PEER_SYNC_STATUS:
            appSmHandlePeerSyncStatus((PEER_SYNC_STATUS_T*)message);
            break;

#ifdef INCLUDE_DFU
        /* Messages from GAIA */
        case APP_GAIA_UPGRADE_ACTIVITY:
            /* We do not monitor activity at present.
               Might be a use to detect long upgrade stalls without a disconnect */
            break;

        case APP_GAIA_DISCONNECTED:
            appSmHandleDfuEnded(FALSE);
            break;

        case APP_GAIA_UPGRADE_CONNECTED:
            appSmHandleUpgradeConnected();
            break;

        case APP_GAIA_UPGRADE_DISCONNECTED:
            appSmHandleUpgradeDisconnected();
            break;

        /* Messages from UPGRADE */
        case APP_UPGRADE_REQUESTED_TO_CONFIRM:
            appSmEnterDfuOnStartup(TRUE);
            break;

        case APP_UPGRADE_REQUESTED_IN_PROGRESS:
            appSmEnterDfuOnStartup(TRUE);
            break;

        case APP_UPGRADE_ACTIVITY:
            /* We do not monitor activity at present.
               Might be a use to detect long upgrade stalls without a disconnect */
            break;

        case APP_UPGRADE_STARTED:
            appSmNotifyUpgradeStarted();
            break;

        case APP_UPGRADE_COMPLETED:
            appSmHandleDfuEnded(FALSE);
            break;
#endif /* INCLUDE_DFU */

        /* GATT messages */
        case APP_GATT_CONNECTION_MADE:
        case APP_GATT_CONNECTION_DROPPED:
            appSmHandleGattConnection();
            break;

        case APP_GATT_CONNECTABLE:
            appSmHandleGattConnectable();
            break;

        /* SM internal messages */
        case SM_INTERNAL_PAIR_HANDSET:
            appSmHandleInternalPairHandset();
            break;
        case SM_INTERNAL_DELETE_HANDSETS:
            appSmHandleInternalDeleteHandsets();
            break;
        case SM_INTERNAL_FACTORY_RESET:
            appSmHandleInternalFactoryReset();
            break;
        case SM_INTERNAL_TIMEOUT_LINK_DISCONNECTION:
            appSmHandleInternalLinkDisconnectionTimeout();
            break;

#ifdef INCLUDE_DFU
        case SM_INTERNAL_ENTER_DFU_UI:
            appSmHandleEnterDfuWithTimeout(appConfigDfuTimeoutAfterEnteringCaseMs());
            break;

        case SM_INTERNAL_ENTER_DFU_UPGRADED:
            appSmHandleEnterDfuWithTimeout(appConfigDfuTimeoutToStartAfterRestartMs());
            break;

        case SM_INTERNAL_ENTER_DFU_STARTUP:
            appSmHandleEnterDfuWithTimeout(appConfigDfuTimeoutToStartAfterRebootMs());
            break;

        case SM_INTERNAL_TIMEOUT_DFU_ENTRY:
            DEBUG_LOG("appSmHandleMessage SM_INTERNAL_TIMEOUT_DFU_ENTRY");

            appSmHandleDfuEnded(TRUE);
            break;

        case SM_INTERNAL_TIMEOUT_DFU_MODE_START:
            DEBUG_LOG("appSmHandleMessage SM_INTERNAL_TIMEOUT_DFU_MODE_START");

            appSmEnterDfuModeInCase(FALSE);
            break;

        case SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT:
            /* No action needed for this timer */
            DEBUG_LOG("appSmHandleMessage SM_INTERNAL_TIMEOUT_DFU_AWAIT_DISCONNECT");
            break;
#endif

        case SM_INTERNAL_NO_DFU:
            appSmHandleNoDfu();
            break;

        case SM_INTERNAL_TIMEOUT_OUT_OF_EAR_A2DP:
            appSmHandleInternalTimeoutOutOfEarA2dp();
            break;

        case SM_INTERNAL_TIMEOUT_IN_EAR_A2DP_START:
            appSmHandleInternalTimeoutInEarA2dpStart();
            break;

        case SM_INTERNAL_TIMEOUT_OUT_OF_EAR_SCO:
            appSmHandleInternalTimeoutOutOfEarSco();
            break;

        case SM_INTERNAL_TIMEOUT_IDLE:
            appSmHandleInternalTimeoutIdle();
            break;

        case SM_INTERNAL_SEND_PEER_SYNC:
            appPeerSyncSend(FALSE);
            break;

        case SM_INTERNAL_REBOOT:
            appSmHandleInternalReboot();
            break;

        case SM_INTERNAL_LINK_DISCONNECTION_COMPLETE:
            appSmHandleInternalAllRequestedLinksDisconnected((SM_INTERNAL_LINK_DISCONNECTION_COMPLETE_T *)message);
            break;

        case SM_INTERNAL_BREDR_CONNECTED:
            appSmHandleInternalBredrConnected();
            break;

        default:
            appHandleUnexpected(id);
            break;
    }
}

bool appSmIsInEar(void)
{
    smTaskData *sm = appGetSm();
    return sm->phy_state == PHY_STATE_IN_EAR;
}

bool appSmIsOutOfEar(void)
{
    smTaskData *sm = appGetSm();
    return (sm->phy_state >= PHY_STATE_IN_CASE) && (sm->phy_state <= PHY_STATE_OUT_OF_EAR_AT_REST);
}

bool appSmIsInCase(void)
{
    smTaskData *sm = appGetSm();
    return (sm->phy_state == PHY_STATE_IN_CASE) || (sm->phy_state == PHY_STATE_UNKNOWN);
}

bool appSmIsOutOfCase(void)
{
    smTaskData *sm = appGetSm();
    return (sm->phy_state >= PHY_STATE_OUT_OF_EAR) && (sm->phy_state <= PHY_STATE_IN_EAR);
}

bool appSmIsConnectable(void)
{
    return appSmStateIsConnectable(appGetState());
}

/*! \todo Need a mechanism to know that an upgrade is in progress. Initial check suggests that this is harder than you'd think. */
bool appSmIsDfuPending(void)
{
#ifdef INCLUDE_DFU
    return (   appGetSm()->enter_dfu_in_case
           || APP_STATE_IN_CASE_DFU == appGetState());
#else
    return FALSE;
#endif
}


void appSmPairHandset(void)
{
    smTaskData *sm = appGetSm();
    MessageSend(&sm->task, SM_INTERNAL_PAIR_HANDSET, NULL);
}

void appSmDeleteHandsets(void)
{
    smTaskData *sm = appGetSm();
    MessageSend(&sm->task, SM_INTERNAL_DELETE_HANDSETS, NULL);
}

#ifdef INCLUDE_DFU
void appSmEnterDfuMode(void)
{
    MessageSend(appGetSmTask(),SM_INTERNAL_ENTER_DFU_UI, NULL);
}

static void appSmStartDfuTimer(void)
{
    appSmHandleEnterDfuWithTimeout(appConfigDfuTimeoutAfterGaiaConnectionMs());
}

void appSmEnterDfuModeInCase(bool enable)
{
    MessageCancelAll(appGetSmTask(), SM_INTERNAL_TIMEOUT_DFU_MODE_START);
    appGetSm()->enter_dfu_in_case = enable;

    if (enable)
    {
        DEBUG_LOG("appSmEnterDfuModeInCase (re)start SM_INTERNAL_TIMEOUT_DFU_MODE_START %dms",
                            appConfigDfuTimeoutToPlaceInCaseMs());
        MessageSendLater(appGetSmTask(), SM_INTERNAL_TIMEOUT_DFU_MODE_START, 
                            NULL, appConfigDfuTimeoutToPlaceInCaseMs());
    }
}


/*! \brief Tell the state machine to enter DFU mode following a reboot.
    \param upgrade_reboot If TRUE, indicates that DFU triggered the reboot.
                          If FALSE, indicates the device was rebooted whilst
                          an upgrade was in progress.
*/
static void appSmEnterDfuOnStartup(bool upgrade_reboot)
{
    MessageSend(appGetSmTask(),
                upgrade_reboot ? SM_INTERNAL_ENTER_DFU_UPGRADED
                               : SM_INTERNAL_ENTER_DFU_STARTUP,
                NULL);
}


/*! \brief Notification to the state machine of Upgrade start */
static void appSmNotifyUpgradeStarted(void)
{
    appSmCancelDfuTimers();
}
#endif /* INCLUDE_DFU */


/*! \brief Initialise the main application state machine.
 */
void appSmInit(void)
{
    smTaskData* sm = appGetSm();

    memset(sm, 0, sizeof(*sm));
    sm->task.handler = appSmHandleMessage;
    sm->state = APP_STATE_NULL;
    sm->phy_state = appPhyStateGetState();

    /* register with connection manager to get notification of (dis)connections */
    appConManagerRegisterConnectionsClient(&sm->task);

    /* register with HFP for changes in state */
    appHfpStatusClientRegister(&sm->task);

    /* register with AV to receive notifications of A2DP and AVRCP activity */
    appAvStatusClientRegister(&sm->task);

    /* register with peer signalling to receive handset commands */
    appPeerSigHandsetCommandsTaskRegister(&sm->task);

    /* register with peer sync to receive PEER_SYNC_STATUS updates */
    appPeerSyncStatusClientRegister(&sm->task);

    //appPeerSigClientRegister(&sm->task);

    /* register with power to receive sleep/shutdown messages. */
    appPowerClientRegister(&sm->task);

    /* set 'always run' events.
       setting the event isn't required, but it sets the task of the SM
       as the one to send the rule action message to. */
    appConnRulesSetEvent(&sm->task, RULE_EVENT_PAGE_SCAN_UPDATE);

    appSetState(APP_STATE_INITIALISING);
}

void appSmConnectHandset(void)
{
    /* Generate event that will run rules to connect to handset */
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_USER_CONNECT);
}

/*! \brief Request a factory reset. */
void appSmFactoryReset(void)
{
    MessageSend(appGetSmTask(), SM_INTERNAL_FACTORY_RESET, NULL);
}

/*! \brief Reboot the earbud. */
extern void appSmReboot(void)
{
    /* Post internal message to action the reboot, this breaks the call
     * chain and ensures the test API will return and not break. */
    MessageSend(appGetSmTask(), SM_INTERNAL_REBOOT, NULL);
}

/*! \brief Start earbud handset handover */
extern void appSmInitiateHandover(void)
{
    appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_HANDOVER_DISCONNECT);
}


