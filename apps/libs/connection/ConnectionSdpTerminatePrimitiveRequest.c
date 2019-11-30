/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSdpTerminatePrimitiveRequest.c        

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
void ConnectionSdpTerminatePrimitiveRequest(Task appTask)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ);
    message->theAppTask = appTask;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_TERMINATE_PRIMITIVE_REQ, message);
}

#endif
