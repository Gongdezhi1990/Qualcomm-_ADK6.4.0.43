/*!
\copyright  Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_conn_rules.h
\brief      Connection Rules Module Interface
*/

#ifndef _AV_HEADSET_CONN_RULES_H_
#define _AV_HEADSET_CONN_RULES_H_

#include "av_headset_message.h"

/*! Enumeration of types of action that the connection rules engine can send
    to the state machine. */
enum    av_headset_conn_rules_messages
{
    /*! Make the device connectable. */
    CONN_RULES_ENABLE_CONNECTABLE = CONN_RULES_MESSAGE_BASE,

    /*! Make the device not connectable. */
    CONN_RULES_DISABLE_CONNECTABLE,

    /*! Start peer pairing. */
    CONN_RULES_PEER_PAIR,

    /*! Use Peer Signalling (AVRCP) to send sync message. */
    CONN_RULES_SEND_PEER_SYNC,

    /*! Use Peer Signalling (AVRCP) to send link-keys message. */
    CONN_RULES_PEER_SEND_LINK_KEYS,

    /*! Start handset pairing. */
    CONN_RULES_HANDSET_PAIR,

    /*! Connect HFP, A2DP and AVRCP to last connected handset. */
    CONN_RULES_CONNECT_HANDSET,

    /*! Connect A2DP and AVRCP to handset that peer is connected to (only TWS+). */
    CONN_RULES_CONNECT_PEER_HANDSET,

    /*! Connect A2DP and AVRCP for audio forwarding to peer earbud. */
    CONN_RULES_CONNECT_PEER,

    /*! Update TDL MRU for peer's handset */
    CONN_RULES_UPDATE_MRU_PEER_HANDSET,

    /*! Send status and role to handset */
    CONN_RULES_SEND_STATE_TO_HANDSET,

    /*! Start timer to pause A2DP streaming. */
    CONN_RULES_A2DP_TIMEOUT,

    /*! Start timer to pause A2DP streaming. */
    CONN_RULES_SCO_TIMEOUT,

    /*! Enable LEDs when out of ear. */
    CONN_RULES_LED_ENABLE,

    /*! Disable LEDs when in ear. */
    CONN_RULES_LED_DISABLE,

    /*! Disconnect links to handset and DFU not pending */
    CONN_RULES_DISCONNECT_HANDSET,

    /*! Disconnect link to peer. */
    CONN_RULES_DISCONNECT_PEER,

    /*! Enter DFU mode as we have entered the case with DFU pending */
    CONN_RULES_ENTER_DFU,

    /*! Update upgrade state */
    CONN_RULES_DFU_ALLOW,

    /*! Allow connections from handset */
    CONN_RULES_ALLOW_HANDSET_CONNECT,

    /*! Reject connections from handset */
    CONN_RULES_REJECT_HANDSET_CONNECT,

    /*! Change page scan settings. */
    CONN_RULES_PAGE_SCAN_UPDATE,

    /*! Make sure the peer link for SCO forwarding is established */
    CONN_RULES_SEND_PEER_SCOFWD_CONNECT,

    /*! Hangup an active call */
    CONN_RULES_HANGUP_CALL,

    /*! Answer an incoming call */
    CONN_RULES_ANSWER_CALL,

    /*! Transfer SCO Audio from handset to the earbud */
    CONN_RULES_SCO_TRANSFER_TO_EARBUD,

    /*! Transfer SCO Audio from Earbud to the handset */
    CONN_RULES_SCO_TRANSFER_TO_HANDSET,

    /*! Switch microphone to use for voice call. */
    CONN_RULES_SELECT_MIC,

    /*! Control if SCO forwarding is enabled or disabled. */
    CONN_RULES_SCO_FORWARDING_CONTROL,

    /*! Start ANC Tuning */
    CONN_RULES_ANC_TUNING_START,

    /*! Stop ANC Tuning */
    CONN_RULES_ANC_TUNING_STOP,

    /*! Change whether we are allowing a new BLE connection (advertising) */
    CONN_RULES_BLE_CONNECTION_UPDATE,

    /*! Any rules with RULE_FLAG_PROGRESS_MATTERS are no longer in progress. */
    CONN_RULES_NOP,
};

/*! \brief Definition of #CONN_RULES_PAGE_SCAN_UPDATE action message. */
typedef struct
{
    /*! TRUE page scan must be enabled.
        FALSE page scan must be disabled. */
    bool enable;
} CONN_RULES_PAGE_SCAN_UPDATE_T;

