/*!
\copyright  Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_peer_sync.c
\brief      Application state machine peer earbud synchronisation.
*/

#include "av_headset.h"
#include "av_headset_log.h"
#include "av_headset_peer_sync.h"
#include "av_headset_device.h"
#include "av_headset_peer_signalling.h"
#include "av_headset_battery.h"
#include "av_headset_conn_rules.h"

#include <bdaddr.h>
#include <panic.h>

static uint16 appPeerSyncReadUint16(const uint8 *data);
static uint24 appPeerSyncReadUint24(const uint8 *data);
static void appPeerSyncWriteUint16(uint8 *data, uint16 val);
static void appPeerSyncWriteUint24(uint8 *data, uint24 val);

/* Uncomment to enable generation of connected/disconnected events
 * per profile for peer-handset link */
//#define PEER_SYNC_PROFILE_EVENTS

/* Macros to manipulate peer sync state. */
/*!@{ */
#define PEER_SYNC_STATE_SET_SENDING(x)      ((x) |= PEER_SYNC_SENDING)
#define PEER_SYNC_STATE_CLEAR_SENDING(x)    ((x) &= ~PEER_SYNC_SENDING)
#define PEER_SYNC_STATE_SET_SENT(x)         ((x) |= PEER_SYNC_SENT)
#define PEER_SYNC_STATE_CLEAR_SENT(x)       ((x) &= ~PEER_SYNC_SENT)
#define PEER_SYNC_STATE_SET_RECEIVED(x)     ((x) |= PEER_SYNC_RECEIVED)
#define PEER_SYNC_STATE_CLEAR_RECEIVED(x)   ((x) &= ~PEER_SYNC_RECEIVED)
#define PEER_SYNC_STATE_RESET(x)            ((x) &= ~(PEER_SYNC_SENDING | PEER_SYNC_SENT | PEER_SYNC_RECEIVED))
/*!@} */

/*! \brief Macro to increment a peer sync sequence number. */
#define PEER_SYNC_SEQNUM_INCR(x)            (x = ((x+1) % 0xFF))

/*! Message creation macro for peer sync module messages. */
#define MAKE_PEER_SYNC_MESSAGE(TYPE) \
    TYPE##_T *message = PanicUnlessNew(TYPE##_T);

/*! Defines used to establish offsets of components in the peer sync message
 *  sent to the peer. */
/*!@{ */
#define PEER_SYNC_MSG_SIZE                          15         /*!< Message length */
#define PEER_SYNC_MSG_BATT_OFFSET                   0
#define PEER_SYNC_MSG_ADDR_TYPE_OFFSET              2
#define PEER_SYNC_MSG_ADDR_OFFSET_LAP               3
#define PEER_SYNC_MSG_ADDR_OFFSET_UAP               6
#define PEER_SYNC_MSG_ADDR_OFFSET_NAP               7
#define PEER_SYNC_MSG_TWS_VERSION_OFFSET            9
#define PEER_SYNC_MSG_STATE_OFFSET                  11
#define PEER_SYNC_MSG_STATE2_OFFSET                 12
#define PEER_SYNC_MSG_TX_SEQNUM_OFFSET              13
#define PEER_SYNC_MSG_RX_SEQNUM_OFFSET              14
/*!@} */

/*! Defines for boolean elements of the state components in the peer sync message. */
/*!@{ */
#define PEER_SYNC_MSG_STATE_A2DP_CONNECTED          (1 << 0)
#define PEER_SYNC_MSG_STATE_A2DP_STREAMING          (1 << 1)
#define PEER_SYNC_MSG_STATE_AVRCP_CONNECTED         (1 << 2)
#define PEER_SYNC_MSG_STATE_HFP_CONNECTED           (1 << 3)
#define PEER_SYNC_MSG_STATE_IN_CASE                 (1 << 4)
#define PEER_SYNC_MSG_STATE_IN_EAR                  (1 << 5)
#define PEER_SYNC_MSG_STATE_STARTUP                 (1 << 6)
#define PEER_SYNC_MSG_STATE_RULES_IN_PROGRESS       (1 << 7)

#define PEER_SYNC_MSG_STATE2_HANDSET_PAIRED         (1 << 0)
#define PEER_SYNC_MSG_STATE2_PAIRING_HANDSET        (1 << 1)
#define PEER_SYNC_MSG_STATE2_ANC_ENABLED            (1 << 2)  /*!< Added to set the ANC status in Peer sync Message */
#define PEER_SYNC_MSG_STATE2_SCO_ACTIVE             (1 << 4)
#define PEER_SYNC_MSG_STATE2_DFU_IN_PROGRESS        (1 << 5)
#define PEER_SYNC_MSG_STATE2_ADVERTISING            (1 << 6)
#define PEER_SYNC_MSG_STATE2_BLE_CONNECTED          (1 << 7)
/*!@} */

