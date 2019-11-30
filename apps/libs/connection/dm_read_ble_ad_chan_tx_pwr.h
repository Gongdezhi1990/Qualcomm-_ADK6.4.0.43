/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_read_ble_ad_chan_tx_pwr.h
    
DESCRIPTION

*/
#ifndef DISABLE_BLE

#ifndef CONNECTION_DM_READ_AD_CHAN_TX_PWR_H_
#define CONNECTION_DM_READ_AD_CHAN_TX_PWR_H_


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
        );


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
        );


#endif  /* CONNECTION_DM_READ_AD_CHAN_TX_PWR_H_ */
#endif  /* DISABLE_BLE */
