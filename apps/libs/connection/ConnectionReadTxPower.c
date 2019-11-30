/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionReadTxPower.c

DESCRIPTION

NOTES
    This interface is used to read the transmit power of the bluetooth radio
	for a particular connection.
*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <vm.h>
#include <string.h>

/*****************************************************************************/
void ConnectionReadTxPower(Task theAppTask, const tp_bdaddr *tpaddr)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_TX_POWER_REQ);
   message->theAppTask = theAppTask;
   message->tpaddr = *tpaddr;
   MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_TX_POWER_REQ, message);
}

