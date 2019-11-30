/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_bad_message_handler.h       

DESCRIPTION
	This file contains the functionality to deal with the DM_BAD_MESSAGE_IND
	that can be sent from BlueStack in response to messages out of state or
	out of sequence.

NOTES

*/

#ifndef DM_BAD_MESSAGE_HANDLER_H_
#define DM_BAD_MESSAGE_HANDLER_H_

#include "connection_private.h"

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
void connectionHandleDmBadMessageInd(const DM_BAD_MESSAGE_IND_T *ind);


#endif

