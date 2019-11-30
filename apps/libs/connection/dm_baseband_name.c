/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_baseband_name.c        

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
#include	"common.h"

#include <message.h>
#include <panic.h>
#include <string.h>
#include <vm.h>


/************(****************************************************************/
void ConnectionChangeLocalName(uint16 size_local_name, const uint8 *local_name)
{
	/* Check params are within allowed values - debug build only */
#ifdef CONNECTION_DEBUG_LIB	
	if (size_local_name == 0 || size_local_name > MAX_NAME_LENGTH)
		CL_DEBUG(("Zero length name passed in\n"));

	if (!local_name)
		CL_DEBUG(("Zero length name passed in\n"));
#endif

    {
		/* All requests are sent through the internal state handler */
		MAKE_CL_MESSAGE(CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ);

		/* Check local name length is valid and less than MAX_NAME_LENGTH chars */
		if (size_local_name && size_local_name <= MAX_NAME_LENGTH)
		{
			uint16 name_length = 0;

			/* Make sure the name is null terminated */
			if (local_name[size_local_name-1] != '\0')
				name_length = size_local_name + 1;
			else
				name_length = size_local_name;

			message->length_name = name_length;
			message->name = (uint8 *)PanicUnlessMalloc(name_length);
			memmove(message->name, local_name, size_local_name);
			message->name[name_length-1] = '\0';
		}
		else
		{
			message->length_name = 0;
			message->name = 0;
		}
		
		MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_CHANGE_LOCAL_NAME_REQ, message);
	}
}


