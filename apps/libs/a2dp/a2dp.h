/****************************************************************************
Copyright (c) 2004 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    a2dp.h

DESCRIPTION
    Header file for the A2DP profile library. The library exposes a functional
    downstream API and an upstream message based API.

*/

/*!
\defgroup a2dp a2dp
\ingroup vm_libs

\brief  Interface to the Advanced Audio Distribution Profile library.

\section a2dp_intro INTRODUCTION
        When a device wishes to start streaming audio content, the device must
        first set up a streaming connection.  During the stream setup, the
        devices select the most suitable audio streaming parameters.
        Application service capability and transport service capability are
        configured during this stream setup procedure.

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

@{

*/


#ifndef A2DP_H_
#define A2DP_H_


#include <connection.h>
#include <library.h>

/*!
    @name Service_Categories

    These are service categories to be used in service capabilities of a Stream
    End Point (SEP).

*/

/*!
    @brief The capability to stream media. This is manditory for the Advance Audio
    Distribution Profile.
*/
#define AVDTP_SERVICE_MEDIA_TRANSPORT       (1)
/*!
    @brief The reporting capability. This is not currently supported.
*/
#define AVDTP_SERVICE_REPORTING             (2)
/*!
    @brief The recovery capability. This is not currently supported.
*/
#define AVDTP_SERVICE_RECOVERY              (3)
/*!
    @brief The content protection capability.
*/
#define AVDTP_SERVICE_CONTENT_PROTECTION    (4)
/*!
    @brief The header compression capability. This is not currently supported.
*/
#define AVDTP_SERVICE_HEADER_COMPRESSION    (5)
/*!
    @brief The multiplexing capability. This is not currently supported.
*/
#define AVDTP_SERVICE_MULTIPLEXING          (6)
/*!
    @brief The codec capability for the Stream End Point.
*/
#define AVDTP_SERVICE_MEDIA_CODEC           (7)
/*!
    @brief The Av Sync delay reporting capability for the Stream End Point.
*/
#define AVDTP_SERVICE_DELAY_REPORTING       (8)


/*!
    @name Service information.

    Used to fill out the fields in a media codec capabilities structure.
*/

/*!
    @brief Defines the codec type as audio.
*/
#define AVDTP_MEDIA_TYPE_AUDIO              (0)
/*!
    @brief Defines the codec type as video.
*/
#define AVDTP_MEDIA_TYPE_VIDEO              (1)
/*!
    @brief Defines the codec type as multimedia.
*/
#define AVDTP_MEDIA_TYPE_MULTIMEDIA         (2)
/*!
    @brief Defines the codec as SBC. Manditory to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_SBC               (0)
/*!
    @brief Defines the codec as MPEG1/2. Optional to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_MPEG1_2_AUDIO     (1)
/*!
    @brief Defines the codec as AAC. Optional to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_MPEG2_4_AAC       (2)
/*!
    @brief Defines the codec as ATRAC. Optional to support for A2DP.
*/
#define AVDTP_MEDIA_CODEC_ATRAC             (4)
/*!
    @brief Defines a codec not supported in the A2DP profile.
*/
#define AVDTP_MEDIA_CODEC_NONA2DP           (0xff)
/*!
     @brief SCMS CP_TYPE value for the content protection capabilities (LSB).
*/
#define AVDTP_CP_TYPE_SCMS_LSB              (0x02)
/*!
     @brief SCMS CP_TYPE value for the content protection capabilities (MSB).
*/
#define AVDTP_CP_TYPE_SCMS_MSB              (0x00)




/*!
    @name Role defines when initialising the A2DP library. A device could register Source and Sink service records.
*/

/*!
    @brief Bit to indicate that the device supports the Sink role. Use for the role in A2dpInit to register the default service record.
*/
#define A2DP_INIT_ROLE_SOURCE                   (1)
/*!
    @brief Bit to indicate that the device supports the Source role. Use for the role in A2dpInit to register the default service record.
*/
#define A2DP_INIT_ROLE_SINK                     (2)


/*!
    @name CSR Faststream IDs.
*/

/*!
    @brief The CSR Vendor ID.
*/
#define A2DP_CSR_VENDOR_ID                      (0x0a000000)

/*!
    @brief The CSR Faststream Codec ID.
*/
#define A2DP_CSR_FASTSTREAM_CODEC_ID            (0x0100)


/*!
    @brief The CSR True Wireless Stereo v3 Codec ID for SBC.
*/
#define A2DP_CSR_TWS_SBC_CODEC_ID               (0x0301)

/*!
    @brief The CSR True Wireless Stereo v3 Codec ID for AAC.
*/
#define A2DP_CSR_TWS_AAC_CODEC_ID               (0x0401)

/*!
    @brief The CSR True Wireless Stereo v3 Codec ID for MP3.
*/
#define A2DP_CSR_TWS_MP3_CODEC_ID               (0x0501)


/*!
    @brief The CSR True Wireless Stereo v3 Codec ID for AptX.
*/
#define A2DP_CSR_TWS_APTX_CODEC_ID               (0x0601)

/*!
    @brief The CSR True Wireless Stereo v3 Codec ID for aptX Adaptive.
*/
#define A2DP_CSR_TWS_APTX_AD_CODEC_ID            (0x0701)

/*!
    @name CSR\Qualcomm apt-X IDs.
*/

/*!
	@brief The APT Vendor ID.
*/
#define A2DP_APT_VENDOR_ID				        (0x4f000000)	

/*!
	@brief The CSR apt-X Codec ID.
*/
#define A2DP_CSR_APTX_CODEC_ID			        (0x0100)

