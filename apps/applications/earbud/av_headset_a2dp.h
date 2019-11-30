/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_a2dp.h
\brief      Header file for A2DP State Machine
*/

#ifndef _AV_HEADSET_A2DP_H_
#define _AV_HEADSET_A2DP_H_

#include <a2dp.h>
#include "av_headset.h"

struct appDeviceAttributes;
struct avInstanceTaskData;
struct avTaskData;

/*! Note that disconnects are not shown for clarity.
@startuml
state A2DP_STATE_DISCONNECTED : No A2DP connection
state A2DP_STATE_CONNECTING_LOCAL : Locally initiated connection in progress
state A2DP_STATE_CONNECTING_REMOTE : Remotely initiated connection is progress
state   A2DP_STATE_CONNECTED_SIGNALLING : Signalling channel connected
state   A2DP_STATE_CONNECTING_MEDIA_LOCAL : Locally initiated media channel connection in progress
state   A2DP_STATE_CONNECTING_MEDIA_REMOTE_SYNC : Remotely initiated media channel connection in progress synced with other instance

[*] --> A2DP_STATE_DISCONNECTED

A2DP_STATE_DISCONNECTED --> A2DP_STATE_CONNECTING_LOCAL : AV_INTERNAL_A2DP_CONNECT_REQ
A2DP_STATE_DISCONNECTED --> A2DP_STATE_CONNECTING_REMOTE : AV_INTERNAL_A2DP_SIGNALLING_CONNECT_IND/\nA2DP_SIGNALLING_CONNECT_IND

A2DP_STATE_CONNECTING_LOCAL --> A2DP_STATE_CONNECTED_SIGNALLING : A2DP_SIGNALLING_CONNECT_CFM
A2DP_STATE_CONNECTING_REMOTE --> A2DP_STATE_CONNECTED_SIGNALLING : A2DP_SIGNALLING_CONNECT_CFM

A2DP_STATE_CONNECTED_SIGNALLING -down-> A2DP_STATE_CONNECTING_MEDIA_LOCAL : AV_INTERNAL_A2DP_CONNECT_MEDIA_REQ
A2DP_STATE_CONNECTED_SIGNALLING -down-> A2DP_STATE_CONNECTING_MEDIA_REMOTE_SYNC : A2DP_MEDIA_OPEN_IND

state A2DP_STATE_CONNECTED_MEDIA {
    A2DP_STATE_CONNECTING_MEDIA_REMOTE_SYNC --> A2DP_STATE_CONNECTED_MEDIA_SUSPENDED : AV_INTERNAL_A2DP_INST_SYNC_RES&&\nA2DP_MEDIA_OPEN_CFM
    A2DP_STATE_CONNECTING_MEDIA_LOCAL --> A2DP_STATE_CONNECTED_MEDIA_SUSPENDED : A2DP_MEDIA_OPEN_CFM
    A2DP_STATE_CONNECTING_MEDIA_LOCAL --> A2DP_STATE_CONNECTED_MEDIA_STARTING_LOCAL_SYNC : A2DP_MEDIA_OPEN_CFM

    state A2DP_STATE_CONNECTED_MEDIA_STREAMING : Media channel streaming
    state A2DP_STATE_CONNECTED_MEDIA_SUSPENDING_LOCAL : Locally initiated media channel suspend in progress
    state A2DP_STATE_CONNECTED_MEDIA_SUSPENDED : Media channel suspended
    state A2DP_STATE_CONNECTED_MEDIA_STARTING_LOCAL_SYNC : Locally initiated media channel start in progress, syncing slave
    state A2DP_STATE_CONNECTED_MEDIA_STARTING_REMOTE_SYNC : Remotely initiated media channel start in progress, syncing slave

    A2DP_STATE_CONNECTED_MEDIA_STARTING_LOCAL_SYNC --> A2DP_STATE_CONNECTED_MEDIA_STREAMING : AV_INTERNAL_A2DP_INST_SYNC_RES&&\nA2DP_MEDIA_START_CFM
    A2DP_STATE_CONNECTED_MEDIA_STREAMING --> A2DP_STATE_CONNECTED_MEDIA_SUSPENDING_LOCAL : AV_INTERNAL_A2DP_SUSPEND_MEDIA_REQ
    A2DP_STATE_CONNECTED_MEDIA_STREAMING --> A2DP_STATE_CONNECTED_MEDIA_SUSPENDED : A2DP_MEDIA_SUSPEND_IND
    A2DP_STATE_CONNECTED_MEDIA_SUSPENDING_LOCAL --> A2DP_STATE_CONNECTED_MEDIA_SUSPENDED : A2DP_MEDIA_SUSPEND_CFM
    A2DP_STATE_CONNECTED_MEDIA_SUSPENDED --> A2DP_STATE_CONNECTED_MEDIA_STARTING_LOCAL_SYNC : AV_INTERNAL_A2DP_RESUME_MEDIA_REQ
    A2DP_STATE_CONNECTED_MEDIA_SUSPENDED --> A2DP_STATE_CONNECTED_MEDIA_STARTING_REMOTE_SYNC : A2DP_MEDIA_START_IND
    A2DP_STATE_CONNECTED_MEDIA_STARTING_REMOTE_SYNC --> A2DP_STATE_CONNECTED_MEDIA_STREAMING : AV_INTERNAL_A2DP_INST_SYNC_RES&&\nA2DP_MEDIA_START_CFM
}
@enduml
*/

