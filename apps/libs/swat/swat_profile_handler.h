/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    swat_profile_handler.c

DESCRIPTION
    File containing the profile handler functions for the swat library.
*/


#ifndef SWAT_PROFILE_HANDLER_H_
#define SWAT_PROFILE_HANDLER_H_


/****************************************************************************
    Header files
*/
#include "swat.h"
#include "swat_private.h"


/****************************************************************************
NAME
	swatProfileHandler

DESCRIPTION
    This function handles all messages sent to the swat profile library
    task. 
*/
void swatProfileHandler(Task task, MessageId id, Message message);


#endif /* SWAT_PROFILE_HANDLER_H_ */
