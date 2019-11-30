/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/


/*!
@file    gatt_manager.h
@brief   Header file for the GATT Manager library.

        This file provides documentation for the GATT Manager VM library API.
*/


#ifndef GATTMANAGER_H_
#define GATTMANAGER_H_

#include <library.h>
#include <csrtypes.h>
#include <message.h>
#include <bdaddr_.h>
#include <gatt.h>


/*!
    @brief GATT Manager Message IDs

    Defines IDs for the GATT Manager messages that are sent to external tasks.
 */
typedef enum __gatt_manager_message_id
{
    GATT_MANAGER_REGISTER_WITH_GATT_CFM = GATT_MANAGER_MESSAGE_BASE,
    GATT_MANAGER_SERVER_ACCESS_IND,
    GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM,
    GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND,
    GATT_MANAGER_REMOTE_SERVER_INDICATION_IND,
    GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM,
    GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM,
    GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM,
    GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM,
    GATT_MANAGER_DISCONNECT_IND,
    GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM,
    GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM,
    GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM,
    GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM,
    GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM,
    GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM,
    GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM,
    GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM,
    GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM,
    GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM,
    GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM,
    GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND,
    GATT_MANAGER_CANCEL_REMOTE_SERVER_CONNECT_CFM,

    /* Library message limit */
    GATT_MANAGER_MESSAGE_TOP

} gatt_manager_message_id_t;


/*!
    @brief GATT Manager status codes.

    Defines the return status codes for GATT Manager upstream messages.
*/
typedef enum __gatt_manager_status
{
    gatt_manager_status_success = 0,            /*! Requested operation was successful */
    gatt_manager_status_not_initialised,        /*! GATT Manager has not been initialised */
    gatt_manager_status_wrong_state,            /*! Wrong state to perform the requested operation */
    gatt_manager_status_invalid_parameters,     /*! The parameters that were passed are invalid */
    gatt_manager_status_failed                  /*! Generic failure */

} gatt_manager_status_t;


/*!
    @brief Registration parameters used by GATT Servers.

    Defines the registration parameters that are required when a GATT servers
    needs to register with the GATT Manager.
 */
typedef struct __gatt_manager_server_registration_params
{
   Task             task;          /*! The task to associate with the handles in the database */
   uint16           start_handle;  /*! This is the first handle in the pre-built GATT database that the server task is interested in */
   uint16           end_handle;    /*! This is the last handle in the pre-built GATT database that the server task is interested in */

} gatt_manager_server_registration_params_t;


/*!
    @brief Registration parameters used by GATT Clients

    Defines the registration parameters that are required when a GATT Clients
    needs to register with the GATT Manager.
*/
typedef struct __gatt_manager_client_registration_params
{
    Task            client_task;    /*! The task to associated with the handles in the database */
    uint16          cid;            /*! The CID of the device on which the service side of the service is to be accessed */
    uint16          start_handle;   /*! The first handle which the service wishes to access */
    uint16          end_handle;     /*! The last handle which the service wishes to access */

} gatt_manager_client_registration_params_t;


/*!
    @brief Service parameters used by GATT Clients.

    Defines the service data a client can retrieve from the GATT Manager.
*/
typedef struct __gatt_manager_client_service_data
{
    uint16  start_handle;  /*! The first handle registered by the client */
    uint16  end_handle;    /*! The last handle registered by the client */

} gatt_manager_client_service_data_t;


/*!
    @brief GATT Access Indication, routed by the GATT MANAGER.

    Indicates that the remote device is accessing a value in the Server database.
    The application must respond with approval (security) and the value
    to be used; GattManagerServerAccessResponse.

    @param  cid Connection identifier of remote device.
    @param  handle Handle being accessed.
    @param  flags uses ATT_ACCESS range.
    @param  offset The offset of the first octet to be accessed.
    @param  size_value The length of the value
    @param  value The value data.
*/
typedef GATT_ACCESS_IND_T GATT_MANAGER_SERVER_ACCESS_IND_T;


