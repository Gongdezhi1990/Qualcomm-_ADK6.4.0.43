/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/* Part of 6.2 */

#include <string.h>
#include <stdio.h>

#include "gatt_ama_server_private.h"

#include "gatt_ama_server_msg_handler.h"
#include "gatt_ama_server_db.h"


/****************************************************************************/
bool GattAmaServerInit(GAMASS *ama_server,
                           Task app_task,
                           const gatt_ama_server_init_params_t *init_params,
                           uint16 start_handle,
                           uint16 end_handle)
{
    gatt_manager_server_registration_params_t registration_params;

    if ((app_task == NULL) || (ama_server == NULL))
    {
        GATT_AMA_SERVER_PANIC(("GAMASS: Invalid Initialisation parameters"));
        return FALSE;
    }
    
    /* Set up library handler for external messages */
    ama_server->lib_task.handler = GattAmaServerMsgHandler;
        
    /* Store the Task function parameter.
       All library messages need to be sent here */
    ama_server->app_task = app_task;
        
    /* Check optional initialisation parameters */
    /* When GATT_MANAGER_USE_CONST_DB is enabled then it is the callers responsibility
     * to register the appropriate GATT ama server configuration when the 
     * const database is registered.
     */
    if (init_params)
    {
        /* Store notifications enable flag */
        ama_server->notifications_enabled = init_params->enable_notifications;
    }
    else
    {
        ama_server->notifications_enabled = FALSE;
    }
        
    /* Setup data required for Ama Service to be registered with the GATT Manager */
    registration_params.task = &ama_server->lib_task;
    registration_params.start_handle = start_handle;
    registration_params.end_handle = end_handle;
        
    /* Register with the GATT Manager and verify the result */
    return (GattManagerRegisterServer(&registration_params) == gatt_manager_status_success);
}