/*!
    @brief The QTI aptX LL Codec ID.
*/
#define A2DP_QTI_APTX_LL_CODEC_ID               (0x0200)

/*!
    @brief The CSR aptX ACL Sprint Codec ID.
*/
#define A2DP_CSR_APTX_ACL_SPRINT_CODEC_ID       (0x0200)

/*!
    @brief The QTI Vendor ID.
*/
#define A2DP_QTI_VENDOR_ID                      (0xd7000000U)

/*!
    @brief The QTI aptX-HD Codec ID.
*/
#define A2DP_QTI_APTXHD_CODEC_ID                (0x2400)

/*!
    @brief The QTI aptX Adaptive Codec ID.
*/
#define A2DP_QTI_APTX_AD_CODEC_ID                (0xAD00)

/*!
    @brief The QTI aptX TWS+ Codec ID.
*/
#define A2DP_QTI_APTX_TWS_PLUS_CODEC_ID         (0x2500)

/*!
    @brief The QTI SBC TWS+ Codec ID.
*/
#define A2DP_QTI_SBC_TWS_PLUS_CODEC_ID          (0x2600)

/*!
    @brief Used to indicate an invalid AV Sync delay
*/
#define A2DP_INVALID_AV_SYNC_DELAY (0xFFFF)


struct __A2DP;


/*!
    @brief The Advanced Audio Distribution Profile structure.
*/
typedef struct __A2DP A2DP;

/*!
    @brief The structure holding the information about the Stream End Points available on the device.
*/
typedef struct _device_sep_list device_sep_list;

/*!
    @brief Stream End Point type (source or sink).

    The Stream End Point type is defined in the AVDTP
    specification.

*/
typedef enum
{
    /*!  This states the device or Stream End Point takes the Source role. */
    a2dp_source,
    /*!  This states the device or Stream End Point takes the Sink role. */
    a2dp_sink,
    /*!  This states the device or Stream End Point has no defined role. */
    a2dp_role_undefined
} a2dp_role_type;


/*!
    @brief Stream End Point (SEP) Media type.

    The Media type of a SEP is defined in the Bluetooth assigned numbers
    document.
*/
typedef enum
{
    sep_media_type_audio,       /*!< Audio.*/
    sep_media_type_video,       /*!< Video.*/
    sep_media_type_multimedia   /*!< Multimedia.*/
} a2dp_sep_media_type;


/*!
    @brief Status code returned in messages from the A2DP library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    a2dp_success,                   /*!< The operation succeeded. */
    a2dp_reconnect_success,         /*!< The library has managed to reconnect a signalling channel following a link loss. */
    a2dp_invalid_parameters,        /*!< Invalid parameters supplied by the client. */
    a2dp_sdp_fail,                  /*!< SDP registration has failed. */
    a2dp_l2cap_fail,                /*!< L2CAP registration has failed. */
    a2dp_operation_fail,            /*!< The operation has failed. */
    a2dp_insufficient_memory,       /*!< No memory to perform the required task. */
    a2dp_wrong_state,               /*!< The library is in the wrong state to perform the operation. */
    a2dp_no_signalling_connection,  /*!< No signalling connection. */
    a2dp_no_media_connection,       /*!< No media connection. */
    a2dp_rejected_by_remote_device, /*!< Was rejected by the remote device. */
    a2dp_disconnect_link_loss,      /*!< Link loss occured. */
    a2dp_closed_by_remote_device,   /*!< Closed by remote device. */
    a2dp_max_connections,           /*!< Library can't support any more signalling/media connections to a remote device */
    a2dp_aborted,                   /*!< Connection was aborted. */
    a2dp_security_reject,           /*!< Security rejected. */
    FORCE_ENUM_TO_MIN_16BIT(a2dp_status_code)
} a2dp_status_code;


/*!
    @brief Type of content protection in use.
*/
typedef enum
{
    /*! No content protection in use. */
    avdtp_no_protection = (0),
    /*! SCMS-T content protection in use. */
    avdtp_scms_protection
} a2dp_content_protection;


/*!
    @brief Audio stream channel mode.

    The specification defines the following channel modes. The SNK must support
    all modes. It is mandatory for the SRC to support mono and at least one of
    the remaining three modes.
*/
typedef enum
{
    a2dp_mono,                                  /*!< Mono channel mode. */
    a2dp_dual_channel,                          /*!< Dual channel mode. */
    a2dp_stereo,                                /*!< Stereo channel mode. */
    a2dp_joint_stereo                           /*!< Joint stereo channel mode. */
} a2dp_channel_mode;


/*!
    @brief Stream End Point (SEP) Information.

    Contains details about a local SEP. The information here is constant for the lifetime of the SEP.
*/
typedef struct
{
    uint8 seid;                                 /*!< Unique ID for the SEP. */
    uint8 resource_id;                          /*!< Resource ID associated with the SEP. If a SEP is configured then all SEPs with the same resource ID will become in use. */
    a2dp_sep_media_type media_type:2;           /*!< The media type of the SEP. */
    a2dp_role_type role:2;                      /*!< The role of the SEP.  */
    unsigned library_selects_settings:1;        /*!< The library_selects_settings is set to TRUE so that the library selects the optimal settings for a codec when initiating a media connection. Setting it to FALSE will allow the application to choose how the SEP should be configured based on the remote SEP capabilities. */
    uint16 flush_timeout;                       /*!< The flush timeout for the SEP. Should be set to 0 to use the default timeout. */
    uint16 size_caps;                           /*!< The size of the capabilities for the SEP. */
    const uint8 *caps;                          /*!< The capabilities for the SEP. These can be taken from one of the default codec capability header files that are supplied by CSR. The service capabilities section of the AVDTP specification details the format of these capabilities. */
} sep_config_type;


