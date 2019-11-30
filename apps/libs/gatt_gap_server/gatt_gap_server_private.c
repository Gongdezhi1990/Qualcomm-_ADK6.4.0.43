/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_private.c

DESCRIPTION
    Contains private functions used across the GAP Server library.
    
NOTES

*/

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* Gatt server headers */
#include "gatt_gap_server_private.h"


/******************************************************************************/
bool gattGapServerSetGapServerTask(GGAPS *gap_server, void(*handler)(Task, MessageId, Message))
{
    if (gap_server)
    {
        gap_server->lib_task.handler = handler;
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
Task gattGapServerGetGapServerTask(GGAPS *gap_server)
{
    if (gap_server)
    {
        return &gap_server->lib_task;
    }
    return NULL;
}


/******************************************************************************/
bool gattGapServerSetClientTask(GGAPS *gap_server, Task app_task)
{
    if (gap_server)
    {
        gap_server->app_task = app_task;
        return TRUE;
    }
    return FALSE;
}
