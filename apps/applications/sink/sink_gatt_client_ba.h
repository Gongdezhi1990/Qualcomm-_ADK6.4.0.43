/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client_ba.h

DESCRIPTION
    Routines to handle the GATT BroadcastAudio Client.
*/

#ifndef _SINK_GATT_CLIENT_BA_H_
#define _SINK_GATT_CLIENT_BA_H_


#ifndef ENABLE_BROADCAST_AUDIO
#undef GATT_BA_CLIENT
#else
#define GATT_BA_CLIENT
#endif


#include <csrtypes.h>
#include <message.h>

#include <gatt_broadcast_client.h>


 /*!
 * Macro to check the UUID of BA 
 */
#ifdef GATT_BA_CLIENT
 #define CHECK_BA_SERVICE_UUID(uuid) (uuid[0] == 0x7905f431u) && \
                    (uuid[1] == 0xb5ce4e99u) && \
                    (uuid[2] == 0xa40f4b1eu) && \
                    (uuid[3] == 0x122d00d0u)
#else
 #define CHECK_BA_SERVICE_UUID(uuid) (FALSE)
#endif
        
/****************************************************************************
NAME    
    sinkGattBAClientSetupAdvertisingFilter
    
DESCRIPTION
    Adds BA service to the advertising filter
*/    
#ifdef GATT_BA_CLIENT
void sinkGattBAClientSetupAdvertisingFilter(void);
#else
#define sinkGattBAClientSetupAdvertisingFilter() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkGattBAClientAddService
    
DESCRIPTION
    Adds BroadcastAudio to list of client connection service.
    
PARAMETERS
    cid             The connection ID
    start           The start handle of the BA service
    end             The end handle of the BA service
    
RETURNS    
    TRUE if the ANCS service was successfully added, FALSE otherwise.
*/
#ifdef GATT_BA_CLIENT
bool sinkGattBAClientAddService(uint16 cid, uint16 start, uint16 end);
#else
#define sinkGattBAClientAddService(cid, start, end) (FALSE)
#endif

/****************************************************************************
NAME    
    sinkGattBAClientRemoveService
    
DESCRIPTION
    Removes the BA service associated with the connection ID.
    
PARAMETERS
    gbsc           The BA client pointer
*/
#ifdef GATT_BA_CLIENT
void sinkGattBAClientRemoveService(GBSC *gbsc);
#else
#define sinkGattBAClientRemoveService(gbsc) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattBAClientMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Client Task library
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    message The message payload
    
*/

#ifdef GATT_BA_CLIENT
void sinkGattBAClientMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattBAClientMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattAncsClientGetSize
    
DESCRIPTION
    Returns the required size of ANCS client if enabled.

*/
#ifdef GATT_BA_CLIENT
#define sinkGattBAClientGetSize(void) (sizeof(GBSC))
#else
#define sinkGattBAClientGetSize(void) (0)
#endif


#endif /* _SINK_GATT_CLIENT_BA_H_ */
