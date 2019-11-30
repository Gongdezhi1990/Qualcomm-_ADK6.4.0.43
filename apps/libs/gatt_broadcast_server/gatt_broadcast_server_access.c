/* Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_broadcast_server_access.h"
#include "gatt_broadcast_server_db.h"

#include <gatt_manager.h>

/*! @brief Handler for data access request on broadcast server characteristics.
 */ 
void broadcast_server_handle_access_ind(GBSS* broadcast_server,
                                               const GATT_MANAGER_SERVER_ACCESS_IND_T* ind)
{
    /* Simple array for returning version and address characeteristics */
    uint8 value[7] = {BROADCAST_SERVICE_VERSION_MAJOR,
                      BROADCAST_SERVICE_VERSION_MINOR,
                      0, 0, 0, 0, 0};

    BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER:ACCESS_IND: Handle:0x%x Flags:0x%x\n",
                            ind->handle, ind->flags));

    /* we only permit reads */
    if (ind->flags & ATT_ACCESS_WRITE)
    {
        BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER:ACCESS_IND: Write not permitted Handle:0x%x\n",
                                ind->handle));
        GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                ind->handle, gatt_status_write_not_permitted, 0, NULL);
        return;
    }

    if (!broadcast_server->association_active)
    {
        BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER:ACCESS_IND: Association Not Active! 0x%x\n",
                                ind->handle));
        GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                ind->handle, gatt_status_read_not_permitted, 0, NULL);
        return;
    }

    /* handle read access only */
    if (ind->flags & ATT_ACCESS_READ)
    {
        switch (ind->handle)
        {
            case HANDLE_BROADCAST_VERSION:
            {
                GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                                ind->handle, gatt_status_success, 2, value);
            }
            break;
            case HANDLE_BROADCAST_STATUS:
            {
                GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                                ind->handle, gatt_status_success,
                                                broadcast_server->assoc_data.device_status_len,
                                                broadcast_server->assoc_data.device_status);
            }
            break;
            case HANDLE_BROADCAST_STREAM_SERVICE_RECORDS:
            {
                BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER:BSSR: offset:0x%x\n", ind->offset));
                if(ind->offset < broadcast_server->assoc_data.stream_service_records_len)
                {
                    GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                                    ind->handle, gatt_status_success, 
                                                    broadcast_server->assoc_data.stream_service_records_len - ind->offset,
                                                    broadcast_server->assoc_data.stream_service_records + (uint8)ind->offset);
                }
                else
                {
                    GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                        ind->handle, gatt_result_invalid_params, 0, NULL);
                }
            }
            break;
            case HANDLE_BROADCAST_ADDRESS:
            {
                value[0] = (broadcast_server->assoc_data.broadcast_addr.lap >> 24) & 0xFF;
                value[1] = (broadcast_server->assoc_data.broadcast_addr.lap >> 16) & 0xFF;
                value[2] = (broadcast_server->assoc_data.broadcast_addr.lap >> 8) & 0xFF;
                value[3] = (broadcast_server->assoc_data.broadcast_addr.lap) & 0xFF;
                value[4] = broadcast_server->assoc_data.broadcast_addr.uap;
                value[5] = (broadcast_server->assoc_data.broadcast_addr.nap >> 8) & 0xFF;
                value[6] = broadcast_server->assoc_data.broadcast_addr.nap & 0xFF;
                GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                                ind->handle, gatt_status_success, 7, value);
            }
            break;
            case HANDLE_BROADCAST_SECURITY_KEY:
            {
                GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                                ind->handle, gatt_status_success,
                                                broadcast_server->assoc_data.seckey_len,
                                                broadcast_server->assoc_data.seckey);
            }
            break;
            case HANDLE_BROADCAST_IDENTIFIER:
            {
                value[0] = (broadcast_server->assoc_data.broadcast_identifier[BROADCAST_IDENTIFIER_PRODUCT_OFFSET] >> 8) & 0xFF;
                value[1] = (broadcast_server->assoc_data.broadcast_identifier[BROADCAST_IDENTIFIER_PRODUCT_OFFSET]) & 0xFF;
                value[2] = (broadcast_server->assoc_data.broadcast_identifier[BROADCAST_IDENTIFIER_VERSION_OFFSET] >> 8) & 0xFF;
                value[3] = (broadcast_server->assoc_data.broadcast_identifier[BROADCAST_IDENTIFIER_VERSION_OFFSET]) & 0xFF;
                GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                                ind->handle, gatt_status_success,
                                                4, value);
            }
            break;
            default:
            {
                BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER: Unknown handle in GATT_MANAGER_SERVER_ACCESS_IND Handle:0x%x\n", ind->handle));
                /* send an access failure, we have to respond to the access_ind */
                GattManagerServerAccessResponse(&broadcast_server->service_task,
                                                ind->cid, ind->handle,
                                                gatt_status_invalid_handle, 0, NULL);
            }
            break;
        }
    }
    else
    {
        BROADCAST_SERVER_DEBUG(("BL:GATTMAN:SERVER: Unsupported GATT flags in GATT_MANAGER_SERVER_ACCESS_IND Handle:0x%x Flags:0x%x\n", ind->handle, ind->flags));
        GattManagerServerAccessResponse(&broadcast_server->service_task, ind->cid,
                                        ind->handle, gatt_status_failure, 0, NULL);
    }
}