/*!
    @brief Holds the details of one local SEP.
*/
typedef enum{
    A2DP_SEP_UNAVAILABLE = 0x01,   /* Set and cleared by client */
    A2DP_SEP_IN_USE      = 0x02,   /* Set and cleared through media channel setup/teardown */
    A2DP_SEP_ERROR       = 0xFF,   /* This is a status code to report error */
    FORCE_ENUM_TO_MIN_16BIT(a2dp_sep_status)
}a2dp_sep_status;
typedef struct sep_data_type
{
    const sep_config_type *sep_config;          /*!< Pointer to the constant details of the SEP. */
    unsigned in_use:2;                          /*!< Used to indicate if the SEP is initially in use. */
} sep_data_type;


#ifndef A2DP_SBC_ONLY
/*!
    @brief aptX LL params type.

    Parameters used for configuring aptX LL.
*/
typedef struct
{
    uint16 target_codec_level;     /*!< Target codec buffer fill level.*/
    uint16 initial_codec_level;    /*!< Initial codec buffer fill level.*/
    uint16 sra_max_rate;           /*!< SRA Maximum rate.*/
    uint16 sra_avg_time;           /*!< SRA Averaging time.*/
    uint16 good_working_level;     /*!< Good Working Buffer Level.*/
} aptx_sprint_params_type;

/*!
    @brief Target TTP latencies for aptX Adaptive NQ2Q mode.
*/
typedef struct
{
    unsigned low_latency_0_in_ms:8; /*!< TTP latency to use in low latency mode 0 */
    unsigned low_latency_1_in_ms:8; /*!< TTP latency to use in low latency mode 1 */
    unsigned high_quality_in_2ms:8; /*!< TTP latency to use in high quality mode */
    unsigned tws_legacy_in_2ms:8;   /*!< TTP latency to use in TWS mode (Legacy Stereo or mono) */
} aptx_adaptive_ttp_latencies_t;

/*!
    @brief Codec parameters specific to aptX Adaptive.
*/
typedef struct
{
    unsigned :13;
    unsigned avrcp_cmd_supported:1;         /*!< Source supports the custom AVRCP command for aptX Adaptive */
    unsigned q2q_enabled:1;                 /*!< If source is compatible Q2Q mode is used */
    unsigned is_twsp_mode:1;                /*!< If CODEC is in TWS+ mode */
    aptx_adaptive_ttp_latencies_t nq2q_ttp; /*!< Target TTP latencies used for NQ2Q mode */
} aptx_adaptive_params_t;
#endif

/*!
    @brief Holds details about the configured codec.

    These details about the configured codec are returned to the application, so it can supply this information to the audio library.
    The audio library will use this information to configure the DSP and route the audio depending on the codec in use.
*/
typedef struct
{
    /*! @todo if the unused is relevant, then order is different on Kalimba wrt Xap */
    unsigned :14;
    unsigned content_protection:1;              /*!< Content protection in use. */
    unsigned latency_reporting:1;               /*!< Latency (delay) reporting in use. */
    unsigned bitpool:8;                         /*!< The bitpool value. */
    unsigned format:8;                          /*!< The format. */
    uint16 packet_size;                         /*!< The packet size. */
    uint32 voice_rate;                          /*!< The voice rate. */
#ifndef A2DP_SBC_ONLY
    aptx_sprint_params_type aptx_sprint_params; /*!< aptX sprint parameters. */
    aptx_adaptive_params_t aptx_ad_params;      /*!< Codec parameters specific to aptX adaptive */
#endif
} codec_data_type;


/*!
    @brief Used to register up to 2 service records on initialisation of the A2DP library.
*/
typedef struct
{
    uint16 size_service_record_a;               /*!< Size of the first service record to be registered. */
    const uint8 *service_record_a;                  /*!< The first service record to be registered. */
    uint16 size_service_record_b;               /*!< Size of the second service record to be registered. */
    const uint8 *service_record_b;                  /*!< The second service record to be registered. */
} service_record_type;


/*
    Do not document this enum.
*/
#ifndef DO_NOT_DOCUMENT
#ifdef BUILD_FOR_23FW
#define A2DP_MESSAGE_BASE 0x6d00
#endif
typedef enum
{
    /* Library initialisation */
    A2DP_INIT_CFM = A2DP_MESSAGE_BASE,      /* 00 */

    /* New API */    
    A2DP_SIGNALLING_CONNECT_IND,            /* 01 */
    A2DP_SIGNALLING_CONNECT_CFM,            /* 02 */
    A2DP_SIGNALLING_DISCONNECT_IND,         /* 03 */
    A2DP_SIGNALLING_LINKLOSS_IND,           /* 04 */
    
    A2DP_MEDIA_OPEN_IND,                    /* 05 */
    A2DP_MEDIA_OPEN_CFM,                    /* 06 */
    A2DP_MEDIA_CLOSE_IND,                   /* 07 */
    A2DP_MEDIA_CLOSE_CFM,                   /* 08 */
    A2DP_MEDIA_START_IND,                   /* 09 */
    A2DP_MEDIA_START_CFM,                   /* 0A */
    A2DP_MEDIA_SUSPEND_IND,                 /* 0B */
    A2DP_MEDIA_SUSPEND_CFM,                 /* 0C */
    A2DP_MEDIA_RECONFIGURE_IND,             /* 0D */
    A2DP_MEDIA_RECONFIGURE_CFM,             /* 0E */
    A2DP_MEDIA_AV_SYNC_DELAY_IND,           /* 0F */
    A2DP_MEDIA_AV_SYNC_DELAY_CFM,           /* 10 */
    A2DP_MEDIA_AV_SYNC_DELAY_UPDATED_IND,   /* 11 */
    /* End of new API */
    
    A2DP_CODEC_CONFIGURE_IND,               /* 12 */
    A2DP_ENCRYPTION_CHANGE_IND,             /* 13 */
#if 0
    A2DP_GET_CURRENT_SEP_CAPABILITIES_CFM,  /* 14 */
#endif
    A2DP_LINKLOSS_RECONNECT_CANCEL_IND,     /* 14 */


    A2DP_MESSAGE_TOP
} A2dpMessageId;
#endif


