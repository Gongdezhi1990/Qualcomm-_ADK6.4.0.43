/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_hrs.h

DESCRIPTION
    Routines to handle messages sent from the GATT Heart Rate Server Task.
    
NOTES

*/

#ifndef _SINK_GATT_SERVER_HR_SENSOR_H_
#define _SINK_GATT_SERVER_HR_SENSOR_H_


#ifndef GATT_ENABLED
#undef GATT_HRS_SERVER
#endif


#include <gatt_heart_rate_server.h>

#include <csrtypes.h>
#include <message.h>

/* Heart Rate notification update time as per spec the heart rate measurements 
    shall be notified for every one second.
*/
#define GATT_SERVER_HR_UPDATE_TIME    1000

/* This timeout is to stop the Heart Rate notifications which is simluated within the application
    Note: This is to be removed when actual sensor implementation is in place.
    
*/
#define STOP_HR_NOTIFICATION_TIME    10000

/* Max HR data that can be send in one ATT notification */
#define MAX_ATT_HR_NOTI_LEN                               (20)

/* HEART RATE MEASUREMENT FLAGS */

#define SENSOR_MEASUREVAL_FORMAT_UINT8   (0x00)
#define SENSOR_MEASUREVAL_FORMAT_UINT16  (0x01)

/* Sensor contact bit */

/* Sensor contact feature support (bit 2 of the flag) */
#define SENSOR_CONTACT_FEATURE_SUPPORTED (0x04)

/* Sensor contact bits when feature is supported (bit 1 - sensor contact bit)*/
#define SENSOR_NOT_IN_CONTACT \
                                       (SENSOR_CONTACT_FEATURE_SUPPORTED | 0x00)
#define SENSOR_IN_CONTACT\
                                       (SENSOR_CONTACT_FEATURE_SUPPORTED | 0x02)

/* RR-Interval status bit (Bit 4 of the flag) */
#define RR_INTERVAL_PRESENT             (0x10)

/* Heart Rate used as a base for HR Measurements */
#define HEART_RATE_IN_BPM               (78)

/* Dummy RR interval in milliseconds */
#define RR_INTERVAL_IN_MS              (0x1e0)

/* Extract low order byte of 16-bit */
#define LE8_L(x)                       ((x) & 0xff)

/* Extract low order byte of 16-bit */
#define LE8_H(x)                       (((x) >> 8) & 0xff)

#ifdef GATT_HRS_SERVER
#define sinkGattHeartRateServerGetSize() sizeof(GHRS_T)
#else
#define sinkGattHeartRateServerGetSize() 0
#endif

/*******************************************************************************
NAME
    sinkGattHeartRateServerInitialise
    
DESCRIPTION
    Initialise HRS server task.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the HRS server task was initialised, FALSE otherwise.
*/
#ifdef GATT_HRS_SERVER
bool sinkGattHeartRateServerInitialise(uint16 **ptr);
#else
#define sinkGattHeartRateServerInitialise(ptr) (TRUE)
#endif

/*******************************************************************************
NAME
    sinkBleNotifyHrMeasurements
    
DESCRIPTION
    Handles notifying the heart rate measurements every second when 
    BLE_INTERNAL_MESSAGE_HR_READ_TIMER internal message is recieved.
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_HRS_SERVER
void sinkBleNotifyHrMeasurements(void);
#else
#define sinkBleNotifyHrMeasurements() ((void)(0)) 
#endif

/*******************************************************************************
NAME
    sinkBleHRSensorNotInContact
    
DESCRIPTION
    While simulating the heart rate measurements are sent every 1 seconds for a period of  10 seconds  
    and upon expiry of timer this function is called to stop sending the heart rate measurements 
    which stops heart rate measurement timer and  indicate to remote device that sensor is no longer 
    in contact with the body.
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_HRS_SERVER
void sinkBleHRSensorNotInContact(void);
#else
#define sinkBleHRSensorNotInContact() ((void)(0)) 
#endif

/*******************************************************************************
NAME
    sinkBleHRSensorInConctact
    
DESCRIPTION
    Handles EventSysBleHrSensorInContact event, The Heart rate sensor contact detected, 
    start sending the Heart rate measurements.
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_HRS_SERVER
void sinkBleHRSensorInContact(void);
#else
#define sinkBleHRSensorInContact() ((void)(0)) 
#endif

/*******************************************************************************
NAME
    sinkGattHeartRateServerMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Heart Rate Service library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
RETURNS
    void
*/
#ifdef GATT_HRS_SERVER
void sinkGattHeartRateServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattHeartRateServerMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattHeartRateServiceEnabled
    
DESCRIPTION
    Returns whether HeartRate service is enabled or not.
*/
bool sinkGattHeartRateServiceEnabled(void);

/*----------------------------------------------------------------------------*
 *  NAME
 *      getHRMeasReading
 *
 *  DESCRIPTION
 *      This function formulates HR Measurement data in a format
 *      given by HR service specification.
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
#ifdef ACTIVITY_MONITORING
void getHRMeasReading(uint8 *p_hr_meas, uint8 *p_length, uint8 sensor_status);
#endif

#endif /* _SINK_GATT_SERVER_HR_SENSOR_H_ */

