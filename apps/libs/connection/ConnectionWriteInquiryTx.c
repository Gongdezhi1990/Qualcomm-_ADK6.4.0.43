/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionWriteInquiryTx.c        

DESCRIPTION

NOTES
    An inquiry can only be initiated by one task at a time. If an inquiry
    request is received while the connection lib is already performing an 
    inquiry a CL_DM_INQUIRE_RESULT message is returned with status set to
    busy.

    Setting the Class of Device field to zero will turn off class of device
    filtering of inquiry results and all devices found will be returned.
*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <vm.h>
#include <string.h>

#ifndef CL_EXCLUDE_INQUIRY

/*****************************************************************************/
void ConnectionWriteInquiryTx(int8 tx_power)
{
#ifdef CONNECTION_DEBUG_LIB
    if ((tx_power < MIN_TX_POWER) || (tx_power > MAX_TX_POWER))
    {
        CL_DEBUG(("Out of range tx_power 0x%x\n", tx_power));
    }
#endif
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ);
        message->tx_power = tx_power;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_INQUIRY_TX_REQ, message);
    }
}

#endif

