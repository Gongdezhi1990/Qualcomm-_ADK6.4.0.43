/*!
\copyright  Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_peer_signalling.h
\brief	    Interface to module providing signalling to headset peer device.
*/

#ifndef AV_HEADSET_PEER_SIGNALLING_H
#define AV_HEADSET_PEER_SIGNALLING_H

#include "av_headset_message.h"
#include "av_headset_tasklist.h"
#include "av_headset_phy_state.h"

/*! Flag used on peer signalling states to indicate if the state represents 
    an activity that will finish. This is reflected in the lock member of 
    \ref peerSigTaskData. */
#define PEER_SIG_STATE_LOCK (0x10)

/*! Channel IDs for peer signalling.
    Bitmask used so that a single client task could register for multiple channels,
    if we support that feature in the future.
 */
typedef enum
{
    /*! Channel ID for SCO Forwarding control messages. */
    PEER_SIG_MSG_CHANNEL_SCOFWD = 1UL << 0,

    /*! Channel ID for Peer Sync messages. */
    PEER_SIG_MSG_CHANNEL_PEER_SYNC = 1UL << 1,

    /*! Maximum channel ID */
    PEER_SIG_MSG_CHANNEL_MAX  = 1UL << 3,
    
    /*! Force enum to 16 bits */
    FORCE_ENUM_TO_MIN_16BIT(peerSigMsgChannel)
} peerSigMsgChannel;

/*! Peer signalling state machine states */
typedef enum
{
    PEER_SIG_STATE_NULL = 0,                                       /*!< Initial state */
    PEER_SIG_STATE_DISCONNECTED = 1,                               /*!< No connection */
    PEER_SIG_STATE_CONNECTING_ACL = 2 + PEER_SIG_STATE_LOCK,       /*!< Connecting ACL */
    PEER_SIG_STATE_CONNECTING_LOCAL = 3 + PEER_SIG_STATE_LOCK,     /*!< Locally initiated connection in progress */
    PEER_SIG_STATE_CONNECTING_REMOTE = 4 + PEER_SIG_STATE_LOCK,    /*!< Remotely initiated connection is progress */
    PEER_SIG_STATE_CONNECTED = 5,                                  /*!< Connnected */
} appPeerSigState;


/*! Peer signalling module state. */
typedef struct
{
    /* State for managing this peer signalling application module */
    TaskData task;                  /*!< Peer Signalling module task */
    appPeerSigState state:5;        /*!< Current state */
    uint16 lock;                    /*!< State machine lock */
    TaskList *peer_sig_client_tasks;/*!< List of tasks registered for notifications
                                         of peer signalling channel availability */
    bool link_loss_occurred;        /*!< TRUE if link-loss has occurred */

    /* State related to maintaining signalling channel with peer */
    bdaddr peer_addr;               /*!< Bluetooth address of the peer we are signalling */
    avInstanceTaskData *av_inst;    /*!< Reference used to communicate using AVRCP with peer via the AV module */

    /* Tasks registered to receive asynchronous incoming messages */
    /* \todo These could move to use the generic msg channel mechanism and simplify
     * the code. */
    Task rx_link_key_task;          /*!< Task to send handset link key to when received from peer */
    Task rx_handset_commands_task;  /*!< Task to send handset commands received from peer */

    /* State required to service various signalling requests */
    Task client_task;               /*!< Task to respond with result of current peer signalling operation */
    uint16 current_op;              /*!< Type of in progress operation. */
    bdaddr handset_addr;            /*!< Address of the handset for current operation */

    /* State related to msg channel facility. */
    TaskList* msg_channel_tasks;         /*!< List of tasks and associated signalling channel. */
    peerSigMsgChannel current_msg_channel; /*!< Remember msg channel in use for TX confirmation msgs. */

} peerSigTaskData;

/*! Enumeration of peer signalling status codes. */
typedef enum
{
    /*! Operation success. */
    peerSigStatusSuccess,

    /*! Failed to send link key to peer. */
    peerSigStatusLinkKeyTxFail,

    /*! Signalling channel with peer earbud established. */
    peerSigStatusConnected,

    /*! Signalling channel with peer earbud disconnected. */
    peerSigStatusDisconnected,

    /*! Signalling channel with peer earbud disconnected due to link-loss. */
    peerSigStatusLinkLoss,

    /*! Failed to send #AVRCP_PEER_CMD_PAIR_HANDSET_ADDRESS. */
    peerSigStatusPairHandsetTxFail,

    /*! Message channel transmission failed. */
    peerSigStatusMsgChannelTxFail,
} peerSigStatus;

/*! Messages that can be sent by peer signalling to client tasks. */

enum av_headset_peer_signalling_messages
{
    /*! Result of operation to send link key to peer. */
    PEER_SIG_LINK_KEY_TX_CFM = PEER_SIG_MESSAGE_BASE,

    /*! Peer link key received. */
    PEER_SIG_LINK_KEY_RX_IND,

    /*! Signalling link to peer established. */
    PEER_SIG_CONNECTION_IND,

    /*! Pair Handset command received. */
    PEER_SIG_PAIR_HANDSET_IND,

