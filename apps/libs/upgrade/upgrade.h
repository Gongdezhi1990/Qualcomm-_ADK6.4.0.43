/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    upgrade.h
    
DESCRIPTION
    Header file for the Upgrade library.
*/
/*!
@file   upgrade.h
@brief  Header file for the Upgrade library

        Interface to the Upgrade library which provides support for upgrading
        CSR device software, using the VM Upgrade mechanism.

        The library exposes a functional downstream API and an upstream message
        based API.

        See @ref otau_library

@page   otau_library Upgrade library integration

This page provides a summary of how the upgrade library upgrade.h
integrates with applications.

@section sec_basic Basics

To make use of the upgrade library, all of the following are needed.

Examples of all of these can be found in the example Sink application
supplied with the ADK. See @ref sec_sinkapp for a summary of the upgrade 
library integration.
        @li @ref subsec_defparset
        @li @ref subsec_tranmech
        @li @ref subsec_libinit
        @li @ref subsec_libint
        @li @ref subsec_permit
        @li @ref subsec_host 

@subsection subsec_defparset Partition set

The library uses the concept of logical partitions to define what is being
upgraded. Apart from a partition used for DFU operations these must be 
paired, such that there is an active partition and a partition available for
upgrade. 

The unused partitions will be kept erased.

@subsection subsec_tranmech Transport mechanism
@subsection subsec_libinit Library initialisation
@subsection subsec_libint Library integration
@subsection subsec_permit Permitting activities
@subsection subsec_host A host applications

@section sec_sinkapp Supplied Sink application

The Sink application supplied as an example makes use of the upgrade 
library through the gaia library (gaia.h).
        
        
*/
#ifndef UPGRADE_H_
#define UPGRADE_H_

#include <library.h>
#include <message.h>

#ifndef UPGRADE_HOST_IF_MSG_BASE
#define UPGRADE_HOST_IF_MSG_BASE 0x500
#endif

#ifdef ENABLE_BATTERY_OPERATION
#define UPGRADE_INIT_POWER_MANAGEMENT   upgrade_battery_powered
#else
#define UPGRADE_INIT_POWER_MANAGEMENT   upgrade_power_management_disabled
#endif

#define UPGRADE_DEFAULT_DEV_VARIANT     "ZARKOV77"

#define UPGRADE_MAX_PARTITION_DATA_BLOCK_SIZE 48

/*!
    @brief Enumeration of status types used by Upgrade library API.
*/
typedef enum
{
    /*! Operation succeeded. */
    upgrade_status_success = 0,
    /*! Operation failed */
    upgrade_status_unexpected_error,
    /*! already connected */
    upgrade_status_already_connected_warning,
    /*! Requested operation failed, an upgrade is in progress */
    upgrade_status_in_progress,
    /*! UNUSED */
    upgrade_status_busy,
    /*! Invalid power management state */
    upgrade_status_invalid_power_state
} upgrade_status_t;

/*!
    @brief Enumeration of message IDs used by Upgrade library when sending 
           messages to the application.
*/
typedef enum
{
    /*! Message sent in response to UpgradeInit(). */
    UPGRADE_INIT_CFM = UPGRADE_UPSTREAM_MESSAGE_BASE,
    /*! Message sent during initialisation of the upgrade library
        to let the VM application know that a restart has occurred 
        and reconnection to a host may be required. */
    UPGRADE_RESTARTED_IND,
    /*! Message sent to application to request applying a downloaded upgrade.
        Note this may include a warm reboot of the device.
        Application must respond with UpgradeApplyResponse() */
    UPGRADE_APPLY_IND,
    /*! Message sent to application to request blocking the system for an extended
        period of time to erase serial flash partitions.
        Application must respond with UpgradeBlockingResponse() */
    UPGRADE_BLOCKING_IND,
    /*! Message sent to application to indicate that blocking operation is finished */
    UPGRADE_BLOCKING_IS_DONE_IND,
    /*! Message sent to application to inform of the current status of an upgrade. */
    UPGRADE_STATUS_IND,
    /*! Message sent to application to request any audio to get shut */
    UPGRADE_SHUT_AUDIO,
    /*! Message sent to application set the audio busy flag and copy the audio
        image to the audio SQIF or Swap to App image in case of audio ROM */
    UPRGADE_COPY_AUDIO_IMAGE_OR_SWAP,
    /*! Message sent to application to reset the audio busy flag should the audio
        image copy fails ensuring a smooth reset to normal */
    UPGRADE_AUDIO_COPY_FAILURE,
    /*! Message sent to application to inform that the actual upgrade has started */
    UPGRADE_START_DATA_IND,
    /*! Message sent to application to inform that the actual upgrade has ended */
    UPGRADE_END_DATA_IND,
    /*! ID for first message outside of this range */
    UPGRADE_UPSTREAM_MESSAGE_TOP
} upgrade_application_message;


