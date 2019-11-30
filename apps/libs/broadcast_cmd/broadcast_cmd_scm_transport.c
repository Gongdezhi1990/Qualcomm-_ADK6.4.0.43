/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    broadcast_cmd_scm_trasport.c

DESCRIPTION
      This file holds hardware specific SCM transport interface definations.
*/

#include <panic.h>
#include <print.h>
#include <vmtypes.h>
#include <scm.h>

#include "broadcast_cmd.h"
#include "broadcast_context.h"
#include "broadcast_cmd_private.h"
#include "broadcast_cmd_debug.h"

/****************************************************************************
 * Hardware Specific SCM transport interface functions
 ****************************************************************************/

/*!
@brief Interface for hardware specific module to register its task as SCM transport task.
*/
void BroadcastCmdScmTransportRegisterReq(Task transport_task)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    if(broadcast_cmd_role_broadcaster == bcmd->role)
    {
        SCMB *scm = BroadcastContextGetBroadcasterScm();
        /* only if we have valid SCM created */
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, so register SCM transport task\n"));
            ScmBroadcastTransportRegisterCfm(scm, transport_task);
        }
    }
    else
    {
        SCMR *scm = BroadcastContextGetReceiverScm();
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMR instance available, so register SCM transport task\n"));
            ScmReceiverTransportRegisterCfm(scm, transport_task);
        }
    }
    
}

/*!
@brief Interface for hardware specific module to de-register its task as SCM transport task.
*/
void BroadcastCmdScmTransportUnRegisterReq(void)
{
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    if(broadcast_cmd_role_broadcaster == bcmd->role)
    {
        SCMB *scm = BroadcastContextGetBroadcasterScm();
        /* only if we have valid SCM created */
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, so de-register SCM transport task\n"));
            /* The trigger for un-register transport task, will send disable cfm where all handles are cleared */
            ScmBroadcastTransportUnRegisterCfm(scm);
            /* Send Shutdown complete message */
           MessageSend(bcmd->client_task, BROADCAST_CMD_SHUTDOWN_CFM, NULL);
        }
    }
    else
    {
        SCMR *scm = BroadcastContextGetReceiverScm();
        if(scm)
        {
            BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMR instance available, so de-register SCM transport task\n"));
            ScmReceiverTransportUnRegisterCfm(scm);
            /* Send Shutdown complete message */
           MessageSend(bcmd->client_task, BROADCAST_CMD_SHUTDOWN_CFM, NULL);
        }
    }
}

/*!
@brief Interface for hardware specific module to inform message send comfirmation
*/
void BroadcastCmdScmSeqmentSendCfm(unsigned header, unsigned num_transmissions)
{
    SCMB *scm = BroadcastContextGetBroadcasterScm();
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    if(broadcast_cmd_role_broadcaster != bcmd->role)
    {  /* We should in broadcaster role to allow sending of this cfm message */
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Invalid try to broadcast SCM cfm msg in %s role\n", BroadcastcmdGetRoleAsString(bcmd->role)));
        return;
    }

    if(scm)
    {
        BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, send Seqment Cfm\n"));
        ScmBroadcastSegmentCfm(scm, header, num_transmissions);
    }
}

 /*!
@brief Interface for hardware specific module to inform message indication
*/
void BroadcastCmdScmSeqmentInd(unsigned header, const uint8 *data)
{
    SCMR* scm = BroadcastContextGetReceiverScm();
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    if(broadcast_cmd_role_receiver != bcmd->role)
    {  /* We should in broadcaster role to allow sending of this cfm message */
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Invalid try to broadcast SCM Ind msg in %s role\n", BroadcastcmdGetRoleAsString(bcmd->role)));
        return;
    }

    if(scm)
    {
        BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, send Seqment Ind\n"));
        ScmReceiverSegmentInd(scm, header, data);
    }
}

/*!
@brief Interface for hardware specific module to inform message Seqment Expired
*/
void BroadcastCmdScmSeqmentExpiredInd(unsigned header)
{
    SCMR* scm = BroadcastContextGetReceiverScm();
    broadcast_cmd_t *bcmd = BroadcastcmdGetInstance();
    
    if(broadcast_cmd_role_receiver != bcmd->role)
    {  /* We should in broadcaster role to allow sending of this cfm message */
        BROADCAST_CMD_DEBUG_INFO(("BCMD: Invalid try to broadcast SCM Exp Ind msg in %s role\n", BroadcastcmdGetRoleAsString(bcmd->role)));
        return;
    }

    if(scm)
    {
        BROADCAST_CMD_DEBUG_INFO(("BCMD: SCMB instance available, send Seqment Exp Ind\n"));
        ScmReceiverSegmentExpiredInd(scm, header);
    }
}