    /*! Result of operation to send AVRCP_PEER_CMD_PAIR_HANDSET_ADDRESS to peer. */
    PEER_SIG_PAIR_HANDSET_CFM,

    /*! Data received over a peer signalling message channel. */
    PEER_SIG_MSG_CHANNEL_RX_IND,

    /*! Confirmation of delivery of a peer signalling message channel transmission. */
    PEER_SIG_MSG_CHANNEL_TX_CFM,

    /*! Peer request to connect to the handset */
    PEER_SIG_CONNECT_HANDSET_IND,

    /*! Confirmation of delivery of connect handset message */
    PEER_SIG_CONNECT_HANDSET_CFM,
};

/*! Message sent to client task with result of operation to send link key to peer. */
typedef struct
{
    peerSigStatus status;           /*!< Status of request */
    bdaddr handset_addr;            /*!< Handset that this response refers to */
} PEER_SIG_LINK_KEY_TX_CFM_T;

/*! Message sent to client task with handset link key received from peer. */
typedef struct
{
    peerSigStatus status;           /*!< Status of request */
    bdaddr handset_addr;            /*!< Handset that this response refers to */
    uint16 key_len;                 /*!< Length of the link key in <B>uint16s, not octets</B> */
    uint16 key[1];                  /*!< Link key. This will be the full length, \ref key_len */
} PEER_SIG_LINK_KEY_RX_IND_T;

/*! Message sent to clients registered to receive notification of peer signalling
    connection and disconnection events.

    The status can be either #peerSigStatusConnected or
    #peerSigStatusDisconnected.
 */
typedef struct
{
    peerSigStatus status;           /*!< Connected / disconected status (see message description) */
} PEER_SIG_CONNECTION_IND_T;

/*! Message sent to pairing module to pair with a handset. */
typedef struct
{
    bdaddr handset_addr;            /*!< Address of handset to pair with */
} PEER_SIG_PAIR_HANDSET_IND_T;

/*! Message content when peer is requesting connect to handset */
typedef struct
{
    bool play_media;
} PEER_SIG_CONNECT_HANDSET_IND_T;

/*! Message sent to client task with result of operation to send pair handset command to peer. */
typedef struct
{
    peerSigStatus status;           /*!< Status of pairing message */
    bdaddr handset_addr;            /*!< Address of handset the status applies to */
} PEER_SIG_PAIR_HANDSET_CFM_T;

/*! Message sent to client task with result of operation to send connect handset command to peer. */
typedef struct
{
    peerSigStatus status;           /*!< Status of connect message */
} PEER_SIG_CONNECT_HANDSET_CFM_T;

/*! \brief Notification of incoming message on a peer signalling channel. */
typedef struct
{
    peerSigMsgChannel channel;      /*!< Channel over which message received. */
    uint16 msg_size;                /*!< Size of data in msg field. */
    uint8 msg[1];                   /*!< Message contents. */
} PEER_SIG_MSG_CHANNEL_RX_IND_T;

/*! \brief Confirmation of delivery of a message channel transmission. */
typedef struct
{
    peerSigStatus status;           /*!< Result of msg channel transmission. */
    peerSigMsgChannel channel;      /*!< Msg channel transmission channel used. */
} PEER_SIG_MSG_CHANNEL_TX_CFM_T;

/*! Internal messages used by peer signalling. */
typedef enum
{
    /*! Message to bring up link to peer */
    PEER_SIG_INTERNAL_STARTUP_REQ,

    /*! Message to shut down link to peer */
    PEER_SIG_INTERNAL_SHUTDOWN_REQ,

    /*! Message to send Link Key to peer */
    PEER_SIG_INTERNAL_LINK_KEY_REQ,

    /*! Message to teardown peer signalling channel due to inactivity */
    PEER_SIG_INTERNAL_INACTIVITY_TIMER,

    /*! Message to send AVRCP_PEER_CMD_PAIR_HANDSET_ADDRESS to peer. */
    PEER_SIG_INTERNAL_PAIR_HANDSET_REQ,

    /*! Message to send AVRCP_PEER_CMD_CONNECT_HANDSET to peer. */
    PEER_SIG_INTERNAL_CONNECT_HANDSET_REQ,

    /*! Message to send message to peer, channel ID is added to message ID
     * to make unique message ID per channel, hence this must be last message
     * in enum */
    PEER_SIG_INTERNAL_MSG_CHANNEL_TX_REQ,
};

/*! Internal message sent to start signalling to a peer */
typedef struct
{
    bdaddr peer_addr;           /*!< Address of peer */
} PEER_SIG_INTERNAL_STARTUP_REQ_T;

/*! Message definition for action to send link key to peer. */
typedef struct
{
    Task client_task;           /*!< Task to receive any response */
    bdaddr handset_addr;        /*!< Handset that link key is for  */
    uint16 key_len;             /*!< Length of link key, in octets */
    uint8 key[1];               /*!< Link key. This will be the full length, key_len */
} PEER_SIG_INTERNAL_LINK_KEY_REQ_T;

