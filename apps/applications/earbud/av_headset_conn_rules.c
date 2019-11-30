/*!
\copyright  Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_conn_rules.c
\brief	    Connection Rules Module
*/

#include "av_headset.h"
#include "av_headset_conn_rules.h"
#include "av_headset_device.h"
#include "av_headset_log.h"
#include "av_headset_av.h"
#include "av_headset_phy_state.h"

#include <bdaddr.h>
#include <panic.h>
#include <system_clock.h>

#pragma unitsuppress Unused

/*! \{
    Macros for diagnostic output that can be suppressed.
    Allows debug of the rules module at two levels. */
#define CONNRULES_LOG(x)       //DEBUG_LOG(x)
#define CONNRULES_LOGF(x, ...) //DEBUG_LOGF(x, __VA_ARGS__)

#define RULE_LOG(x)         DEBUG_LOG(x)
#define RULE_LOGF(x, ...)   DEBUG_LOGF(x, __VA_ARGS__)
/*! \} */

/* Enable logging of the time taken to run rules */
//#define CONNRULES_TIMING_LOG_ENABLED

/*! Macro to split a uint64 into 2 uint32 that the debug macro can handle. */
#define PRINT_ULL(x)   ((uint32)(((x) >> 32) & 0xFFFFFFFFUL)),((uint32)((x) & 0xFFFFFFFFUL))

/* Forward declaration for use in RULE_ACTION_RUN_PARAM macro below */
static ruleAction appConnRulesCopyRunParams(const void* param, size_t size_param);

/*! \brief Macro used by rules to return RUN action with parameters to return.
    Copies the parameters/data into conn_rules where the rules engine can uses
    it when building the action message.
*/
#define RULE_ACTION_RUN_PARAM(x)   appConnRulesCopyRunParams(&(x), sizeof(x))

/*! \brief Macro to set the status of a rule.
    Guards against setting the status of a rule that carrys the
    RULE_FLAG_ALWAYS_EVALUATE property.
*/
#define SET_RULE_STATUS(r, new_status) \
    { \
        if (r->flags == RULE_FLAG_ALWAYS_EVALUATE) \
        { DEBUG_LOG("Cannot set status of RULE_FLAG_ALWAYS_EVALUATE rule"); Panic(); } \
        else \
        { r->status = new_status; } \
    }

/*! \brief Flags to control rule calling.
*/
typedef enum
{
    RULE_FLAG_NO_FLAGS          = 0x00,

    /*! Always evaluate this rule on any event. */
    RULE_FLAG_ALWAYS_EVALUATE   = 0x01,

    /*! This rule should be considered when reporting
     * rules that are in progress. */
    RULE_FLAG_PROGRESS_MATTERS  = 0x02,
} ruleFlags;

/*! \brief Function pointer definition for a rule */
typedef ruleStatus (*ruleFunc)(void);

/*! \brief Definition of a rule entry. */
typedef struct
{
    /*! Events that trigger this rule */
    connRulesEvents events;

    /*! Current state of the rule. */
    ruleStatus status;

    ruleFlags flags;

    /*! Pointer to the function to evaluate the rule. */
    ruleFunc rule;

    /*! Message to send when rule determines action to be run. */
    MessageId message;
} ruleEntry;

/*! Macro to declare a function, based on the name of a rule */
#define DEFINE_RULE(name) \
    static ruleAction name(void)

/*! Macro used to create an entry in the rules table */
#define RULE(event, name, message) \
    { event, RULE_STATUS_NOT_DONE, RULE_FLAG_NO_FLAGS, name, message }

/*! Macro used to create an entry in the rules table */
#define RULE_ALWAYS(event, name, message) \
    { event, RULE_STATUS_NOT_DONE, RULE_FLAG_ALWAYS_EVALUATE, name, message }

/*! Macro used to create an entry in the rules table that requires additional
    flags. */
#define RULE_WITH_FLAGS(event, name, message, flags) \
    { event, RULE_STATUS_NOT_DONE, flags, name, message } 

/*! \{
    Rule function prototypes, so we can build the rule tables below. */
DEFINE_RULE(rulePeerPair);
DEFINE_RULE(ruleAutoHandsetPair);
DEFINE_RULE(rulePeerSync);
DEFINE_RULE(rulePeerSyncIfNotComplete);
DEFINE_RULE(ruleForwardLinkKeys);

DEFINE_RULE(ruleSyncConnectHandset);
DEFINE_RULE(ruleSyncConnectPeerHandset);
DEFINE_RULE(ruleSyncConnectPeer);
DEFINE_RULE(ruleSyncDisconnectPeer);
DEFINE_RULE(ruleSyncDisconnectHandset);
DEFINE_RULE(ruleDisconnectPeer);
DEFINE_RULE(rulePairingConnectPeerHandset);

DEFINE_RULE(ruleUserConnectHandset);
DEFINE_RULE(ruleUserConnectPeerHandset);
DEFINE_RULE(ruleUserConnectPeer);

DEFINE_RULE(ruleOutOfCaseConnectHandset);
DEFINE_RULE(ruleOutOfCaseConnectPeerHandset);
DEFINE_RULE(ruleOutOfCaseConnectPeer);

DEFINE_RULE(ruleLinkLossConnectHandset);
DEFINE_RULE(ruleLinkLossConnectPeerHandset);

DEFINE_RULE(ruleUpdateMruHandset);
DEFINE_RULE(ruleSendStatusToHandset);
DEFINE_RULE(ruleOutOfEarA2dpActive);
DEFINE_RULE(ruleOutOfEarScoActive);
DEFINE_RULE(ruleInEarScoTransferToEarbud);
DEFINE_RULE(ruleOutOfEarLedsEnable);
DEFINE_RULE(ruleInEarLedsDisable);
DEFINE_RULE(ruleInCaseDisconnectHandset);
DEFINE_RULE(ruleInCaseDisconnectPeer);
DEFINE_RULE(ruleInCaseEnterDfu);
DEFINE_RULE(ruleOutOfCaseAllowHandsetConnect);
DEFINE_RULE(ruleInCaseRejectHandsetConnect);

DEFINE_RULE(ruleDfuAllowHandsetConnect);
DEFINE_RULE(ruleCheckUpgradable);

DEFINE_RULE(rulePageScanUpdate);

DEFINE_RULE(ruleInCaseScoTransferToHandset);
DEFINE_RULE(ruleSelectMicrophone);
DEFINE_RULE(ruleScoForwardingControl);

DEFINE_RULE(ruleBothConnectedDisconnect);

DEFINE_RULE(rulePairingConnectTwsPlusA2dp);
DEFINE_RULE(rulePairingConnectTwsPlusHfp);

DEFINE_RULE(ruleHandoverDisconnectHandset);
DEFINE_RULE(ruleHandoverConnectHandset);
DEFINE_RULE(ruleHandoverConnectHandsetAndPlay);

DEFINE_RULE(ruleOutOfCaseAncTuning);
DEFINE_RULE(ruleInCaseAncTuning);

DEFINE_RULE(ruleBleConnectionUpdate);
/*! \} */

