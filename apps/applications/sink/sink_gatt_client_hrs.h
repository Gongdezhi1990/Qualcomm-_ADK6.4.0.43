/*******************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_client_hrs.h

DESCRIPTION
    Routines to handle messages sent from the GATT Heart rate client task.
    
NOTES

*/

#ifndef _SINK_GATT_CLIENT_HRS_H_
#define _SINK_GATT_CLIENT_HRS_H_


#ifndef GATT_ENABLED
#undef GATT_HRS_CLIENT
#endif

#include <csrtypes.h>
#include <message.h>

#include <gatt_heart_rate_client.h>

/****************************************************************************
NAME    
    sinkGattHrsClientSetupAdvertisingFilter
    
DESCRIPTION
    Adds Heart rate service to the advertising filter
*/    
#ifdef GATT_HRS_CLIENT
void sinkGattHrsClientSetupAdvertisingFilter(void);
#else
#define sinkGattHrsClientSetupAdvertisingFilter() ((void)(0))
#endif

/****************************************************************************
NAME    
    sinkGattHrsClientAddService
    
DESCRIPTION
    Adds Heart rate client connection service.
    
PARAMETERS
    cid              The connection ID
    start           The start handle of the Heart Rate Service
    end             The end handle of the Heart Rate Service
    
RETURNS    
    TRUE if the Heart Rate Service was successfully added, FALSE otherwise.
*/
#ifdef GATT_HRS_CLIENT
bool sinkGattHrsClientAddService(uint16 cid, uint16 start, uint16 end);
#else
#define sinkGattHrsClientAddService(cid, start, end) (FALSE)
#endif


/****************************************************************************
NAME    
    sinkGattHrsClientRemoveService
    
DESCRIPTION
    Removes the Heart Rate Service
    
PARAMETERS
    ghrsc           The Heart rate service client pointer
*/
#ifdef GATT_HRS_CLIENT
void sinkGattHrsClientRemoveService(GHRSC_T *ghrsc);
#else
#define sinkGattHrsClientRemoveService(ghrsc) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattHrsClientMsgHandler
    
DESCRIPTION
    Handle messages from the GATT Client Task library
    
PARAMETERS
    task          The task the message is delivered
    id              The ID for the GATT message
    message     The message payload
    
*/

#ifdef GATT_HRS_CLIENT
void sinkGattHrsClientMsgHandler(Task task, MessageId id, Message message);
#else
#define sinkGattHrsClientMsgHandler(task, id, message) ((void)(0))
#endif

/*******************************************************************************
NAME
    sinkGattHrsClientEnabled
    
DESCRIPTION
    Returns if HRS client is enabled.
*/
#ifdef GATT_HRS_CLIENT
#define sinkGattHrsClientEnabled(void) (TRUE)
#else
#define sinkGattHrsClientEnabled(void) (FALSE)
#endif

/*******************************************************************************
NAME
    sinkGattHrsClientGetSize
    
DESCRIPTION
    Returns the required size of HRS client if enabled.

*/
#ifdef GATT_HRS_CLIENT
#define sinkGattHrsClientGetSize(void) (sizeof(GHRSC_T))
#else
#define sinkGattHrsClientGetSize(void) (0)
#endif

#endif /* _SINK_GATT_CLIENT_HRS_H_ */


