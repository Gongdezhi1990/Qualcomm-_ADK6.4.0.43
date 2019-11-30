/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat.h

DESCRIPTION
    Header file for the SWAT profile library. The library exposes a functional
    downstream API and an upstream message based API.
*/
/*!
@file   swat.h
@brief  Interface to the Subwoofer Audio Transfer Profile library.

        When a device wishes to start streaming subwoofer audio content, the 
        device must first setup a streaming connection. During the stream setup,
        the devices select between either standard or low latency media channel.
        
        
        Once a streaming connection is established and the start streaming
        procedure is executed, audio can be streamed from the Source (SRC)
        to the Sink (SNK).

        This library provides the low level services to permit an audio stream
        to be configured, started, stopped and suspended.  This library
        provides the stream configuration and control.  The actual streaming of
        data is performed by the underlying firmware.  The audio stream is
        routed to the Digital Signal Processor (DSP) present on CSR BlueCore
        Multimedia devices.  The CPU intensive operation of encoding/decoding a
        media stream is performed by the DSP.

        The library exposes a functional downstream API and an upstream message
        based API.
*/

#ifndef SWAT_H_
#define SWAT_H_

#include <library.h>
#include <connection.h>

#include <sink.h>


#define SWAT_SIGNALLING_PSM     0x8001   /* L2CAP PSM for the signalling channel */
#define SWAT_MEDIA_PSM          0x8003   /* L2CAP PSM for the media channel */



/*!
    @brief SWAT media channel states
    
    Defines the recognised SWAT media channel states
*/
typedef enum
{
/*00*/  swat_media_closed = 0,          /*< The media channel is closed (not active) */
/*01*/  swat_media_opening,         /*< The media channel is in the process of opening */
/*02*/  swat_media_open,            /*< The media chaneel is open, but not streaming */
/*03*/  swat_media_starting,        /*< The media channel is in the process of starting to stream */
/*04*/  swat_media_streaming,       /*< The media channel is open and contains streaming audio data */
/*05*/  swat_media_suspending,      /*< The media channel is in the process of suspending a stream */
/*06*/  swat_media_closing,         /*< The media channel is in the process of closing down */
/*ff*/	swat_media_invalid = 0xff	/*< invalid value for swat media channel state */
} swatMediaState;



/*!
    @brief Swat roles
    
    The specification defines the SWAT roles.
*/
typedef enum
{
    swat_role_source,   /*< Role: SRC */
    swat_role_sink      /*< Role: SNK */
} swat_role;



/*!
    @brief Status code returned in messages from the SWAT library

    This status code indicates the outcome of the request.
*/
typedef enum
{
/*0x0*/    swat_success,                        /*< The operation succeeded. */
/*0x1*/    swat_init_failed,                    /*< An initiailisation failure */
/*0x2*/    swat_wrong_state,                    /*< The library is in the wrong state to perform the operation. */
/*0x3*/    swat_rejected_by_remote_device,      /*< The operation was rejected by the remote device. */
/*0x4*/    swat_sdp_fail,                       /*< SDP registration has failed */
/*0x5*/    swat_l2cap_fail,                     /*< L2CAP registration has failed */
/*0x6*/    swat_unknown_psm,                    /*< L2CAP registration for unrecognised PSM */
/*0x7*/    swat_l2cap_error,                    /*< L2CAP error */
/*0x8*/    swat_esco_fail,                      /*< ESCO registration has failed or ESCO connection failure*/
/*0x9*/    swat_no_signalling_connection,       /*< No signalling connection */
/*0xa*/    swat_signalling_already_connecting,  /*< Signalling channel is connecting */
/*0xb*/    swat_signalling_is_disconnecting,    /*< Signalling channel is disconnecting */
/*0xc*/    swat_signalling_already_connected,   /*< Signalling channel is connected */
/*0xd*/    swat_no_media_connection,            /*< No media connection */
/*0xe*/    swat_disconnect_link_loss,           /*< Link loss occured */
/*0xf*/    swat_max_connections,                /*< Library can't support any more signalling/media connections to a remote device */
/*0x10*/   swat_bad_data,                       /*< Data supplied by client is not recognised by library */
/*0x11*/   swat_signalling_no_response,         /*< The remote device did not respond to the signalling request */
/*0x12*/   swat_unknown_device,                 /*< Unknown device that is not allowed */
/*0x13*/   swat_signalling_error,               /*< An error occured when trying to send a signalling command to the remote device */
/*0x14*/   swat_rejected_by_client              /*< Used when the application task rejected a request from the remote device */
} swat_status_code;



