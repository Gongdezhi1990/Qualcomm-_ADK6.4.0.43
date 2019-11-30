/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast.h
  @brief Interface to the Broadcast library.

  This library supports both the broadcaster and receiver roles in a broadcast
  environment.

  A single instance of the broadcast library is associated with a single
  broadcast (LT_ADDR), in either the broadcaster or receiver roles. An
  application should create an instance of the library for each broadcast
  stream it wishes to transmit or receive. 

  Typical usage for a broadcast role would be:-
    - #BroadcastInit()
    - #BroadcastConfigureBroadcaster()
    - #BroadcastStartBroadcast()
    - #BroadcastStopBroadcast()

  Typical usage for a receiver role would be:-
    - #BroadcastInit()
    - #BroadcastConfigureReceiver()
    - #BroadcastStartReceiver()
    - #BroadcastStopReceiver()
*/

#ifndef BROADCAST_H_
#define BROADCAST_H_

#include <library.h>
#include <message.h>
#include <bdaddr.h>
#include <sink.h>
#include <source.h>
#include <connection.h>
#include <csb.h>

/****************************************************************************
 * Defines
 ****************************************************************************/

/* Specific 'pmalloc' pool configurations. */
#define PMALLOC_BROADCAST_EXTRAS {2048, 1 },

/* Define our own message base if we don't get one from library.h */
#ifndef BROADCAST_MESSAGE_BASE
#define BROADCAST_MESSAGE_BASE                  0x7E00
#endif

/*! Currently only 1 broadcast address is supported. */ 
#define LT_ADDR_DEFAULT                         2

/*! Length in bytes of the the device info association data field. */
#define BROADCAST_DEVICE_STATUS_MAX_LEN         2
/*! Length in bytes of the maximum size of the security key, typically will
    be 16 bytes => 128 bits key, plux 2 bytes of type information. */
#define BROADCAST_KEY_MAX_LEN                   18
/*! Length of the broadcast identifier field of the association data. */
#define BROADCAST_IDENTIFIER_LEN                2

/* Product & Version identifier offsets needs to be modified in case BROADCAST_IDENTIFIER_LEN changes */
#define BROADCAST_IDENTIFIER_PRODUCT_OFFSET        0
#define BROADCAST_IDENTIFIER_VERSION_OFFSET         1

/****************************************************************************
 * Typedefs and Enumerations
 ****************************************************************************/

struct __BROADCAST;
/*! Opaque reference to an instance of the broadcast library. */
typedef struct __BROADCAST BROADCAST;

/*! LT address of a broadcast. */
typedef uint16 ltaddr;

/*! Types of roles in a broadcast environment.
 */
typedef enum
{
    /*! Broadaster role, this device sends data. */
    broadcast_role_broadcaster,

    /*! Receiver role, this device receives data from a device in
        the broadcaster role. */
    broadcast_role_receiver
} broadcast_role;

/*! Broadcast library status codes.
 */
