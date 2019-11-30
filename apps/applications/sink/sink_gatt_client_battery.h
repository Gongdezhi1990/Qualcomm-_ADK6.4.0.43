/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/



#ifndef _SINK_GATT_CLIENT_BATTERY_H_
#define _SINK_GATT_CLIENT_BATTERY_H_


#ifndef GATT_ENABLED
#undef GATT_BATTERY_CLIENT
#endif


#include <gatt_battery_client.h>
#include "sink_powermanager.h"

#include <csrtypes.h>
#include <message.h>

/****************************************************************************
NAME    
    sinkGattBasClientSetupAdvertisingFilter
    
DESCRIPTION
    Adds Battery service to the advertising filter
*/    
#ifdef GATT_BATTERY_CLIENT
void gattBatteryClientSetupAdvertisingFilter(void);
#else
#define gattBatteryClientSetupAdvertisingFilter() ((void)(0))
#endif

/****************************************************************************
NAME    
    gattBatteryClientInit
    
DESCRIPTION
    Initialisation of battery client service
*/    
#ifdef GATT_BATTERY_CLIENT
void gattBatteryClientInit(void);
#else
#define gattBatteryClientInit() ((void)(0))
#endif

        
/****************************************************************************
NAME    
    gattBatteryClientAddService
    
DESCRIPTION
    Adds battery to list of client connection service.
    
PARAMETERS
    cid             The connection ID
    start           The start handle of the battery service
    end             The end handle of the battery service
    
RETURNS    
    TRUE if the battery service was successfully added, FALSE otherwise.
*/
#ifdef GATT_BATTERY_CLIENT
bool gattBatteryClientAddService(uint16 cid, uint16 start, uint16 end);
#else
#define gattBatteryClientAddService(cid, start, end) (FALSE)
#endif


/****************************************************************************
NAME    
    gattBatteryClientRemoveService
    
DESCRIPTION
    Removes the battery service associated with the connection ID.
    
PARAMETERS
    gbasc           The battery client pointer
    cid             The connection ID
*/
#ifdef GATT_BATTERY_CLIENT
void gattBatteryClientRemoveService(GBASC *gbasc, uint16 cid);
#else
#define gattBatteryClientRemoveService(gbasc, cid) ((void)(0))
#endif


/*******************************************************************************
NAME
    gattBatteryClientMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Client Task library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
*/

#ifdef GATT_BATTERY_CLIENT
void gattBatteryClientMsgHandler(Task task, MessageId id, Message message);
#else
#define gattBatteryClientMsgHandler(task, id, message) ((void)(0))
#endif


/*******************************************************************************
NAME
    gattBatteryClientGetCachedLevel
    
DESCRIPTION
    Gets the cached battery level of a remote device when in client role.
    
PARAMETERS
    cid    connection id of the connected remote server

RETURNS
    The cached battery level of the remote server.

*/
#ifdef GATT_BATTERY_CLIENT
uint8 gattBatteryClientGetCachedLevel(uint16 cid);
#else
#define gattBatteryClientGetCachedLevel(cid) (BATTERY_LEVEL_INVALID & 0xFF)
#endif


/*******************************************************************************
NAME
    sinkGattBatteryClientEnabled
    
DESCRIPTION
    Returns if battery client is enabled.

*/
#ifdef GATT_BATTERY_CLIENT
#define sinkGattBatteryClientEnabled(void) (TRUE)
#else
#define sinkGattBatteryClientEnabled(void) (FALSE)
#endif

/*******************************************************************************
NAME
    sinkGattBatteryClientGetSize
    
DESCRIPTION
    Returns the required size of battery client if enabled.

*/
#ifdef GATT_BATTERY_CLIENT
#define sinkGattBatteryClientGetSize(void) (sizeof(GBASC))
#else
#define sinkGattBatteryClientGetSize(void) (0)
#endif


#endif /* _SINK_GATT_BATTERY_CLIENT_H_ */