/*!
    @brief Enumeration of message IDs used by the Upgrade library when sending
           messages to the transport task.
*/
typedef enum
{
    /*! Message sent in reponse to UpgradeTransportConnectRequest(). */
    UPGRADE_TRANSPORT_CONNECT_CFM = UPGRADE_DOWNSTREAM_MESSAGE_BASE,
    /*! Message sent in response to UpgradeTransportDisconnectRequest(). */
    UPGRADE_TRANSPORT_DISCONNECT_CFM,
    /*! Message sent to a transport to send a data packet to a client. */
    UPGRADE_TRANSPORT_DATA_IND,
    /*! Message sent in response to UpgradeProcessDataRequest(). */
    UPGRADE_TRANSPORT_DATA_CFM,


    /*! ID for first message outside of this range */
    UPGRADE_DOWNSTREAM_MESSAGE_TOP
} upgrade_transport_message;


/*!
    @brief Types of permission that are given to the upgrade library. 

    A VM application must give permission for the VM upgrade library to both
    perform upgrade operations in general, and also for specific operations
    which have the potential to effect system behaviour, such as reboots or
    pauses due to serial flash erasing.
 */
typedef enum
{
    /*! The upgrade library is not permitted to perform upgrade.
        Note that the upgrade library may reject this if an upgrade is already
        in progress. */
    upgrade_perm_no,

    /*! Configures the upgrade library to run in an autonomous mode. The library
        will assume the answer to any decisions point is yes and will automatically
        act as such, for instance reboots and flash erase operations. */
    upgrade_perm_assume_yes,

    /*! Configures the upgrade library to always ask the VM application when
        decision points are reached. This option requires the VM application to
        handle additional messages from the upgrade library and responding
        appropriately or upgrade may not complete successfully. */
    upgrade_perm_always_ask
} upgrade_permission_t;

/*!
    @brief States which the upgrade library may be in.
*/
typedef enum
{
    /*! The upgrade library is not currently downloading or commiting an upgrade. */
    upgrade_state_idle,

    /*! The upgrade library is in the process of downloading an upgrade. */
    upgrade_state_downloading,

    /*! The upgrade library is in the process of commiting an upgrade. */
    upgrade_state_commiting,

    /*! The upgrade library has completed an upgrade. */
    upgrade_state_done
} upgrade_state_t;


/*!
    @brief Flags to indicate Upgrade End state while ending the Upgrade download
*/
typedef enum
{
    /*! The upgrade library is not currently in Abort or Complete of upgrade. */
    upgrade_end_state_none,

    /*! The upgrade library is in the process of aborting an upgrade. */
    upgrade_end_state_abort,

    /*! The upgrade library is in the process of Upgrade download complete. */
    upgrade_end_state_complete

} upgrade_end_state_t;


/*!
    @brief Enumeration power management initialization.

    There are the possible states of power management.
*/
typedef enum 
{
    /*! There is no power management in place (e.g. device is not battery powered) */
    upgrade_power_management_disabled,

    /*! This is the initialized mode when battery powered  */
    upgrade_battery_powered
} upgrade_power_management_t;