typedef enum
{
    /*! The requested operation was successful. */
    broadcast_success,

    /*! Failed to initialise a Broadcast library instance. */
    broadcast_init_failed,

    /*! The requested operation is not permitted for the configured role. */
    broadcast_bad_role,

    /*! A configured parameter is not valid. */
    broadcast_bad_param,

    /*! The BROADCAST reference used in an API call is not valid */
    broadcast_bad_instance,
    
    /*! The operation is not supported for the specified LT_ADDR. */
    broadcast_invalid_lt_addr,

    /*! Unable to start the sync train with configured parameters. */
    broadcast_sync_train_start_failed,
    
    /*! Unable to receive the sync train with configured parameters. */
    broadcast_sync_train_receive_failed,

    /*! Unable to reconfigure a broadcast, it is still active. */
    broadcast_configure_failed_active,

    /*! Successfully discovered the sync train. */
    broadcast_sync_train_found,

    /*! A non-continuous sync train run has ended. */
    broadcast_sync_train_tx_ended,

    /*! Unable to start the data broadcast with the configured parameters. */
    broadcast_data_broadcast_start_failed,

    /*! Failed to receive the broadcast data with the parameters found in the
        sync train. */
    broadcast_data_broadcast_receive_failed,

    /*! Failed to configure association. */
    broadcast_association_config_failed,

    /*! A Broadcast instance has an active broadcast. */
    broadcast_is_active,

    /*! A Broadcast instance has no active broadcast. */
    broadcast_not_active,

    /*! A Broadcast instance has an active sync train. */
    broadcast_sync_train_is_active,

    /*! A Broadcast instance has no active sync train. */
    broadcast_sync_train_not_active,

    /*! A broadcast library destroy request failed, the instance is still
     * active. Stop the broadcast or receive first. */
    broadcast_destroy_failed_still_active,

    /*! Association failed due to unsupported broadcaster version. */
    broadcast_association_failed_unsupported_version,

    /*! A Broadcast instance in the receiver role is not permitted to request
        operations in the current mode. */
    broadcast_bad_mode
} broadcast_status;

/*! Modes in which a broadcaster or receiver may be started and stopped.
 */
typedef enum
{
    /*! Automatically start/stop broadcast or receive of both the data
        broadcast and the sync train. */
    broadcast_mode_auto,

    /*! Only start/stop broadcast of the data broadcast. 
        Not valid in the receiver role. */
    broadcast_mode_broadcast_only,

    /*! Only start/stop broadcast or receive of the sync train. */
    broadcast_mode_synctrain_only
} broadcast_mode;

/*! Parameters for configuring transmission of broadcast data.
 */
typedef struct
{
    /*! Informs the BR/EDR controller whether it is allowed to sleep. */
    bool lpo_allowed;

    /*! Bitmask of permitted packet types. */
    uint16 packet_type;

    /*! Minimum interval between CSB packets in slots.
        Range: 0x0002-0xFFFE; only even values are valid. */
    uint16 interval_min;

    /*! Maximum interval between CSB packets in slots.
        Range: 0x0002-0xFFFE; only even values are valid. */
    uint16 interval_max;

    /*! Duration in slots after which the BR/EDR controller reports a CSB
        timeout event if it is unable to transmit a CSB packet.
        Range: 0x0002-0xFFFE; only even values are valid. */
    uint16 supervision_timeout;
} broadcaster_csb_params;

/*! Parameters for configuring transmission of broadcast sync train.
 */
typedef struct
{
    /*! Minimum interval between sync train packets in slots.
        Range: 0x0020-0xFFFE; only even values are valid. */
    uint16 interval_min;
    
    /*! Maximum interval between sync train packets in slots.
        Range: 0x0020-0xFFFE; only even values are valid. */
    uint16 interval_max;
    
    /*! Duration in slots to continue sending the sync train.
        Configure a value of 0 to continuously send the sync train
        and not timeout.
        Range: 0x00000000 or 0x00000002-07FFFFFE; only even values are valid. */
    uint32 sync_train_timeout;
    
    /*! Single octet of application specific data to be included in the
        sync train.
        Range: 0x00-0xFF. */
    uint8 service_data;
} broadcaster_sync_params;

/*! Parameters for configuring reception of broadcast data.
 */
typedef struct
{
    /*! Interval between CSB packet instances in slots.
        Range: 0x0002-0xFFFE; only even values are valid. */
    uint16 interval;

    /*! (CLKNslave - CLK) % 2^38. */
    uint32 clock_offset;

    /*! CLK for next CSB instant. */
    uint32 next_csb_clock;

    /*! AFH Map from the broadcaster. */
    AfhMap afh_map;

    /*! CSB Receive timeout. */
    uint16 supervision_timeout;

    /*! Timing accuracy of the master in ppm. Typical values are 20ppm and
        250ppm. */
    uint8 remote_timing_accuracy;

    /*! Number of consecutive CSB instants the receiver may skip
        after a CSB packet is successfully received. */
    uint16 skip;

    /*! Bitmask of permitted packet types. */
    uint16 packet_type;
} receiver_csb_params;

