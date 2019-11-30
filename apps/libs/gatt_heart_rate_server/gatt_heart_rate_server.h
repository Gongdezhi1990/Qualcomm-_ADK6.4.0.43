/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

/*!
@file    
@brief   Header file for the GATT heart rate sensor library.

        This file provides documentation for the GATT heart rate sensor library
        API (library name: gatt_heart_rate_server).
*/

#ifndef GATT_HEART_RATE_SERVER_H
#define GATT_HEART_RATE_SERVER_H


#include <csrtypes.h>
#include <message.h>

#include <library.h>

#include "gatt_manager.h"

/*! @brief Heart Rate Sensor library  data structure type .
 */

/* This structure is made public to application as application is responsible for managing resources 
* for the elements of this structure. The data elements are indented to use by Heart Rate Sensor lib only. 
* Application SHOULD NOT access (read/write) any elements of this library structure at any point of time and doing so  
* may cause undesired behaviour of this library functionalities
*/
typedef struct _gatt_hr_server_t
{
    TaskData lib_task;
    Task app_task;
}_gatt_hr_server_t;

/*! @brief GATT Heart Rate Sensor[GHRS]Library Instance.
 */
typedef struct  _gatt_hr_server_t GHRS_T;

/*! @brief Contents of the GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message that is sent by the library,
    due to a read of the Heart rate measurement client configuration characteristic.
 */
typedef struct __GATT_HR_SERVER_READ_CLIENT_CONFIG_IND
{
    const GHRS_T *hr_sensor;    /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
} GATT_HR_SERVER_READ_CLIENT_CONFIG_IND_T;

/*! @brief Contents of the GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND message that is sent by the library,
    due to a write of the  Heart rate measurement client configuration characteristic.
 */
typedef struct __GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND
{
    const GHRS_T *hr_sensor;    /*! Reference structure for the instance  */
    uint16 cid;                 /*! Connection ID */
    uint16 config_value;        /*! Client Configuration value to be written */
} GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND_T;

/*! @brief Enumeration of messages an application task can receive from the Heart rate sensor library.
 */
typedef enum
{
    /* Server messages */
    GATT_HR_SERVER_READ_CLIENT_CONFIG_IND = GATT_HR_SERVER_MESSAGE_BASE, /* 00 */
    GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND,                              /* 01 */
    
    /* Library message limit */
    GATT_HR_SERVER_MESSAGE_TOP
} gatt_hr_server_message_id_t;

/*!
    @brief Initialises the Heart Rate Sensor Library.

    @param appTask The Task that will receive the messages sent from this Heart Rate service library.
    @param hr_sensor A valid area of memory that the Heart rate sensor library can use.Must be of at least the size of GHRS_T
    @param start_handle This indicates the start handle of the HR Sensor service
    @param end_handle This indicates the end handle of the HR Sensor service
    
    @return TRUE if success, FALSE otherwise.

*/
bool GattHrServerInit(Task appTask, GHRS_T *const hr_sensor, uint16 start_handle, uint16 end_handle);

/*!
    @brief This API is used to return a Heart rate Measurerment client configuration value to the library when a 
    GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message is received.

    @param hr_sensor The pointer that was in the payload of the GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message.
    @param cid The connection identifier from the GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message.
    @param client_config The client configuration to return to the library
    
    @return TRUE if success, FALSE otherwise

*/
bool GattHrServerReadClientConfigResponse(const GHRS_T *hr_sensor, uint16 cid, uint16 client_config);

/*!
    @brief This API is used to notify a remote Heart rate collector of the Heart rate measurement value. 
    This will only be allowed if notifications have been enabled by the remote device.

    @param hr_sensor The instance pointer that was passed into the GattBatteryServerInit API.
    @param cid The connection identifier from the GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message.
    @param hrm_length The length of the heart rate mesaurement value.
    @param hrm_data The heart rate measurement value to send in the notification. The heart rate measurement
    data is as follows
     
     *  --------------------------------------
     *  | Flags      |   HR meas Value |  RR Intervals -----------  
     *  --------------------------------------
     *      Flags field: 
     *              Bit 0: Heart Rate Value Format bit, 0 - UINT8 Format, 1 - UINT16 Format
     *              Bit 1: Sensor Contact Status bit, 0 - No or Poor Contact, 1 - In contact
     *              Bit 2: Sensor Contact Support bit, 0 - Sensor Contact feature is supported, 1 - Sensor Contact feature not supported
     *              Bit 3: Energy Expended Status bit, 0 - Energy Expended field not present, 1 - Energy Expended field is present
     *              Bit 4: RR-Interval bit, 0 - RR-Interval values are not present, 1 - RR-Interval values present
     *              Bits 5-7: RFU (Resereved for future use must be set to 0)
     *
     *      Heart Rate Measurement Value Field:
     *              <= 255 bpm if Heart Rate Value Format bit set to 0, >255 bpm Heart Rate Value Format bit set to 1
     *      
     *      RR-Interval Field:
     *              One or mor RR-Interval values if  RR-Interval bit set to 1.
     *
    
    @return TRUE if success, FALSE otherwise

*/
bool GattHrServerSendNotification(const GHRS_T *hr_sensor, uint16 cid, uint16 hrm_length, uint8 *hrm_data);

#endif /* GATT_HEART_RATE_SERVER_H */

