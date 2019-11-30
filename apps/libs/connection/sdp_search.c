/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_search.c        

DESCRIPTION
        

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include <panic.h>
#include <string.h>

#ifndef CL_EXCLUDE_SDP

/*****************************************************************************/
void ConnectionSdpOpenSearchRequest(Task appTask, const bdaddr* bd_addr)
{
#ifdef CONNECTION_DEBUG_LIB
    if(bd_addr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
    }
#endif

    {
        /* Send an internal message */
        MAKE_CL_MESSAGE(CL_INTERNAL_SDP_OPEN_SEARCH_REQ);
        message->theAppTask = appTask;
        message->bd_addr = *bd_addr;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_OPEN_SEARCH_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionSdpCloseSearchRequest(Task appTask)
{
    /* Send an internal message */
    MAKE_CL_MESSAGE(CL_INTERNAL_SDP_CLOSE_SEARCH_REQ);
    message->theAppTask = appTask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_CLOSE_SEARCH_REQ, message);    
}


/*****************************************************************************/
#define REF_FLAG 0x8000 /* Pass result as a reference instead of a value */
void ConnectionSdpServiceSearchRequest(Task appTask, const bdaddr *bd_addr, uint16 max_num_recs, uint16 size_srch_pttrn, const uint8 *search_pattern)
{
#ifdef CONNECTION_DEBUG_LIB
    if (size_srch_pttrn == 0)
        CL_DEBUG(("sdp - search pattern not supplied\n"));
    if (max_num_recs == 0)
        CL_DEBUG(("sdp - max number of records set to zero\n"));
    if(bd_addr == NULL)
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
#endif

    {
        /* Create an internal message and send it */
        MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, size_srch_pttrn);
        message->theAppTask = appTask;
        message->bd_addr = *bd_addr;
        message->max_responses = max_num_recs;
        message->length = size_srch_pttrn;

        /* Get reference flag from max_num_recs */
        if (max_num_recs & REF_FLAG)
        {
            message->flags = CONNECTION_FLAG_SDP_REFERENCE;
            max_num_recs &= ~REF_FLAG;
        }
        else
        {
            message->flags = 0;
        }
        

        if (size_srch_pttrn)
            memmove(message->search_pattern, search_pattern, size_srch_pttrn);
        else
            message->search_pattern[0] = 0;
    
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_SERVICE_SEARCH_REQ, message);
    }
}

void ConnectionSdpServiceSearchRefRequest(Task appTask, const bdaddr *bd_addr, uint16 max_num_recs, uint16 size_srch_pttrn, const uint8 *search_pattern)
{
    ConnectionSdpServiceSearchRequest(
        appTask,
        bd_addr,
        max_num_recs | REF_FLAG,
        size_srch_pttrn,
        search_pattern);
}

#endif





