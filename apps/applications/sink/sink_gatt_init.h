/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_init.h

DESCRIPTION
    Contains GATT initialisation routines.
    
NOTES

*/

#ifndef _SINK_GATT_INIT_H_
#define _SINK_GATT_INIT_H_


#include "sink_gatt.h"

#include <csrtypes.h>


/*******************************************************************************
NAME
    sinkGattInitInitialiseDevice
    
DESCRIPTION
    This function is used to setup GATT for the device.
    If no server tasks are required, GATT Manager shall be initialised and no
    database will be used. Otherwise a bitmask needs to select which server
    tasks are required and the database shall be built dynamically based on this
    requirement.
    
PARAMETERS
    None
    
RETURNS
    TRUE if the initialisation was successful, FALSE otherwise
*/
#ifdef GATT_ENABLED
bool sinkGattInitInitialiseDevice(void);
#else
#define sinkGattInitInitialiseDevice() (TRUE)


#endif

#endif