/*! \brief Set of rules to run on Earbud startup. */
ruleEntry appConnRules[] =
{
    /*! \{
        Rules that should always run on any event */
    RULE_WITH_FLAGS(RULE_EVENT_PAGE_SCAN_UPDATE,       rulePageScanUpdate,     CONN_RULES_PAGE_SCAN_UPDATE,      RULE_FLAG_ALWAYS_EVALUATE),
    RULE_WITH_FLAGS(RULE_EVENT_BLE_CONNECTABLE_CHANGE, ruleBleConnectionUpdate,CONN_RULES_BLE_CONNECTION_UPDATE, RULE_FLAG_ALWAYS_EVALUATE),
    RULE_WITH_FLAGS(RULE_EVENT_CHECK_DFU,              ruleCheckUpgradable,    CONN_RULES_DFU_ALLOW,             RULE_FLAG_ALWAYS_EVALUATE),
    /*! \} */
    /*! \{
        Startup (power on) rules */
    RULE(RULE_EVENT_STARTUP,                    rulePeerPair,               CONN_RULES_PEER_PAIR),
    RULE(RULE_EVENT_STARTUP,                    rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    /*! \} */

    RULE(RULE_EVENT_PEER_UPDATE_LINKKEYS,       ruleForwardLinkKeys,        CONN_RULES_PEER_SEND_LINK_KEYS),
    RULE(RULE_EVENT_PEER_CONNECTED,             ruleForwardLinkKeys,        CONN_RULES_PEER_SEND_LINK_KEYS),

    /*! \{
        Rules that are run when peer link-loss happens */
    RULE(RULE_EVENT_PEER_LINK_LOSS,             rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    /*! \} */

    /*! \{
        Rules that are run when handset connects */
    RULE(RULE_EVENT_HANDSET_A2DP_CONNECTED,     rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_HANDSET_A2DP_CONNECTED,     ruleSendStatusToHandset,    CONN_RULES_SEND_STATE_TO_HANDSET),
    RULE(RULE_EVENT_HANDSET_AVRCP_CONNECTED,    rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_HANDSET_AVRCP_CONNECTED,    ruleSendStatusToHandset,    CONN_RULES_SEND_STATE_TO_HANDSET),
    RULE(RULE_EVENT_HANDSET_HFP_CONNECTED,      rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_HANDSET_HFP_CONNECTED,      ruleSendStatusToHandset,    CONN_RULES_SEND_STATE_TO_HANDSET),
    RULE(RULE_EVENT_HANDSET_HFP_CONNECTED,      ruleSyncConnectPeer,        CONN_RULES_SEND_PEER_SCOFWD_CONNECT),

    RULE(RULE_EVENT_HANDSET_A2DP_CONNECTED,     ruleBothConnectedDisconnect,CONN_RULES_DISCONNECT_HANDSET),
    RULE(RULE_EVENT_HANDSET_AVRCP_CONNECTED,    ruleBothConnectedDisconnect,CONN_RULES_DISCONNECT_HANDSET),
    RULE(RULE_EVENT_HANDSET_HFP_CONNECTED,      ruleBothConnectedDisconnect,CONN_RULES_DISCONNECT_HANDSET),
    /*! \} */

    /*! \{
        Rules that are run when handset disconnects */
    RULE(RULE_EVENT_HANDSET_A2DP_DISCONNECTED,  rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_HANDSET_AVRCP_DISCONNECTED, rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_HANDSET_HFP_DISCONNECTED,   rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_HANDSET_A2DP_DISCONNECTED,  ruleDisconnectPeer,         CONN_RULES_DISCONNECT_PEER),
    RULE(RULE_EVENT_HANDSET_AVRCP_DISCONNECTED, ruleDisconnectPeer,         CONN_RULES_DISCONNECT_PEER),
    RULE(RULE_EVENT_HANDSET_HFP_DISCONNECTED,   ruleDisconnectPeer,         CONN_RULES_DISCONNECT_PEER),

    /*! \{
        Receive handset link-key from peer */
    RULE(RULE_EVENT_RX_HANDSET_LINKKEY,         rulePeerSync,               CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_RX_HANDSET_LINKKEY,         rulePairingConnectPeerHandset, CONN_RULES_CONNECT_HANDSET),
    /*! \} */

    /*! \{
        Rules that are run when peer synchronisation is successful */
    RULE(RULE_EVENT_PEER_SYNC_VALID,            ruleSyncConnectPeer,        CONN_RULES_CONNECT_PEER),
    RULE(RULE_EVENT_PEER_SYNC_VALID,            ruleUpdateMruHandset,       CONN_RULES_UPDATE_MRU_PEER_HANDSET),
    RULE(RULE_EVENT_PEER_SYNC_VALID,            ruleSyncDisconnectPeer,     CONN_RULES_DISCONNECT_PEER),
    RULE(RULE_EVENT_PEER_SYNC_VALID,            ruleSyncDisconnectHandset,  CONN_RULES_DISCONNECT_HANDSET),
    /*! \} */

    /*! \{
        Rules that are run when user has request a connect */
    RULE(RULE_EVENT_USER_CONNECT,               rulePeerSyncIfNotComplete,  CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_USER_CONNECT,               ruleUserConnectPeer,        CONN_RULES_CONNECT_PEER),
    RULE_WITH_FLAGS(RULE_EVENT_USER_CONNECT,    ruleUserConnectHandset,     CONN_RULES_CONNECT_HANDSET,     RULE_FLAG_PROGRESS_MATTERS),
    RULE(RULE_EVENT_USER_CONNECT,               ruleUserConnectPeerHandset, CONN_RULES_CONNECT_PEER_HANDSET),
    /*! \} */

    /*! \{
        Rules that are run when handset link-loss happens */
    RULE(RULE_EVENT_HANDSET_LINK_LOSS,          ruleLinkLossConnectHandset,     CONN_RULES_CONNECT_HANDSET),
    RULE(RULE_EVENT_HANDSET_LINK_LOSS,          ruleLinkLossConnectPeerHandset, CONN_RULES_CONNECT_PEER_HANDSET),
    /*! \} */

    /*! \{
        Rules that are run on physical state changes */
    RULE(RULE_EVENT_OUT_CASE,                   rulePeerSync,                       CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_OUT_CASE,                   ruleOutOfCaseAllowHandsetConnect,   CONN_RULES_ALLOW_HANDSET_CONNECT),
    RULE(RULE_EVENT_OUT_CASE,                   ruleAutoHandsetPair,                CONN_RULES_HANDSET_PAIR),
    RULE(RULE_EVENT_OUT_CASE,                   ruleOutOfCaseConnectPeer,           CONN_RULES_CONNECT_PEER),
    RULE_WITH_FLAGS(RULE_EVENT_OUT_CASE,        ruleOutOfCaseConnectHandset,        CONN_RULES_CONNECT_HANDSET, RULE_FLAG_PROGRESS_MATTERS),
    RULE(RULE_EVENT_OUT_CASE,                   ruleOutOfCaseAncTuning,             CONN_RULES_ANC_TUNING_STOP),

    RULE(RULE_EVENT_IN_CASE,                    rulePeerSync,                       CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_IN_CASE,                    ruleInCaseDisconnectHandset,        CONN_RULES_DISCONNECT_HANDSET),
    RULE(RULE_EVENT_IN_CASE,                    ruleInCaseDisconnectPeer,           CONN_RULES_DISCONNECT_PEER),
    RULE(RULE_EVENT_IN_CASE,                    ruleInCaseEnterDfu,                 CONN_RULES_ENTER_DFU),
    RULE(RULE_EVENT_IN_CASE,                    ruleInCaseRejectHandsetConnect,     CONN_RULES_REJECT_HANDSET_CONNECT),
    RULE(RULE_EVENT_IN_CASE,                    ruleInCaseAncTuning,                CONN_RULES_ANC_TUNING_START),

    RULE(RULE_EVENT_OUT_EAR,                    rulePeerSync,                       CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_OUT_EAR,                    ruleOutOfEarA2dpActive,             CONN_RULES_A2DP_TIMEOUT),
    RULE(RULE_EVENT_OUT_EAR,                    ruleOutOfEarScoActive,              CONN_RULES_SCO_TIMEOUT),
    RULE(RULE_EVENT_OUT_EAR,                    ruleSelectMicrophone,               CONN_RULES_SELECT_MIC),
    RULE(RULE_EVENT_OUT_EAR,                    ruleOutOfEarLedsEnable,             CONN_RULES_LED_ENABLE),

    RULE(RULE_EVENT_IN_EAR,                     rulePeerSync,                       CONN_RULES_SEND_PEER_SYNC),
    RULE(RULE_EVENT_IN_EAR,                     ruleInEarLedsDisable,               CONN_RULES_LED_DISABLE),
    RULE(RULE_EVENT_IN_EAR,                     ruleInEarScoTransferToEarbud,       CONN_RULES_SCO_TRANSFER_TO_EARBUD),
    RULE(RULE_EVENT_IN_EAR,                     ruleSelectMicrophone,               CONN_RULES_SELECT_MIC),
    /*! \} */

    /*! \{
        Rules that are run on peer state changes */
    RULE(RULE_EVENT_PEER_IN_EAR,                ruleInEarScoTransferToEarbud,       CONN_RULES_SCO_TRANSFER_TO_EARBUD),
    RULE(RULE_EVENT_PEER_IN_EAR,                ruleSelectMicrophone,               CONN_RULES_SELECT_MIC),
    RULE(RULE_EVENT_PEER_IN_EAR,                ruleScoForwardingControl,           CONN_RULES_SCO_FORWARDING_CONTROL),
    RULE(RULE_EVENT_PEER_OUT_EAR,               ruleOutOfEarScoActive,              CONN_RULES_SCO_TIMEOUT),
    RULE(RULE_EVENT_PEER_OUT_EAR,               ruleSelectMicrophone,               CONN_RULES_SELECT_MIC),
    RULE(RULE_EVENT_PEER_OUT_EAR,               ruleScoForwardingControl,           CONN_RULES_SCO_FORWARDING_CONTROL),
    RULE_WITH_FLAGS(RULE_EVENT_PEER_IN_CASE,    ruleSyncConnectHandset,             CONN_RULES_CONNECT_HANDSET, RULE_FLAG_PROGRESS_MATTERS),
    RULE(RULE_EVENT_PEER_IN_CASE,               ruleInCaseScoTransferToHandset,     CONN_RULES_SCO_TRANSFER_TO_HANDSET),
    RULE_WITH_FLAGS(RULE_EVENT_PEER_HANDSET_DISCONNECTED,  ruleSyncConnectHandset,  CONN_RULES_CONNECT_HANDSET, RULE_FLAG_PROGRESS_MATTERS),
    RULE(RULE_EVENT_PEER_HANDSET_CONNECTED,     ruleBothConnectedDisconnect,        CONN_RULES_DISCONNECT_HANDSET),

    RULE(RULE_EVENT_PEER_A2DP_SUPPORTED,        rulePairingConnectTwsPlusA2dp,      CONN_RULES_CONNECT_PEER_HANDSET),
    RULE(RULE_EVENT_PEER_HFP_SUPPORTED,         rulePairingConnectTwsPlusHfp,       CONN_RULES_CONNECT_PEER_HANDSET),
    /*! \} */

    RULE(RULE_EVENT_SCO_ACTIVE,                 ruleScoForwardingControl,           CONN_RULES_SCO_FORWARDING_CONTROL),
    RULE(RULE_EVENT_SCO_INACTIVE,               ruleInCaseDisconnectHandset,        CONN_RULES_DISCONNECT_HANDSET),

    RULE(RULE_EVENT_DFU_CONNECT,                ruleDfuAllowHandsetConnect,         CONN_RULES_ALLOW_HANDSET_CONNECT),
    RULE(RULE_EVENT_HANDOVER_DISCONNECT,        ruleHandoverDisconnectHandset,      CONN_RULES_DISCONNECT_HANDSET),
    RULE(RULE_EVENT_HANDOVER_RECONNECT,         ruleHandoverConnectHandset,         CONN_RULES_CONNECT_HANDSET),
    RULE(RULE_EVENT_HANDOVER_RECONNECT_AND_PLAY,ruleHandoverConnectHandsetAndPlay,  CONN_RULES_CONNECT_HANDSET),
};

/*! \brief Types of event that can cause connect rules to run. */
typedef enum
{
    RULE_CONNECT_USER,              /*!< User initiated connection */
    RULE_CONNECT_PAIRING,           /*!< Connect on startup */
    RULE_CONNECT_PEER_SYNC,         /*!< Peer sync complete initiated connection */
    RULE_CONNECT_OUT_OF_CASE,       /*!< Out of case initiated connection */
    RULE_CONNECT_LINK_LOSS,         /*!< Link loss recovery initiated connection */
    RULE_CONNECT_PEER_OUT_OF_CASE,  /*!< Peer out of case initiated connection */
} ruleConnectReason;

/*****************************************************************************
 * RULES FUNCTIONS
 *****************************************************************************/

/*! @brief Rule to determine if Earbud should start automatic peer pairing
    This rule determins if automatic peer pairing should start, it is triggered
    by the startup event.
    @startuml

    start
        if (IsPairedWithPeer()) then (no)
            :Start peer pairing;
            end
        else (yes)
            :Already paired;
            stop
    @enduml 
*/
static ruleAction rulePeerPair(void)
{
    if (!appDeviceGetPeerBdAddr(NULL))
    {
        RULE_LOG("ruleStartupPeerPaired, run");
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("ruleStartupPeerPaired, done");
        return RULE_ACTION_COMPLETE;
    }
}

/*! @brief Rule to determine if Earbud should start automatic handset pairing
    @startuml

    start
        if (IsInCase()) then (yes)
            :Earbud is in case, do nothing;
            end
        endif
        if (IsPairedWithHandset()) then (yes)
            :Already paired with handset, do nothing;
            end
        endif
        if (IsPeerSyncComplete()) then (no)
            :Not sync'ed with peer, defer;
            end
        endif
        if (IsPeerPairing()) then (yes)
            :Peer is already pairing, do nothing;
            end
        endif
        if (IsPeerPairWithHandset()) then (yes)
            :Peer is already paired with handset, do nothing;
            end
        endif
        if (IsPeerInCase()) then (yes)
            :Start pairing, peer is in case;
            stop
        endif

        :Both Earbuds are out of case;
        if (IsPeerLeftEarbud) then (yes)
            stop
        else (no)
            end
        endif
    @enduml 
*/
static ruleAction ruleAutoHandsetPair(void)
{
    /* NOTE: Ordering of these checks is important */

    if (appSmIsInCase())
    {
        RULE_LOG("ruleAutoHandsetPair, ignore, we're in the case");
        return RULE_ACTION_IGNORE;
    }

    if (appDeviceGetHandsetBdAddr(NULL))
    {
        RULE_LOG("ruleAutoHandsetPair, complete, already paired with handset");
        return RULE_ACTION_COMPLETE;
    }

    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleAutoHandsetPair, defer, not synced with peer");
        return RULE_ACTION_DEFER;
    }

    if (appPeerSyncIsPeerPairing())
    {
        RULE_LOG("ruleAutoHandsetPair, defer, peer is already in pairing mode");
        return RULE_ACTION_DEFER;
    }

    if (appSmIsPairing())
    {
        RULE_LOG("ruleAutoHandsetPair, ignore, already in pairing mode");
        return RULE_ACTION_IGNORE;
    }

    if (appPeerSyncHasPeerHandsetPairing())
    {
        RULE_LOG("ruleAutoHandsetPair, complete, peer is already paired with handset");
        return RULE_ACTION_COMPLETE;
    }

    if (appPeerSyncIsPeerInCase())
    {
        RULE_LOG("ruleAutoHandsetPair, run, no paired handset, we're out of case, peer is in case");
        return RULE_ACTION_RUN;
    }
    else
    {
        /* Both out of case, neither pairing or paired.  Left wins, right loses */
        if (appConfigIsLeft())
        {
            RULE_LOG("ruleAutoHandsetPair, run, no paired handset, we're out of case, peer is out of case, we're left earbud");
            return RULE_ACTION_RUN;
        }
        else
        {
            RULE_LOG("ruleAutoHandsetPair, ignore, no paired handset, we're out of case, peer is out of case, but we're right earbud");
            return RULE_ACTION_IGNORE;
        }
    }
}

/*! @brief Rule to determine if Earbud should attempt to synchronize with peer Earbud
    @startuml

    start
        if (IsPairedWithPeer() and !IsInCaseDfu()) then (yes)
            :Start peer sync;
            stop
        else (no)
            :Already paired;
            end
    @enduml 
*/
static ruleAction rulePeerSync(void)
{
    if (appDeviceGetPeerBdAddr(NULL) && !appSmIsInDfuMode())
    {
        RULE_LOGF("rulePeerSync, run (state x%x)",appGetState());
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("rulePeerSync, ignore as there's no peer - or in DFU");
        return RULE_ACTION_IGNORE;
    }
}

/*! @brief Rule to determine if Earbud should attempt to synchronize with peer Earbud
 * only if peer sync hasn't completed previously.
 */
static ruleAction rulePeerSyncIfNotComplete(void)
{
    if (appPeerSyncIsComplete())
        return RULE_ACTION_COMPLETE;
    else
        return rulePeerSync();
}

/*! @brief Rule to determine if Earbud should attempt to forward handset link-key to peer
    @startuml

    start
        if (IsPairedWithPeer()) then (yes)
            :Forward any link-keys to peer;
            stop
        else (no)
            :Not paired;
            end
    @enduml 
*/
static ruleAction ruleForwardLinkKeys(void)
{
    if (appDeviceGetPeerBdAddr(NULL))
    {
        RULE_LOG("ruleForwardLinkKeys, run");
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("ruleForwardLinkKeys, ignore as there's no peer");
        return RULE_ACTION_IGNORE;
    }
}

/*! @brief Sub-rule to determine if Earbud should connect based on battery level.

    @startuml

    start
        if (Battery Voltage > Peer Battery Voltage) then (yes)
            :Our battery voltage is higher, connect;
            end
        else
            if (Battery Voltage = Peer Battery Voltage) then (yes)
                if (IsLeftEarbud()) then (yes)
                    :Left takes preference;
                    end
                else (no)
                    :Right doesn't connect;
                    stop
                endif
            else
                :Our battery voltage is lower, don't connect;
                stop
        endif
    endif
    @enduml 
*/
static ruleAction ruleConnectBatteryVoltage(ruleConnectReason reason)
{
    uint16 battery_level, peer_battery_level;
    UNUSED(reason);

    appPeerSyncGetPeerBatteryLevel(&battery_level, &peer_battery_level);
    RULE_LOGF("ruleConnectBatteryVoltage, battery %u, peer battery %u", battery_level, peer_battery_level);

    if (battery_level > peer_battery_level)
    {
        RULE_LOG("ruleConnectBatteryVoltage, run as our battery is higher");
        return RULE_ACTION_RUN;
    }
    else if (battery_level == peer_battery_level)
    {
        if (appConfigIsLeft())
        {
            RULE_LOG("ruleConnectBatteryVoltage, equal, run as left earbud");
            return RULE_ACTION_RUN;
        }
        else
        {
            RULE_LOG("ruleConnectBatteryVoltage, equal, ignore as right earbud");
            return RULE_ACTION_IGNORE;
        }
    }
    else
    {
            RULE_LOG("ruleConnectBatteryVoltage, ignore as our battery is lower");
            return RULE_ACTION_IGNORE;
    }
}

/*! @brief Sub-rule to determine if Earbud should connect to standard handset
*/
static ruleAction ruleConnectHandsetStandard(ruleConnectReason reason)
{
    if ((reason == RULE_CONNECT_USER) || (reason == RULE_CONNECT_LINK_LOSS))
    {     
        RULE_LOG("ruleConnectHandsetStandard, run as standard handset and user requested connection/link-loss");
        return RULE_ACTION_RUN;
    }
    else if ((reason == RULE_CONNECT_PEER_SYNC) || (reason == RULE_CONNECT_OUT_OF_CASE))
    {
        /* Check if out of case */
        if (appSmIsOutOfCase())
        {
            /* Check if peer is in case */
            if (appPeerSyncIsPeerInCase())
            {
                RULE_LOG("ruleConnectHandsetStandard, run as standard handset and not in case but peer is in case");
                return RULE_ACTION_RUN;
            }
            else
            {
                if (appPeerSyncPeerRulesInProgress())
                {
                    RULE_LOG("ruleConnectHandsetStandard, ignore as peer is out of case but has rules in progress");
                    return RULE_ACTION_IGNORE;
                }

                /* Both out of case */

                if (!appPeerSyncHasPeerHandsetPairing())
                {
                    RULE_LOG("ruleConnectHandsetStandard, both out of case but peer isn't paired with handset");
                    return RULE_ACTION_RUN;
                }

                if (reason == RULE_CONNECT_OUT_OF_CASE /*|| reason == RULE_CONNECT_PEER_OUT_OF_CASE*/)
                {
                    RULE_LOG("ruleConnectHandsetStandard, calling ruleConnectBatteryVoltage() as standard handset and both out of case but peer not connected");
                    return ruleConnectBatteryVoltage(reason);
                }
                else
                {
                    RULE_LOG("ruleConnectHandsetStandard, ignore as standard handset and both out of case");
                    return RULE_ACTION_IGNORE;                    
                }
            }
        }
        else
        {
            RULE_LOG("ruleConnectHandsetStandard, ignore as in case");
            return RULE_ACTION_IGNORE;                                
        }
    }
    else
    {
        return ruleConnectBatteryVoltage(reason);
    }
}

/*! @brief Rule to determine if Earbud should connect to Handset
    @startuml

    start
    if (IsInCase()) then (yes)
        :Never connect when in case;
        end
    endif

    if (IsPeerSyncComplete()) then (yes)
        if (Not IsPairedWithHandset()) then (yes)
            :Not paired with handset, don't connect;
            end
        endif        
        if (IsHandsetA2dpConnected() and IsHandsetAvrcpConnected() and IsHandsetHfpConnected()) then (yes)
            :Already connected;
            end
        endif

        if (IsTwsPlusHandset()) then (yes)
            :Handset is a TWS+ device;
            if (WasConnected() or Reason is 'User', 'Start-up' or 'Out of Case') then (yes)
                if (not just paired) then (yes)
                    :Connect to handset;
                    end
                else
                    :Just paired, handset will connect to us;
                    stop
                endif
            else (no)
                :Wasn't connected before;
                stop
            endif
        else (no)
            if (IsPeerConnectedA2dp() or IsPeerConnectedAvrcp() or IsPeerConnectedHfp()) then (yes)
                :Peer already has profile(s) connected, don't connect;
                stop
            else (no)
                if (WasConnected() or Reason is 'User', 'Start-up' or 'Out of Case') then (yes)
                    :run RuleConnectHandsetStandard();
                    end
                else (no)
                    :Wasn't connected before;
                    stop
                endif
            endif
        endif
    else (no)
        :Not sync'ed with peer;
        if (IsPairedWithHandset() and IsHandsetTwsPlus() and WasConnected()) then (yes)
            :Connect to handset, it is TWS+ handset;
            stop
        else (no)
            :Don't connect, not TWS+ handset;
            end
        endif
    endif

    @enduml 
*/
static ruleAction ruleConnectHandset(ruleConnectReason reason,
                                     rulePostHandsetConnectAction post_connect_action)
{
    bdaddr handset_addr;
    connRulesTaskData *conn_rules = appGetConnRules();
    CONN_RULES_CONNECT_HANDSET_T action = {0, post_connect_action};

    RULE_LOGF("ruleConnectHandset, reason %u", reason);

    /* Don't attempt to connect if we're in the case */
    if (appSmIsInCase())
    {
        RULE_LOG("ruleConnectHandset, ignore as in case");
        return RULE_ACTION_IGNORE;
    }

    /* Don't attempt to connect if we're pairing */
    if (appSmIsPairing())
    {
        RULE_LOG("ruleConnectHandset, ignore as pairing");
        return RULE_ACTION_IGNORE;
    }

    /* Check we have sync'ed with peer */
    if (appPeerSyncIsComplete())
    {
        if (appPeerSyncPeerDfuInProgress())
        {
            RULE_LOG("ruleConnectHandset, ignore as peer has DFU in progress");
            return RULE_ACTION_IGNORE;
        }

        /* Don't attempt to connect if peer is pairing */
        if (appPeerSyncIsPeerPairing())
        {
            RULE_LOG("ruleConnectHandset, ignore as peer is pairing");
            return RULE_ACTION_IGNORE;
        }

        /* If we're not paired with handset then don't connect */
        if (!appDeviceGetHandsetBdAddr(&handset_addr))
        {
            RULE_LOG("ruleConnectHandset, ignore as not paired with handset");
            return RULE_ACTION_IGNORE;
        }

        /* If we're already connected to handset then don't connect */
        if (appDeviceIsHandsetA2dpConnected() && appDeviceIsHandsetAvrcpConnected() && appDeviceIsHandsetHfpConnected())
        {
            RULE_LOG("ruleConnectHandset, ignore as already connected to handset");
            return RULE_ACTION_IGNORE;
        }

        /* Peer is not connected to handset, so we should connect to our handset if it's a TWS+ handset or
           it's a standard handset and our battery level is higer */

        /* Check if TWS+ handset, if so just connect, otherwise compare battery levels
         * if we have higher battery level connect to handset */
        if (appDeviceIsTwsPlusHandset(&handset_addr))
        {
            /* this call will read persistent store, so just called once and re-use
             * results */
            action.profiles = appDeviceWasConnectedProfiles(&handset_addr);

            /* Always attempt to connect HFP and A2DP if user initiated connect, or out-of-case connect, or pairing connect */
            if ((reason == RULE_CONNECT_OUT_OF_CASE) || (reason == RULE_CONNECT_USER) || (reason == RULE_CONNECT_PAIRING))
                action.profiles |= DEVICE_PROFILE_HFP | DEVICE_PROFILE_A2DP;

            /* Check if device was connected before, or we connecting due to user request */
            if (action.profiles != 0 ||
                (reason == RULE_CONNECT_USER) ||
                (reason == RULE_CONNECT_PAIRING) ||
                (reason == RULE_CONNECT_OUT_OF_CASE))
            {
                if (conn_rules->allow_connect_after_pairing || !appDeviceHasJustPaired(&handset_addr))
                {
                    RULE_LOGF("ruleConnectHandset, run as TWS+ handset for profiles:%u", action.profiles);
                    return RULE_ACTION_RUN_PARAM(action);
                }
                else
                {
                    RULE_LOG("ruleConnectHandset, ignore as just paired with TWS+ handset");
                    return RULE_ACTION_IGNORE;
                }
            }
            else
            {
                RULE_LOG("ruleConnectHandset, ignore as TWS+ handset but wasn't connected before");
                return RULE_ACTION_IGNORE;
            }
        }
        else
        {
            /* If peer is connected to handset then we shouldn't connect using this rule, use ruleConnectPeerHandset. */
            if (appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetHfpConnected())
            {
                RULE_LOG("ruleConnectHandset, ignore as peer has already connected");
                return RULE_ACTION_IGNORE;
            }

            /* Check if device was connected before, or we connecting due to user request or startup */
            if (appDeviceWasConnected(&handset_addr) ||
                (reason == RULE_CONNECT_USER) ||
                (reason == RULE_CONNECT_PAIRING) ||
                (reason == RULE_CONNECT_OUT_OF_CASE))
            {
                action.profiles = appDeviceWasConnectedProfiles(&handset_addr);

                /* Always attempt to connect HFP and A2DP if user initiated connect, or out-of-case connect, or pairing connect */
                if ((reason == RULE_CONNECT_OUT_OF_CASE) || (reason == RULE_CONNECT_USER) || (reason == RULE_CONNECT_PAIRING))
                    action.profiles |= DEVICE_PROFILE_HFP | DEVICE_PROFILE_A2DP;

                RULE_LOG("ruleConnectHandset, calling ruleConnectHandsetStandard()");
                if (ruleConnectHandsetStandard(reason) == RULE_ACTION_RUN)
                {
                    RULE_LOG("ruleConnectHandset, run as standard handset we were connected to before");
                    return RULE_ACTION_RUN_PARAM(action);
                }
                else
                {
                    RULE_LOG("ruleConnectHandset, ignore, standard handset but not connected before");
                    return RULE_ACTION_IGNORE;
                }
            }
            else
            {
                RULE_LOG("ruleConnectHandset, ignore as standard handset but wasn't connected before");
                return RULE_ACTION_IGNORE;
            }
        }
    }
    else
    {
        /* Defer is peer sync is still in progress */
        if (appPeerSyncIsInProgress())
        {
            RULE_LOG("ruleConnectHandset, defer as peer sync in progress");
            return RULE_ACTION_DEFER;
        }

        uint8 profiles = 0;
        if (appDeviceGetHandsetBdAddr(&handset_addr) &&
            appDeviceIsTwsPlusHandset(&handset_addr) &&
            ((profiles = appDeviceWasConnectedProfiles(&handset_addr)) != 0))
        {
            /* Always attempt to connect HFP and A2DP if user initiated connect, or out-of-case connect, or pairing connect */
            if ((reason == RULE_CONNECT_OUT_OF_CASE) || (reason == RULE_CONNECT_USER) || (reason == RULE_CONNECT_PAIRING))
                profiles |= DEVICE_PROFILE_HFP | DEVICE_PROFILE_A2DP;

            RULE_LOG("ruleConnectHandset, run as TWS+ handset, as connected before, despite peer sync fail");
            return RULE_ACTION_RUN_PARAM(profiles);
        }
        else
        {
            RULE_LOG("ruleConnectHandset, defer as not sync'ed with peer");
            return RULE_ACTION_DEFER;
        }
    }
}

/*! @brief Wrapper around ruleConnectHandset() that calls it with 'Peer sync' connect reason
*/
static ruleAction ruleSyncConnectHandset(void)
{
    return ruleConnectHandset(RULE_CONNECT_PEER_SYNC, RULE_POST_HANDSET_CONNECT_ACTION_NONE);
}

/*! @brief Wrapper around ruleConnectHandset() that calls it with 'User' connect reason
*/
static ruleAction ruleUserConnectHandset(void)
{
    return ruleConnectHandset(RULE_CONNECT_USER, RULE_POST_HANDSET_CONNECT_ACTION_PLAY_MEDIA);
}

/*! @brief Wrapper around ruleConnectHandset() that calls it with 'Out of case' connect reason
*/
static ruleAction ruleOutOfCaseConnectHandset(void)
{
    RULE_LOG("ruleOutOfCaseConnectHandset");
    return ruleConnectHandset(RULE_CONNECT_OUT_OF_CASE, RULE_POST_HANDSET_CONNECT_ACTION_NONE);
}

/*! @brief Wrapper around ruleConnectHandset() that calls it with 'link loss' connect reason
*/
static ruleAction ruleLinkLossConnectHandset(void)
{
    RULE_LOG("ruleLinkLossConnectHandset");
    return ruleConnectHandset(RULE_CONNECT_LINK_LOSS, RULE_POST_HANDSET_CONNECT_ACTION_NONE);
}

/*! @brief Rule to determine if Earbud should connect to Handset even when synchronisation with peer Earbud failed
    @startuml

    start
    if (IsInCase()) then (yes)
        :Never connect when in case;
        stop
    endif

    if (Not IsPeerSyncComplete()) then (yes)
        :Not sync'ed with peer;
        if (IsPairedWithHandset() and Not IsHandsetConnected()) then (yes)
            if (IsTwsPlusHandset()) then (yes)
                :Connect to handset, it is TWS+ handset;
                stop
            else (no)
                :Don't connect, not TWS+ handset;
                end
            endif
        else (no)
            :Don't connected, not paired or already connected;
            end
        endif
    else
        :Do nothing as not sync'ed with peer Earbud;
        end
    endif
    @enduml
*/
static ruleAction ruleNoSyncConnectHandset(void)
{
    bdaddr handset_addr;

    /* Don't attempt to connect if we're in the case */
    if (appSmIsInCase())
    {
        RULE_LOG("ruleConnectHandset, ignore as nin case");
        return RULE_ACTION_IGNORE;
    }

    /* Check we haven't sync'ed with peer */
    if (!appPeerSyncIsComplete())
    {
        /* Not sync'ed with peer, so connect to handset if it's a TWS+ handset */

        /* Check we're paired with handset and not already connected */
        if (appDeviceGetHandsetBdAddr(&handset_addr) && !appDeviceIsHandsetConnected())
        {
            /* Check if TWS+ handset, if so just connect */
            if (appDeviceIsTwsPlusHandset(&handset_addr))
            {
                RULE_LOG("ruleConnectHandsetNoSync, run as not sync'ed but TWS+ handset");
                return RULE_ACTION_RUN;
            }
            else
            {
                RULE_LOG("ruleConnectHandsetNoSync, ignore as not sync'ed but standard handset");
                return RULE_ACTION_IGNORE;
            }
        }
        else
        {
            RULE_LOG("ruleConnectHandsetNoSync, ignore as not paired or already connected to handset");
            return RULE_ACTION_IGNORE;
        }
    }
    else
    {
        RULE_LOG("ruleConnectHandsetNoSync, ignore as sync'ed with peer");
        return RULE_ACTION_IGNORE;
    }
}

/*! @brief Rule to determine if Earbud should connect to peer's Handset
    @startuml

    start
    if (IsInCase()) then (yes)
        :Never connect when in case;
        stop
    endif

    if (Not IsPeerSyncComplete()) then (yes)
        :Not sync'ed with peer;
        end
    endif

    if (IsPeerHandsetA2dpConnected() or IsPeerHandsetAvrcpConnected() or IsPeerHandsetHfpConnected()) then (yes)
        if (IsPeerHandsetTws()) then (yes)
            if (IsPairedWithHandset())) then (yes)
                if (Not JustPaired()) then (yes)
                    if (Reason is 'User' or 'Start-up' or 'Out of case') then (yes)
                        :Connect to peer's handset;
                        stop
                    else (no)
                        :Don't connect to peer's handset;
                        end
                    endif
                else (no)
                    :Don't connect as just paired;
                    end
                endif
            else (no)
                :Not paired with peer's handset;
                end
            endif
        else (no)
            :Peer is connected to standard handset;
            end
        endif
    else (no)
        :Don't connect as peer is not connected to handset;
        end
    endif
    @enduml 
*/
static ruleAction ruleConnectPeerHandset(ruleConnectReason reason)
{
    connRulesTaskData *conn_rules = appGetConnRules();

    /* Don't attempt to connect if we're in the case */
    if (appSmIsInCase())
    {
        RULE_LOG("ruleConnectHandset, ignore as in case");
        return RULE_ACTION_IGNORE;
    }

    /* Don't attempt to connect if we're pairing */
    if (appSmIsPairing())
    {
        RULE_LOG("ruleConnectPeer, ignore as pairing");
        return RULE_ACTION_IGNORE;
    }

    /* Don't run rule if we haven't sync'ed with peer */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleConnectPeerHandset, defer as not sync'ed with peer");
        return RULE_ACTION_DEFER;
    }

    /* Don't attempt to connect if peer is pairing */
    if (appPeerSyncIsPeerPairing())
    {
        RULE_LOG("ruleConnectPeerHandset, ignore as peer is pairing");
        return RULE_ACTION_IGNORE;
    }

    /* If connecting due to pairing or peer is connected to handset then we should also connect to this handset if it's TWS+ */
    if ((reason == RULE_CONNECT_PAIRING) ||
        appPeerSyncIsPeerHandsetA2dpConnected() ||
        appPeerSyncIsPeerHandsetHfpConnected())
    {
        /*  Check peer's handset is TWS+ */
        if (appPeerSyncIsPeerHandsetTws())
        {
            bdaddr handset_addr;
            appPeerSyncGetPeerHandsetAddr(&handset_addr);

            /* Check we paired with this handset */
            if (appDeviceIsHandset(&handset_addr))
            {
                if (conn_rules->allow_connect_after_pairing || !appDeviceHasJustPaired(&handset_addr))
                {
                    if ((reason == RULE_CONNECT_USER) ||
                        (reason == RULE_CONNECT_PAIRING) ||
                        (reason == RULE_CONNECT_OUT_OF_CASE)
                        || (conn_rules->allow_connect_after_pairing && 
                            appDeviceHasJustPaired(&handset_addr))
                        )
                    {
                        uint8 profiles = 0;

                        if (appPeerSyncIsPeerHandsetA2dpConnected())
                            profiles |= DEVICE_PROFILE_A2DP;
                        if (appPeerSyncIsPeerHandsetHfpConnected())
                            profiles |= DEVICE_PROFILE_HFP;
                        if (appPeerSyncIsPeerHandsetAvrcpConnected())
                            profiles |= DEVICE_PROFILE_AVRCP;

                        /* Always attempt to connect HFP, A2DP and AVRCP if pairing connect */
                        if (reason == RULE_CONNECT_PAIRING)
                            profiles |= DEVICE_PROFILE_HFP | DEVICE_PROFILE_A2DP | DEVICE_PROFILE_AVRCP;

                        RULE_LOGF("ruleConnectPeerHandset, run as peer is connected to TWS+ handset, profiles:%u", profiles);

                        return RULE_ACTION_RUN_PARAM(profiles);
                    }
                    else
                    {
                        RULE_LOG("ruleConnectPeerHandset, ignore as peer is connected to TWS+ handset but not user or startup connect and not just paired (or allow_connect_after_pairing disabled)");
                        return RULE_ACTION_IGNORE;
                    }
                }
                else
                {
                    RULE_LOG("ruleConnectPeerHandset, ignore as just paired with peer's TWS+ handset or allow_connect_after_pairing disabled");
                    return RULE_ACTION_IGNORE;
                }
            }
            else
            {
                RULE_LOG("ruleConnectPeerHandset, ignore as peer is connected to TWS+ handset but we're not paired with it");
                return RULE_ACTION_IGNORE;
            }
        }
        else
        {
            RULE_LOG("ruleConnectPeerHandset, ignore as peer is connected to standard handset");
            return RULE_ACTION_IGNORE;
        }
    }
    else
    {
        /* Peer is not connected to handset, don't connect as ruleConnectHandset handles this case */
        RULE_LOG("ruleConnectPeerHandset, done as peer is not connected");
        return RULE_ACTION_COMPLETE;
    }
}

