/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_handler.h      

DESCRIPTION
    Deals with DM_HCI_ULP (BLE) prims from bluestack.

NOTES

*/

/****************************************************************************
NAME    
    connectionHandleBleSetScanCfm

DESCRIPTION
    Handles BlueStack cfm message for BLE Set Scan.

RETURNS
    void
*/

void connectionHandleDmBleAdvertisingReportInd(
        const DM_HCI_ULP_ADVERTISING_REPORT_IND_T *ind
        );

void connectionHandleDmBleSetAdvertisingDataCfm(
        const DM_HCI_ULP_SET_ADVERTISING_DATA_CFM_T *cfm
        );

void connectionHandleDmBleSetAdvertisingParamsCfm(
        const DM_HCI_ULP_SET_ADVERTISING_PARAMETERS_CFM_T *cfm
        );

void connectionHandleDmSetBleConnectionParametersCfm(
        const DM_SET_BLE_CONNECTION_PARAMETERS_CFM_T *m
        );

void connectionHandleDmBleSetScanParamatersCfm(
        const DM_HCI_ULP_SET_SCAN_PARAMETERS_CFM_T *cfm
        );

void connectionHandleDmBleSetScanResponseDataCfm(
        const DM_HCI_ULP_SET_SCAN_RESPONSE_DATA_CFM_T *cfm
        );

void connectionHandleDmBleReadWhiteListSizeCfm(
        const DM_HCI_ULP_READ_WHITE_LIST_SIZE_CFM_T* cfm
        );

void connectionHandleDmBleClearWhiteListCfm(
        const DM_HCI_ULP_CLEAR_WHITE_LIST_CFM_T* cfm
        );

void connectionHandleDmBleAddDeviceToWhiteListCfm(
        const DM_HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_CFM_T* cfm
        );

void connectionHandleDmBleRemoveDeviceFromWhiteListCfm(
        const DM_HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_CFM_T* cfm
        );
