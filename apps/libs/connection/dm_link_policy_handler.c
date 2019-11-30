/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_link_policy_handler.c

DESCRIPTION
    This file contains the implementation of the link policy management entity.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_link_policy_handler.h"

#include <bdaddr.h>
#include <panic.h>
#include <stdlib.h>
#include <string.h>
#include <vm.h>
#include <sink.h>



/*****************************************************************************/
static void sendRoleCfmToClient(
        Task client,
        hci_status status,
        hci_role role,
        Sink sink
        )
{
    if (client)
    {
        MAKE_CL_MESSAGE(CL_DM_ROLE_CFM);
        message->status = status;
        message->role = role;
        message->sink = sink;
        MessageSend(client, CL_DM_ROLE_CFM, message);
    }
}


/*****************************************************************************/
static bool sendSetRoleRequest(Sink sink, hci_role role)
{
    tp_bdaddr    tpaddr;

    if (SinkGetBdAddr(sink, &tpaddr))
    {
        MAKE_PRIM_C(DM_HCI_SWITCH_ROLE_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
        prim->role = connectionConvertHciRole_t(role);
        VmSendDmPrim(prim);

        return TRUE;
    }
    return FALSE;
}


/*****************************************************************************/
static bool sendGetRoleRequest(Sink sink)
{
    tp_bdaddr    tpaddr;

    if (SinkGetBdAddr(sink, &tpaddr))
    {
        MAKE_PRIM_C(DM_HCI_ROLE_DISCOVERY_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
        VmSendDmPrim(prim);

        return TRUE;
    }
    return FALSE;
}


/****************************************************************************
Set the link supervision timeout on the ACL link identified by the specified
Bluetooth Device Address to the specified timeout value.
*/
static void setLinkSupervisionTimeout(const bdaddr* addr, uint16 timeout)
{
    MAKE_PRIM_C(DM_HCI_WRITE_LINK_SUPERV_TIMEOUT_REQ);
    prim->handle = 0;
    BdaddrConvertVmToBluestack(&prim->bd_addr, addr);
    prim->timeout = timeout;
    VmSendDmPrim(prim);
}


/****************************************************************************
The Power Table specifies the link policy power settings for the link
identified by the Bluetooth Device Address.
*/
static void processPowerTable(
        const bdaddr *addr,
        uint16 num_entries,
        lp_power_table const *power_table
        )
{
    /* This is a bit naughty since we're relying on the fact that the two
     * tables are identical, but they have been designed to be so for the sake
     * of efficiency go for it!
     */
    LP_POWERSTATE_T *my_power_table =
        (LP_POWERSTATE_T *)PanicUnlessMalloc(
                num_entries*sizeof(LP_POWERSTATE_T)
                );

    memmove(my_power_table, power_table, num_entries*sizeof(LP_POWERSTATE_T));

    {
        MAKE_PRIM_T(DM_LP_WRITE_POWERSTATES_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, addr);
        prim->num_states = num_entries;
        prim->states = VmGetHandleFromPointer(my_power_table);
        VmSendDmPrim(prim);
    }
}


/*****************************************************************************/
void connectionHandleLinkPolicySetRoleReq(
        connectionLinkPolicyState* linkPolicyState,
        const CL_INTERNAL_DM_SET_ROLE_REQ_T* req
        )
{
    if(!linkPolicyState->roleLock)
    {
        /* Set the requested role by sending a request to Bluestack*/
        if (!sendSetRoleRequest(req->sink, req->role))
            /* If the client passed us an invalid sink tell it */
            sendRoleCfmToClient(
                    connectionGetAppTask(),
                    hci_error_no_connection,
                    req->role,
                    req->sink
                    );
        else
            /* Otherwise set the resource lock */
            linkPolicyState->roleLock = req->sink;
    }
    else
    {
        /* Role request in progress, post a conditional internal
           message to queue request for processing later */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_ROLE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_SET_ROLE_REQ,
                message,
                (Task*) &linkPolicyState->roleLock
                );
    }
}


/*****************************************************************************/
void connectionHandleLinkPolicyGetRoleReq(
        connectionLinkPolicyState* linkPolicyState,
        const CL_INTERNAL_DM_GET_ROLE_REQ_T* req
        )
{
    if(!linkPolicyState->roleLock)
    {
        /* Get the requested role by sending a request to Bluestack*/
        if (!sendGetRoleRequest(req->sink))
            /* If the client passed us an invalid sink tell it */
            sendRoleCfmToClient(
                    connectionGetAppTask(),
                    hci_error_no_connection,
                    hci_role_dont_care,
                    req->sink
                    );
        else
            /* Otherwise set the resource lock */
            linkPolicyState->roleLock = req->sink;
    }
    else
    {
        /* Role request in progress, post a conditional internal
           message to queue request for processing later */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_GET_ROLE_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
                connectionGetCmTask(),
                CL_INTERNAL_DM_GET_ROLE_REQ,
                message,
                (Task*) &linkPolicyState->roleLock
                );
    }
}


