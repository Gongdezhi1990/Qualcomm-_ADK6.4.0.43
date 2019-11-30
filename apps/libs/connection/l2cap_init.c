/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    l2cap_init.c        

DESCRIPTION
    This file contains the functions to initialise the L2CAP component of 
    the connection library    

NOTES

*/

/****************************************************************************
    Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "init.h"
#include    "l2cap_init.h"


/****************************************************************************
NAME
    connectionL2capInit

DESCRIPTION
    This Function is called to initialise L2CAP

RETURNS
    void
*/
void connectionL2capInit(void)
{
    /* Confirm object has been initialised */
    connectionSendInternalInitCfm(connectionInitL2cap);
}