/*! @brief Wrapper around ruleSyncConnectPeerHandset() that calls it with 'Peer sync' connect reason
*/
static ruleAction ruleSyncConnectPeerHandset(void)
{
    return ruleConnectPeerHandset(RULE_CONNECT_PEER_SYNC);
}

/*! @brief Wrapper around ruleSyncConnectPeerHandset() that calls it with 'User' connect reason
*/
static ruleAction ruleUserConnectPeerHandset(void)
{
    return ruleConnectPeerHandset(RULE_CONNECT_USER);
}

/*! @brief Wrapper around ruleSyncConnectPeerHandset() that calls it with 'Out of case' connect reason
*/
static ruleAction ruleOutOfCaseConnectPeerHandset(void)
{
    RULE_LOG("ruleOutOfCaseConnectPeerHandset");
    return ruleConnectPeerHandset(RULE_CONNECT_OUT_OF_CASE);
}


/*! @brief Wrapper around ruleSyncConnectPeerHandset() that calls it with 'link loss' connect reason
*/
static ruleAction ruleLinkLossConnectPeerHandset(void)
{
    RULE_LOG("ruleLinkLossConnectPeerHandset");
    return ruleConnectPeerHandset(RULE_CONNECT_LINK_LOSS);
}

static ruleAction rulePairingConnectPeerHandset(void)
{
    RULE_LOG("rulePairConnectPeerHandset");
    return ruleConnectPeerHandset(RULE_CONNECT_PAIRING);
}