/*!
    @brief Swat response code
    
    The specification defines the following responses to operations / requests.
*/
typedef enum
{
    swat_accept = 1,    /*< The request was accepted */
    swat_reject         /*< The request was rejected */
} swat_response_code;



/*!
    @brief Media Latency mode
    
    The specification defines the following media channels. The SNK must support all mdoes.
*/
typedef enum
{
    SWAT_MEDIA_NONE = 0x0,          /*< no media type */
    SWAT_MEDIA_STANDARD = 0x1,      /*< Standard latency mode */
    SWAT_MEDIA_LOW_LATENCY = 0x2    /*< Low latency mode */
} swatMediaType;



/*!
    @brief Swat states
    
    The specification defines the SWAT states.
*/
typedef enum
{
    swat_state_idle,         /*< Idle state */
    swat_state_opening,      /*< Opening state */
    swat_state_open,         /*< Open state */
    swat_state_streaming,    /*< Streaming state */
    swat_state_closing       /*< Closing state */
} swat_state;



/*
    Do not document this enum.
*/
typedef enum
{
    SWAT_INIT_CFM = SWAT_MESSAGE_BASE,      /* 00 */
    
    SWAT_SIGNALLING_CONNECT_IND,            /* 01 */
    SWAT_SIGNALLING_CONNECT_CFM,            /* 02 */
    SWAT_SIGNALLING_DISCONNECT_IND,         /* 03 */
    SWAT_SIGNALLING_DISCONNECT_CFM,         /* 04 */
    
    SWAT_MEDIA_OPEN_IND,                    /* 05 */
    SWAT_MEDIA_OPEN_CFM,                    /* 06 */
    SWAT_MEDIA_CLOSE_IND,                   /* 07 */
    SWAT_MEDIA_CLOSE_CFM,                   /* 08 */
    SWAT_MEDIA_START_IND,                   /* 09 */
    SWAT_MEDIA_START_CFM,                   /* 0a */
    SWAT_MEDIA_SUSPEND_IND,                 /* 0b */
    SWAT_MEDIA_SUSPEND_CFM,                 /* 0c */
    
    SWAT_SET_VOLUME_IND,                    /* 0d */
    SWAT_SET_VOLUME_CFM,                    /* 0e */
    
    SWAT_SAMPLE_RATE_IND,                   /* 0f */
    SWAT_SAMPLE_RATE_CFM,                   /* 10 */
    
    SWAT_GET_VERSION_IND,                   /* 11 */
    SWAT_GET_VERSION_CFM,                   /* 12 */

    
    SWAT_MESSAGE_TOP
    
} SwatMessageId;



/****************************************************************************
    
    SWAT API Functions
    
****************************************************************************/



/*!
    @brief Initialise the SWAT library
    
    @param clientTask The client task (usually the application) initialising the library.
    @param max_remote_devices The maximum number of remote devices allowed.
    @param role The role of the device as defined in the SWAT specification (SRC or SNK).
    @param auto_handle Flag to indicate whether the SWAT library should auto handle messages or leave for the application to handle
    @param service_record Pointer to a client supplied service record. Set zero to use library default
    @param size_service_record Size of the data area referenced by pointer "service_record"
    @param esco_config Pointer to esco Configuration. Set zero to use library default.
    
    The call to SwatInit initialises the Swat library. The initialisation function should be called only once.
    
    No further library functions should be called until the SWAT_INIT_CFM message has been
    received by the client task.
*/
void SwatInit(Task clientTask, uint16 max_remote_devs, swat_role role, bool auto_handle, const uint8 *service_record, uint16 size_service_record, const sync_config_params * esco_config);