/*!
    @brief This message is sent as a response to calling A2dpInit.

    This message indicates the outcome of initialising the library.
*/
typedef struct
{
    /*! Status of the profile initialisation. */
    a2dp_status_code    status;
} A2DP_INIT_CFM_T;


/*!
    @brief This message is sent to indicate that the application must choose how the codec should be configured,
    based on the remote codec capabilities.
    The application must respond immediately using the A2dpCodecConfigureResponse API. The application must choose the
    required parameters to use for this codec.

    Only ever received if the application is setting up an outgoing media connection and the local SEP, as identified by the 
    local_seid, is NOT configured to have codec parameters chosen automatically by the A2DP library.  
    The codec_service_caps will start with AVDTP_SERVICE_MEDIA_CODEC as the first byte, and then the other codec information follows.
    The format is detailed in the 'Media Codec Capabilities' section of the AVDTP profile specification.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8   device_id;
    /*! local seid corresponding to the remote codec */
    uint8   local_seid;
    /*! The size of the codec capabilities supplied. */
    uint16  size_codec_service_caps;
    /*! The codec capabilities of the remote device. */
    uint8   codec_service_caps[1];
} A2DP_CODEC_CONFIGURE_IND_T;


/*!
    @brief This is an unsolicited message sent to the application whenever the
    encryption status of the sink owned by this A2DP profile instance changes.

    This message is generated as a result of a procedure initiated by the
    remote device. The application does not have to take any action on
    receiving the message.  However, some applications may choose not to
    proceed with a connection if encyrption is disabled on it (by the remote
    device).
*/
typedef struct
{
    /*! Pointer to a2dp profile instance that is handling the signalling
      connection.*/
    A2DP    *a2dp;
    /*! Encryption status of this connection, TRUE if encrypted, FALSE
      otherwise. */
    bool    encrypted;
} A2DP_ENCRYPTION_CHANGE_IND_T;


/*!
    @brief Initialise the A2DP library.

    @param clientTask The client task (usually the application) initialising the library.
    @param role The role of the device as defined in the A2DP specification (SRC or SNK).
    @param service_records The service records to use or NULL.
    @param size_seps The number of Stream End Points supported on this device.
    @param seps The Stream End Points supported on this device.
    @param linkloss_timeout The period, in seconds, over which to attempt reconnection following a link loss.

    The call to A2dpInit initialises the A2dp library. The initialisation function should be called only once.

    If the client has supplied one or two service records then these will be used. Alternatively if the service_records
    parameter is set to NULL then the library can register its default service record(s) based on the role(s) supplied.
    The role should be a bit mask using the A2DP_INIT_ROLE_SOURCE and A2DP_INIT_ROLE_SINK definitions. If no role or
    service records are supplied then it is up to the application to register service records on a separate occasion.

    All the SEPs supported on the local device must also be supplied here. The library will allocate
    memory for this SEP information then pass a pointer back to the application in the A2DP_INIT_CFM
    message. This pointer must then be supplied when new connections are being created so the A2DP
    library instances knows about all the available SEPs.

    A2DP_INIT_CFM message will be sent to the clientTask indicating the result of the
    library initialisation.

    No further library functions should be called until the A2DP_INIT_CFM
    message has been received by the clientTask.
*/
void A2dpInit(Task clientTask, uint16 role, service_record_type *service_records, uint8 size_seps, sep_data_type *seps, uint16 linkloss_timeout);


/*!
    @brief Configure the maximum number of remote devices.

    @param max_remote_devs The maximum number remote devices that can connect
    
    This function can be used to set the maximum number of remote devices that
    can connect to either one (single point) or two (multi point). If this 
    function is called with max_remote_devs set to one there can only be at 
    most one device connected or it will fail. 
*/
bool A2dpConfigureMaxRemoteDevices(uint8 max_remote_devs);


/*********** New API ************/

#define INVALID_DEVICE_ID  0xFF
#define INVALID_STREAM_ID  0xFF

typedef enum
{
    a2dp_signalling_idle,           /*!< A Signalling channel is not present */
    a2dp_signalling_connecting,     /*!< A Signalling channel is being connected */
    a2dp_signalling_connected,      /*!< A Signalling channel has connected */
    a2dp_signalling_disconnecting   /*!< A Signalling channel is being disconnected */
} a2dp_signalling_state;


/*!
    @brief Connect a Signalling channel to the specified remote device.

    @param addr The Bluetooth address of the remote device.

    The call to A2dpSignallingConnectRequest attempts to establish a Signalling channel with the specified remote device. 
    Only one Signalling channel per device is allowed.

    A2DP_SIGNALLING_CONNECT_CFM message will be sent to the client indicating the result of the connect request.
*/
bool A2dpSignallingConnectRequest(const bdaddr *addr);


/*!
    @brief Connect a Signalling channel to the specified remote device and respond to specific task.

    @param addr The Bluetooth address of the remote device.
    @param clientTask The client task to send A2DP_SIGNALLING_CONNECT_CFM messages to.

    The call to A2dpSignallingConnectRequest attempts to establish a Signalling channel with the specified remote device. 
    Only one Signalling channel per device is allowed.

    A2DP_SIGNALLING_CONNECT_CFM message will be sent to the client indicating the result of the connect request.
*/
bool A2dpSignallingConnectRequestWithTask(const bdaddr *addr, Task clientTask);


