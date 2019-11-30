/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_authorise.c        

DESCRIPTION
    This file contains the management entity responsible 
    Asynchronous Connection-Less (ACL) links

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"

#include    <message.h>
#include    <vm.h>

void ConnectionDmAclOpen( const bdaddr* bd_addr )
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_ACL_OPEN_REQ);
    message->bd_addr = *bd_addr;

    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_ACL_OPEN_REQ, message);
}        
  
/*****************************************************************************/
void ConnectionDmAclClose(const bdaddr* bd_addr, bool detach_all)
{   
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_ACL_CLOSE_REQ);
    message->bd_addr = *bd_addr;
	/* reason only required if force closing ACL */
    message->reason = 0;
    if (detach_all)
        message->flags = DM_ACL_FLAG_ALL;
    else
        message->flags = 0x00;

    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_ACL_CLOSE_REQ, message);
}  

/*****************************************************************************/
void ConnectionDmAclDetach(const bdaddr* bd_addr, hci_status reason, bool detach_all)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_DM_ACL_CLOSE_REQ);
    message->bd_addr = *bd_addr;
	/* reason required if force closing ACL */
    message->reason = reason;
    if (detach_all)
        message->flags = DM_ACL_FLAG_FORCE | DM_ACL_FLAG_ALL;
    else
        message->flags = DM_ACL_FLAG_FORCE;

    MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_ACL_CLOSE_REQ, message);
}



