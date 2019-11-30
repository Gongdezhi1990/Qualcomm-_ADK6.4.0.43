/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    rfc_init.c        

DESCRIPTION
    This file contains the functions to initialise the RFCOMM component of 
    the connection library    

NOTES

*/

/****************************************************************************
    Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    "rfc_init.h"

#include    <vm.h>


/****************************************************************************
NAME
connectionRfcInit

DESCRIPTION
This Function is called to initialise the RFCOMM protocol layer in Bluestack

RETURNS

*/
void connectionRfcInit(void)
{
    /* Send the RFCOMM init message. */
    {
        MAKE_PRIM_T(RFC_INIT_REQ);
		VmSendRfcommPrim(prim);
	}
}
