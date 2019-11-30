/*!
\copyright  Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.\n
            All Rights Reserved.\n
            Qualcomm Technologies International, Ltd. Confidential and Proprietary.
\version    
\file       av_headset_av.h
\brief      Header file AV State Machines (A2DP & AVRCP) 

    @startuml

    [*] -down-> NULL : appAvInit()
    NULL : Initialising AV application module
    NULL -down-> INITIALISING_A2DP : A2dpInit()

    INITIALISING_A2DP : Initialising A2DP profile library
    INITIALISING_A2DP -down-> INITIALISING_AVRCP : A2DP_INIT_CFM/AvrcpInit()

    INITIALISING_AVRCP : Initialising AVRCP profile library
    INITIALISING_AVRCP -down-> IDLE : AVRCP_INIT_CFM

    IDLE : Initialised and ready for connections

    @enduml
*/

#ifndef _AV_HEADSET_AV_H_
#define _AV_HEADSET_AV_H_

#include <a2dp.h>
#include <avrcp.h>
#include "av_headset.h"
#include "av_headset_device.h"
#include "av_headset_tasklist.h"
#include "av_headset_a2dp.h"
#include "av_headset_avrcp.h"

#ifdef INCLUDE_AV

/*! Maximum number of AV connections for TWS */
#define AV_MAX_NUM_TWS (1)
/*! Maximum number of AV connections for audio Sinks */
#define AV_MAX_NUM_SNK (1)

/*! \brief Maximum number of AV connections

    Defines the maximum number of simultaneous AV connections that are allowed.

    This is based on the number of links we want as TWS or as a standard Sink.

    Expectation is a value of 1 for a standard AV headset, or 2 for a True Wireless
    headset.
*/    
#define AV_MAX_NUM_INSTANCES (AV_MAX_NUM_TWS + AV_MAX_NUM_SNK)

/*! Special value to indicate a volume has not been set. */
#define VOLUME_UNSET 0xff

/*! Maximum allowed volume setting. */
#define VOLUME_MAX (127)

/*! \brief Test if a volume is set.
    \param volume The volume to test.
    \return TRUE if the volume is set.
*/
#define appAvVolumeIsSet(volume) ((volume) != VOLUME_UNSET)

/*! \brief Codec types used for instance identification */
typedef enum
{
    AV_CODEC_ANY,
    AV_CODEC_TWS,
    AV_CODEC_NON_TWS
} avCodecType;

/*! \brief A2DP stream endpoint IDs 

    \note Don't changing the ordering of these IDs as the A2DP code
          makes assumptions on the ordering.
*/
enum
{
    AV_SEID_INVALID = 0,
    AV_SEID_SBC_SRC = 0x01,                 /*!< SBC source endpoint for PTS testing */

    AV_SEID_SBC_SNK = 0x02,                 /*!< SBC Sink endpoint for standard handsets */
    AV_SEID_AAC_SNK = 0x03,                 /*!< AAC Sink endpoint for standard handsets */
    AV_SEID_APTX_SNK = 0x04,                /*!< APTX Sink endpoint for standard handsets */
    AV_SEID_APTX_ADAPTIVE_SNK = 0x05,       /*!< APTX adaptive Sink endpoint for standard handsets (placeholder) */

    AV_SEID_SBC_MONO_TWS_SRC = 0x06,        /*!< Mono TTP SBC Source endpoint for earbud forwarding TWS */
    AV_SEID_AAC_STEREO_TWS_SRC = 0x07,      /*!< Stereo TTP AAC Source endpoint for earbud forwarding TWS */
    AV_SEID_APTX_MONO_TWS_SRC = 0x08,       /*!< Mono TTP APTX Source endpoint for earbud forwarding TWS  */
    AV_SEID_APTX_ADAPTIVE_TWS_SRC = 0x09,   /*!< TTP APTX adaptive Source endpoint for earbud forwarding TWS (placeholder) */

