/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_avrcp.h
\brief      Header file for AVRCP State Machine

    @startuml

    [*] -down-> NULL
    NULL -down-> DISCONNECTED

    DISCONNECTED : No AVRCP connection
    DISCONNECTED -down-> CONNECTING_LOCAL : ConnectReq
    DISCONNECTED -down-> CONNECTING_LOCAL_WAIT_RESPONSE : ConnectReq/clients to notify
    DISCONNECTED -down-> CONNECTING_REMOTE_WAIT_RESPONSE : ConnectInd/clients to notify
    DISCONNECTED -down-> CONNECTING_REMOTE : ConnectInd

    CONNECTING_LOCAL_WAIT_RESPONSE -down-> CONNECTING_LOCAL : client notification complete
    CONNECTING_REMOTE_WAIT_RESPONSE -down-> CONNECTING_REMOTE : client notification complete

    CONNECTING_LOCAL -down-> CONNECTED : AVRCP_CONNECT_CFM
    CONNECTING_REMOTE -down-> CONNECTED : AVRCP_CONNECT_CFM

    CONNECTED -left-> DISCONNECTING : Disconnect Req/Ind

    DISCONNECTING -right-> DISCONNECTED
    
    @enduml

*/

#ifndef _AV_HEADSET_AVRCP_H_
#define _AV_HEADSET_AVRCP_H_

#include <avrcp.h>
#include "av_headset.h"

struct appDeviceAttributes;
struct avInstanceTaskData;
struct avTaskData;

/*! Flag used on AVRCP states to indicate if the state represents an activity
    that will finish. */
#define AVRCP_STATE_LOCK (0x10)

/*! Application A2DP state machine states */
typedef enum
{
    AVRCP_STATE_NULL = 0,                                               /*!< Initial state */
    AVRCP_STATE_DISCONNECTED = 1,                                       /*!< No AVRCP connection */
    AVRCP_STATE_CONNECTING_LOCAL = 2 + AVRCP_STATE_LOCK,                /*!< Locally initiated connection in progress */
    AVRCP_STATE_CONNECTING_LOCAL_WAIT_RESPONSE = 3 + AVRCP_STATE_LOCK,  /*!< Locally initiated connection in progress, waiting got client task to respond */
    AVRCP_STATE_CONNECTING_REMOTE = 4 + AVRCP_STATE_LOCK,               /*!< Remotely initiated connection is progress */
    AVRCP_STATE_CONNECTING_REMOTE_WAIT_RESPONSE = 5 + AVRCP_STATE_LOCK, /*!< Remotely initiated connection in progress, waiting got client task to respond */
    AVRCP_STATE_CONNECTED = 6,                                          /*!< Control channel connected */
    AVRCP_STATE_DISCONNECTING = 7 + AVRCP_STATE_LOCK                    /*!< Disconnecting control channel */
} avAvrcpState;

typedef struct avrcpTaskData
{
    AVRCP          *avrcp;                /*!< AVRCP profile library instance */
    avAvrcpState    state;                /*!< Current state of AVRCP state machine */
    uint16          lock;                 /*!< AVRCP operation lock, used to serialise AVRCP operations */
    uint16          notification_lock;    /*!< Register notification lock, used to serialise notifications */
    uint16          playback_lock;        /*!< Playback status lock, set when AVRCP Play or Pause sent.  Cleared when playback status received, or after timeout. */
    unsigned        op_id:8;              /*!< Last sent AVRCP operation ID, used to determine which confirmation or error tone to play */
    unsigned        op_state:1;           /*!< Last sent AVRCP operation state, used to determine if operation was button press or release */
    unsigned        op_repeat:1;          /*!< Last send AVRCP operation was a repeat */
    unsigned        supported_events:13;  /*!< Bitmask of events supported by other device, i.e. notifcations registered to us. See the avrcp_supported_events enum.*/
    unsigned        changed_events:13;    /*!< Bitmask of events that have changed but notifications not sent. See the avrcp_supported_events enum.*/
    unsigned        registered_events:13; /*!< Bitmask of events that we have registered successfully. See the avrcp_supported_events enum.*/
    TaskList       *client_list;          /*!< List of clients for AVRCP messages for this link */
    uint8           client_lock;          /*!< Count of the number of clients registered for this AVRCP link */
    uint8           client_responses;     /*!< Count of outstanding response to requests sent to registered clients */
    Task            vendor_task;          /*!< Task to receive vendor commands */
    uint8          *vendor_data;          /*!< Data for current vendor command */
    uint16          vendor_opid;          /*!< Operation identifier of the current vendor command */
    unsigned        remotely_initiated:1; /*!< Was this connection initiated by the far end */                                         
    unsigned        volume_time_valid:1;  /*!< Does volume_time field contain valid time */
    rtime_t         volume_time;          /*!< Time of last volume change, only valid if volume_time_valid is set */
    uint8           volume;               /*!< Current avrcp instance volume */
    avrcp_play_status play_status;        /*! Current play status of the AVRCP connection.
                                              This is not always known. See \ref avrcp_play_hint */
    avrcp_play_status play_hint;          /*!< Our local guess at the play status. Not always accurate. */
} avrcpTaskData;

    
/*! \brief Get operation lock */
#define appAvrcpGetLock(theInst)        ((theInst)->avrcp.lock)

