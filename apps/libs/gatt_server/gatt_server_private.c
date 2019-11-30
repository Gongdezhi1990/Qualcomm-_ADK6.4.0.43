/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_private.c

DESCRIPTION
    Contains private functions used across the GATT Server library.
    
NOTES

*/

/* Firmware headers */
#include <stdlib.h>

/* Gatt server headers */
#include "gatt_server_private.h"


/******************************************************************************/
bool gattServerSetGattServerTask(GGATTS *gatt_server, void(*handler)(Task, MessageId, Message))
{
    if (gatt_server)
    {
        gatt_server->lib_task.handler = handler;
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
Task gattServerGetGattServerTask(GGATTS *gatt_server)
{
    if (gatt_server)
    {
        return &gatt_server->lib_task;
    }
    return NULL;
}


/******************************************************************************/
bool gattServerSetClientTask(GGATTS *gatt_server, Task app_task)
{
    if (gatt_server)
    {
        gatt_server->app_task = app_task;
        return TRUE;
    }
    return FALSE;
}