/*! \brief Actions to take after connecting handset. */
typedef enum
{
    RULE_POST_HANDSET_CONNECT_ACTION_NONE,       /*!< Do nothing more */
    RULE_POST_HANDSET_CONNECT_ACTION_PLAY_MEDIA, /*!< Play media */
} rulePostHandsetConnectAction;

/*! \brief Definition of #CONN_RULES_CONNECT_HANDSET action message. */
typedef struct
{
    /*! bitmask of profiles to connect. */
    uint8 profiles;
    /*! Action to take post-connect */
    rulePostHandsetConnectAction action;
} CONN_RULES_CONNECT_HANDSET_T;

/*! \brief Definition of #CONN_RULES_CONNECT_PEER_HANDSET action message. */
typedef struct
{
    /*! bitmask of profiles to connect. */
    uint8 profiles;
} CONN_RULES_CONNECT_PEER_HANDSET_T;

/*! \brief Definition of #CONN_RULES_CONNECT_PEER action message. */
typedef struct
{
    /*! bitmask of profiles to connect. */
    uint8 profiles;
} CONN_RULES_CONNECT_PEER_T;

/*! \brief Definition of #CONN_RULES_SELECT_MIC action message. */
typedef struct
{
    /*! TRUE use local microphone, FALSE use remote microphone. */
    micSelection selected_mic;
} CONN_RULES_SELECT_MIC_T;

/*! \brief Definition of #CONN_RULES_SCO_FORWARDING_CONTROL action message. */
typedef struct
{
    /*! TRUE enable SCO forwarding, FALSE disable SCO forwarding. */
    bool forwarding_control;
} CONN_RULES_SCO_FORWARDING_CONTROL_T;

/*! \brief Definition of #CONN_RULES_DISCONNECT_HANDSET action message. */
typedef struct
{
    /*! TRUE handover the handset to the other earbud after disconnecting.
        FALSE just disconnect */
    bool handover;
} CONN_RULES_DISCONNECT_HANDSET_T;


/*! \brief Definition of #CONN_RULES_BLE_CONNECTION_UPDATE action message. */
typedef struct
{
    /*! TRUE BLE connections are allowed
        FALSE BLE connections are not allowed*/
    bool enable;
} CONN_RULES_BLE_CONNECTION_UPDATE_T;


/*! \brief Definition of #CONN_RULES_DFU_ALLOW action message. */
typedef struct
{
    /*! TRUE    Upgrades (DFU) are allowed.
        FALSE   Upgrades are not allowed*/
    bool enable;
} CONN_RULES_DFU_ALLOW_T;


/*! \brief Current rule status */
typedef enum
{
    /*! rule has not been processed at all */
    RULE_STATUS_NOT_DONE,
    /*! rule has been called, but did not generated an action */
    RULE_STATUS_IGNORED,
    /*! rule has been called, generated an action, but has not yet been completed */
    RULE_STATUS_IN_PROGRESS,
    /*! rule has been called, generated an action and completed */
    RULE_STATUS_COMPLETE,
    /*! rule has been called, did not generate an action at this time due to incomplete
        rule preconditions, the rule must be reevaluated when subsequent events occur. */
    RULE_STATUS_DEFERRED
} ruleStatus;

/*! Definition of all the events that may have rules associated with them */
#define RULE_EVENT_STARTUP                       (1ULL << 0)     /*!< Startup */

#define RULE_EVENT_HANDSET_A2DP_CONNECTED        (1ULL << 1)     /*!< Handset connected */
#define RULE_EVENT_HANDSET_A2DP_DISCONNECTED     (1ULL << 2)     /*!< Handset disconnected */
#define RULE_EVENT_HANDSET_AVRCP_CONNECTED       (1ULL << 3)     /*!< Handset connected */
#define RULE_EVENT_HANDSET_AVRCP_DISCONNECTED    (1ULL << 4)     /*!< Handset disconnected */
#define RULE_EVENT_HANDSET_HFP_CONNECTED         (1ULL << 5)     /*!< Handset connected */
#define RULE_EVENT_HANDSET_HFP_DISCONNECTED      (1ULL << 6)     /*!< Handset disconnected */

#define RULE_EVENT_PEER_CONNECTED                (1ULL << 7)     /*!< Peer connected */
#define RULE_EVENT_PEER_DISCONNECTED             (1ULL << 8)     /*!< Peer disconnected */
#define RULE_EVENT_PEER_SYNC_VALID               (1ULL << 9)     /*!< Peer sync exchanged */
#define RULE_EVENT_PEER_SYNC_FAILED              (1ULL << 10)    /*!< Peer sync failed */
#define RULE_EVENT_PEER_UPDATE_LINKKEYS          (1ULL << 11)    /*!< Linkey for handset updated, potentially need to forward to peer */
#define RULE_EVENT_RX_HANDSET_LINKKEY            (1ULL << 12)    /*!< Receive derived TWS linkkey from peer device */