/*! Parameters for configuring reception of broadcast sync train.
 */
typedef struct
{
    /*! Bluetooth address of the broadcaster */
    bdaddr bd_addr;

    /*! Duration in slots to search for the sync train.
        Configure a value of 0x0000 to not timeout and keep scanning
        for a sync train.
        Range: 0x0000 or 0x0022-0xFFFE; only even values are valid. */
    uint16 sync_scan_timeout;

    /*! Duration in slots to listen for a sync train packet on a single
        frequency.
        Range: 0x0022-0xFFFE; only even values are valid. */
    uint16 sync_scan_window;

    /*! Duration in slots between the start of consecutive scan windows.
        Range: 0x0002-0xFFFE; only even values are valid. */
    uint16 sync_scan_interval;
} receiver_sync_params;

/*! @brief Types of security key.
 */
typedef enum
{
    /*! Private key - Long term key. */
    broadcast_seckey_type_private,
    /*! Party Mode Security Key - not yet supported. */
    broadcast_seckey_type_temporary
} broadcast_seckey_type;

/*! @brief Definition of data required for association.
 */
typedef struct
{
    /*! Bluetooth address of the Broadcaster. */
    bdaddr broadcast_addr;

    /*! Length of the device info data.
        If 0, no data in device_info field. */
    uint16 device_status_len;

    /*! Length of the broadcast stream service records data 
        If 0, no data in stream_service_records field. */
    uint16 stream_service_records_len;

    /*! Length of the data in seckey.
        If 0, no data in seckey field. */
    uint16 seckey_len;

    /*! Security key data. */
    uint8 seckey[BROADCAST_KEY_MAX_LEN];

    /*! Device info data. */
    uint8 device_status[BROADCAST_DEVICE_STATUS_MAX_LEN];

    /*! Broadcast Stream Service Records (BSSR) data. */
    uint8* stream_service_records;
    
    /*! Vendor specific indentifer information. */
    uint16 broadcast_identifier[BROADCAST_IDENTIFIER_LEN];

    /*! Broadcast service version. */
    uint16 broadcast_service_version;
} broadcast_assoc_data;

/****************************************************************************
 * Message Interface
 ****************************************************************************/

/*! Messages sent by the broadcast library to the task registered with
    #BroadcastInit(), typically a VM application. 

    Messages can be classed in 4 categories:-
        - Common
        - Broadcaster
        - Receiver
        - Association
*/
typedef enum
{
    /*! Library initialisation confirmation message.
        Defined in #BROADCAST_INIT_CFM_T. */
    BROADCAST_INIT_CFM = BROADCAST_MESSAGE_BASE,

    /*! Library destruction confirmation message.
        Defined in #BROADCAST_DESTROY_CFM_T. */
    BROADCAST_DESTROY_CFM,

    /*! Broadcaster or receiver configuration confirmation message.
        Defined in #BROADCAST_CONFIGURE_CFM_T. */
    BROADCAST_CONFIGURE_CFM,
    /*! Asynchronous status indication message.
        Defined in #BROADCAST_STATUS_IND_T. */
    BROADCAST_STATUS_IND,

    /*! Broadcaster start operation confirmation message.
        Defined in #BROADCAST_START_BROADCAST_CFM_T. */
    BROADCAST_START_BROADCAST_CFM,
    /*! Broadcaster stop operation confirmation message.
        Defined in #BROADCAST_STOP_BROADCAST_CFM_T. */
    BROADCAST_STOP_BROADCAST_CFM,
    /*! Broadcast stop indication message.
        Defined in #BROADCAST_STOP_BROADCAST_IND_T. */
    BROADCAST_STOP_BROADCAST_IND,

    /*! Receiver start operation confirmation message.
        Defined in #BROADCAST_START_RECEIVER_CFM_T. */
    BROADCAST_START_RECEIVER_CFM,
    /*! Receiver stop operation confirmation message.
        Defined in #BROADCAST_STOP_RECEIVER_CFM_T. */
    BROADCAST_STOP_RECEIVER_CFM,
    /*! Receiver stop indication message.
        Defined in #BROADCAST_STOP_RECEIVER_IND_T. */
    BROADCAST_STOP_RECEIVER_IND,

    /*! Indication that a new CSB AFH map is available.
        Defined in #BROADCAST_AFH_UPDATE_IND_T. */
    BROADCAST_AFH_UPDATE_IND,
    /*! Indication that a new CSB AFH map has now been applied.
        Defined in #BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND_T. */
    BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND,

    BROADCAST_MESSAGE_TOP
} BroadcastMessageId;