/*! \brief Application A2DP state machine states */
typedef enum
{
    A2DP_STATE_NULL                                             = 0x00, /*!< Initial state */
    A2DP_STATE_DISCONNECTED                                     = 0x01, /*!< No A2DP connection */
    A2DP_STATE_CONNECTING_LOCAL	                                = 0x02, /*!< Locally initiated connection in progress */
    A2DP_STATE_CONNECTING_REMOTE                                = 0x03, /*!< Remotely initiated connection is progress */
    A2DP_STATE_CONNECTING_CROSSOVER                             = 0x04, /*!< Locally and remotely initiated connection is progress */
        A2DP_STATE_CONNECTED_SIGNALLING                         = 0x10, /*!< Signalling channel connected */
        A2DP_STATE_CONNECTING_MEDIA_LOCAL                       = 0x11, /*!< Locally initiated media channel connection in progress */
        A2DP_STATE_CONNECTING_MEDIA_REMOTE_SYNC                 = 0x12, /*!< Remotely initiated media channel connection in progress */
            A2DP_STATE_CONNECTED_MEDIA	                        = 0x30, /*!< Media channel connected (parent-pseudo state) */
                A2DP_STATE_CONNECTED_MEDIA_STREAMING            = 0x31, /*!< Media channel streaming */
                A2DP_STATE_CONNECTED_MEDIA_STREAMING_MUTED      = 0x32, /*!< Media channel streaming but muted (suspend failed) */
                A2DP_STATE_CONNECTED_MEDIA_SUSPENDING_LOCAL     = 0x33, /*!< Locally initiated media channel suspend in progress */
                A2DP_STATE_CONNECTED_MEDIA_SUSPENDED	        = 0x34, /*!< Media channel suspended */
                A2DP_STATE_CONNECTED_MEDIA_RECONFIGURING        = 0x35, /*!< Media channel suspended, reconfiguring the codec */
                A2DP_STATE_CONNECTED_MEDIA_STARTING_LOCAL_SYNC  = 0x70, /*!< Locally initiated media channel start in progress, syncing slave */
                A2DP_STATE_CONNECTED_MEDIA_STARTING_REMOTE_SYNC = 0x71, /*!< Remotely initiated media channel start in progress, syncing slave */
            A2DP_STATE_DISCONNECTING_MEDIA                      = 0x13, /*!< Locally initiated media channel disconnection in progress */
    A2DP_STATE_DISCONNECTING                                    = 0x0A  /*!< Disconnecting signalling and media channels */
} avA2dpState;

/*! \brief AV suspend reasons

    The suspend reasons define the cause for suspending the AV streaming,
    a reason must be specified when calling appAvStreamingSuspend() and
    appAvStreamingResume().
*/
typedef enum
{
    AV_SUSPEND_REASON_SCO    = (1 << 0), /*!< Suspend AV due to active SCO link */
    AV_SUSPEND_REASON_HFP    = (1 << 1), /*!< Suspend AV due to HFP activity */
    AV_SUSPEND_REASON_AV     = (1 << 2), /*!< Suspend AV due to AV activity */
    AV_SUSPEND_REASON_RELAY  = (1 << 3), /*!< Suspend AV due to master suspend request */
    AV_SUSPEND_REASON_REMOTE = (1 << 4), /*!< Suspend AV due to remote request */
    AV_SUSPEND_REASON_SCOFWD = (1 << 5)  /*!< Suspend AV due to SCO forwarding */
} avSuspendReason;

