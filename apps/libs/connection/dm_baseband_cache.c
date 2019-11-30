/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_baseband_cache.c        

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
void ConnectionWriteCachedPageMode(const bdaddr *addr, page_scan_mode ps_mode, page_scan_rep_mode ps_rep_mode)
{
    /* Send an internal message requesting this action */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ);
    message->bd_addr = *addr;
    message->ps_mode = ps_mode;
    message->ps_rep_mode = ps_rep_mode;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_CACHED_PAGE_MODE_REQ, message);
}


/*****************************************************************************/
void ConnectionWriteCachedClockOffset(const bdaddr *addr, uint16 clk_offset)
{
    /* Send an internal message requesting this action */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ);
    message->bd_addr = *addr;
    message->clock_offset = clk_offset;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_CACHED_CLK_OFFSET_REQ, message);
}


/*****************************************************************************/
void ConnectionClearParameterCache(const bdaddr *addr)
{
    /* Send an internal message requesting this action */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ);
    message->bd_addr = *addr;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_CLEAR_PARAM_CACHE_REQ, message);
}