/*****************************************************************************/
void connectionHandleDmSwitchRoleComplete(
        connectionLinkPolicyState *linkPolicyState,
        const DM_HCI_SWITCH_ROLE_CFM_T* ind
        )
{
    if(linkPolicyState->roleLock)
    {
        sendRoleCfmToClient(
                connectionGetAppTask(),
                connectionConvertHciStatus(ind->status),
                connectionConvertHciRole(ind->role),
                linkPolicyState->roleLock
                );

        /* Reset the lock */
        linkPolicyState->roleLock = 0;
    }
    else
    {
        hci_status status;
        hci_role role;
        bdaddr addr;

        MAKE_CL_MESSAGE(CL_DM_ROLE_IND);

        status = connectionConvertHciStatus(ind->status);
        role = connectionConvertHciRole(ind->role);
        BdaddrConvertBluestackToVm(&addr, &ind->bd_addr);

        message->status = status;
        message->role = role;
        message->bd_addr = addr;
        MessageSend(connectionGetAppTask(), CL_DM_ROLE_IND, message);
    }
}


/*****************************************************************************/
void connectionHandleRoleDiscoveryComplete(
        connectionLinkPolicyState* linkPolicyState,
        const DM_HCI_ROLE_DISCOVERY_CFM_T* ind
        )
{
    if(linkPolicyState->roleLock)
    {
        sendRoleCfmToClient(
                connectionGetAppTask(),
                connectionConvertHciStatus(ind->status),
                connectionConvertHciRole(ind->role),
                linkPolicyState->roleLock
                );

        /* Reset the lock */
        linkPolicyState->roleLock = 0;
    }
}


/*****************************************************************************/
void connectionHandleSetLinkSupervisionTimeoutReq(
        const CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ_T* req
        )
{
    tp_bdaddr tpaddr;

    if (SinkGetBdAddr(req->sink, &tpaddr))
        setLinkSupervisionTimeout(&tpaddr.taddr.addr, req->timeout);
}


/*****************************************************************************/
void connectionHandleLinkSupervisionTimeoutInd(
        const DM_HCI_LINK_SUPERV_TIMEOUT_IND_T* ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_LINK_SUPERVISION_TIMEOUT_IND);
    BdaddrConvertBluestackToVm(&message->bd_addr, &ind->bd_addr);
    message->timeout = ind->timeout;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_LINK_SUPERVISION_TIMEOUT_IND,
            message
            );
}


/*****************************************************************************/
void connectionHandleSetLinkPolicyReq(
        const CL_INTERNAL_DM_SET_LINK_POLICY_REQ_T* req
        )
{
    tp_bdaddr tpaddr;

    if (SinkGetBdAddr(req->sink, &tpaddr))
        processPowerTable(
                &tpaddr.taddr.addr,
                req->size_power_table,
                req->power_table
                );
}


/****************************************************************************/
void connectionLinkPolicyHandleWritePowerStatesCfm(
        const DM_LP_WRITE_POWERSTATES_CFM_T *cfm
        )
{
    /* The attempt to set a power table may have failed */
    if (
            (cfm->result != LP_POWERSTATES_SUCCESS) &&
            (cfm->result != LP_POWERSTATES_UNKNOWN_DEVICE)
       )
    {
        /* A client is passing in duff power tables - panic as low power
         * support will not work
         */
        Panic();
    }
}


/*****************************************************************************/
void connectionHandleSetSniffSubRatePolicyReq(
        connectionReadInfoState* infoState,
        const CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ_T* req
        )
{
    tp_bdaddr tpaddr;

    /* If we have a valid address and dev supports SSR */
    if (SinkGetBdAddr(req->sink, &tpaddr) && infoState->version >= bluetooth2_1)
    {
        MAKE_PRIM_C(DM_HCI_SNIFF_SUB_RATE_REQ);
        BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);
        prim->max_remote_latency = req->max_remote_latency;
        prim->min_remote_timeout = req->min_remote_timeout;
        prim->min_local_timeout = req->min_local_timeout;
        VmSendDmPrim(prim);
    }
}


/****************************************************************************/
void connectionHandleSniffSubRatingInd(const DM_HCI_SNIFF_SUB_RATING_IND_T *ind)
{
    /* tell the app we are sniff subrating */
    MAKE_CL_MESSAGE(CL_DM_SNIFF_SUB_RATING_IND);
    message->status = connectionConvertHciStatus(ind->status);
    BdaddrConvertBluestackToVm(&message->bd_addr, &ind->bd_addr);
    message->transmit_latency = ind-> transmit_latency;
    message->receive_latency = ind->receive_latency;
    message->remote_timeout = ind->remote_timeout;
    message->local_timeout = ind->local_timeout;
    MessageSend(connectionGetAppTask(), CL_DM_SNIFF_SUB_RATING_IND, message);
}

/*****************************************************************************/
void connectionHandleWriteLinkPolicySettingsReq(
        const CL_INTERNAL_DM_WRITE_LINK_POLICY_SETTINGS_REQ_T* req
        )
{
    MAKE_PRIM_C(DM_HCI_WRITE_LINK_POLICY_SETTINGS_REQ);
    BdaddrConvertVmToBluestack(&prim->bd_addr, &req->bd_addr);
    prim->link_policy_settings = req->link_policy_settings;
    VmSendDmPrim(prim);
}

