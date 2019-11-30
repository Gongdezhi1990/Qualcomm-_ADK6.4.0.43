/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_read_ble_ad_chan_tx_pwr.c

DESCRIPTION
    Message handling functions related to getting the BLE Advertising 
    Channel Transmit Power from Bluestack.

NOTES
    These functions use the same Connection Library 'txPwrLock' state to 
    store the App Task (pointer) as the BR/EDR functions to read the TX
    Power Level. 

*/

#ifndef DISABLE_BLE

/****************************************************************************
    Header files
*/
#include <connection.h>
#include <connection_private.h>
#include <common.h>
#include <dm_read_ble_ad_chan_tx_pwr.h>

#include <bdaddr.h>
#include <string.h>
#include <vm.h>

/****************************************************************************
NAME
    connectionHandleReadTxPower

DESCRIPTION
    Handle the internal message requesting a read of the current BLE 
    Advertising Channel Transmit Power.

RETURNS
    void
*/
void connectionHandleBleReadAdChanTxPwr(
        connectionReadTxPwrState *state, 
        const CL_INTERNAL_DM_BLE_READ_AD_CHAN_TX_PWR_REQ_T *req
        )
{
        /* Check the state of the task lock before doing anything */
        if (!state->txPwrLock)
        {
            /* One request at a time */
            state->txPwrLock = req->theAppTask;

            /* Issue request to read the tx power for the particular 
             * connection */
            {
                MAKE_PRIM_C(DM_HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_REQ);
                VmSendDmPrim(prim);
            }
        }
        else
        {
            /* Queue up the request until current request is completed. */
            MAKE_CL_MESSAGE(CL_INTERNAL_DM_BLE_READ_AD_CHAN_TX_PWR_REQ);
            COPY_CL_MESSAGE(req, message);
            MessageSendConditionallyOnTask(
                    connectionGetCmTask(), 
                    CL_INTERNAL_DM_READ_TX_POWER_REQ,
                    message, 
                    &state->txPwrLock
                    );
        }
}

/****************************************************************************
NAME
    connectionHandleBleReadAdChanTxPwrCfm

DESCRIPTION
    This function handles a confirmation containing the Advertising Channel
    Transmit Power (if successful) and the status of the request for that 
    value.

RETURNS
    void
*/
void connectionHandleReadBleAdChanTxPwrCfm(
        connectionReadTxPwrState *state, 
        const DM_HCI_ULP_READ_ADVERTISING_CHANNEL_TX_POWER_CFM_T *cfm
        )
{
    if(state->txPwrLock)
    {
        MAKE_CL_MESSAGE(CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM);
        message->tx_power = cfm->tx_power;
        message->status = connectionConvertHciStatus(cfm->status);
        MessageSend(
                state->txPwrLock, 
                CL_DM_BLE_READ_ADVERTISING_CHANNEL_TX_POWER_CFM,
                message
                );
        /* Message Scenario Complete - unlock for queue. */
        state->txPwrLock = 0;
    }
}

#else
#include <bdaddr_.h>
#endif
