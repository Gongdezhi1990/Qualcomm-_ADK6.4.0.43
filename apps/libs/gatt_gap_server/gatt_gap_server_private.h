/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_private.h
    
DESCRIPTION
    Contains private data and routines to the GAP Server library.
*/

#ifndef GATT_GAP_SERVER_PRIVATE_H_
#define GATT_GAP_SERVER_PRIVATE_H_


/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <stdlib.h>

/* GATT Server headers */
#include "gatt_gap_server.h"


/*******************************************************************************
NAME
    gattGapServerSetGapServerTask
    
DESCRIPTION
    Setup the GATT server task data, this task is required to handle messages
    sent from external tasks to the GATT Server task.
    
PARAMETERS
    gap_server  Pointer to the GATT server run-data.
    handler     Pointer to a message handler.
    
RETURN
    TRUE if the task was set, FALSE otherwise.
*/
bool gattGapServerSetGapServerTask(GGAPS *gap_server, 
                                    void(*handler)(Task, MessageId, Message));


/*******************************************************************************
NAME
    gattGapServerGetGapServerTask
    
DESCRIPTION
    Get the GAP Server library task.
    
PARAMETERS
    gap_server Pointer to the GAP server run-data.
    
RETURN
    The GAP Servers library task.
*/
Task gattGapServerGetGapServerTask(GGAPS *gap_server);


/*******************************************************************************
NAME
    gattGapServerSetClientTask
    
DESCRIPTION
    Set the GAP Servers client task, this is the task which owns the GAP
    Server instance.
    
PARAMETERS
    gap_server  Pointer to the GAP server run-data.
    app_task    The client task to set.
    
RETURN
    TRUE if the client task was set, FALSE otherwise.
*/
bool gattGapServerSetClientTask(GGAPS *gap_server, Task app_task);


#endif
