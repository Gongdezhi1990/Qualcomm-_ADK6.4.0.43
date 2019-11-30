/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_transmit_power_server_private.h"

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_transmit_power_server_msg_handler.h"


/****************************************************************************/
bool GattTransmitPowerServerInitTask(Task appTask ,
                                GTPSS *const tps,
                                uint16 start_handle,
                                uint16 end_handle)
{
    /*Registration parameters for transmit power service library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;

    /* validate the input parameters */
    if( INPUT_PARAM_NULL(appTask, tps))
    {
        GATT_TRANSMIT_POWER_SERVER_PANIC(("TPS: Invalid Initialisation parameters"));
    }

    /* Reset all the server service library memory */
    memset(tps, 0, sizeof(GTPSS));

    /*Set up the library task handler for external messages
     * power transmit server service library receives gatt manager messages here
     */
    tps->lib_task.handler = transmit_power_server_ext_msg_handler;
    /*Store application message handler as application messages need to be posted here */

    tps->app_task = appTask;

    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &tps->lib_task;

    /* Try to register this instance of transmit power service library to Gatt manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    
    return FALSE;
}