    AV_SEID_SBC_MONO_TWS_SNK = 0x0A,        /*!< Mono TTP SBC Sink endpoint for peer earbud receiving TWS */
    AV_SEID_AAC_STEREO_TWS_SNK = 0x0B,      /*!< Stereo TTP AAC Sink endpoint for peer earbud receiving TWS */
    AV_SEID_APTX_MONO_TWS_SNK = 0x0C,       /*!< Mono TTP APTX Sink endpoint for peer earbud receiving TWS & TWS+ Handsets */
    
    /* Virtual SEID, generated when A2DP SEID from library is AV_SEID_APTX_ADAPTIVE_SNK but configured for TWS+ */
    AV_SEID_APTX_ADAPTIVE_TWS_SNK = AV_SEID_APTX_ADAPTIVE_SNK + 0x10,  /*!< Mono TTP APTX adaptive Sink endpoint for TWS+ Handsets */
};

extern const sep_config_type av_sbc_snk_sep;
extern const sep_config_type av_aac_snk_sep;
extern const sep_config_type av_aptx_snk_sep;
extern const sep_config_type av_aptx_mono_tws_snk_sep;
extern const sep_config_type av_aptx_adaptive_tws_snk_sep;
extern const sep_config_type av_sbc_mono_tws_snk_sep;
extern const sep_config_type av_aac_stereo_tws_snk_sep;
extern const sep_config_type av_aptx_mono_tws_src_sep;
extern const sep_config_type av_sbc_mono_tws_src_sep;
extern const sep_config_type av_aac_stereo_tws_src_sep;
extern const sep_config_type av_sbc_src_sep;

extern void appAvUpdateSbcMonoTwsCapabilities(uint8 *caps, uint32_t sample_rate);
extern void appAvUpdateAptxMonoTwsCapabilities(uint8 *caps, uint32_t sample_rate);
extern void appAvUpdateSbcCapabilities(uint8 *caps, uint32_t sample_rate);


/*! \brief AV task state machine states */
typedef enum
{
    AV_STATE_NULL,                  /*!< Startup state */
    AV_STATE_INITIALISING_A2DP,     /*!< Initialising A2DP profile library */
    AV_STATE_INITIALISING_AVRCP,    /*!< Initialising AVRCP profile library */
    AV_STATE_IDLE                   /*!< Initialised and ready for connections */
} avState;

/*! \brief AV instance structure

    This structure contains all the information for a AV (A2DP & AVRCP)
    connection.
*/
typedef struct avInstanceTaskData
{
    TaskData        av_task;                    /*!< Task/Message information for this instance */
    
    bdaddr          bd_addr;                    /*!< Bluetooth Address of remote device */
    bool            detach_pending;             /*!< Flag indicating if the instance is about to be detatched */

    TaskList        *av_status_client_list;     /*!< List of tasks to get status indications about this AV instance */

    avrcpTaskData   avrcp;
    a2dpTaskData    a2dp;

} avInstanceTaskData;  


/*! When responding to an incoming AVRCP connection, these are the 
    allowed responses */
typedef enum
{
    AV_AVRCP_REJECT,
    AV_AVRCP_ACCEPT,
    AV_AVRCP_ACCEPT_PASSIVE     /*!< The passive means accept connection, but make no attempt to maintain it */
} avAvrcpAccept;

/*! \brief AV task data

    Structure used to hold data relevant only to the AV task. Other modules
    should not access this information directly.
*/
typedef struct avTaskData
{
    TaskData        task;                   /*!< Task for messages */
    unsigned        state:2;                /*!< Current state of AV state machine */
    unsigned        volume_repeat:1;
    avSuspendReason suspend_state;          /*!< Bitmap of active suspend reasons */
    uint8           volume;                 /*!< The AV volume */
    avInstanceTaskData *av_inst[AV_MAX_NUM_INSTANCES];  /*!< AV Instances */

    TaskList        *avrcp_client_list;     /*!< List of tasks registered via \ref appAvAvrcpClientRegister */
    TaskList        *av_status_client_list; /*!< List of tasks registered via \ref appAvStatusClientRegister.
                                                These receive indications for AVRCP, A2DP and A2DP streaming */
} avTaskData;