/*!
    @brief Issues a response to an incoming Signalling channel connect request from a remote device.

    @param device_id The identifier of the remote device attempting to establish a Signalling channel.
    @param accept The response to send to either accept or reject the connect request.

    The call to A2dpSignallingConnectResponse is made on receipt of an A2DP_SIGNALLING_CONNECT_IND message.
    This function allows an application to either accept or reject the incoming connect request.
    Passing TRUE for the accept parameter will accept the connection, FALSE will reject it.
    The device_id is obtained from the A2DP_SIGNALLING_CONNECT_IND message.
    Only one Signalling channel per device is allowed.

    A2DP_SIGNALLING_CONNECT_CFM message will be sent to the client indicating the result of the
    connect request.
*/
bool A2dpSignallingConnectResponse(uint8 device_id, bool accept);


/*!
    @brief Issues a response to an incoming Signalling channel connect request from a remote device,
           and respond to a specific task.

    @param device_id The identifier of the remote device attempting to establish a Signalling channel.
    @param accept The response to send to either accept or reject the connect request.
    @param clientTask The client task to send A2DP_SIGNALLING_CONNECT_CFM messages to.

    The call to A2dpSignallingConnectResponse is made on receipt of an A2DP_SIGNALLING_CONNECT_IND message.
    This function allows an application to either accept or reject the incoming connect request.
    Passing TRUE for the accept parameter will accept the connection, FALSE will reject it.
    The device_id is obtained from the A2DP_SIGNALLING_CONNECT_IND message.
    Only one Signalling channel per device is allowed.

    A2DP_SIGNALLING_CONNECT_CFM message will be sent to the client indicating the result of the
    connect request.
*/
bool A2dpSignallingConnectResponseWithTask (uint8 device_id, bool accept, Task clientTask);


/*!
    @brief Disconnect a Signalling channel from the specified remote device.

    @param device_id The identifier of the remote device.

    The call to A2dpSignallingDisconnectRequest attempts to disconnect an established Signalling channel from
    the specified remote device.
    Any Media channels connected to the remote device will also be closed as a result of calling this function.

    A2DP_SIGNALLING_DISCONNECT_CFM message will be sent to the client indicating the result of the disconnect request.
*/
bool A2dpSignallingDisconnectRequest(uint8 device_id);


/*!
    @brief Obtain the Signalling channel sink.

    @param device_id The identifier of the remote device.

    The call to A2dpSignallingGetSink will return immediately with the sink of a Signalling channel connected to the 
    specified remote device.
    A sink value of NULL will be returned if no Signalling channel has been established.
    
*/
Sink A2dpSignallingGetSink(uint8 device_id);

/*!
    @brief Obtain the signalling state for the specified remote device.

    @param device_id The identifier of the remote device.

    The call to A2dpSignallingGetState will return immediately with the signalling state of the specified remote device.
*/
a2dp_signalling_state A2dpSignallingGetState(uint8 device_id);


/*!
    @brief Obtain the Bluetooth address of the specified remote device.

    @param device_id The identifier of the remote device.
    @param addr Pointer to a structure used to return the remote device's Bluetooth address.

    The call to A2dpDeviceGetBdaddr will, on success, return immediately with TRUE and place the Bluetooth address of the remote device 
    in the supplied structure.
    If the specified device is idle, the function will return FALSE and place a zero Bluetooth address in the supplied structure.
*/
bool A2dpDeviceGetBdaddr (uint8 device_id, bdaddr *addr);


/*!
    @brief Enables or disables automatic linkloss management.

    @param device_id The identifier of the remote device.
    @param enable Indicates whether to enable/disable automatic linkloss management.

    Used to selectively enable/disable automatic linkloss management within the A2DP library on a per device basis.  The default state for
    all new connections is disabled, regardless of the linkloss timeout specified in A2dpInit.  This can be called on the fly to enable/disable
    linkloss management for specific devices as and when required.  
    If enabled then the linkloss timeout specified in the call to A2dpInit must be non-zero for any automatic linkloss management to take place.
    If the specified device is not connected, the function will return FALSE.
*/
bool A2dpDeviceManageLinkloss (uint8 device_id, bool enable);

/*!
    @brief Read the linkloss flag of a given device.

    @param device_id The identifier of the remote device.

    Used to check if a device is currently in a linloss.
    Returns TRUE if the linkloss flag for this device is set, FALSE otherwise
*/
bool A2dpDeviceIsInLinkloss (uint16 device_id);

/*!
    @brief An unsolicited message sent when a remote device attempts to establish a Signalling channel.

    This message indicates the device attempting to establish connection.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
    /*! Bluetooth address of the remote device */
    bdaddr addr;
} A2DP_SIGNALLING_CONNECT_IND_T;


/*!
    @brief This message is sent as a response to calling either A2dpSignallingConnectRequest or A2dpSignallingConnectResponse.

    This message indicates the outcome of the Signalling channel connect operation.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
    /*! Outcome of the connect operation. */
    a2dp_status_code status;
    /*! Bluetooth address of the remote device */
    bdaddr addr;
    /*! Indicates if the signalling channel was initiated by the local device */
    bool locally_initiated;
} A2DP_SIGNALLING_CONNECT_CFM_T;