/*! Message sent in response to #BroadcastInit() to confirm status
    of library initialisation.
 
    Possible BROADCAST_INIT_CFM_T::status values are:-
        - #broadcast_success
        - #broadcast_init_failed
        - #broadcast_bad_role
 */
typedef struct
{
    /*! Pointer to the instance of the broadcast library
        created by the initialisation operation. */
    BROADCAST *broadcast;

    /*! Status of the library initialisation. */
    broadcast_status status;
} BROADCAST_INIT_CFM_T;

/*! Message sent in response to #BroadcastDestroy() to confirm status
    of library destruction.
 
    Possible BROADCAST_DESTROY_CFM_T::status values are:-
        - #broadcast_success
 */
typedef struct
{
    /*! Status of the library initialisation. */
    broadcast_status status;
} BROADCAST_DESTROY_CFM_T;

/*! Asynchronous status messages from the Broadcast library.

    Future extensions - SCM/BMP messaging indications.

    Possible BROADCAST_STATUS_IND_T::status values are
        - #broadcast_sync_train_tx_ended
 */
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! New status of the broadcast library. */
    broadcast_status status;
} BROADCAST_STATUS_IND_T;

/*! Message sent in response to #BroadcastConfigureBroadcaster()
    or #BroadcastConfigureReceiver() to confirm status of library 
    configuration operation.
 
    Possible BROADCAST_CONFIGURE_CFM_T::status values are
        - #broadcast_success
        - #broadcast_configure_failed_active
        - #broadcast_bad_param
        - #broadcast_invalid_lt_addr
        - #broadcast_bad_role
*/
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! Status of the library configuration operation. */
    broadcast_status status;
} BROADCAST_CONFIGURE_CFM_T;

/*! Message sent in response to #BroadcastStartBroadcast() to confirm
    result of the requested start operation.
 
    Possible BROADCAST_START_BROADCAST_CFM_T::status values are
        - #broadcast_success
        - #broadcast_data_broadcast_start_failed
        - #broadcast_sync_train_start_failed
        - #broadcast_bad_role
 */
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! Status of the requested start operation. */
    broadcast_status status;
    
    /*! Mode in which the start request in #BroadcastStartBroadcast() was
        made */
    broadcast_mode mode;

    /*! Stream sink into which data may be passed for broadcasting.
        Valid if status is #broadcast_success. */
    Sink sink;
} BROADCAST_START_BROADCAST_CFM_T;

/*! Message sent in response to #BroadcastStopBroadcast() to confirm result
    of the requested stop operation.
 
    Possible BROADCAST_STOP_BROADCAST_CFM_T::status values are
        - #broadcast_success
        - #broadcast_bad_role
 */
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! Status of the requested stop operation. */
    broadcast_status status;
    
    /*! Mode in which the start request in #BroadcastStopBroadcast() was
        made */
    broadcast_mode mode;
} BROADCAST_STOP_BROADCAST_CFM_T;

