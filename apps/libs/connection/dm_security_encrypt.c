/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_security_encrypt.c

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
#include    <sink.h>


/*****************************************************************************/
void ConnectionSmEncrypt(Task theAppTask, Sink sink, uint16 encrypt)
{
#ifdef CONNECTION_DEBUG_LIB
    if ((encrypt != TRUE) && (encrypt != FALSE))
    {
        CL_DEBUG(("Invalid value passin for the encrypt flag 0x%x\n", encrypt));
    }

    if(!sink)
    {
        CL_DEBUG(("Null sink passed in\n"));
    }
#endif

    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SM_ENCRYPT_REQ);
        message->theAppTask = theAppTask;
        message->sink = sink;
        message->encrypt = encrypt;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_SM_ENCRYPT_REQ, message);
    }
}


/*****************************************************************************/
void ConnectionSmEncryptionKeyRefreshSink(Sink sink)
{
#ifdef CONNECTION_DEBUG_LIB
    if(!sink)
    {
        CL_DEBUG(("Null sink passed in\n"));
    }
#endif

    {
        tp_bdaddr tpaddr;
        if (SinkGetBdAddr(sink, &tpaddr))
        {
            MAKE_CL_MESSAGE(CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ);
            message->taddr = tpaddr.taddr;
            MessageSend(
                    connectionGetCmTask(),
                    CL_INTERNAL_SM_ENCRYPTION_KEY_REFRESH_REQ,
                    message
                    );
        }
    }
}

/*****************************************************************************/
void ConnectionReadAPT(Task theAppTask, const tp_bdaddr* tpaddr)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_READ_APT_REQ);
   message->theAppTask = theAppTask;
   message->tpaddr = *tpaddr;
   MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_READ_APT_REQ, message);
}

/*****************************************************************************/
void ConnectionWriteAPT(Task theAppTask, 
                        const tp_bdaddr* tpaddr, 
                        uint16 apt, 
                        cl_apt_route_event apt_route_event)
{
   MAKE_CL_MESSAGE(CL_INTERNAL_DM_WRITE_APT_REQ);
   message->theAppTask = theAppTask;
   message->tpaddr = *tpaddr;
   message->apt = apt;
   message->apt_route_event = apt_route_event;
   MessageSend(connectionGetCmTask(), CL_INTERNAL_DM_WRITE_APT_REQ, message);
}
