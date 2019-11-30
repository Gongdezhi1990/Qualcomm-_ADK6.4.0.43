/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_baseband_flushtimeout.c        

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
void ConnectionWriteFlushTimeout(Sink sink, uint16 flush_timeout)
{
#ifdef CONNECTION_DEBUG_LIB    
    if (sink == 0)
        CL_DEBUG(("Invalid sink.\n"));
#endif

    /* 
        TODO B-4358 the flush timeout is set on a per ACL basis. If we allow 
        libs to set this individually this might result in inappropriate values
        being set if two profile libs have connections on the same ACL. However we don't
        really want a complicated arbitration mechanism like we have for low power since 
        not that many libs specifically need to set this - most profiles seem to say 
        something about setting this to an appropriate value. 
    */

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ);
        message->sink = sink;
        message->flush_timeout = flush_timeout;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_FLUSH_TIMEOUT_REQ, message);
    }
}
