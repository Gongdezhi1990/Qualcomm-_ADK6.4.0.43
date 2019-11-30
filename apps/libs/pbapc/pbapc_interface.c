/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_interface.c
    
DESCRIPTION
    PhoneBook Access Profile Client Library - public interface.

*/
#include <bdaddr.h>
#include "pbapc_extern.h"
#include "pbapc_private.h"

/* define a global pointer array to pbapcState */
PBAPC *Pbapc[MAX_PBAPC_CONNECTIONS] = {NULL, NULL};   

/***************************************************************************
 * NAME
 *  pbapcSendAuthChallenge
 *
 * DESCRIPTION
 *  Send the Authentication Challenge String
 **************************************************************************/
static void pbapcSendAuthChallenge( PBAPC *pbapc, 
                                    const uint8 *nonce)
{
    if( nonce )
    {
        MAKE_PBAPC_MESSAGE(PBAPC_INT_AUTH_CLG);
        memmove( message->nonce, nonce, PBAPC_OBEX_SIZE_DIGEST);
        MessageSend((Task)pbapc, PBAPC_INT_AUTH_CLG, message);
    }
    else
    {
         MessageSend((Task)pbapc, PBAPC_INT_AUTH_CLG, PBAPC_NO_PAYLOAD);
    }
}

/***************************************************************************
 * NAME
 *  pbapcSendAuthResponse
 *
 * DESCRIPTION
 *  Send the Authentication Response String
 **************************************************************************/
static void pbapcSendAuthResponse( PBAPC *pbapc, 
                                    const uint8 *digest, 
                                    uint16 sizeUserId, 
                                    const uint8 *userId ) 
{
    MAKE_PBAPC_MESSAGE_WITH_LEN(PBAPC_INT_AUTH_RESP, sizeUserId );
    memmove(message->digest, digest, PBAPC_OBEX_SIZE_DIGEST);
    message->sizeUserId = sizeUserId;
    if(sizeUserId) memmove(message->userId, userId, sizeUserId ); 
    MessageSend((Task)pbapc, PBAPC_INT_AUTH_RESP, message);
}

/***************************************************************************
 * NAME
 *  PbapcGetLinkFrmAddr
 *
 * DESCRIPTION
 *  Get the device_id of PBAPC link connected to bdAddr
 **************************************************************************/
uint16 PbapcGetLinkFrmAddr(const bdaddr *bdAddr)
{
    uint16 device_id = pbapc_invalid_link;
    
    for(device_id = 0; device_id < MAX_PBAPC_CONNECTIONS; device_id++)
    {
        if(Pbapc[device_id] && BdaddrIsSame(&(Pbapc[device_id]->bdAddr), bdAddr))
            return(device_id);
    }
    
    return(device_id);
}

/***************************************************************************
 * NAME
 *  pbapcAddPbapDevice
 *
 * DESCRIPTION
 *  Add a new PBAPC link to remote device with bdAddr
 **************************************************************************/
static bool pbapcAddPbapDevice(const bdaddr *bdAddr, pbapc_link_priority *priority)
{
    *priority = pbapc_invalid_link;
    
    if(PbapcGetNoOfConnection() < 2)
    {
        /* check whether the device has been connected or connecting */
        *priority = PbapcGetLinkFrmAddr(bdAddr);
        
        if(*priority == pbapc_invalid_link)
        {
            uint8 device_id = 0;
        
            for(device_id = 0; device_id < MAX_PBAPC_CONNECTIONS; device_id++)
            {
                 if( !Pbapc[device_id] || 
                     !((Pbapc[device_id])->connect_state == pbapc_connected || 
                       (Pbapc[device_id])->connect_state == pbapc_connecting) )
                 {
                    *priority = (pbapc_link_priority)device_id;
                    return TRUE;
                }
            }            
        }
        else if((Pbapc[*priority])->connect_state == pbapc_connected || 
                (Pbapc[*priority])->connect_state == pbapc_connecting)
        {
            /* This device has been connected or connecting, don't try to reconnect it */
            return FALSE;
        }
    }
    
    return FALSE;
}   

