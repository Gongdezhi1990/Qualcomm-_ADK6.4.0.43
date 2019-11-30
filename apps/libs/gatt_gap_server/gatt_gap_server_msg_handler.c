/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_msg_handler.c

DESCRIPTION
    Contains the "GAP Server" Message handler, handles external messages
    sent to the "GAP Server" task.
    
NOTES

*/


/* Firmware headers */
#include <csrtypes.h>
#include <message.h>

/* "GATT Server" headers */
#include "gatt_gap_server_private.h"

#include "gatt_gap_server_msg_handler.h"
#include "gatt_gap_server_access.h"

/* External lib headers */
#include "gatt_manager.h"



/******************************************************************************/
void gattGapServerMsgHandler(Task task, MessageId id, Message payload)
{
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
            gattGapServerHandleGattManagerAccessInd(task, (const GATT_MANAGER_SERVER_ACCESS_IND_T*)payload);
            break;

        default:
            /* Unrecognised GATT Manager message */
            break;
    }
}

