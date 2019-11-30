/****************************************************************************
Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.

FILE NAME
    hid_connect.c
DESCRIPTION
    This file contains functions to handle (dis)connection of HID control and
    interrupt channels.
*/

#include "hid_private.h"
#include "hid.h"
#include "hid_common.h"
#include "hid_connect.h"
#include "hid_control_handler.h"

#include <app/bluestack/types.h>
#include <app/bluestack/bluetooth.h>
#include <app/bluestack/l2cap_prim.h>

/***********************************************************
NAME 
     HidConnect
DESCRIPTION
    Initiates connection.
RETURNS
     void
*/
void HidConnect(HID_LIB *hid_lib, Task task, const bdaddr *bd_addr, uint16 hid_conftab_length, const uint16 *hid_conftab)
{
    /* Send an internal message */
    MAKE_HID_MESSAGE(HID_INTERNAL_CONNECT_REQ);
    message->bd_addr = *bd_addr;
    message->app_task = task;
    message->hid_conftab_length = hid_conftab_length;
    message->hid_conftab = hid_conftab;
    MessageSend(&hid_lib->task, HID_INTERNAL_CONNECT_REQ, message);
}

/***********************************************************
NAME 
     HidConnectResponse
DESCRIPTION
     Handles connect response by sending an internal message
RETURNS
     void
*/
void HidConnectResponse(HID *hid, Task task, bool accept, uint16 hid_conftab_length, const uint16 *hid_conftab)
{
    /* Send an internal message */
    MAKE_HID_MESSAGE(HID_INTERNAL_CONNECT_RES);
    message->app_task = task;
    message->accept = accept;
    message->hid_conftab_length = hid_conftab_length;
    message->hid_conftab = hid_conftab;
    MessageSend(&hid->task, HID_INTERNAL_CONNECT_RES, message);
}

/****************************************************************************
NAME 
     HidDisconnect
DESCRIPTION
     Disconnect the service level connection for the supported profile.
RETURNS
     void
*/
void HidDisconnect(HID *hid)
{
    /* Send an internal message to kick off the disconnect */
    MessageSend(&hid->task, HID_INTERNAL_DISCONNECT_REQ, 0);
}

/****************************************************************************
NAME 
     hidHandleInternalConnectControlInd
DESCRIPTION
RETURNS
     void
*/
void hidHandleInternalConnectControlInd(HID *hid, const HID_INTERNAL_CONNECT_IND_T *ind)
{
    HID_PRINT(("hidHandleInternalConnectControlInd, hid=%p\n", (void *)hid));

    /* Move connection to connecting state */
    hidConnConnecting(hid, ind->psm, ind->connection_id, ind->identifier);

    /* Move to connecting state */
    hidSetState(hid, hidConnectingRemote);
}

/****************************************************************************
NAME 
     hidHandleInternalConnectInterruptInd
DESCRIPTION
RETURNS
     void
*/
void hidHandleInternalConnectInterruptInd(HID *hid, const HID_INTERNAL_CONNECT_IND_T *ind)
{
    HID_PRINT(("hidHandleInternalConnectInterruptInd, hid=%p\n", (void *)hid));
        
    /* Accept incoming connection - use application specified conftab */
    hidConnConnecting(hid, ind->psm, ind->connection_id, ind->identifier);
    ConnectionL2capConnectResponse(&hid->task, TRUE, HID_PSM_INTERRUPT,
                                   hid->connection[HID_CON_INTERRUPT].con.id, ind->identifier,
                                   hid->hid_conftab_length, hid->hid_conftab);
}

/****************************************************************************
NAME 
     hidRejectInternalConnectInd
DESCRIPTION
RETURNS
     void
*/
void hidRejectInternalConnectInd(HID *hid, const HID_INTERNAL_CONNECT_IND_T *ind)
{
    HID_PRINT(("hidRejectInternalConnectInd, hid=%p\n", (void *)hid));

    /* Reject incoming connection */
    ConnectionL2capConnectResponse(&hid->task, FALSE, ind->psm, ind->connection_id, ind->identifier, 0,0);            
}

