/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    gatt_device_info_client.h
@brief   Header file for the GATT Device Information client library.

        This file provides documentation for the GATT Device information client library
        API (library name: gatt_device_info_client).
*/

#ifndef GATT_DEVICE_INFO_CLIENT_H_
#define GATT_DEVICE_INFO_CLIENT_H_

#include <csrtypes.h>
#include <message.h>

#include <library.h>

/*! Bitmask for Device Information Service Characteristic Handles 
*/
#define    MANUFACTURER_NAME_CHAR        0x0001
#define    MODEL_NUMBER_CHAR             0x0002
#define    SERIAL_NUMBER_CHAR            0x0004
#define    HARDWARE_REVISION_CHAR        0x0008
#define    FIRMWARE_REVISION_CHAR        0x0010
#define    SOFTWARE_REVISION_CHAR        0x0020
#define    SYSTEM_ID_CHAR                0x0040
#define    IEEE_DATA_LIST_CHAR           0x0080
#define    PNP_ID_CHAR                   0x0100

/*! @brief The Device Info internal structure for the client role.

    This structure is visible to the application as it is responsible for managing resource to pass to the Device info library.
    The elements of this structure are only modified by the Device info library.
    The application SHOULD NOT modify the values at any time as it could lead to undefined behaviour.
    
 */
typedef struct __GDISC
{
    TaskData lib_task;
    Task app_task;
    uint16 manufacturer_name_handle;
    uint16 model_number_handle;
    uint16 serial_number_handle;
    uint16 hardware_revision_handle;
    uint16 firmware_revision_handle;
    uint16 software_revision_handle;
    uint16 system_id_handle;
    uint16 ieee_data_list_handle;
    uint16 pnp_id_handle;
} GDISC;


/*! @brief Enumeration of messages a client task may receive from the Device Info client library.
 */
typedef enum
{
    GATT_DEVICE_INFO_CLIENT_INIT_CFM = GATT_DEVICE_INFO_CLIENT_MESSAGE_BASE,
    GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM,

    /* Library message limit */
    GATT_DEVICE_INFO_CLIENT_MESSAGE_TOP

} GattDeviceInfoClientMessageId;

/*!
    @brief Status code returned from the GATT Device Info client library

    This status code indicates the outcome of the request.
*/
typedef enum
{
    gatt_device_info_client_status_success,
    gatt_device_info_client_status_not_allowed,
    gatt_device_info_client_status_failed
} gatt_device_info_client_status_t;

/*!
    @brief Device Information Characteristic type for GATT Device Info client library
*/
typedef enum
{
    gatt_device_info_client_manufacturer_name,
    gatt_device_info_client_model_number,
    gatt_device_info_client_serial_number,
    gatt_device_info_client_hardware_revision,
    gatt_device_info_client_firmware_revision,
    gatt_device_info_client_software_revision,
    gatt_device_info_client_system_id,
    gatt_device_info_client_ieee_data_list,
    gatt_device_info_client_pnp_id,
    gatt_device_info_client_invalid_char
} gatt_device_info_type_t;

/*!
    @brief Parameters used by the Initialisation API, valid value of these  parameters are must for library initialisation  
*/
typedef struct
{
     uint16 cid;                /*!Connection ID of the GATT connection on which the server side Device Information service need to be accessed*/
     uint16 start_handle;       /*! The first handle of Device Information service need to be accessed*/
     uint16 end_handle;         /*!The last handle of Device Information service need to be accessed */
} GATT_DEVICE_INFO_CLIENT_INIT_PARAMS_T;

/*!
    @brief After the VM application has used the GATT manager library to establish a connection to a discovered BLE device in the Client role,
    it can discover any supported services in which it has an interest. It should then register with the relevant client service library
    (passing the relevant CID and handles to the service). For the Device Info client it will use this API. The GATT manager 
    will then route notifications and indications to the correct instance of the client service library for the CID.

    @param app_task The Task that will receive the messages sent from this Device Info client library.
    @param device_info_client A valid area of memory that the service library can use.
    @param client_init_params Initialisation parameters, CID,Start handle, end handle of Device Information Service 
    
    @return TRUE if successful, FALSE otherwise

*/
bool GattDeviceInfoClientInit(Task app_task, 
                              GDISC *const device_info_client,  
                              const GATT_DEVICE_INFO_CLIENT_INIT_PARAMS_T *const client_init_params);

/*!@brief Device Information client library initialisation confirmation 
*/
typedef struct PACK_STRUCT  __GATT_DEVICE_INFO_CLIENT_INIT_CFM
{
    const GDISC *device_info_client;                /*! Reference structure for the instance */
    uint16 cid;                                     /*! Connection Identifier for remote device */
    uint16 supported_char_mask;                     /*! Bitmask indicating the supported Device info characteristics */
    gatt_device_info_client_status_t status;        /*!status as per gatt_device_info_client_status_t */
} GATT_DEVICE_INFO_CLIENT_INIT_CFM_T;

/*!
    @brief This API is used to read a device information Characteristics from a remote device.
           If successful, a GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM message will be sent to the registered application Task

    @param device_info_client The client instance that was passed into the GattDeviceInfoClientInit API.
    @param device_info_type The Device Information Characteristic  to read.
    
    @return TRUE if successful, FALSE otherwise

*/
bool GattDeviceInfoClientReadCharRequest(GDISC *device_info_client,
                                         gatt_device_info_type_t device_info_type);

/*! @brief Contents of the GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM message that is sent by the library,
    as response to a read of the remote Device Information characteristic.
 */
typedef struct PACK_STRUCT __GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM
{
    const GDISC *device_info_client;            /*! Reference structure for the instance */
    gatt_device_info_type_t device_info_type;   /*! The Device Information Characteristic  to read. */
    gatt_device_info_client_status_t status;    /*!status as per gatt_device_info_client_status_t */
    uint16 size;                                /*! The size of the characteristic retrieved. */
    uint8 value[1];                             /*! The value of the characteristic retrieved. */
} GATT_DEVICE_INFO_CLIENT_READ_CHAR_CFM_T;

/*!
    @brief When a GATT connection is removed, the application must remove all client service instances that were
    associated with the connection (using the CID value).
    This is the clean up routine as a result of calling the GattDeviceInfoClientInit API. That is,
    the GattDeviceInfoClientInit API is called when a connection is made, and the GattDeviceInfoClientDestroy is called 
    when the connection is removed.

    @param device_info_client The client instance that was passed into the GattDeviceInfoClientInit API.
    
    @return TRUE if successful, FALSE otherwise

*/
bool GattDeviceInfoClientDestroy(GDISC *device_info_client);
    
#endif
