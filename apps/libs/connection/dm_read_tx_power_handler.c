/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_read_tx_power_handler.c

DESCRIPTION
    This file contains the implementation of the functions to hande transmit 
    power read msg.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_read_tx_power_handler.h"

#include <bdaddr.h>
#include <string.h>
#include <vm.h>

/****************************************************************************
NAME
    connectionHandleReadTxPower

DESCRIPTION
    This function will initiate a read of the tx power of the device

RETURNS
    void
*/
void connectionHandleReadTxPower(
                                connectionReadTxPwrState *state, 
                                const CL_INTERNAL_DM_READ_TX_POWER_REQ_T *req)
{
        /* Check the state of the task lock before doing anything */
        if (!state->txPwrLock)
        {
            /* One request at a time */
            state->txPwrLock = req->theAppTask;

            /* Issue request to read the tx power for the particular 
                connection */
            {
                MAKE_PRIM_C(DM_HCI_READ_TX_POWER_LEVEL_REQ);
                BdaddrConvertTpVmToBluestack(&prim->tp_addrt, &req->tpaddr);
                prim->type = 0; /* Get the current power */
                VmSendDmPrim(prim);
            }
        }
        else
        {
            /* Tx power request currently being performed, queue up the request
             */
            MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_TX_POWER_REQ);
            COPY_CL_MESSAGE(req, message);
            MessageSendConditionallyOnTask(connectionGetCmTask(), 
                                              CL_INTERNAL_DM_READ_TX_POWER_REQ,
                                              message, &state->txPwrLock);
        }
}

/****************************************************************************
NAME
    connectionHandleReadTxPowerComplete

DESCRIPTION
    This function handles a read tx result

RETURNS
    void
*/
void connectionHandleReadTxPowerComplete(
                            connectionReadTxPwrState *state, 
                            const DM_HCI_READ_TX_POWER_LEVEL_CFM_T *cfm)
{
    if(state->txPwrLock)
    {
        MAKE_CL_MESSAGE(CL_DM_READ_TX_POWER_CFM);
        message->status = connectionConvertHciStatus(cfm->status);
        BdaddrConvertTpBluestackToVm(&message->tpaddr, &cfm->tp_addrt);
        message->tx_power = cfm->pwr_level;
        MessageSend(state->txPwrLock, CL_DM_READ_TX_POWER_CFM, message);
        state->txPwrLock = 0;
    }
}

