/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_device_manager.h

DESCRIPTION
    Contains functions to manage remote devices used by the SWAT library
*/

#ifndef _SWAT_DEVICE_MANAGER_H_
#define _SWAT_DEVICE_MANAGER_H_

#include "swat_private.h"

/* external lib includes */
#include <bdaddr.h>


#define ERROR_MAX_DEVICES 0xFF  /* Error indicating the maximum number of devices have been added */


/****************************************************************************
FUNCTION:
    swatAddDevice
    
DESCRIPTION:
    Function to add the device with Bluetooth address *addr.
        If device is added, returns the device ID.
        If device cannot be added (Max devices exist), returns SWAT_ERROR.
*/
int swatAddDevice(const bdaddr *addr);


/****************************************************************************
FUNCTION:
    swatRemoveDevice
    
DESCRIPTION:
    Function to remove the device with ID *device_id*
        If device is removed, returns TRUE
        If device was not found in list, returns FALSE
*/
bool swatRemoveDevice(uint16 device_id);


/****************************************************************************
FUNCTION:
    swatFindDeviceFromBdaddr
    
DESCRIPTION:
    Function to return a pointer to the remoteDevice structure for 
    the device that has Bluetooth address *addr.
    If device is unknown to the SWAT library, returns NULL.
*/
remoteDevice* swatFindDeviceFromBdaddr (const bdaddr *addr);


/****************************************************************************
FUNCTION:
    swatFindDeviceFromSink
    
DESCRIPTION:
    Function to return a pointer to the remoteDevice structure for 
    the device connected to sink
    If device is unknown to the SWAT library, OR the sink is not valid,
    this function returns NULL.
*/
remoteDevice* swatFindDeviceFromSink(Sink sink);


/****************************************************************************
FUNCTION:
    swatFindLocalDeviceFromSink
    
DESCRIPTION:
    Function to return a pointer to the remoteDevice structure for 
    the device with a sink that matches *sink*
    If device is unknown to the SWAT library, this function returns NULL.
*/
remoteDevice* swatFindLocalDeviceFromSink(Sink sink);


#endif /* _SWAT_DEVICE_MANAGER_H_ */
