/* Copyright (c) 2016 - 2018 Qualcomm Technologies International, Ltd. */
/*    */
/****************************************************************************

FILE NAME
    sink_accessory.h

DESCRIPTION
    Support for Accessory. 

    This feature is included in  an add-on installer 

*/
#ifndef _SINK_ACCESSORY_H_
#define _SINK_ACCESSORY_H_

#include <connection.h>
#include "sink_accessory.h"


/****************************************************************************
NAME
    sinkAccesoryMsgHandler

DESCRIPTION
    Handle messages from an Accessory library.
*/
#define sinkAccessoryMsgHandler(task, id, message) ((void) 0)


/****************************************************************************
NAME
    sinkAccesoryIsEnabled

DESCRIPTION
    Returns TRUE if Accessory is enabled. FALSE otherwise.
*/
#define sinkAccesoryIsEnabled() (FALSE)


/****************************************************************************
NAME
    sinkAccessoryInit

DESCRIPTION
    Initialisation of Accessory.

RETURNS
     None
*/
#define sinkAccessoryInit() ((void)(0))


/****************************************************************************
NAME
    sinkAccessoryGetDeviceInfo

DESCRIPTION
     Function to get Accessory Device info based on device index.

RETURNS
     None
*/
#define sinkAccessoryGetDeviceInfo(info, device_index) ((void)(0))


/****************************************************************************
NAME
    sinkAccessoryGetButtonState

DESCRIPTION
     Function to get Accessory Device button state.

RETURNS
     Button state pressed or not.
*/
#define sinkAccessoryGetButtonState() (0)


/****************************************************************************
NAME
    sinkAccessorySetButtonState

DESCRIPTION
     Function to Set Accessory Device button state.

RETURNS
     None
*/
#define sinkAccessorySetButtonState() ((void)(0))


/****************************************************************************
NAME
    sinkAccessoResetButtonState

DESCRIPTION
     Function to Reset Accessory Device button state.

RETURNS
     None
*/
#define sinkAccessoryResetButtonState() ((void)(0))


/****************************************************************************
NAME
    sinkAccessoryGetTransport

DESCRIPTION
     Function to get current Transport type.

RETURNS
     Transport type whether accessory uses BT or USB transport.
*/
#define sinkAccessoryGetTransport() ((void)(0))


/******************************************************************************
NAME
    sinkAccessoryConnectRequest

DESCRIPTION
    Handles the Accessory connection

RETURNS
     None
*/
#define sinkAccessoryConnectRequest(pAddr) ((void)(0))


/******************************************************************************
NAME
    sinkAccessoryDisconnect

DESCRIPTION
     Disconnect all accessory links

RETURNS
     None
*/
#define sinkAccessoryDisconnect() ((void)(0))


/******************************************************************************
NAME
    sinkHandleAccessoryDisconnect

DESCRIPTION
     Disconnect accessory connection with the provided device
*/
#define sinkAccessoryDisconnectDevice(bdaddr) ((void)(0))


/******************************************************************************
NAME
    sinkAccessoryReportUserEvent

DESCRIPTION
    Handles the Accesory user event.

RETURNS
     None
*/
#define sinkAccessoryReportUserEvent(id) (FALSE)


/****************************************************************************
NAME
      sinkAccessoryConnectedToPeer

DESCRIPTION
    Checks if the accessory is connected to remote device

RETURNS
     TRUE if connected to remote device else FALSE.
*/
#define sinkAccessoryConnectedToPeer(pAddr) ((void)(0))


/****************************************************************************
NAME
      sinkAccessoryUpdateDeviceName

DESCRIPTION
    Updates the device name in accessory data structure

RETURNS
     None
*/
#define sinkAccessoryUpdateDeviceName(local_name_msg) ((void)(0))


/******************************************************************************
NAME
    sinkAccessoryVolumeRoute

DESCRIPTION
    Handles the HID Volume to device.

RETURNS
    TRUE if HID volume is routed to peer device
*/
#define sinkAccessoryVolumeRoute(direction) (FALSE)


/******************************************************************************
NAME
    sinkAccessoryHwInit

DESCRIPTION
    Initialises the coprocessor

RETURNS
    void
*/
#define sinkAccessoryHwInit() ((void) 0)

/*******************************************************************************
NAME
    sinkAccessoryLaunch

DESCRIPTION
    Launch an accessory-specific application

RETURNS
    void
*/
#define sinkAccessoryLaunch() ((void) 0)

#endif /* _SINK_ACCESSORY_H_ */