/*! \brief Internal message IDs */
enum
{
    AV_INTERNAL_REMOTE_IND,
    AV_INTERNAL_REMOTE_REPEAT,
    AV_INTERNAL_VOLUME_UP_REPEAT,
    AV_INTERNAL_VOLUME_DOWN_REPEAT,
};

/*! \brief Message IDs from AV to main application task */
enum av_headset_av_messages
{
    AV_INIT_CFM = AV_MESSAGE_BASE,
    AV_A2DP_CONNECTED_IND,
    AV_A2DP_DISCONNECTED_IND,

    AV_AVRCP_CONNECT_IND,
    AV_AVRCP_CONNECT_CFM,
    AV_AVRCP_DISCONNECT_IND,            /*!< Indication sent when a link starts disconnecting */
    
    AV_AVRCP_SET_VOLUME_IND,
    AV_AVRCP_VOLUME_CHANGED_IND,
    AV_AVRCP_PLAY_STATUS_CHANGED_IND,
    
    AV_AVRCP_CONNECTED_IND,
    AV_AVRCP_DISCONNECTED_IND,          /*!< Indication sent when a link completed disconnection */
    AV_AVRCP_VENDOR_PASSTHROUGH_IND,
    AV_AVRCP_VENDOR_PASSTHROUGH_CFM,

    AV_CREATE_IND,
    AV_DESTROY_IND,
    AV_STREAMING_ACTIVE_IND,
    AV_STREAMING_INACTIVE_IND
};



/*! \brief Message sent to indicate that an A2DP link has disconnected. 
    This is sent to all tasks registered for messages */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     /*!< Bluetooth address of connected device */
    bool local_initiated;               /*!< Whether the connection was requested on this device */
} AV_A2DP_CONNECTED_IND_T;


/*! \brief Reasons for an A2DP link disconnection. 
    This is a reduced list compared to hci disconnect codes available */
typedef enum
{
    AV_A2DP_CONNECT_FAILED,         /*!< A connection attempt failed */
    AV_A2DP_DISCONNECT_LINKLOSS,    /*!< Link dropped */
    AV_A2DP_DISCONNECT_NORMAL,      /*!< A requested disconnect completed */
    AV_A2DP_DISCONNECT_ERROR,       /*!< Disconnect due to some error */
} avA2dpDisconnectReason;

/*! \brief Message sent to indicate that an A2DP link has disconnected. 
    This is sent to all tasks registered for messages */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     /*!< Bluetooth address of disconnected device */
    avA2dpDisconnectReason reason;      /*!< Reason for disconnection */
} AV_A2DP_DISCONNECTED_IND_T;




/*! \brief Message sent to indicate that an AVRCP link has connected. 
    This is sent to all tasks registered for avrcp messages */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     /*!< Bluetooth address of connected device */
    uint16 connection_id;               /*!< Connection ID */
    uint16 signal_id;                   /*!< Signalling identifier  */
} AV_AVRCP_CONNECT_IND_T;

/*! Message sent to indicate result of requested AVRCP link connection */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    avrcp_status_code status;           /*!< Status of the connection request */
} AV_AVRCP_CONNECT_CFM_T;

/*! Message sent to indicate that an AVRCP link connection has disconnected */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    avrcp_status_code status;           /*!< Reason for the disconnection */
} AV_AVRCP_DISCONNECT_IND_T;



/*! Message sent to indicate an AVRCP link has connected */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     /*!< Bluetooth address of the disconnected device */
    Sink sink;                          /*!< The Sink for the link */
} AV_AVRCP_CONNECTED_IND_T;

