/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionReadInquiryTx.c        

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
void ConnectionReadInquiryTx(Task theAppTask)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_INQUIRY_TX_REQ);
   message->theAppTask = theAppTask;
   MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_INQUIRY_TX_REQ, message);
}

#endif


