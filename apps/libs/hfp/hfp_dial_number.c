/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    hfp_dial_number.c

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
    HfpDialNumberRequest

DESCRIPTION
    This function issues a request to the AG to dial the supplied number.
    The request is issued on the SLC associated with the hfp profile instance 
    passed in by the application. 
    
    The length argument specifies the length in bytes of the number to be sent. 
    
    The number is specified as an array and can include valid dial characters 
    such as '+'. The message returned indicates whether the command was 
    recognised by the AG or not. 

MESSAGE RETURNED
    HFP_DIAL_NUMBER_CFM

RETURNS
    void
*/
void HfpDialNumberRequest(hfp_link_priority priority, uint16 length, const uint8 *number)
{
#ifdef HFP_DEBUG_LIB
    if (!length)
        HFP_DEBUG(("Zero length passed in.\n"));

    if (!number)
        HFP_DEBUG(("Null number ptr passed in.\n"));
#endif

    /* Send an internal message */
    {
        MAKE_HFP_MESSAGE_WITH_LEN(HFP_INTERNAL_AT_ATD_NUMBER_REQ, length);
        message->link = hfpGetLinkFromPriority(priority);
        message->length = length;
        memmove(message->number, number, length);
        MessageSend(&theHfp->task, HFP_INTERNAL_AT_ATD_NUMBER_REQ, message);
    }
}