/*! \brief Message sent to indicate an AVRCP link has completed disconnection process 
    Normally expect a \ref AV_AVRCP_DISCONNECT_IND, followed by \ref AV_AVRCP_DISCONNECTED_IND.
 */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     /*!< Bluetooth address of the disconnected device */
} AV_AVRCP_DISCONNECTED_IND_T;

/*! Message sent to report an incoming passthrough message */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    uint16 opid;                        /*!< The passthrough operation ID */
    uint16 size_payload;                /*!< Size of the variable length payload (in octets) */
    uint8 payload[1];                   /*!< Actual payload. The message is variable length. */
} AV_AVRCP_VENDOR_PASSTHROUGH_IND_T;

/*! Message sent to confirm an outgoing passthrough has been processed */
typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    avrcp_status_code status;           /*!< The status of the request. Can be failure. */
    uint16 opid;                        /*!< The passthrough operation ID */
} AV_AVRCP_VENDOR_PASSTHROUGH_CFM_T;

/*! Internal message for a remote control message */
typedef struct
{
    avc_operation_id op_id; /*!< Command to repeat */
    uint8 state;            /*!< State relevant to the command. Often just 0 or 1. */
    bool beep;              /*!< Whether to issue a UI indication when this command is applied */
} AV_INTERNAL_REMOTE_IND_T;

typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                    
    uint8 volume;
} AV_AVRCP_SET_VOLUME_IND_T;

typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     
    uint8 volume;
} AV_AVRCP_VOLUME_CHANGED_IND_T;

typedef struct
{
    avInstanceTaskData *av_instance;    /*!< The AV instance this applies to */
    bdaddr bd_addr;                     
    avrcp_response_type play_status;
} AV_AVRCP_PLAY_STATUS_CHANGED_IND_T;


/*! Internal message for a repeated remote control message */
typedef struct
{
    avc_operation_id op_id; /*!< Command to repeat */
    uint8 state;            /*!< State relevant to the command. Often just 0 or 1. */
    bool beep;              /*!< Whether to issue a UI indication when this command is applied */
} AV_INTERNAL_REMOTE_REPEAT_T;

/*! Internal message for a volume repeat */
typedef struct
{
    int16 step;         /*!< Step to adjust volume by +ve or -ve */
} AV_INTERNAL_VOLUME_REPEAT_T;

/*! \brief Reasons why the other instance needs to sync */
typedef enum a2dp_inst_sync_reasons
{
    A2DP_INST_SYNC_REASON_MEDIA_CONNECTING,    /*!< The instance's media channel is connecting */
    A2DP_INST_SYNC_REASON_MEDIA_DISCONNECTED,  /*!< The instance's media channel was disconnected */
    A2DP_INST_SYNC_REASON_MEDIA_SUSPENDED,     /*!< The instance's media channel is connected/suspended */
    A2DP_INST_SYNC_REASON_MEDIA_STARTING,      /*!< The instance's media channel stream is starting */
    A2DP_INST_SYNC_REASON_CODEC_RECONFIGURED,  /*!< The instance's codec is reconfigured */
    A2DP_INST_SYNC_REASON_MEDIA_STREAMING,     /*!< The instance's media channel is now streaming */
} a2dpInstSyncReason;

/*! \brief Internal A2DP & AVRCP message IDs */
enum av_headset_internal_messages
{
    AV_INTERNAL_A2DP_BASE,
    AV_INTERNAL_A2DP_SIGNALLING_CONNECT_IND = AV_INTERNAL_A2DP_BASE,
    AV_INTERNAL_A2DP_CONNECT_REQ,
    AV_INTERNAL_A2DP_DISCONNECT_REQ,
    AV_INTERNAL_A2DP_SUSPEND_MEDIA_REQ,
    AV_INTERNAL_A2DP_RESUME_MEDIA_REQ,
    AV_INTERNAL_A2DP_CONNECT_MEDIA_REQ,
    AV_INTERNAL_A2DP_SYNC_MEDIA_IND,
    AV_INTERNAL_A2DP_DISCONNECT_MEDIA_REQ,
    AV_INTERNAL_AVRCP_UNLOCK_IND,
    AV_INTERNAL_A2DP_DESTROY_REQ,
    AV_INTERNAL_A2DP_INST_SYNC_IND,     /*!< Indication one instance wants the other instance to sync to its state. */
    AV_INTERNAL_A2DP_INST_SYNC_RES,     /*!< The instance's reponse to the sync indication. */
    AV_INTERNAL_A2DP_CODEC_RECONFIG_IND,/*!< Indication the instance's codec was reconfigured, the other instance may
                                             also need to reconfigure */
    AV_INTERNAL_A2DP_TOP,