#define RULES_EVENT_BATTERY_CRITICAL             (1ULL << 13)    /*!< Battery voltage is critical */
#define RULES_EVENT_BATTERY_LOW                  (1ULL << 14)    /*!< Battery voltage is low */
#define RULES_EVENT_BATTERY_OK                   (1ULL << 15)    /*!< Battery voltage is OK */
#define RULES_EVENT_CHARGER_CONNECTED            (1ULL << 16)    /*!< Charger is connected */
#define RULES_EVENT_CHARGER_DISCONNECTED         (1ULL << 17)    /*!< Charger is disconnected */
#define RULES_EVENT_CHARGING_COMPLETED           (1ULL << 18)    /*!< Charging is complete */

#define RULE_EVENT_IN_EAR                        (1ULL << 19)    /*!< Earbud put in ear. */
#define RULE_EVENT_OUT_EAR                       (1ULL << 20)    /*!< Earbud taken out of ear. */
#define RULE_EVENT_IN_CASE                       (1ULL << 21)    /*!< Earbud put in the case. */
#define RULE_EVENT_OUT_CASE                      (1ULL << 22)    /*!< Earbud taken out of case. */

#define RULE_EVENT_MOTION_DETECTED               (1ULL << 23)    /*!< Earbud started moving. */
#define RULE_EVENT_NO_MOTION_DETECTED            (1ULL << 24)    /*!< Earbud stopped moving. */

#define RULE_EVENT_USER_CONNECT                  (1ULL << 25)    /*!< User has requested connect */
                                                       // 26 Unused
#define RULE_EVENT_DFU_CONNECT                   (1ULL << 27)    /*!< Connect for DFU purposes */

#define RULE_EVENT_PAGE_SCAN_UPDATE              (1ULL << 28)    /*!< 'event' for checking page scan */

#define RULE_EVENT_PEER_IN_CASE                  (1ULL << 29)    /*!< Peer put in the case */
#define RULE_EVENT_PEER_OUT_CASE                 (1ULL << 30)    /*!< Peer taken out of the case */
#define RULE_EVENT_PEER_IN_EAR                   (1ULL << 31)    /*!< Peer put in the ear */
#define RULE_EVENT_PEER_OUT_EAR                  (1ULL << 32)    /*!< Peer taken out of ear */

#define RULE_EVENT_PEER_A2DP_CONNECTED           (1ULL << 33)    /*!< Peer handset A2DP connected */
#define RULE_EVENT_PEER_A2DP_DISCONNECTED        (1ULL << 34)    /*!< Peer handset A2DP disconnected */
#define RULE_EVENT_PEER_HFP_CONNECTED            (1ULL << 35)    /*!< Peer handset HFP connected */
#define RULE_EVENT_PEER_HFP_DISCONNECTED         (1ULL << 36)    /*!< Peer handset HFP disconnected */
#define RULE_EVENT_PEER_AVRCP_CONNECTED          (1ULL << 37)    /*!< Peer handset AVRCP connected */
#define RULE_EVENT_PEER_AVRCP_DISCONNECTED       (1ULL << 38)    /*!< Peer handset AVRCP disconnected */
#define RULE_EVENT_PEER_HANDSET_CONNECTED        (1ULL << 39)    /*!< Peer has handset connection */
#define RULE_EVENT_PEER_HANDSET_DISCONNECTED     (1ULL << 40)    /*!< Peer handset has disconnected */

#define RULE_EVENT_PEER_LINK_LOSS                (1ULL << 41)    /*!< Peer link-loss has occurred */
#define RULE_EVENT_HANDSET_LINK_LOSS             (1ULL << 42)    /*!< Handset link-loss has occurred */

#define RULE_EVENT_PEER_A2DP_SUPPORTED           (1ULL << 43)    /*!< Peer handset A2DP now supported */
#define RULE_EVENT_PEER_AVRCP_SUPPORTED          (1ULL << 44)    /*!< Peer handset AVRCP now supported */
#define RULE_EVENT_PEER_HFP_SUPPORTED            (1ULL << 45)    /*!< Peer handset HFP now supported */

#define RULE_EVENT_SCO_ACTIVE                    (1ULL << 46)    /*!< SCO call active */
#define RULE_EVENT_SCO_INACTIVE                  (1ULL << 47)    /*!< SCO call inactive */