/*!
    @brief Enumeration power management state.

    There are the possible states of power management.
*/
typedef enum
{
    /*! Battery is above the low level threshold and upgrade is allowed. 
        This state is set also when charger is removed and battery is ok */
    upgrade_battery_ok,

    /*! Battery is below the low level threshold and upgrade is no longer allowed.
        This state is set also when charger is removed and battery still hasn't got enough charge */ 
    upgrade_battery_low,

    /*! Indicate that battery is charging */
    upgrade_charger_connected
    
} upgrade_power_state_t;

/*!
    @brief Reasons for reconnection.

    Gives the reason why the library is asking the application to trigger 
    a host reconnection.
*/
typedef enum
{
    /*! reconnection not needed, no upgrade is in progress. */
    
    upgrade_reconnect_not_required,

    /*! High priority request for reconnection.

    The upgrade is not yet confirmed and host action is required to accept or
    reject the upgrade */
    
    upgrade_reconnect_required_for_confirm,

    /*! Low priority request for reconnection.

    An upgrade has completed and reconnecting to the host will allow the host
    to report the success. 

    The upgrade cannot now be rolled back so reconnection is not mandatory. */

    upgrade_reconnect_recommended_as_completed,

    /*! Optional request for reconnection.

    An upgrade was in progress, but had not reached a critical state. The 
    application is best placed to decide if reconnecting to a host is a priority.

    For example, a sound bar - with an external power source, might choose
    to enable connections; a headset on battery power might not - it was probably
    reset for a reason */

    upgrade_reconnect_recommended_in_progress

} upgrade_reconnect_recommendation_t;

/*!
    @brief A {major, minor} pair defines an upgrade version.

    The meaning of major and minor is left to the VM app,
    but the values are used in the upgrade file header
    to check if an upgrade file is compatible with the
    currently running VM app.

    The version refers to the current configuration as sent in
    an  upgrade file. This could combine an application version along
    with any other partitions programmed in an external flash device, 
    such as audio prompts.
*/
typedef struct
{
    /*! Nominally the major version of the current configuration programmed
        on the device. 
        Interpretation is left entirely to the application.*/
    uint16 major;
    /*! Nominally the minor version of the current configuration programmed
        on the device. 
        Interpretation is left to the application.

        The value of 0xFFFF is reserved.*/
    uint16 minor;
} upgrade_version;

/*!
    @brief Definition of the message sent in response to UpgradeTransportConnectRequest().
*/
typedef struct 
{
    /*! Status of the transport connection request. */
    upgrade_status_t     status;
} UPGRADE_TRANSPORT_CONNECT_CFM_T;

/*!
    @brief Definition of the message sent in response to UpgradeProcessDataRequest().
*/
typedef struct
{
    /*! Status of the data request. */
    upgrade_status_t     status;
    uint8                packet_type;
} UPGRADE_TRANSPORT_DATA_CFM_T;

/*!
    @brief Definition of the message sent to a transport to send a data packet to a client.
*/
typedef struct
{
    /*! Determines if in UPGRADE_PARTITION_DATA_STATE_DATA */
    bool                 is_data_state;
    /*! Size of data packet to send. */
    uint16               size_data;
    /*! Pointer to data packet to send. */
    uint8                data[1];
} UPGRADE_TRANSPORT_DATA_IND_T;

/*!
    @brief Definition of the message sent in response to #UpgradeTransportDisconnectRequest().
*/
typedef struct
{
    /*! Status of the transport disconnection request. */
    upgrade_status_t     status;
} UPGRADE_TRANSPORT_DISCONNECT_CFM_T;

/*!
    @brief Definition of the message sent in response to #UpgradeInit().
*/
typedef struct
{
    /*! Status of the library initialisation request. */
    upgrade_status_t     status;
} UPGRADE_INIT_CFM_T;