/****************************************************************************
 * NAME
 *  PbapcInit
 *
 * DESCRIPTION
 *  Initialize the PBAP Client library
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcInit( Task theAppTask )
{
    PBAPC_LOG(("PBAPC Init\n"));
    MessageSend( PanicNull( pbapcCreateTask(theAppTask) ), 
                 PBAPC_INT_REG_SDP, PBAPC_NO_PAYLOAD );
}

/***************************************************************************
 * NAME
 *  PbapcConnectRequest
 *
 * DESCRIPTION
 *  Open a PBAP Connection with a PBAP Server 
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcConnectRequest( Task theAppTask, const bdaddr *bdAddr ) 
{
    Task state = pbapcCreateTask( theAppTask );
    
    PBAPC_ASSERT(bdAddr, ("PBAPC - Null Bluetooth address pointer\n")); 
    
    /* Send an internal message */
    if(state)
    {
        pbapc_link_priority priority;
        
		/* Add PBAP device */
        if( !pbapcAddPbapDevice( bdAddr, &priority) )
        {
            if(priority != pbapc_invalid_link)
            {
                /* Connected or connection is going on */
                pbapcMsgSendConnectCfm( theAppTask, Pbapc[priority], bdAddr,
                                        (Pbapc[priority]->connect_state == pbapc_connected) ? pbapc_success : pbapc_pending, 
                                        (Pbapc[priority]->srvRepos), 0 );
            }

            /* Free the temporal task */
            MessageSend(state, PBAPC_INT_TASK_DELETE, 0);
        }
        else
	    {
            MAKE_PBAPC_MESSAGE(PBAPC_INT_CONNECT);        
            message->bdAddr = *bdAddr;
            MessageSend(state, PBAPC_INT_CONNECT, message);
            
            Pbapc[priority]  = (PBAPC *)state;
            ((PBAPC *)state)->device_id = priority;
            ((PBAPC *)state)->connect_state = pbapc_connecting;            
		}
    }
    else
    {
        pbapcMsgSendConnectCfm( theAppTask, NULL, bdAddr,
                                pbapc_failure, 0, 0 );
    }
}

/***************************************************************************
 * NAME
 * PbapcConnectAuthResponse
 *
 * DESCRIPTION
 *  Authenticate the OBEX Connection
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcConnectAuthResponse( uint16 device_id, 
                               const uint8 *digest, 
                               uint16 sizeUserId, 
                               const uint8 *userId, 
                               const uint8 *nonce)
{
    PBAPC *pbapc = Pbapc[device_id];
    PBAPC_ASSERT( (pbapc && digest && nonce), ("Invalid parameters"));

    /* Send the Local Authentication challenge */
    pbapcSendAuthChallenge( pbapc, nonce );

    /* Send the Authentication response for remote challenge */
    pbapcSendAuthResponse( pbapc, digest, sizeUserId, userId );
}

/***************************************************************************
 * NAME
 *  PbapcPullPhoneBookRequest
 *
 * DESCRIPTION
 *  Pull Phone Book function request
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcPullPhonebookRequest( uint16 device_id, 
                                PbapcPhoneRepository repository, 
                                PbapcPhoneBook phonebook,
                                const PbapcPullPhonebookParams *params)
{
    uint8 reps;
	PBAPC *pbapc = Pbapc[device_id];
 
    PBAPC_ASSERT( ( pbapc ), ("Invalid Params"));

    reps =  pbapcGetSupportedRepositories(pbapc);
    
    /* local repositry is default */
    if((repository &= reps) == pbap_any ) repository = pbap_local;

    if( repository )
    {
        MAKE_PBAPC_MESSAGE(PBAPC_INT_GET_PHONEBOOK);
        memset( message, 0,sizeof(PBAPC_INT_GET_PHONEBOOK_T) );
        message->repository = repository;
        message->phonebook = (phonebook <=  pbap_telecom)? 
                              pbap_pb: phonebook;
        if( params )
        {
            message->filterLo = params->filter.filterLow;
            message->filterHi = params->filter.filterHigh;
            message->format = (params->format == pbap_format_def)?
                               pbap_format_21: params->format;
            message->maxList = params->maxList;
            message->listStart = params->listStart;
        }
        MessageSend( (Task)pbapc, PBAPC_INT_GET_PHONEBOOK, message);
    }
    else
    {
        pbapcMsgSendPullPhonebookErrCfm( pbapc, pbapc_ppb_no_repository );
    }
}

