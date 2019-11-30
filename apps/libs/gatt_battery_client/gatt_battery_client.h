/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    gatt_battery_client.h
@brief   Header file for the GATT battery client library.

        This file provides documentation for the GATT battery client library
        API (library name: gatt_battery_client).
*/

#ifndef GATT_BATTERY_CLIENT_H_
#define GATT_BATTERY_CLIENT_H_

#include <csrtypes.h>
#include <message.h>

#include <library.h>


/*! @brief The battery internal structure for the client role.

    This structure is visible to the application as it is responsible for managing resource to pass to the battery library.
    The elements of this structure are only modified by the battery library.
    The application SHOULD NOT modify the values at any time as it could lead to undefined behaviour.

 */
typedef struct __GBASC
{
    TaskData lib_task;
    Task app_task;
    uint16 handle_level_start;
    uint16 handle_level_end;
    uint16 pending_cmd;
    uint16 pending_uuid;
} GBASC;


/*!
    @brief Status code returned from the GATT battery client library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    gatt_battery_client_status_success,
    gatt_battery_client_status_invalid_parameter,
    gatt_battery_client_status_not_allowed,
    gatt_battery_client_status_failed,
    gatt_battery_client_status_not_supported
} gatt_battery_client_status_t;


/*!
    @brief persistent data for each known battery server device.

    Each battery server device that is bonded can have data associated against
    it so that re-connections are much faster in that no GATT discovery is 
    required.
*/
typedef struct
{
    uint16  battery_level_handle;

} gatt_battery_client_device_data_t;


/*!
    @brief Contents of the GATT_BATTERY_CLIENT_INIT_CFM message that is sent by the library,
    as a response to the initialisation request.
 */
typedef struct __GATT_BATTERY_CLIENT_INIT_CFM
{
    const GBASC *battery_client;
    uint16 battery_level_handle;
    gatt_battery_client_status_t  status;

} GATT_BATTERY_CLIENT_INIT_CFM_T;


/*! @brief Contents of the GATT_BATTERY_CLIENT_READ_LEVEL_CFM message that is sent by the library,
    as response to a read of the remote battery level.
 */
typedef struct __GATT_BATTERY_CLIENT_READ_LEVEL_CFM
{
    const GBASC *battery_client;
    gatt_battery_client_status_t status;
    uint8 battery_level;
} GATT_BATTERY_CLIENT_READ_LEVEL_CFM_T;

/*! @brief Contents of the GATT_BATTERY_CLIENT_LEVEL_IND message that is sent by the library,
    as a result of a notification of the remote battery level.
 */
typedef struct __GATT_BATTERY_CLIENT_LEVEL_IND
{
    const GBASC *battery_client;
    uint8 battery_level;
} GATT_BATTERY_CLIENT_LEVEL_IND_T;

/*! @brief Contents of the GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM message that is sent by the library,
    as a result of setting notifications on the server.
 */
typedef struct __GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM
{
    const GBASC *battery_client;
    gatt_battery_client_status_t status;
} GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM_T;

/*! @brief Contents of the GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM message that is sent by the library,
    as a result of a read descriptor of the server.
 */
typedef struct __GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM
{
    const GBASC *battery_client;
    gatt_battery_client_status_t status;
    uint16 descriptor_uuid;
    uint16 size_value;
    uint8 value[1];
} GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM_T;

/*! @brief Enumeration of messages a client task may receive from the battery client library.
 */
typedef enum
{
    /* Client messages */
    GATT_BATTERY_CLIENT_INIT_CFM = GATT_BATTERY_CLIENT_MESSAGE_BASE,            /* 00 */
    GATT_BATTERY_CLIENT_READ_LEVEL_CFM,                                          /* 01 */
    GATT_BATTERY_CLIENT_LEVEL_IND,                                              /* 02 */
    GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM,                                    /* 03 */
    GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM,                            /* 04 */

    /* Library message limit */
    GATT_BATTERY_CLIENT_MESSAGE_TOP
} gatt_battery_client_message_id_t;


/*!
    @brief After the VM application has used the GATT manager library to establish a connection to a discovered BLE device in the Client role,
    it can discover any supported services in which it has an interest. It should then register with the relevant client service library
    (passing the relevant CID and handles to the service). For the battery client it will use this API. The GATT manager 
    will then route notifications and indications to the correct instance of the client service library for the CID.

    @param battery_client A valid area of memory that the service library can use.
    @param app_task The Task that will receive the messages sent from this battery client library.
    @param cid The connection ID.
    @param start_handle The start handle of the battery client instance.
    @param end_handle The end handle of the battery client instance.
    @param device_data Persistent data for the device.

    @return TRUE if successful, FALSE otherwise

*/
bool GattBatteryClientInit(GBASC *battery_client,
                           Task app_task,
                           uint16 cid,
                           uint16 start_handle,
                           uint16 end_handle,
                           gatt_battery_client_device_data_t * device_data);

/*!
    @brief This API is used to read a battery level from a remote device.

    @param battery_client The client instance that was passed into the GattBatteryClientInit API.

    @return A GATT_BATTERY_CLIENT_READ_LEVEL_CFM message will be sent to the registered application Task.

*/
void GattBatteryClientReadLevelRequest(const GBASC *battery_client);

/*!
    @brief This API is used to write the client characteristic configuration on a remote device,
    to enable notifications with the server.
    An error will be returned if the server does not support notifications.

    @param battery_client The client instance that was passed into the GattBatteryClientInit API.
    @param notifications_enable Set to TRUE to enable notifications on the server, FALSE to disable them.

    @return A GATT_BATTERY_CLIENT_SET_NOTIFICATION_ENABLE_CFM message will be sent to the registered application Task.

*/
void GattBatteryClientSetNotificationEnableRequest(const GBASC *battery_client, bool notifications_enable);

/*!
    @brief This API is used to read a descriptor of the battery level characteristic.
    This will only read the first descriptor found with the specified UUID, as it assumes
    a unique characteristic descriptor UUID exists per battery level characteristic.

    @param battery_client The client instance that was passed into the GattBatteryClientInit API.
    @param descriptor_uuid The UUID of the characteristic descriptor.

    @return A GATT_BATTERY_CLIENT_READ_DESCRIPTOR_CFM message will be sent to the registered application Task.

*/
void GattBatteryClientReadDescriptorRequest(const GBASC *battery_client, uint16 descriptor_uuid);

/*!
    @brief When a GATT connection is removed, the application must remove all client service instances that were
    associated with the connection (using the CID value).
    This is the clean up routine as a result of calling the GattBatteryClientInit API. That is,
    the GattBatteryClientInit API is called when a connection is made, and the GattBatteryClientDestroy is called 
    when the connection is removed.

    @param battery_client The client instance that was passed into the GattBatteryClientInit API.

    @return TRUE if successful, FALSE otherwise

*/
bool GattBatteryClientDestroy(GBASC *battery_client);


#endif
