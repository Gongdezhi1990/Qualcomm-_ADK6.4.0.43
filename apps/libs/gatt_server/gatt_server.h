/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server.h
    
DESCRIPTION
    Header file for the "GATT Server" VM Library.
*/


/*!
@file    gatt_server.h
@brief   Header file for the "GATT Server" VM library.

        This file provides documentation for the "GATT Server" VM library
        API.
*/


#ifndef GATT_GATT_SERVER_H_
#define GATT_GATT_SERVER_H_

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* External library headers */
#include <library.h>
#include <gatt.h>


/*! @brief The GATT Server internal structure for the server role.
    
    This structure is visible to the application as it is responsible for 
    managing resource to pass to the "GATT Server" library.
    The elements of this structure are only modified by the "GATT Server"
    library. The application SHOULD NOT modify the values at any time as it 
    could lead to undefined behaviour.
 */
typedef struct __GGATTS
{
    TaskData  lib_task;
    Task      app_task;
} GGATTS;


/*!
    @brief Status code returned from the "GATT server" library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    gatt_server_status_success,
    gatt_server_status_registration_failed,
    gatt_server_status_invalid_parameter,
    gatt_server_status_not_allowed,
    gatt_server_status_failed
    
} gatt_server_status_t;


/*!
    @brief "GATT Server" Message IDs
    
    Defines IDs for the "GATT Server" task messages that are sent to
    external tasks.
*/
typedef enum __gatt_gatt_server_message_id
{
    GATT_SERVER_READ_CLIENT_CONFIG_IND = GATT_SERVER_MESSAGE_BASE,
    GATT_SERVER_WRITE_CLIENT_CONFIG_IND,
    GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM,

    /* Library message limit */
    GATT_SERVER_MESSAGE_TOP
    
} gatt_server_message_id_t;


/*!
    @brief Contents of the GATT_SERVER_READ_CLIENT_CONFIG_IND message that
    is sent by the library, due to a read of the local GATT Service Changed
    client configuration descriptor. The application must call
    GattServerReadClientConfigResponse() to respond to this message.


    @param cid          The Connection ID that wants to read the descriptor.
    @param handle       The Handle being accessed.
 */
typedef struct PACK_STRUCT __GATT_SERVER_READ_CLIENT_CONFIG_IND
{
    uint16 cid;
    uint16 handle;

} GATT_SERVER_READ_CLIENT_CONFIG_IND_T;



/*! 
    @brief Contents of the SERVER_WRITE_CLIENT_CONFIG_IND message that
    is sent by the library, due to a write of the local GATT Service Changed 
    client configuration descriptor.
    
    @param cid          The connection ID that wrote the descriptor.
    @param config_value The value that was written to the descriptor.
*/
typedef struct PACK_STRUCT __GATT_SERVER_WRITE_CLIENT_CONFIG_IND
{
    uint16 cid;
    uint16 config_value;
    
} GATT_SERVER_WRITE_CLIENT_CONFIG_IND_T;


/*!
    @brief Contains status of the send Service Changed Indication request.
    
    This message is received in response to 
    GattServerSendServiceChangedIndication(). When an Indication is
    successfully sent, this message is returned with status 
    'gatt_status_success_sent'.
    Then, when the remote device responds to the notification, this message
    should be returned with status 'gatt_status_success'.
    
    @param  status indicates the outcome of the send indication request.
    @param  cid Connection identifier of remote device.
*/
typedef struct PACK_STRUCT __GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM
{
    gatt_status_t   status;
    uint16          cid;
    
} GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM_T;


/*!
    @brief Initialises the "GATT Server" Service Library.
    
    @param gatt_server A valid area of memory that the service library can use.
    @param app_task The Task that will receive the messages sent from this 
           GATT server library.
    @param start_handle Reports the start handle ofthe GATT server
    @param end_handle Reports the end handle ofthe GATT server
    
    @return The status result of calling the API.

*/
gatt_server_status_t GattServerInit(GGATTS *gatt_server, Task app_task, uint16 start_handle, uint16 end_handle);


/*!
    @brief Send an indication to the remote device for the GATT Service
           Changed characteristic. This should only be used to inform bonded
           devices when the local GATT database has changed.
    
    @param gatt_server Pointer to the memory area holding the GATT Servers
           run-data.
    @param cid The connection ID to send the indication to.
    
    @return TRUE if the indication was sent, FALSE otherwise.
*/
bool GattServerSendServiceChangedIndication(GGATTS *gatt_server, uint16 cid);

/*!
    @brief Send a response to the remote device for the GATT Service
           Changed Client Configuration read request. This should always be
           called in response to receiving GATT_SERVER_READ_CLIENT_CONFIG_IND
           message.

    @param gatt_server Pointer to the memory area holding the GATT Servers
           run-data.
    @param cid    The connection ID to send the indication to.
    @param handle The Handle being accessed.
    @param config The client configuration .

    @return TRUE if the response was sent, FALSE otherwise.
*/
bool GattServerReadClientConfigResponse(GGATTS *gatt_server, uint16 cid, uint16 handle, uint16 config);


#endif