/*! Macros to get components from a peer sync raw message. */
/*!@{ */
#define PEER_SYNC_GET_STATE(x)          (x[PEER_SYNC_MSG_STATE_OFFSET])
#define PEER_SYNC_GET_STATE2(x)         (x[PEER_SYNC_MSG_STATE2_OFFSET])
#define PEER_SYNC_GET_BATTERY(x)        (appPeerSyncReadUint16(&(x[PEER_SYNC_MSG_BATT_OFFSET])))
#define PEER_SYNC_GET_HANDSET_LAP(x)    (appPeerSyncReadUint24(&(x[PEER_SYNC_MSG_ADDR_OFFSET_LAP])))
#define PEER_SYNC_GET_HANDSET_UAP(x)    (x[PEER_SYNC_MSG_ADDR_OFFSET_UAP])
#define PEER_SYNC_GET_HANDSET_NAP(x)    (appPeerSyncReadUint16(&(x[PEER_SYNC_MSG_ADDR_OFFSET_NAP])))
#define PEER_SYNC_GET_TWS_VERSION(x)    (appPeerSyncReadUint16(&(x[PEER_SYNC_MSG_TWS_VERSION_OFFSET])))
#define PEER_SYNC_GET_TX_SEQNUM(x)      (x[PEER_SYNC_MSG_TX_SEQNUM_OFFSET])
#define PEER_SYNC_GET_RX_SEQNUM(x)      (x[PEER_SYNC_MSG_RX_SEQNUM_OFFSET])
/*!@} */

/*! Macros to get components from a peer sync raw message. */
/*!@{ */
#define PEER_SYNC_SET_STATE(x,val)          (x[PEER_SYNC_MSG_STATE_OFFSET] = val)
#define PEER_SYNC_SET_STATE2(x,val)         (x[PEER_SYNC_MSG_STATE2_OFFSET] = val)
#define PEER_SYNC_SET_BATTERY(x,val)        (appPeerSyncWriteUint16(&(x[PEER_SYNC_MSG_BATT_OFFSET]),val))
#define PEER_SYNC_SET_ADDR_TYPE(x,val)      (x[PEER_SYNC_MSG_ADDR_TYPE_OFFSET] = val)
#define PEER_SYNC_SET_HANDSET_LAP(x,val)    (appPeerSyncWriteUint24(&(x[PEER_SYNC_MSG_ADDR_OFFSET_LAP]),val))
#define PEER_SYNC_SET_HANDSET_UAP(x,val)    (x[PEER_SYNC_MSG_ADDR_OFFSET_UAP] = val)
#define PEER_SYNC_SET_HANDSET_NAP(x,val)    (appPeerSyncWriteUint16(&(x[PEER_SYNC_MSG_ADDR_OFFSET_NAP]),val))
#define PEER_SYNC_SET_TWS_VERSION(x,val)    (appPeerSyncWriteUint16(&(x[PEER_SYNC_MSG_TWS_VERSION_OFFSET]),val))
#define PEER_SYNC_SET_TX_SEQNUM(x,seq)      (x[PEER_SYNC_MSG_TX_SEQNUM_OFFSET] = seq)
#define PEER_SYNC_SET_RX_SEQNUM(x,seq)      (x[PEER_SYNC_MSG_RX_SEQNUM_OFFSET] = seq)
/*!@} */

/*! Macros to get boolean values from state and pairing components. */
/*!@{ */
#define PEER_SYNC_STATE_IS_A2DP_CONNECTED(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_A2DP_CONNECTED) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_A2DP_STREAMING(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_A2DP_STREAMING) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_AVRCP_CONNECTED(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_AVRCP_CONNECTED) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_HFP_CONNECTED(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_HFP_CONNECTED) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_IN_CASE(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_IN_CASE) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_IN_EAR(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_IN_EAR) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_STARTUP(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_STARTUP) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_RULES_IN_PROGRESS(x) \
    ((PEER_SYNC_GET_STATE(x) & PEER_SYNC_MSG_STATE_RULES_IN_PROGRESS) ? TRUE : FALSE)