/*!
    @brief GATT Characteristic Value Notification.

    This message is received when a server is configured to notify a
    Characteristic Value to a client without expecting any Attribute Protocol
    layer acknowledgement that the notification was successfully received.

    @param cid Connection identifier of the remote device.
    @param handle Handle of the characteristic value.
    @param size_value Length of the characteristic value.
    @param value The characteristic value.
*/
typedef GATT_NOTIFICATION_IND_T GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T;


/*!
    @brief GATT Characteristic Value Indication.

    This sub-procedure is used when a server is configured to indicate a
    Characteristic Value to a client and expects an Attribute Protocol layer
    acknowledgement that the indication was successfully received.

    @param cid Connection identifier of the remote device.
    @param handle Handle of the characteristic value.
    @param size_value Length of the characteristic value.
    @param value The characteristic value.
*/
typedef GATT_INDICATION_IND_T GATT_MANAGER_REMOTE_SERVER_INDICATION_IND_T;


/*!
    @brief GATT Manager Disconnection Indication. This message will be sent to
    the task that initialised the GATT Manager library.

    @param status Disconnection status
    @param cid Connection identifier of remote device
*/
typedef GATT_DISCONNECT_IND_T GATT_MANAGER_DISCONNECT_IND_T;


/*!
    @brief Initialises the GATT Manager Library.

           If servers are required, the application must register each
           server with the GATT Manager using the API GattManagerRegisterServer().

           After all required servers have been registered, the application
           must call the API GattMaangerRegisterGatt() to register the GATT
           Manager with the GATT Library.

           If no servers are required, after successful initialisation, the
           application must call the API GattManagerRegisterWithGatt().

           GattManagerInit() will cause a panic if malloc fails or if it is
           called a second time without calling GattManagerDeInit() inbetween
           the calls.

    @param task The task that initialised the GATT Manager; GATT Manager
           messages will be sent to this task, must not be NULL.

    @return FALSE if task is NULL, otherwise true.
*/
bool GattManagerInit(Task task);


/*!
    @brief De-Initialises the GATT Manager Library.

    @return void
*/
void GattManagerDeInit(void);


/*!
    @brief This API is used to register the Const Database with the GATT
           Manager.

           This functions must not be called if there are no Servers to be
           registered.

           A Const database must be successfully registered with the GATT
           Manager before calling GattManagerRegisterServer().

    @param db_ptr pointer to const database, must not be NULL
    @param size   size of the const database, must be > 0

    @return TRUE on success
*/
bool GattManagerRegisterConstDB(const uint16* db_ptr, uint16 size);


/*!
    @brief This API is used to register the GATT Manager with the GATT library.

           If servers are required they must have been registered with the
           GATT Manager before calling this API as this function will register
           the GATT database with the GATT library.

           If servers are not required, this API can be called immediately
           after GattManagerInit() returned success.

           The application  should wait for a GATT_MANAGER_REGISTER_WITH_GATT_CFM message
           that will indicate the outcome of the GATT registration request.

    @return void
*/
void GattManagerRegisterWithGatt(void);

/*!
    @brief GATT Manager has registered with the GATT library.

    @param status Indicates the outcome of the registration request.
*/
typedef struct PACK_STRUCT __GATT_MANAGER_REGISTER_WITH_GATT_CFM
{
    gatt_manager_status_t  status;

} GATT_MANAGER_REGISTER_WITH_GATT_CFM_T;


/*!
    @brief Function for GATT Servers to register themselves with the GATT
           Manager.

    @param server Registration parameters the server must supply to register
           with the GATT Manager.

    @return GATT Manager status code indicating the outcome of the server
            registration request.
*/
gatt_manager_status_t GattManagerRegisterServer(const gatt_manager_server_registration_params_t *server);


/*!
    @brief Start BLE Advertising and wait for a client device to discover and
           connect to the local device.

           GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM  indicating failure or when a
           client device has successfully connected.

    @param task The task that wants responsibility for managing the connection
           to the client device.
    @param conn_type The type of connection to use.
    @param taddr This is only required when using gatt_connection_ble_slave_directed
           advertising. It is the address of device the directed adverts are
           intended for; no other devices will receive the directed adverts.

    @return void
*/
void GattManagerWaitForRemoteClient (
        Task task,
        const typed_bdaddr *taddr,
        gatt_connection_type conn_type
        );