    AV_INTERNAL_AVRCP_BASE,
    AV_INTERNAL_AVRCP_CONNECT_IND = AV_INTERNAL_AVRCP_BASE, /*!< Internal indication of signalling channel connection */
    AV_INTERNAL_AVRCP_CONNECT_RES,
    AV_INTERNAL_AVRCP_CONNECT_REQ,            /*!< Internal request to connect AVRCP */
    AV_INTERNAL_AVRCP_CONNECT_LATER_REQ,      /*!< Internal request to connect AVRCP later */
    AV_INTERNAL_AVRCP_DISCONNECT_REQ,         /*!< Internal request to disconnect AVRCP */
    AV_INTERNAL_AVRCP_DISCONNECT_LATER_REQ,   /*!< Internal request to disconnect AVRCP later */
    AV_INTERNAL_AVRCP_DESTROY_REQ,            /*!< Internal request to clean up */
    AV_INTERNAL_AVRCP_REMOTE_REQ,             /*!< Internal request to send AVRCP passthrough command */
    AV_INTERNAL_AVRCP_REMOTE_REPEAT_REQ,      /*!< Internal request to send repeat AVRCP passthrough command */
    AV_INTERNAL_AVRCP_VENDOR_PASSTHROUGH_RES,
    AV_INTERNAL_AVRCP_VENDOR_PASSTHROUGH_REQ, /*!< Internal request to send a vendor passthrough command */
    AV_INTERNAL_AVRCP_NOTIFICATION_REGISTER_REQ,
    AV_INTERNAL_AVRCP_PLAY_REQ,               /*!< Internal request to send AVRCP play */
    AV_INTERNAL_AVRCP_PAUSE_REQ,              /*!< Internal request to send AVRCP pause */
    AV_INTERNAL_AVRCP_PLAY_TOGGLE_REQ,        /*!< Internal request to send AVRCP play or pause depending on current playback status */
    AV_INTERNAL_AVRCP_CLEAR_PLAYBACK_LOCK_IND, /*!< Timeout waiting for playback status notifiation, clear lock */
    AV_INTERNAL_SET_ABSOLUTE_VOLUME_IND,      /*!> Internal message to apply volume */
    AV_INTERNAL_AVRCP_TOP,

    AV_INTERNAL_VOLUME_STORE_REQ,
};

/*! Internal indication of signalling channel connection */
typedef struct
{
    uint16      device_id;              /*!< A2DP Device Identifier */
    unsigned    flags:6;                /*!< Connect flags */
} AV_INTERNAL_A2DP_SIGNALLING_CONNECT_IND_T;

/*! Internal request to connect signalling channel */
typedef struct
{
    unsigned    flags:6;                /*!< Connect flags */
    unsigned    num_retries:3;          /*!< Number of connect retries */
} AV_INTERNAL_A2DP_CONNECT_REQ_T;

/*! Internal request to disconnect */
typedef struct
{
    unsigned    flags:6;                /*!< Disconnect flags */
} AV_INTERNAL_A2DP_DISCONNECT_REQ_T;

/*! Internal request to connect media channel */
typedef struct
{
    uint8  seid;                        /*!< Required SEID, or 0 if no preference */
    uint16 delay_ms;                    /*!< Delay in milliseconds to connect media channel */
} AV_INTERNAL_A2DP_CONNECT_MEDIA_REQ_T;

