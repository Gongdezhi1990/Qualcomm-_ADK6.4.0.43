/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_advetising.h      

DESCRIPTION
    This file contains the prototypes for BLE DM advertising msgs from Bluestack .

NOTES

*/

/****************************************************************************
NAME    
    connectionHandleDmBleAdvParamUpdateInd

DESCRIPTION
    Handle the DM_ULP_ADV_PARAM_UPDATE_IND message from Bluestack and pass it
    on to the appliction that initialised the CL.

RETURNS
    void
*/
void connectionHandleDmBleAdvParamUpdateInd( 
        const DM_ULP_ADV_PARAM_UPDATE_IND_T *ind
        );

