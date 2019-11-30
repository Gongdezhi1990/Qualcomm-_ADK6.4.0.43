/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sdp_register.c        

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
void ConnectionRegisterServiceRecord(Task appTask, uint16 num_rec_bytes, const uint8 *service_record)
{
	/* Check some record has been supplied */
	if((num_rec_bytes == 0) || (service_record == NULL))
	{
		/* Return Failure if there is no service record passed */
		MAKE_CL_MESSAGE(CL_SDP_REGISTER_CFM);
		message->status = fail;
		message->service_handle = 0;
		MessageSend(appTask, CL_SDP_REGISTER_CFM, message);
        
	}
	else
	{
		/* Send an internal message to the state machine */
		MAKE_CL_MESSAGE(CL_INTERNAL_SDP_REGISTER_RECORD_REQ);
		message->theAppTask = appTask;
		message->record_length = num_rec_bytes;
			message->record = (uint8 *) service_record;
		MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_REGISTER_RECORD_REQ, message);
	}
}


/*****************************************************************************/
void ConnectionUnregisterServiceRecord(Task appTask, uint32 service_record_hdl)
{
	/* Create an internal message and send it to the state machine */
	MAKE_CL_MESSAGE(CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ);
	message->theAppTask = appTask;
	message->service_handle = service_record_hdl;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_SDP_UNREGISTER_RECORD_REQ, message);
}

#endif
