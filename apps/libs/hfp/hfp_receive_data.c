/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    hfp_receive_data.c

DESCRIPTION
    

NOTES

*/


/****************************************************************************
    Header files
*/
#include "hfp.h"
#include "hfp_private.h"
#include "hfp_parse.h"
#include "hfp_receive_data.h"
#include "hfp_link_manager.h"

#include <panic.h>
#include <stream.h>
#include <print.h>

/****************************************************************************
NAME    
    hfpHandleReceivedData

DESCRIPTION
    Called when we get an indication from the firmware that there's more data 
    received and waiting in the RFCOMM buffer. Parse it.

RETURNS
    void
*/
void hfpHandleReceivedData(Source source)
{
    uint16 len;
    hfp_link_data* link = hfpGetLinkFromSink(StreamSinkFromSource(source));
    
    /* Ignore this if it's not for one of the HFP links */
    len = link ? SourceSize(source) : 0;
    
    /* Only bother parsing if there is something to parse */
    while (len > 0)
    {
        /* Keep parsing while we have data in the buffer */
        if (!parseSource(source, (Task)link))
            break;
        
        /* Check we have more data to parse */
        len = SourceSize(source);
    }
}
