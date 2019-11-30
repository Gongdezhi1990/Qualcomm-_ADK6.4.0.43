/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_rscs.h

DESCRIPTION
    Routines to handle messages sent from the GATT Running Speed and Cadence Server Task.
    
NOTES

*/

#ifndef _SINK_GATT_SERVER_RSC_SENSOR_H_
#define _SINK_GATT_SERVER_RSC_SENSOR_H_

#ifdef ACTIVITY_MONITORING
#ifndef GATT_ENABLED
#undef GATT_RSCS_SERVER
#endif


#include <gatt_running_speed_cadence_server.h>

#include <csrtypes.h>
#include <message.h>

/* Running Speed and Cadence notification update time as per spec the Running Speed and Cadence measurements 
    shall be notified for every one second.
*/
#define GATT_SERVER_RSC_UPDATE_TIME    1000

/* This timeout is to stop the Running Speed and Cadence notifications which is simulated within the application
    Note: This is to be removed when actual sensor implementation is in place.
    
*/
#define STOP_RSC_NOTIFICATION_TIME    10000

/* Max RSC data that can be send in one ATT notification */
#define MAX_ATT_RSC_NOTI_LEN                               (20)

/* Running Speed and Cadence Measurement Flags Field attributes */

#define INST_STRIDE_LENGTH_PRESENT  1 << 0
#define TOTAL_DISTANCE_PRESENT      1 << 1
#define WALK_OR_RUN_PRESENT         1 << 2

#define STEP_COUNT_PRESENT          1 << 0
#define CONTEXT_PRESENT             1 << 1

/* Extract low order byte of 16-bit */
#define LE8_L(x)                       ((x) & 0xff)

/* Extract high order byte of 16-bit */
#define LE8_H(x)                       (((x) >> 8) & 0xff)

#ifdef GATT_RSCS_SERVER
#define sinkGattRSCServerGetSize() sizeof(GRSCS_T)
#else
#define sinkGattRSCServerGetSize() 0
#endif

/*******************************************************************************
NAME
    sinkGattRSCServerInitialise
    
DESCRIPTION
    Initialize RSCS server task.
    
PARAMETERS
    ptr - pointer to allocated memory to store server task's RSC data.
    
RETURNS
    TRUE if the RSCS server task was initialized, FALSE otherwise.
*/
#ifdef GATT_RSCS_SERVER
bool sinkGattRSCServerInitialise(uint16 **ptr);
#else
/* This is set to TRUE if the server is not defined, so as not to block the flow
 * of execution for initialiseOptionalServerTasks in sink_gatt_init.c. If it was
 * false, that function would return prematurely denoting an error in
 * initialization instead of merely skipping an undefined service. */
#define sinkGattRSCServerInitialise(ptr) (TRUE)
#endif

/*******************************************************************************
NAME
    sinkBleNotifyRSCMeasurements
    
DESCRIPTION
    Handles notifying the Running Speed and Cadence measurements every second when 
    BLE_INTERNAL_MESSAGE_RSC_READ_TIMER internal message is received.
    
PARAMETERS
    None
    
RETURNS
    None.
*/
#ifdef GATT_RSCS_SERVER
void sinkBleNotifyRSCMeasurements(void);
#else
#define sinkBleNotifyRSCMeasurements() ((void)(0)) 
#endif

/*******************************************************************************
NAME
    sinkGattRSCServerMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Running Speed and Cadence Service library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
RETURNS
    void
*/
#ifdef GATT_RSCS_SERVER
void sinkGattRSCServerMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattRSCServerMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattRSCServiceEnabled
    
DESCRIPTION
    Returns whether RSC service is enabled or not.
*/
#ifdef GATT_RSCS_SERVER
#define sinkGattRSCServiceEnabled(void) (TRUE)
#else
#define sinkGattRSCServiceEnabled(void) (FALSE)
#endif

#endif

#endif /* _SINK_GATT_SERVER_RSC_SENSOR_H_ */