/*!
    @brief Sent if a BR/EDR device attempts to connect GATT

    @param taddr Typed BD_ADDR of the client device.
    @param flags Connection configuration flags.
    @param cid Connection identifier of remote device.
    @param mtu Connection MTU.
*/
typedef GATT_CONNECT_IND_T GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND_T;

/*!
    @brief Respond to GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND

    @param cid Connection identifier of remote device.
    @param flags Connection configuration flags.
    @param accept TRUE to accept, FALSE to reject

    @return void
*/
void GattManagerRemoteClientConnectResponse(uint16 cid, uint16 flags, bool accept);

/*!
    @brief Sent in response to a GattManagerWaitForRemoteClient().

    @param status Indicates the outcome of the client connection.
    @param taddr Typed BD_ADDR of the client device.
    @param flags Connection configuration flags.
    @param cid Connection identifier of remote device.
    @param mtu Connection MTU.
*/
typedef GATT_CONNECT_CFM_T GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T;


/*!
    @brief Cancel the BLE Advertising after the function
           GattManagerWaitForRemoteClient() was called, this will stop BLE
           Advertising.

           A GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM_T message shall be sent
           indicating the outcome of the request.

    @return void
*/
void GattManagerCancelWaitForRemoteClient(void);

/*!
    @brief Sent in response to GattManagerCancelWaitForRemoteClient().

    @param status Indicates the outcome of the request.
*/
typedef struct PACK_STRUCT __GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM
{
    gatt_manager_status_t   status;

} GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM_T;


/*!
    @brief Respond to rerouted GATT Access Indication.

    Used in response to GATT_MANAGER_SERVER_ACCESS_IND message, indicating that the remote
    device wants to access a value in the server database and that the
    application must provide permission for the value to return, depending on the
    permission flags set.

    @param task The server task that is sending the access response
    @param cid Connection identifier of the remote device
    @param handle Handle of the value
    @param result Result of Access Indication.
    @param size_value the size of the value (octets)
    @param value The value as an octet array, which is copied.

    @return TRUE if the response was sent, FALSE otherwise.
*/
bool GattManagerServerAccessResponse(
        Task   task,
        uint16 cid,
        uint16 handle,
        uint16 result,
        uint16 size_value,
        const uint8 *value
        );


/*!
    @brief GATT Remote Client Notification

    Sends a notification to the remote client for the handle and value. There
    is no response to the application task if this action fails.

    The notification does not update the characteristic value in the server
    ATT database itself.

    GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM_T message is returned.

    @param task        The server task that is sending the notification request
    @param cid         Connection identifier of the remote device
    @param handle      Handle of the characteristic
    @param size_value  Length of the characteristic value to be notified.
    @param value       The characteristic value to be notified. This is copied.

    @return void
*/
void GattManagerRemoteClientNotify(
        Task   task,
        uint16 cid,
        uint16 handle,
        uint16 size_value,
        const uint8 *value
        );

/*!
    @brief GATT Remote Client Notification Confirmation.

    Sent in response to a GattManagerRemoteClientNotify().

    @param  status indicates the outcome of the send notification request.
    @param  cid Connection identifier of remote device.
    @param  handle The handle ID of the characteristic the notification was
            requested for.
*/
typedef GATT_NOTIFICATION_CFM_T GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM_T;


/*!
    @brief GATT Remote Client Indication Request.

    Sends a indication to the client for the handle and value. The client
    should respond to the indication.

    The indication does not update the characteristic value in the server
    ATT database itself.

    GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM message is returned.

    @param task        The server task that is sending the indication request
    @param cid         Connection identifier of the remote device
    @param handle      Handle of the characteristic
    @param size_value  Length of the characteristic value to be indicated.
    @param value       The characteristic value to be indicated. This is copied.

    @return void

*/
void GattManagerRemoteClientIndicate(
        Task   task,
        uint16 cid,
        uint16 handle,
        uint16 size_value,
        const uint8 *value
        );

