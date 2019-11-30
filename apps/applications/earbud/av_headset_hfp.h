/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_hfp.h
\brief      Header file for HFP State Machine

    @startuml
    
    [*] --> NULL
    NULL --> HFP_STATE_INITIALISING_HFP
    
    HFP_STATE_INITIALISING_HFP : Initialising HFP instance 
    HFP_STATE_INITIALISING_HFP --> HFP_STATE_DISCONNECTED : HFP_INIT_CFM

    HFP_STATE_DISCONNECTED : No HFP connection 
    state HFP_STATE_CONNECTING {
        HFP_STATE_CONNECTING_LOCAL : Locally initiated connection in progress 
        HFP_STATE_CONNECTING_REMOTE : Remotely initiated connection is progress 
        HFP_STATE_DISCONNECTED -down-> HFP_STATE_CONNECTING_LOCAL
        HFP_STATE_DISCONNECTED -down-> HFP_STATE_CONNECTING_REMOTE
    }

    state HFP_STATE_CONNECTED {

        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_IDLE : no_call_setup/no_call
        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_INCOMING : incoming_call_setup/no_call
        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_OUTGOING : outgoing_call_setup/no_call
        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_OUTGOING : outgoing_call_alerting_setup/no_call

        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_ACTIVE : no_call_setup/call
        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_ACTIVE : incoming_call_setup/call
        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_ACTIVE : outgoing_call_setup/call
        HFP_STATE_CONNECTING --> HFP_STATE_CONNECTED_ACTIVE : outgoing_call_alerting_setup/call

        HFP_STATE_CONNECTED_IDLE : HFP connected, no call in progress 
        HFP_STATE_CONNECTED_IDLE -down-> HFP_STATE_CONNECTED_ACTIVE
        HFP_STATE_CONNECTED_OUTGOING : HFP connected, outgoing call in progress 
        HFP_STATE_CONNECTED_INCOMING -right-> HFP_STATE_CONNECTED_ACTIVE
        HFP_STATE_CONNECTED_INCOMING : HFP connected, incoming call in progress 
        HFP_STATE_CONNECTED_OUTGOING -left-> HFP_STATE_CONNECTED_ACTIVE
        HFP_STATE_CONNECTED_ACTIVE : HFP connected, active call in progress 
        HFP_STATE_CONNECTED_ACTIVE -down-> HFP_STATE_DISCONNECTING
        HFP_STATE_DISCONNECTING : 

        HFP_STATE_DISCONNECTING -up-> HFP_STATE_DISCONNECTED
    }

    @enduml

*/

#ifndef _AV_HEADSET_HFP_H_
#define _AV_HEADSET_HFP_H_

#include <hfp.h>
#include "av_headset.h"
#include "av_headset_tasklist.h"
#include "av_headset_battery.h"

#ifdef INCLUDE_HFP

struct appDeviceAttributes;



/*! \brief Application HFP state machine states */
typedef enum
{
    HFP_STATE_NULL,                         /*!< Initial state */
    HFP_STATE_INITIALISING_HFP,             /*!< Initialising HFP instance */
    HFP_STATE_DISCONNECTED,                 /*!< No HFP connection */
    HFP_STATE_CONNECTING_LOCAL,             /*!< Locally initiated connection in progress */
    HFP_STATE_CONNECTING_REMOTE,            /*!< Remotely initiated connection is progress */
    /* HFP_STATE_CONNECTD (Parent state) */
        HFP_STATE_CONNECTED_IDLE,           /*!< HFP connected, no call in progress */
        HFP_STATE_CONNECTED_OUTGOING,       /*!< HFP connected, outgoing call in progress */
        HFP_STATE_CONNECTED_INCOMING,       /*!< HFP connected, incoming call in progress */
        HFP_STATE_CONNECTED_ACTIVE,         /*!< HFP connected, active call in progress */
    HFP_STATE_DISCONNECTING					/*!< HFP disconnecting in progress */
} hfpState;

#define HFP_CONNECT_NO_ERROR_UI (1 << 0)    /*!< Don't indicate connection error */
#define HFP_CONNECT_NO_UI       (1 << 1)    /*!< Don't indicate connection success or error */
#define	HFP_DISCONNECT_NO_UI    (1 << 2)    /*!< Don't indicate disconnection */

