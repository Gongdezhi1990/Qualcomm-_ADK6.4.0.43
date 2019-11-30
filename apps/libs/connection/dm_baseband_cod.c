/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_baseband_cod.c        

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
#include    "common.h"

#include <message.h>
#include <panic.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionWriteClassOfDevice(uint32 cod)
{
    /* All requests are sent through the internal state handler */
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ);
    message->class_of_device = cod;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_CLASS_OF_DEVICE_REQ, message);
}


