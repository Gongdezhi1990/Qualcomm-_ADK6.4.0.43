/****************************************************************************
Copyright (c) 2011 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_ble_handler.c      

DESCRIPTION
    Handles with DM_HCI_ULP (BLE) prims from bluestack.

NOTES

*/

/****************************************************************************
    Header files
*/

#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_ble_handler.h"

#include <bdaddr.h>
#include <vm.h>
#include <string.h>
#include <app/bluestack/types.h>
#include <app/bluestack/dm_prim.h>

#ifndef DISABLE_BLE

/****************************************************************************
NAME    
    connectionHandleDmBleAdvertisingReportInd

DESCRIPTION
    Handles BLE report ind

RETURNS
    void
*/

void connectionHandleDmBleAdvertisingReportInd(
        const DM_HCI_ULP_ADVERTISING_REPORT_IND_T *ind
        )
{
    uint8 *temp = (uint8 *)VmGetPointerFromHandle(ind->data);
    MAKE_CL_MESSAGE_WITH_LEN(
            CL_DM_BLE_ADVERTISING_REPORT_IND,ind->length_data-1
            );

    message->num_reports = ind->num_reports;
    message->event_type = connectionConvertBleEventType(ind->event_type);

    BdaddrConvertTypedBluestackToVm(&message->current_taddr, &ind->current_addrt);
    BdaddrConvertTypedBluestackToVm(
            &message->permanent_taddr,
            &ind->permanent_addrt
            );

    message->rssi = ind->rssi;
    message->size_advertising_data = ind->length_data;

    memmove(message->advertising_data, temp, ind->length_data);
    free(temp);
    
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_ADVERTISING_REPORT_IND,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleSetAdvertisingDataCfm

DESCRIPTION
    Handles status of Advertising Data Request

RETURNS
    void
*/


void connectionHandleDmBleSetAdvertisingDataCfm(
        const DM_HCI_ULP_SET_ADVERTISING_DATA_CFM_T *cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_SET_ADVERTISING_DATA_CFM);
    message->status = (cfm->status) ? fail : success;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_SET_ADVERTISING_DATA_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleSetAdvertisingParametersCfm

DESCRIPTION
    Handles status of Advertising Parameters Request

RETURNS
    void
*/

void connectionHandleDmBleSetAdvertisingParamsCfm(
    const DM_HCI_ULP_SET_ADVERTISING_PARAMETERS_CFM_T *cfm)
{
    MAKE_CL_MESSAGE(CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM);
    message->status = (cfm->status) ? fail : success;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_SET_ADVERTISING_PARAMS_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmSetBleConnectionParametersCfm

DESCRIPTION
    Handles status of connection parameters request

RETURNS
    void
*/
void connectionHandleDmSetBleConnectionParametersCfm(
    const DM_SET_BLE_CONNECTION_PARAMETERS_CFM_T *m)
{
    MAKE_CL_MESSAGE(CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM);
    
    if (m->status == HCI_SUCCESS)
        message->status = success;
    else
        message->status = fail;
    
    MessageSend(connectionGetAppTask(),
                CL_DM_BLE_SET_CONNECTION_PARAMETERS_CFM,
                message);
}


/****************************************************************************
NAME    
    connectionHandleDmSetBleConnectionParametersCfm

DESCRIPTION
    Handles status of connection parameters request

RETURNS
    void
*/
void connectionHandleDmBleSetScanParamatersCfm(
        const DM_HCI_ULP_SET_SCAN_PARAMETERS_CFM_T *cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_SET_SCAN_PARAMETERS_CFM);
    message->status = (cfm->status == HCI_SUCCESS) ? success : fail;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_SET_SCAN_PARAMETERS_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleSetScanResponseDataCfm

DESCRIPTION
    Handles status of Set Scan Response Data Request

RETURNS
    void
*/
void connectionHandleDmBleSetScanResponseDataCfm(
        const DM_HCI_ULP_SET_SCAN_RESPONSE_DATA_CFM_T *cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM);
    message->status = (cfm->status) ? fail : success;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_SET_SCAN_RESPONSE_DATA_CFM, 
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleReadWhiteListSizeCfm

DESCRIPTION
    Handles the CFM to reading the size of the BLE White List. 

RETURNS
    void
*/
void connectionHandleDmBleReadWhiteListSizeCfm(
        const DM_HCI_ULP_READ_WHITE_LIST_SIZE_CFM_T* cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM);
    message->status = (cfm->status) ? fail : success;
    message->white_list_size = cfm->white_list_size;

    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_READ_WHITE_LIST_SIZE_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleClearWhiteListCfm

DESCRIPTION
    Handles the CFM for Clearing the BLE White List in the controller.

RETURNS
    void
*/
void connectionHandleDmBleClearWhiteListCfm(
        const DM_HCI_ULP_CLEAR_WHITE_LIST_CFM_T* cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_CLEAR_WHITE_LIST_CFM);
    message->status = (cfm->status) ? fail : success;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_CLEAR_WHITE_LIST_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleAddDeviceToWhiteListCfm

DESCRIPTION
    Handles the CFM for Adding a device to the White List in the controller.

RETURNS
    void
*/
void connectionHandleDmBleAddDeviceToWhiteListCfm(
        const DM_HCI_ULP_ADD_DEVICE_TO_WHITE_LIST_CFM_T* cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM);
    message->status = (cfm->status) ? fail : success;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_ADD_DEVICE_TO_WHITE_LIST_CFM,
            message
            );
}

/****************************************************************************
NAME    
    connectionHandleDmBleRemoveDeviceFromWhiteListCfm

DESCRIPTION
    Handles the CFM for removing a device from the White List in the 
    controller.

RETURNS
    void
*/
void connectionHandleDmBleRemoveDeviceFromWhiteListCfm(
        const DM_HCI_ULP_REMOVE_DEVICE_FROM_WHITE_LIST_CFM_T* cfm
        )
{
    MAKE_CL_MESSAGE(CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM);
    message->status = (cfm->status) ? fail : success;
    MessageSend(
            connectionGetAppTask(),
            CL_DM_BLE_REMOVE_DEVICE_FROM_WHITE_LIST_CFM,
            message
            );
}

#endif /* DISABLE_BLE */

/* End-of-File */