/*!
    @brief Connect a signalling channel to the specified remote device.
    
    @param addr the Bluetooth address of the remote device
    
    The call to SwatSignallingConnectRequest attempts to establish a signalling channel with the specified remote device.
    Only one signalling channel per device is allowed.
    
    SWAT_SIGNALLING_CONNECT_CFM message will be sent to the client indicating the result of the connect request.
*/
bool SwatSignallingConnectRequest(bdaddr *addr);



/*!
    @brief Issue a response to an incoming signalling channel connect request from a remote device.
    
    @param device_id The identifier for the remote device
    @param connection_id The connection ID for the connection to send the response for
    @param identifer The identifier for the connection to send the response for
    @param accept The response to send to either accept or reject the connect request.
        
    The call to SwatSignallingConnectResponse is made on receipt of an SWAT_SIGNALLING_CONNECT_IND message.
    This function allows an application to either accept or reject the incoming connect request.
    Passing TRUE for the accept parameter will accept the connection, FALSE will reject it.
    The device_id is obtained from the SWAT_SIGNALLING_CONNECT_IND message.
    Only one signalling channel per device is allowed.
    
    SWAT_SIGNALLING_CONNECT_CFM message will be sent to the client indicating the result of the 
    conect request.
*/
bool SwatSignallingConnectResponse(uint16 device_id, uint16 connection_id, uint8 identifier, bool accept);



/*!
    @brief Disconnect a signalling channel from the specified remote device.
    
    @param device_id The identifier of the remote device.
    
    The call to SwatSignallingDisconnectRequest attempts to disconnect an established signalling channel from
    the specified remote device.
    Any media channels connected to the remote device will also be closed as a result of calling this function.
    
    SWAT_SIGNALLING_DISCONNECT_CFM message will be sent to the client indicating the result of the disconnect request
*/
bool SwatSignallingDisconnectRequest(uint16 device_id);



/*!
    @brief Connect a media channel to the specified remote device.
    
    @param device_id The identifier of the remote device.
    @param media_type The type of media channel to request (standard or high latency).
    
    The call to SwatMediaOpenRequest attempts to establish a media channel with the specified remote device.
    
    SWAT_MEDIA_OPEN_CFM message will be sent to the client indicating the result of the open request.
*/
bool SwatMediaOpenRequest(uint16 device_id, swatMediaType media_type);



/*!
    @brief Issues a response to an incoming Media channel open request from a remote device.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel this response is for
    @param accept The response to send to either accept or reject the open request.
    
    The call to SwatMediaOpenResponse is made on receipt of a SWAT_MEDIA_OPEN_IND message.
    This function allows the application to either accept or rejct the incoming open request.
    Passing TRUE for the accept parameter will accept the connection, FALSE will reject it.
    The device_id is obtained from the SWAT_MEDIA_OPEN_IND
    
    SWAT_MEDIA_OPEN_CFM message will be sent to the client indicating the result of the open request.
*/
bool SwatMediaOpenResponse(uint16 device_id, swatMediaType media_type, bool accept);



/*!
    @brief Request to close a media channel from the specified remote device.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel to close
    
    The call to SwatMediaCloseRequest attempts to close an established media channel from the
    specified remote device.
    
    SWAT_MEDIA_CLOSE_CFM message will be sent to the client indicating the result of the close request.
*/
bool SwatMediaCloseRequest(uint16 device_id, swatMediaType media_type);



/*!
    @brief Issues a response to an incoming Media channel close request from a remote device.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel this response is for
    @param accept The response to send to either accept or reject the close request.
    
    The call to SwatMediaCloseResponse is made on receipt of a SWAT_MEDIA_CLOSE_IND message.
    This function allows the application to either accept or rejct the incoming close request.
    Passing TRUE for the accept parameter will accept the close request, FALSE will reject it.
    The device_id is obtained from the SWAT_MEDIA_OPEN_IND
    
    SWAT_MEDIA_CLOSE_CFM message will be sent to the client indicating the result of the open request.
*/
bool SwatMediaCloseResponse(uint16 device_id, swatMediaType media_type, bool accept);



