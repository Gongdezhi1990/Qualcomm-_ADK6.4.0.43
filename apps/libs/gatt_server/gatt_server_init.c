/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_init.c

DESCRIPTION
    The "GATT Server" initialisation routines.
    
NOTES

*/

/* Firmware headers */
#include <stdlib.h>

/* GATT "GATT Server" library headers */
#include "gatt_server_private.h"
#include "gatt_server_msg_handler.h"
#include "gatt_server_db.h"

/* External library headers */
#include "gatt_manager.h"

/******************************************************************************/
gatt_server_status_t GattServerInit(GGATTS *gatt_server, Task app_task, uint16 start_handle, uint16 end_handle)
{
    if (gatt_server && app_task)
    {
        gatt_manager_status_t                      registration_status;
        gatt_manager_server_registration_params_t  params;
        
        gattServerSetGattServerTask(gatt_server, gattServerMsgHandler);
        gattServerSetClientTask(gatt_server, app_task);
        
        params.task = gattServerGetGattServerTask(gatt_server);
        params.start_handle = start_handle;
        params.end_handle = end_handle;
        {
            registration_status = GattManagerRegisterServer(&params);
            if (registration_status == gatt_manager_status_success)
            {
                return gatt_server_status_success;
            }
        }
        return gatt_server_status_registration_failed;
    }
    return gatt_server_status_invalid_parameter;
}