/*! \brief Internal indication to sync A2DP instances. The sender may lock waiting
    for a #AV_INTERNAL_A2DP_INST_SYNC_RES before processing further internal messages.
*/
typedef struct
{
    avInstanceTaskData *instance;       /*!< The instance to send the response, or NULL for no response. */
    a2dpInstSyncReason reason;          /*!< Reason for the sync */
    uint8 seid;                         /*!< Currently active SEID */
    uint8 device_id;                    /*!< The A2DP device id */
    uint8 stream_id;                    /*!< The A2DP stream id */
    uint8 sync_id;                      /*!< The ID for this sync indication. */
} AV_INTERNAL_A2DP_INST_SYNC_IND_T;

/*! \brief Internal response to sync A2DP instances.
*/
typedef struct
{
    a2dpInstSyncReason reason;          /*!< Reason for the sync */
    uint8 sync_id;                      /*!< The ID for this sync indication. */
} AV_INTERNAL_A2DP_INST_SYNC_RES_T;

/*! \brief Internal indication that the A2DP codec has been reconfigured */
typedef struct
{
    uint8      device_id;         /*!< A2DP Device Identifier */
    uint8      stream_id;         /*!< A2DP Stream Identifier */
} AV_INTERNAL_A2DP_CODEC_RECONFIG_IND_T;

/*! Internal request to suspend streaming */
typedef struct
{
    avSuspendReason reason;             /*!< Suspend reason */
} AV_INTERNAL_A2DP_SUSPEND_MEDIA_REQ_T;

/*! Internal request to resume streaming */
typedef struct
{
    avSuspendReason reason;             /*!< Start reason */
} AV_INTERNAL_A2DP_RESUME_MEDIA_REQ_T;

/*! Internal request to send SEP capabilities to remote device */
typedef struct
{
    /*! A2dp library transaction id that should be passed back to library when calling A2dpGetCapsResponse() */
    uint8  id;
    /*! The seid being queried */
    const sep_config_type *sep_config;
} AV_INTERNAL_A2DP_GET_CAPS_IND_T;

/*! Internal indication of channel connection */
typedef struct
{
    uint16  connection_id;  /*!< The ID for this connection.  Must be returned as part of the AvrcpConnectResponse API. */
    uint16  signal_id;      /*!< Signalling identifier */
} AV_INTERNAL_AVRCP_CONNECT_IND_T;

/*! Internal routing of a response to a connection request, indicating acceptance */
typedef struct
{
    Task            ind_task;       /*!< Task that received indication */
    Task            client_task;    /*!< Task responding */
    uint16          connection_id;  /*!< Connection ID */
    uint16          signal_id;      /*!< Signalling identifier  */
    avAvrcpAccept   accept;         /*!< How the connection is accepted/rejected */
} AV_INTERNAL_AVRCP_CONNECT_RES_T;

/*! Internal request to connect AVRCP channel */
typedef struct
{
    /*! Task requesting the connection. */
    Task    client_task;
} AV_INTERNAL_AVRCP_CONNECT_REQ_T;

/*! Internal request message to disconnect AVRCP channel */
typedef struct
{
    Task    client_task;    /*!< Task requesting the disconnect.  */
} AV_INTERNAL_AVRCP_DISCONNECT_REQ_T;

/*! Internal message returning the response to a 
    \ref AV_INTERNAL_AVRCP_VENDOR_PASSTHROUGH_REQ */
typedef struct
{
    avrcp_response_type response;       /*!< Response code */
} AV_INTERNAL_AVRCP_VENDOR_PASSTHROUGH_RES_T;

/*! Internal message to pass a vendor command over AVRCP.
    Used for TWS signalling to both peer and handset */
typedef struct
{
    Task client_task;           /*!< Task to receive response messages */
    avc_operation_id op_id;     /*!< ID to be sent */
    uint16 size_payload;        /*!< Number of octets in the payload, 0 allowed */
    uint8 payload[1];           /*!< Start of command payload. Message is variable length */
} AV_INTERNAL_AVRCP_VENDOR_PASSTHROUGH_REQ_T;

