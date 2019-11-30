/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <stdio.h>
#include <stdlib.h>

#include "gatt_imm_alert_client_private.h"

#include <gatt.h>
#include <gatt_manager.h>

#include "gatt_imm_alert_client_msg_handler.h"

#include "gatt_imm_alert_client_debug.h"

/***************************************************************************
NAME
    send_imm_alert_client_error

DESCRIPTION
   Utility function to send confirmation responses to application  
*/
void send_imm_alert_client_cfm(GIASC_T *const imm_alert_client,
              const uint16 cid,
              const uint16 handle,
              const gatt_imm_alert_client_status status,
              const imm_alert_cfm_mesage_type cfmType)
{
    switch(cfmType)
    {
        /* Characteristic handle discovery indication */
        case IMM_ALERT_CHAR_HANDLE_CFM:
        {
            MAKE_IMM_ALERT_CLIENT_MESSAGE(GATT_IMM_ALERT_CLIENT_INIT_CFM);
            /* Fill in client reference */
            message->imm_alert_client = imm_alert_client;
            /* Fill in the alert handle */
            message->imm_alert_handle = handle;
            /* Fill in the connection ID */
            message->cid = cid;
            /* Fill in the status */
            message->status = status;
            /* send the confirmation message to app task  */
            MessageSend(imm_alert_client->app_task, GATT_IMM_ALERT_CLIENT_INIT_CFM, message);
        }
        break;

        /* Set Alert level confirmation  */
        case IMM_ALERT_WRITE_REQUEST_CFM:
        {
            MAKE_IMM_ALERT_CLIENT_MESSAGE(GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM);
            /* Fill in client reference */
            message->imm_alert_client = imm_alert_client;     
             /* Fill in the connection ID */
            message->cid = cid;
            /* Fill in the status */
            message->status = status;
            /* send the confirmation message to app task  */
            MessageSend(imm_alert_client->app_task, GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM, message);
        }
        break;
        
        default:
        {
            /*Should not reach here */
            GATT_IMM_ALERT_CLIENT_DEBUG_PANIC(("Func:send_imm_alert_client_cfm(): unknown confirmation Type\n"));
        }
        break;
    }
}

/***************************************************************************
NAME
    send_imm_alert_client_error

DESCRIPTION
   Utility function to send error response 
*/
static void send_imm_alert_client_error(GIASC_T *const imm_alert_client,
            const uint16 cid,
            const gatt_status_t status,
            const imm_alert_cfm_mesage_type cfmType)
{
    /* If the GATT connection does not exists, inform same to application */
    GATT_IMM_ALERT_CLIENT_DEBUG_INFO(("Func:send_imm_alert_client_error(); status = %x,cfmType= %x, cid = %x\n",status,cfmType,cid));

    /* Inform application that GATT connection is not existing  for this connection ID */
    if(status == gatt_status_invalid_cid)
    {
        send_imm_alert_client_cfm(imm_alert_client,cid,imm_alert_client->alert_handle, gatt_imm_alert_client_status_no_connection,cfmType);
    }
    else
    {
        /* Any other failure; inform application with failure status */
        send_imm_alert_client_cfm(imm_alert_client,cid, imm_alert_client->alert_handle, gatt_imm_alert_client_status_failed,cfmType);
    }
}

/***************************************************************************
NAME
    imm_alert_client_set_alert_level

DESCRIPTION
   Try to set the Immediate Alert Level with the remote service 
*/
static void imm_alert_client_set_alert_level(GIASC_T *const imm_alert_client,
                const IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT_T *set_alert)
{
    gatt_imm_alert_set_level alert_level = set_alert->alert_level;
    
    /* Write the characteristic value to trigger the Alert */
    GattManagerWriteWithoutResponse((Task)&imm_alert_client->lib_task,
                                    imm_alert_client->alert_handle,
                                    GATT_IMM_ALERT_VALUE_LEN,
                                    (uint8 *)&alert_level);
}

/***************************************************************************
NAME
    handle_imm_alert_discover_char_cfm

DESCRIPTION
   Handles immediate alert characteristics handle confirmation 
*/
static void handle_imm_alert_discover_char_cfm(GIASC_T *const imm_alert_client,
               const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*const char_cfm)
{
    if(imm_alert_client != NULL)
    {
        if(char_cfm->status == gatt_status_success)
        {
            /* Verify the char UIID is of what immediate alert client is interested, else ignore */
            if((char_cfm->uuid_type == gatt_uuid16)&&(char_cfm->uuid[0] == IMM_ALERT_LEVEL_CHAR_UUID))
            {
                /* Store the alert level handle for future use */
                imm_alert_client->alert_handle = char_cfm->handle;
                /* Report success to application */
                send_imm_alert_client_cfm(imm_alert_client,char_cfm->cid,imm_alert_client->alert_handle,
                                                         gatt_imm_alert_client_status_success,IMM_ALERT_CHAR_HANDLE_CFM);
            }
            /* Ignore other char UUID's */
        }
        else
        {
           /* Report error to application */
           send_imm_alert_client_error(imm_alert_client,char_cfm->cid,
                                                      char_cfm->status,IMM_ALERT_CHAR_HANDLE_CFM);
        }
    }
}