/*! @brief Rule to determine if Earbud should connect A2DP & AVRCP to peer Earbud
    @startuml

    start
    if (IsPeerA2dpConnected()) then (yes)
        :Already connected;
        end
    endif

    if (IsPeerSyncComplete()) then (yes)
        if (IsPeerInCase()) then (yes)
            :Peer is in case, so don't connect to it;
            end
        endif

        if (IsPeerHandsetA2dpConnected() or IsPeerHandsetHfpConnected()) then (yes)
            if (IsPeerHandsetTws()) then (yes)
                :Don't need to connect to peer, as peer is connected to TWS+ handset;
                end
            else (no)
                :Don't need to connect, peer will connect to us;
                end
            endif
        else (no)    
            :Peer is not connected to handset yet;
            if (IsPairedWithHandset()) then (yes)
                if (Not IsTwsHandset()) then (yes)
                    if (IsHandsetA2dpConnected() or IsHandsetHfpConnected()) then (yes)
                        :Connect to peer as  connected to standard handset, peer won't be connected;
                        stop
                    else (no)
                        :Run RuleConnectHandsetStandard() to determine if we're going to connect to handset;
                        if (RuleConnectHandsetStandard()) then (yes)
                            :Will connect to handset, so should also connect to peer;
                            stop
                        else (no)
                            :Won't connect to handset, so don't connect to peer;
                            end
                        endif
                    endif
                else (no)
                    :Don't connect to peer, as connected to TWS+ handset;
                    end
                endif
            else (no)
                :Don't connect to peer, as not paired with handset;
                end
            endif
        endif
    else (no)
        :Not sync'ed with peer;
        end
    endif

    @enduml 
*/
static ruleAction ruleConnectPeer(ruleConnectReason reason)
{
    bdaddr handset_addr;

    /* Don't run rule if we're connected to peer */
    if (appDeviceIsPeerA2dpConnected() && appDeviceIsPeerScoFwdConnected())
    {
        RULE_LOG("ruleConnectPeer, ignore as already connected to peer");
        return RULE_ACTION_IGNORE;
    }

    /* Don't attempt to connect if we're pairing */
    if (appSmIsPairing())
    {
        RULE_LOG("ruleConnectPeer, ignore as pairing");
        return RULE_ACTION_IGNORE;
    }

    /* Check we have sync'ed with peer */
    if (appPeerSyncIsComplete())
    {
        /* Check if peer is in case */
        if (appPeerSyncIsPeerInCase())
        {
            RULE_LOG("ruleConnectPeer, ignore as peer is in case");
            return RULE_ACTION_IGNORE;
        }

        /* Don't attempt to connect if peer is pairing */
        if (appPeerSyncIsPeerPairing())
        {
            RULE_LOG("ruleConnectPeer, ignore as peer is pairing");
            return RULE_ACTION_IGNORE;
        }

        /* Check if peer is connected to handset */
        if (appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetHfpConnected())
        {
            /* Don't connect to peer if handset is TWS+  */
            if (appPeerSyncIsPeerHandsetTws())
            {
                RULE_LOG("ruleConnectPeer, ignore as peer is connected to TWS+ handset");
                return RULE_ACTION_IGNORE;
            }
            else
            {
                RULE_LOG("ruleConnectPeer, ignore as peer is connected to standard handset and peer will connect to us");
                return RULE_ACTION_IGNORE;
            }
        }
        else
        {
            /* Peer is not connected to handset yet */
            /* Get handset address */
            if (appDeviceGetHandsetBdAddr(&handset_addr))
            {
                /* Check if the handset we would connect to is a standard handset */
                if (!appDeviceIsTwsPlusHandset(&handset_addr))
                {
                    bdaddr peer_addr;
                    uint8 profiles;
                    
                    appDeviceGetPeerBdAddr(&peer_addr);
                    profiles = appDeviceWasConnectedProfiles(&peer_addr);

                    /* Always attempt to connect A2DP and SCOFWD if user initiated connect,
                     * out-of-case connect or sync connect */
                    if ((reason == RULE_CONNECT_OUT_OF_CASE) || 
                        (reason == RULE_CONNECT_USER) ||
                        (reason == RULE_CONNECT_PEER_SYNC))
                    {
                        profiles |= DEVICE_PROFILE_A2DP | DEVICE_PROFILE_SCOFWD;
                    }

                    /* Check if we're already connected to handset */
                    if (appDeviceIsHandsetA2dpConnected() || appDeviceIsHandsetHfpConnected())
                    {
                        RULE_LOG("ruleConnectPeer, run as connected to standard handset, peer won't be connected");
                        return RULE_ACTION_RUN_PARAM(profiles);
                    }
                    else
                    {
                        /* Not connected to handset, if we are going to connect to standard handset, we should also connect to peer */
                        RULE_LOG("ruleConnectPeer, calling ruleConnectHandsetStandard() to determine if we're going to connect to handset");
                        if (ruleConnectHandsetStandard(reason) == RULE_ACTION_RUN)
                        {
                            RULE_LOG("ruleConnectPeer, run as connected/ing to standard handset");
                            return RULE_ACTION_RUN_PARAM(profiles);
                        }
                        else
                        {
                            RULE_LOG("ruleConnectPeer, ignore as not connected/ing to standard handset");
                            return RULE_ACTION_IGNORE;
                        }
                    }
                }
                else
                {
                    RULE_LOG("ruleConnectPeer, ignore as connected/ing to TWS+ handset");
                    return RULE_ACTION_IGNORE;
                }
            }
            else
            {
                RULE_LOG("ruleConnectPeer, ignore as no handset, so no need to connect to peer");
                return RULE_ACTION_IGNORE;
            }
        }
    }
    else
    {
        /* Peer sync is not complete */
        RULE_LOG("ruleConnectPeer, defer as peer sync not complete");
        return RULE_ACTION_DEFER;
    }
}