#define PEER_SYNC_STATE_IS_HANDSET_PAIRED(x) \
    ((PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_HANDSET_PAIRED) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_HANDSET_PAIRING(x) \
    ((PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_PAIRING_HANDSET) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_SCO_ACTIVE(x) \
    ((PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_SCO_ACTIVE) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_DFU_IN_PROGRESS(x) \
    ((PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_DFU_IN_PROGRESS) ? TRUE : FALSE)
#define PEER_SYNC_STATE_IS_ADVERTISING(x) \
   (!!(PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_ADVERTISING))
#define PEER_SYNC_STATE_IS_BLE_CONNECTED(x) \
   (!!(PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_BLE_CONNECTED))
#define PEER_SYNC_STATE_IS_ANC_ENABLED(x) \
   ((PEER_SYNC_GET_STATE2(x) & PEER_SYNC_MSG_STATE2_ANC_ENABLED) ? TRUE : FALSE)
/*!@} */

/*! Macros to set boolean values in state and pairing components. */
/*!@{ */
#define PEER_SYNC_STATE_SET_A2DP_CONNECTED(x) \
    ((x) ? PEER_SYNC_MSG_STATE_A2DP_CONNECTED : 0)
#define PEER_SYNC_STATE_SET_A2DP_STREAMING(x) \
    ((x) ? PEER_SYNC_MSG_STATE_A2DP_STREAMING : 0)
#define PEER_SYNC_STATE_SET_AVRCP_CONNECTED(x) \
    ((x) ? PEER_SYNC_MSG_STATE_AVRCP_CONNECTED : 0)
#define PEER_SYNC_STATE_SET_HFP_CONNECTED(x) \
    ((x) ? PEER_SYNC_MSG_STATE_HFP_CONNECTED : 0)
#define PEER_SYNC_STATE_SET_IS_STARTUP(x) \
    ((x) ? PEER_SYNC_MSG_STATE_STARTUP : 0)
#define PEER_SYNC_STATE_SET_IS_IN_CASE(x) \
    ((x) ? PEER_SYNC_MSG_STATE_IN_CASE : 0)
#define PEER_SYNC_STATE_SET_IS_IN_EAR(x) \
    ((x) ? PEER_SYNC_MSG_STATE_IN_EAR : 0)
#define PEER_SYNC_STATE_SET_RULES_IN_PROGRESS(x) \
    ((x) ? PEER_SYNC_MSG_STATE_RULES_IN_PROGRESS : 0)

#define PEER_SYNC_STATE_SET_IS_HANDSET_PAIRED(x) \
    ((x) ? PEER_SYNC_MSG_STATE2_HANDSET_PAIRED : 0)
#define PEER_SYNC_STATE_SET_IS_HANDSET_PAIRING(x) \
    ((x) ? PEER_SYNC_MSG_STATE2_PAIRING_HANDSET : 0)
#define PEER_SYNC_STATE_SET_SCO_ACTIVE(x) \
    ((x) ? PEER_SYNC_MSG_STATE2_SCO_ACTIVE : 0)
#define PEER_SYNC_STATE_SET_DFU_IN_PROGRESS(x) \
    ((x) ? PEER_SYNC_MSG_STATE2_DFU_IN_PROGRESS : 0)
#define PEER_SYNC_STATE_SET_IS_ADVERTISING(x) \
      ((x) ? PEER_SYNC_MSG_STATE2_ADVERTISING : 0)
#define PEER_SYNC_STATE_SET_IS_BLE_CONNECTED(x) \
      ((x) ? PEER_SYNC_MSG_STATE2_BLE_CONNECTED : 0)
#define PEER_SYNC_STATE_SET_IS_ANC_ENABLED(x) \
      ((x) ? PEER_SYNC_MSG_STATE2_ANC_ENABLED : 0)
/*!@} */

static uint16 appPeerSyncReadUint16(const uint8 *data)
{
    return data[0] + ((uint16)data[1] << 8);
}

static uint24 appPeerSyncReadUint24(const uint8 *data)
{
    return data[0] + ((uint16)data[1] << 8) + ((uint32)data[2] << 16);
}

static void appPeerSyncWriteUint16(uint8 *data, uint16 val)
{
    data[0] = val & 0xFF;
    data[1] = (val >> 8) & 0xFF;
}

static void appPeerSyncWriteUint24(uint8 *data, uint24 val)
{
    data[0] = val & 0xFF;
    data[1] = (val >> 8) & 0xFF;
    data[2] = (val >> 16) & 0xFF;
}

static void appPeerSyncUpdateA2dpConnected(bool connected)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_a2dp_state = ps->peer_a2dp_connected;

    /* update the state with latest from peer */
    ps->peer_a2dp_connected = connected;

    /* first profile connected or last profile disconnected then inform rules
     * that peer handset has connected/disconnected */
    if ((current_a2dp_state != connected) &&
        (!ps->peer_hfp_connected && !ps->peer_avrcp_connected))
    {
        DEBUG_LOGF("appPeerSyncUpdateA2dpConnected Prev %u New %u", current_a2dp_state, connected);
        appConnRulesSetEvent(appGetSmTask(),
                             connected ? RULE_EVENT_PEER_HANDSET_CONNECTED
                                       : RULE_EVENT_PEER_HANDSET_DISCONNECTED);
    }
#ifdef PEER_SYNC_PROFILE_EVENTS
    /* generate per profile connected/disconnected events for peer-handset link */
    if (!current_a2dp_state && connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_A2DP_CONNECTED);
    }
    else if (current_a2dp_state && !connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_A2DP_DISCONNECTED);
    }
#endif
}

static void appPeerSyncUpdateAvrcpConnected(bool connected)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_avrcp_state = ps->peer_avrcp_connected;

    ps->peer_avrcp_connected = connected;

    /* first profile connected or last profile disconnected then inform rules
     * that peer handset has connected/disconnected */
    if ((current_avrcp_state != connected) &&
        (!ps->peer_hfp_connected && !ps->peer_a2dp_connected))
    {
        DEBUG_LOGF("appPeerSyncUpdateAvrcpConnected Prev %u New %u", current_avrcp_state, connected);
        appConnRulesSetEvent(appGetSmTask(),
                             connected ? RULE_EVENT_PEER_HANDSET_CONNECTED
                                       : RULE_EVENT_PEER_HANDSET_DISCONNECTED);
    }
#ifdef PEER_SYNC_PROFILE_EVENTS
    /* generate per profile connected/disconnected events for peer-handset link */
    if (!current_avrcp_state && connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_AVRCP_CONNECTED);
    }
    else if (current_avrcp_state && !connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_AVRCP_DISCONNECTED);
    }
#endif
}

static void appPeerSyncUpdateHfpConnected(bool connected)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_hfp_state = ps->peer_hfp_connected;

    /* update state as if calls to set an event in the rules engine later
     * may check this state */
    ps->peer_hfp_connected = connected;

    /* first profile connected or last profile disconnected then inform rules
     * that peer handset has connected/disconnected */
    if ((current_hfp_state != connected) &&
        (!ps->peer_a2dp_connected && !ps->peer_avrcp_connected))
    {
        DEBUG_LOGF("appPeerSyncUpdateHfpConnected Prev %u New %u", current_hfp_state, connected);
        appConnRulesSetEvent(appGetSmTask(),
                             connected ? RULE_EVENT_PEER_HANDSET_CONNECTED
                                       : RULE_EVENT_PEER_HANDSET_DISCONNECTED);
    }
