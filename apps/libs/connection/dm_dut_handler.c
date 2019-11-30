/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_dut_handler.c        

DESCRIPTION    

NOTES

*/


/****************************************************************************
	Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "dm_dut_handler.h"

#include <vm.h>


/****************************************************************************/
void connectionSendDutCfmToClient(Task appTask, connection_lib_status result)
{
    if (appTask)
    {
        /* Send a cfm to the app task. */
        MAKE_CL_MESSAGE(CL_DM_DUT_CFM);
        message->status = result;
        MessageSend(appTask, CL_DM_DUT_CFM, message);
    }
}


/****************************************************************************/
void connectionHandleEnterDutModeReq(connectionReadInfoState *infoState)
{
	/* Make device discoverable and connectable */
	ConnectionWriteScanEnable(hci_scan_enable_inq_and_page);

	/* Disable security */

	if(infoState->version < bluetooth2_1)
	{
		ConnectionSmSetSecurityMode(connectionGetCmTask(), sec_mode0_off, hci_enc_mode_off);
	}
	else
	{
		MAKE_PRIM_C(DM_HCI_ENABLE_DUT_MODE_REQ);
		VmSendDmPrim(prim);
	}
}

/****************************************************************************/
void connectionHandleDutSecurityDisabled(Task appTask, const DM_SM_INIT_CFM_T *cfm)
{
	/* If we have successfully disabled security, enter DUT mode */
	if (cfm->status == HCI_SUCCESS)
	{
		MAKE_PRIM_C(DM_HCI_ENABLE_DUT_MODE_REQ);
		VmSendDmPrim(prim);
	}
	else
		connectionSendDutCfmToClient(appTask, fail);
}


/****************************************************************************/
void connectionHandleDutCfm(Task appTask, const DM_HCI_ENABLE_DUT_MODE_CFM_T *cfm)
{
	if (cfm->status == HCI_SUCCESS)
		connectionSendDutCfmToClient(appTask, success);
	else
		connectionSendDutCfmToClient(appTask, fail);
}

#ifndef DISABLE_BLE

/****************************************************************************/
void connectionSendUlpTransmitterTestCfmToClient(Task appTask, hci_status result)
{
    if (appTask)
    {
        /* Send a cfm to the app task. */
        MAKE_CL_MESSAGE(CL_DM_BLE_TRANSMITTER_TEST_CFM);
        message->status = result;
        MessageSend(appTask, CL_DM_BLE_TRANSMITTER_TEST_CFM, message);
    }
}

/****************************************************************************/
void connectionSendUlpReceiverTestCfmToClient(Task appTask, hci_status result)
{
    if (appTask)
    {
        /* Send a cfm to the app task. */
        MAKE_CL_MESSAGE(CL_DM_BLE_RECEIVER_TEST_CFM);
        message->status = result;
        MessageSend(appTask, CL_DM_BLE_RECEIVER_TEST_CFM, message);
    }
}

/****************************************************************************/
void connectionSendUlpTestEndCfmToClient(Task appTask, hci_status result, uint16 number_of_packets)
{
    if (appTask)
    {
        /* Send a cfm to the app task. */
        MAKE_CL_MESSAGE(CL_DM_BLE_TEST_END_CFM);
        message->status = result;
        message->number_of_rx_packets = number_of_packets;
        MessageSend(appTask, CL_DM_BLE_TEST_END_CFM, message);
    }
}

/****************************************************************************/
void connectionHandleUlpTransmitterTestReq(const CL_INTERNAL_DM_ULP_TRANSMITTER_TEST_REQ_T *message)
{
    MAKE_PRIM_C(DM_HCI_ULP_TRANSMITTER_TEST_REQ);
    prim->tx_channel = message->tx_channel;
    prim->length_test_data = message->length_test_data;
    prim->packet_payload = message->packet_payload;
    VmSendDmPrim(prim);
}

/****************************************************************************/
void connectionHandleUlpReceiverTestReq(const CL_INTERNAL_DM_ULP_RECEIVER_TEST_REQ_T *message)
{
    MAKE_PRIM_C(DM_HCI_ULP_RECEIVER_TEST_REQ);
    prim->rx_channel = message->rx_channel;
    VmSendDmPrim(prim);
}

/****************************************************************************/
void connectionHandleUlpTestEndReq(void)
{
    MAKE_PRIM_C(DM_HCI_ULP_TEST_END_REQ);
    VmSendDmPrim(prim);
}

#endif /* DISABLE_BLE */
