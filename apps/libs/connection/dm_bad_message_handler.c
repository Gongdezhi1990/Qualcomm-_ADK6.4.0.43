/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_bad_message_handler.c        

DESCRIPTION
	This file contains the functionality to deal with the DM_BAD_MESSAGE_IND
	that can be sent from BlueStack in response to messages out of state or
	out of sequence.

NOTES

*/

/****************************************************************************
	Header files
*/
#include <vm.h>

#include "connection.h"
#include "connection_private.h"
#include "dm_bad_message_handler.h"


/****************************************************************************
NAME
	connectionHandleDmBadMessageInd

FUNCTION
	This function is called to handle the DM_BAD_MESSAGE_IND prim from
	BlueStack. The 'message' handle must be freed. In debug mode, the prim
	id of the offending message is output.

RETURNS
	None
*/
void connectionHandleDmBadMessageInd(const DM_BAD_MESSAGE_IND_T *ind)
{
    uint16 *bad_message = (uint16 *)VmGetPointerFromHandle(ind->message);
    
#ifdef CONNECTION_DEBUG_LIB
    if (ind->message_length)
    {
        CL_DEBUG_INFO((
            "DM_BAD_MESSAGE_IND_T: Bad Message Prim 0x%04x\n", 
            bad_message[0]
            ));
    }
#endif

    free(bad_message);
}