#ifdef PEER_SYNC_PROFILE_EVENTS
    /* generate per profile connected/disconnected events for peer-handset link */
    if (!current_hfp_state && connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_HFP_CONNECTED);
    }
    else if (current_hfp_state && !connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_HFP_DISCONNECTED);
    }
#endif
}

static void appPeerSyncUpdatePeerInCase(bool peer_in_case)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_peer_in_case_state = ps->peer_in_case;

    ps->peer_in_case = peer_in_case;

    if (!current_peer_in_case_state && peer_in_case)
    {
        DEBUG_LOGF("appPeerSyncUpdatePeerInCase Prev %u New %u", current_peer_in_case_state, peer_in_case);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_IN_CASE);
    }
    else if (current_peer_in_case_state && !peer_in_case)
    {
        DEBUG_LOGF("appPeerSyncUpdatePeerInCase Prev %u New %u", current_peer_in_case_state, peer_in_case);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_OUT_CASE);
    }
}

static void appPeerSyncUpdatePeerInEar(bool peer_in_ear)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_peer_in_ear_state = ps->peer_in_ear;

    ps->peer_in_ear = peer_in_ear;

    if (!current_peer_in_ear_state && peer_in_ear)
    {
        DEBUG_LOGF("appPeerSyncUpdatePeerInEar Prev %u New %u", current_peer_in_ear_state, peer_in_ear);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_IN_EAR);
    }
    else if (current_peer_in_ear_state && !peer_in_ear)
    {
        DEBUG_LOGF("appPeerSyncUpdatePeerInEar Prev %u New %u", current_peer_in_ear_state, peer_in_ear);
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_OUT_EAR);
    }
}


static void appPeerSyncUpdateAdvertising(bool advertising)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_advertising_state = ps->peer_advertising;

    ps->peer_advertising = advertising;

    if (current_advertising_state != advertising)
    {
        DEBUG_LOGF("appPeerSyncUpdateAdvertising Prev %u New %u", current_advertising_state, advertising);

        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);
    }
}


static void appPeerSyncUpdateBleConnection(bool ble)
{
    peerSyncTaskData* ps = appGetPeerSync();
    bool current_ble_state = ps->peer_ble_connected;

    ps->peer_ble_connected = ble;

    if (current_ble_state != ble)
    {
        DEBUG_LOGF("appPeerSyncUpdateBleConnection Prev %u New %u", current_ble_state, ble);

        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_BLE_CONNECTABLE_CHANGE);
    }
}
/*
 *
 * Update the peer device ANC state based on receieved ANC state of peer device
 *
 */
static void appPeerSyncUpdateAncState(bool anc)
{
    DEBUG_LOG("appPeerSyncUpdateAncState");

    peerSyncTaskData* ps = appGetPeerSync();
    bool current_anc_state = ps->peer_anc_enabled;
    bool peer_phy_state = ps->peer_in_case;
    ps->peer_anc_enabled = anc;

    if(appSmIsOutOfCase() && (!peer_phy_state))
    {
        bool actual_anc_state = appKymeraAncIsEnabled();

        DEBUG_LOGF("appPeerSyncUpdateAncState Prev %u New %u", current_anc_state, anc);

        if (anc && (!actual_anc_state))
        {
            /* Update remote devcie ANC state when we receive the ANC update request*/
            if (anc != current_anc_state)
            {
                appKymeraAncDelayedEnable(ANC_PEER_REMOTE_DEVICE_TIME_DELAY);
            }
             /* Update local device ANC state when we receive the ANC
              * confirmation from remote device */
            else
            {
                appKymeraAncDelayedEnable(ANC_PEER_LOCAL_DEVICE_TIME_DELAY);
            }

        }
        else if ((!anc) && actual_anc_state)
        {
            /* Update remote devcie ANC state when we receive the ANC update request*/
            if (anc != current_anc_state)
            {
                appKymeraAncDelayedDisable(ANC_PEER_REMOTE_DEVICE_TIME_DELAY);
            }
            /* Update local device ANC state when we receive the ANC
             * confirmation from remote device */
            else
            {
                appKymeraAncDelayedDisable(ANC_PEER_LOCAL_DEVICE_TIME_DELAY);
            }
       }
    }
}

/*! \brief Dump the contents of a peer sync message to the console.

    \param[in] msg Pointer to a peer sync message.
*/
static void appPeerSyncMsgDebug(const uint8* msg)
{
    uint32 bitfield;

    DEBUG_LOGF("    appPeerSync txseq %x rxseq %x",
                PEER_SYNC_GET_TX_SEQNUM(msg),
                PEER_SYNC_GET_RX_SEQNUM(msg));

    DEBUG_LOGF("    appPeerSync battery %u, bdaddr %04x%08x, version %04x",
                PEER_SYNC_GET_BATTERY(msg),
                PEER_SYNC_GET_HANDSET_NAP(msg),
                (PEER_SYNC_GET_HANDSET_UAP(msg) << 24) | PEER_SYNC_GET_HANDSET_LAP(msg),
                PEER_SYNC_GET_TWS_VERSION(msg));

    bitfield =  (!!PEER_SYNC_STATE_IS_STARTUP(msg)           << 28) |
                (!!PEER_SYNC_STATE_IS_RULES_IN_PROGRESS(msg) << 24) |
                (!!PEER_SYNC_STATE_IS_A2DP_CONNECTED(msg)    << 20) |
                (!!PEER_SYNC_STATE_IS_AVRCP_CONNECTED(msg)   << 16) |
                (!!PEER_SYNC_STATE_IS_HFP_CONNECTED(msg)     << 12) |
                (!!PEER_SYNC_STATE_IS_A2DP_STREAMING(msg)    << 8)  |
                (!!PEER_SYNC_STATE_IS_SCO_ACTIVE(msg)        << 4)  |
                (!!PEER_SYNC_STATE_IS_DFU_IN_PROGRESS(msg)   << 0);
    DEBUG_LOGF("    appPeerSync (startup|RIP|a2dp_conn|avrcp_conn|hfp_conn|a2dp_streaming|sco|dfu)=%08x", bitfield);

    bitfield =  (!!PEER_SYNC_STATE_IS_IN_CASE(msg)           << 20) |
                (!!PEER_SYNC_STATE_IS_IN_EAR(msg)            << 16) |
                (!!PEER_SYNC_STATE_IS_HANDSET_PAIRING(msg)   << 12) |
                (!!PEER_SYNC_STATE_IS_HANDSET_PAIRED(msg)    <<  8) | 
                (!!PEER_SYNC_STATE_IS_ADVERTISING(msg)       <<  4) |
                (!!PEER_SYNC_STATE_IS_BLE_CONNECTED(msg)     <<  0);
    DEBUG_LOGF("    appPeerSync (in_case|in_ear|is_pairing|have_handset_pairing|advertising|BLE)=%06x", bitfield);
}

