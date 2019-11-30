/*!
\copyright  Copyright (c) 2005 - 2018 Qualcomm Technologies International, Ltd.
            All Rights Reserved.
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_peer_sync.h
\brief	    Application peer synchronisation logic interface.
*/

#ifndef _AV_HEADSET_PEER_SYNC_H_
#define _AV_HEADSET_PEER_SYNC_H_

#include "av_headset_peer_signalling.h"

/*! \brief The state of synchronisation with the peer.
    @startuml
    participant Earbud1 as EB1
    participant Earbud2 as EB2
    participant Handset as HS
    EB1 -> EB2: Peer Sync
    EB1 <- EB2: Peer Sync
    EB1 -> EB2: Connect A2DP/AVRCP
    EB1 -> HS: Connect A2DP/AVRCP
    note left: Peer Sync information of\nEarbud1 is now out of date
    EB1 -> EB2: Peer Sync
    ==  ==
    EB1 -> HS: Disconnect A2DP/AVRCP
    note left: Peer Sync information of\nEarbud1 is now out of date
    EB1 -> EB2: Peer Sync
    @enduml
*/
typedef enum sm_peer_sync_states
{
    PEER_SYNC_NONE       = 0x00,       /*!< No peer sync messages sent or received. */

    PEER_SYNC_SENDING    = 0x01,       /*!< Peer sync has been transmitted. */
    PEER_SYNC_SENT       = 0x02,       /*!< Peer sync has been transmitted and delivery confirmed. */
    PEER_SYNC_RECEIVED   = 0x04,       /*!< Peer sync has been received. */

    /*!< Peer sync sent and received and peer state is valid. */
    PEER_SYNC_COMPLETE   = PEER_SYNC_RECEIVED | PEER_SYNC_SENT
} peerSyncState;

/*! Peer Sync module data. */
typedef struct
{
    /* -- State to run the peer sync module -- */
    TaskData task;                      /*!< Peer sync module Task. */

    peerSyncState peer_sync_state;      /*!< The state of synchronisation with the peer */

    uint8 peer_sync_tx_seqnum;          /*!< Peer sync transmit sequence number. */
    uint8 peer_sync_rx_seqnum;          /*!< Peer sync receive sequence number. */

    /* -- Event notification -- */
    TaskList* peer_sync_status_tasks;   /*!< List of tasks registered to receive PEER_SYNC_STATUS
                                             messages. */
    
    /* -- Information synchronised via peer sync -- */
    uint16 sync_battery_level;          /*!< Battery level that was sent in sync message */
    uint16 peer_battery_level;          /*!< Battery level of peer that was received in sync message */
    uint16 peer_handset_tws;            /*!< The peer's handset's TWS version */
    bdaddr peer_handset_addr;           /*!< The peer's handset's address */
    
    bool peer_a2dp_connected:1;         /*!< The peer has A2DP connected */
    bool peer_a2dp_streaming:1;         /*!< The peer has A2DP streaming */
    bool peer_avrcp_connected:1;        /*!< The peer has AVRCP connected */
    bool peer_hfp_connected:1;          /*!< The peer has HFP connected */
    bool peer_in_case:1;                /*!< The peer is in the case */
    bool peer_in_ear:1;                 /*!< The peer is in the ear */
    bool peer_is_pairing:1;             /*!< The peer is pairing */
    bool peer_has_handset_pairing:1;    /*!< The peer is paired with a handset */
    bool peer_rules_in_progress:1;      /*!< Peer still has rules in progress. */
    bool sent_in_progress:1;            /*!< Have sent a peer sync with rules_in_progress set */
    bool peer_sco_active:1;             /*!< The peer has an active SCO */
    bool peer_dfu_in_progress:1;        /*!< The peer earbud is performing DFU. */
    bool peer_advertising:1;            /*!< The peer earbud is BLE advertising */
    bool peer_ble_connected:1;          /*!< The peer earbud has a BLE connection */
    bool peer_anc_enabled:1;            /*!< The peer earbud has ANC enabled */

} peerSyncTaskData;

/*! Enumeration of messages which peer sync module can send. */
enum peerSyncMessages
{
    /*! Message sent to registered clients to notify of current peer sync state. */
    PEER_SYNC_STATUS = PEER_SYNC_MESSAGE_BASE,
};

/*! \brief Definition of #PEER_SYNC_STATUS message. */
typedef struct
{
    /*! Current state of peer sync completion. 
        TRUE peer sync is complete, otherwise FALSE. */
    bool peer_sync_complete;
} PEER_SYNC_STATUS_T;

/* PEER SYNC MODULE APIs
 ***********************/
/*! \brief Send a peer sync to peer earbud.

    \param response TRUE if this is a response peer sync.
 */
