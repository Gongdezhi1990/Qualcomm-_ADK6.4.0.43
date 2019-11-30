/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    ConnectionSmEncryptionKeyRefresh.c        

DESCRIPTION
    This file contains the management entity responsible for device security

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include    <message.h>
#include    <string.h>
#include    <vm.h>


/*****************************************************************************/
void ConnectionSmEncryptionKeyRefresh(const typed_bdaddr* taddr)
{
#ifdef CONNECTION_DEBUG_LIB
    if(taddr == NULL)
    {
       CL_DEBUG(("Out of range Bluetooth Address 0x%p\n", (void*)taddr)); 
    }
#endif
    
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ);
        message->taddr = *taddr;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ, message); 
    }
}


