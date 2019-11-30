/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server.h
    
DESCRIPTION
    Header file for the GATT "GAP SServer" VM Library.
*/


/*!
@file    gatt_gap_server.h
@brief   Header file for the "GAP Server" VM library.

        This file provides documentation for the "GAP Server" VM library
        API.
*/


#ifndef GATT_GAP_SERVER_H_
#define GATT_GAP_SERVER_H_

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* External library headers */
#include <library.h>
#include <gatt.h>


/*! @brief The GAP Server internal structure for the server role.
    
    This structure is visible to the application as it is responsible for 
    managing resource to pass to the "GAP Server" library.
    The elements of this structure are only modified by the "GAP Server"
    library. The application SHOULD NOT modify the values at any time as it 
    could lead to undefined behaviour.
 */
typedef struct __GGAPS
{
    TaskData  lib_task;
    Task      app_task;
} GGAPS;


/*!
    @brief Status code returned from the "GAP server" library

    This status code indicates the outcome of the request.
*/
typedef enum __gatt_gap_server_status
{
    gatt_gap_server_status_success,
    gatt_gap_server_status_registration_failed,
    gatt_gap_server_status_invalid_parameter,
    gatt_gap_server_status_not_allowed,
    gatt_gap_server_status_failed
    
} gatt_gap_server_status_t;


/*!
    @brief "GAP Server" Message IDs
    
    Defines IDs for the "GAP Server" task messages that are sent to
    external tasks.
*/
typedef enum __gatt_gap_server_message_id
{
    GATT_GAP_SERVER_READ_DEVICE_NAME_IND = GATT_GAP_SERVER_MESSAGE_BASE,
    GATT_GAP_SERVER_READ_APPEARANCE_IND,

    /* Library message limit */
    GATT_GAP_SERVER_MESSAGE_TOP
    
} gatt_gap_server_message_id_t;


/*!
    @brief Defines the GATT_GAP_SERVER_READ_DEVICE_NAME_IND message that
           is sent by this library.
           GattGapServerReadDeviceNameResponse() should be used to respond to 
           the request.
    
    @param gap_server   Pointer to the memory area holding the GAP Servers run-data.
    @param cid          The Connection ID that wants to read the characteristic.
    @param name_offset  The offset into the name that should be sent in the response.
 */
typedef struct PACK_STRUCT __GATT_GAP_SERVER_READ_DEVICE_NAME_IND
{
    const GGAPS *gap_server;
    uint16 cid;
    uint16 name_offset;
    
} GATT_GAP_SERVER_READ_DEVICE_NAME_IND_T;



/*! 
    @brief Defines the GATT_GAP_SERVER_READ_APPEARANCE_IND message that
           is sent by this library.
           GattGapServerReadAppearanceResponse() should be used to respond to 
           the request.
    
    @param gap_server   Pointer to the memory area holding the GAP Servers run-data.
    @param cid          The Connection ID that wants to read the characteristic.
*/
typedef struct PACK_STRUCT __GATT_GAP_SERVER_READ_APPEARANCE_IND
{
    const GGAPS *gap_server;
    uint16 cid;
    
} GATT_GAP_SERVER_READ_APPEARANCE_IND_T;


/*!
    @brief Initialises the "GAP Server" Service Library.
    
    @param gap_server A valid area of memory that the service library can use.
    @param app_task The Task that will receive the messages sent from this 
           GAP server library.
    @param start_handle Reports the start handle ofthe GAP server
    @param end_handle Reports the end handle ofthe GAP server
    
    @return The status result of calling the API.

*/
gatt_gap_server_status_t GattGapServerInit(GGAPS *gap_server, Task app_task, uint16 start_handle,
                                                                       uint16 end_handle);


/*!
    @brief Respond to a GATT_GAP_SERVER_READ_DEVICE_NAME_IND message.
    
    @param gap_server Pointer to the memory area holding the GAP Servers
           run-data.
    @param cid The connection ID to send the response to.
    @param size The size of the device name data that is to be sent to the
           remote device. This size must equal the name length from the offset position 
           requested in the GATT_GAP_SERVER_READ_DEVICE_NAME_IND message to the end of the name.
           A size of 0 can be returned if the name cannot be returned due to an invalid 
           offset position requested for the name.
    @param data The device name to send to the remote device, starting from the offset
           requested in the GATT_GAP_SERVER_READ_DEVICE_NAME_IND message.
   
    @return status of the request.
*/
gatt_gap_server_status_t GattGapServerReadDeviceNameResponse(GGAPS *gap_server,
                                                             uint16 cid,
                                                             uint16 size,
                                                             uint8 * data);


/*!
    @brief Respond to a GATT_GAP_SERVER_READ_APPEARANCE_IND message.
    
    @param gap_server Pointer to the memory area holding the GAP Servers
           run-data.
    @param cid The connection ID to send the response to.
    @param size The size of the device name data that is to be sent to the
           remote device.
    @param data The device name to send to the remote device.
    
    @return status of the request.
*/
gatt_gap_server_status_t GattGapServerReadAppearanceResponse(GGAPS *gap_server,
                                                             uint16 cid,
                                                             uint16 size,
                                                             uint8 * data);
#endif
