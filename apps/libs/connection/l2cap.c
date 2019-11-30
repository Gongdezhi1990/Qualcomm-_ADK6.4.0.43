/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    l2cap.c        

DESCRIPTION
	File containing the l2cap API function implementations.	

NOTES

*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include    "connection_private.h"
#include    <app/bluestack/l2cap_prim.h>

#ifndef CL_EXCLUDE_L2CAP

/*****************************************************************************/
void ConnectionL2capRegisterRequest(Task clientTask, uint16 psm, uint16 flags)
{
    MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_REGISTER_REQ);
    message->clientTask = clientTask;
	message->app_psm = psm;
    message->flags = flags;
    MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_REGISTER_REQ, message);

}


/*****************************************************************************/
void ConnectionL2capConnectRequest(Task appTask, const bdaddr *addr, uint16 psm_local, uint16 psm_remote, uint16 conftab_length, const uint16* conftab)
{
	/* Check a non null address ptr has been passed in */
#ifdef CONNECTION_DEBUG_LIB
	if (!addr)
		CL_DEBUG(("Null address ptr passed in.\n"));
#endif

	{
	/* Send an internal message */
		MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_CONNECT_REQ);
		message->theAppTask = appTask;
		message->bd_addr = *addr;
		message->psm_local = psm_local;
		message->psm_remote = psm_remote;
        message->conftab_length = conftab_length;
        message->conftab = conftab;

		MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_REQ, message);
	}
}

/*****************************************************************************/
void ConnectionL2capTpConnectRequest(Task appTask, const tp_bdaddr *taddr, uint16 psm_local, uint16 psm_remote, uint16 conftab_length, uint16* conftab)
{
    /* Check a non null address ptr has been passed in */
#ifdef CONNECTION_DEBUG_LIB
    if (!taddr)
        CL_DEBUG(("Null address ptr passed in.\n"));	
#endif

    {
    /* Send an internal message */
        MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_TP_CONNECT_REQ);
        message->theAppTask = appTask;
        message->taddr = *taddr;
        message->psm_local = psm_local;
        message->psm_remote = psm_remote;
        message->length = conftab_length;
        message->data = conftab;

        MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_TP_CONNECT_REQ, message);
    }
}

/*****************************************************************************/
void ConnectionL2capAddCreditRequest(Task appTask, uint16 connection_id, uint16 credits)
{
    /* Send an internal message */
    MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_ADD_CREDIT_REQ);
    message->theAppTask = appTask;
    message->connection_id = connection_id;
    message->credits = credits;

    MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_ADD_CREDIT_REQ, message);
}

/*****************************************************************************/
void ConnectionL2capConnectResponse(Task appTask, bool response, uint16 psm, uint16 connection_id, uint8 identifier, uint16 conftab_length, const uint16* conftab)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_CONNECT_RES);
	message->theAppTask = appTask;
	message->response = response;
    message->identifier = identifier;
	message->psm_local = psm;
	message->connection_id = connection_id;
    message->conftab_length = conftab_length;
    message->conftab = conftab;

	MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_CONNECT_RES, message);
}

/*****************************************************************************/
void ConnectionL2capTpConnectResponse(Task appTask, bool response, uint16 psm, uint16 connection_id, uint8 identifier, uint16 conftab_length, uint16* conftab)
{
    /* Send an internal message */
    MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_TP_CONNECT_RES);
    message->theAppTask = appTask;
    message->response = response;
    message->identifier = identifier;
    message->psm_local = psm;
    message->connection_id = connection_id;
    message->length = conftab_length;
    message->data = conftab;

    MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_TP_CONNECT_RES, message);
}

/*****************************************************************************/
void ConnectionL2capMapConnectionlessRequest(Task theAppTask, const bdaddr * addr, uint16 psm_local, uint16 psm_remote, l2cap_connectionless_data_type type)
{
    
    /* Check a non null address ptr has been passed in */
#ifdef CONNECTION_DEBUG_LIB
    if (!addr)
        CL_DEBUG(("Null address ptr passed in.\n"));
#endif

    {
        /* Send an internal message */
        MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ);
        message->theAppTask = theAppTask;
        message->bd_addr = *addr;
        message->psm_local = psm_local;
        message->psm_remote = psm_remote;
        message->type = type;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ, message);
    }
}

/*****************************************************************************/
void ConnectionL2capMapConnectionlessResponse(Task theAppTask, Source source, l2cap_connectionless_data_type type)
{
    if (source)
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_RES);
        message->theAppTask = theAppTask;
        message->source = source;
        message->type = type;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_RES, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Source is NULL!\n"));
    }
#endif
}

/*****************************************************************************/
void ConnectionL2capUnmapConnectionlessRequest(Sink sink)
{
    if (sink)
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_UNMAP_CONNECTIONLESS_REQ);
        message->sink = sink;
        MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_UNMAP_CONNECTIONLESS_REQ, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Sink is NULL!\n"));
    }
#endif
}

/*****************************************************************************/
void ConnectionL2capDisconnectRequest(Task appTask, Sink sink)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_DISCONNECT_REQ);
	message->theAppTask = appTask;	
	message->sink = sink;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_DISCONNECT_REQ, message);
}

/*****************************************************************************/
void ConnectionL2capDisconnectResponse(uint8 identifier, Sink sink)
{
	/* Send an internal message */
	MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_DISCONNECT_RSP);
    message->identifier = identifier;
	message->sink = sink;
	MessageSend(connectionGetCmTask(), CL_INTERNAL_L2CAP_DISCONNECT_RSP, message);
}

#endif
