/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_msg_handler.c

DESCRIPTION
    Contains the "GATT Server" Message handler, handles external messages
    sent to the "GATT Server" task.
    
NOTES

*/

/* "GATT Server" headers */
#include "gatt_server_private.h"

#include "gatt_server_msg_handler.h"
#include "gatt_server_access.h"
#include "gatt_server_indication.h"

/* External lib headers */
#include "gatt_manager.h"

/******************************************************************************/
void gattServerMsgHandler(Task task, MessageId id, Message payload)
{
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
            gattServerHandleGattManagerAccessInd(task, (const GATT_MANAGER_SERVER_ACCESS_IND_T*)payload);
            break;

        case GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM:
            gattServerHandleGattManagerIndicationCfm(task, (const GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM_T*)payload);
            break;

        default:
            /* Unrecognised GATT Manager message */
            break;
    }
}