/*!
    @brief This message can be sent unsolicited or as a response to calling A2dpSignallingDisconnectRequest.

    This message indicates the outcome of either party requesting disconnection of the Signalling channel.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
    /*! Reason for the disconnection. */
    a2dp_status_code status;
    /*! Bluetooth address of the remote device */
    bdaddr addr;
    /*! Sink of the signalling channel that has just been disconnected */
    Sink sink;
} A2DP_SIGNALLING_DISCONNECT_IND_T;


/*!
    @brief This message is sent if a link loss condition has occurred and is for informational purposes only.

    This message indicates the device with which link loss has occurred.  The library will attempt to re-establish a
    Signalling channel for the linkloss_timeout period specified in the library initialisation function A2dpInit.
    If a Signalling Channel is re-established, an A2DP_SIGNALLING_CONNECT_CFM message will be issued.
    If reconnection fails, an A2DP_SIGNALLING_DISCONNECT_IND will be issued.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
} A2DP_SIGNALLING_LINKLOSS_IND_T;

/*!
    @brief This message is sent if a link loss reconnection timeout is reached. This helps to stop the link loss reminder

    This message indicates the device with which link loss reconnection timeout has occurred.  The linkloss_timeout period 
    is specified in the library initialisation function A2dpInit.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
} A2DP_LINKLOSS_RECONNECT_CANCEL_IND_T;



/*!
    @brief Choose to accept, with configuration parameters, or reject a remote codec.

    @param device_id The identifier of the remote device.
    @param accept Indicates whether to accept this codec or not.
    @param local_seid Local seid corresponding to remote codec, obtained from A2DP_CODEC_CONFIGURE_IND.
    @param size_codec_service_caps Size, in bytes, of supplied codec service caps.  May be zero only if not accepting remote codec.
    @param codec_service_caps Service caps containing parameters to configure remote codec.  May be NULL only if not accepting remote codec.
    
    Called in response to receiving an A2DP_CODEC_CONFIGURE_IND notification, which can be issued by the library on receipt of an
    AVDTP_GET_CAPABILITIES_RSP.
    Accepting the codec will cause the library to issue an AVDTP_SET_CONFIGURATION_CMD with the supplied
    codec parameters.  
    Rejecting the codec will cause the library to select the next remote codec from its preferred list, potentially resulting in another
    A2DP_CODEC_CONFIGURE_IND being issued, or causing an A2DP_MEDIA_OPEN_CFM notification indicating failure if all compatible remote codecs 
    have been exhausted.
*/
bool A2dpCodecConfigureResponse(uint8 device_id, bool accept, uint8 local_seid, uint16 size_codec_service_caps, uint8 *codec_service_caps);


typedef enum
{
    a2dp_stream_idle,               /*!< Dormant state.  No actions being performed.  No Media channel present. */
    a2dp_stream_discovering,        /*!< A list of codecs is being requested */
    a2dp_stream_configuring,        /*!< Negotiating a suitable codec and parameters */
    a2dp_stream_configured,         /*!< A codec has been selected and configured */
    a2dp_stream_opening,            /*!< A Media channel is being opened */
    a2dp_stream_open,               /*!< A Media channel has been established */
    a2dp_stream_starting,           /*!< Preparing to begin audio streaming */
    a2dp_stream_streaming,          /*!< Audio is cuurently being streamed over a Media channel */
    a2dp_stream_suspending,         /*!< Preparing to cease audio streaming */
    a2dp_stream_closing,            /*!< A Media channel is being closed */
    a2dp_stream_reconfiguring,      /*!< Reconfiguring a codec's parameters */
    a2dp_stream_aborting,           /*!< An error condition has occurred */
    FORCE_ENUM_TO_MIN_16BIT(a2dp_stream_state)
} a2dp_stream_state;


/*!
    @brief Connect a Media channel to the specified remote device.

    @param device_id The identifier of the remote device.
    @param size_seid_list The size of any supplied seid list.
    @param seid_list A priority ordered list of local SEIDs to use for the media connection.

    The call to A2dpMediaOpenRequest attempts to establish a Media channel with the specified remote device.
    The SEIDs specified in the seid list are used to override the default seid list, specified in A2dpInit.
    The seid list is optional and both params should be set to zero if the default seid list is required.
        
    A2DP_MEDIA_OPEN_CFM message will be sent to the client indicating the result of the open request.
*/
bool A2dpMediaOpenRequest(uint8 device_id, uint16 size_seid_list, uint8 *seid_list);


/*!
    @brief Issues a response to an incoming Media channel open request from a remote device.

    @param device_id The identifier of the remote device attempting to open a Media channel.
    @param accept The response to send to either accept or reject the open request.

    The call to A2dpMediaOpenResponse is made on receipt of an A2DP_MEDIA_OPEN_IND message.
    This function allows an application to either accept or reject the incoming open request.
    Passing TRUE for the accept parameter will accept the connection, FALSE will reject it.
    The device_id is obtained from the A2DP_MEDIA_OPEN_IND message.
    
    A2DP_MEDIA_OPEN_CFM message will be sent to the client indicating the result of the open request.
*/
bool A2dpMediaOpenResponse(uint8 device_id, bool accept);


/*!
    @brief Request to close a Media channel from the specified remote device.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpMediaCloseRequest attempts to close an established Media channel from
    the specified remote device.

    A2DP_MEDIA_CLOSE_CFM message will be sent to the client indicating the result of the close request.
*/
bool A2dpMediaCloseRequest(uint8 device_id, uint8 stream_id);


/*!
    @brief Request to start streaming audio data over a Media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpMediaStartRequest attempts to place the specified Media channel, connected to 
    the specified remote device, in a streaming state.

    A2DP_MEDIA_START_CFM message will be sent to the client indicating the result of the start request.
*/
bool A2dpMediaStartRequest(uint8 device_id, uint8 stream_id);


