/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_read_tx_power_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_READ_TX_POWER_HANDLER_H_
#define    CONNECTION_DM_READ_TX_POWER_HANDLER_H_


/****************************************************************************
NAME
    connectionHandleReadTxPower

DESCRIPTION
    This function will initiate a read of the tx power of the device

RETURNS
    void
*/
void connectionHandleReadTxPower(connectionReadTxPwrState *state,
                                const CL_INTERNAL_DM_READ_TX_POWER_REQ_T *req);

/****************************************************************************
NAME	
    connectionHandleReadTxPowerComplete

DESCRIPTION
    This function handles a read tx result

RETURNS
    void
*/
void connectionHandleReadTxPowerComplete(connectionReadTxPwrState *state,
                                const DM_HCI_READ_TX_POWER_LEVEL_CFM_T *cfm);


#endif    /* CONNECTION_DM_READ_TX_POWER_HANDLER_H_ */
