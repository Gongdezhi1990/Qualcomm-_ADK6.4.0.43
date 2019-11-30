/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_dut_handler.h
    
DESCRIPTION

*/

#ifndef    CONNECTION_DM_DUT_HANDLER_H_
#define    CONNECTION_DM_DUT_HANDLER_H_


/****************************************************************************
NAME    
    connectionHandleEnterDutModeReq

DESCRIPTION
    Enter device under test (DUT) mode.

*/
void connectionHandleEnterDutModeReq(connectionReadInfoState *infoState);


/****************************************************************************
NAME    
    connectionHandleDutSecurityDisabled

DESCRIPTION
    Called when we have a confirmation that security mode has been disabled.

*/
void connectionHandleDutSecurityDisabled(Task appTask, const DM_SM_INIT_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionHandleDutCfm

DESCRIPTION
    We've received a cfm from BlueStack indicating the outcome of the attempt
    to enter DUT mode.

*/
void connectionHandleDutCfm(Task appTask, const DM_HCI_ENABLE_DUT_MODE_CFM_T *cfm);


/****************************************************************************
NAME    
    connectionSendDutCfmToClient

DESCRIPTION
    Send a confirm message to the client (the task that initialised the 
    connection lib) indicating the outcome of the request to enter DUT mode.

*/
void connectionSendDutCfmToClient(Task appTask, connection_lib_status result);

#ifndef DISABLE_BLE

/****************************************************************************
NAME
    connectionSendUlpTransmitterTestCfmToClient

DESCRIPTION
    Send a confirm message to the client (the task that initialised the
    connection lib) indicating the outcome of the request to enter BLE
    transmitter test mode.

*/
void connectionSendUlpTransmitterTestCfmToClient(Task appTask, hci_status result);

/****************************************************************************
NAME
    connectionSendUlpReceiverTestCfmToClient

DESCRIPTION
    Send a confirm message to the client (the task that initialised the
    connection lib) indicating the outcome of the request to enter BLE
    receiver test mode.

*/
void connectionSendUlpReceiverTestCfmToClient(Task appTask, hci_status result);

/****************************************************************************
NAME
    connectionSendUlpTestEndCfmToClient

DESCRIPTION
    Send a confirm message to the client (the task that initialised the
    connection lib) indicating the outcome of the request to end BLE
    test mode.

*/
void connectionSendUlpTestEndCfmToClient(Task appTask, hci_status result, uint16 number_of_packets);

/****************************************************************************
NAME
    connectionHandleUlpTransmitterTestReq

DESCRIPTION
    Enter BLE transmitter test mode.

*/
void connectionHandleUlpTransmitterTestReq(const CL_INTERNAL_DM_ULP_TRANSMITTER_TEST_REQ_T *message);

/****************************************************************************
NAME
    connectionHandleUlpReceiverTestReq

DESCRIPTION
    Enter BLE receiver test mode.

*/
void connectionHandleUlpReceiverTestReq(const CL_INTERNAL_DM_ULP_RECEIVER_TEST_REQ_T *message);

/****************************************************************************
NAME
    connectionHandleUlpTestEndReq

DESCRIPTION
    End BLE test mode.

*/
void connectionHandleUlpTestEndReq(void);

#endif /* DISABLE_BLE */

#endif    /* CONNECTION_DM_DUT_HANDLER_H_ */