/*! @brief Wrapper around ruleConnectPeer() that calls it with 'Peer sync' connect reason
*/
static ruleAction ruleSyncConnectPeer(void)
{
    RULE_LOG("ruleSyncConnectPeer");
    return ruleConnectPeer(RULE_CONNECT_PEER_SYNC);
}

/*! @brief Wrapper around ruleConnectPeer() that calls it with 'User' connect reason
*/
static ruleAction ruleUserConnectPeer(void)
{
    RULE_LOG("ruleUserConnectPeer");
    return ruleConnectPeer(RULE_CONNECT_USER);
}

/*! @brief Wrapper around ruleConnectPeer() that calls it with 'Out of case' connect reason
*/
static ruleAction ruleOutOfCaseConnectPeer(void)
{
    RULE_LOG("ruleOutOfCaseConnectPeer");
    return ruleConnectPeer(RULE_CONNECT_OUT_OF_CASE);
}

/*! @brief Rule to determine if most recently used handset should be updated
    @startuml

    start
    if (Not IsPeerSyncComplete()) then (yes)
        :Peer sync not completed;
        end 
    endif

    if (IsPeerHandsetA2dpConnected() or IsPeerHandsetHfpConnected()) then (yes)
        if (IsPairedPeerHandset()) then (yes)
            :Update MRU handzset as peer is connected to handset;
            stop
        else (no)
            :Do nothing as not paired to peer's handset;
            end
        endif
    else
        :Do nothing as peer is not connected to handset;
        end
    endif
    @enduml
*/
static ruleAction ruleUpdateMruHandset(void)
{
    /* Don't run rule if we haven't sync'ed with peer */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleUpdateMruHandset, defer as not sync'ed with peer");
        return RULE_ACTION_DEFER;
    }

    /* If peer is connected to handset then we should mark this handset as most recently used */
    if (appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetHfpConnected())
    {
        /* Check we paired with this handset */
        bdaddr handset_addr;
        appPeerSyncGetPeerHandsetAddr(&handset_addr);
        if (appDeviceIsHandset(&handset_addr))
        {
            RULE_LOG("ruleUpdateMruHandset, run as peer is connected to handset");
            return RULE_ACTION_RUN;
        }
        else
        {
            RULE_LOG("ruleUpdateMruHandset, ignore as not paired with peer's handset");
            return RULE_ACTION_IGNORE;
        }
    }
    else
    {
        /* Peer is not connected to handset */
        RULE_LOG("ruleUpdateMruHandset, ignore as peer is not connected");
        return RULE_ACTION_IGNORE;
    }

}

/*! @brief Rule to determine if Earbud should send status to handset over HFP and/or AVRCP
    @startuml

    start
    if (IsPairedHandset() and IsTwsPlusHandset()) then (yes)
        if (IsHandsetHfpConnected() or IsHandsetAvrcpConnected()) then (yes)
            :HFP and/or AVRCP connected, send status update;
            stop
        endif
    endif

    :Not connected with AVRCP or HFP to handset;
    end
    @enduml
*/
static ruleAction ruleSendStatusToHandset(void)
{
    bdaddr handset_addr;

    if (appDeviceGetHandsetBdAddr(&handset_addr) && appDeviceIsTwsPlusHandset(&handset_addr))
    {
        if (appDeviceIsHandsetHfpConnected() || appDeviceIsHandsetAvrcpConnected())
        {
            RULE_LOG("ruleSendStatusToHandset, run as TWS+ handset");
            return RULE_ACTION_RUN;
        }
    }

    RULE_LOG("ruleSendStatusToHandset, ignore as not connected to TWS+ handset");
    return RULE_ACTION_IGNORE;
}

/*! @brief Rule to determine if A2DP streaming when out of ear
    Rule is triggered by the 'out of ear' event
    @startuml

    start
    if (IsAvStreaming()) then (yes)
        :Run rule, as out of ear with A2DP streaming;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleOutOfEarA2dpActive(void)
{
    if (appAvIsStreaming())
    {
        RULE_LOG("ruleOutOfEarA2dpActive, run as A2DP is active and earbud out of ear");
        return RULE_ACTION_RUN;
    }

    RULE_LOG("ruleOutOfEarA2dpActive, ignore as A2DP not active out of ear");
    return RULE_ACTION_IGNORE;
}

/*! @brief Rule to determine if SCO active when out of ear
    Rule is triggered by the 'out of ear' event
    @startuml

    start
    if (IsScoActive()) then (yes)
        :Run rule, as out of ear with SCO active;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleOutOfEarScoActive(void)
{
    /* Don't run rule if we haven't sync'ed with peer, as this rule uses peer state */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleOutOfEarScoActive, defer as not sync'ed with peer");
        return RULE_ACTION_DEFER;
    }

    if (appScoFwdIsSending() && appPeerSyncIsPeerInEar())
    {
        RULE_LOG("ruleOutOfEarScoActive, ignore as we have SCO forwarding running and peer is in ear");
        return RULE_ACTION_IGNORE;
    }

    if (appHfpIsScoActive() && appSmIsOutOfEar())
    {
        RULE_LOG("ruleOutOfEarScoActive, run as SCO is active and earbud out of ear");
        return RULE_ACTION_RUN;
    }

    RULE_LOG("ruleOutOfEarScoActive, ignore as SCO not active out of ear");
    return RULE_ACTION_IGNORE;
}


static ruleAction ruleInEarScoTransferToEarbud(void)
{
    /* Check HFP state first, so if no active call we avoid unnecessary peer sync */
    if (!appHfpIsCallActive())
    {
        RULE_LOG("ruleInEarScoTransferToEarbud, ignore as this earbud has no active call");
        return RULE_ACTION_IGNORE;
    }

    /* May already have SCO audio if kept while out of ear in order to service slave
     * for SCO forwarding */
    if (appHfpIsScoActive())
    {
        RULE_LOG("ruleInEarScoTransferToEarbud, ignore as this earbud already has SCO");
        return RULE_ACTION_IGNORE;
    }
    
    /* Peer sync must be complete as this rule uses peer state */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleInEarScoTransferToEarbud, defer as not sync'ed with peer");
        return RULE_ACTION_DEFER;
    }

    /* For TWS+ transfer the audio the local earbud is in Ear.
     * For TWS Standard, transfer the audio if local earbud or peer is in Ear. */
    if (appSmIsInEar() || (!appDeviceIsTwsPlusHandset(appHfpGetAgBdAddr()) && appPeerSyncIsPeerInEar()))
    {
        RULE_LOG("ruleInEarScoTransferToEarbud, run as call is active and an earbud is in ear");
        return RULE_ACTION_RUN;
    }

    RULE_LOG("ruleInEarScoTransferToEarbud, ignore as SCO not active or both earbuds out of the ear");
    return RULE_ACTION_IGNORE;
}

/*! @brief Rule to determine if LED should be enabled when out of ear
    Rule is triggered by the 'out of ear' event
    @startuml

    start
    if (Not IsLedsInEarEnabled()) then (yes)
        :Run rule, as out of ear and LEDs were disabled in ear;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleOutOfEarLedsEnable(void)
{
    if (!appConfigInEarLedsEnabled())
    {
        RULE_LOG("ruleOutOfEarLedsEnable, run as out of ear");
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("ruleOutOfEarLedsEnable, ignore as out of ear but in ear LEDs enabled");
        return RULE_ACTION_IGNORE;
    }
}

/*! @brief Rule to determine if LED should be disabled when in ear
    Rule is triggered by the 'in ear' event
    @startuml

    start
    if (Not IsLedsInEarEnabled()) then (yes)
        :Run rule, as in ear and LEDs are disabled in ear;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleInEarLedsDisable(void)
{
    if (!appConfigInEarLedsEnabled())
    {
        RULE_LOG("ruleInEarLedsDisable, run as in ear");
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("ruleInEarLedsDisable, ignore as in ear but in ear LEDs enabled");
        return RULE_ACTION_IGNORE;
    }
}

/*! @brief Determine if a handset disconnect should be allowed */
static bool handsetDisconnectAllowed(void)
{
    /* Handset disconnect not allows if SCO forwarding is sending.  
     * Disconnecting is allowed if DFU is pending as disconnect only affects
     * HFP and A2DP/AVRCP */    
    return appDeviceIsHandsetConnected() && !appScoFwdIsSending();
}

/*! @brief Rule to determine if Earbud should disconnect from handset when put in case
    Rule is triggered by the 'in case' event
    @startuml

    start
    if (IsInCase() and IsHandsetConnected() and Not ScoFwdIsSending()) then (yes)
        :Disconnect from handset as now in case;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleInCaseDisconnectHandset(void)
{
    if (appSmIsInCase() && handsetDisconnectAllowed())
    {
        RULE_LOG("ruleInCaseDisconnectHandset, run as in case and handset connected");
        // Try to handover the handset connection to the other earbud
        bool handover = TRUE;
        return RULE_ACTION_RUN_PARAM(handover);
    }
    else
    {
        RULE_LOG("ruleInCaseDisconnectHandset, ignore as not in case or handset not connected or master of active call forwarding");
        return RULE_ACTION_IGNORE;
    }
}


/*! @brief Rule to connect A2DP to TWS+ handset if peer Earbud has connected A2DP for the first time */
static ruleAction rulePairingConnectTwsPlusA2dp(void)
{
    bdaddr handset_addr;
    bdaddr peer_handset_addr;
    connRulesTaskData *conn_rules = appGetConnRules();

    if (!conn_rules->allow_connect_after_pairing)
        return RULE_ACTION_IGNORE;

    /* need to access peer state, so peer sync must be complete */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("rulePairingConnectTwsPlusA2dp, defer, not synced with peer");
        return RULE_ACTION_DEFER;
    }

    if (!appSmIsOutOfCase())
    {
        RULE_LOG("rulePairingConnectTwsPlusA2dp, ignore, not out of case");
        return RULE_ACTION_IGNORE;
    }

    appDeviceGetHandsetBdAddr(&handset_addr);
    appPeerSyncGetPeerHandsetAddr(&peer_handset_addr);

    if (!appDeviceIsTwsPlusHandset(&handset_addr))
    {
        RULE_LOG("rulePairingConnectTwsPlusA2dp, ignore, not TWS+ handset");
        return RULE_ACTION_IGNORE;
    }

    if (!BdaddrIsSame(&handset_addr, &peer_handset_addr))
    {
        RULE_LOG("rulePairingConnectTwsPlusA2dp, ignore, not same handset as peer");
        return RULE_ACTION_IGNORE;
    }

    if (appDeviceIsHandsetA2dpConnected())
    {
        RULE_LOG("rulePairingConnectTwsPlusA2dp, ignore, A2DP already connected");
        return RULE_ACTION_IGNORE;
    }

    uint8 profiles = DEVICE_PROFILE_A2DP;
    RULE_LOG("rulePairingConnectTwsPlusA2dp, connect A2DP");
    return RULE_ACTION_RUN_PARAM(profiles);
}

/*! @brief Rule to connect HFP to TWS+ handset if peer Earbud has connected HFP for the first time */
static ruleAction rulePairingConnectTwsPlusHfp(void)
{
    bdaddr handset_addr;
    bdaddr peer_handset_addr;
    connRulesTaskData *conn_rules = appGetConnRules();

    if (!conn_rules->allow_connect_after_pairing)
        return RULE_ACTION_IGNORE;

    /* need to access peer state, so peer sync must be complete */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("rulePairingConnectTwsPlusHfp, defer, not synced with peer");
        return RULE_ACTION_DEFER;
    }

    if (!appSmIsOutOfCase())
    {
        RULE_LOG("rulePairingConnectTwsPlusHfp, ignore, not out of case");
        return RULE_ACTION_IGNORE;
    }

    appDeviceGetHandsetBdAddr(&handset_addr);
    appPeerSyncGetPeerHandsetAddr(&peer_handset_addr);

    if (!appDeviceIsTwsPlusHandset(&handset_addr))
    {
        RULE_LOG("rulePairingConnectTwsPlusHfp, ignore, not TWS+ handset");
        return RULE_ACTION_IGNORE;
    }

    if (!BdaddrIsSame(&handset_addr, &peer_handset_addr))
    {
        RULE_LOG("rulePairingConnectTwsPlusHfp, ignore, not same handset as peer");
        return RULE_ACTION_IGNORE;
    }

    if (appDeviceIsHandsetHfpConnected())
    {
        RULE_LOG("rulePairingConnectTwsPlusHfp, ignore, HFP already connected");
        return RULE_ACTION_IGNORE;
    }

    uint8 profiles = DEVICE_PROFILE_HFP;
    RULE_LOG("rulePairingConnectTwsPlusA2dp, connect HFP");
    return RULE_ACTION_RUN_PARAM(profiles);
}