/*! \brief Wrapper for debug print of peer sync message transmit.

    \param[in] msg Pointer to a peer sync message.
 */
static void appPeerSyncTxMsgDebug(const uint8* msg)
{
    DEBUG_LOG("appPeerSyncTxMsgDebug TX");
    appPeerSyncMsgDebug(msg);
}

/*! \brief Wrapper for debug print of peer sync message receive.

    \param[in] msg Pointer to a peer sync message.
 */
static void appPeerSyncRxMsgDebug(const uint8* msg)
{
    DEBUG_LOG("appPeerSyncRxMsgDebug RX");
    appPeerSyncMsgDebug(msg);
}

/*! \brief Send a #PEER_SYNC_STATUS message to registered clients. */
static void appPeerSyncSendStatus(void)
{
    MAKE_PEER_SYNC_MESSAGE(PEER_SYNC_STATUS);
    peerSyncTaskData* ps = appGetPeerSync();
    message->peer_sync_complete = ps->peer_sync_state == PEER_SYNC_COMPLETE ? TRUE : FALSE;
    appTaskListMessageSend(ps->peer_sync_status_tasks, PEER_SYNC_STATUS, message);
}

/*! \brief Send a peer sync to peer earbud.
 */
void appPeerSyncSend(bool response)
{
    bdaddr peer_addr;
    bdaddr active_handset_addr;
    bdaddr paired_handset_addr;
    uint16 tws_version = DEVICE_TWS_UNKNOWN;
    peerSyncTaskData* ps = appGetPeerSync();

    DEBUG_LOGF("appPeerSyncSend (response %u)", response);

    /* Can only send this if we have a peer earbud */
    if (appDeviceGetPeerBdAddr(&peer_addr))
    {
        /* Try and find last connected handset address, may not exist.
           Only set active bdaddr in peer sync if a profile connection to the handset exists. */
        BdaddrSetZero(&active_handset_addr);
        BdaddrSetZero(&paired_handset_addr);
        if (appDeviceGetHandsetBdAddr(&paired_handset_addr))
        {
            if (appDeviceIsHandsetAnyProfileConnected())
            {
                active_handset_addr = paired_handset_addr;
                tws_version = appDeviceTwsVersion(&active_handset_addr);
            }
        }

        /* Handle peer sync state and transmit sequence number changes */
        if (!response)
        {
            /* mark sent peer sync as invalid, until we get a confirmation of
             * delivery of the peer sync TX */
            PEER_SYNC_STATE_CLEAR_SENT(ps->peer_sync_state);
            PEER_SYNC_STATE_SET_SENDING(ps->peer_sync_state);

            /* if this isn't a response sync, also mark received as invalid,
             * so that peer sync isn't complete until we get back the response
             * sync from the peer. Prevents rules firing that require up to
             * date peer sync information from peer after local state has
             * changed */
            PEER_SYNC_STATE_CLEAR_RECEIVED(ps->peer_sync_state);
            
            /* not a response sync, increment our TX seqnum */
            PEER_SYNC_SEQNUM_INCR(ps->peer_sync_tx_seqnum);
        }
        
        /* remember if the last peer sync said rules were in progress, such
         * that if the rules engine sends a CONN_RULES_NOP we know if we
         * need to send another peer sync to clear the peer_rules_in_progress
         * for the peer */
        ps->sent_in_progress = appConnRulesInProgress();
        /* Store battery level we sent, so we can compare with peer */
        ps->sync_battery_level = appBatteryGetVoltage();

        /* build the peer sync message */
        uint8 message[PEER_SYNC_MSG_SIZE];
        const uint8 state = (PEER_SYNC_STATE_SET_A2DP_CONNECTED(appDeviceIsHandsetA2dpConnected())) +
                            (PEER_SYNC_STATE_SET_A2DP_STREAMING(appDeviceIsHandsetA2dpStreaming())) +
                            (PEER_SYNC_STATE_SET_AVRCP_CONNECTED(appDeviceIsHandsetAvrcpConnected())) +
                            (PEER_SYNC_STATE_SET_HFP_CONNECTED(appDeviceIsHandsetHfpConnected())) +
                            (PEER_SYNC_STATE_SET_IS_STARTUP(appGetState() == APP_STATE_STARTUP)) +
                            (PEER_SYNC_STATE_SET_IS_IN_CASE(appSmIsInCase())) +
                            (PEER_SYNC_STATE_SET_IS_IN_EAR(appSmIsInEar())) +
                            (PEER_SYNC_STATE_SET_RULES_IN_PROGRESS(ps->sent_in_progress));
        const uint8 state2 = (PEER_SYNC_STATE_SET_IS_HANDSET_PAIRED(!BdaddrIsZero(&paired_handset_addr))) +
                             (PEER_SYNC_STATE_SET_IS_HANDSET_PAIRING(appGetState() == APP_STATE_HANDSET_PAIRING)) +
                             (PEER_SYNC_STATE_SET_SCO_ACTIVE(appHfpIsScoActive())) +
                             (PEER_SYNC_STATE_SET_DFU_IN_PROGRESS(appSmIsInDfuMode())) + 
                             (PEER_SYNC_STATE_SET_IS_ADVERTISING(appSmIsBleAdvertising())) +
                             (PEER_SYNC_STATE_SET_IS_BLE_CONNECTED(appSmHasBleConnection())) +
                             (PEER_SYNC_STATE_SET_IS_ANC_ENABLED(ps->peer_anc_enabled));
        PEER_SYNC_SET_BATTERY(message, ps->sync_battery_level);
        PEER_SYNC_SET_ADDR_TYPE(message, 0); // AVRCP_PEER_CMD_ADD_LINK_KEY_ADDR_TYPE_BREDR
        PEER_SYNC_SET_HANDSET_LAP(message, active_handset_addr.lap);
        PEER_SYNC_SET_HANDSET_UAP(message, active_handset_addr.uap);
        PEER_SYNC_SET_HANDSET_NAP(message, active_handset_addr.nap);
        PEER_SYNC_SET_TWS_VERSION(message, tws_version);
        PEER_SYNC_SET_STATE(message, state);
        PEER_SYNC_SET_STATE2(message, state2);
        PEER_SYNC_SET_TX_SEQNUM(message, ps->peer_sync_tx_seqnum);
        PEER_SYNC_SET_RX_SEQNUM(message, ps->peer_sync_rx_seqnum);

        /* dump contents to debug */
        appPeerSyncTxMsgDebug(message);

        /* send message on our peer signalling peer sync channel */
        appPeerSigMsgChannelTxRequest(&ps->task, &peer_addr, PEER_SIG_MSG_CHANNEL_PEER_SYNC,
                                      message, PEER_SYNC_MSG_SIZE);

        /* reset the event marking peer sync as valid, we'll set it
         * again once peer sync is completed */
        appConnRulesResetEvent(RULE_EVENT_PEER_SYNC_VALID);
    }
}