/*!
    @brief Request to start streaming audio data over the media channel.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel to start streaming on.
    
    The call to SwatMediaStartRequest attempts to place the specified media channel, connected to
    the specified remote device, in a streaming state.
    
    SWAT_MEDIA_START_CFM message will be sent to the client indicating the result of the start request.
*/
bool SwatMediaStartRequest(uint16 device_id, swatMediaType media_type);



/*!
    @brief Issues a response to an incoming start request from a remote device.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel this START response is for
    @param accept The response to send to either accept or reject the start request.
    
    The call to SwatMediaStartResponse is made on receipt of a SWAT_MEDIA_START_IND message.
    This function allows an application to either accept or reject the incoming start request.
    Passing TRUE for the accept parameter will accept the request, FALSE will reject it.
    Both device_id and media_type are obtained fromt he SWAT_MEDIA_START_CFM message.
    
    SWAT_MEDIA_START_CFM message will be sent to the client indicating the result of the start request.
*/
bool SwatMediaStartResponse(uint16 device_id, swatMediaType media_type, bool accept);



/*!
    @brief Request to cease the streaming of audio data over a media channel.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel to start streaming on.
    
    The call to SwatMediaSuspendRequest attempts to return the specified media channel, connected to
    the specified remote device, from a streaming state back to an open state.
    
    SWAT_MEDIA_SUSPEND_CFM message will be sent to the client indicating the result of the suspend request.
*/
bool SwatMediaSuspendRequest(uint16 device_id, swatMediaType media_type);


/*!
    @brief Issues a response to an incoming suspend request from a remote device.
    
    @param device_id The identifier of the remote device.
    @param media_type The media channel to aim the response at
    @param accept The response to send to either accept or reject the suspend request.
    
    The call to SwatMediaSuspendResponse is made on receipt of a SWAT_MEDIA_SUSPEND_IND message.
    This function allows an application to either accept or reject the incoming start request.
    Passing TRUE for the accept parameter will accept the request, FALSE will reject it.
    Both device_id and media_type are obtained fromt he SWAT_MEDIA_START_CFM message.
    
    SWAT_MEDIA_SUSPEND_CFM message will be sent to the client indicating the result of the start request.
*/
bool SwatMediaSuspendResponse(uint16 device_id, swatMediaType media_type, bool accept);



/*!
    @brief Request to set the volume level of the remote device
    
    @param device_id The identifier of the remote device
    @param volume The volume level to send to the remote device.
    @param sub_trim The subwoofer trim value to send to the remote device.
    
    The call to SwatSetVolume attempts to set the volume level of the remote device.
    
    SWAT_SET_VOLUME_CFM message will be sent to the client indicating the result of the set volume request.
*/
bool SwatSetVolume(uint16 device_id, uint8 volume, uint8 sub_trim);



/*!
    @brief Inform the remote device of the current sample rate
    
    @param device_id The identifier of the remote device.
    @param rate The sample rate being used by the device
    
    The call to SwatSendSampleRate can only be made when the local device has been registered as a SRC device.
    When the local device is not registered as a source device, this function will return FALSE, otherwise will return TRUE
    
    SWAT_SEND_SAMPLE_RATE_CFM message will be sent to the client indicating the result of the SWTA_SAMPLE_RATE_CMD
*/
bool SwatSendSampleRateCommand(uint16 device_id, uint16 rate);



/*!
    @brief Function to allow client task to respond to a SWAT_SAMPLE_RATE_IND message
    
    @param device_id The identifier of the remote device.
    @param rate The sample rate being used by the device
    
    SWAT_SEND_SAMPLE_RATE_CFM message will be sent to the client indicating the result of the SWAT_SAMPLE_RATE_CMD
*/
bool SwatSampleRateResponse(uint16 device_id, uint16 rate);