/****************************************************************************
NAME 
     hidHandleInternalConnectRes
DESCRIPTION
RETURNS
     void
*/
void hidHandleInternalConnectRes(HID *hid, const HID_INTERNAL_CONNECT_RES_T *res)
{
    HID_PRINT(("hidHandleInternalConnectRes, hid=%p\n", (void *)hid));
    
    /* Check if application accepted control connection */
    if (res->accept)
    {
        HID_PRINT(("Accept connection, (hid=%p)\n", (void *)hid));
        
        /* Store pointer to connection configuration, update task */
        hid->hid_conftab_length = res->hid_conftab_length;
        hid->hid_conftab = res->hid_conftab;
        hid->app_task = res->app_task; 

        /* Accept or reject incoming connection. */
        ConnectionL2capConnectResponse(&hid->task, TRUE, HID_PSM_CONTROL,
                                       hid->connection[HID_CON_CONTROL].con.id, hid->connection[HID_CON_CONTROL].identifer,
                                       0, NULL);
    }
    else
    {
        HID_PRINT(("Reject connection, (destroy hid=%p)\n", (void *)hid));

        /* Clear task associated with connection */
        hid->app_task = 0;

        /* Reject incoming connection - B-91344 - will BS free the conftab? */
        ConnectionL2capConnectResponse(
            &hid->task, 
            FALSE, 
            HID_PSM_CONTROL,
            hid->connection[HID_CON_CONTROL].con.id, 
            hid->connection[HID_CON_CONTROL].identifer,
            res->hid_conftab_length,
            res->hid_conftab);
    }
}

/****************************************************************************
NAME 
     hidHandleInternalConnectTimeoutInd
DESCRIPTION
RETURNS
     void
*/
void hidHandleInternalConnectTimeoutInd(HID *hid)
{
    HID_PRINT(("hidHandleInternalConnectTimeoutInd, hid=%p\n", (void *)hid));

    /* Store connect failure reason */
    hid->connect_status = hid_connect_timeout;
        
    /* Move to disconnecting failed state */
    hidSetState(hid, hidDisconnectingFailed);
}

/****************************************************************************
NAME 
     hidHandleInternalConnectReq
DESCRIPTION
RETURNS
     void
*/
void hidHandleInternalConnectReq(HID_LIB *hid_lib, const HID_INTERNAL_CONNECT_REQ_T *req)
{
    HID *hid;
    HID_PRINT(("hidHandleInternalConnectReq, nap=%d, uap=%d, lap=%ld\n", req->bd_addr.nap, req->bd_addr.uap, req->bd_addr.lap));
    
    /* Attempt to allocate HID instance */
    hid = hidCreate(hid_lib, &req->bd_addr);
    if (hid != NULL)
    {
        HID_PRINT(("Control connection, outgoing (create hid=%p)\n", (void *)hid));
        
        /* Store pointer to connection configuration, update task */
        hid->hid_conftab_length = req->hid_conftab_length;
        hid->hid_conftab = req->hid_conftab;
        hid->app_task = req->app_task; 

        /* Move to connecting state */
        hidSetState(hid, hidConnectingLocal);
    }
    else
    {
        MAKE_HID_MESSAGE(HID_CONNECT_CFM);
        HID_PRINT(("Control connection, outgoing (creation failure)\n"));
        
        /* Send HID_CONNECT_CFM with resource failure */
        message->status = hid_connect_out_of_resources;
        message->interrupt_sink = 0;
        MessageSend(req->app_task, HID_CONNECT_CFM, message);
    } 
}

/****************************************************************************
NAME 
     hidHandleInternalDisconnectReq
DESCRIPTION
RETURNS
     void
*/
void hidHandleInternalDisconnectReq(HID *hid)
{
    HID_PRINT(("hidHandleInternalDisconnectReq, hid=%p\n", (void *)hid));
    
    /* Move to disconnecting local state */
    hidSetState(hid, hidDisconnectingLocal);
}

/****************************************************************************
NAME 
     hidHandleInternalDisconnectTimeoutInd
DESCRIPTION
    Called when we have timeout out waiting for the remote end to disconnect
    both channels, we just enter the disconnect local state which will 
    clean up any connections left.
RETURNS
     void
*/
void hidHandleInternalDisconnectTimeoutInd(HID *hid)
{
    HID_PRINT(("hidHandleInternalDisconnectTimeoutInd, hid=%p\n", (void *)hid));

    /* Set disconnect reason */
    hid->disconnect_status = hid_disconnect_timeout;

    /* Move to disconnecting failed state */
    hidSetState(hid, hidDisconnectingFailed);
}