/*!
    @brief GATT Remote Client Indication Confirmation.

    This message is received in response to GattManagerRemoteClientIndicate().
    When an Indication is successfully sent, this message is returned with
    status 'gatt_status_success_sent'. Then, when the remote device responds to
    the notification, this message should be returned with status
    'gatt_status_success'.

    @param  status indicates the outcome of the send indication request.
    @param  cid Connection identifier of remote device.
    @param  handle The handle ID of the characteristic the indication was
            requested for.
*/
typedef GATT_INDICATION_CFM_T GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM_T;


/*!
    @brief Function for GATT clients to register themselves with the GATT
           Manager.

    @param client Registration parameters the client must supply to register
           with the GATT Manager.

    @return GATT Manager status code indicating the outcome of the client
            registration request.
*/
gatt_manager_status_t GattManagerRegisterClient(const gatt_manager_client_registration_params_t *client);


/*!
    @brief Function for GATT clients to unregister themselves from the GATT
           Manager.

    @param client The unique Task instance associated with this client.

    @return GATT Manager status code indicating the outcome of the client
            unregistration request.
*/
gatt_manager_status_t GattManagerUnregisterClient(Task client);


/*!
    @brief Function for GATT clients to access data they registered with the
           GATT Manager.

    @param client The unique Task instance associated with this client.
    @param service_data Pointer to a data area that will be populated by this function.

    @return TRUE if the client data was found and populated, FALSE otherwise.
*/
bool GattManagerGetClientData(const Task client, gatt_manager_client_service_data_t * service_data);


/*!
    @brief Attempt connection to a remote server device.

           A GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM message will be returned once
           a connection attempt is complete.

    @param task The task that wants responsibility for managing the connection
           to the server device.
    @param taddr The address of the device to make a connection to.
    @param conn_type The type of connection to use.
    @param conn_timeout If TRUE, the attempt will timeout.

    @return void.
*/
void GattManagerConnectToRemoteServer(
        const Task task,
        const typed_bdaddr      *taddr,
        gatt_connection_type    conn_type,
        bool                    conn_timeout
        );

/*!
    @brief Sent in response to a GattManagerConnectToRemoteServer().

    @param status Indicates the outcome of the client connection.
    @param taddr Typed BD_ADDR of the client device.
    @param flags Connection configuration flags.
    @param cid Connection identifier of remote device.
    @param mtu Connection MTU.
*/
typedef GATT_CONNECT_CFM_T GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM_T;


/*!
    @brief Cancel the BLE connection after the function
           GattManagerConnectToRemoteServer() was called.

           A GATT_MANAGER_CANCEL_REMOTE_SERVER_CONNECT_CFM_T message shall be sent
           indicating the outcome of the request.

           This message will be sent to the task supplied as well as the
           task associated with the connection in 
           GattManagerConnectToRemoteServer().

    @param  Task to receive cancel confirm. Confirm will also be sent to the task 
            specified in GattManagerConnectToRemoteServer if different.

    @return void
*/
void GattManagerCancelConnectToRemoteServer(Task task);


/*!
    @brief Sent in response to GattManagerCancelConnectToRemoteServer().

    Status will be gatt_manager_status_success if a connection was cancelled.
    Other status codes represent a failure. gatt_manager_status_wrong_state 
    indicates that there was no connection in progress. 
    gatt_manager_status_failed may indicate that the cancel was tried too
    quickly.

    @param status Indicates the outcome of the request. 
*/
typedef struct PACK_STRUCT __GATT_MANAGER_CANCEL_REMOTE_SERVER_CONNECT_CFM
{
    gatt_manager_status_t   status;

} GATT_MANAGER_CANCEL_REMOTE_SERVER_CONNECT_CFM_T;


/*!
    @brief GATT Characteristic Discovery: Discover all characteristics
           defined within the clients registered handle range.

    @note  If this API is called with a NULL client task, it will Panic.

    @param client Unique client task whose characteristics shall be discovered.

    @return a GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM message is sent to the
            client.
*/
void GattManagerDiscoverAllCharacteristics(const Task client);