/*! Message definition for action to send pair handset command. */
typedef struct
{
    Task client_task;           /*!< Task to receive any response */
    bdaddr handset_addr;        /*!< Handset that peer should try to pair with */
} PEER_SIG_INTERNAL_PAIR_HANDSET_REQ_T;

/*! Message definition for action to send connect handset command. */
typedef struct
{
    Task client_task;           /*!< Task to receive any response */
    bool play_media;            /*!< Play media once handset is connected */
} PEER_SIG_INTERNAL_CONNECT_HANDSET_REQ_T;

/*! Structure used to request message channel transmission to peer. */
typedef struct
{
    Task client_task;           /*!< Task to receive the msg tx result. */
    peerSigMsgChannel channel;  /*!< Channel over which to transmit message. */
    uint16 msg_size;            /*!< Size of data in msg. */
    uint8 msg[1];               /*!< Message data to transmit. */
} PEER_SIG_INTERNAL_MSG_CHANNEL_TX_REQ_T;

/*! \brief Initialise the peer signalling module. 
 */
void appPeerSigInit(void);

/*! \brief Send handset link key to peer headset. 

    \param      task         Task to send confirmation message to.
    \param[in]  peer_addr    Address of peer earbud.
    \param[in]  handset_addr Address of handset to pair with.
    \param[in]  key          Pointer to the link key to send
    \param      key_len      Length of the link key <B>in uint16's (not octets)</B>
*/
void appPeerSigLinkKeyToPeerRequest(Task task, const bdaddr *peer_addr, const bdaddr *handset_addr,
                                  const uint16 *key, uint16 key_len);

/*! \brief Inform peer earbud of address of handset with which it should pair.

    \param[in] task         Task to send confirmation message to.
    \param[in] peer_addr    Address of peer earbud.
    \param[in] handset_addr Address of handset to pair with.
*/
void appPeerSigTxPairHandsetRequest(Task task, const bdaddr* peer_addr, const bdaddr* handset_addr);

/*! \brief Request peer earbud connect to the handset.
    \param[in] task       Task to send confirmation message to.
    \param[in] peer_addr  Address of peer earbud.
    \param[in] play_media Whether media should be played once the handset is connected.
*/
void appPeerSigTxConnectHandsetRequest(Task task, const bdaddr* peer_addr, bool play_media);

/*! \brief Register task with peer signalling for Link Key TX/RX operations.
 
    \param client_task Task to send messages to regarding link key operations.
 */
void appPeerSigLinkKeyTaskRegister(Task client_task);

/*! \brief Unregister task with peer signalling for Link Key TX/RX operations.
 */
void appPeerSigLinkKeyTaskUnregister(void);

/*! \brief Try and connect peer signalling channel with specified peer earbud.
 
    \param[in] peer_addr BT address of peer earbud to connect.
 */
void appPeerSigConnect(bdaddr* peer_addr);

/*! \brief Register to receive peer signalling notifications.

    \param  client_task Task to register to receive peer signalling messages.
 */
void appPeerSigClientRegister(Task client_task);

/*! \brief Unregister task that is currently receiving peer signalling notifications.

    \param  client_task Task that was registered for peer signalling messages.
 */
void appPeerSigClientUnregister(Task client_task);

/*! \brief Register task to receive handset commands.

    \param handset_commands_task    Task to receive handset commands.
  */
void appPeerSigHandsetCommandsTaskRegister(Task handset_commands_task);

/*! \brief Force peer signalling to disconnect AVRCP if it is up. 

    \note This function should only be called if the intention
    is to restart the device. The device state is not guaranteed to
    be consistent following this.
*/
void appPeerSigForceDisconnect(void);

/*! \brief Register to receive PEER_SIG_MSG_CHANNEL_RX_IND messages for a channel.

    \param[in] task         Task to receive PEER_SIG_MSG_CHANNEL_RX_IND messages
    \param     channel_mask Mask of channels to receive messages for.
 */
void appPeerSigMsgChannelTaskRegister(Task task, peerSigMsgChannel channel_mask);

/*! \brief Stop receiving PEER_SIG_MSG_CHANNEL_RX_IND messages.
 
    \param[in] task         Task to cancel receiving PEER_SIG_MSG_CHANNEL_RX_IND messages
    \param[in] channel_mask Mask of channels to unregister.
*/
void appPeerSigMsgChannelTaskUnregister(Task task, peerSigMsgChannel channel_mask);

/*! \brief Request a transmission on a message channel.

    \param[in] task      Task to send confirmation message to.
    \param[in] peer_addr Address of peer earbud.
    \param channel       Channel to transmit on.
    \param[in] msg       Payload of message.
    \param msg_size      Length of message in bytes.
 */
void appPeerSigMsgChannelTxRequest(Task task,
                                   const bdaddr* peer_addr,
                                   peerSigMsgChannel channel,
                                   const uint8* msg, uint16 msg_size);

/*****************************************************************************
 * TEST FUNCTIONS
 ******************************************************************************/
/*! \brief TEST FUNCTION to force a link to peer device */
void appPeerSigForceLinkToPeer(void);

#endif /* AV_HEADSET_PEER_SIGNALLING_H */
