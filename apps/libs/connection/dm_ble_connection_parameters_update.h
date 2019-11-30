/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_dm_ble_connection_parameters_update.h      

DESCRIPTION
    This file contains the prototypes for BLE Dm connection update functions.

NOTES

*/

#ifndef DISABLE_BLE
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
    const CL_INTERNAL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_REQ_T *req
    );

/****************************************************************************
NAME    
    connectionHandleDmBleConnectionParametersUpdateReq

DESCRIPTION
    Send the L2CA_CONNECTION_PAR_UPDATE_REQ.

RETURNS
    void
*/
void connectionHandleDmBleUpdateConnectionParametersCfm(
    connectionL2capState *l2cap,
    const DM_BLE_UPDATE_CONNECTION_PARAMETERS_CFM_T *cfm
    );

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
        );


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
        );

#endif