/*! Message sent to provide an indication that the broadcaster has stopped
    transmitting roadcast data. This message is triggered if no CSB packets
    are able to be transmitted for BroadcastStartBroadcast supervision_timeout
    slots.

    The broadcast library instance will transition to a state in which
    no CSB is transmitted and the sync train is no longer running.

    The application must respond to this message by calling
    #BroadcastBroadcasterTimeoutResponse(). 
    
    After calling the response function:- 
        - the CSB stream sink is no longer valid.
        - the application may attempt to restart broadcasting.
        
    Previous broadcaster configuration is retained and #BroadcastConfigureBroadcaster()
    does not need to be called.
 */
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;
} BROADCAST_STOP_BROADCAST_IND_T;

/*! Message sent in response to #BroadcastStartReceiver() to confirm
    the result of the requested start operation.
    
    Possible BROADCAST_START_RECEIVER_CFM_T::status values are
        - #broadcast_success:
            The receiver was started in #broadcast_mode_auto mode and succeeded.
            Broadcast reception is now active and the Source field of the
            message is valid.
        - #broadcast_sync_train_found:
            The receiver was started in #broadcast_mode_synctrain_only and
            succeeded. A sync train was discovered, the synctrain field of the
            message is valid, the Source field of the message is not valid.
        - #broadcast_sync_train_receive_failed:
            receiver_sync_params::sync_scan_timeout was reached before the sync
            train was found. Source and csb parameters fields of the message are
            not valid.
        - #broadcast_data_broadcast_receive_failed:
            The receiver was started in #broadcast_mode_auto mode and found a
            sync train, but failed to subsequently receive the data broadcast.
            The stream source field is not valid, but the csb_params is valid.
        - #broadcast_bad_role
*/
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! Status of the requested start operation. */
    broadcast_status status;

    /*! Mode in which the start request in #BroadcastStartReceiver() was
        made */
    broadcast_mode mode;

    /*! Stream source from which broadcast data may be received.
        Valid if status is #broadcast_success. */
    Source source;

    /*! Details of the broadcast stream, discovered in the sync train.
        Valid if BROADCAST_START_RECEIVER_CFM_T::status has the value
        #broadcast_success or #broadcast_sync_train_found. */
    receiver_csb_params csb_params;
} BROADCAST_START_RECEIVER_CFM_T;

/*! Message sent in response to #BroadcastStopReceiver() to confirm the result
    of the requested stop operation. 

    Possible BROADCAST_STOP_RECEIVER_IND_T::status values are:-
        - #broadcast_success
        - #broadcast_bad_role
*/
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! Status of the requested start operation. */
    broadcast_status status;
} BROADCAST_STOP_RECEIVER_CFM_T;

/*! Message sent to provide an indication that the receiver has stopped
    receiving broadcast data. This message is triggered if no CSB packets
    are received for BroadcastStartReceiver supervision_timeout slots. 

    The broadcast library instance will transition to a state in which
    no CSB data is received.

    The application must respond to this message by calling
    #BroadcastReceiverTimeoutResponse(). 
    
    After calling the response function:-
        - the CSB stream source is no longer valid.
        - the application may attempt to restart receiving.
    
    Previous receiver configuration is retained and #BroadcastConfigureReceiver()
    does not need to be called.
    
*/
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;
} BROADCAST_STOP_RECEIVER_IND_T;

/*! Message sent to the application to indicate that a new CSB AFH channel
    map is available.
*/
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! The new channel map. */
    AfhMap channel_map;

    /*! The clock instant when the new map was calculated. */
    uint32 clock;
} BROADCAST_AFH_UPDATE_IND_T;

/*! Message sent to the application to indicate that a new CSB AFH channel
    map has now been applied.
*/
typedef struct
{
    /*! Pointer to the instance of the broadcast library. */
    BROADCAST *broadcast;

    /*! The new channel map now in use. */
    AfhMap channel_map;
} BROADCAST_AFH_CHANNEL_MAP_CHANGED_IND_T;

/****************************************************************************
 * Functions
 ****************************************************************************/
