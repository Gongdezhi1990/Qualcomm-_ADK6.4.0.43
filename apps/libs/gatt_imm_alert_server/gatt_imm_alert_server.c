/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include <gatt.h>

#include "gatt_imm_alert_server_private.h"

#include "gatt_imm_alert_server_msg_handler.h"


/****************************************************************************/

bool GattImmAlertServiceServerInit(Task appTask ,
                                GIASS_T *const imm_alert_server,
                                uint16 start_handle,
                                uint16 end_handle)
{
    /*Registration parameters for immediate alert service library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;
    
    /* validate the input parameters */
    if( INPUT_PARAM_NULL(appTask,imm_alert_server))
    {
        GATT_IMM_ALERT_SERVER_PANIC(("GIASS: Invalid Initialisation parameters"));
    }
    
    /* Reset all the service library memory */
    memset(imm_alert_server, 0, sizeof(GIASS_T));
    /*Set up the library task handler for external messages
     * immediate alert service library receives gatt manager messages here
     */
    imm_alert_server->lib_task.handler = imm_alert_server_ext_msg_handler;
    /*Store application message handler as application messages need to be posted here */
    imm_alert_server->app_task = appTask;
    /* Fill in the registration parameters */
    reg_params.start_handle = start_handle;
    reg_params.end_handle   = end_handle;
    reg_params.task = &imm_alert_server->lib_task;
    /* Try to register this instance of immediate alert service library to Gatt manager */
    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        /* Mark Lib init is done */
       return TRUE;
    }
    return FALSE;
}