struct appTaskData;

/*! \brief HFP instance structure

    This structure contains all the information for an HFP connection.
*/	
typedef struct
{
    TaskData task;									/*!< HFP task */

    hfpState    state;                              /*!< Current state */
    uint16      hfp_lock;                           /*!< HFP operation lock, used to serialise HFP operations */

    uint8       volume;                             /*!< Speaker volume */
    uint8       mic_volume;                         /*!< Microphone volume/gain */
    unsigned    in_band_ring:1;                     /*!< AG supports in-band ringing tone */
    unsigned    volume_repeat:1;
    unsigned    voice_recognition_active:1;         /*!< Voice recognition is active */
    unsigned    voice_recognition_request:1;        /*!< Voice recognition pending */
    
    unsigned    mute_active:1;                      /*!< Microphone mute is active */
    unsigned    caller_id_active:1;                 /*!< Caller ID is active */
    
    unsigned    flags:3;                            /*!< Flags indicating if we should connect/disconnect silently */
    
    hfp_call_state  call_state:4;                   /*!< Current call state */
    unsigned        call_accepted:1;                /*!< Flag indicating if we have accepted the call */
#ifdef INCLUDE_AV
    unsigned    sco_av_reconnect:1;                 /*!< Flag indicating if we connect AV after SCO */
#endif
    unsigned    sco_ui_indication:1;                /*!< Flag indicating if we need UI indication for SCO */    
    unsigned	encrypted:1;                        /*!< Flag indicating if ACL is encrypted */
    unsigned    detach_pending:1;                   /*!< Flag indicating that ACL detach is pending */
    unsigned    disconnect_reason:4;				/*!< Flag indicating the disconnect reason */
    
    hfp_profile profile;                            /*!< Profile currently used */
    uint16      sco_supported_packets;              /*!< Bitmap of supported SCO packets by both headset and AG*/
    bdaddr      ag_bd_addr;                         /*!< Address of connected AG */
    Sink        sco_sink;                           /*!< Sink for SCO, 0 if no SCO active */
    Sink        slc_sink;                           /*!< Sink for SLC */

    TaskList*   slc_status_notify_list;             /*!< List of tasks to notify of SLC connection status. */
    TaskList*   status_notify_list;                 /*!< List of tasks to notify of general HFP status changes */
    Task        at_cmd_task;                        /*!< Task to handle TWS+ AT commands. */

    bool        sco_forward_handling_volume;        /*!< SCOFWD is handling setting volume in kymera. */

    batteryRegistrationForm battery_form;
} hfpTaskData;

/*! \brief HFP settings structure

    This structure defines the HFP settings that are stored
    in persistent store.
*/    
typedef struct
{
    unsigned int volume:4;          /*!< Speaker volume */
    unsigned int mic_volume:4;      /*!< Microphone volume */
} hfpPsConfigData;

/*! \brief Internal message IDs */
enum
{
    HFP_INTERNAL_CONFIG_WRITE_REQ,				/*!< Internal message to store the HFP device config */
    HFP_INTERNAL_MUTE_IND,						/*!< Internal message to play mute reminder tone */
    HFP_INTERNAL_SCO_UNENCRYPTED_IND,			/*!< Internal message to indicate SCO is unencrypted */
    HFP_INTERNAL_HSP_INCOMING_TIMEOUT,			/*!< Internal message to indicate timeout from incoming call */
    HFP_INTERNAL_HFP_CONNECT_REQ,				/*!< Internal message to connect to HFP */
    HFP_INTERNAL_HFP_DISCONNECT_REQ,			/*!< Internal message to disconnect HFP */
    HFP_INTERNAL_HFP_LAST_NUMBER_REDIAL_REQ,	/*!< Internal message to request last number redial */
    HFP_INTERNAL_HFP_VOICE_DIAL_REQ,			/*!< Internal message to request voice dial */
    HFP_INTERNAL_HFP_VOICE_DIAL_DISABLE_REQ,	/*!< Internal message to disable voice dial */
    HFP_INTERNAL_HFP_CALL_ACCEPT_REQ,			/*!< Internal message to accept an incoming call */
    HFP_INTERNAL_HFP_CALL_REJECT_REQ,			/*!< Internal message to reject an incoming call */
    HFP_INTERNAL_HFP_CALL_HANGUP_REQ,			/*!< Internal message to hang up an active call */
    HFP_INTERNAL_HFP_MUTE_REQ,					/*!< Internal message to mute an active call */
    HFP_INTERNAL_HFP_TRANSFER_REQ,				/*!< Internal message to transfer active call between AG and device */
    HFP_INTERNAL_VOLUME_UP,						/*!< Internal message to increase the volume on the active call */
    HFP_INTERNAL_VOLUME_DOWN					/*!< Internal message to decrease the volume on the active call */
};