extern void appPeerSyncSend(bool response);

/*! \brief Determine if peer sync is complete.
    \return bool TRUE if peer sync is complete, FALSE otherwise.

    A complete peer sync is defined as both peers having sent their most
    up to date peer sync message and having received a peer sync from 
    their peer.
 */
extern bool appPeerSyncIsComplete(void);

/*! \brief Clear peer sync for both sent and received status. */
extern void appPeerSyncResetSync(void);

/*! \brief Register a task to receive #PEER_SYNC_STATUS messages.
    \param task Task which #PEER_SYNC_STATUS messages will be sent to.
 */
extern void appPeerSyncStatusClientRegister(Task task);

/*! \brief Unregister a task to stop receiving #PEER_SYNC_STATUS messages.
    \param task Previously registered task, for which #PEER_SYNC_STATUS should no longer be sent. */
extern void appPeerSyncStatusClientUnregister(Task task);

/*! \brief Initialise the peer sync module. */
extern void appPeerSyncInit(void);

/* FUNCTIONS TO QUERY INFORMATION SYNCHRONISED BY PEER SYNC
 **********************************************************/
/*! \brief Query if the peer's handset supports TWS+.
    \return TRUE if supported, otherwise FALSE. */
extern bool appPeerSyncIsPeerHandsetTws(void);

/*! \brief Query if the peer has an A2DP connection to its handset.
    \return TRUE if connected, otherwise FALSE. */
extern bool appPeerSyncIsPeerHandsetA2dpConnected(void);

/*! \brief Query if the peer is A2DP streaming from its handset.
    \return TRUE if streaming, otherwise FALSE. */
extern bool appPeerSyncIsPeerHandsetA2dpStreaming(void);

/*! \brief Query if the peer has an AVRCP connection to its handset.
    \return TRUE if connected, otherwise FALSE. */
extern bool appPeerSyncIsPeerHandsetAvrcpConnected(void);

/*! \brief Query if the peer has an HFP connection to its handset.
    \return TRUE if connected, otherwise FALSE. */
extern bool appPeerSyncIsPeerHandsetHfpConnected(void);

/*! \brief Query if the peer is in-case.
    \return TRUE if in-case, otherwise FALSE. */
extern bool appPeerSyncIsPeerInCase(void);

/*! \brief Query if the peer is in-ear.
    \return TRUE if in-ear, otherwise FALSE. */
extern bool appPeerSyncIsPeerInEar(void);

/*! \brief Query if the peer is pairing.
    \return TRUE if pairing, otherwise FALSE. */
extern bool appPeerSyncIsPeerPairing(void);

/*! \brief Query if the peer is paired with a handset.
    \return TRUE if paired, otherwise FALSE. */
extern bool appPeerSyncHasPeerHandsetPairing(void);


/*! \brief Query if the peer is BLE advertising.

    \return TRUE if advertising, otherwise FALSE. */
extern bool appPeerSyncIsPeerAdvertising(void);


/*! \brief Query if the peer is paired with a handset.

    \return TRUE if in a BLE connection, otherwise FALSE. */
extern bool appPeerSyncIsPeerBleConnected(void);

/*! \brief Query if the peer is ANC enabled.

    \return TRUE if ANC enabled, otherwise FALSE. */

extern bool appPeerSyncIsPeerAncEnabled(void);

/*! \brief Query if peer is still processing rules.
    \return TRUE if peer still has rules in progress, otherwise FALSE. */
extern bool appPeerSyncPeerRulesInProgress(void);

/*! \brief Query if peer sync is in progress.
    \return TRUE if peer sync is in progress, otherwise FALSE. */
extern bool appPeerSyncIsInProgress(void);

/*! \brief Query synchronised battery levels.

    \param[out] battery_level       This earbud's battery_level (sent in last sync).
    \param[out] peer_battery_level  The peer earbud's battery level (received in the last sync).
*/
extern void appPeerSyncGetPeerBatteryLevel(uint16 *battery_level, uint16 *peer_battery_level);

/*! \brief Query the peer earbud SCO status.
    \return bool TRUE if peer earbud has an active SCO, FALSE otherwise.
*/
extern bool appPeerSyncIsPeerScoActive(void);

/*! \brief Query the peer handset's address.

    \param[out] peer_handset_addr The address.
*/
extern void appPeerSyncGetPeerHandsetAddr(bdaddr *peer_handset_addr);

/*! \brief Query if peer DFU is in progress.
    \return TRUE if peer DFU is in progress, otherwise FALSE. */
extern bool appPeerSyncPeerDfuInProgress(void);

#endif /* _AV_HEADSET_PEER_SYNC_H_ */
