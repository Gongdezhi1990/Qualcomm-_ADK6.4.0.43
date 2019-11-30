/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionWriteInquiryscanActivity.c        

DESCRIPTION
    This file contains the implementation of the entity responsible for 
    configuring the local baseband. This includes making the device
    discoverable, connectable etc.

NOTES

*/


/****************************************************************************
    Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "common.h"

#include <message.h>
#include <panic.h>
#include <string.h>
#include <vm.h>

#ifndef CL_EXCLUDE_INQUIRY_SCAN

/*****************************************************************************/
void ConnectionWriteInquiryscanActivity(uint16 interval, uint16 window)
{
    /* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB    
    if ((interval < HCI_INQUIRYSCAN_INTERVAL_MIN) || (interval > HCI_INQUIRYSCAN_INTERVAL_MAX))
    {
        CL_DEBUG(("Out of range inquiry scan interval 0x%x\n", interval));
    }

    if ((window < HCI_INQUIRYSCAN_WINDOW_MIN) || (window > HCI_INQUIRYSCAN_WINDOW_MAX))
    {
        CL_DEBUG(("Out of range inquiry scan window 0x%x\n", window));
    }
#endif    
    
    {
        /* All requests are sent through the internal state handler */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ);
        message->is_interval = interval;
        message->is_window = window;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_INQSCAN_ACTIVITY_REQ, message);
    }
}

#endif

