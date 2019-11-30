/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_dial_memory.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_link_manager.h"

#include <panic.h>
#include <string.h>


/****************************************************************************
NAME    
    HfpDialMemoryRequest

DESCRIPTION
    This function issues a request to the AG to dial from the the supplied 
    memory_location (the HFP specification defines the command but it is 
    AG implementation dependent how this is implemented). The request is 
    issued on the SLC associated with the hfp profile instance passed in by 
    the application. 
    
    The length argument specifies the length in bytes of the memory_location 
    to be sent. 
    
    The memory_location is the location the AG must dial from. The message 
    returned indicates whether the command was recognised by the AG or not.
    
MESSAGE RETURNED
    HFP_DIAL_MEMORY_CFM

RETURNS
    void
*/
void HfpDialMemoryRequest(hfp_link_priority priority, uint16 length, const uint8 *memory_location)
{
#ifdef HFP_DEBUG_LIB
    if (!length)
        HFP_DEBUG(("Zero length passed in.\n"));

    if (!memory_location)
        HFP_DEBUG(("Null memory location ptr passed in.\n"));
#endif

    /* Send an internal message */
    {
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_INTERNAL_AT_ATD_MEMORY_REQ, length);
        message->link = hfpGetLinkFromPriority(priority);
        message->length = length;
        memmove(message->memory, memory_location, length);
        MessageSend(&theHfp->task, HFP_INTERNAL_AT_ATD_MEMORY_REQ, message);
    }
}
