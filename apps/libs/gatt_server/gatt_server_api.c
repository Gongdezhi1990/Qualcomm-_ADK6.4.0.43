/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_api.c

DESCRIPTION
    Contains the "GATT Server" API routines
    
NOTES

*/

/* Firmware headers */
#include <stdlib.h>

/* GATT Server headers */
#include "gatt_server_private.h"
#include "gatt_server_api.h"


/******************************************************************************/
bool gattServerSendReadClientConfigInd(Task task, uint16 cid, uint16  handle)
{
    if (task)
    {
        MAKE_GATT_SERVER_MESSAGE(GATT_SERVER_READ_CLIENT_CONFIG_IND);
        message->cid = cid;
        message->handle = handle;
        MessageSend(task, GATT_SERVER_READ_CLIENT_CONFIG_IND, message);
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
bool gattServerSendWriteClientConfigInd(Task task, uint16 cid, uint16 value)
{
    if (task)
    {
        MAKE_GATT_SERVER_MESSAGE(GATT_SERVER_WRITE_CLIENT_CONFIG_IND);
        message->cid = cid;
        message->config_value = value;
        MessageSend(task, GATT_SERVER_WRITE_CLIENT_CONFIG_IND, message);
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
bool gattServerSendServiceChangedIndicationCfm(Task task,
                                               uint16 cid,
                                               gatt_status_t status)
{
    if (task)
    {
        MAKE_GATT_SERVER_MESSAGE(GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM);
        message->cid = cid;
        message->status = status;
        MessageSend(task, GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM, message);
        return TRUE;
    }
    return FALSE;
}