/*!
    @brief Create an instance of the Broadcast library.

    Create an instance of the Broadcast library. The library instance will
    be for a specific role, either broadcaster or receiver.

    The task specified in the app_task parameter will receive a 
    #BROADCAST_INIT_CFM message indicating the result of the library instance 
    initialisation.
    
    If successful, the message will contain a reference to the broadcast library
    intance, which must be passed in further broadcast library API calls.

    @param app_task [IN] Application task to which messages are sent.
    @param role [IN] Broadcast role of the library.
*/
void BroadcastInit(Task app_task, broadcast_role role);


/*!
    @brief Destroy a broadcast library instance.

    Valid Roles : BROADCASTER and RECEIVER

    Delete the broadcast library instance, and the resources associated with it.
    This call will also delete the LT_ADDR reserved with BT subsystem. Any
    stream sources and sinks previously returned by the library will no longer
    be valid.

    The application task associated with the #BROADCAST library instance will
    receive a #BROADCAST_DESTROY_CFM message confirming completion of the
    operation.

    @param broadcast [IN] Pointer to an instance of the broadcast library.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastDestroy(BROADCAST* broadcast);

/*!
    @brief Configure the Broadcast library for a broadcast role.

    Valid Roles : BROADCASTER.

    Provide the broadcast (CSB) and sync train parameters to use for a
    broadcast on a specific #ltaddr.

    The first time this function is called it will register the lt_addr for
    broadcast use. If multiple ACLs are active the lt_addr may already be in
    use, and #broadcast_invalid_lt_addr status will be returned in the
    #BROADCAST_CONFIGURE_CFM message. The application may try alternate LT_ADDR
    values in the range 0..7, a future library extension may be for the broadcast
    library to manage this and automatically find a free LT_ADDR for the
    broadcast.

    This function can provide both the csb parameters and sync train parameters
    to completely configure a broadcast in a single call. The broadcast may
    then be started using #BroadcastStartBroadcast() with the sync train
    automatically started.

    The caller may reconfigure the parameters for the data broadcast or the
    sync train with a subsequent call. Either csb_params, sync_params or
    both may be specified.
    
    Sync train parameters may be reconfigured at any time, broadcast parameters
    may only be modified when the broadcast it not active.

    The application task associated with the #BROADCAST library instance will
    receive a #BROADCAST_CONFIGURE_CFM message with the status of the
    configuration operation.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param lt_addr [IN] Broadcast address.
    @param csb_params [IN] Parameters for the data broadcast on lt_addr.
    @param sync_params [IN] Parameters for the sync train about the data broadcast on lt_addr.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastConfigureBroadcaster(BROADCAST* broadcast,
                                   ltaddr lt_addr,
                                   broadcaster_csb_params* csb_params,
                                   broadcaster_sync_params* sync_params);

/*!
    @brief Start a broadcast.

    Valid Roles : BROADCASTER.

    This function can be used to start a broadcast, the broadcast must have
    already been configured using #BroadcastConfigureBroadcaster().

    A broadcast can be started in several different modes, auto, broadcast only
    and sync train only.
    
    In auto mode, the data broadcast will be started and the sync train
    automatically started too. In broadcast only mode, the data broadcast will
    be started, but not the sync train. The sync train may be started
    independently using sync train only mode.
    
    Note that an attempt to start a sync train when the broadcast is not
    already running will fail.

    The application task associated with the #BROADCAST library instance will
    receive a #BROADCAST_START_BROADCAST_CFM message with the status of the
    requested operation. If the start operation is successful the message will
    contain a stream Sink handle which the application can pass data into
    for broadcast.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param start_mode [IN] Mode in which to start a broadcast.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastStartBroadcast(BROADCAST* broadcast, broadcast_mode start_mode);

/*!
    @brief Stop a broadcast.

    Valid Roles : BROADCASTER.

    This function can be used to stop a broadcast. In auto mode, both
    the data broadcast and the sync train will be stopped. In sync train
    only mode, the sync train will be stopped, but the data broadcast will
    remain running.

    Note that stopping in broadcast only mode, will automatically stop
    the sync train, as it is not permitted to run the sync train when
    not also broadcasting.

    The application task associated with the #BROADCAST library instance will
    receive a #BROADCAST_STOP_BROADCAST_CFM message with the status of the
    requested operation.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param stop_mode [IN] Mode in which to stop a broadcast.
*/
broadcast_status BroadcastStopBroadcast(BROADCAST* broadcast, broadcast_mode stop_mode);