/*! \brief Handle confirmation of peer sync transmission.
 */
static void appPeerSyncHandlePeerSigMsgChannelTxCfm(const PEER_SIG_MSG_CHANNEL_TX_CFM_T *cfm)
{
    peerSyncTaskData* ps = appGetPeerSync();

    if (cfm->status == peerSigStatusSuccess)
    {
        DEBUG_LOG("appPeerSyncHandlePeerSigMsgChannelTxCfm, success");

        /* Update peer sync state */
        PEER_SYNC_STATE_CLEAR_SENDING(ps->peer_sync_state);
        PEER_SYNC_STATE_SET_SENT(ps->peer_sync_state);

        /* have we successfully received and sent peer sync messages? */
        if (appPeerSyncIsComplete())
        {
            /* Generate peer sync valid event, may cause previously deferred
             * rules to be evaluated again  */
            appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_SYNC_VALID);
        }
    }
    else
    {
        DEBUG_LOGF("appPeerSyncHandlePeerSigMsgChannelTxCfm, failed, status %u", cfm->status);
        /*! \todo should we clear RULE_EVENT_PEER_SYNC_VALID on failure here? */

        /* Reset peer sync state here so we can determine later on that peer sync has failed */
        PEER_SYNC_STATE_RESET(ps->peer_sync_state);
    }
    
    /* always tell peer sync status clients the current state */
    appPeerSyncSendStatus();
}

/*! \brief Handle indication of incoming peer sync from peer.
 */