/*!
    @brief Issues a response to an incoming start streaming request from a remote device.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.
    @param accept The response to send to either accept or reject the open request.

    The call to A2dpMediaStartResponse is made on receipt of an A2DP_MEDIA_START_IND message.
    This function allows an application to either accept or reject the incoming start request.
    Passing TRUE for the accept parameter will accept the request, FALSE will reject it.
    Both device_id and stream_id are obtained from the A2DP_MEDIA_START_IND message.
    
    A2DP_MEDIA_START_CFM message will be sent to the client indicating the result of the start request.
*/
bool A2dpMediaStartResponse(uint8 device_id, uint8 stream_id, bool accept);


/*!
    @brief Request to cease the streaming of audio data over a Media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpMediaSuspendRequest attempts to return the specified Media channel, connected to 
    the specified remote device, from a streaming state back to an open state.

    A2DP_MEDIA_SUSPEND_CFM message will be sent to the client indicating the result of the suspend request.
*/
bool A2dpMediaSuspendRequest(uint8 device_id, uint8 stream_id);


/*!
    @brief Request to reconfigure the current audio codec used on a Media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.
    @param service_caps_size Size of the reconfiguration service capabilities data structue.
    @param service_caps Pointer to reconfiguration service capabilities data structure.

    The call to A2dpMediaReconfigureRequest attempts to reconfigure the capabilities of current codec used on the 
    specified Media channel.

    A2DP_MEDIA_RECONFIGURE_CFM message will be sent to the client indicating the result of the reconfigure request.
*/
bool A2dpMediaReconfigureRequest(uint8 device_id, uint8 stream_id, uint16 service_caps_size, const uint8 *service_caps);

/*!
    @brief Issue a delay report to inform a connected Source of the current AV Sync delay for the codec used by the specified Media channel.

    @param device_id The identifier of the remote device.
    @param seid The identifier of the media stream/channel.
    @param delay Current AV Sync delay, in 1/10 milliseconds.

    The call to A2dpMediaAvSyncDelayRequest will issue a delay report to inform an AV Source of the current AV Sync delay 
    present at the Sink device for the specified Media channel.

    A2DP_MEDIA_AV_SYNC_DELAY_CFM message will be sent to the client indicating the success of the delay report.
*/
bool A2dpMediaAvSyncDelayRequest(uint8 device_id, uint8 seid, uint16 delay);

/*!
    @brief Causes a delay report to be issued to a connected Source to setup the initial AV Sync delay for the codec used by the specified Media channel.

    @param device_id The identifier of the remote device.
    @param seid Local SEID corresponding to the AV Sync Delay update
    @param delay Current AV Sync delay, in 1/10 milliseconds.

    Called in response to an A2DP_MEDIA_AV_SYNC_DELAY_IND message that requires a response to be issued.
    The call to A2dpMediaAvSyncDelayResponse will issue a delay report to inform an AV Source of the initial AV Sync delay 
    present at the Sink device for the specified Media channel.

    A2DP_MEDIA_AV_SYNC_DELAY_CFM message will be sent to the client indicating the success of the delay report.
*/
uint16 A2dpMediaAvSyncDelayResponse (uint8 device_id, uint8 seid, uint16 delay);

/*!
    @brief Obtain the sink of a Media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpMediaGetSink will return immediately with the sink of the specified Media channel, connected to the 
    specified remote device.
    A sink value of NULL will be returned if the specifed Media channel has not been established.
    
*/
Sink A2dpMediaGetSink(uint8 device_id, uint8 stream_id);

/*!
    @brief Obtain the local role of a Media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpMediaGetRole will return immediately with the local role of the specified Media channel, connected to the 
    specified remote device.  The role value will indicate if the endpoint is operating as a source or a sink.
    A role value of a2dp_role_undefined will be returned if the specifed Media channel has not been established.
    
*/
a2dp_role_type A2dpMediaGetRole(uint8 device_id, uint8 stream_id);

/*!
    @brief Obtain the stream state of the specified media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpMediaGetState will return immediately with the stream state of the specified media channel, 
    connected to the specified remote device.
*/
a2dp_stream_state A2dpMediaGetState(uint8 device_id, uint8 stream_id);

typedef struct
{
    /*! The sampling rate for the PCM hardware. Can be used when calling AudioConnect. */
    uint32                  rate;
    /*! The channel mode for the audio being streamed. Can be used when calling AudioConnect.*/
    a2dp_channel_mode       channel_mode;
    /*! The local SEID in use. This is for informational purposes only. */
    uint8                   seid;
    /*! Sink for the media transport channel. */
    Sink                    sink;
    /*! The codec parameters to pass into the audio library. Can be used when calling AudioConnect. */
    codec_data_type         codecData;
    /*! Size of the configured capabilities. */
    uint16                  size_configured_codec_caps;
    /*! The configured capabilities. They will have the form: [AVDTP_SERVICE_MEDIA_TRANSPORT] [0] [AVDTP_SERVICE_MEDIA_CODEC] [size media codec data] ... etc.*/
    uint8                   configured_codec_caps[1];
} a2dp_codec_settings;

/*!
    @brief Obtain the codec settings for the specified media channel.

    @param device_id The identifier of the remote device.
    @param stream_id The identifier of the media stream/channel.

    The call to A2dpCodecGetSettings will return immediately with a pointer to a structure containing details of the 
    configured (negotiated) codec and the selected parameters for operation.  The information contained within this
    structure can be used to configure the appropriate audio plugin, based on the local SEID.
    It is expected that the application will free the memory associated with this pointer once it has finished with it.
    A NULL pointer is returned if no codec has been been selected for the specified Media channel.
*/
a2dp_codec_settings * A2dpCodecGetSettings(uint8 device_id, uint8 stream_id);