/*!
    @brief Response message for GattManagerDiscoverAllCharacteristics()

    @param cid Connection identifier of the remote device
    @param declaration Handle of the characteristic declaration
    @param handle Handle of the characteristic value
    @param properties Characteristic properties
    @param uuid_type UUID type of the characteristic
    @param uuid UUID of the characteristic
    @param more_to_come Flag indicating if more services will follow (TRUE) or not (FALSE)
    @param status Status of the request
*/
typedef GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_T GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T;


/*!
    @brief GATT Characteristic Descriptor Discovery: Discover all characteristic
           descriptors sub-procedure.

    @note  If this API is called with a NULL client task, it will Panic.

    @param client Unique client task whose characteristics shall be discovered.
    @param start_handle The first handle to discover characteristic descriptors from.
    @param end_handle The last handle to discover characteristic descriptors to.

    @return a GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM message is sent
            to the client.
*/
void GattManagerDiscoverAllCharacteristicDescriptors(const Task client,
                                                     uint16 start_handle,
                                                     uint16 end_handle);

/*!
    @brief Response message for GattManagerDiscoverAllCharacteristicDescriptors()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param uuid_type UUID type of the characteristic
    @param uuid UUID of the characteristic
    @param more_to_come Flag indicating if more services will follow (TRUE) or not (FALSE)
    @param status Status of the request
*/
typedef GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T;


/*!
    @brief Read Characteristic Value from the remote device.

    @note  If this API is called with a NULL client task, it will Panic.

           A GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM message will be returned
           once the read is complete or if it fails.

    @param client       Client task, this task must have been registered
                        with GATT manager via GattManagerRegisterClient() before
                        it can be used.
    @param handle       Handle of the characteristic

    @return void
*/
void GattManagerReadCharacteristicValue(const Task client, uint16 handle);

/*!
    @brief Response message for GattManagerReadCharacteristicValue()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
    @param size_value Length of the characteristic value
    @param value The characteristic value
*/
typedef GATT_READ_CHARACTERISTIC_VALUE_CFM_T GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM_T;


/*!
    @brief GATT Characteristic Value Read: Read Using Characteristic UUID
           sub-procedure. This sub-procedure is used to read a characteristic
           value from a server when the client wants to search for a specific
           characteristic UUID.

    @note  If this API is called with a NULL client task, it will Panic.
           The specific characteristic may exist multiple times in the
           clients handle range and it is up to the client to perform another
           Read Using Characteristic UUID sub-procedure should it wish to read
           another value.

    @param client Unique client task whose characteristics shall be discovered.
    @param start_handle The first handle the characteristic should be defined after.
    @param end_handle The last handle the characteristic should be defined before.
    @param uuid_type UUID type of the characteristic.
    @param uuid UUID of the characteristic.

    @return a GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM message is sent to the
            client.
*/
void GattManagerReadUsingCharacteristicUuid(const Task client,
                                            uint16 start_handle,
                                            uint16 end_handle,
                                            gatt_uuid_type_t uuid_type,
                                            const gatt_uuid_t *uuid);

/*!
    @brief Response message for GattManagerReadUsingCharacteristicUuid()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param more_to_come Flag indicating if more services will follow (TRUE) or not (FALSE)
    @param status Status of the request
    @param size_value Length of the characteristic value
    @param value The characteristic value
*/
typedef GATT_READ_USING_CHARACTERISTIC_UUID_CFM_T GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM_T;


/*!
    @brief GATT Characteristic Value Read: Read Long Characteristic Value
           sub-procedure. This sub-procedure is used to read a Characteristic
           Value from a server when the client knows its handle and the length
           of the Characteristic Value is longer than can be sent in a single
           Read Response message.

    @note  If this API is called with a NULL client task, it will Panic.
           This request can also be used to perform GATT Read Long
           Characteristic Descriptor sub-procedure.

    @param client Unique client task that wants to read from the characteristic.
    @param handle The handle of the characteristic to read from.

    @return a GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM message is sent to the
            client.
*/
void GattManagerReadLongCharacteristicValue(Task client, uint16 handle);

