/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_indication.h
    
DESCRIPTION
    Contains the "GATT Server" routines that handle GATT access requests.
    
NOTES
    
*/


#ifndef GATT_SERVER_INDICATION_H_
#define GATT_SERVER_INDICATION_H_


/* Firmware includes */
#include <csrtypes.h>

/* External lib headers */
#include "gatt_manager.h"


/*******************************************************************************
NAME
    gattServerHandleGattManagerIndicationCfm
    
DESCRIPTION
    Handle GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM messages.
    
PARAMETERS
    gatt_task   The "GATT Server" task.
    cfm         Pointer to GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM message to handle
    
RETURN
    TRUE if the message was handled, FALSE otherwise.
*/
bool gattServerHandleGattManagerIndicationCfm(Task gatt_task, const GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM_T * cfm);


#endif