/*! \brief A2DP module state */
typedef struct a2dpTaskData
{
    avA2dpState     state;                 /*!< Current state of A2DP state machine */
    uint16          lock;                  /*!< A2DP operation lock, used to serialise A2DP operations */
    uint8           device_id;             /*!< A2DP device identifier from A2DP library */
    uint8           stream_id;             /*!< A2DP stream identifier from A2DP library */
    Sink            media_sink;            /*!< Sink for A2DP media (streaming) */
    uint8           current_seid;          /*!< Currently active SEID */
    uint8           sync_counter;          /*!< Used to counter syncs and provide sync ids */
    avSuspendReason suspend_state;         /*!< Bitmap of active suspend reasons */
    unsigned        flags:6;               /*!< General connection flags */
    unsigned        num_seids:3;           /*!< Number of supported SEIDs */
    unsigned        connect_retries:3;     /*!< Number of connection retries */
    unsigned        local_initiated:1;     /*!< Flag to indicate if connection was locally initiated */
    unsigned        disconnect_reason:4;   /*!< Reason for disconnect */
} a2dpTaskData;

/*! \brief Check if SEID is for non-TWS CODEC */
#define appA2dpIsSeidNonTwsSink(seid) \
    (((seid) >= AV_SEID_SBC_SNK) && \
     ((seid) <= AV_SEID_APTX_ADAPTIVE_SNK))

/*! \brief Check if SEID is for TWS Sink CODEC */
#define appA2dpIsSeidTwsSink(seid) \
    (((seid) >= AV_SEID_SBC_MONO_TWS_SNK) && \
     ((seid) <= AV_SEID_APTX_ADAPTIVE_TWS_SNK))

/*! \brief Check if SEID is for non-TWS Source CODEC */
#define appA2dpIsSeidNonTwsSource(seid) \
    ((seid) == AV_SEID_SBC_SRC)

/*! \brief Check if SEID is for TWS Source CODEC */
#define appA2dpIsSeidTwsSource(seid) \
    (((seid) >= AV_SEID_SBC_MONO_TWS_SRC) && \
     ((seid) <= AV_SEID_APTX_ADAPTIVE_TWS_SRC))
     
/*! \brief Check if SEID is for TWS CODEC */
#define appA2dpIsSeidTws(seid) \
    (appA2dpIsSeidTwsSink(seid) || appA2dpIsSeidTwsSource(seid))

/*! \brief Check if SEID is for Sink */
#define appA2dpIsSeidSink(seid) \
    (appA2dpIsSeidTwsSink(seid) || appA2dpIsSeidNonTwsSink(seid))

/*! \brief Check if SEID is for Source */
#define appA2dpIsSeidSource(seid) \
    (appA2dpIsSeidTwsSource(seid) || appA2dpIsSeidNonTwsSource(seid))

/*! \brief Check this instance is a non-TWS Sink */
#define appA2dpIsSinkNonTwsCodec(theInst) \
    (appA2dpIsSeidNonTwsSink(theInst->a2dp.current_seid))

/*! \brief Check this instance is a TWS Sink */
#define appA2dpIsSinkTwsCodec(theInst) \
    (appA2dpIsSeidTwsSink(theInst->a2dp.current_seid))

/*! \brief Check this instance is a Sink */
#define appA2dpIsSinkCodec(theInst) \
    (appA2dpIsSeidSink(theInst->a2dp.current_seid))

/*! \brief Check this instance is a Source */
#define appA2dpIsSourceCodec(theInst) \
    (appA2dpIsSeidSource(theInst->a2dp.current_seid))

/*! \brief Check this instance is a TWS Source or Sink */
#define appA2dpIsTwsCodec(theInst) \
    (appA2dpIsSeidTws(theInst->a2dp.current_seid))
    
/*! \brief Get A2DP lock */
#define appA2dpGetLock(theInst) ((theInst)->a2dp.lock)

/*!@{ \name A2DP lock bit masks */
#define APP_A2DP_TRANSITION_LOCK 1U
#define APP_A2DP_KYMERA_LOCK     2U
/*!@} */