/***************************************************************************
 * NAME
 *  PbapcPullContinue
 *
 * DESCRIPTION
 *  Continue the ongoing pull operation.
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcPullContinue( uint16 device_id )
{
    PBAPC *pbapc = Pbapc[device_id];

    if( pbapc )
    {
        MAKE_PBAPC_MESSAGE( PBAPC_INT_GET_CONTINUE );
        message->proceed = TRUE;
        MessageSend( (Task)pbapc, PBAPC_INT_GET_CONTINUE, message );
    }
    else
    {
        PBAPC_DEBUG(("Invalid Handle\n"));
    }
}

/***************************************************************************
 * NAME
 *  PbapcPullComplete
 *
 * DESCRIPTION
 *  Abort an ongoing pull operation or mark the pull operation as complete.
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcPullComplete( uint16 device_id )
{
    PBAPC *pbapc = Pbapc[device_id];

    if( pbapc )
    {
        MAKE_PBAPC_MESSAGE( PBAPC_INT_GET_CONTINUE );
        message->proceed = FALSE;
        MessageSend( (Task)pbapc, PBAPC_INT_GET_CONTINUE, message );
    }
    else
    {
        PBAPC_DEBUG(("Invalid Handle\n"));
    }
}

/***************************************************************************
 * NAME
 *  PbapcSetPhonebookRequest
 *
 * DESCRIPTION
 *  Change the phone book 
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcSetPhonebookRequest( uint16 device_id, 
                               PbapcPhoneRepository repository, 
                               PbapcPhoneBook phonebook )
{
    uint8 reps;
    PBAPC *pbapc = Pbapc[device_id];

    PBAPC_ASSERT((phonebook !=  pbap_root ), ("Invalid phonebook"));  
    PBAPC_ASSERT(( pbapc), (" Invalid handle "));

    reps =  pbapcGetSupportedRepositories(pbapc);

    /* local repositry is default */
    if((repository &= reps) == pbap_any ) repository = pbap_local;
       
    if( repository )
    {
        MAKE_PBAPC_MESSAGE(PBAPC_INT_SET_PHONEBOOK);        
        message->repository = repository;
        message->phonebook = phonebook;
        MessageSend( (Task)pbapc, PBAPC_INT_SET_PHONEBOOK, message);
    }
    else
    {
        pbapcMsgSendSetPhonebookCfm( pbapc, pbapc_spb_no_repository, 0, 0 );
    }
}

