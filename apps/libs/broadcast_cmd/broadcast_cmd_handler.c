/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_handler.c

DESCRIPTION
      A utility file to handle SCM messages.
*/

#include <panic.h>
#include <print.h>
#include <vmtypes.h>
#include <scm.h>
#include <csb.h>
#include <stdlib.h>

#include "broadcast_cmd_handler.h"
#include "broadcast_cmd.h"
#include "broadcast_context.h"
#include "broadcast_cmd_debug.h"
#include "broadcast_cmd_private.h"


/*************************************************************************/
static void handleBroadcasterScmMsgCfm(SCM_BROADCAST_MESSAGE_CFM_T *cfm)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();

    if(cfm->handle == bcmd->broadcaster_handle->afh_update_handle)
    {
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Received AFH Update MSG_CFM\n"));
        /* There is no data to send */    
        MessageSend(bcmd->client_task, BROADCAST_CMD_AFH_UPDATE_CFM, NULL);
        /* reset the handle */
        bcmd->broadcaster_handle->afh_update_handle = 0;
    }
    else
    {
        /* Invalid handle */
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Not expecting this %u MSG_CFM\n", cfm->handle));
    }
}

/*************************************************************************/
static void handleReceiverScmMsgInd(SCM_RECEIVER_MESSAGE_IND_T *ind)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    if(ind && ind->size_data && ind->data)
    {
        if(ind->data[0] == bcmd_scm_msg_id_afh_update_full)
        {
            MAKE_MESSAGE_WITH_LEN(BROADCAST_CMD_AFH_FULL_UPDATE_IND, ind->size_data);
            BROADCAST_CMD_DEBUG_INFO(("BCMD: Received AFH Update MSG_IND\n"));
            /* copy the data */
            message->size_data = ind->size_data;
            memcpy(message->data, ind->data, message->size_data);
            MessageSend(bcmd->client_task, BROADCAST_CMD_AFH_FULL_UPDATE_IND, message);
        }
        else
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: Not expecting this %d MSG_IND\n", ind->data[0]));
        }
        free(ind->data);
    }
}

/*! @brief Send BROADCAST_CMD_INIT_CFM message to client task, */
static void handleScmInitCfm(void *scm, scm_status status)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    MAKE_MESSAGE(BROADCAST_CMD_INIT_CFM);
    message->role = bcmd->role;
    message->status = broadcast_cmd_failed;

    BROADCAST_CMD_DEBUG_INFO(("BCMD: Role %d Staus %d\n", bcmd->role, status));   

    if(scm_success == status)
    {
        message->status = broadcast_cmd_success;
        /* Successfully created broadcaster SCM instance. Store it in context library */
        if(broadcast_cmd_role_broadcaster == bcmd->role)
        {
            PanicNotNull(BroadcastContextGetBroadcasterScm());
            BroadcastContextSetBroadcasterScm((SCMB*)scm);
        }
        else
        {
            PanicNotNull(BroadcastContextGetReceiverScm());
            BroadcastContextSetReceiverScm((SCMR*)scm);
        }
    }
    /* Send the application the status of SCM init */
    MessageSend(bcmd->client_task, BROADCAST_CMD_INIT_CFM, message);
}

/*************************************************************************/
static void handleScmEnableCfm(scm_status status)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    MAKE_MESSAGE(BROADCAST_CMD_ENABLE_CFM);
    message->role = bcmd->role;
    message->status = broadcast_cmd_failed;
    BROADCAST_CMD_DEBUG_INFO(("BCMD: Role %d Staus %d\n", bcmd->role, status));   
    
    if(scm_success == status)
        message->status = broadcast_cmd_success; 
    /* Send the application the status of SCM Enable */
    MessageSend(bcmd->client_task, BROADCAST_CMD_ENABLE_CFM, message);
}

/*************************************************************************/
static void handleScmDisableCfm(void *scm, scm_status status)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    Task task = bcmd->scm_disable_task ? bcmd->scm_disable_task : bcmd->client_task;

    if(scm)
    {
        MAKE_MESSAGE(BROADCAST_CMD_DISABLE_CFM);
        message->role = bcmd->role;
        message->status = (scm_success == status) ? broadcast_cmd_success : broadcast_cmd_failed;
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Role %d Staus %d\n", bcmd->role, status));

        /* Send the application the status of SCM Enable */
        MessageSend(task, BROADCAST_CMD_DISABLE_CFM, message);
    }
}

