/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>

#include "gatt_link_loss_server_private.h"

#include "gatt_link_loss_server_access.h"
#include "gatt_link_loss_server_msg_handler.h"

/****************************************************************************/
bool GattLinkLossServerInit(Task appTask,
                                                         GLLSS_T *const link_loss_server,
                                                         uint16 start_handle,
                                                         uint16 end_handle)
{
    /*Registration parameters for Link Loss service library to GATT manager  */
    gatt_manager_server_registration_params_t reg_params;

    /* validate the input parameters */
    if(INPUT_PARAM_NULL(appTask, link_loss_server))
    {
        GATT_LINK_LOSS_SERVER_PANIC(("GLLS: Invalid Initialisation parameters"));
    }

        /* Reset all the service library memory */
        memset(link_loss_server, 0, sizeof(GLLSS_T));

        /*Set up the library task handler for external messages
     * Link loss service library receives gatt manager messages here      */
        link_loss_server->lib_task.handler = linkLossServerMsgHandler;

        /*Store application message handler as application messages need to be posted here */
        link_loss_server->app_task = appTask;
        /* Fill in the registration parameters */
        reg_params.start_handle = start_handle;
        reg_params.end_handle = end_handle;
        reg_params.task = &link_loss_server->lib_task;

        /* Try to register this instance of link loss service library to Gatt manager */
        if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
        {
        return TRUE;
        }

    return FALSE;
}

/****************************************************************************/
void GattLinkLossServerReadLevelResponse(
                                const GLLSS_T *link_loss_server, 
                                uint16 cid, 
                                uint8 alert_level)
{
    if (link_loss_server != NULL)
    {
        send_link_loss_alert_level_access_rsp(link_loss_server, cid, alert_level);
    }
}

