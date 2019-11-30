/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionWriteInquiryAccessCode.c        

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

#ifndef CL_EXCLUDE_INQUIRY_SCAN

/*****************************************************************************/
void ConnectionWriteInquiryAccessCode(Task theAppTask, const uint32 *iac, uint16 num_iac)
{
    /* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB
    if ((num_iac < MIN_WRITE_IAC_LAP) || (num_iac > MAX_WRITE_IAC_LAP))
    {
        CL_DEBUG(("Out of range num_iac 0x%x\n", num_iac));
    }
#endif

    {
        MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_DM_WRITE_IAC_LAP_REQ, sizeof(uint32) * num_iac);
        message->theAppTask = theAppTask;
        message->num_iac = num_iac;
        memmove(&message->iac, iac, sizeof(uint32) * num_iac);

        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_IAC_LAP_REQ, message);
    }
}

#endif