/***************************************************************************
 * NAME
 *  PbapcPullVcardListingRequest
 *
 * DESCRIPTION
 *  Pull a vCard Listing
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/

void PbapcPullVcardListingRequest( uint16 device_id, 
                                   PbapcPhoneBook phonebook, 
                                   const PbapcPullvCardListParams *params )
{
    uint16 len = 0;
    PBAPC *pbapc = Pbapc[device_id];

    PBAPC_ASSERT( pbapc, ("Invalid Params"));
    if( params && params->srchVal ) len = params->srchValLen;

    if( len <= PBAPC_MAX_SRCH_STR_LEN )
    {
        MAKE_PBAPC_MESSAGE_WITH_LEN( PBAPC_INT_GET_VCARD_LIST, len+1); 
        memset( message, 0, sizeof(PBAPC_INT_GET_VCARD_LIST_T) );

        message->phonebook = phonebook;

        if( params )
        {
            message->order = params->order;  
            message->srchAttr = params->srchAttr;
            if( len )
            {
                memmove( message->srchVal, params->srchVal, len );
                if(message->srchVal[len-1] != '\0' )
                {
                    message->srchVal[len] = '\0';
                    len++;
                }
            }
            message->srchValLen = len;
            message->maxList = params->maxList;
            message->listStart = params->listStart;
        }

        MessageSend( (Task)pbapc, PBAPC_INT_GET_VCARD_LIST, message);
    }
    else
    {
        pbapcMsgSendPullvCardListErrCfm( pbapc,pbapc_vcl_no_param_resources);
    }
}


/***************************************************************************
 * NAME
 *  PbapcPullVcardEntryRequest
 *
 * DESCRIPTION
 *  Pull a specific vCard
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcPullVcardEntryRequest( uint16 device_id,
                                 uint32 entry, 
                                 const PbapcPullvCardEntryParams* params )
{
    PBAPC *pbapc = Pbapc[device_id];

    if( pbapc )
    {
        MAKE_PBAPC_MESSAGE(PBAPC_INT_GET_VCARD);
        memset(message, 0, sizeof(PBAPC_INT_GET_VCARD_T));
        message->entry = entry;
        if( params )
        {
            message->filterLo = params->filter.filterLow;
            message->filterHi = params->filter.filterHigh;
            message->format = (params->format == pbap_format_def)?
                           pbap_format_21: params->format;
        }
        MessageSend((Task)pbapc, PBAPC_INT_GET_VCARD, message);
    }
    else
    {
        PBAPC_DEBUG(("Invalid Handle\n"));
    }   
}

/***************************************************************************
 * NAME
 *  PbapcDisconnectRequest
 *
 * DESCRIPTION
 *  Disconnect the PBAP Connection
 *
 * PARAMETERS
 *  Refer pbapc.h 
 **************************************************************************/
void PbapcDisconnectRequest(uint16 device_id)
{
    PBAPC *pbapc = Pbapc[device_id];

    if( pbapc )
    {
        /* Send an internal message */
        MessageSend( (Task)pbapc, PBAPC_INT_DISCONNECT, PBAPC_NO_PAYLOAD);
    }
    else
    {
        PBAPC_DEBUG(("Invalid Handle\n"));
    } 
}

/***************************************************************************
 * NAME
 *  PbapcGetNoOfConnection
 *
 * DESCRIPTION
 *  Check how many PBAPC links have been connected
 **************************************************************************/
uint8 PbapcGetNoOfConnection(void)
{
    uint8 NoOfDevices = 0;

    if((Pbapc[pbapc_primary_link]) && (Pbapc[pbapc_primary_link])->connect_state == pbapc_connected)
        NoOfDevices++;
 
    if((Pbapc[pbapc_secondary_link]) && (Pbapc[pbapc_secondary_link])->connect_state == pbapc_connected)
        NoOfDevices++;
   
    return NoOfDevices;
}

/***************************************************************************
 * NAME
 *  PbapcGetServerProperties
 *
 * DESCRIPTION
 *  Check server the supported repositories mask
 **************************************************************************/
uint8 PbapcGetServerProperties(uint16 device_id)
{
	PBAPC *pbapc = Pbapc[device_id];
	
    if(pbapc)
	{
        return(pbapc->srvRepos);
    }
    else
    {
        PBAPC_DEBUG(("Invalid Handle\n"));
        return pbap_current;
    }
}


/*!
	@brief Obtain the sink of the corresponding pbap link.
	@param device_id PBAPC Session Handle.
	
	@return the server to obtain the sink of obex connection.
*/
Sink PbapcGetSink(uint16 device_id)
{
    PBAPC *pbapc = Pbapc[device_id];
	
    if(pbapc)
	{
        return(ObexGetSink( pbapc->handle ));
    }
    else
    {
        PBAPC_DEBUG(("Invalid Handle\n"));
        return NULL;
    }
    
}
