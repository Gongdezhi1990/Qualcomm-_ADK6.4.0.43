/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionReadBleAdvChannTxPwr.c

DESCRIPTION

NOTES
    API to read the BLE Avertising Channel Transmit Power.

*/


/****************************************************************************
    Header files
*/
#include <connection.h>
#include <connection_private.h>

#include <vm.h>
#include <string.h>

#ifndef DISABLE_BLE
/*****************************************************************************/
void ConnectionDmBleReadAdvertisingChannelTxPower(Task theAppTask)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_BLE_READ_AD_CHAN_TX_PWR_REQ);
   message->theAppTask = theAppTask;
   MessageSend(
           connectionGetCmTask(), 
           CL_INTERNAL_DM_BLE_READ_AD_CHAN_TX_PWR_REQ, 
           message
           );
}
#endif