/*!@{ \name Lock reasons 
    The AVRCP instance can be 'locked' for a number of reasons listed here, 
    potentially simultaneously.
    
    The lock can be used in MessageSendConditionally() to hold a message back
    until all locks are released.
*/
#define APP_AVRCP_LOCK_STATE            (1 << 0)
#define APP_AVRCP_LOCK_PASSTHROUGH_REQ  (1 << 1)
#define APP_AVRCP_LOCK_PASSTHROUGH_IND  (1 << 2)
/*!@} */

/*! \brief Set operation lock */
#define appAvrcpSetLock(theInst, set_lock)      ((theInst)->avrcp.lock |= (set_lock))

/*! \brief Clear operation lock */
#define appAvrcpClearLock(theInst, clear_lock)    ((theInst)->avrcp.lock &= ~(clear_lock))

/*! \brief Is AVRCP state is 'disconnected' */
#define appAvrcpIsDisconnected(theInst) \
    ((theInst)->avrcp.state == AVRCP_STATE_DISCONNECTED)

/*! \brief Is AVRCP state is 'connected' */
#define appAvrcpIsConnected(theInst) \
    ((theInst)->avrcp.state == AVRCP_STATE_CONNECTED)

/*! Check if the specified event type is supported */
#define appAvrcpIsEventSupported(theInst, event) \
    ((theInst)->avrcp.supported_events & (1 << ((event) - 1)))

#define appAvrcpSetEventSupported(theInst, event) \
    ((theInst)->avrcp.supported_events |= (1 << ((event) - 1)))

#define appAvrcpClearEventSupported(theInst, event) \
    ((theInst)->avrcp.supported_events &= ~(1 << ((event) - 1)))


/*! Check if the specified event type is supported */
#define appAvrcpIsEventChanged(theInst, event) \
    ((theInst)->avrcp.changed_events & (1 << ((event) - 1)))

#define appAvrcpSetEventChanged(theInst, event) \
    ((theInst)->avrcp.changed_events |= (1 << ((event) - 1)))

#define appAvrcpClearEventChanged(theInst, event) \
    ((theInst)->avrcp.changed_events &= ~(1 << ((event) - 1)))


/*! Check if the specified event type has been registered */
#define appAvrcpIsEventRegistered(theInst, event) \
    ((theInst)->avrcp.registered_events & (1 << ((event) - 1)))

#define appAvrcpSetEventRegistered(theInst, event) \
    ((theInst)->avrcp.registered_events |= (1 << ((event) - 1)))

#define appAvrcpClearEventRegistered(theInst, event) \
    ((theInst)->avrcp.registered_events &= ~(1 << ((event) - 1)))


extern void appAvrcpInstanceInit(struct avInstanceTaskData *theAv);
extern void appAvrcpHandleAvrcpConnectIndicationNew(struct avTaskData *theAv, const AVRCP_CONNECT_IND_T *ind);
extern void appAvrcpRejectAvrcpConnectIndicationNew(struct avTaskData *theAv, const AVRCP_CONNECT_IND_T *ind);

extern avAvrcpState appAvrcpGetState(struct avInstanceTaskData *theAv);
extern void appAvrcpRemoteControl(struct avInstanceTaskData *theInst, avc_operation_id op_id, uint8 rstate, bool beep, uint16 repeat_ms);
extern bool appAvrcpIsValidClient(struct avInstanceTaskData *theInst, Task client_task);

extern void appAvrcpVendorPassthroughRequest(struct avInstanceTaskData *theInst, avc_operation_id op_id, uint16 size_payload, const uint8 *payload);
extern void appAvrcpVendorPassthroughResponse(struct avInstanceTaskData *theInst, avrcp_response_type);
extern Task appAvrcpVendorPassthroughRegister(struct avInstanceTaskData *theInst, Task client_task);
extern void appAvrcpNotificationsRegister(struct avInstanceTaskData *theInst, uint16 events);
extern void appAvAvrcpVolumeNotification(struct avInstanceTaskData *theInst, uint8 volume);
extern void appAvAvrcpPlayStatusNotification(struct avInstanceTaskData *theInst, avrcp_play_status play_status);

extern void appAvrcpInstanceHandleMessage(struct avInstanceTaskData *theInst, MessageId id, Message message);

#endif
