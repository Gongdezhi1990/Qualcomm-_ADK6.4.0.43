/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_broadcast_server_msg_handler.h"
#include "gatt_broadcast_server.h"
#include "gatt_broadcast_server_access.h"

#include <gatt_manager.h>

/*! @brief Broadcast Server message handler.
 */
void broadcast_server_msg_handler(Task task, MessageId id, Message message)
{
    GBSS* broadcast_server = (GBSS*)task;

    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER: GATT_MANAGER_SERVER_ACCESS_IND\n"));
            broadcast_server_handle_access_ind(broadcast_server,
                                               (GATT_MANAGER_SERVER_ACCESS_IND_T*)message);
        }
        break;
        default:
        {
            BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER: *** Unhandled message *** (0x%x)\n", id));
        }
        break;
    }
}
