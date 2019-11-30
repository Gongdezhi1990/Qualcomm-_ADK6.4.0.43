/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_client_dis.h

DESCRIPTION
    Routines to handle messages sent from the GATT Device Information Service Client Task.
    
NOTES

*/

#ifndef _SINK_GATT_CLIENT_DIS_H_
#define _SINK_GATT_CLIENT_DIS_H_


#ifndef GATT_ENABLED
#undef GATT_DIS_CLIENT
#endif


#include <csrtypes.h>
#include <message.h>

#include <gatt_device_info_client.h>


/****************************************************************************
NAME    
    sinkGattDisClientSetupAdvertisingFilter
    
DESCRIPTION
    Adds Device Information service to the advertising filter
*/    
#ifdef GATT_DIS_CLIENT
void sinkGattDisClientSetupAdvertisingFilter(void);
#else
#define sinkGattDisClientSetupAdvertisingFilter() ((void)(0))
#endif

/****************************************************************************
NAME    
    
DESCRIPTION
    Adds Device Information Service client connection service.
    
PARAMETERS
    cid              The connection ID
    start           The start handle of the Device Information Service
    end             The end handle of the Device Information Service
    
RETURNS    
    TRUE if the Device Information Service was successfully added, FALSE otherwise.
*/
#ifdef GATT_DIS_CLIENT
bool sinkGattDisClientAddService(uint16 cid, uint16 start, uint16 end);
#else
#define sinkGattDisClientAddService(cid, start, end) (FALSE)
#endif

/****************************************************************************
NAME    
    sinkGattDISClientRemoveService
    
DESCRIPTION
    Removes the DIS service associated with the connection ID.
    
PARAMETERS
    gdisc           The DIS client pointer
    cid             The connection ID
*/
#ifdef GATT_DIS_CLIENT
void sinkGattDISClientRemoveService(GDISC *gdisc, uint16 cid);
#else
#define sinkGattDISClientRemoveService(gdisc, cid) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattDisClientMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Client Task library
    
PARAMETERS
    task     The task the message is delivered
    id       The ID for the GATT message
    message  The message payload
    
*/

#ifdef GATT_DIS_CLIENT
void sinkGattDisClientMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattDisClientMsgHandler(task, id, message) ((void)(0))
#endif


/*******************************************************************************
NAME
    sinkGattDisClientEnabled
    
DESCRIPTION
    Returns TRUE if DIS client is enabled.
*/
#ifdef GATT_DIS_CLIENT
#define sinkGattDisClientEnabled(void) (TRUE)
#else
#define sinkGattDisClientEnabled(void) (FALSE)
#endif

/*******************************************************************************
NAME
    sinkGattDisClientGetSize
    
DESCRIPTION
    Returns the required size of DIS client if enabled.

*/
#ifdef GATT_DIS_CLIENT
#define sinkGattDisClientGetSize(void) (sizeof(GDISC))
#else
#define sinkGattDisClientGetSize(void) (0)
#endif

#endif /* _SINK_GATT_CLIENT_DIS_H_ */
