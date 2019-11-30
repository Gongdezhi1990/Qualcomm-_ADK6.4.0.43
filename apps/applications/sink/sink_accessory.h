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

#ifdef ENABLE_IAP2
#include <ps.h>
#include <bdaddr.h>
#include <iap2.h>
#include "sink_volume.h"
#include "sink_debug.h"

#ifdef DEBUG_ACCESSORY
#define DEBUG_IAP2     
#endif

#ifdef DEBUG_IAP2
#define IAP2_DEBUG(x) DEBUG(x)
#else
#define IAP2_DEBUG(x) 
#endif

#define MAX_IAP2_CONNECTIONS    2

/* Fix size for maximum attribute */
#define IAP_MAX_SDP_ATTRIBUTE_LEN        81

/* Max payload size of a EAP packet */
#define MAX_EAP_PKT_SIZE        (256)

#define BLUETOOTH_HID_COMPONENT_BIT         (1UL << 29)

/* iAP2 link priority is used to identify different iAP2 links to
   AG devices using the order in which the devices were connected. 
*/
typedef enum
{
    /*! First iap2 link */
    iap2_primary_link,
    /*! Second iap2 link */
    iap2_secondary_link, 
    /*! Invalid iap2 link */
    iap2_invalid_link
} iap2_link_priority;

/* Accessory Connection state */
typedef enum
{
    /* accessory  idle */
    sink_accessory_state_idle,
    /* accessory connecting */
    sink_accessory_state_connecting,
    /* accessory connected */
    sink_accessory_state_connected,
    /* accessory disconnecting */
    sink_accessory_state_disconnecting
} sink_accessory_state_t;

/*!
    @brief Global data for Accessory.
*/
typedef struct __accessory_info
{
    /* Server Bluetooth address */
    bdaddr      peer_bd_addr;
    iap2_link   *link;
    uint16      data_session_id;
    unsigned    busy:1;
    unsigned    hid_over_iap2:1;
    unsigned    unused: 14;
    /* Device id for accessory connection */
    sink_accessory_state_t      device_state;
} accessory_info;

#endif

/****************************************************************************
NAME
    sinkAccesoryMsgHandler

DESCRIPTION
    Handle messages from an Accessory library.
*/
#ifdef ENABLE_IAP2
void sinkAccessoryMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkAccessoryMsgHandler(task, id, message) ((void) 0)
#endif


/****************************************************************************
NAME
    sinkAccesoryIsEnabled

DESCRIPTION
    Returns TRUE if Accessory is enabled. FALSE otherwise.
*/
#ifdef ENABLE_IAP2
#define sinkAccesoryIsEnabled()  TRUE
#else
#define sinkAccesoryIsEnabled() (FALSE)
#endif


/****************************************************************************
NAME
    sinkAccessoryInit

DESCRIPTION
    Initialisation of Accessory.

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessoryInit(void);
#else
#define sinkAccessoryInit() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAccessoryGetDeviceInfo

DESCRIPTION
     Function to get Accessory Device info based on device index.

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessoryGetDeviceInfo(accessory_info *info, int device_index);
#else
#define sinkAccessoryGetDeviceInfo(info, device_index) ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAccessoryGetButtonState

DESCRIPTION
     Function to get Accessory Device button state.

RETURNS
     Button state pressed or not.
*/
#ifdef ENABLE_IAP2
uint16 sinkAccessoryGetButtonState(void);
#else
#define sinkAccessoryGetButtonState() (0)
#endif


/****************************************************************************
NAME
    sinkAccessorySetButtonState

DESCRIPTION
     Function to Set Accessory Device button state.

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessorySetButtonState(void);
#else
#define sinkAccessorySetButtonState() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAccessoResetButtonState

DESCRIPTION
     Function to Reset Accessory Device button state.

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessoryResetButtonState(void);
#else
#define sinkAccessoryResetButtonState() ((void)(0))
#endif


/****************************************************************************
NAME
    sinkAccessoryGetTransport

DESCRIPTION
     Function to get current Transport type.

RETURNS
     Transport type whether accessory uses BT or USB transport.
*/
#ifdef ENABLE_IAP2
iap2_transport_t sinkAccessoryGetTransport(void);
#else
#define sinkAccessoryGetTransport() ((void)(0))
#endif


/******************************************************************************
NAME
    sinkAccessoryConnectRequest

DESCRIPTION
    Handles the Accessory connection

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessoryConnectRequest(bdaddr * pAddr);
#else
#define sinkAccessoryConnectRequest(pAddr) ((void)(0))
#endif


/******************************************************************************
NAME
    sinkAccessoryDisconnect

DESCRIPTION
     Disconnect all accessory links

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessoryDisconnect(void);
#else
#define sinkAccessoryDisconnect() ((void)(0))
#endif


/******************************************************************************
NAME
    sinkHandleAccessoryDisconnect

DESCRIPTION
     Disconnect accessory connection with the provided device
*/
#ifdef ENABLE_IAP2
void sinkAccessoryDisconnectDevice(const bdaddr *bd_addr);
#else
#define sinkAccessoryDisconnectDevice(bdaddr) ((void)(0))
#endif


/******************************************************************************
NAME
    sinkAccessoryReportUserEvent

DESCRIPTION
    Handles the Accesory user event.

RETURNS
     None
*/
#ifdef ENABLE_IAP2
bool sinkAccessoryReportUserEvent(uint16 id);
#else
#define sinkAccessoryReportUserEvent(id) (FALSE)
#endif


/****************************************************************************
NAME
      sinkAccessoryConnectedToPeer

DESCRIPTION
    Checks if the accessory is connected to remote device

RETURNS
     TRUE if connected to remote device else FALSE.
*/
#ifdef ENABLE_IAP2
bool sinkAccessoryConnectedToPeer(const bdaddr *pAddr);
#else
#define sinkAccessoryConnectedToPeer(pAddr) ((void)(0))
#endif


/****************************************************************************
NAME
      sinkAccessoryUpdateDeviceName

DESCRIPTION
    Updates the device name in accessory data structure

RETURNS
     None
*/
#ifdef ENABLE_IAP2
void sinkAccessoryUpdateDeviceName(const CL_DM_LOCAL_NAME_COMPLETE_T* local_name_msg);
#else
#define sinkAccessoryUpdateDeviceName(local_name_msg) ((void)(0))
#endif


/******************************************************************************
NAME
    sinkAccessoryVolumeRoute

DESCRIPTION
    Handles the HID Volume to device.

RETURNS
    TRUE if HID volume is routed to peer device
*/
#ifdef ENABLE_IAP2
bool sinkAccessoryVolumeRoute(volume_direction direction);
#else
#define sinkAccessoryVolumeRoute(direction) (FALSE)
#endif


/******************************************************************************
NAME
    sinkAccessoryHwInit

DESCRIPTION
    Initialises the coprocessor

RETURNS
    void
*/
#ifdef ENABLE_IAP2
void sinkAccessoryHwInit(void);
#else
#define sinkAccessoryHwInit() ((void) 0)
#endif

/*******************************************************************************
NAME
    sinkAccessoryLaunch

DESCRIPTION
    Launch an accessory-specific application

RETURNS
    void
*/
#ifdef ENABLE_IAP2
void sinkAccessoryLaunch(void);
#else
#define sinkAccessoryLaunch() ((void) 0)
#endif

#endif /* _SINK_ACCESSORY_H_ */