/*! Internal message to initiate a remote control request, possibly repeating. */
typedef struct
{
    avc_operation_id op_id; /*!< Operation ID */
    uint8 state;            /*!< Button press or release */
    unsigned ui:1;          /*!< Flag when set indicates tone should be played */
    uint16 repeat_ms;       /*!< Period between repeats (0 for none) */
} AV_INTERNAL_AVRCP_REMOTE_REQ_T;

/*! Internal message to trigger a remote control request repeatedly. 
    Same structure as AV_INTERNAL_AVRCP_REMOTE_REQ_T, which is for the first
    request. */
typedef AV_INTERNAL_AVRCP_REMOTE_REQ_T AV_INTERNAL_AVRCP_REMOTE_REPEAT_REQ_T;

/*! Internal message to initiate registering notifications. */
typedef struct
{
    avrcp_supported_events event_id;
} AV_INTERNAL_AVRCP_NOTIFICATION_REGISTER_REQ_T;

/*! \brief Get the AV volume */
#define appAvVolumeGet() (appGetAv()->volume)

/*! \brief Get Sink for AV instance */
#define appAvGetSink(theInst) \
    (appA2dpIsConnected(theInst) ? A2dpSignallingGetSink((theInst)->a2dp.device_id) : \
                                   appAvrcpIsConnected(theInst) ? AvrcpGetSink((theInst)->avrcp.avrcp) : 0)

/*! \brief Check if a AV instance has same Bluetooth Address */
#define appAvIsBdAddr(bd_addr) \
    (appAvInstanceFindFromBdAddr((bd_addr)) != NULL)


/*! A2DP connect/disconnect flags */
typedef enum
{
    A2DP_CONNECT_NOFLAGS      = 0,         /*!< Empty flags set */
    A2DP_CONNECT_SILENT_ERROR = (1 << 0),  /*!< Don't indicate connection error */
    A2DP_CONNECT_SILENT       = (1 << 1),  /*!< Don't indicate connection success */
    A2DP_CONNECT_MEDIA        = (1 << 3),  /*!< Connect media channel automatically */
    A2DP_DISCONNECT_SILENT    = (1 << 4),  /*!< Don't indicate disconnection */
    A2DP_START_MEDIA_PLAYBACK = (1 << 5),  /*!< Start media playback on connect */
} appAvA2dpConnectFlags;

extern void appAvInit(void);

extern bool appAvA2dpConnectRequest(const bdaddr *bd_addr, appAvA2dpConnectFlags a2dp_flags);
extern bool appAvA2dpMediaConnectRequest(const bdaddr *bd_addr);
extern bool appAvA2dpDisconnectRequest(avInstanceTaskData *av_inst);

extern bool appAvAvrcpConnectRequest(Task client_task, const bdaddr *bd_addr);
extern void appAvAvrcpConnectResponse(Task ind_task, Task client_task, const bdaddr *bd_addr, uint16 connection_id, uint16 signal_id, avAvrcpAccept accept);
extern bool appAvAvrcpDisconnectRequest(Task client_task, avInstanceTaskData* av_inst);

extern void appAvAvrcpClientRegister(Task client_task, uint8 interests);


extern bool appAvConnectPeer(void);
extern bool appAvDisconnectPeer(void);
extern bool appAvConnectHandset(bool play);
extern bool appAvConnectHandsetA2dpMedia(void);
extern bool appAvDisconnectHandset(void);
extern bool appAvConnectWithBdAddr(const bdaddr *bd_addr);

extern void appAvStatusClientRegister(Task client_task);

extern void appAvStreamingSuspend(avSuspendReason reason);
extern void appAvStreamingResume(avSuspendReason reason);

