/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_link_policy.c        

DESCRIPTION
    This file contains the implementation of the link policy management 
    entity. This is responsible for arbitrating between the different low 
    power mode requirements of the connection library clients.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"


/*****************************************************************************/
void ConnectionSetRole(Task task, Sink sink, hci_role role)
{
#ifdef CONNECTION_DEBUG_LIB    
    if (sink == 0)
    {
        CL_DEBUG(("Invalid sink passed in\n"));
    }

    if (role > hci_role_dont_care)
    {
        CL_DEBUG(("Invalid role passed in\n"));
    }
#endif 

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_ROLE_REQ);
        message->theAppTask = task;
        message->sink = sink;
        message->role = role;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_SET_ROLE_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionSetLinkSupervisionTimeout(Sink sink, uint16 timeout)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ);
    message->sink = sink;
    message->timeout = timeout;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_SET_LINK_SUPERVISION_TIMEOUT_REQ, message);
}


/*****************************************************************************/
void ConnectionSetLinkPolicy(Sink sink, uint16 size_power_table, lp_power_table const *power_table)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_LINK_POLICY_REQ);
    message->sink = sink;
    message->size_power_table = size_power_table;
    message->power_table = power_table;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_SET_LINK_POLICY_REQ, message);
}

/*****************************************************************************/
void ConnectionWriteLinkPolicySettings(const bdaddr* bd_addr, const lp_settings lps)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_LINK_POLICY_SETTINGS_REQ);
    message->bd_addr = *bd_addr;
    message->link_policy_settings = lps;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_LINK_POLICY_SETTINGS_REQ, message);
}


/*****************************************************************************/
void ConnectionSetSniffSubRatePolicy(Sink sink, uint16 max_remote_latency, uint16 min_remote_timeout, uint16 min_local_timeout)
{
#ifdef CONNECTION_DEBUG_LIB
    if (connectionGetBtVersion() < bluetooth2_1)
    {
        CL_DEBUG(("Sniff subrating is not supported on (pre BT 2.1)\n"));
    }
    else
#endif
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ);
        message->sink = sink;
        message->max_remote_latency = max_remote_latency;
        message->min_remote_timeout = min_remote_timeout;
        message->min_local_timeout = min_local_timeout;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_SET_SNIFF_SUB_RATE_POLICY_REQ, message);
    }
}
