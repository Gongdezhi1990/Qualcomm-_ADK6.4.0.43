/****************************************************************************
Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client_ancs.h

DESCRIPTION
    Routines to handle the GATT ANCS Client.
*/

#ifndef _SINK_GATT_CLIENT_ANCS_H_
#define _SINK_GATT_CLIENT_ANCS_H_

#ifndef GATT_ENABLED
#undef GATT_ANCS_CLIENT
#endif

#include <csrtypes.h>
#include <message.h>
#include <gatt_apple_notification_client.h>

 /*!
 * Macro to check the UUID of ANCS, 7905F431-B5CE-4E99-A40F-4B1E122D00D0
 */
#ifdef GATT_ANCS_CLIENT
 #define CHECK_ANCS_SERVICE_UUID(uuid) (uuid[0] == 0x7905f431u) && \
                    (uuid[1] == 0xb5ce4e99u) && \
                    (uuid[2] == 0xa40f4b1eu) && \
                    (uuid[3] == 0x122d00d0u)
#else
 #define CHECK_ANCS_SERVICE_UUID(uuid) (FALSE)
#endif
        
/****************************************************************************
NAME    
    sinkGattAncsClientSetupAdvertisingFilter
    
DESCRIPTION
    Adds ANCS service to the advertising filter
*/    
#ifdef GATT_ANCS_CLIENT
void sinkGattAncsClientSetupAdvertisingFilter(void);
#else
#define sinkGattAncsClientSetupAdvertisingFilter() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkGattAncsClientAddService
    
DESCRIPTION
    Adds Apple Notification Center Service to list of client connection service.
    
PARAMETERS
    cid             The connection ID
    start           The start handle of the ANCS service
    end             The end handle of the ANCS service
    
RETURNS    
    TRUE if the ANCS service was successfully added, FALSE otherwise.
*/
#ifdef GATT_ANCS_CLIENT
bool sinkGattAncsClientAddService(uint16 cid, uint16 start, uint16 end);
#else
#define sinkGattAncsClientAddService(cid, start, end) (FALSE)
#endif

/****************************************************************************
NAME    
    sinkGattAncsClientRemoveService
    
DESCRIPTION
    Removes the ANCS service associated with the connection ID.
    
PARAMETERS
    gancs           The ANCS client pointer
*/
#ifdef GATT_ANCS_CLIENT
void sinkGattAncsClientRemoveService(GANCS *gancs);
#else
#define sinkGattAncsClientRemoveService(gancs) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattAncsClientMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Client Task library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
*/

#ifdef GATT_ANCS_CLIENT
void sinkGattAncsClientMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattAncsClientMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattAncsClientGetSize
    
DESCRIPTION
    Returns the required size of ANCS client if enabled.

*/
#ifdef GATT_ANCS_CLIENT
#define sinkGattAncsClientGetSize(void) (sizeof(GANCS))
#else
#define sinkGattAncsClientGetSize(void) (0)
#endif

/*******************************************************************************
NAME
    sinkGattAncsClientDisableNotificationsByDefault

DESCRIPTION
    Notifications will be disabled during client initialisation
*/
#ifdef GATT_ANCS_CLIENT
void sinkGattAncsClientDisableNotificationsByDefault(void);
#else
#define sinkGattAncsClientDisableNotificationsByDefault() ((void)(0))
#endif

#endif /* _SINK_GATT_CLIENT_ANCS_H_ */
