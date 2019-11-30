/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */

#ifdef ENABLE_BROADCAST_GATT_SERVICE

#include "gatt_broadcast_server.h"
#include "gatt_broadcast_server_db.h"
#include "gatt_broadcast_server_msg_handler.h"

#include <message.h>
#include <gatt.h>
#include <gatt_manager.h>
#include <stdlib.h>

/*! @brief Permit clients to read contents of broadcast server database
 */
gatt_broadcast_server_status GattBroadcastServerEnable(GBSS* const broadcast_server,
                                                       bool enable)
{
    if(broadcast_server)
    {
        broadcast_server->association_active = enable;
        return gatt_broadcast_server_status_success;
    }
    return gatt_broadcast_server_invalid_instance;
}

/*! @brief Setup the contents of the Broadcast server characteristics.
 
    Only store seckey, codec_info and vendor_info if they have non-zero length.
 */
gatt_broadcast_server_status GattBroadcastServerSetupCharacteristics(GBSS* const broadcast_server,
                                                                     broadcast_assoc_data* const assoc_data)
{
    if(broadcast_server && assoc_data)
    {
        /* copy simple fields */
        broadcast_server->assoc_data.broadcast_addr = assoc_data->broadcast_addr;
        memmove(broadcast_server->assoc_data.broadcast_identifier,
                assoc_data->broadcast_identifier, BROADCAST_IDENTIFIER_LEN*sizeof(uint16));

        /* only update seckey if non-zero length */
        if (assoc_data->seckey_len != 0)
        {
            broadcast_server->assoc_data.seckey_len = assoc_data->seckey_len;
            memmove(broadcast_server->assoc_data.seckey, assoc_data->seckey,
                    assoc_data->seckey_len);
        }

        /* update device status */
        if (assoc_data->device_status_len != 0)
        {
            broadcast_server->assoc_data.device_status_len = assoc_data->device_status_len;
            memmove(broadcast_server->assoc_data.device_status, assoc_data->device_status,
                    assoc_data->device_status_len);
        }

        /* update codec_info */
        if (assoc_data->stream_service_records_len != 0)
        {
            /* copy reference to updated stream service records */
            broadcast_server->assoc_data.stream_service_records_len = assoc_data->stream_service_records_len;
            broadcast_server->assoc_data.stream_service_records = assoc_data->stream_service_records;
        }

        return gatt_broadcast_server_status_success;
    }
    return gatt_broadcast_server_status_setup_char_failed;
}

/*! @brief Register the broadcast service server with the GATT manager.
 */
gatt_broadcast_server_status GattBroadcastServerInit(Task client_task,
                                                      GBSS* const broadcast_server,
                                                      uint16 start_handle,
                                                      uint16 end_handle)
{
    gatt_broadcast_server_status retval = gatt_broadcast_server_status_invalid_parameter;

    /* if we have a client and it provided an instance of the service
     * try to register this instance of the broadcast server with the GATT
     * manager */
    if ((client_task != NULL) && (broadcast_server != NULL))
    {
        gatt_manager_server_registration_params_t reg_params;

        /* setup our broadcast server data structure */
        memset(broadcast_server, 0, sizeof(*broadcast_server));
        broadcast_server->service_task.handler = broadcast_server_msg_handler;
        broadcast_server->client_task = client_task;

        /* setup the GATT manager parameter structure */
        reg_params.start_handle = start_handle;
        reg_params.end_handle = end_handle;
        reg_params.task = &broadcast_server->service_task;

        /* register our broadcast server with the GATT manager */
        if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
        {
            broadcast_server->association_active = FALSE;
            retval = gatt_broadcast_server_status_success;
        }
        else
        {
            retval = gatt_broadcast_server_status_registration_failed;
        }
    }
        
    BROADCAST_SERVER_DEBUG(("BL:GATT:SERVER: Init %s\n",
                            retval == gatt_broadcast_server_status_success ?
                                                                  "success":"failed"));
    return retval;
}

#else

typedef int iamnotempty;

#endif /* ENABLE_BROADCAST */
