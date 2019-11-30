/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>

#include <gatt_manager.h>

#include "gatt_device_info_server_private.h"

#include "gatt_device_info_server_msg_handler.h"
#include "gatt_device_info_server_access.h"


/****************************************************************************/
void deviceInfoServerMsgHandler(Task task, MessageId id, Message payload)
{
    gdiss_t *const dev_info_server = (gdiss_t*)task;

    if(dev_info_server != NULL)
    {
        switch (id)
        {
            case GATT_MANAGER_SERVER_ACCESS_IND:
            {
                /* Read/write access to characteristic */
                handleDeviceInfoServerAccess(dev_info_server, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)payload);
            }
            break;
            case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
            {
                /* Library just absorbs confirmation messages */
            }
            break;
            default:
            {
                /* Unrecognised GATT Manager message */
                GATT_DEVICE_INFO_SERVER_DEBUG_PANIC(("GATT Manager Server Msg not handled\n"));
            }
            break;
        }
    }
}


