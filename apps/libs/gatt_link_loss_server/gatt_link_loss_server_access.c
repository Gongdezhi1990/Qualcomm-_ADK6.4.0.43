/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_link_loss_server_private.h"

#include "gatt_link_loss_server_access.h"

/***************************************************************************
NAME
    send_link_loss_access_rsp

DESCRIPTION
    Send an access response to the GATT Manager library.
*/
static void send_link_loss_access_rsp(Task task,
                                    uint16 cid,
                                    uint16 handle,
                                    uint16 result,
                                    uint16 size_value,
                                    const uint8 *value)
{
    if (!GattManagerServerAccessResponse(task, cid, handle, result, size_value, value))
    {
        /* The GATT Manager should always know how to send this response */
        GATT_LINK_LOSS_DEBUG_PANIC(("GLLS: Couldn't send GATT access response\n"));
    }
}

/***************************************************************************/
void send_link_loss_alert_level_access_rsp(const GLLSS_T *const link_loss_server, uint16 cid, uint8 alert_level)
{
    send_link_loss_access_rsp((Task)&link_loss_server->lib_task, cid, HANDLE_LINK_LOSS_ALERT_LEVEL, gatt_status_success, 1, &alert_level);
}

/***************************************************************************
NAME
    send_link_loss_access_error_rsp

DESCRIPTION
    Send an error access response to the GATT Manager library.
*/
static void send_link_loss_access_error_rsp(const GLLSS_T *const link_loss_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *const access_ind, uint16 error)
{
    send_link_loss_access_rsp((Task)&link_loss_server->lib_task, access_ind->cid, access_ind->handle, error, 0, NULL);
}

/***************************************************************************
NAME
    link_loss_alert_level_access

DESCRIPTION
    Handles the GATT_ACCESS_IND message sent to the HANDLE_LINK_LOSS_ALERT_LEVEL handle.
*/
static void link_loss_alert_level_access(GLLSS_T *const link_loss_server,
               const GATT_MANAGER_SERVER_ACCESS_IND_T *const access_ind)
{
    GATT_LINK_LOSS_DEBUG_INFO((" GLLS: handle_link_loss_alert_level_access_ind(), Access Ind flags = %x \n",access_ind->flags));

    if(link_loss_server != NULL)
    {
        if (access_ind->flags & ATT_ACCESS_READ)
        {
            MAKE_LINK_LOSS_MESSAGE(GATT_LLS_ALERT_LEVEL_READ_REQ);
            /* Inform app about the change in alert level */
            message->link_loss_server = link_loss_server;         /* Pass the instance which can be returned in the response */
            message->cid = access_ind->cid;         /*Fill in CID*/
            MessageSend(link_loss_server->app_task, GATT_LLS_ALERT_LEVEL_READ_REQ, message);
        }
        else if ( access_ind->flags & ATT_ACCESS_WRITE_COMPLETE)
        {
            if(access_ind->size_value == 1)
            {
                MAKE_LINK_LOSS_MESSAGE(GATT_LLS_ALERT_LEVEL_CHANGE_IND);
                /* Inform app about the change in alert level */
                message->link_loss_server = link_loss_server;         /* Pass the instance which can be returned in the response */
                message->cid = access_ind->cid;         /*Fill in CID*/
                message->alert_level = access_ind->value[0];
                MessageSend(link_loss_server->app_task, GATT_LLS_ALERT_LEVEL_CHANGE_IND, message);
                send_link_loss_access_rsp((Task)&link_loss_server->lib_task, access_ind->cid, HANDLE_LINK_LOSS_ALERT_LEVEL, gatt_status_success, 0, NULL);
            }
            else
            {
                send_link_loss_access_error_rsp(link_loss_server, access_ind, gatt_status_invalid_length);
            }
        }
        else
        {
            /* Reject access requests that aren't read/write, which shouldn't happen. */
            send_link_loss_access_error_rsp(link_loss_server, access_ind, gatt_status_request_not_supported);
        }
    }
}

/***************************************************************************
NAME
    link_loss_service_access

DESCRIPTION
    Deals with access of the HANDLE_LINK_LOSS_SERVICE handle.
*/
static void link_loss_service_access(GLLSS_T *const link_loss_server, const GATT_MANAGER_SERVER_ACCESS_IND_T *const access_ind)
{
    if(link_loss_server != NULL)
    {
        if (access_ind->flags & ATT_ACCESS_READ)
        {
            send_link_loss_access_rsp((Task)&link_loss_server->lib_task, access_ind->cid, HANDLE_LINK_LOSS_SERVICE, gatt_status_success, 0, NULL);
        }
        else if (access_ind->flags & ATT_ACCESS_WRITE)
        {
            /* Write of link loss service handle not allowed. */
            send_link_loss_access_error_rsp(link_loss_server, access_ind, gatt_status_write_not_permitted);
        }
        else
        {
            /* Reject access requests that aren't read/write, which shouldn't happen. */
            send_link_loss_access_error_rsp(link_loss_server, access_ind, gatt_status_request_not_supported);
        }
    }
}

/***************************************************************************
NAME
    handle_link_loss_access

DESCRIPTION
    Handles the GATT_ACCESS_IND message that was sent to the library.
*/
void handle_link_loss_access(GLLSS_T *const link_loss_server,
              const GATT_MANAGER_SERVER_ACCESS_IND_T *const access_ind)
{
    GATT_LINK_LOSS_DEBUG_INFO((" GLLS: handle_link_loss_access(), Handle = %x \n",access_ind->handle));
    
    switch (access_ind->handle)
    {
        case HANDLE_LINK_LOSS_SERVICE:
        {
            link_loss_service_access(link_loss_server, access_ind);
        }
        break;

        case HANDLE_LINK_LOSS_ALERT_LEVEL:
        {
              /* This is the handle we are interested */
             link_loss_alert_level_access(link_loss_server, access_ind);
        }
        break;
        
        default:
        {
            /* Respond to invalid handles */
            send_link_loss_access_error_rsp(link_loss_server, access_ind, gatt_status_invalid_handle);
        }
        break;
    }
}