/*!
    @brief Function to obtain the current media channel state
    
    This function returns the SWAT_STANDARD_LATENCY media channel state
*/
swatMediaState SwatGetMediaState(uint16 device_id);



/*!
    @brief Function to obtain the current media channel state
    
    This function returns the SWAT_LOW_LATENCY media channel state
*/
swatMediaState SwatGetMediaLLState(uint16 device_id);



/*!
    @brief Function to obtain the sink for the media channel
    
    This function returns the sink of the active media channel, it
    will return NULL if there is no active media channel
*/
Sink SwatGetMediaSink(uint16 device_id);



/*!
    @brief Function to obtain the active media channel
    
    This function returns the media channel that is active, SWAT
    specification states that only one media channel may be active
    at any one time.
*/
swatMediaType SwatGetMediaType(uint16 device_id);



/*!
    @brief Function to obtain the sink for the media channel
    
    This function returns the sink of the signalling channel
*/
Sink SwatGetSignallingSink(uint16 device_id);



/*!
    @brief Function to obtain the Bluetooth address of the remote device
    
    This function returns the Bluetooth of the remote device that has
    the SWAT assigned id *device_id*
*/
bdaddr * SwatGetBdaddr(uint16 device_id);


/****************************************************************************/
/* function to get Subwoofer version number */
/****************************************************************************/
bool SwatSendGetVersionNoCommand(uint16 device_id);


/****************************************************************************
    
    SWAT Messages sent to the application task
    
****************************************************************************/



/*!
    @brief This message is sent as a response to calling SwatInit

    This message indicates the outcome of initialising the library.
*/
typedef struct
{
    /*! Status of the profile initialisation. */
    swat_status_code    status;
    
} SWAT_INIT_CFM_T;



/*!
    @brief An unsolicited message sent when a remote device attempts to establish a Signalling channel

    This message indicates the device attempting to establish connection
*/
typedef struct
{
    /*! The Identifier for the remote device */
    uint16 device_id;
    
    /*! The connection ID for the connection request */
    uint16 connection_id;
    
    /*! The identifier for the connection request */
    uint8 identifier;
    
    /*! The Bluetooth address of the remote device requesting connection */
    bdaddr bd_addr;
    
} SWAT_SIGNALLING_CONNECT_IND_T;



/*!
    @brief This message is sent as a response to calling either SwatSignallingConnectRequest or SwatSignallingConnectResponse

    This message indicates the outcome of the signalling channel connect operation
*/
typedef struct
{
    /*! Outcome of the connect operation */
    swat_status_code status;
    
    /*! The unique device ID assigned for the remote device */
    uint16 device_id;
    
    /*! The sink identifying the connection */
    Sink sink;
    
} SWAT_SIGNALLING_CONNECT_CFM_T;



/*!
    @brief An unsolicited message sent when a remote device is going to disconnect SWAT

    This message indicates the disconnecting device
*/
typedef struct
{
    /*! Outcome of the disconnect operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
} SWAT_SIGNALLING_DISCONNECT_CFM_T;



/*!
    @brief This message is sent as a response to calling either SwatSignallingDisconnectRequest or SwatSignallingDisconnectResponse

    This message indicates the device attempting to tear down connection
*/
typedef struct
{   
    /*! Identifier for the remote device */
    uint16 device_id;
    
} SWAT_SIGNALLING_DISCONNECT_IND_T;




/*!
    @brief An unsolicited message sent when a remote device attempts to establish a media channel

    This message indicates the device attempting to establish connection
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! Indicates which media type is being requested */
    swatMediaType media_type;
    
} SWAT_MEDIA_OPEN_IND_T;



/*!
    @brief This message is sent as a response to calling either SwatMediaOpenRequest or SwatMediaOpenResponse

    This message indicates the outcome of the media channel open operation
*/
typedef struct
{
    /*! Outcome of the open operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The media channel type (standard / low latency) */
    swatMediaType media_type;
    
    /*! The Sink associated with the media channel */
    Sink audio_sink;
    
} SWAT_MEDIA_OPEN_CFM_T;



