/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_messages.c
    
DESCRIPTION
    PhoneBook Access Profile Client Library - message send functions.

*/
#include <string.h>
#include "pbapc_extern.h"
#include "pbapc_private.h"

/****************************************************************************
 *NAME    
 *  pbapcMsgInitCfm
 *
 *DESCRIPTION
 *  Send Init Confirmation message to the application. 
 ***************************************************************************/
void pbapcMsgInitCfm( Task              theAppTask, 
                      uint32            sdpHandle, 
                      PbapcLibStatus    status )
{
    MAKE_PBAPC_MESSAGE( PBAPC_INIT_CFM );
    message->sdpHandle = (status == pbapc_success)? sdpHandle: 0;
    message->status = status;
    MessageSend( theAppTask, PBAPC_INIT_CFM, message );
}

/***************************************************************************
 *NAME    
 *  pbapcMsgSendConnectCfm 
 *
 *DESCRIPTION
 *  Send Connect Confirmation message to the application. 
 **************************************************************************/
void pbapcMsgSendConnectCfm( Task theAppTask,
                             PBAPC *state, 
                             const bdaddr* addr,
                             PbapcLibStatus status, 
                             uint8  repository,
                             uint16 pktSize )
{
    if(state)
    {
        state->connect_state = (status == pbapc_success) ? pbapc_connected  : (state->connect_state);
    }
    
    {        
        MAKE_PBAPC_MESSAGE(PBAPC_CONNECT_CFM);
        message->device_id = (state) ? (state->device_id) : (pbapc_invalid_link);
        message->bdAddr = *addr;
        message->status = status;
        message->packetSize = pktSize;
        message->repositories = repository & 0x03;

        MessageSend(theAppTask, PBAPC_CONNECT_CFM, message);
	}
}

/***************************************************************************
 *NAME    
 *  pbapcMsgSendAuthRspCfm
 *
 *DESCRIPTION
 *  Handle the authentication response 
 **************************************************************************/
void pbapcMsgSendAuthRspCfm( PBAPC *state,
                             const uint8* digest )
{
    MAKE_PBAPC_MESSAGE(PBAPC_AUTH_RESPONSE_CFM);
    message->device_id = state->device_id;
    memmove( message->digest, digest, PBAPC_OBEX_SIZE_DIGEST );
    MessageSend( pbapcGetAppTask(state), PBAPC_AUTH_RESPONSE_CFM, message );
}


/***************************************************************************
 *NAME    
 *  pbapcMsgSendAuthReqInd 
 *
 *DESCRIPTION
 *  Send Request to start local authenticaton
 **************************************************************************/
void pbapcMsgSendAuthReqInd( PBAPC *state,
                             const uint8 *nonce,
                             PbapcObexAuthOptions options,
                             uint16 sizeRealm, 
                             const uint8* realm )
{
    MAKE_PBAPC_MESSAGE(PBAPC_AUTH_REQUEST_IND);

    message->device_id = state->device_id;
    memmove( message->nonce, nonce, PBAPC_OBEX_SIZE_DIGEST );
    message->options = options;
    message->sizeRealm = sizeRealm;
    message->realm = realm;
    MessageSend( pbapcGetAppTask(state), PBAPC_AUTH_REQUEST_IND, message );
}

/***************************************************************************
 *NAME    
 * pbapcMsgSendPullPhoneBookCfm
 *
 *DESCRIPTION
 * Send a confirmation message for Pull phonebook or Pull vCard Listing 
 * Confirmation  
 **************************************************************************/
void pbapcMsgSendPullPhoneBookCfm ( PBAPC* state,
                                    MessageId id,
                                    PbapcLibStatus status,
                                    uint16 pbSize,
                                    uint8 newMisscall,
                                    uint16 len,
                                    Source src )
{
    MAKE_PBAPC_MESSAGE( PBAPC_PULL_PHONEBOOK_CFM );

    message->device_id = state->device_id;
    message->status = status;
    message->pbookSize = pbSize;
    message->newMissCalls = newMisscall;
    message->src = src;
    message->dataLen = len;

    MessageSend( pbapcGetAppTask(state), id, message ); 
}       

/****************************************************************************
 *NAME    
 *  pbapcMsgSendSetPhonebookCfm 
 *
 *DESCRIPTION
 *  Send set Phone book Confirmation message to the application. 
 ***************************************************************************/
void pbapcMsgSendSetPhonebookCfm( PBAPC *state,
                                  PbapcLibStatus status,
                                  PbapcPhoneRepository repos,
                                  PbapcPhoneBook pb )       
{
    MAKE_PBAPC_MESSAGE(PBAPC_SET_PHONEBOOK_CFM);
    message->device_id = state->device_id;
    message->status = status;
    message->repository = repos;
    message->phonebook = pb;
            
    MessageSend( pbapcGetAppTask(state), PBAPC_SET_PHONEBOOK_CFM, message);
}

 
/***************************************************************************
 *NAME    
 * pbapcMsgSendPullVcardCfm
 *
 *DESCRIPTION
 * Send a confirmation message for Pull vCard
 * Confirmation  
 **************************************************************************/
void pbapcMsgSendPullVcardCfm ( PBAPC* state,
                                PbapcLibStatus status,
                                uint16 len,
                                Source src )
{
    MAKE_PBAPC_MESSAGE( PBAPC_PULL_VCARD_ENTRY_CFM );

    message->device_id = state->device_id;
    message->status = status;
    message->src = src;
    message->dataLen = len;

    MessageSend( pbapcGetAppTask(state),PBAPC_PULL_VCARD_ENTRY_CFM,message );
}       
 
/***************************************************************************
 *NAME    
 * pbapcMsgSendDisconnectCfm
 *
 *DESCRIPTION
 * Send Disconnect Confirmation message to the application.
 **************************************************************************/
void pbapcMsgSendDisconnectCfm( PBAPC* state, PbapcLibStatus status)
{
    MAKE_PBAPC_MESSAGE(PBAPC_DISCONNECT_IND);
    message->device_id = state->device_id;
    message->status    = status;
    MessageSend( pbapcGetAppTask(state),PBAPC_DISCONNECT_IND, message);
    
    /* Set the connect state of current PBAPC link.*/
    state->connect_state    = pbapc_disconnected;
    Pbapc[state->device_id] = NULL;
}

