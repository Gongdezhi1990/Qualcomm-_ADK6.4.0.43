/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    l2cap_connection_parameters_update.c

DESCRIPTION
    Implements ConnectionDmBleConnectionParametersUpdateReq().

NOTES
    Internal message and L2CA_CONNECTION_PAR_UPDATE_CFM are implemented in
    l2cap_handler.c
*/

#ifndef DISABLE_BLE

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"
#include "dm_ble_connection_parameters_update.h"

#include <vm.h>
#include <bdaddr.h>


/****************************************************************************
NAME    
    ConnectionDmBleConnectionParametersUpdateReq

DESCRIPTION
    Send an internal message, to ensure the task to return the status is 
    indicated and queue messages.

RETURNS
    void
*/
void ConnectionDmBleConnectionParametersUpdateReq(
        Task theAppTask,
        typed_bdaddr *taddr,
        uint16 min_interval,
        uint16 max_interval,
        uint16 latency,
        uint16 timeout,
        uint16 min_ce_length,
        uint16 max_ce_length
        )
{
    connectionState *state = (connectionState*)connectionGetCmTask();
    MAKE_CL_MESSAGE(
        CL_INTERNAL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_REQ);

    message->theAppTask = theAppTask;

    /* construct the L2CAP prim */
    message->taddr          = *taddr;
    message->min_interval   = min_interval;
    message->max_interval   = max_interval;
    message->latency        = latency;
    message->timeout        = timeout;
    message->min_ce_length  = min_ce_length;
    message->max_ce_length  = max_ce_length;

    /* Use fixed cid L2CAP lock because LE is in fact using a fixed cid.
     * And that lock isn't very busy and it's nice to avoid adding extra
     * lock variables consuming our precious memory. */
    MessageSendConditionallyOnTask(
        &state->task,
        CL_INTERNAL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_REQ,
        message,
        &state->l2capState.mapLock);
}


/****************************************************************************
NAME    
    connectionHandleDmBleConnectionParametersUpdateReq

DESCRIPTION
    Send the L2CA_CONNECTION_PAR_UPDATE_REQ.

RETURNS
    void
*/
void connectionHandleDmBleConnectionParametersUpdateReq(
    connectionL2capState *l2cap,
    const CL_INTERNAL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_REQ_T *req)
{
    MAKE_PRIM_C(DM_BLE_UPDATE_CONNECTION_PARAMETERS_REQ);

    /* Use fixed cid L2CAP lock because LE is in fact using a fixed cid.
     * And that lock isn't very busy and it's nice to avoid adding extra
     * lock variables consuming our precious memory. */
    l2cap->mapLock = req->theAppTask;

    /* Not used in VM API. */
    prim->connection_handle = 0;

    BdaddrConvertTypedVmToBluestack(&prim->addrt, &req->taddr);

    prim->conn_interval_min     = req->min_interval;
    prim->conn_interval_max     = req->max_interval;
    prim->conn_latency          = req->latency;
    prim->supervision_timeout   = req->timeout;
    prim->minimum_ce_length     = req->min_ce_length;
    prim->maximum_ce_length     = req->max_ce_length;
    
    VmSendDmPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleDmBleConnectionParametersUpdateReq

DESCRIPTION
    Send the CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM.

RETURNS
    void
*/
void connectionHandleDmBleUpdateConnectionParametersCfm(
    connectionL2capState *l2cap,
    const DM_BLE_UPDATE_CONNECTION_PARAMETERS_CFM_T *cfm)
{
    if (l2cap->mapLock)
    {
        MAKE_CL_MESSAGE(CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM);

        BdaddrConvertTypedBluestackToVm(&message->taddr, &cfm->addrt);

        message->status = connectionConvertHciStatus(cfm->status);

        MessageSend(l2cap->mapLock,
                    CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM,
                    message);

        /* release the lock */
        l2cap->mapLock = NULL;
    }
}

/****************************************************************************
NAME    
    connectionHandleDmBleAcceptConnectionParUpdateInd

DESCRIPTION
    Handle the DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND message from Bluestack
    and pass it on to the application that initialised the CL.

RETURNS
    void
*/
void connectionHandleDmBleAcceptConnectionParUpdateInd(
        const DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T *ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND);

    BdaddrConvertTypedBluestackToVm(&message->taddr, &ind->bd_addrt);
    message->id                     = ind->signal_id;
    message->conn_interval_min      = ind->conn_interval_min;
    message->conn_interval_max      = ind->conn_interval_max;
    message->conn_latency           = ind->conn_latency;
    message->supervision_timeout    = ind->supervision_timeout;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND,
            message);
}

/****************************************************************************
NAME    
    connectionHandleDmBleConnectionUpdateCompleteInd

DESCRIPTION
    Handle the DM_HCI_ULP_CONNECTION_UPDATE_COMPLETE_IND message from Bluestack
    and pass it on to the application that initialised the CL.

RETURNS
    void
*/
void connectionHandleDmBleConnectionUpdateCompleteInd(
        const DM_HCI_ULP_CONNECTION_UPDATE_COMPLETE_IND_T *ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND);

    BdaddrConvertTypedBluestackToVm(&message->taddr, &ind->addrt);
    message->status      = connectionConvertHciStatus(ind->status);
    message->conn_interval      = ind->conn_interval;
    message->conn_latency           = ind->conn_latency;
    message->supervision_timeout    = ind->supervision_timeout;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND,
            message);
}


/****************************************************************************
NAME    
    ConnectionDmBleAcceptConnectionParUpdateResponse

DESCRIPTION
    Called in response to the application receiving the 
    CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND message.

RETURNS
    void
*/
void ConnectionDmBleAcceptConnectionParUpdateResponse(
        bool                accept_update,
        const typed_bdaddr  *taddr,
        uint16              id,
        uint16              conn_interval_min,
        uint16              conn_interval_max,
        uint16              conn_latency,
        uint16              supervision_timeout
        )
{
    MAKE_PRIM_T(DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_RSP);

    BdaddrConvertTypedVmToBluestack(&prim->bd_addrt, taddr);
    prim->signal_id             = (l2ca_identifier_t)id;
    prim->conn_interval_min     = conn_interval_min;
    prim->conn_interval_max     = conn_interval_max;
    prim->conn_latency          = conn_latency;
    prim->supervision_timeout   = supervision_timeout;

    prim->result = (accept_update) ? 
        L2CAP_CONNECTION_PARAMETER_UPDATE_ACCEPT :
        L2CAP_CONNECTION_PARAMETER_UPDATE_REJECT;
    
    VmSendDmPrim(prim);
}

#else /* !DISBALE_BLE */
#   include <bdaddr_.h>
#endif 

/* End-of-File */