/*!
    @brief An unsolicited message sent when a remote device attempts to tear down a media channel

    This message indicates the device attempting to tear down the connection
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! Indicates which media type is to be disconnected */
    swatMediaType media_type;
    
} SWAT_MEDIA_CLOSE_IND_T;



/*!
    @brief This message is sent as a response to calling either SwatMediaCloseRequest or SwatMediaCloseResponse

    This message indicates the outcome of the media channel disconnect operation
*/
typedef struct
{
    /*! Outcome of the disconnect operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The media channel type (standard / low latency) */
    swatMediaType media_type;
    
} SWAT_MEDIA_CLOSE_CFM_T;



/*!
    @brief An unsolicited message sent when a remote device requests to start streaming media

    This message indicates the device attempting to start streaming media
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The media channel associated with the START request */
    swatMediaType media_type;
    
} SWAT_MEDIA_START_IND_T;



/*!
    @brief This message is sent as a response to calling either SwatMediaStartRequest or SwatMediaStartResponse

    This message indicates the outcome of the start streaming operation
*/
typedef struct
{
    /*! Outcome of the start operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The media channel associated with the START request */
    swatMediaType media_type;
    
} SWAT_MEDIA_START_CFM_T;



/*!
    @brief An unsolicited message sent when a remote device requests to suspend streaming media

    This message indicates the device attempting to suspend streaming media
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The media channel associated with the START request */
    swatMediaType media_type;
    
} SWAT_MEDIA_SUSPEND_IND_T;



/*!
    @brief This message is sent as a response to calling either SwatMediaSuspendRequest or SwatMediaSuspendResponse

    This message indicates the outcome of the suspend streaming operation
*/
typedef struct
{
    /*! Outcome of the suspend operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The media channel associated with the SUSPEND request */
    swatMediaType media_type;
    
} SWAT_MEDIA_SUSPEND_CFM_T;



/*!
    @brief This message is sent as a response to call SwatSetVolume

    This message indicates the outcome of the set volume operation
*/
typedef struct
{
    /*! Outcome of the get codec type operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! Absolute volume level that has been set */
    uint8 volume;
    
    /*! Subwoofer volume trim gain that has been set */
    uint8 sub_trim;
    
} SWAT_SET_VOLUME_CFM_T;



/*!
    @brief An unsolicited message sent when a remote device modifies the volume

    This message indicates the device is sending a volume update
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! Absolute volume level that has been sent */
    uint8   volume;
    
    /*! Subwoofer volume trim gain that has been sent */
    uint8   sub_trim;
    
} SWAT_SET_VOLUME_IND_T;



/*!
    @brief An unsolicited message sent when a remote device informs of the sample rate in use

    This message indicates the device informing of its sample rate
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! Sample Rate in use */
    uint16 sample_rate;

} SWAT_SAMPLE_RATE_IND_T;

/*!
    @brief This message is sent as a response to calling SwatSendSampleRateCommand

    This message indicates the outcome of the send sample rate request
*/
typedef struct
{
    /*!* Outcome of the send sample rate operation */
    swat_status_code status;
    
    /*! Identifier for the remote device */
    uint16 device_id;
    
    /*! The sample rate that was sent to the remote device */
    uint16 sample_rate;
    
} SWAT_SAMPLE_RATE_CFM_T;

/*!
    @brief An unsolicited message sent when a remote device requests the version

    This message indicates the device requesting the version
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;
    
} SWAT_GET_VERSION_IND_T;


/*!
    @brief This message is sent as a response to calling SwatGetVersion

    This message indicates the outcome of the get version request
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint16 device_id;

    /*! Outcome of the get version operation */
    swat_status_code status;
    
    /*! The major version number */
    uint16 major;
    
    /*! The minor version number */
    uint16 minor;
    
} SWAT_GET_VERSION_CFM_T;


#endif /* SWAT_H_ */