/*!
    @brief Response message for GattManagerReadLongCharacteristicValue()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param more_to_come Flag indicating if more services will follow (TRUE) or not (FALSE)
    @param status Status of the request
    @param offset Offset to the characteristic value in this message
    @param size_value Length of the characteristic value
    @param value The characteristic value
*/
typedef GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM_T GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM_T;


/*!
    @brief GATT Characteristic Value Write: Write Without Response
           sub-procedure. This sub-procedure is used to write a Characteristic
           Value to a server when the client knows the Characteristic Value
           Handle and the client does not need an acknowledgement that the write
           was successfully performed.

    @note  If this API is called with a NULL client task, it will Panic.
           This sub-procedure only writes the first (ATT_MTU - 3) octets
           of a Characteristic Value. This sub-procedure cannot be used to write
           a long characteristic; instead the Write Long Characteristic Values
           sub-procedure should be used.

    @param client Unique client task that wants to read from the characteristic.
    @param handle Handle of the characteristic.
    @param size_value Length of the characteristic value to be written.
    @param value The characteristic value to be written.

    @return a GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM message is sent to the client.
*/
void GattManagerWriteWithoutResponse(const Task client,
                                     uint16 handle,
                                     uint16 size_value,
                                     uint8 *value);

/*!
    @brief Response message for GattManagerWriteWithoutResponse()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
*/
typedef GATT_WRITE_WITHOUT_RESPONSE_CFM_T GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM_T;


/*!
    @brief GATT Characteristic Value Write: Signed Write Without Response
           sub-procedure. This sub-procedure is used to write a Characteristic
           Value to a server when the client knows the Characteristic Value
           Handle and the ATT Bearer is not encrypted. This sub-procedure shall
           only be used if the Characteristic Properties authenticated bit is
           enabled and the client and server device share a bond.

    @note  If this API is called with a NULL client task, it will Panic.
           This sub-procedure only writes the first (ATT_MTU - 15) octets
           of an Attribute Value. This sub-procedure cannot be used to write a
           long Attribute; instead the Write Long Characteristic Values
           sub-procedure should be used.

    @param client Unique client task that wants to read from the characteristic.
    @param handle Handle of the characteristic.
    @param size_value Length of the characteristic value to be written.
    @param value The characteristic value to be written.

    @return a GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM is sent to the client.
*/
void GattManagerSignedWriteWithoutResponse(const Task client,
                                           uint16 handle,
                                           uint16 size_value,
                                           uint8 *value);

/*!
    @brief Response message for GattManagerSignedWriteWithoutResponse()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
*/
typedef GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM_T GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM_T;


/*!
    @brief GATT Characteristic Value Write: Write Characteristic Value
           sub-procedure. This sub-procedure is used to write a Characteristic
           Value to a server when the client knows the Characteristic Value
           Handle.

    @note  If this API is called with a NULL client task, it will Panic.
           This sub-procedure only writes the first (ATT_MTU - 3) octets of a
           Characteristic Value. This sub-procedure cannot be used to write a
           long Attribute; instead the Write Long Characteristic Values
           sub-procedure should be used.
           This request can also be used to perform GATT Write Characteristic
           Descriptor sub-procedure.

    @param client Unique client task that wants to read from the characteristic.
    @param handle Handle of the characteristic.
    @param size_value Length of the characteristic value to be written.
    @param value The characteristic value to be written.

    @return a GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM message is sent to the client.
*/
void GattManagerWriteCharacteristicValue(const Task client,
                                         uint16 handle,
                                         uint16 size_value,
                                         const uint8 *value);

/*!
    @brief Response message for GattManagerWriteCharacteristicValue()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
*/
typedef GATT_WRITE_CHARACTERISTIC_VALUE_CFM_T GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T;


