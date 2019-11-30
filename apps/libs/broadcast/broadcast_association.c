/* Copyright (c) 2015 - 2019 Qualcomm Technologies International, Ltd. */
/*  */
/*!
  @file broadcast_asociation.c
  @brief Association handling module of the Broadcast library.
*/

#include "broadcast.h"
#include "broadcast_private.h"
#include "broadcast_msg_client.h"
#include "gatt_broadcast_server.h"
#include "gatt_broadcast_client.h"

#include <stdlib.h>
#include <message.h>

/*! @brief Check if this receiver can support the version of the broadcaster.
 
    If the Broadcaster major version matches the receiver major version, and
    the receiver minor version is greater than or equal to the broadcaster
    minor version, we have a match.
 */
static bool broadcastAssocValidateVersion(uint16 version)
{
    uint8 major = (version >> 8) & 0xFF;
    uint8 minor = version & 0xFF;

    return ((BROADCAST_SERVICE_VERSION_MAJOR == major) &&
            (BROADCAST_SERVICE_VERSION_MINOR >= minor));
}

/*! @brief Handle the association data received from an Application. */
broadcast_status BroadcastSetAssocInfo(BROADCAST* broadcast,
                                    broadcast_assoc_data* assoc_data)
{
    if (!broadcastAssocValidateVersion(assoc_data->broadcast_service_version))
    {
        /* return failure, we don't support the version of the broadcaster */
        return broadcast_association_failed_unsupported_version;
    }
    else
    {
        /* copy association data received from the Broadcast service client, it'll get
         * free'd shortly when we destroy the client instance below. */
        RECEIVER_ASSOC_DATA(broadcast).broadcast_addr = assoc_data->broadcast_addr;

        RECEIVER_ASSOC_DATA(broadcast).seckey_len = assoc_data->seckey_len;
        memmove(RECEIVER_ASSOC_DATA(broadcast).seckey,
                assoc_data->seckey, assoc_data->seckey_len);

        RECEIVER_ASSOC_DATA(broadcast).device_status_len = assoc_data->device_status_len;
        memmove(RECEIVER_ASSOC_DATA(broadcast).device_status, 
                assoc_data->device_status, assoc_data->device_status_len);

        RECEIVER_ASSOC_DATA(broadcast).stream_service_records_len = assoc_data->stream_service_records_len;
        /* if we already have some stream service records, free is to avoid the leak */
        if (RECEIVER_ASSOC_DATA(broadcast).stream_service_records)
        {
            free(RECEIVER_ASSOC_DATA(broadcast).stream_service_records);
        }
        RECEIVER_ASSOC_DATA(broadcast).stream_service_records = malloc(assoc_data->stream_service_records_len);
        if (RECEIVER_ASSOC_DATA(broadcast).stream_service_records)
        {   
            memmove(RECEIVER_ASSOC_DATA(broadcast).stream_service_records,
                    assoc_data->stream_service_records,
                    assoc_data->stream_service_records_len);
        }
        memmove(RECEIVER_ASSOC_DATA(broadcast).broadcast_identifier,
                assoc_data->broadcast_identifier, BROADCAST_IDENTIFIER_LEN);

        /* Association data has been copied successfully, return success status to BA Receiver application */
        return broadcast_success;
    }
}

/*! @brief Returns the association data of the passed bradcast instance to an Application. */
broadcast_assoc_data* BroadcastGetAssocInfo(BROADCAST* broadcast)
{
    return &RECEIVER_ASSOC_DATA(broadcast);
}
