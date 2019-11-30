/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_dut.c        

DESCRIPTION    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"


/*****************************************************************************/
void ConnectionEnterDutMode(void)
{    
    /* All requests are sent through the internal state handler */    
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_DUT_REQ, 0);
}

#ifndef DISABLE_BLE
/****************************************************************************/
void ConnectionBleTransmitterTest(uint8 tx_channel, uint8 data_length, uint8 test_pattern)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_ULP_TRANSMITTER_TEST_REQ);
    message->tx_channel = tx_channel;
    message->length_test_data = data_length;
    message->packet_payload = test_pattern;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_ULP_TRANSMITTER_TEST_REQ, message);
}

/****************************************************************************/
void ConnectionBleReceiverTest(uint8 rx_channel)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_ULP_RECEIVER_TEST_REQ);
    message->rx_channel = rx_channel;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_ULP_RECEIVER_TEST_REQ, message);
}

/****************************************************************************/
void ConnectionBleTestEnd(void)
{
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_ULP_TEST_END_REQ, 0);
}

#endif /* DISABLE_BLE */