static void appPeerSyncHandlePeerSigMsgChannelTxInd(const PEER_SIG_MSG_CHANNEL_RX_IND_T *ind)
{
    peerSyncTaskData* ps = appGetPeerSync();
    uint8 was_supported;

    if (ind->msg_size != PEER_SYNC_MSG_SIZE)
    {
        DEBUG_LOGF("appPeerSyncHandlePeerSigMsgChannelTxInd, bad RX len %u", ind->msg_size);
        return;
    }

    /* dump incoming peer sync to debug */
    appPeerSyncRxMsgDebug(ind->msg);

    /* remember latest peer state */
    ps->peer_battery_level       = PEER_SYNC_GET_BATTERY(ind->msg);
    ps->peer_handset_addr.lap    = PEER_SYNC_GET_HANDSET_LAP(ind->msg);
    ps->peer_handset_addr.uap    = PEER_SYNC_GET_HANDSET_UAP(ind->msg);
    ps->peer_handset_addr.nap    = PEER_SYNC_GET_HANDSET_NAP(ind->msg);
    ps->peer_handset_tws         = PEER_SYNC_GET_TWS_VERSION(ind->msg);
    ps->peer_a2dp_streaming      = PEER_SYNC_STATE_IS_A2DP_STREAMING(ind->msg);
    ps->peer_is_pairing          = PEER_SYNC_STATE_IS_HANDSET_PAIRING(ind->msg);
    ps->peer_has_handset_pairing = PEER_SYNC_STATE_IS_HANDSET_PAIRED(ind->msg);
    ps->peer_rules_in_progress   = PEER_SYNC_STATE_IS_RULES_IN_PROGRESS(ind->msg);
    ps->peer_sco_active          = PEER_SYNC_STATE_IS_SCO_ACTIVE(ind->msg);
    ps->peer_dfu_in_progress     = PEER_SYNC_STATE_IS_DFU_IN_PROGRESS(ind->msg);

    /* remember and possibly generate local events for some peer state changes */
    appPeerSyncUpdatePeerInCase(PEER_SYNC_STATE_IS_IN_CASE(ind->msg));
    appPeerSyncUpdatePeerInEar(PEER_SYNC_STATE_IS_IN_EAR(ind->msg));
    appPeerSyncUpdateA2dpConnected(PEER_SYNC_STATE_IS_A2DP_CONNECTED(ind->msg));
    appPeerSyncUpdateAvrcpConnected(PEER_SYNC_STATE_IS_AVRCP_CONNECTED(ind->msg));
    appPeerSyncUpdateHfpConnected(PEER_SYNC_STATE_IS_HFP_CONNECTED(ind->msg));
    appPeerSyncUpdateAdvertising(PEER_SYNC_STATE_IS_ADVERTISING(ind->msg));
    appPeerSyncUpdateBleConnection(PEER_SYNC_STATE_IS_BLE_CONNECTED(ind->msg));
    appPeerSyncUpdateAncState(PEER_SYNC_STATE_IS_ANC_ENABLED(ind->msg));
    /* RX sequence number from peer indicates it has seen our peer sync TX 
     * this is a response sync matching the latest TX we have sent */
    if (PEER_SYNC_GET_RX_SEQNUM(ind->msg) == ps->peer_sync_tx_seqnum)
    {
        /* Update peer sync RX state */
        PEER_SYNC_STATE_SET_RECEIVED(ps->peer_sync_state);
    }

    was_supported = appDeviceSetProfileConnectedAndSupportedFlagsFromPeer(
                        &ps->peer_handset_addr,
                        ps->peer_a2dp_connected,
                        ps->peer_avrcp_connected,
                        ps->peer_hfp_connected,
                        hfp_handsfree_107_profile); // TODO: Get HFP profile version from peer

    if (!(was_supported & DEVICE_PROFILE_A2DP) & ps->peer_a2dp_connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_A2DP_SUPPORTED);
    }
    if (!(was_supported & DEVICE_PROFILE_AVRCP) & ps->peer_avrcp_connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_AVRCP_SUPPORTED);
    }
    if (!(was_supported & DEVICE_PROFILE_HFP) & ps->peer_hfp_connected)
    {
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_HFP_SUPPORTED);
    }

    /* TX seqnum has changed from the last we saw, so this is a new peer sync, which
     * requires a response peer sync from us */
    /* Also ensures we send a sync to a startup sync to get a rebooted peer 
     * earbud back to a completed sync state */
    if (PEER_SYNC_GET_TX_SEQNUM(ind->msg) != ps->peer_sync_rx_seqnum)
    {
        ps->peer_sync_rx_seqnum = PEER_SYNC_GET_TX_SEQNUM(ind->msg);
        appPeerSyncSend(TRUE);
    }

    /* Set peer sync valid event if we've received and successfully send peer sync messages */
    if (appPeerSyncIsComplete())
    {
        DEBUG_LOG("appPeerSyncHandlePeerSigMsgChannelTxInd, peer sync complete");

        /* inform status clients that peer sync is now complete */
        appPeerSyncSendStatus();

        /* Run peer sync valid rules */
        appConnRulesSetEvent(appGetSmTask(), RULE_EVENT_PEER_SYNC_VALID);
    }
}

/*! \brief Determine if peer sync is complete.
 */
bool appPeerSyncIsComplete(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return ps->peer_sync_state == PEER_SYNC_COMPLETE;
}

/*! \brief NOP indicates the rules engine now has no rules in progress.
 * This is equivalent to calling appConnRulesInProgress() and getting
 * a FALSE return value.
 */
static void appPeerSyncHandleConnRulesNop(void) 
{
    peerSyncTaskData* ps = appGetPeerSync();

    DEBUG_LOG("appPeerSyncHandleConnRulesNop");

    /* if we've sent a peer sync message with the rules in progress
     * * bit set, then send another to inform peer earbud this is no
     * * longer the case. */
    if (ps->sent_in_progress)
    {
        appPeerSyncSend(FALSE);
    }
}

/*! \brief Peer Sync module message handler. */
static void appPeerSyncHandleMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
        /* Peer Signalling Messages */
        case PEER_SIG_MSG_CHANNEL_RX_IND:
            appPeerSyncHandlePeerSigMsgChannelTxInd((PEER_SIG_MSG_CHANNEL_RX_IND_T*)message);
            break;
        case PEER_SIG_MSG_CHANNEL_TX_CFM:
            appPeerSyncHandlePeerSigMsgChannelTxCfm((PEER_SIG_MSG_CHANNEL_TX_CFM_T*)message);
            break;

        /* Rules Engine Messages */
        case CONN_RULES_NOP:
            appPeerSyncHandleConnRulesNop();
            break;

        default:
            appHandleUnexpected(id);
            break;
    }
}