/*!
    @brief Definition of message that may be sent as a result of #UpgradeInit() being called.

    The message indicates that the upgrade library is not currently connected to
    a host and that a host connection may be needed. It allows the application
    to trigger a reconnection state if appropriate.

    The message may be sent at other times in future. 
*/
typedef struct
{
    /*! Strength of recommendation for reconnecting. */
    upgrade_reconnect_recommendation_t  reason;
} UPGRADE_RESTARTED_IND_T;


/*!
    @brief Message indicating change in state of the upgrade library.
*/
typedef struct
{
    /*! Current library state. */
    upgrade_state_t state;
} UPGRADE_STATUS_IND_T;

/*!
    @brief Message indicating end of Upgrade download state.
*/
typedef struct
{
    /*! End of upgrade download state. */
    upgrade_end_state_t state;
} UPGRADE_END_DATA_IND_T;


/*!
    @brief Arrangement of physical flash partitions for a logical flash partition
        that is managed by this library.
*/
typedef enum
{
    /*! Indicates a partition, that is not part of a pair, and that will always be 
        erased after an upgrade completes. */
    UPGRADE_LOGICAL_BANKING_SINGLE_KEEP_ERASED,
    /*! Partition used for DFU (synonym for #UPGRADE_LOGICAL_BANKING_SINGLE_KEEP_ERASED) */
    UPGRADE_LOGICAL_BANKING_SINGLE_DFU = UPGRADE_LOGICAL_BANKING_SINGLE_KEEP_ERASED,
    /*! A partition pair that is not used for a file system. These partitions will
        be of type raw serial */
    UPGRADE_LOGICAL_BANKING_DOUBLE_UNMOUNTED,
    /*! A partition pair that is used in a file system. These partitions will
        be of type read only (although they can be upgraded). Following an upgrade 
        of these partitions the file system table will be updated so that 
        the newly updated partition is active. */
    UPGRADE_LOGICAL_BANKING_DOUBLE_MOUNTED,
    /*! A partition, that is not part of a pair, and is not kept erased by the 
        upgrade library. 

        @note This type of partition is not supported at present. */
    UPGRADE_LOGICAL_BANKING_SINGLE_ERASE_TO_UPDATE
} UPGRADE_LOGICAL_PARTITION_ARRANGEMENT;


/*!
    @brief Element defining the physical partitions in flash that apply to
        this logical partition.

    Each logical partition should be defined using the macro 
    #UPGRADE_PARTITION_SINGLE or #UPGRADE_PARTITION_DOUBLE.

    The values for bank1 and bank2 should be the same as would be present in
    an entry for @ref PSKEY_FSTAB. The order of bank1 and bank2 may be 
    important when the application is first run. If no information can be 
    derived from the flash device, the library will assume that the physical 
    partition referenced by bank1 is in use; and that the physical partition
    referenced by bank2 can be erased. */
typedef struct
{
    /*! The first bank of logical partition group. If the bank is part of a 
        pair (see #banking) then the upgrade library will assume this partition
        is in used when a device is started for the first time. */
    unsigned                    bank1:14;
    /*! unused at present */
    unsigned                    spare:2;
    /*! The second bank of logical partition group. This is only used for
        paired partitions, but CSR recommend setting the value to match #bank1 for
        non-paired partitions.

        If the bank is part of a 
        pair (see #banking) then the upgrade library will assume this partition
        is in used when a device is started for the first time. */
    unsigned                    bank2:14;
    /*! The type for this logical partition. 

    @note #banking is actually of type #UPGRADE_LOGICAL_PARTITION_ARRANGEMENT, 
    but is set to unsigned for compatibility reasons.  */
    unsigned                    banking:2;
} UPGRADE_UPGRADABLE_PARTITION_T;

/*! @brief macro to be used when initialising an #UPGRADE_UPGRADABLE_PARTITION_T
        entry. 

        The macro is recommended as the supported bank types may change in future, 
        possibly requiring a layout change in the structure - or more complex 
        initialisation.  */
