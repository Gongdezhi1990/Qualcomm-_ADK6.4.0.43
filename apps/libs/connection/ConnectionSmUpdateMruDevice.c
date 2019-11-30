/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSmUpdateMruDevice.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "connection_tdl.h"

#include <message.h>
#include <string.h>
#include <vm.h>


/*****************************************************************************/
void ConnectionSmUpdateMruDevice(const bdaddr *bd_addr)
{
    /* Update the Trusted Device Index so that the specified device is recorded
       as the most recently used device */
    connectionAuthUpdateMru(bd_addr);
}