/*! @brief Rule to decide if one Earbud should disconnect from handset, if both connected.
 */
static ruleAction ruleBothConnectedDisconnect(void)
{
      bdaddr handset_addr;
      bdaddr peer_handset_addr;

    /* need to access peer state, so peer sync must be complete */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleBothConnectedDisconnect, defer, not synced with peer");
        return RULE_ACTION_DEFER;
    }

    appDeviceGetHandsetBdAddr(&handset_addr);
    appPeerSyncGetPeerHandsetAddr(&peer_handset_addr);

    /* if both earbuds connected to the same TWS Standard handset */
    if (   appDeviceIsHandsetConnected()
        &&
           (appPeerSyncIsPeerHandsetA2dpConnected()   ||
            appPeerSyncIsPeerHandsetHfpConnected()    ||
            appPeerSyncIsPeerHandsetAvrcpConnected())
        &&
            (!appDeviceIsTwsPlusHandset(&handset_addr))
        &&
            (BdaddrIsSame(&handset_addr, &peer_handset_addr)))
    {
        /* Score each earbud, to determine which should disconnect.
         * Weight an active SCO or active A2DP stream higher, so that even if
         * only 1 profile is connected, but there is active audio this will
         * count higher than all profiles being connected. */
        int this_earbud_score = (appDeviceIsHandsetA2dpConnected() ? 1 : 0) +
                                (appDeviceIsHandsetAvrcpConnected() ? 1 : 0) + 
                                (appDeviceIsHandsetHfpConnected() ? 1 : 0) +
                                (appHfpIsScoActive() ? 3 : 0) +
                                (appDeviceIsHandsetA2dpStreaming() ? 3 : 0);
        int other_earbud_score = (appPeerSyncIsPeerHandsetA2dpConnected() ? 1 : 0) +
                                (appPeerSyncIsPeerHandsetAvrcpConnected() ? 1 : 0) + 
                                (appPeerSyncIsPeerHandsetHfpConnected() ? 1 : 0) +
                                (appPeerSyncIsPeerScoActive() ? 3 : 0) +
                                (appPeerSyncIsPeerHandsetA2dpStreaming() ? 3 : 0);

        /* disconnect lowest scoring earbud */
        if (this_earbud_score < other_earbud_score)
        {
            RULE_LOGF("ruleBothConnectedDisconnect, run as lower score: this %u other %u",
                        this_earbud_score, other_earbud_score);
            return RULE_ACTION_RUN;
        }
        else if (this_earbud_score > other_earbud_score)
        {
            RULE_LOGF("ruleBothConnectedDisconnect, ignore as higher score: this %u other %u",
                    this_earbud_score, other_earbud_score);
            return RULE_ACTION_IGNORE;
        }
        else
        {
            /* equal scores, disconnect the left */
            if (appConfigIsLeft())
            {
                RULE_LOGF("ruleBothConnectedDisconnect, run, same score and we're left: this %u other %u",
                        this_earbud_score, other_earbud_score);
                return RULE_ACTION_RUN;
            }
            else
            {
                RULE_LOGF("ruleBothConnectedDisconnect, ignore, same score and we're right: this %u other %u",
                        this_earbud_score, other_earbud_score);
                return RULE_ACTION_IGNORE;
            }
        }
    }

    RULE_LOG("ruleBothConnectedDisconnect, ignore, both earbuds not connected to the same TWS Standard handset");
    return RULE_ACTION_IGNORE;
}

/*! @brief Rule to determine if Earbud should disconnect A2DP/AVRCP/SCOFWD from peer when put in case
    Rule is triggered by the 'in case' event
    @startuml

    start
    if (IsInCase() and IsPeerA2dpConnected() and IsPeerAvrcpConnectedForAv()) then (yes)
        :Disconnect from peer as now in case;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleInCaseDisconnectPeer(void)
{
    if (appSmIsInCase() && (appDeviceIsPeerA2dpConnected() || 
                            appDeviceIsPeerAvrcpConnectedForAv() ||
                            appDeviceIsPeerScoFwdConnected()))
    {
        if (appScoFwdIsSending())
        {
            RULE_LOG("ruleInCaseDisconnectPeer, ignore as master of active SCO forwarding");
            return RULE_ACTION_IGNORE;
        }

        RULE_LOG("ruleInCaseDisconnectPeer, run as in case and peer connected");
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("ruleInCaseDisconnectPeer, ignore as not in case or peer not connected");
        return RULE_ACTION_IGNORE;
    }
}

/*! @brief Rule to determine if Earbud should start DFU  when put in case
    Rule is triggered by the 'in case' event
    @startuml

    start
    if (IsInCase() and DfuUpgradePending()) then (yes)
        :DFU upgrade can start as it was pending and now in case;
        stop
    endif
    end
    @enduml
*/
static ruleAction ruleInCaseEnterDfu(void)
{
#ifdef INCLUDE_DFU
    if (appSmIsInCase() && appSmIsDfuPending())
    {
        RULE_LOG("ruleInCaseCheckDfu, run as still in case & DFU pending/active");
        return RULE_ACTION_RUN;
    }
    else
    {
        RULE_LOG("ruleInCaseCheckDfu, ignore as not in case or no DFU pending");
        return RULE_ACTION_IGNORE;
    }
#else
    return RULE_ACTION_IGNORE;
#endif
}


static ruleAction ruleDfuAllowHandsetConnect(void)
{
#ifdef INCLUDE_DFU
    bdaddr handset_addr;

    /* If we're already connected to handset then don't connect */
    if (appDeviceIsHandsetConnected())
    {
        RULE_LOG("ruleDfuAllowHandsetConnect, ignore as already connected to handset");
        return RULE_ACTION_IGNORE;
    }
    /* This rule has been run when entering the special DFU mode. 
       This should only be entered when we restart during an update, or if
       the user has requested it (only possible if appConfigDfuOnlyFromUiInCase()
       is TRUE */
    if (appConfigDfuOnlyFromUiInCase())
    {
        RULE_LOG("ruleDfuAllowHandsetConnect - run as use in case DFU");
        return RULE_ACTION_RUN;
    }
    if (appSmIsInDfuMode())
    {
        RULE_LOG("ruleDfuAllowHandsetConnect, run as in DFU mode");
        return RULE_ACTION_RUN;
    }
    RULE_LOG("ruleDfuAllowHandsetConnect, ignore as not DFU");
#endif /* INCLUDE_DFU */

    return RULE_ACTION_IGNORE;
}


static ruleAction ruleCheckUpgradable(void)
{
    bool allow_dfu = TRUE;
    bool block_dfu = FALSE;

    if (appSmIsOutOfCase())
    {
        if (appConfigDfuOnlyFromUiInCase())
        {
            RULE_LOG("ruleCheckUpgradable, block as only allow DFU from UI (and in case)");
            return RULE_ACTION_RUN_PARAM(block_dfu);
        }
        if (appSmHasBleConnection() && appConfigDfuAllowBleUpgradeOutOfCase())
        {
            RULE_LOG("ruleCheckUpgradable, allow as BLE connection");
            return RULE_ACTION_RUN_PARAM(allow_dfu);
        }
        if (appDeviceIsHandsetConnected() && appConfigDfuAllowBredrUpgradeOutOfCase())
        {
            RULE_LOG("ruleCheckUpgradable, allow as BREDR connection");
            return RULE_ACTION_RUN_PARAM(allow_dfu);
        }

        RULE_LOG("ruleCheckUpgradable, block as out of case and not permitted");
        return RULE_ACTION_RUN_PARAM(block_dfu);
    }
    else
    {
        if (appConfigDfuOnlyFromUiInCase())
        {
            if (appSmIsDfuPending())
            {
                RULE_LOG("ruleCheckUpgradable, allow as in case - DFU pending");
                return RULE_ACTION_RUN_PARAM(allow_dfu);
            }
            RULE_LOG("ruleCheckUpgradable, block as only allow DFU from UI");
            return RULE_ACTION_RUN_PARAM(block_dfu);
        }

        RULE_LOG("ruleCheckUpgradable, allow as in case");
        return RULE_ACTION_RUN_PARAM(allow_dfu);
    }
}


/*! @brief Rule to determine if Earbud should disconnect A2DP/AVRCP from peer Earbud
    @startuml

    start
    if (Not IsPeerA2dpConnected() and Not IsPeerAvrcoConnectedForAv()) then (yes)
        :Not connected, do nothing;
        stop
    endif

    if (Not IsHandsetPaired()) then (yes)
        :Not paired with handset, disconnect from peer;
        stop
    endif

    if (IsHandsetA2dpConnected()) then (yes)
        if (IsTwsPlusHandset()) then (yes)
            :Connected to TWS+ handset, no need for A2DP/AVCRP to peer;
            stop
        else
            :Connected to standard handset, still require A2DP/AVRCP to peer;
            end
        endif
    else
        :Not connected with A2DP to handset;
        end
    endif    
    @enduml
*/
static ruleAction ruleDisconnectPeer(void)
{
    bdaddr handset_addr;

    /* Don't run rule if we're not connected to peer */
    if (!appDeviceIsPeerA2dpConnected() &&
        !appDeviceIsPeerAvrcpConnectedForAv() &&
        !appDeviceIsPeerScoFwdConnected())
    {
        RULE_LOG("ruleDisconnectPeer, ignore as not connected to peer");
        return RULE_ACTION_IGNORE;
    }

    /* If we're not paired with handset then disconnect */
    if (!appDeviceGetHandsetBdAddr(&handset_addr))
    {
        RULE_LOG("ruleDisconnectPeer, run as not paired with handset");
        return RULE_ACTION_RUN;
    }

    /* If we're connected to a handset, but it's a TWS+ handset then we don't need connction to peer */
    if (appDeviceIsHandsetA2dpConnected() || appDeviceIsHandsetHfpConnected())
    {
        if (appDeviceIsTwsPlusHandset(&handset_addr))
        {
            RULE_LOG("ruleDisconnectPeer, run as connected to TWS+ handset");
            return RULE_ACTION_RUN;
        }
        else
        {
            RULE_LOG("ruleDisconnectPeer, ignore as connected to standard handset");
            return RULE_ACTION_IGNORE;
        }
    }
    else
    {
        RULE_LOG("ruleDisconnectPeer, run as not connected handset");
        return RULE_ACTION_RUN;
    }
}

static ruleAction ruleOutOfCaseAllowHandsetConnect(void)
{
    RULE_LOG("ruleOutOfCaseAllowHandsetConnect, run as out of case");
    return RULE_ACTION_RUN;
}

static ruleAction ruleInCaseRejectHandsetConnect(void)
{
#ifdef INCLUDE_DFU
    if (appSmIsDfuPending())
    {
        RULE_LOG("ruleInCaseRejectHandsetConnect, ignored as DFU pending");
        return RULE_ACTION_IGNORE;
    }
#endif

    RULE_LOG("ruleInCaseRejectHandsetConnect, run as in case and no DFU");
    return RULE_ACTION_RUN;
}

static ruleAction ruleInCaseAncTuning(void)
{
    if (appConfigAncTuningEnabled())
        return RULE_ACTION_RUN;
    else
        return RULE_ACTION_IGNORE;
}

static ruleAction ruleOutOfCaseAncTuning(void)
{
    if (appConfigAncTuningEnabled())
        return RULE_ACTION_RUN;
    else
        return RULE_ACTION_IGNORE;
}

static ruleAction ruleSyncDisconnectPeer(void)
{
    /* Peer sync must be complete as this rule uses peer state */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleSyncDisconnectPeer, defer as not sync'ed with peer");
        return RULE_ACTION_DEFER;
    }

    /* Don't run rule if we're not connected to peer */
    if (!appDeviceIsPeerA2dpConnected() &&
        !appDeviceIsPeerAvrcpConnectedForAv() &&
        !appDeviceIsPeerScoFwdConnected())
    {
        RULE_LOG("ruleSyncDisconnectPeer, ignore as not connected to peer");
        return RULE_ACTION_IGNORE;
    }

    if (appPeerSyncIsPeerPairing())
    {
        RULE_LOG("ruleSyncDisconnectPeer, run as peer is pairing");
        return RULE_ACTION_RUN;
    }

    return RULE_ACTION_IGNORE;
}