/*!
    @brief GATT Characteristic Value Write: Write Long Characteristic Value
           sub-procedure. This sub-procedure is used to write a Characteristic
           Value to a server when the client knows the Characteristic Value
           Handle but the length of the Characteristic Value is longer than can
           be sent in a single Write Characteristic Value message.

    @note  If this API is called with a NULL client task, it will Panic.

    @param client Unique client task that wants to read from the characteristic.
    @param handle Handle of the characteristic.
    @param size_value Length of the characteristic value to be written.
    @param value The characteristic value to be written.

    @return a GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM message is sent to the
            client.
*/
void GattManagerWriteLongCharacteristicValue(const Task client,
                                             uint16 handle,
                                             uint16 size_value,
                                             const uint8 *value);

/*!
    @brief Response message for GattManagerWriteLongCharacteristicValue()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
*/
typedef GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM_T GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM_T;


/*!
    @brief GATT Characteristic Value Write: Reliable Write sub-procedure.
           This sub-procedure is used to write a Characteristic Value to a
           server when the client knows the Characteristic Value Handle, and
           assurance is required that the correct Characteristic Value is going
           to be written by transferring the Characteristic Value to be written
           in both directions before the write is performed.

    @note  If this API is called with a NULL client task, it will Panic.
           This sub-procedure can also be used when multiple values must be
           written, in order, in a single operation.
           When this sub-procedure is used the Characteristic Values are first
           transferred to the server and then back to the client using
           GattReliableWritePrepareRequest(). At this point the GATT library
           checks that the value is correct and returns the confirmation message
           to the application. After the confirmation the client can request
           another Reliable Write, or request the server to store or cancel all
           queued Reliable Writes using GattReliableWriteExecuteRequest().

    @param client Unique client task that wants to read from the characteristic.
    @param handle Handle of the characteristic.
    @param offset Offset to the characteristic value.
    @param size_value Length of the characteristic value to be written.
    @param value The characteristic value to be written.

    @return a GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM message is sent to the client.
*/
void GattManagerReliableWritePrepare(const Task client,
                                     uint16 handle,
                                     uint16 offset,
                                     uint16 size_value,
                                     uint8 *value);

/*!
    @brief Response message for GattManagerReliableWritePrepare()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
*/
typedef GATT_RELIABLE_WRITE_PREPARE_CFM_T GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM_T;


/*!
    @brief GATT Characteristic Value Write: Execute Reliable Write
           sub-procedure. This sub-procedure is used to execute a prepared
           reliable write request. The client task shall receive a
           GATT_RELIABLE_WRITE_EXECUTE_CFM message if the request was actioned.

    @note  If this API is called with a NULL client task, it will Panic.

    @param client Unique client task that wants to read from the characteristic.
    @param execute A flag choosing whether to store (TRUE) or cancel (FALSE)
           prepared write queue on the server.

    @return a GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM message is sent to the client.
*/
void GattManagerReliableWriteExecute(const Task client, bool execute);

/*!
    @brief Response message for GattManagerReliableWriteExecute()

    @param cid Connection identifier of the remote device
    @param handle Handle of the characteristic value
    @param status Status of the request
*/
typedef GATT_RELIABLE_WRITE_EXECUTE_CFM_T GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM_T;


/*!
    @brief GATT Characteristic Value Notification Indication acknowledgement

    This function is used to acknowledge to the server that the client has received
    Handle Value Indication, and server may send another indication.

    Only one indication may be sent by the server at a time.

    This function is used in response to GATT_MANAGER_REMOTE_SERVER_INDICATION_IND message.

    @param cid          Connection identifier of the remote device
*/
void GattManagerIndicationResponse(uint16 cid);


/*!
    @brief Disconnect a GATT connection.

    @param cid The connection identifier of the connection to disconnect.
*/
void GattManagerDisconnectRequest(uint16 cid);


/*!
    \brief Get the adjusted handle from the Gatt Database.
    for an existing connection.

    \param task The task whose adjusted handle is to be obtained.
    \param handle Handle.
    \return uint16 value of the adjusted handle.
*/
uint16 GattManagerGetServerDatabaseHandle(Task task, uint16 handle);

#endif /* GATTMANAGER_H_ */