extern void appAvVolumeHandleAvDisconnect(avInstanceTaskData *theInst);
extern void appAvVolumeHandleAvrcpConnect(avInstanceTaskData *theInst);
extern void appAvVolumeHandleA2dpConnect(avInstanceTaskData *theInst);
extern void appAvVolumeSet(uint8 volume, avInstanceTaskData *);
extern void appAvVolumeStart(int16 step);
extern void appAvVolumeStop(int16 step);
extern bool appAvVolumeRepeat(int16 step);
extern bool appAvVolumeChange(int16 step);
extern void appAvInstanceHandleAvAvrcpSetVolumeInd(avInstanceTaskData *theInst, AV_AVRCP_SET_VOLUME_IND_T *ind);
extern void appAvInstanceHandleAvAvrcpVolumeChangedInd(avInstanceTaskData *theInst, AV_AVRCP_VOLUME_CHANGED_IND_T *ind);
extern void appAvInstanceHandleAvAvrcpPlayStatusChangedInd(avInstanceTaskData *theOtherInst, AV_AVRCP_PLAY_STATUS_CHANGED_IND_T *ind);

extern void appAvPause(bool ui);
extern void appAvPlay(bool ui);
extern void appAvPlayToggle(bool ui);
extern void appAvStop(bool ui);
extern void appAvForward(void);
extern void appAvBackward(void);
extern void appAvFastForwardStart(void);
extern void appAvFastForwardStop(void);
extern void appAvRewindStart(void);
extern void appAvRewindStop(void);

extern void appAvVolumeUpRemoteStart(void);
extern void appAvVolumeUpRemoteStop(void);
extern void appAvVolumeDownRemoteStart(void);
extern void appAvVolumeDownRemoteStop(void);

extern avInstanceTaskData *appAvInstanceCreate(const bdaddr *bd_addr);
extern void appAvInstanceDestroy(avInstanceTaskData *theInst);

extern avInstanceTaskData *appAvGetA2dpSink(avCodecType codec_type);
extern avInstanceTaskData *appAvGetA2dpSource(void);
extern avInstanceTaskData *appAvInstanceFindFromBdAddr(const bdaddr *bd_addr);
extern avInstanceTaskData *appAvInstanceFindA2dpState(const avInstanceTaskData *theInst, uint8 mask, uint8 expected);
extern avInstanceTaskData *appAvInstanceFindAvrcpForPassthrough(void);
extern avInstanceTaskData *appAvInstanceFindAvrcpFromBdAddr(const bdaddr *bd_addr);
extern avInstanceTaskData *appAvInstanceFindAvrcpOther(avInstanceTaskData *theInst);

extern bool appAvInstanceShouldConnectMediaChannel(const avInstanceTaskData *theInst, uint8 *seid);
extern bool appAvInstanceShouldStartMediaStreamingOnMediaOpenCfm(const avInstanceTaskData *theInst);
extern bool appAvInstanceShouldStartMediaStreamingOnMediaStartInd(const avInstanceTaskData *theInst);

extern void appAvInstanceA2dpConnected(avInstanceTaskData *theInst); 
extern void appAvInstanceA2dpDisconnected(avInstanceTaskData *theInst); 
extern void appAvInstanceAvrcpConnected(avInstanceTaskData *theInst); 
extern void appAvInstanceAvrcpDisconnected(avInstanceTaskData *theInst); 


extern bool appAvHasAConnection(void);
extern bool appAvIsConnectedMaster(void);
extern bool appAvIsConnectedSlave(void);

extern bool appAvIsDisconnected(void);
extern bool appAvIsStreaming(void);
extern avrcp_play_status appAvPlayStatus(void);
extern void appAvHintPlayStatus(avrcp_play_status status);

bool appAvIsValidInst(avInstanceTaskData* theInst);

extern bool appAvIsAvrcpConnected(avInstanceTaskData* theInst);
extern void appAvVolumeAttributeStore(avTaskData *theAv);

extern bool appAvInstanceStartMediaPlayback(avInstanceTaskData* theInst);

#endif

#endif