/***************************************************************************
NAME
    handle_imm_alert_write_response_cfm

DESCRIPTION
   Handles immediate  alert set alert level confirmation
*/
static void handle_imm_alert_write_response_cfm(GIASC_T *const imm_alert_client,
              const GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM_T *const write_cfm)
{
    if(imm_alert_client != NULL)
    {
        if(write_cfm->status == gatt_status_success)
        {
            /* Report success to application */
            send_imm_alert_client_cfm(imm_alert_client,write_cfm->cid,imm_alert_client->alert_handle,
                                                      gatt_imm_alert_client_status_success,IMM_ALERT_WRITE_REQUEST_CFM);
        }
        else
        {
             /* Report error to application */
             send_imm_alert_client_error(imm_alert_client,write_cfm->cid,
                                                        write_cfm->status,IMM_ALERT_WRITE_REQUEST_CFM);
        }
    }
}

static void handle_imm_alert_client_gatt_manager_msg(Task task, MessageId id, Message msg)
{

    GIASC_T *const imm_alert_client = (GIASC_T*)task;

    GATT_IMM_ALERT_CLIENT_DEBUG_INFO(("handle_imm_alert_client_gatt_manager_msg(%d) \n",id));


    switch(id)
    {

        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM:
        {
            /* Characteristic handle discovery */
            handle_imm_alert_discover_char_cfm(imm_alert_client,(const GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T*)msg);
        }
        break;

        case GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM:
        {
            /* Write Confirmation */
            handle_imm_alert_write_response_cfm(imm_alert_client,(const GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM_T*)msg);
        }
        break;

        default:
        {
            /* Unknown Message , Ignore */
        }
        break;
    }

}

/***************************************************************************
NAME
    handle_imm_alert_ext_message

DESCRIPTION
   Handles immediate Alert Service External messages (From GATT manager )
*/
static void handle_imm_alert_ext_message(MessageId id)
{
    UNUSED(id);

    /* GATT unrecognised messages */
    GATT_IMM_ALERT_CLIENT_DEBUG_PANIC(("Unknown Message received from GATT lib 0x%x\n",id));
}

/***************************************************************************
NAME
    handle_imm_alert_internal_message

DESCRIPTION
   Handles immediate Alert Service internal messages 
*/
static void  handle_imm_alert_internal_message(Task task, MessageId id, Message msg)
{
    GIASC_T *const imm_alert_client = (GIASC_T*)task;
    
    GATT_IMM_ALERT_CLIENT_DEBUG_INFO(("handle_imm_alert_internal_messge(%d) \n",id));
    
    switch(id)
    {
        /* Connect Request */
        case IMM_ALERT_CLIENT_INTERNAL_MSG_CONNECT:
        {
            /* Start by discovering Characteristic handles */
            GattManagerDiscoverAllCharacteristics((Task)&imm_alert_client->lib_task);
        }
        break;
        /* Alert level set request */
        case IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT:
        {
            imm_alert_client_set_alert_level(imm_alert_client,(const IMM_ALERT_CLIENT_INTERNAL_MSG_SET_ALERT_T*)msg);
        }
        break;

        default:
        {
            /* Inetrnal unrecognised messages */
            GATT_IMM_ALERT_CLIENT_DEBUG_PANIC(("Unknown Message received from Internal To lib \n"));
        }
        break;
    }
}

/****************************************************************************/
void imm_alert_client_msg_handler(Task task, MessageId id, Message msg)
{
     GATT_IMM_ALERT_CLIENT_DEBUG_INFO(("imm_alert_client_msg_handler (ID= %d)\n",id));
     /* Check mesage is from GATT Manager */
    if((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        handle_imm_alert_client_gatt_manager_msg(task,id,msg);
    }
    else if ((id >= GATT_MESSAGE_BASE) && (id < GATT_MESSAGE_TOP))
    {
        handle_imm_alert_ext_message(id);
    }
    /* Check message is internal Message */
    else if((id > IMM_ALERT_CLIENT_INTERNAL_MSG_BASE) && (id < IMM_ALERT_CLIENT_INTERNAL_MSG_TOP))
    {
        handle_imm_alert_internal_message(task,id,msg);
    }
    else
    {
        /* Unknown message , can not handle */
        GATT_IMM_ALERT_CLIENT_DEBUG_PANIC(("Unknown Message received  \n"));
    }
}