#define UPGRADE_PARTITION_SINGLE(bank,banktype) \
            { bank, 0, bank, UPGRADE_LOGICAL_BANKING_SINGLE_##banktype }

/*! @brief macro to be used when initialising an #UPGRADE_UPGRADABLE_PARTITION_T
        entry. 

        The macro is recommended as the supported bank types may change in future, 
        possibly requiring a layout change in the structure - or more complex 
        initialisation.  */
#define UPGRADE_PARTITION_DOUBLE(first_bank,second_bank,banktype) \
            { first_bank, 0, second_bank, UPGRADE_LOGICAL_BANKING_DOUBLE_##banktype }


/*!
    @brief Initialise the Upgrade library.

    Must be called before any other Upgrade library API is used.
    Upgrade library will respond with #UPGRADE_INIT_CFM_T message, the library
    is only considered initialised if the status field of the #UPGRADE_INIT_CFM_T
    message is #upgrade_status_success.

    The initialisation informs the upgrade library of the partitions that 
    may be upgraded. Only partitions referenced here can be upgraded.

    If your application uses upgradable raw serial (RS) partitions then you 
    should not make any assumptions about the active partition, instead call
    the function @ref UpgradeGetPartitionInUse to retrieve the currently 
    active partition ID.

    The upgrade library must be initialised with an "enabled" permission of
    either #upgrade_perm_assume_yes or #upgrade_perm_always_ask. If the
    application wishes to disable upgrades it must call #UpgradePermit() with
    the #upgrade_perm_no parameter, after successful initalisation. 
    Note that this may be rejected if an upgrade is already in progress.
    The upgrade library will send an #UPGRADE_STATUS_IND message when an
    in progress upgrade has been completed, after which upgrades may be
    disabled.
    
    @param appTask The application task to receive Upgrade library messages.
    @param dataPskey  Pskey the Upgrade library may use to store data.
    @param dataPskeyStart The word offset within the PSKey from which the upgrade
           may store data.
    @param logicalPartitions Pointer to an array defining the partitions that
           are managed by the upgrade library. The first entry defines
           logical partition 0 etc.
    @param numPartitions Number of logical partitions in the logicalPartitions
           array
    @param power_mode Current power management state
    @param dev_variant Name of this device variant. Checked against the variant
           in the upgrade file header. If NULL, check is ignored.
    @param init_perm Initial state of upgrade library permissions.
    @param init_version Factory-default upgrade version of this app.
           Once an upgrade has been done, the upgrade version from
           the upgrade file header will be used.
    @param init_config_version Factory-default PS data config version of this app.
           Once an upgrade has been done, the PS config version from
           the upgrade file header will be used.
*/
void UpgradeInit(Task appTask,
                 uint16 dataPskey,
                 uint16 dataPskeyStart,
                 const UPGRADE_UPGRADABLE_PARTITION_T *logicalPartitions,
                 uint16 numPartitions,
                 upgrade_power_management_t power_mode,
                 const char *dev_variant,
                 upgrade_permission_t init_perm,
                 const upgrade_version *init_version,
                 uint16 init_config_version);

/*!
    @brief Find out current physical partition for a logical partion

    This function uses the array of logical partitions supplied during 
    AppInit as well as the librarys knowledge of the state of
    partitions to return a reference to the physical partition that
    is active.

    This is most useful for raw serial (RS) partitions as these need to
    be accessed directly by the application. Read only (RO) partitions are
    typically mounted as file systems and the appropriate partition
    will already be active.

    @param logicalPartition Index into the table of 
            UPGRADE_UPGRADABLE_PARTITION_T supplied during @ref UpgradeInit

    @return Definition of physical partition, 0 in case of error. The 
            partition information matches the format used in @ref
            PSKEY_FSTAB
*/
uint16 UpgradeGetPartitionInUse (uint16 logicalPartition);

/*!
    @brief Main message handler for the upgrade library.
    @param task Task of the message sender.
    @param id Message identifier.
    @param message Message contents.
*/
void UpgradeHandleMsg(Task task, MessageId id, Message message);

/*!
    @brief Specify the type of permission which the upgrade has to perform
           upgrade operations.

    The default permission is provided by the VM application in #UpgradeInit(),
    this must be either #upgrade_perm_assume_yes or #upgrade_perm_always_ask.
    The VM application may subsequently use this function to modify the type
    of permission as required.
    
    The upgrade library may reject an attempt by the VM application to disable
    upgrades, if an upgrade is already in progress and the platforms does not 
    support the resume feature. If rejected due to an upgrade already in
    progress, the upgrade library will send a #UPGRADE_STATUS_IND message to
    the VM application with a state of upgrade_state_done.

    There are two types of enabled permission, "assume yes" and "always ask".
    
    The upgrade library can be enabled in an autonomous mode, using the
    #upgrade_perm_assume_yes permission type. During operation, the upgrade
    library will assume the answer at key decision points is yes and continue
    automatically.

    Key decision points are :-
    * Rebooting the device as part of completing an upgrade
    * Blocking the system to erase serial flash partition, may effect audio.

    The VM application can participate in these decisions by configuring
    the upgrade library with #upgrade_perm_always_ask permission type. In
    this mode, the upgrade library will send #UPGRADE_APPLY_IND and
    #UPGRADE_BLOCKING_IND messages to the VM application.
    The VM application must reply with #UpgradeApplyResponse() or 
    #UpgradeBlockingResponse() respectively or the upgrade process will not
    succeed.

    @param perm #upgrade_perm_no Disable upgades.
                #upgrade_perm_assume_yes Enable upgrades, automatic mode.
                #upgrade_perm_always_ask Enable upgrades, interactive mode.

    @return upgrade_status_t #upgrade_status_success Requested permission change
                             was accepted.
                             #upgrade_status_in_progress Requested permission denied.
*/
upgrade_status_t UpgradePermit(upgrade_permission_t perm);

/*!
    @brief Connect a transport to the Upgrade library.

    When a client wants to initiate an upgrade, the transport
    must first connect to the upgrade library so that it knows
    which Task to use to send messages to a client.

    The Upgrade library will respond by sending
    UPGRADE_TRANSPORT_CONNECT_CFM to transportTask.

    @param transportTask The Task used by the transport.
    @param need_data_cfm Whether the Task needs UPGRADE_TRANSPORT_DATA_CFM messages.
    @param request_multiple_blocks Whether the Task can handle multiple block requests.
*/
void UpgradeTransportConnectRequest(Task transportTask, bool need_data_cfm, bool request_multiple_blocks);

/*!
    @brief Pass a data packet from a client to the Upgrade library.

    All data packets from a client should be sent to the Upgrade library
    via this function. Data packets must be in order but do not need
    to contain a whole upgrade message.

    The Upgrade library will respond by sending
    UPGRADE_TRANSPORT_DATA_CFM to the Task set in
    UpgradeTransportConnectRequest().

    @param size_data Size of the data packet
    @param data Pointer to the data packet
*/
void UpgradeProcessDataRequest(uint16 size_data, uint8 *data);

/*!
    @brief Disconnect a transport from the Upgrade library.

    When a transport no longer needs to use the Upgrade
    library it must disconnect.

    The Upgrade library will respond by sending
    UPGRADE_TRANSPORT_DISCONNECT_CFM to the Task set in
    UpgradeTransportConnectRequest().
*/
void UpgradeTransportDisconnectRequest(void);

/*!
    @brief Inform the Upgrade library of the result an attempt to perform DFU from serial flash.

    @param message MESSAGE_DFU_SQIF_STATUS message containing status of DFU operation.
*/
void UpgradeDfuStatus(MessageDFUFromSQifStatus *message);

/*!
    @brief Inform the Upgrade library of the result an attempt erase SQIF.

    @param message #MESSAGE_IMAGE_UPGRADE_ERASE_STATUS message containing status of erase operation.
*/
void UpgradeEraseStatus(Message message);

/*!
    @brief Inform the Upgrade library of the result an attempt copy SQIF.

    @param message #MESSAGE_IMAGE_UPGRADE_COPY_STATUS message containing status of copy operation.
*/
void UpgradeCopyStatus(Message message);

/*!
    @brief Inform the Upgrade library of the result an attempt copy Audio SQIF.

    @param message #MESSAGE_IMAGE_UPGRADE_AUDIO_STATUS message containing status of copy operation.
*/
void UpgradeCopyAudioStatus(Message message);

/*!
    @brief Inform the Upgrade library of the result an attempt to calculate teh hash over all sections.

    @param message #MESSAGE_IMAGE_UPGRADE_HASH_ALL_SECTIONS_UPDATE_STATUS message containing status of the hash operation.
*/
void UpgradeHashAllSectionsUpdateStatus(Message message);

/*!
    @brief Control reboot of device in interactive mode.

    The upgrade library needs to reboot the device to complete an upgrade. If 
    the VM application configured the library with #upgrade_perm_always_ask,
    the application will receive #UPGRADE_APPLY_IND messages when the upgrade
    library needs to reboot.

    If the application configured the library with #upgrade_perm_assume_yes,
    the application will not receive #UPGRADE_APPLY_IND messages and the upgrade
    library will automatically reboot when required.

    The application must use this API to tell the upgrade library when it can
    reboot. If the application wants to reboot immediately, it should pass a
    postpone parameter of 0.

    If the application wants to postpone the action it can specify a delay
    in milliseconds to postpone. The upgrade library will resend the
    #UPGRADE_APPLY_IND message after that delay has elapsed.
    
    @param postpone Time in milliseconds to postpone.
*/
void UpgradeApplyResponse(uint32 postpone);

/*!
    @brief Control audio shutdown in order to copy the audio image.

    In order to copy the audio image from apps1 to the audio SQIF, all audio
    must be shut otherwise, the currator will panic and the copy will fail.
*/
void UpgradeApplyAudioShutDown(void);

/*!
    @brief Control blocking any new audio and makes the call for the audio image
           copy.

    Sets the audio busy flag and calls the function responsible for any trap
    calls to P0 which handle the audio image copy.
*/
void UpgradeCopyAudioImage(void);

/*!
    @brief Control unblocking the audio if the audio image fails to copy.

    In order to ensure that the audio image can be copied, the audio busy flag
    is set. Should the audio copy fail for any reason, this message ensures that
    that the flag is reset so that the system can resume to normal.
*/
void UpgradeApplyAudioCopyFailed(void);

/*!
    @brief Control blocking of the system in interactive mode.

    The upgrade library needs to erase serial flash partitions to complete
    an upgrade, and possibly at other times to clean up an incomplete upgrade.

    Serial flash erase operations are blocking, they block the entire system
    for the duration of the erase, the period of the block is dependent upon 
    the size of the serial flash partition being erased and the characteristics
    of the flash part used. Erase operations can disrupt streaming audio.

    If the VM application configured the library with #upgrade_perm_always_ask,
    the application will receive #UPGRADE_BLOCKING_IND messages when the upgrade
    library needs to block the system for partition erase.

    If the application configured the library with #upgrade_perm_assume_yes,
    the application will not receive #UPGRADE_BLOCKING_IND messages and the 
    upgrade library will automatically erase flash partitions when required.

    The application must use this API to tell the upgrade library when it can
    block the system. If the application wants to block immediately, it should 
    pass a postpone parameter of 0.

    If the application wants to postpone the action it can specify a delay in 
    milliseconds to postpone. The upgrade library will resend the
    #UPGRADE_APPLY_IND message after that delay has elapsed.
    
    @param postpone Time in milliseconds to postpone.
*/
void UpgradeBlockingResponse(uint32 postpone);

/*!
    @brief Receives the current state of the power management from the Sink App

    @param state Current power management state

*/
upgrade_status_t UpgradePowerManagementSetState(upgrade_power_state_t state);


/*!
    @brief Query the upgrade library to see if we are part way through an upgrade.

    This is used by the application during early boot to check if the reason
    for rebooting is because we have reached the 
    UPGRADE_RESUME_POINT_POST_REBOOT point.

    Note: This is only to be called during the early init phase, before UpgradeInit
          has been called.
*/
bool UpgradeRunningNewApplication(uint16 dataPskey, uint16 dataPskeyStart);
/*!
    @brief Inform the Upgrade library of an application partition validation result.
    
    The validation result of a call to ValidationInitiateExecutablePartition
    is sent to the main app task in a MESSAGE_EXE_FS_VALIDATION_STATUS
    message.

    The result must be passed on to the Upgrade library via this function.
    
    @param pass TRUE if the validation passed, FALSE otherwise.
*/
void UpgradeApplicationValidationStatus(bool pass);
/*!
    @brief To check if the upgrade state is in Data transfer mode.
        
    Returns TRUE if the Upgrade state is in Data transfer.
*/
bool UpgradeIsDataTransferMode(void);
/*!
    @brief To fetch the partition size when in UPGRADE_PARTITION_DATA_STATE_DATA
        
    Returns 32byte size of the partition.
*/
uint32 UpgradeGetPartitionSizeInPartitionDataState(void);
/*!
    @brief To check if the upgrade lib is in UPGRADE_PARTITION_DATA_STATE_DATA
        
    Returns TRUE if in UPGRADE_PARTITION_DATA_STATE_DATA
*/
bool  UpgradeIsPartitionDataState(void);
/*!
    @brief To set the partition data block size which the upgrade lib could expect from its transport
        
    Returns None
*/
void UpgradeSetPartitionDataBlockSize(uint32 size);

/*!
    @brief To inform vm app that downloading of upgrade data from host app has begun.
        
    Returns None
*/
void UpgradeSendStartUpgradeDataInd(void);
/*!
    @brief To inform vm app that downloading of upgrade data from host app has ended.
        
    Returns None
*/
void UpgradeSendEndUpgradeDataInd(upgrade_end_state_t state);

/*!
    @brief To inform Upgrade library to reset the chip and swap to the Upgraded Image
        
    Returns None
*/
void UpgradeImageSwap(void);


/*!
    @brief Query if Upgrade library has been initialised.
    @return bool TRUE if Upgrade lib has been initialised, FALSE otherwise.
*/
bool UpgradeIsInitialised(void);

/*!
    @brief Query if Upgrade library has a transport connected.
    @return bool TRUE if Upgrade lib has a transport connected, FALSE otherwise.
*/
bool UpgradeTransportInUse(void);

/*!
    @brief Get the version information from the upgrade library.
    @param pointer to uint16 major version
    @param pointer to uint16 minor version
    @param pointer to uint16 config version
    @return bool TRUE if the major, minor and config have been populated, FALSE otherwise.
*/
bool UpgradeGetVersion(uint16 *major, uint16 *minor, uint16 *config);

/*!
    @brief Get the "in progress" version information from the upgrade library.
    @param pointer to uint16 major version
    @param pointer to uint16 minor version
    @param pointer to uint16 config version
    @return bool TRUE if the major, minor and config have been populated, FALSE otherwise.
*/
bool UpgradeGetInProgressVersion(uint16 *major, uint16 *minor, uint16 *config);

/*!
    @brief Get an indication of whether a partial upgrade has been interrupted.
    @return bool TRUE if a partial upgrade has been interrupted, FALSE otherwise.
*/
bool UpgradePartialUpdateInterrupted(void);

/*!
    @brief Get an indication of whether the device has rebooted for an upgrade.
    @return bool TRUE if the device has successfully rebooted for an upgrade, FALSE otherwise.
*/
bool UpgradeIsRunningNewImage(void);

#endif /* UPGRADE_H_ */
