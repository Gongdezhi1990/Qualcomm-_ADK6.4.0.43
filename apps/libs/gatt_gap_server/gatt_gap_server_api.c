/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_api.c

DESCRIPTION
    Contains the "GAP Server" API routines
    
NOTES

*/

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* GAP Server headers */
#include "gatt_gap_server_private.h"

#include "gatt_gap_server_api.h"


/******************************************************************************/
bool gattGapServerSendReadDeviceNameInd(GGAPS *gap_server, uint16 cid, uint16 offset)
{
    if (gap_server->app_task)
    {
        MAKE_GATT_GAP_SERVER_MESSAGE(GATT_GAP_SERVER_READ_DEVICE_NAME_IND);
        message->gap_server = gap_server;
        message->cid = cid;
        message->name_offset = offset;
        MessageSend(gap_server->app_task, GATT_GAP_SERVER_READ_DEVICE_NAME_IND, message);
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
bool gattGapServerSendReadAppearanceInd(const GGAPS *gap_server, uint16 cid)
{
    if (gap_server->app_task)
    {
        MAKE_GATT_GAP_SERVER_MESSAGE(GATT_GAP_SERVER_READ_APPEARANCE_IND);
        message->gap_server = gap_server;
        message->cid = cid;
        MessageSend(gap_server->app_task, GATT_GAP_SERVER_READ_APPEARANCE_IND, message);
        return TRUE;
    }
    return FALSE;
}