/*! \brief Message IDs from HFP to main application task */
enum av_headset_hfp_messages
{
    APP_HFP_INIT_CFM = APP_HFP_MESSAGE_BASE,    /*!< Indicate HFP has been initialised */
    APP_HFP_CONNECTED_IND,                      /*!< SLC connected */
    APP_HFP_DISCONNECTED_IND,                   /*!< SLC disconnected */
    APP_HFP_SCO_CONNECTED_IND,                  /*!< Active SCO connected*/
    APP_HFP_SCO_DISCONNECTED_IND,               /*!< SCO channel disconnect */
    APP_HFP_SLC_STATUS_IND,                     /*!< SLC status updated */
    APP_HFP_AT_CMD_CFM,                         /*!< Result of an send AT command request */
    APP_HFP_AT_CMD_IND,                         /*!< AT command received not handled within HFP profile  */
    APP_HFP_SCO_INCOMING_RING_IND,              /*!< There is an incoming call (not connected) */
    APP_HFP_SCO_INCOMING_ENDED_IND,             /*!< Incoming call has gone away (unanswered) */
    APP_HFP_VOLUME_IND,                         /*!< New HFP volume level */
};

/*! Message sent to status_notify_list clients indicating HFP profile has connected. */
typedef struct
{
    bdaddr bd_addr;			/*!< Address of AG */
} APP_HFP_CONNECTED_IND_T;

/*! \brief HFP disconnect reasons */
typedef enum
{
    APP_HFP_CONNECT_FAILED,			/*!< Connect attempt failed */
    APP_HFP_DISCONNECT_LINKLOSS,	/*!< Disconnect due to link loss following supervision timeout */
    APP_HFP_DISCONNECT_NORMAL,		/*!< Disconnect initiated by local or remote device */
    APP_HFP_DISCONNECT_ERROR		/*!< Disconnect due to unknown reason */
} appHfpDisconnectReason;

/*! Message sent to status_notify_list clients indicating HFP profile has disconnected. */
typedef struct
{
    bdaddr bd_addr;					/*!< Address of AG */
    appHfpDisconnectReason reason;  /*!< Disconnection reason */
} APP_HFP_DISCONNECTED_IND_T;

/*! Message sent to status_notify_list clients indicating SLC state. */
typedef struct
{
    bool slc_connected;				/*!< SLC Connected True/False */
    hfp_link_priority priority;		/*!< Priority of the link - used for multiple hfp connections */
    bdaddr bd_addr;					/*!< Address of AG */
} APP_HFP_SLC_STATUS_IND_T;

/*! Message sent to at_cmd_task with result of AT command transmission. */
typedef struct
{
    bool status;					/*!< Status indicating if AT command was sent successfully */
} APP_HFP_AT_CMD_CFM_T;

/*! Message sent to at_cmd_task indicating new incoming TWS AT command. */
typedef struct
{
    hfp_link_priority priority;		/*!< Priority of the link - used for multiple hfp connections */
    uint16 size_data;				/*!< Size of the AT command */
    uint8 data[1];					/*!< AT command */
} APP_HFP_AT_CMD_IND_T;

/*! Definition of #APP_HFP_VOLUME_IND message sent to registered
    clients. */
typedef struct
{
    uint8 volume;           /*!< Latest HFP volume level. */
} APP_HFP_VOLUME_IND_T;

/*! Internal connect request message */
typedef struct
{
    bdaddr addr;            /*!< Address of AG */
    hfp_profile profile;    /*!< Profile to use */
    uint16 flags;           /*!< Connection flags */
} HFP_INTERNAL_HFP_CONNECT_REQ_T;

/*! Internal disconnect request message */
typedef struct
{	
	bool silent;            /*!< Disconnect silent flag */
} HFP_INTERNAL_HFP_DISCONNECT_REQ_T;