/*! \brief Clear peer sync for both sent and received status. */
void appPeerSyncResetSync(void)
{
    DEBUG_LOG("appPeerSyncResetSync");
    PEER_SYNC_STATE_RESET(appGetPeerSync()->peer_sync_state);
}

/*! \brief Register a task to receive #PEER_SYNC_STATUS messages. */
void appPeerSyncStatusClientRegister(Task task)
{
    peerSyncTaskData* ps = appGetPeerSync();
    DEBUG_LOGF("appPeerSyncStatusClientRegister %p", task);
    appTaskListAddTask(ps->peer_sync_status_tasks, task);
}

/*! \brief Unregister a task to stop receiving #PEER_SYNC_STATUS messages. */
void appPeerSyncStatusClientUnregister(Task task)
{
    peerSyncTaskData* ps = appGetPeerSync();
    DEBUG_LOGF("appPeerSyncStatusClientUnregister %p", task);
    appTaskListRemoveTask(ps->peer_sync_status_tasks, task);
}

/*! \brief Setup the peer sync module, before it can used. */
void appPeerSyncInit(void)
{
    peerSyncTaskData* ps = appGetPeerSync();

    DEBUG_LOG("appPeerSyncInit");

    memset(ps, 0, sizeof(*ps));
    ps->task.handler = appPeerSyncHandleMessage;

    ps->peer_sync_state = PEER_SYNC_NONE;
    ps->peer_sync_tx_seqnum = 0xFF;
    ps->peer_sync_rx_seqnum = 0xFF;
    
    ps->peer_sync_status_tasks = appTaskListInit();

    /* Register a channel for peer signalling */
    appPeerSigMsgChannelTaskRegister(&ps->task, PEER_SIG_MSG_CHANNEL_PEER_SYNC);

    /* register with rules engine to receive notifications that no rules
     * are in progress. Used in peer sync messages. */
    appConnRulesNopClientRegister(&ps->task);
}

void appPeerSyncGetPeerBatteryLevel(uint16 *battery_level, uint16 *peer_battery_level)
{
    peerSyncTaskData* ps = appGetPeerSync();

    *battery_level = ps->sync_battery_level;
    *peer_battery_level = ps->peer_battery_level;
}

void appPeerSyncGetPeerHandsetAddr(bdaddr *peer_handset_addr)
{
    peerSyncTaskData* ps = appGetPeerSync();
    *peer_handset_addr = ps->peer_handset_addr;
}

bool appPeerSyncIsPeerHandsetA2dpConnected(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return appPeerSyncIsComplete() && ps->peer_a2dp_connected;
}

bool appPeerSyncIsPeerHandsetA2dpStreaming(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return appPeerSyncIsComplete() && ps->peer_a2dp_streaming;
}

bool appPeerSyncIsPeerScoActive(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return appPeerSyncIsComplete() && ps->peer_sco_active;
}

bool appPeerSyncIsPeerHandsetAvrcpConnected(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return appPeerSyncIsComplete() && ps->peer_avrcp_connected;
}

bool appPeerSyncIsPeerHandsetHfpConnected(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return appPeerSyncIsComplete() && ps->peer_hfp_connected;
}

bool appPeerSyncIsPeerHandsetTws(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return appPeerSyncIsComplete() && (ps->peer_handset_tws == DEVICE_TWS_VERSION);
}

bool appPeerSyncIsPeerInCase(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    if (appPeerSyncIsComplete())
        return ps->peer_in_case;
    else
        return PHY_STATE_UNKNOWN;
}

bool appPeerSyncIsPeerInEar(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    if (appPeerSyncIsComplete())
        return ps->peer_in_ear;
    else
        return PHY_STATE_UNKNOWN;
}

bool appPeerSyncIsPeerPairing(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    if (appPeerSyncIsComplete())
        return ps->peer_is_pairing;
    else
        return FALSE;
}


bool appPeerSyncIsPeerAdvertising(void)
{
    peerSyncTaskData* ps = appGetPeerSync();

    if (appPeerSyncIsComplete())
        return ps->peer_advertising;

    return FALSE;
}


bool appPeerSyncIsPeerBleConnected(void)
{
    peerSyncTaskData* ps = appGetPeerSync();

    if (appPeerSyncIsComplete())
        return ps->peer_ble_connected;

    return FALSE;
}

/*

  This function is used to Check Peer earbud ANC Status

*/
bool appPeerSyncIsPeerAncEnabled(void)
{
    DEBUG_LOG("appPeerSyncIsPeerAncEnabled");
    peerSyncTaskData* ps = appGetPeerSync();

    if (appPeerSyncIsComplete())
        return ps->peer_anc_enabled;

    return FALSE;

}

bool appPeerSyncHasPeerHandsetPairing(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    if (appPeerSyncIsComplete())
        return ps->peer_has_handset_pairing;
    else
        return FALSE;
}

bool appPeerSyncPeerRulesInProgress(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    if (appPeerSyncIsComplete())
        return ps->peer_rules_in_progress;
    else
        /* default to TRUE, FALSE is more dangerous it would let rules run */
        return TRUE;
}

bool appPeerSyncIsInProgress(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    return (ps->peer_sync_state > PEER_SYNC_NONE) &&
           (ps->peer_sync_state < PEER_SYNC_COMPLETE);
}

bool appPeerSyncPeerDfuInProgress(void)
{
    peerSyncTaskData* ps = appGetPeerSync();
    if (appPeerSyncIsComplete())
        return ps->peer_dfu_in_progress;
    else
        return FALSE;
}
