/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_dis.h

DESCRIPTION
    Routines to handle messages sent from the GATT Device Information Server Task.
    
NOTES

*/

#ifndef _SINK_GATT_SERVER_DIS_H_
#define _SINK_GATT_SERVER_DIS_H_


#ifndef GATT_ENABLED
#undef GATT_DIS_SERVER
#endif


#include <gatt_device_info_server.h>

#include <csrtypes.h>
#include <message.h>

/* DIS manufacturer name information maximum length. 

  * Manufacturer name string will truncates to max length, in case if user entries more than 31 characters. */
#define GATT_DIS_MAX_MANUF_NAME_LEN     31

#ifdef GATT_DIS_SERVER
#define sinkGattDeviceInfoServerGetSize() sizeof(gdiss_t)
#else
#define sinkGattDeviceInfoServerGetSize() 0
#endif

/*******************************************************************************
NAME
    sinkGattDeviceInfoServerInitialise
    
DESCRIPTION
    Initialise DIS server task.
    
PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if the DIS server task was initialised, FALSE otherwise.
*/
#ifdef GATT_DIS_SERVER
bool sinkGattDeviceInfoServerInitialise(uint16 **ptr);
#else
#define sinkGattDeviceInfoServerInitialise(ptr) (TRUE)
#endif

/*******************************************************************************
NAME
    sinkGattDeviceInfoServiceEnabled
    
DESCRIPTION
    Returns whether DeviceInfo service is enabled or not.
*/
bool sinkGattDeviceInfoServiceEnabled(void);

#endif /* _SINK_GATT_SERVER_DIS_H_ */