/*! Internal mute request message */
typedef struct
{
    bool mute;              /*!< Mute enable/disable */
} HFP_INTERNAL_HFP_MUTE_REQ_T;

/*! Internal audio transfer request message */
typedef struct
{
    bool transfer_to_ag;    /*!< Transfer to AG/Headset */
} HFP_INTERNAL_HFP_TRANSFER_REQ_T;

/*! \brief Get HFP sink */
#define appHfpGetSink() (appGetHfp()->slc_sink)

/*! \brief Get HFP lock */
#define appHfpGetLock() (appGetHfp()->hfp_lock)

/*! \brief Set HFP lock */
#define appHfpSetLock(lock) (appGetHfp()->hfp_lock = (lock))

/*! \brief Get current HFP state */
#define appHfpGetState() (appGetHfp()->state)

/*! \brief Get current AG address */
#define appHfpGetAgBdAddr() (&(appGetHfp()->ag_bd_addr))

/*! \brief Is HFP connected */
#define appHfpIsConnected() \
    ((appHfpGetState() >= HFP_STATE_CONNECTED_IDLE) && (appHfpGetState() <= HFP_STATE_CONNECTED_ACTIVE))

/*! \brief Is HFP in a call */
#define appHfpIsCall() \
    ((appHfpGetState() >= HFP_STATE_CONNECTED_OUTGOING) && (appHfpGetState() <= HFP_STATE_CONNECTED_ACTIVE))

/*! \brief Is HFP in an active call */
#define appHfpIsCallActive() \
    (appHfpGetState() == HFP_STATE_CONNECTED_ACTIVE)

/*! \brief Is HFP in an incoming call */
#define appHfpIsCallIncoming() \
    (appHfpGetState() == HFP_STATE_CONNECTED_INCOMING)

/*! \brief Is HFP in an outgoing call */
#define appHfpIsCallOutgoing() \
    (appHfpGetState() == HFP_STATE_CONNECTED_OUTGOING)

/*! \brief Is HFP disconnected */
#define appHfpIsDisconnected() \
    ((appHfpGetState() < HFP_STATE_CONNECTING_LOCAL) || (appHfpGetState() > HFP_STATE_DISCONNECTING))
            
/*! \brief Is HFP SCO active */
#define appHfpIsScoActive() \
    (appGetHfp()->sco_sink != 0)

/*! \brief Is HFP SCO/ACL encrypted */
#define appHfpIsEncrypted() \
	(appGetHfp()->encrypted)
	
/*! \brief Is HFP voice recognition active */
#define appHfpIsVoiceRecognitionActive() \
    (appGetHfp()->voice_recognition_active)

/*! \brief Is current profile HSP */
#define appHfpIsHsp() \
	(appGetHfp()->profile == hfp_headset_profile)

/*! \brief Is microphone muted */
#define appHfpIsMuted() \
    (appGetHfp()->mute_active)

#define appHfpIsScoFwdHandlingVolume() \
    (appGetHfp()->sco_forward_handling_volume)

extern void appHfpInit(void);
extern bool appHfpConnectHandset(void);
extern bool appHfpConnectWithBdAddr(const bdaddr *bd_addr, hfp_profile profile);
extern void appHfpDisconnectInternal(void);
extern void appHfpVolumeStart(int16 step);
extern void appHfpVolumeStop(int16 step);
extern void appHfpMuteToggle(void);
extern void appHfpTransferToAg(void);
extern void appHfpTransferToHeadset(void);
extern void appHfpCallVoice(void);
extern void appHfpCallVoiceDisable(void);
extern void appHfpCallAccept(void);
extern void appHfpCallReject(void);
extern void appHfpCallHangup(void);
extern void appHfpCallLastDialed(void);
extern void appHfpSetDefaultAttributes(struct appDeviceAttributes *attributes);
extern void appHfpClientRegister(Task task);
extern void appHfpSendAtCmdReq(hfp_link_priority priority, char* cmd);
extern void appHfpRegisterAtCmdTask(Task task);
extern void appHfpStatusClientRegister(Task task);
extern uint8 appHfpGetVolume(void);
extern void appHfpScoFwdHandlingVolume(bool enabled);

#else

#define appHfpIsScoActive() (FALSE)

#endif
#endif