static ruleAction ruleSyncDisconnectHandset(void)
{
    /* Peer sync must be complete as this rule uses peer state */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleSyncDisconnectHandset, defer as not sync'ed with peer");
        return RULE_ACTION_DEFER;
    }

    /* Don't run rule if we're not connected to handset */
    if (!appDeviceIsHandsetA2dpConnected() &&
        !appDeviceIsHandsetAvrcpConnected() &&
        !appDeviceIsHandsetHfpConnected())
    {
        RULE_LOG("ruleSyncDisconnectHandset, ignore as not connected to handset");
        return RULE_ACTION_IGNORE;
    }

    if (appPeerSyncIsPeerPairing())
    {
        RULE_LOG("ruleSyncDisconnectHandset, run as peer is pairing");
        return RULE_ACTION_RUN;
    }

    return RULE_ACTION_IGNORE;
}




/*! @brief Rule to determine if page scan settings should be changed.

    @startuml
        (handset1)
        (handset2)
        (earbud1)
        (earbud2)
        earbud1 <-> earbud2 : A
        earbud1 <--> handset1 : B
        earbud2 <--> handset2 : C
    @enduml
    A = link between earbuds
    B = link from earbud1 to handset1
    C = link from earbud2 to handset2
    D = earbud1 handset is TWS+
    E = earbud2 handset is TWS+
    F = earbud is connectable
    Links B and C are mutually exclusive.

    Page scan is controlled as defined in the following truth table (X=Don't care).
    Viewed from the perspective of Earbud1.

    Peer sync must be complete for the rule to run if the state of the peer handset
    affects the result of the rule.

    A | B | C | D | E | F | Page Scan On
    - | - | - | - | - | - | ------------
    X | X | X | X | X | 0 | 0
    0 | X | X | X | X | 1 | 1
    1 | 0 | 0 | X | X | 1 | 1
    1 | 0 | 1 | X | 1 | 1 | 1
    1 | 0 | 1 | X | 0 | 1 | 0
    1 | 1 | X | X | X | 1 | 0
*/
static ruleAction rulePageScanUpdate(void)
{
    bool ps_on = FALSE;
    bool sm_connectable = appSmIsConnectable();
    bool peer = appDeviceIsPeerConnected();
    bool handset = appDeviceIsHandsetAnyProfileConnected();
    bool peer_sync = appPeerSyncIsComplete();
    bool peer_handset = appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetAvrcpConnected() || appPeerSyncIsPeerHandsetHfpConnected();
    bool peer_handset_tws = appPeerSyncIsPeerHandsetTws();

    if (!sm_connectable)
    {
        // Not connectable
        ps_on = FALSE;
    }
    else if (!peer)
    {
        // No peer
        ps_on = TRUE;
    }
    else if (handset)
    {
        // Peer, handset
        ps_on = FALSE;
    }
    else if (!peer_sync)
    {
        // Peer, no handset (need peer sync to decide)
        RULE_LOG("rulePageScanUpdate, peer sync not complete, ignore");
        return RULE_ACTION_IGNORE;
    }
    else if (peer_handset)
    {
        if (peer_handset_tws)
        {
            // Peer, no handset, tws peer handset
            ps_on = TRUE;
        }
        else
        {
            // Peer, no handset, standard peer handset
            ps_on = FALSE;
        }
    }
    else
    {
        // Peer, no handset, no peer handset
        ps_on = TRUE;
    }

#ifndef DISABLE_LOG
    /* Reduce log utilisation by this frequently run rule by compressing the
       booleans into nibbles in a word. When printed in hex, this will display
       as a binary bitfield. */
    uint32 log_val = (!!sm_connectable   << 20) |
                     (!!peer             << 16) |
                     (!!handset          << 12) |
                     (!!peer_sync        << 8)  |
                     (!!peer_handset     << 4)  |
                     (!!peer_handset_tws << 0);
    RULE_LOGF("rulePageScanUpdate, (sm_connectable,peer,handset,peer_sync,peer_handset,peer_handset_tws)=%06x, ps=%d", log_val, ps_on);
#endif

    if (ps_on && !appScanManagerIsPageScanEnabledForUser(SCAN_MAN_USER_SM))
    {
        /* need to enable page scan and it is not already enabled for the SM user */
        RULE_LOG("rulePageScanUpdate, run, need to enable page scan");

        /* using CONN_RULES_PAGE_SCAN_UPDATE message which take a bool parameter,
         * use RULE_ACTION_RUN_PARAM macro to prime the message data and indicate
         * to the rules engine it should return it in the message to the client task */
        return RULE_ACTION_RUN_PARAM(ps_on);
    }
    else if (!ps_on && appScanManagerIsPageScanEnabledForUser(SCAN_MAN_USER_SM))
    {
        /* need to disable page scan and it is currently enabled for SM user */
        RULE_LOG("rulePageScanUpdate, run, need to disable page scan");
        return RULE_ACTION_RUN_PARAM(ps_on);
    }

    return RULE_ACTION_IGNORE;
}

static ruleAction ruleInCaseScoTransferToHandset(void)
{
    /* check for active SCO first to avoid unnecessary peer sync */
    if (!appHfpIsScoActive())
    {
        RULE_LOG("ruleInCaseScoTransferToHandset, ignore as no active call");
        return RULE_ACTION_IGNORE;
    }
    /* peer sync must be complete as this rule uses peer state information */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleInCaseScoTransferToHandset, defer as peer sync not complete");
        return RULE_ACTION_DEFER;
    }
    if (appSmIsInCase() && appPeerSyncIsPeerInCase())
    {
        RULE_LOG("ruleInCaseScoTransferToHandset, run, we have active call but both earbuds in case");
        return RULE_ACTION_RUN;
    }
    RULE_LOG("ruleInCaseScoTransferToHandset, ignored");
    return RULE_ACTION_IGNORE;
}

static ruleAction ruleSelectMicrophone(void)
{
    micSelection selected_mic = MIC_SELECTION_LOCAL;

    /* peer sync must be complete as this rule uses peer state information */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleSelectMicrophone, defer as peer sync not complete");
        return RULE_ACTION_DEFER;
    }
    if (!appSmIsInEar() && appPeerSyncIsPeerInEar())
    {
        selected_mic = MIC_SELECTION_REMOTE;
        RULE_LOG("ruleSelectMicrophone, SCOFWD master out of ear and slave in ear use remote microphone");
        return RULE_ACTION_RUN_PARAM(selected_mic);
    }
    if (appSmIsInEar())
    {
        RULE_LOG("ruleSelectMicrophone, SCOFWD master in ear, use local microphone");
        return RULE_ACTION_RUN_PARAM(selected_mic);
    }

    RULE_LOG("ruleSelectMicrophone, ignore as both earbuds out of ear");
    return RULE_ACTION_IGNORE;
}

static ruleAction ruleScoForwardingControl(void)
{
    const bool forwarding_enabled = TRUE;
    const bool forwarding_disabled = FALSE;

    /* only need to consider this rule if we have SCO audio on the earbud,
     * also prevents the slave running this rule */
    if (!appHfpIsScoActive())
    {
        RULE_LOG("ruleScoForwardingControl, ignore as no active SCO");
        return RULE_ACTION_IGNORE;
    }
    /* peer sync must be complete as this rule uses peer state information */
    if (!appPeerSyncIsComplete())
    {
        RULE_LOG("ruleScoForwardingControl, defer as peer sync not complete");
        return RULE_ACTION_DEFER;
    }
    if (!appPeerSyncIsPeerInEar())
    {
        RULE_LOG("ruleScoForwardingControl, run and disable forwarding as peer out of ear");
        return RULE_ACTION_RUN_PARAM(forwarding_disabled);
    }
    if (appPeerSyncIsPeerInEar())
    {
        RULE_LOG("ruleScoForwardingControl, run and enable forwarding as peer in ear");
        return RULE_ACTION_RUN_PARAM(forwarding_enabled);
    }
    
    RULE_LOG("ruleScoForwardingControl, ignore");
    return RULE_ACTION_IGNORE;
}


/*! @brief Rule to validate whether handover should be initiated */
static ruleAction ruleHandoverDisconnectHandset(void)
{
    bdaddr addr;
    if (appDeviceIsHandsetConnected() &&
        appDeviceGetHandsetBdAddr(&addr) &&
        !appDeviceIsTwsPlusHandset(&addr))
    {
        bool allowed = !appSmIsDfuPending();
        return allowed ? RULE_ACTION_RUN_PARAM(allowed) : RULE_ACTION_DEFER;
    }
    return RULE_ACTION_IGNORE;
}

/*! @brief Rule to validate whether handover should cause handset to be connected */
static ruleAction ruleHandoverConnectHandset(void)
{
    return ruleConnectHandset(RULE_CONNECT_USER, RULE_POST_HANDSET_CONNECT_ACTION_NONE);
}

/*! @brief Rule to validate whether handover should cause handset to be connected
    then media played */
static ruleAction ruleHandoverConnectHandsetAndPlay(void)
{
    return ruleConnectHandset(RULE_CONNECT_USER, RULE_POST_HANDSET_CONNECT_ACTION_PLAY_MEDIA);
}


/*! @brief Rule to determine if BLE connection settings should be changed

    \todo include UML documentation. 

*/
static ruleAction ruleBleConnectionUpdate(void)
{
    bool peer = appDeviceIsPeerConnected();
    bool peer_sync = appPeerSyncIsComplete();
    bool peer_handset = appPeerSyncIsPeerHandsetA2dpConnected() || appPeerSyncIsPeerHandsetAvrcpConnected() || appPeerSyncIsPeerHandsetHfpConnected();
    bool peer_handset_paired = appPeerSyncHasPeerHandsetPairing();
    bool peer_ble_advertising = appPeerSyncIsPeerAdvertising();
    bool peer_ble_connected = appPeerSyncIsPeerBleConnected();
    bool left = appConfigIsLeft();
    bool peer_dfu = appPeerSyncPeerDfuInProgress();
    bool in_case = appSmIsInCase();
    bool peer_in_case = appPeerSyncIsPeerInCase();
    bool ble_connectable = appSmStateAreNewBleConnectionsAllowed(appGetState());
    bool paired_with_peer = appDeviceGetPeerBdAddr(NULL);
    bool paired_with_handset = appDeviceGetHandsetBdAddr(NULL);

    bool has_ble_connection = appSmHasBleConnection();
    bool is_ble_connecting = appSmIsBleAdvertising();

    bool connectable;

        /* Use our own status to decide if we should be connectable */
    connectable = paired_with_peer && paired_with_handset && ble_connectable && !has_ble_connection;
    DEBUG_LOG("ruleBleConnectionUpdate Paired(peer:%d, handset:%d). BLE(allowed:%d,allowed_out_case:%d,has:%d,is_trying:%d)",
                    paired_with_peer,paired_with_handset,
                    ble_connectable, appConfigBleAllowedOutOfCase(),
                    has_ble_connection, is_ble_connecting);

        /* Now take the peer status into account */
    if (connectable && peer)
    {
        /* We know that we have a peer connection, but waiting for synchronisation */
        if (!peer_sync)
        {
            DEBUG_LOG("ruleBleConnectionUpdate Connected to peer, so wait until Peer synchronised IGNORE for now");
            return RULE_ACTION_IGNORE;
        }

        if (peer_handset || peer_dfu || (peer_ble_advertising && ! peer_in_case) || peer_ble_connected)
        {
            DEBUG_LOG("ruleBleConnectionUpdate Peer has handset connection:%d, DFU:%d, BLE-adv:%d (out of case), or BLE-connection. We don't want to do BLE",
                        peer_handset, peer_dfu, peer_ble_advertising, peer_ble_connected);

            connectable = FALSE;
        }
        else if (!peer_handset_paired)
        {
            DEBUG_LOG("ruleBleConnectionUpdate Peer has no handset.");
        }
        else if (in_case)
        {
            /* Do nothing. We want both to be connectable, unless DFU/BLE
               which were eliminated above */
            DEBUG_LOG("ruleBleConnectionUpdate In case, ignore excuses not to advertise");
        }
        else if (!peer_in_case)
        {
            uint16 our_battery;
            uint16 peer_battery;

            appPeerSyncGetPeerBatteryLevel(&our_battery,&peer_battery);
            if (our_battery < peer_battery)
            {
                DEBUG_LOG("ruleBleConnectionUpdate Peer (out of case) has stronger battery.");
                connectable = FALSE;
            }
            else if (our_battery == peer_battery)
            {
                if (!left)
                {
                    DEBUG_LOG("ruleBleConnectionUpdate we have same battery, and are right handset. Don't do BLE.");
                    connectable = FALSE;
                }
            }
        }
    }
    else if (connectable)
    {
        DEBUG_LOG("ruleBleConnectionUpdate No peer connection, just using local values");
    }

    if (connectable == is_ble_connecting)
    {
        RULE_LOG("ruleBleConnectionUpdate, IGNORE - no change");
        return RULE_ACTION_IGNORE;
    }

    if (connectable)
    {
        RULE_LOG("ruleBleConnectionUpdate, run, need to allow new BLE connections");
    }
    else 
    {
        RULE_LOG("ruleBleConnectionUpdate, run, need to disallow new BLE connections");
    }
    return RULE_ACTION_RUN_PARAM(connectable);

}


