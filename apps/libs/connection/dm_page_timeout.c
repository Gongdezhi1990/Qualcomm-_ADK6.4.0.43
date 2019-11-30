/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_page_timeout.c        

DESCRIPTION
    This file contains the functions allowing the page timeout for the device to be configured.

NOTES

*/

/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <message.h>

/*****************************************************************************/
void ConnectionSetPageTimeout(uint16 page_timeout)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_PAGE_TIMEOUT_REQ);
    /* If page timeout is zero use HCI default timeout. Interval and retries not used */
    if (!page_timeout)
        message->timeout = 0x2000;
    else
        message->timeout = page_timeout;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_PAGE_TIMEOUT_REQ, message);
}