/*************************************************************************
  Utility function to handle Broadcaster related SCM messages
*/
static void handleBroadcasterScmMessage(Task task, MessageId id, Message msg)
{
    UNUSED(task);

    switch(id)
    {
        case SCM_BROADCAST_INIT_CFM:
        {
            SCM_BROADCAST_INIT_CFM_T* scm_msg = (SCM_BROADCAST_INIT_CFM_T*)msg;
            BROADCAST_CMD_DEBUG_INFO(("SCM_BROADCAST_INIT_CFM\n"));
            handleScmInitCfm(scm_msg->scm, scm_msg->status);
        }
        break;
        case SCM_BROADCAST_ENABLE_CFM:
        {
            BROADCAST_CMD_DEBUG_INFO(("SCM_BROADCAST_ENABLE_CFM\n"));
            handleScmEnableCfm(((SCM_BROADCAST_ENABLE_CFM_T*)msg)->status);
        }
        break;
        case SCM_BROADCAST_DISABLE_CFM:
        {
            SCM_BROADCAST_DISABLE_CFM_T* scm_msg = (SCM_BROADCAST_DISABLE_CFM_T*)msg;
            BROADCAST_CMD_DEBUG_INFO(("SCM_BROADCAST_DISABLE_CFM\n"));
            handleScmDisableCfm((void*)scm_msg->scm, scm_msg->status);
        }
        break;
        case SCM_BROADCAST_MESSAGE_CFM:
        {
            BROADCAST_CMD_DEBUG_INFO(("SCM_BROADCAST_MESSAGE_CFM\n"));
            handleBroadcasterScmMsgCfm((SCM_BROADCAST_MESSAGE_CFM_T*)msg);
        }
        break;
        default:
            BROADCAST_CMD_DEBUG_INFO(("BCMD: %d Unhandled Broadcaster Message\n", id));
        break;
    }
}

/*************************************************************************
  Utility function to handle Receiver related SCM messages
*/
static void handleReceiverScmMessage(Task task, MessageId id, Message msg)
{
    UNUSED(task);

    switch(id)
    {
        case SCM_RECEIVER_INIT_CFM:
        {
            SCM_RECEIVER_INIT_CFM_T* scm_msg = (SCM_RECEIVER_INIT_CFM_T*)msg;
            BROADCAST_CMD_DEBUG_INFO(("SCM_RECEIVER_INIT_CFM\n"));
            handleScmInitCfm(scm_msg->scm, scm_msg->status);
        }
        break;
        case SCM_RECEIVER_ENABLE_CFM:
        {
            BROADCAST_CMD_DEBUG_INFO(("SCM_RECEIVER_ENABLE_CFM\n"));
            handleScmEnableCfm(((SCM_RECEIVER_ENABLE_CFM_T*)msg)->status);
        }
        break;
        case SCM_RECEIVER_DISABLE_CFM:
        {
            SCM_RECEIVER_DISABLE_CFM_T* scm_msg = (SCM_RECEIVER_DISABLE_CFM_T*)msg;
            BROADCAST_CMD_DEBUG_INFO(("SCM_RECEIVER_DISABLE_CFM\n"));
            handleScmDisableCfm((void*)scm_msg->scm, scm_msg->status);
        }
        break;
        case SCM_RECEIVER_MESSAGE_IND:
        {
            BROADCAST_CMD_DEBUG_INFO(("SCM_RECEIVER_MESSAGE_IND\n"));
            handleReceiverScmMsgInd((SCM_RECEIVER_MESSAGE_IND_T*)msg);
        }
        break;
        default:
            BROADCAST_CMD_DEBUG_INFO(("BCMD: %d Unhandled Broadcaster Message\n", id));
        break;
    }
}

/***************************************************************************/
void BroadcastCmdScmMessageHandler(Task task, MessageId id, Message message)
{
    if(id >= SCM_BROADCAST_INIT_CFM && id <= SCM_BROADCAST_MESSAGE_CFM)
    {
        handleBroadcasterScmMessage(task, id, message);
    }
    else if(id >= SCM_RECEIVER_INIT_CFM && id <= SCM_RECEIVER_MESSAGE_IND)
    {
        handleReceiverScmMessage(task, id, message);
    }
    else
    {
        BROADCAST_CMD_DEBUG_INFO(("BCMD: %d Invalid ID, cannot be handled\n", id));
    }
}
