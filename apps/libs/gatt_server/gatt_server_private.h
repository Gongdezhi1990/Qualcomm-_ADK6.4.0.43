/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_private.h
    
DESCRIPTION
    Contains private data and routines to the GATT Server library.
*/

#ifndef GATT_SERVER_PRIVATE_H_
#define GATT_SERVER_PRIVATE_H_


/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* GATT Server headers */
#include "gatt_server.h"


/*******************************************************************************
NAME
    gattServerSetGattServerTask
    
DESCRIPTION
    Setup the GATT server task data, this task is required to handle messages
    sent from external tasks to the GATT Server task.
    
PARAMETERS
    gatt_server Pointer to the GATT server run-data.
    handler     Pointer to a message handler.
    
RETURN
    TRUE if the task was set, FALSE otherwise.
*/
bool gattServerSetGattServerTask(GGATTS *gatt_server, 
                                 void(*handler)(Task, MessageId, Message));


/*******************************************************************************
NAME
    gattServerGetGattServerTask
    
DESCRIPTION
    Get the GATT Server library task.
    
PARAMETERS
    gatt_server Pointer to the GATT server run-data.
    
RETURN
    The GATT Servers library task.
*/
Task gattServerGetGattServerTask(GGATTS *gatt_server);


/*******************************************************************************
NAME
    gattServerSetClientTask
    
DESCRIPTION
    Set the GATT Servers client task, this is the task which owns the GATT
    Server instance.
    
PARAMETERS
    gatt_server Pointer to the GATT server run-data.
    app_task    The client task to set.
    
RETURN
    TRUE if the client task was set, FALSE otherwise.
*/
bool gattServerSetClientTask(GGATTS *gatt_server, Task app_task);


#endif
