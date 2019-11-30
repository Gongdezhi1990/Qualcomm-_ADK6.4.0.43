/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionWritePagescanActivity.c        

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

/*****************************************************************************/
void ConnectionWritePagescanActivity(uint16 interval, uint16 window)
{
    /* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB    
    if ((interval < HCI_PAGESCAN_INTERVAL_MIN) || (interval > HCI_PAGESCAN_INTERVAL_MAX))
    {
        CL_DEBUG(("Out of range page scan interval 0x%x\n", interval));
    }

    if ((window < HCI_PAGESCAN_WINDOW_MIN) || (window > HCI_PAGESCAN_WINDOW_MAX))
    {
        CL_DEBUG(("Out of range page scan window 0x%x\n", window));
    }
#endif

    {
        /* All requests are sent through the internal state handler */
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ);
        message->ps_interval = interval;
        message->ps_window = window;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_PAGESCAN_ACTIVITY_REQ, message);
    }
}


