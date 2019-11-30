/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    gatt_scan_params_client.h
@brief   Header file for the GATT Scan Parameters Client library.

        This file provides documentation for GATT Scan Parameters Client library
        API (library name: gatt_scan_params_client).
*/


#ifndef GATT_SCAN_PARAMS_CLIENT_H
#define GATT_SCAN_PARAMS_CLIENT_H

#include <csrtypes.h>
#include <message.h>

#include "library.h"

/*!
    @brief persistent data for each known Scan Parameters server device.

    Each Scan Parameter server device that is bonded can have data associated against
    it so that re-connections are much faster in that case no GATT discovery is required.
*/
typedef struct
{
    uint16 scan_interval_window_handle;
    uint16 scan_refresh_handle;
} gatt_scan_params_device_data_t;

/* This structure is made public to application as application is responsible for managing resources 
* for the elements of this structure. The data elements are indented to use by Scan Params Client lib only. 
* Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
* may cause undesired behaviour of this library functionalities
*/
typedef struct _gatt_scan_params_client_t
{
    TaskData lib_task;                  /*! Lib Task*/
    Task app_task;                      /*!Application Registered Task */
    uint16 scan_interval_window_handle; /*! Discovered Scan Interval window handle */
    uint16 scan_refresh_handle;         /*! Discovered Scan refresh handle */
    uint16 scan_interval;               /* Scan Interval value */
    uint16 scan_window;                 /* Scan window value */
}_gatt_scan_params_client_t;

/*! @brief GATT SCAN PARAMS Client [GSPC]Library Instance.
 */
typedef struct _gatt_scan_params_client_t GSPC_T;

/*! @brief Enumeration of messages a client task may receive from the client library.
 */
typedef enum __gatt_scan_params_message_id
{
    GATT_SCAN_PARAMS_CLIENT_INIT_CFM = GATT_SCAN_PARAMS_CLIENT_MESSAGE_BASE,    /* 00 */

    /* Library message limit */
    GATT_SCAN_PARAMS_CLIENT_MESSAGE_TOP
} gatt_scan_params_message_id_t;

/*!
    @brief Enumeration for Scan Parameters client status code  
*/
typedef enum
{
    gatt_scan_params_client_status_success,             /* Request was success */
    gatt_scan_params_client_status_invalid_param,       /* Request failed because of invalid parameters */
    gatt_scan_params_client_status_failed               /* Request has been failed */
}gatt_scan_params_client_status;

/*!
    @brief Parameters used by the Initialisation API, valid value of these  parameters are must for library initialisation  
*/
typedef struct
{
     uint16 cid;                /*!Connection ID of the GATT connection on which the server side Scan Parameters service need to be accessed*/
     uint16 start_handle;       /*! The first handle of Scan Parameter service need to be accessed*/
     uint16 end_handle;         /*!The last handle of Scan Parameter service need to be accessed */
     uint16 scan_interval;      /*! The application supplied scan interval to be sent to remote scan server */
     uint16 scan_window;        /*! The application supplied scan window to be sent to remote scan server */
} GATT_SCAN_PARAMS_CLIENT_INIT_PARAMS_T;

/*!
    @brief Initialises the Scan Parameters Client Library.
     Initialize Scan Parameters client library handles, It starts finding out the characteristic handles of Scan Parameters service.
     Once the initialisation has been completed, GATT_SCAN_PARAMS_CLIENT_INIT_CFM will be received with 
     status as enumerated as gatt_scan_params_client_status.'gatt_scan_params_client_status_success' has to 
     be considered initialisation of library is done successfully and all the required charecteristics has been found out

     NOTE:This interface need to be invoked for every new gatt connection when the client wish to use 
     Scan Parameters client library  

    @param appTask The Task that will receive the messages sent from this Scan Parameters client library.
    @param scan_params_client A valid area of memory that the service library can use.Must be of at least the size of GSPC_T
    @param client_init_params as defined in GATT_SCAN_PARAMS_CLIENT_INIT_PARAMS_T , it is must all the parameters are valid
                The memory allocated for GATT_SCAN_PARAMS_CLIENT_INIT_PARAMS_T can be freed once the API returns.
    @param device_data Persistent data for the device
    @return TRUE if success, FALSE otherwise
*/
bool GattScanParamsClientInit(Task appTask ,
                                GSPC_T *const scan_params_client,
                                const GATT_SCAN_PARAMS_CLIENT_INIT_PARAMS_T *const client_init_params,
                                const gatt_scan_params_device_data_t *device_data);

/*!@brief Scan Parameters client library initialisation confirmation 
*/
typedef struct __GATT_SCAN_PARAMS_CLIENT_INIT_CFM
{
    const GSPC_T *scan_params_client;            /*! Reference structure for the instance */
    uint16 scan_interval_window_handle;          /*! Scan interval window handle */
    uint16 scan_refresh_handle;                  /*! Scan refresh handle */
    gatt_scan_params_client_status status;       /*! status as per gatt_scan_params_client_status */
} GATT_SCAN_PARAMS_CLIENT_INIT_CFM_T;


/*!
    @brief This API is used to set the Scan Interval Window on Scan Parameters service of server side .
    
    NOTE:GATT_SCAN_PARAMS_CLIENT_INIT_CFM_T should return gatt_scan_params_client_status_success before using this API,
    eles interface will fail

    @param scan_params_client Memory area used for initiating Scan Params client, used in GattScanParamsClientInit() .
    @param scan_interval  The scan interval to be sent to remote scan server 
    @param scan_window  The scan window to be sent to remote scan server 
    
    @return TRUE if operation was succesfull. FALSE otherwise.

*/
bool GattScanParamsSetIntervalWindow(GSPC_T *scan_params_client,
                                        uint16 scan_interval, uint16 scan_window);

/*!
    @brief When a GATT connection is removed, the application must remove the client service instance 
    This is the clean up routine as a result of calling the GattScanParamsClientInit API. That is,
    the GattScanParamsClientInit API is called when a connection is made, and the GattScanParamsClientDeInit is called 
    when the connection is removed.

    @param scan_params_client The client instance that was passed into the GattScanParamsClientInit API.
    
    @return TRUE if operation was succesfull. FALSE otherwise..

*/
bool GattScanParamsClientDeInit(GSPC_T *scan_params_client);

#endif /* GATT_SCAN_PARAMS_CLIENT_H */
