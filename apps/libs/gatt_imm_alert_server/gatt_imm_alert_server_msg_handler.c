/* Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <stdlib.h>

#include "gatt_imm_alert_server_private.h"

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_imm_alert_server_msg_handler.h"

/***************************************************************************
NAME
    send_gatt_imm_alert_access_resp

DESCRIPTION
    Sends Access Response to GATT manager
*/
static void send_gatt_imm_alert_access_resp(Task libtask,
                uint16 cid,
                uint16 result,
                uint16 handle,
                uint16 size_value,
                const uint8 *value)
{
      if(!GattManagerServerAccessResponse(libtask, cid, handle, result, size_value, value))
        {
            /* Not able to send the access response */
            GATT_IMM_ALERT_SERVER_DEBUG_PANIC(("GattManagerServerAccessResponse() Failed\n"));
        }
}

/***************************************************************************
NAME
    send_imm_alert_access_resp

DESCRIPTION
    Prepare the access response for GATT manager.
*/
static void send_imm_alert_access_resp(GIASS_T *const imm_alert_service,
              const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind,
              uint16 status)
{
        /* send the access response */
        send_gatt_imm_alert_access_resp((Task)&imm_alert_service->lib_task,access_ind->cid,status,access_ind->handle,0,NULL);
}

/***************************************************************************
NAME
    handle_imm_alert_level_access_ind

DESCRIPTION
    Handles the GATT_ACCESS_IND message sent to the HANDLE_IMM_ALERT_LEVEL handle.
*/
static void handle_imm_alert_level_access_ind(GIASS_T *const imm_alert_service,
               const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    gatt_status_t status = gatt_status_request_not_supported;
    
    GATT_IMM_ALERT_SERVER_DEBUG_INFO((" Func:handle_imm_alert_level_access_ind(), Access Ind flags = %x \n",access_ind->flags));

    if (access_ind->flags & ATT_ACCESS_WRITE)
    {  
        MAKE_IMM_ALERT_MESSAGE(GATT_IMM_ALERT_SERVER_WRITE_LEVEL_IND);
        /* Pass the instance which can be returned in the response */
        message->imm_alert_service = imm_alert_service;     
        /* Fill in the alert level */
        message->alert_level = (gatt_imm_alert_level)access_ind->value[0];
        /*Fill in CID*/
        message->cid = access_ind->cid;
        MessageSend(imm_alert_service->app_task, GATT_IMM_ALERT_SERVER_WRITE_LEVEL_IND, message);
        status = gatt_status_success;
        GATT_IMM_ALERT_SERVER_DEBUG_INFO(("GATT_IMM_ALERT_SERVER_WRITE_LEVEL_IND notified to Application task \n"));
    }
    else if (access_ind->flags & ATT_ACCESS_READ)
    {
        status = gatt_status_read_not_permitted;
    }
    
    /* Send the access reponse */
    send_imm_alert_access_resp(imm_alert_service,access_ind,status);
}

/***************************************************************************
NAME
    handle_imm_alert_service_access_ind

DESCRIPTION
    Handles the GATT_ACCESS_IND message for HANDLE_IMM_ALERT_SERVICE that was sent to the library.
*/
static void handle_imm_alert_service_access_ind(GIASS_T *const imm_alert_service,
              const GATT_MANAGER_SERVER_ACCESS_IND_T * access_ind)
{
     gatt_status_t status = gatt_status_request_not_supported;

     if (access_ind->flags & ATT_ACCESS_READ)
     {
        status = gatt_status_success;
     }
     else if (access_ind->flags & ATT_ACCESS_WRITE)
     {
        /* Write not permitted */
       status = gatt_status_write_not_permitted;
     }
    send_imm_alert_access_resp(imm_alert_service,access_ind,status);
}

/***************************************************************************
NAME
    handle_imm_alert_access_ind

DESCRIPTION
    Handles the GATT_ACCESS_IND message that was sent to the library.
*/
static void handle_imm_alert_access_ind(GIASS_T *const imm_alert_service,
              const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind)
{
    GATT_IMM_ALERT_SERVER_DEBUG_INFO((" Func:handle_imm_alert_access_ind(), Handle = %x \n",access_ind->handle));
    
    switch (access_ind->handle)
    {
        case HANDLE_IMM_ALERT_LEVEL:
        {
              /* This is the handle we are interested */
             handle_imm_alert_level_access_ind(imm_alert_service, access_ind);
        }
        break;

        case HANDLE_IMM_ALERT_SERVICE:
        {
           /* Service Handle read request */
           handle_imm_alert_service_access_ind(imm_alert_service, access_ind);
        }
        break;
        
        default:
        {
            /* Respond to invalid handles */
            send_imm_alert_access_resp(imm_alert_service,access_ind,gatt_status_invalid_handle);
        }
        break;
    }
}

/****************************************************************************/
void imm_alert_server_ext_msg_handler(Task task, MessageId id, Message msg)
{

    GIASS_T *imm_alert_service = (GIASS_T*)task;
    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
             /* ATT Access IND is received */
            GATT_IMM_ALERT_SERVER_DEBUG_INFO((" Received 'GATT_MANAGER_SERVER_ACCESS_IND' \n"));
            handle_imm_alert_access_ind(imm_alert_service, (const GATT_MANAGER_SERVER_ACCESS_IND_T *)msg);
        }
        break;
        
        default:
        {
            /* GATT unrecognised messages */
            GATT_IMM_ALERT_SERVER_DEBUG_PANIC(("Func: imm_alert_server_ext_msg_handler()  Unknown message \n"));
        }
        break;
    }
}