/*!
    @brief Respond to broadcaster transmission timeout.

    Valid Roles : BROADCASTER.

    Must be called by the application after receipt of a #BROADCAST_STOP_BROADCAST_IND
    message. After calling this function the application may attempt to restart
    the broadcaster.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
*/
broadcast_status BroadcastBroadcasterTimeoutResponse(BROADCAST* broadcast);

/*!
    @brief Configure a broadcast receiver.

    Valid Roles : RECEIVER.

    This function is used to configure reception of a data broadcast.
    An application will configure the sync_params for discovery of the sync
    train, within which details of the broadcast can be found.

    The application task associated with the #BROADCAST library instance will
    reiceve a #BROADCAST_CONFIGURE_CFM message with the status of the
    requested operation.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param sync_params [IN] Parameters for finding a sync train to discover a broadcast.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastConfigureReceiver(BROADCAST* broadcast,
                                receiver_sync_params* sync_params);

/*!
    @brief Configure a broadcast receiver out-of-band.

    Valid Roles : RECEIVER.

    An application may use this receiver configuration function to start
    receiving a broadcast if it has already acquired the broadcast
    configuration parameters out-of-band, i.e. not by discovery from the
    sync train.

    Alternatively, this function may be used to modify the receiver CSB
    parameters to suit application requirments, for instance a low power
    limbo mode.

    The application task associated with the #BROADCAST library instance will
    reiceve a #BROADCAST_CONFIGURE_CFM message with the status of the
    requested operation.

    Possible status values returned in #BROADCAST_CONFIGURE_CFM are:-
        - #broadcast_success
        - #broadcast_bad_param
        - #broadcast_bad_role

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param bd_addr [IN] Bluetooth address of the Broadcasting device.
    @param lt_addr [IN] Address on which data is being broadcast.
    @param csb_params [IN] Parameters for the data broadcast on lt_addr.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastConfigureReceiverOOB(BROADCAST* broadcast,
                                   bdaddr bd_addr, ltaddr lt_addr,
                                   receiver_csb_params* csb_params);

/*!
    @brief Start a broadcast receiver.

    Valid Roles : RECEIVER.

    This function is used to begin receiving a broadcast data stream. Reception
    may be started in two different modes.

    In auto mode (#broadcast_mode_auto), the broadcast library will attempt to
    discover the sync train using the parameters provided in
    #BroadcastConfigureReceiver(), if successful, reception of the broadcast
    will be automatically started.

    In sync train only mode (#broadcast_mode_synctrain_only), only discovery
    of the sync train will be started, and if successful then details of the
    discovered broadcast will be returned to the application.

    Broadcast only mode (#broadcast_mode_broadcast_only) is not a valid mode
    in the receiver role.

    The application task associated with the #BROADCAST library instance will
    receive a #BROADCAST_START_RECEIVER_CFM message with the status of the
    requested operation. In auto mode and if the start operation is successful
    in receiving a broadcast, the message will contain details of the discovered
    broadcast and a stream Source handle from which the application may receive
    the broadcast data stream, or connect it to further stream objects such as 
    DSP ports. In sync train only mode, if successful the message will contain
    details of the discovered broadcast only.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param start_mode [IN] Mode in which to start the receiver.
    @param supervision_timeout [IN] Duration in slots to continue listening for
                        CSB packets after the last successfully received CSB packet.
                        Range: 0x0002-0xFFFE; only even values are valid.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastStartReceiver(BROADCAST* broadcast, broadcast_mode start_mode,
                            uint16 supervision_timeout);

/*!
    @brief Stop a broadcast receiver.

    Valid Roles : RECEIVER.

    This function is used when the application wishes to end broadcast
    receive operations. If a broadcast is currently being received, it will
    be stopped, and the stream Source will no longer be valid following this
    call.

    If the receiver was searching for a sync train, this will be stopped.

    The application task associated with the #BROADCAST library instance will
    receive a #BROADCAST_STOP_RECEIVER_CFM message with the status of the
    requested operation.

    @param broadcast [IN] Pointer to an instance of the broadcast library.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastStopReceiver(BROADCAST* broadcast);

/*!
    @brief Respond to receiver timeout.

    Valid Roles : RECEIVER.

    Must be called by the application after receipt of a #BROADCAST_STOP_RECEIVER_IND
    message. After calling this function the application may attempt to restart
    the receiver.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
*/
broadcast_status BroadcastReceiverTimeoutResponse(BROADCAST* broadcast);

