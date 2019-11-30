/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd.c

DESCRIPTION
      A utility library to handle hardware specific dependency of SCM library.
*/

#include <stdlib.h>

#include <panic.h>
#include <print.h>
#include <vmtypes.h>
#include <scm.h>

#include "broadcast_cmd.h"
#include "broadcast_cmd_handler.h"
#include "broadcast_context.h"
#include "broadcast_cmd_private.h"
#include "broadcast_cmd_debug.h"

/* Static Global instance of the broadcast command library */
static broadcast_cmd_t broadcast_cmd_ctx;

/****************************************************************************
 * Private functions for accessing broadcast cmd library instance
 ****************************************************************************/
 /*!
    @brief A private utility function to get the instance of the brocast cmd library
*/
 broadcast_cmd_t* BroadcastcmdGetInstance(void)
{
    return &broadcast_cmd_ctx;
}

/*!
    @brief Create an instance of the SCM broadcast library.
*/
void BroadcastCmdScmInit(Task client_task, broadcast_cmd_role role)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    /* Do we have valid client task to respond */
    PanicNull(client_task);

    BROADCAST_CMD_DEBUG_INFO(("BCMD: BroadcastCmdScmInit for %s role\n", BroadcastcmdGetRoleAsString(role)));
    bcmd->lib_task.handler = BroadcastCmdScmMessageHandler;
    bcmd->role = role;
    /* based on role, call the corresponding SCM init */
    if(broadcast_cmd_role_broadcaster == role)
    {
        /* register the application task to send any required messages */
        bcmd->client_task = client_task;
        /* Since its broadcaster role, also initialize the SCM handles */
        PanicNotNull(bcmd->broadcaster_handle);
        bcmd->broadcaster_handle = (broadcaster_t*)PanicUnlessMalloc(sizeof(broadcaster_t));
        memset(bcmd->broadcaster_handle, 0, sizeof(broadcaster_t));
        /* Initalize SCM library */
        ScmBroadcastInit(&bcmd->lib_task);
    }
    else
    {
        bcmd->client_task = client_task;
        ScmReceiverInit(&bcmd->lib_task);
    }
}


/*!
    @brief Enable/disable broadcasting.
*/
void BroadcastCmdScmEnable(Task transport_task)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    BROADCAST_CMD_DEBUG_INFO(("BCMD: BroadcastCmdScmEnable for %s role\n", BroadcastcmdGetRoleAsString(bcmd->role)));
    if(broadcast_cmd_role_broadcaster == bcmd->role)
    {
        SCMB *scm = BroadcastContextGetBroadcasterScm();
        /* only if we have valid SCM created */
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, enable SCM\n"));
            ScmBroadcastEnable(scm, transport_task);
        }
    }
    else
    {
        SCMR *scm = BroadcastContextGetReceiverScm();
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMR instance available, enable SCM\n"));
            ScmReceiverEnable(scm, transport_task);
        }
    }
}

/*!
    @brief Enable/disable broadcasting.
*/
void BroadcastCmdScmDisable(void)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    BROADCAST_CMD_DEBUG_INFO(("BCMD: BroadcastCmdScmDisable for %s role\n", BroadcastcmdGetRoleAsString(bcmd->role)));
    if(broadcast_cmd_role_broadcaster == bcmd->role)
    {
        SCMB *scm = BroadcastContextGetBroadcasterScm();
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, disable SCM\n"));
            ScmBroadcastDisable(scm);
        }
    }
    else
    {
        SCMR *scm = BroadcastContextGetReceiverScm();
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMR instance available, disable SCM\n"));
            ScmReceiverDisable(scm);
        }
    }
}

/*!
    @brief Broadcasting a AFH update SCM message
*/
bool BroadcastCmdScmSendAfhUpdate(const uint8* data, unsigned size_data, unsigned num_transmissions)
{
    SCMB *scm = BroadcastContextGetBroadcasterScm();
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    /* We need to be in broadcaster role to send message */
    if(broadcast_cmd_role_broadcaster != bcmd->role)
    {
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Invalid try to broadcast SCM message in %s role\n", BroadcastcmdGetRoleAsString(bcmd->role)));
        return FALSE;
    }
    
    /* Get the broadcaster SCM instance and ensure SCM is currently not busy */    
    if(scm && !ScmBroadcastIsBusy(scm))
    {
        if(bcmd->broadcaster_handle)
        {
            bcmd->broadcaster_handle->afh_update_handle = ScmBroadcastMessageReq(scm, 
                                                                data, 
                                                                size_data, 
                                                                num_transmissions);
            BROADCAST_CMD_DEBUG_INFO(("BCMD: Broadcast AFH Update SCM message handle %u\n",
                                                bcmd->broadcaster_handle->afh_update_handle));
            return (bcmd->broadcaster_handle->afh_update_handle ? TRUE : FALSE);    
        }
    }
    BROADCAST_CMD_DEBUG_INFO(("BCMD: Failed to broadcast SCM message as either SCMB instance is not avialable or SCM lib is busy\n"));
    return FALSE;
}

/*!
    @brief Register a different Task to receive BROADCAST_CMD_DISABLE_CFM.
*/
void BroadcastCmdScmRegisterDisableHandler(Task task)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();

    bcmd->scm_disable_task = task;
}

/*!
    @brief Destroy the scm instance contained in this library.
*/
void BroadcastCmdScmDestroy(void)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();

    if(broadcast_cmd_role_broadcaster == bcmd->role)
    {
        SCMB *scmb = BroadcastContextGetBroadcasterScm();
        if (scmb)
        {
            ScmBroadcastDestroy(scmb);
            BroadcastContextSetBroadcasterScm(NULL);

            if(bcmd->broadcaster_handle)
            {
                free(bcmd->broadcaster_handle);
                bcmd->broadcaster_handle = NULL;
            }
        }
    }
    else if(broadcast_cmd_role_receiver == bcmd->role)
    {
        SCMR *scmr = BroadcastContextGetReceiverScm();
        if (scmr)
        {
            ScmReceiverDestroy(scmr);
            BroadcastContextSetReceiverScm(NULL);
        }
    }

    /* Reset the disable_cfm Task */
    bcmd->scm_disable_task = (Task)NULL;

    /* Reset the role */
    bcmd->role = broadcast_cmd_role_unknown;
}