/*!
    @brief Set the availablity of a Stream Endpoint.

    @param device_id The identifier of the remote device.
    @param seid The stream endpoint identifier.
    @param available Set to TRUE to mark the SEID as available, otherwise set to FALSE

    A call to A2dpCodecSetAvailable will set the availability of the specified Stream Endpoint.
    Marking an endpoint as unavailable will make it invisible to a remote device and will not be returned in an 
    AVDTP Discover response.  
    Attempts by a remote device to configure an unavailable endpoint will result in a SEP_IN_USE error response.
    
    Returns TRUE if the specified endpoint is valid and thus was marked as (un)available, otherwise returns FALSE.
*/
bool A2dpCodecSetAvailable(uint8 device_id, uint8 seid, bool available);

/*!
    @brief Get the availablity of a Stream Endpoint.

    @param device_id The identifier of the remote device.
    @param seid The stream endpoint identifier.

    A call to A2dpCodecGetAvailable will return immediately with the current availability of the specified Stream Endpoint.
    A return value of TRUE indicates that the endpoint is available.
    A return value of FALSE indicates that the endpoint is unavailable (in use).
*/
a2dp_sep_status A2dpCodecGetAvailable(uint8 device_id, uint8 seid);

/*!
    @brief An unsolicited message sent when a remote device attempts to establish a Media channel.

    This message indicates the device attempting to open a Media channel.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Local SEID being opened */
    uint8  seid;
} A2DP_MEDIA_OPEN_IND_T;


/*!
    @brief This message is sent as a response to calling either A2dpMediaOpenRequest or A2dpMediaOpenResponse.

    This message indicates the outcome of the Media channel open operation.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8 stream_id;
    /*! Local SEID opened */
    uint8  seid;
    /*! Outcome of the media open operation. */
    a2dp_status_code status;
    /*! Indicates if the media channel was initiated by the local device */
    bool locally_initiated;
} A2DP_MEDIA_OPEN_CFM_T;


/*!
    @brief This message can be sent unsolicited or as a response to calling A2dpMediaCloseRequest.

    This message indicates the outcome of either party requesting the close of a Media channel.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8  stream_id;
    /*! Reason for the media close. */
    a2dp_status_code status;
} A2DP_MEDIA_CLOSE_IND_T;


/*!
    @brief An unsolicited message indicating the remote device that has started a Media stream.

    This message indicates the remote device that has placed a Media stream into its streaming state.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8  stream_id;
} A2DP_MEDIA_START_IND_T;


/*!
    @brief This message is sent as a response to calling A2dpMediaStartRequest.

    This message indicates the result of the start request.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8 device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8 stream_id;
    /*! Outcome of the media start operation. */
    a2dp_status_code status;
} A2DP_MEDIA_START_CFM_T;


/*!
    @brief An unsolicited message indicating the remote device that has suspended a Media stream.

    This message indicates the remote device that has placed a Media stream into its open state.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8  stream_id;
} A2DP_MEDIA_SUSPEND_IND_T;


/*!
    @brief This message is sent as a response to calling A2dpMediaSuspendRequest.

    This message indicates the result of the suspend request.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8  stream_id;
    /*! Outcome of the media suspend operation. */
    a2dp_status_code status;
} A2DP_MEDIA_SUSPEND_CFM_T;


/*!
    @brief An unsolicited message indicating the remote device has reconfigured a Media stream.

    This message indicates the remote device has altered the operating parameters of the codec
    currently in use on a Media stream.  Calling A2dpCodecGetSettings will obtain the reconfigured
    codec parameters.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8  stream_id;
} A2DP_MEDIA_RECONFIGURE_IND_T;


/*!
    @brief This message is sent as a response to calling A2dpMediaReconfigureRequest.

    This message indicates the result of the reconfigure request.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Identifier for a media stream connected to the remote device */
    uint8  stream_id;
    /*! Outcome of the media reconfigure operation. */
    a2dp_status_code status;
} A2DP_MEDIA_RECONFIGURE_CFM_T;


/*!
    @brief An unsolicited message indicating the AV Sync delay of the remote device requires an update.

    This message indicates the remote device requires an update of the AV Sync delay for the configured 
    Media stream.  This should be done by calling A2dpMediaAvSyncDelayResponse with an appropriate AV Sync
    delay value.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Local SEID corresponding to the required AV Sync Delay update */
    uint8  seid;
} A2DP_MEDIA_AV_SYNC_DELAY_IND_T;

/*!
    @brief This message is sent as a response to calling either A2dpMediaAvSyncDelayRequest or
    A2dpMediaAvSyncDelayResponse.

    This message indicates the remote device has accepted the update of the AV Sync delay for the 
    specified Media stream.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Local SEID corresponding to the AV Sync Delay update */
    uint8  seid;
    /*! Outcome of the av sync delay operation. */
    a2dp_status_code status;
} A2DP_MEDIA_AV_SYNC_DELAY_CFM_T;


/*!
    @brief An unsolicited message indicating the AV Sync delay of the remote device has been updated.

    This message indicates the remote device has updated the AV Sync delay for the configured Media stream.
*/
typedef struct
{
    /*! Identifier for the remote device */
    uint8  device_id;
    /*! Local SEID corresponding to the updated AV Sync Delay */
    uint8  seid;
    /*! Updated remote AV Sync delay, in 1/10 milliseconds */
    uint16 delay;
} A2DP_MEDIA_AV_SYNC_DELAY_UPDATED_IND_T;

#endif /* A2DP_H_ */

/** @} */