/*****************************************************************************
 * END RULES FUNCTIONS
 *****************************************************************************/

/*! \brief Update the status of a rule.
    \return bool TRUE if the status of a rule was updated, FALSE otherwise.

    This function will also check if all rules for an event are now complete,
    i.e. RULE_STATUS_COMPLETE, if so it will automatically clear that event.
*/
static bool appConRulesSetRuleStatus(MessageId message, ruleStatus status, ruleStatus new_status, connRulesEvents event)
{
    int rule_index;
    int num_rules = sizeof(appConnRules) / sizeof(ruleEntry);
    connRulesEvents event_mask = 0;
    bool did_set_status = FALSE;

    for (rule_index = 0; rule_index < num_rules; rule_index++)
    {
        ruleEntry *rule = &appConnRules[rule_index];
        if ((rule->message == message) && (rule->status == status) && (rule->events & event))
        {
            CONNRULES_LOGF("appConnRulesSetStatus, rule %d, status %d", rule_index, new_status);
            SET_RULE_STATUS(rule, new_status);
            did_set_status = TRUE;
            /* Build up set of events where rules are complete */
            event_mask |= rule->events;
        }
    }

    /* Check if all rules for an event are now complete, if so clear event */
    for (rule_index = 0; rule_index < num_rules; rule_index++)
    {
        ruleEntry *rule = &appConnRules[rule_index];
        if (rule->events & event)
        {
            /* Clear event if this rule is not complete */
            if (rule->status != RULE_STATUS_COMPLETE)
                event_mask &= ~rule->events;
        }
    }

    if (event_mask)
    {
        CONNRULES_LOGF("appConnRulesSetStatus, event %08lx%08lx complete", PRINT_ULL(event_mask));
        appConnRulesResetEvent(event_mask);
    }

    return did_set_status;
}

/*! \brief Run a single rule */
inline static ruleAction appConnRulesRunRule(int rule_index)
{
    ruleEntry *rule = &appConnRules[rule_index];
#ifdef CONNRULES_TIMING_LOG_ENABLED
    rtime_t start = SystemClockGetTimerTime();
    ruleAction action = rule->rule();
    rtime_t finish = SystemClockGetTimerTime();
    DEBUG_LOGF("appConnRulesCheck timing rule %d took %u us", rule_index, rtime_sub(finish, start));
    return action;
#else
    return rule->rule();
#endif
}

/*! \brief Run all the rules */
inline static void appConnRulesRunRules(void)
{
    connRulesTaskData *conn_rules = appGetConnRules();
    int rule_index;
    int num_rules = sizeof(appConnRules) / sizeof(ruleEntry);
    connRulesEvents events = conn_rules->events;

    CONNRULES_LOGF("appConnRulesCheck, starting events %08lx%08lx", PRINT_ULL(events));

    for (rule_index = 0; rule_index < num_rules; rule_index++)
    {
        ruleEntry *rule = &appConnRules[rule_index];
        ruleAction action;

        /* On check rules that match event */
        if ((rule->events & events) == rule->events ||
             rule->flags == RULE_FLAG_ALWAYS_EVALUATE)
        {
            /* Skip rules that are now complete */
            if (rule->status == RULE_STATUS_COMPLETE)
                continue;

            /* Stop checking rules for this event if rule is in progress */
            if (rule->status == RULE_STATUS_IN_PROGRESS)
            {
                events &= ~rule->events;
                CONNRULES_LOGF("appConnRulesCheck, in progress, filtered events %08lx%08lx", PRINT_ULL(events));
                continue;
            }

            /* Run the rule */
            CONNRULES_LOGF("appConnRulesCheck, running rule %d, status %d, events %08lx%08lx",
                                                    rule_index, rule->status, PRINT_ULL(events));
            action = appConnRulesRunRule(rule_index);

            /* handle result of the rule */
            if ((action == RULE_ACTION_RUN) ||
                (action == RULE_ACTION_RUN_WITH_PARAM))
            {
                TaskList *rule_tasks = appTaskListInit();
                TaskListData data = {0};
                Task task = 0;

                CONNRULES_LOG("appConnRulesCheck, rule in progress");

                /* mark rule as in progress, but not if this is an always
                 * evaluate rule */
                if (rule->flags != RULE_FLAG_ALWAYS_EVALUATE)
                {
                    SET_RULE_STATUS(rule, RULE_STATUS_IN_PROGRESS);
                }

                /*  Build list of tasks to send message to */
                while (appTaskListIterateWithData(conn_rules->event_tasks, &task, &data))
                {
                    if ((data.u64 & rule->events) == rule->events)
                    {
                        appTaskListAddTask(rule_tasks, task);
                    }
                }

                /* Send rule message to tasks in list. */
                if (action == RULE_ACTION_RUN)
                {
                    PanicFalse(conn_rules->size_rule_message_data == 0);
                    PanicFalse(conn_rules->rule_message_data == NULL);

                    /* for no parameters just send the message with id */
                    appTaskListMessageSendId(rule_tasks, rule->message);
                }
                else if (action == RULE_ACTION_RUN_WITH_PARAM)
                {
                    PanicFalse(conn_rules->size_rule_message_data != 0);
                    PanicFalse(conn_rules->rule_message_data != NULL);

                    /* for rules with parameters, use the message data that
                     * the rule will have placed in conn_rules already */
                    appTaskListMessageSendWithSize(rule_tasks, rule->message,
                                                   conn_rules->rule_message_data,
                                                   conn_rules->size_rule_message_data);

                    /* do not need to free rule_message_data, it has been
                     * used in the message system and will be freed once
                     * automatically once delivered, just clean up local
                     * references */
                    conn_rules->rule_message_data = NULL;
                    conn_rules->size_rule_message_data = 0;
                }
                appTaskListDestroy(rule_tasks);

                /* Stop checking rules for this event
                 * we only want to continue processing rules for this event after
                 * it has been marked as completed */
                if (rule->flags != RULE_FLAG_ALWAYS_EVALUATE)
                    events &= ~rule->events;
                continue;
            }
            else if (action == RULE_ACTION_COMPLETE)
            {
                CONNRULES_LOG("appConnRulesCheck, rule complete");
                if (rule->flags != RULE_FLAG_ALWAYS_EVALUATE)
                    appConRulesSetRuleStatus(rule->message, rule->status, RULE_STATUS_COMPLETE, rule->events);
            }
            else if (action == RULE_ACTION_IGNORE)
            {
                CONNRULES_LOG("appConnRulesCheck, rule ignored");
                if (rule->flags != RULE_FLAG_ALWAYS_EVALUATE)
                    appConRulesSetRuleStatus(rule->message, rule->status, RULE_STATUS_COMPLETE, rule->events);
            }
            else if (action == RULE_ACTION_DEFER)
            {
                CONNRULES_LOG("appConnRulesCheck, rule deferred");
                SET_RULE_STATUS(rule, RULE_STATUS_DEFERRED);
            }
        }
    }
}

static void appConnRulesCheck(void)
{
#ifdef CONNRULES_TIMING_LOG_ENABLED
    rtime_t start = SystemClockGetTimerTime();
    appConnRulesRunRules();
    rtime_t finish = SystemClockGetTimerTime();
    DEBUG_LOGF("appConnRulesCheck timing total run time %u us", rtime_sub(finish, start));
#else
    appConnRulesRunRules();
#endif
}



/*! \brief Initialise the connection rules module. */
void appConnRulesInit(void)
{
    connRulesTaskData *conn_rules = appGetConnRules();
    conn_rules->events = 0;
    conn_rules->event_tasks = appTaskListWithDataInit();
    conn_rules->nop_tasks = appTaskListInit();

#ifdef ALLOW_CONNECT_AFTER_PAIRING
    conn_rules->allow_connect_after_pairing = TRUE;
#else
    conn_rules->allow_connect_after_pairing = FALSE;
#endif
}

void appConnRulesSetEvent(Task client_task, connRulesEvents event_mask)
{
    connRulesTaskData *conn_rules = appGetConnRules();
    TaskListData data = {0};

    conn_rules->events |= event_mask;
    CONNRULES_LOGF("appConnRulesSetEvent, new event %08lx%08lx, events %08lx%08lx", PRINT_ULL(event_mask), PRINT_ULL(conn_rules->events));

    if (appTaskListGetDataForTask(conn_rules->event_tasks, client_task, &data))
    {
        data.u64 |= event_mask;
        appTaskListSetDataForTask(conn_rules->event_tasks, client_task, &data);
    }
    else
    {
        data.u64 |= event_mask;
        appTaskListAddTaskWithData(conn_rules->event_tasks, client_task, &data);
    }

    appConnRulesCheck();
}

void appConnRulesResetEvent(connRulesEvents event)
{
    int rule_index;
    int num_rules = sizeof(appConnRules) / sizeof(ruleEntry);
    connRulesTaskData *conn_rules = appGetConnRules();
    TaskListData data = {0};
    Task iter_task = 0;

    conn_rules->events &= ~event;
    //CONNRULES_LOGF("appConnRulesResetEvent, new event %08lx%08lx, events %08lx%08lx", PRINT_ULL(event), PRINT_ULL(conn_rules->events));

    /* Walk through matching rules resetting the status */
    for (rule_index = 0; rule_index < num_rules; rule_index++)
    {
        ruleEntry *rule = &appConnRules[rule_index];

        if (rule->events & event)
        {
            //CONNRULES_LOGF("appConnRulesResetEvent, resetting rule %d", rule_index);
            SET_RULE_STATUS(rule, RULE_STATUS_NOT_DONE);
        }
    }

    /* delete the event from any tasks on the event_tasks list that is registered
     * for it. If a task has no remaining events, delete it from the list */
    while (appTaskListIterateWithData(conn_rules->event_tasks, &iter_task, &data))
    {
        if ((data.u64 & event) == event)
        {
            CONNRULES_LOGF("appConnRulesResetEvent, clearing event %08lx%08lx", PRINT_ULL(event));
            data.u64 &= ~event;
            if (data.u64)
            {
                appTaskListSetDataForTask(conn_rules->event_tasks, iter_task, &data);
            }
            else
            {
                appTaskListRemoveTask(conn_rules->event_tasks, iter_task);
            }
        }
    }
}

connRulesEvents appConnRulesGetEvents(void)
{
    connRulesTaskData *conn_rules = appGetConnRules();
    return conn_rules->events;
}

void appConnRulesSetRuleComplete(MessageId message)
{
    appConnRulesSetRuleWithEventComplete(message, RULE_EVENT_ALL_EVENTS_MASK);
}

void appConnRulesSetRuleWithEventComplete(MessageId message, connRulesEvents event)
{
    connRulesTaskData *conn_rules = appGetConnRules();
    bool did_complete_in_progress_rule = appConRulesSetRuleStatus(message, RULE_STATUS_IN_PROGRESS, RULE_STATUS_COMPLETE, event);

    /* if we have just completed a previously 'in-progress' rule and there
     * are no more rules in-progress (for which RULE_FLAG_PROGRESS_MATTERS
     * is set) then indicate to registered clients that the rules engine
     * has nothing in progress */
    if (did_complete_in_progress_rule &&
        !appConnRulesInProgress())
    {
        appTaskListMessageSendId(conn_rules->nop_tasks, CONN_RULES_NOP);
    }

    appConnRulesCheck();
}

/*! \brief Copy rule param data for the engine to put into action messages.
    \param param Pointer to data to copy.
    \param size_param Size of the data in bytes.
    \return RULE_ACTION_RUN_WITH_PARAM to indicate the rule action message needs parameters.
 */
static ruleAction appConnRulesCopyRunParams(const void* param, size_t size_param)
{
    connRulesTaskData *conn_rules = appGetConnRules();
    conn_rules->rule_message_data = PanicUnlessMalloc(size_param);
    conn_rules->size_rule_message_data = size_param;
    memcpy(conn_rules->rule_message_data, param, size_param);
    return RULE_ACTION_RUN_WITH_PARAM;
}

/*! \brief Determine if there are still rules in progress.
    Only consider those rules which have the RULE_FLAG_PROGRESS_MATTERS set.
*/
bool appConnRulesInProgress(void)
{
    int rule_index;
    int num_rules = sizeof(appConnRules) / sizeof(ruleEntry);
    bool rc = FALSE;

    for (rule_index = 0; rule_index < num_rules; rule_index++)
    {
        ruleEntry *rule = &appConnRules[rule_index];
        if ((rule->flags == RULE_FLAG_PROGRESS_MATTERS) &&
            (rule->status == RULE_STATUS_IN_PROGRESS))
        {
            CONNRULES_LOGF("appConnRulesInProgress rule %u in progress", rule_index);
            rc = TRUE;
        }
    }
    return rc;
}

 /*! \brief Register a task to receive notifications that no rules are in progress. */
void appConnRulesNopClientRegister(Task task) 
{     
    connRulesTaskData *conn_rules = appGetConnRules();     
    appTaskListAddTask(conn_rules->nop_tasks, task); 
}

