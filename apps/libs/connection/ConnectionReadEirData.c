/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionReadEirData.c        

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
void ConnectionReadEirData(Task theAppTask)
{
    /* NOTE We do not need to check if the underlying bt version supports
     * EIR data, if it doesn't we would send the CL_DM_READ_EIR_DATA_CFM
     * with error status (hci_error_unsupported_feature) anyway
     */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_EIR_DATA_REQ);
    message->task = theAppTask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_EIR_DATA_REQ, message);
}

#endif