#define RULE_EVENT_BLE_CONNECTABLE_CHANGE        (1ULL << 48)    /*!< Check status of BLE connection creation */

#define RULE_EVENT_HANDOVER_DISCONNECT           (1ULL << 49)    /*!< Handover disconnect */
#define RULE_EVENT_HANDOVER_RECONNECT            (1ULL << 50)    /*!< Handover reconnect */
#define RULE_EVENT_HANDOVER_RECONNECT_AND_PLAY   (1ULL << 51)    /*!< Handover reconnect then play media */

#define RULE_EVENT_CHECK_DFU                     (1ULL << 52)    /*!< Check whether upgrades should be allowed */

#define RULE_EVENT_ALL_EVENTS_MASK               (0xFFFFFFFFFFFFFFFFULL)


/*! Action required by the rules engine. This type is used as the 
    return value from a rule. */
typedef enum
{
    /*! Let the rule engine know that it should send the message defined for the rule */
    RULE_ACTION_RUN,
    /*! Let the rule engine know that no message is needed, as the rule has been completed */
    RULE_ACTION_COMPLETE,
    /*! Let the rule engine know that no message is needed, as the rule not required  */
    RULE_ACTION_IGNORE,
    /*! Let the rule engine know that no message is needed - AT THE MOMENT - but that it
        must reevaluate this rule when subsequent events occur. */
    RULE_ACTION_DEFER,
    /*! Let the rule engine know that it should send the message defined for the rule
        and that the message has a body with parameters. */
    RULE_ACTION_RUN_WITH_PARAM
} ruleAction;

/*! Type to hold a bitmask of active connection rules */
typedef unsigned long long connRulesEvents;
/* Protect against unsigned long long being less than 8 bytes, e.g. BlueCore. */
STATIC_ASSERT(sizeof(connRulesEvents) >= 8, appConnRules_eventSizeBadness);

/*! \brief Connection Rules task data. */
typedef struct
{
    /*! Connection rules module task. */
    TaskData task;

    /*! Set of active events */
    connRulesEvents events;

    /*! Message contents data for rules that wish to run with parameters. */
    void* rule_message_data;
    /*! Size of the data in #rule_message_data */
    size_t size_rule_message_data;

    /*! Set of tasks registered for event actions */
    TaskList* event_tasks;

    /*! Set of tasks registered to receive notification that rules are
     *  no longer in progress. */
    TaskList* nop_tasks;

    bool allow_connect_after_pairing;
} connRulesTaskData; 

/*! \brief Initialise the connection rules module. */
extern void appConnRulesInit(void);

/*! \brief Set an event or events
    \param client_task The client task to receive rule actions from this event
    \param event Events to set that will trigger rules to run
    This function is called to set an event or events that will cause the relevant
    rules in the rules table to run.  Any actions generated will be sent as message
    to the client_task
*/    
extern void appConnRulesSetEvent(Task client_task, connRulesEvents event);

/*! \brief Reset/clear an event or events 
    \param event Events to clear
    This function is called to clear an event or set of events that was previously
    set by appConnRulesSetEvent().  Clear event will reset any rule that was run for event.
*/    
extern void appConnRulesResetEvent(connRulesEvents event);

/*! \brief Get set of active events 
    \return The set of active events.
*/
extern connRulesEvents appConnRulesGetEvents(void);

/*! \brief Mark rules as complete from messaage ID 
    \param message Message ID that rule(s) generated
    This function is called to mark rules as completed, the message parameter
    is used to determine which rules can be marked as completed.
*/    
extern void appConnRulesSetRuleComplete(MessageId message);

/*! \brief Mark rules as complete from message ID and set of events
    \param message Message ID that rule(s) generated
    \param event Event or set of events that trigger the rule(s)
    This function is called to mark rules as completed, the message and event parameter
    is used to determine which rules can be marked as completed.
*/    
extern void appConnRulesSetRuleWithEventComplete(MessageId message, connRulesEvents event);

/*! \brief Determine if there are still rules in progress.
    \return bool TRUE there are rules with #RULE_FLAG_PROGRESS_MATTERS set
    which have a status of RULE_STATUS_IN_PROGRESS, otherwise FALSE.
*/
extern bool appConnRulesInProgress(void);

/*! \brief Register a task to receive notifications that no rules are in progress.

    \param[in] task Task to receive notifications.
*/
extern void appConnRulesNopClientRegister(Task task);

#endif /* _AV_HEADSET_CONN_RULES_H_ */