/*!
    @brief Determine if a broadcast is currently active.

    Valid Roles : BROADCASTER and RECEIVER

    @param broadcast [IN] Pointer to an instance of the broadcast library.

    @return broadcast_status #broadcast_is_active or #broadcast_not_active.
*/
broadcast_status BroadcastIsBroadcasting(BROADCAST* broadcast);

/*!
    @brief Determine if a sync train is currently active.

    Valid Roles : BROADCASTER.

    @param broadcast [IN] Pointer to an instance of the broadcast library.

    @return broadcast_status #broadcast_sync_train_is_active or #broadcast_sync_train_not_active.
*/
broadcast_status BroadcastIsSyncTrainActive(BROADCAST* broadcast);

/*!
    @brief Get the current configuration of the library in the broadcast role.

    Valid Roles : BROADCASTER.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param csb_params [OUT] Pointer to a structure into which the library will
                            copy the current #broadcaster_csb_params.
    @param sync_params [OUT] Pointer to a structure into which the library will
                             copy the current #broadcaster_sync_params.
    
    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastBroadcasterGetConfiguration(BROADCAST* broadcast,
                                          broadcaster_csb_params* csb_params,
                                          broadcaster_sync_params* sync_params);

/*!
    @brief Get the current configuration of the library in the receiver role.

    Valid Roles : RECEIVER.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param csb_params [OUT] Pointer to a structure into which the library will
                            copy the current #receiver_csb_params.
    @param sync_params [OUT] Pointer to a structure into which the library will
                             copy the current #receiver_sync_params.
    
    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastReceiverGetConfiguration(BROADCAST* broadcast,
                                       receiver_csb_params* csb_params,
                                       receiver_sync_params* sync_params);

/*!
    @brief Get the CSB interval of the library in the receiver role.

    Valid Roles : RECEIVER.

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param interval [OUT] Pointer to a variable into which the library will
                            copy the current CSB interval.

    @return broadcast_status Result of requested operation.
*/
broadcast_status BroadcastReceiverGetCSBInterval(BROADCAST *broadcast,
                                                 uint16 *interval);

/*! @brief Handle the association data received from an Application

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @param ind pointer to broadcast_assoc_data structure.
    @return broadcast_status 
 */ 
broadcast_status BroadcastSetAssocInfo(BROADCAST* broadcast, broadcast_assoc_data* assoc_data);

/*! @brief Returns the association data of the passed bradcast instance to an Application

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @return pointer to broadcast_assoc_data structure 
 */
broadcast_assoc_data* BroadcastGetAssocInfo(BROADCAST* broadcast);

/*! @brief Returns the LT_ADDR as configured

    @param broadcast [IN] Pointer to an instance of the broadcast library.
    @return LT_ADDR value which was configured 
 */
uint8 BroadcastGetLtAddr(BROADCAST* broadcast);

#endif