/*! \brief Set A2DP lock bit for transition states */
#define appA2dpSetTransitionLockBit(theInst) (theInst)->a2dp.lock |= APP_A2DP_TRANSITION_LOCK
/*! \brief Clear A2DP lock bit for transition states */
#define appA2dpClearTransitionLockBit(theInst) (theInst)->a2dp.lock &= ~APP_A2DP_TRANSITION_LOCK
/*! \brief Set A2DP lock bit waiting for kymera operations  */
#define appA2dpSetKymeraLockBit(theInst) (theInst)->a2dp.lock |= APP_A2DP_KYMERA_LOCK
/*! \brief Set A2DP lock bit waiting for kymera operations  */
#define appA2dpClearKymeraLockBit(theInst) (theInst)->a2dp.lock &= ~APP_A2DP_KYMERA_LOCK

/*!@{ \name Masks to use to check for the current state meeting some conditions */
#define A2DP_STATE_MASK_CONNECTED_SIGNALLING            (A2DP_STATE_CONNECTED_SIGNALLING)
#define A2DP_STATE_MASK_CONNECTED_MEDIA                 (A2DP_STATE_CONNECTED_MEDIA)
#define A2DP_STATE_MASK_CONNECTED_MEDIA_STREAMING       (A2DP_STATE_CONNECTED_MEDIA | 0x0F)
/*!@} */


/*! \brief Is A2DP state 'connected signalling' */
#define appA2dpIsStateConnectedSignalling(a2dp_state) \
    (((a2dp_state) & A2DP_STATE_MASK_CONNECTED_SIGNALLING) == A2DP_STATE_CONNECTED_SIGNALLING)
	  
/*! \brief Is A2DP state 'connected media' */
#define appA2dpIsStateConnectedMedia(a2dp_state) \
    (((a2dp_state) & A2DP_STATE_MASK_CONNECTED_MEDIA) == A2DP_STATE_CONNECTED_MEDIA)

/*! \brief Is A2DP state 'connected media streaming' */
#define appA2dpIsStateConnectedMediaStreaming(a2dp_state) \
    ((a2dp_state) == A2DP_STATE_CONNECTED_MEDIA_STREAMING)

/*! \brief Is kymera started in this current state.
    \todo This could be encoded in the state vector. */
#define appA2dpIsKymeraOnInState(a2dp_state) \
    (((a2dp_state) == A2DP_STATE_CONNECTED_MEDIA_STARTING_LOCAL_SYNC) || \
     ((a2dp_state) == A2DP_STATE_CONNECTED_MEDIA_STARTING_REMOTE_SYNC) || \
     ((a2dp_state) == A2DP_STATE_CONNECTED_MEDIA_STREAMING))

/*! \brief Is A2DP instance media channel connected */
#define appA2dpIsConnectedMedia(theInst) \
    appA2dpIsStateConnectedMedia((theInst)->a2dp.state)

/*! \brief Is A2DP instance streaming */
#define appA2dpIsStreaming(theInst) \
    appA2dpIsStateConnectedMediaStreaming((theInst)->a2dp.state)

/*! \brief Is A2DP instance disconnected */
#define appA2dpIsDisconnected(theInst) \
    ((theInst)->a2dp.state == A2DP_STATE_DISCONNECTED)

/*! \brief Quick check for whether a2dp is connected on the specified AV instance */
#define appA2dpIsConnected(theInst) \
    (((theInst)->a2dp.state & A2DP_STATE_MASK_CONNECTED_SIGNALLING) == A2DP_STATE_CONNECTED_SIGNALLING)

extern void appA2dpInstanceInit(struct avInstanceTaskData *theAv, uint8 suspend_state);
extern void appA2dpSignallingConnectIndicationNew(struct avTaskData *theAv, const A2DP_SIGNALLING_CONNECT_IND_T *ind);
extern void appA2dpRejectA2dpSignallingConnectIndicationNew(struct avTaskData *theAv, const A2DP_SIGNALLING_CONNECT_IND_T *ind);
extern void appA2dpVolumeSet(struct avInstanceTaskData *theAv, uint16 volume);
extern void appA2dpSetDefaultAttributes(struct appDeviceAttributes *attributes);
extern avA2dpState appA2dpGetState(struct avInstanceTaskData *theAv);
extern void appA2dpInstanceHandleMessage(struct avInstanceTaskData *theInst, MessageId id, Message message);
extern uint8 appA2dpConvertSeidFromSinkToSource(uint8 seid);
extern bool appA2dpSeidsAreCompatible(const struct avInstanceTaskData *inst1, const struct avInstanceTaskData *inst2);

#endif
