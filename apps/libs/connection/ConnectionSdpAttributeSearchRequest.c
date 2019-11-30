/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSdpAttributeSearchRequest.c        

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
#define REF_FLAG 0x8000 /* Pass result as a reference instead of a value */
void ConnectionSdpAttributeSearchRequest(Task appTask, const bdaddr *bd_addr, uint16 max_num_recs, uint32 service_hdl, uint16 size_attribute_list, const uint8 *attribute_list)
{
#ifdef CONNECTION_DEBUG_LIB
    if (size_attribute_list == 0)
        CL_DEBUG(("sdp - attribute search pattern not supplied\n"));
    if (max_num_recs == 0)
        CL_DEBUG(("sdp - max number of attribute bytes set to zero\n"));
    if(bd_addr == NULL)
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)bd_addr));
#endif

    {
        MAKE_CL_MESSAGE_WITH_LEN(CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, size_attribute_list);

        /* Get reference flag from max_num_recs */
        if (max_num_recs & REF_FLAG)
        {
            message->flags = CONNECTION_FLAG_SDP_REFERENCE;
            max_num_recs &= (uint16)~REF_FLAG;
        }
        else
        {
            message->flags = 0;
        }

        message->theAppTask = appTask;
        message->bd_addr = *bd_addr;
        message->service_handle = service_hdl;
        message->size_attribute_list = size_attribute_list;
        memmove(message->attribute_list, attribute_list, size_attribute_list);
        message->max_num_attr = max_num_recs;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_ATTRIBUTE_SEARCH_REQ, message);
    }
}

void ConnectionSdpAttributeSearchRefRequest(Task appTask, const bdaddr *bd_addr, uint16 max_num_recs, uint32 service_hdl, uint16 size_attribute_list, const uint8 *attribute_list)
{
    ConnectionSdpAttributeSearchRequest(
        appTask,
        bd_addr,
        max_num_recs | REF_FLAG,
        service_hdl,
        size_attribute_list,
        attribute_list);
}
#endif
