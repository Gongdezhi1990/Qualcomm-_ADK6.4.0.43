/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_device_info_server_private.h"

#include "gatt_device_info_server_access.h"
#include "gatt_device_info_server_db.h"
#include "gatt_device_info_server_msg_handler.h"

/* Only one instance of Device Information Service is supported */
/****************************************************************************/
bool GattDeviceInfoServerInit(Task appTask, gdiss_t *const dev_info_server,
                              gatt_dis_init_params_t *const  init_params,
                              uint16 start_handle,
                              uint16 end_handle)
{
    /*Registration parameters for immediate alert service library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;

    if((appTask == NULL) || (dev_info_server == NULL) || (init_params == NULL))
    {
        GATT_DEVICE_INFO_SERVER_PANIC(("GDISS: Invalid Initialisation parameters"));
        return FALSE;
    }

    /* Reset all the service library memory */
    memset(dev_info_server, 0, sizeof(gdiss_t));
    /*Set up the library task handler for external messages
     * Device Information server library receives gatt manager messages here
     */
    dev_info_server->lib_task.handler = deviceInfoServerMsgHandler;
    /*Store application message handler as application messages need to be posted here */
    dev_info_server->app_task = appTask;
    dev_info_server->dis_params = *init_params;

    /* Fill in the registration parameters */
    reg_params.task = &dev_info_server->lib_task;
    reg_params.start_handle = start_handle;
    reg_params.end_handle = end_handle;
    /* Try to register this instance of Device information service library to Gatt manager */
    return (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success);
}

