/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_goep_handler.h
    
DESCRIPTION
    Handler functions for GOEP or CL messages.

*/

#include <string.h>
#include <sink.h>

#include "pbapc_extern.h"
#include "pbapc_private.h"
#include "pbapc_util.h"

static const uint8 pbapTarget[] = 
{
    0x79, 0x61, 0x35, 0xf0,   
    0xf0, 0xc5,   
    0x11, 0xd8,
    0x09, 0x66,   
    0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

/*************************************************************************
 * NAME
 *  pbapcGoepConnect
 *
 * DESCRIPTION 
 *  Send a GOEP connect request 
 *************************************************************************/
void  pbapcGoepConnect( pbapcState *state,
                        const bdaddr *addr,
                        uint16       channel )
{
    Task appTask;
	uint16 device_id = state->device_id;
    uint8 repos = state->srvRepos;
    const ObexConnParams params = { { pbapcGoepHandler }, sizeof(pbapcState),
                                    pbapTarget, sizeof(pbapTarget), FALSE };

    /* store the application task and free the temporary task */
    appTask = state->theAppTask;
    MessageFlushTask((Task)state);
    state->currCom = pbapc_com_none;

    /* Delete the task after PBAPC_TASK_DELETE_DELAY, This is to handle messages directed
       to the task with in that time. the only possible message is 
       any continuation responses for this SDP attribute search. */
    MessageSendLater(&state->task,PBAPC_INT_TASK_DELETE, 0, PBAPC_TASK_DELETE_DELAY);

    /* initiate the connection attempt and start using the new state */
    state = (pbapcState*)ObexConnectRequest(addr, channel, &params);
    
    Pbapc[device_id]     = state;
	state->bdAddr        = *addr;
	state->device_id     = device_id;
    state->connect_state = pbapc_connecting;

    state->theAppTask = appTask;
    state->currCom = pbapc_com_connect;
    state->srvRepos = repos;
    state->pb.currRepos = pbap_any;
    state->pb.currPb = pbap_root;
}

/*************************************************************************
 * NAME
 * convertObexError
 *
 * DESCRIPTION 
 *   Error status conversion 
 *************************************************************************/
static PbapcLibStatus convertObexError( ObexStatus status)
{
     PbapcLibStatus ret = pbapc_success;
    
    switch (status)
    {
    case obex_success:
    case obex_remote_success: /*Fallthrough */
        break;

    case obex_remote_disconnect:
    case obex_transport_failure: /*Fallthrough */
        ret = pbapc_remote_disconnect;
        break;

    case obex_local_unauth:
    case obex_unauthorized: /*Fallthrough */
        ret = pbapc_spb_unauthorised;
        break;

    case obex_invalid_parameters:   
    case obex_invalid_command: /*Fallthrough */      
        ret = pbapc_not_idle; 
        break;

    case obex_host_abort:
    case obex_local_abort: /*Fallthrough */
        ret = pbapc_aborted;
        break;
        
    case obex_continue:
        ret = pbapc_pending;
        break;

    case obex_not_found:
        ret = pbapc_spb_not_found;
        break;

    default:
        ret = pbapc_failure;
        break;
    }
    
    return ret;
}

/*************************************************************************
 * NAME
 *  handleGoepSetPathCfm
 *
 * DESCRIPTION 
 *   Handle the Pull Phone book Response
 *************************************************************************/
static void handleGoepSetPathCfm(pbapcState *state, OBEX_SET_PATH_CFM_T *msg)
{
    PbapcLibStatus status;
    bool complete = TRUE;

    PBAPC_ASSERT(state->handle, ("Not Connected"));

    status = convertObexError( msg->status );
    if( status ==  pbapc_success ) complete = pbapcPbCompleteState( state ); 

    if( complete ) 
    {
        pbapcMsgSendSetPhonebookCfm( state, status, 
                                     state->pb.currRepos,
                                     state->pb.currPb );
        state->currCom = pbapc_com_none;
   }
   else
   {
       state->setPbState = pbapcPbNextState ( state->pb ); 
       pbapcSetPb( state );
   }            
}

/*************************************************************************
 * NAME
 * handlevCardResp
 *
 * DESCRIPTION 
 *   Handle the vCard Entry response
 *************************************************************************/
static void handlevCardResp( pbapcState *state, 
                             PbapcLibStatus status ) 
{
    Source src=0;
    uint16 objLen = 0;

    if( status == pbapc_pending || status == pbapc_success )
    {
        ObexSeqHeaderId hdr = ( status == pbapc_pending )? obex_body_hdr: 
                                                          obex_ebody_hdr;
        /* Get the body */
        src = ObexObjFilterHdrValue( state->handle, hdr, &objLen );
    }

    pbapcMsgSendPullVcardCfm( state, status, objLen, src);  

}

/*************************************************************************
 * NAME
 * handlePullListResp
 *
 * DESCRIPTION 
 *   Handle the Pull Phone book Response
 *************************************************************************/
static void handlePullListResp( pbapcState *state, 
                                MessageId  id, 
                                PbapcLibStatus status, 
                                uint16 len, 
                                Source src )
{
    uint16 pbSize = 0;
    uint8  newMissCall = 0;
    uint16 objLen = 0;

    if( status == pbapc_pending || status == pbapc_success )
    {
        const uint8* appParam = NULL;
        ObexSeqHeaderId hdr = ( status == pbapc_pending )? obex_body_hdr: 
                                                          obex_ebody_hdr;

        appParam = ObexObjMapHdrValue( obex_app_params, len, src, &objLen);

        if( appParam )
        {
            uint8 offset = 0;

            while( offset < objLen )
            {
                /* Total pkt size of a 2 byte app param is 4 and 
                    1 byte app param is 3*/
                if( ( appParam[ offset ] ==  pbapc_app_phonebook_size )  &&
                    ( objLen >= (offset + WORD_APP_PARAM_LEN ) ) )
                {
                    /* Byte 2 and 3 contains the value */
                    pbSize = ( appParam[ offset + 2 ] << 8 ) | 
                             ( appParam[ offset + 3 ] );
                    offset += WORD_APP_PARAM_LEN;
                }
                else if( ( appParam[ offset ] == pbapc_app_new_missed_calls) 
                        && (objLen >= (offset + BYTE_APP_PARAM_LEN ) ) )
                {
                    /* Byte 2 contains the value */
                    newMissCall = appParam[ offset + 2 ];
                    offset += BYTE_APP_PARAM_LEN;
                }
                else
                {
                    /* Error - Unknown header - ignore */
                    break;
                }
            }    
        }
        
        /* Get the body */
        src = ObexObjFilterHdrValue( state->handle, hdr, &objLen );
    }

    pbapcMsgSendPullListcfm( state, id, status, pbSize,
                             newMissCall, objLen, src );
                             
}

/*************************************************************************
 * NAME
 * handleGoepGetCfm 
 *
 * DESCRIPTION 
 *   Handle the Get Cfm message
 *************************************************************************/
static void handleGoepGetCfm( pbapcState *state, OBEX_GET_CFM_T* msg )
{
    PbapcLibStatus status;
    MessageId  id;

    PBAPC_ASSERT(state->handle, ("Not Connected"));

    status = convertObexError( msg->status );

    switch( state->currCom )
    {
    case  pbapc_com_pull_phonebook:
        id = PBAPC_PULL_PHONEBOOK_CFM;
        break;
    case  pbapc_com_pull_vcard_list:
        id = PBAPC_PULL_VCARD_LISTING_CFM;
        break;
    case  pbapc_com_pull_vcard:
        id = PBAPC_PULL_VCARD_ENTRY_CFM;
        break;
    default:
        PBAPC_DEBUG(("Unknown State"));
        return;
    }

    if( id == PBAPC_PULL_VCARD_ENTRY_CFM )
    {
        handlevCardResp(  state, status );
    }
    else
    {
        handlePullListResp( state, id, status, 
                            msg->sourceLen, 
                            msg->source );
    }
    
    if( status != pbapc_pending ) state->currCom = pbapc_com_none;
}



/*************************************************************************
 * NAME
 * handleGoepConnectCfm
 *
 * DESCRIPTION 
 *   Handle the Connect Cfm message
 *************************************************************************/
static void handleGoepConnectCfm( pbapcState *state,
                                  OBEX_CONNECT_CFM_T *msg )
{
    PbapcLibStatus status = pbapc_success;
    uint16 packetSize = 0;
    tp_bdaddr tpaddr;
    
    status = convertObexError(msg->status);

    if (status == pbapc_success)
    {
        packetSize = msg->maxPacketLen;
        state->handle = msg->session;    
        SinkGetBdAddr( ObexGetSink( state->handle ), &tpaddr );
    }
    else
    { 
        /* Couldn't connect */
        state->handle = NULL;
        memset(&tpaddr, 0, sizeof(tp_bdaddr));
    }
        
	Pbapc[state->device_id] = (status == pbapc_success) ? state : (NULL);
    
    state->currCom= pbapc_com_none;
    pbapcMsgSendConnectCfm( state->theAppTask, state, &tpaddr.taddr.addr,
                            status, state->srvRepos, packetSize );
}


/*************************************************************************
 * NAME
 * handleGoepAuthClgInd
 *
 * DESCRIPTION 
 *   Handle the Authentication Challenge indication message
 *************************************************************************/
static void handleGoepAuthClgInd( pbapcState *state, 
                                  OBEX_AUTH_CLG_IND_T *msg)
{
    if( state->currCom != pbapc_com_connect ) return;

    pbapcMsgSendAuthReqInd( state, 
                            msg->nonce,
                            msg->options,
                            msg->sizeRealm,
                            msg->realm );       
}


/*************************************************************************
 * NAME
 * handleGoepSessionCfm
 *
 * DESCRIPTION 
 *   Handle the Session Confirmation message
 *************************************************************************/
static void handleGoepSessionCfm( pbapcState* state, 
                                  OBEX_CREATE_SESSION_CFM_T* msg )
{
    PbapcLibStatus status;
    Task theAppTask = state->theAppTask;
    uint8 repos = state->srvRepos;
    uint16 device_id = state->device_id;
            
    if( msg->status ==  obex_success )
    {
        status = pbapc_pending;
        state->handle = msg->session;    
    }
    else
    {
        state->currCom = pbapc_com_none;
        status = pbapc_failure;
        state = NULL;
    }

    /* Reset the Pbapc[] if the obex session failed */
    Pbapc[device_id] = state;
    
    pbapcMsgSendConnectCfm( theAppTask, state, 
                            &msg->remoteAddr,
                            status, repos , 0); 
}

/****************************************************************************
 *NAME    
 *  handleObexDeleteSessionInd
 *
 *DESCRIPTION
 *  handle the Delete Session
 ***************************************************************************/
static void handleObexDeleteSessionInd( pbapcState *state, 
                                        OBEX_DELETE_SESSION_IND_T *msg)
{
    /* clear the current session info */
    if (msg->session == state->handle)
    {
        state->handle = NULL;
    
        state->currCom = pbapc_com_none;
        
        pbapcMsgSendDisconnectCfm(state, pbapc_success);
    }

    /* let the Obex library free the state */
    ObexDeleteSessionResponse( msg->session );
    Pbapc[state->device_id] = NULL;
}


/****************************************************************************
 *NAME    
 *  pbabcGoepHandler
 *
 *DESCRIPTION
 *  Handler for messages received by the PBABC Task from OBEX/CL.
 ***************************************************************************/
void pbapcGoepHandler(Task task, MessageId id, Message message)
{
       /* Get task control block */
    pbapcState *state = (pbapcState*)task;

    if( id <  PBAPC_INT_ENDOFLIST )
    {
        pbapcIntHandler( task, id, message );
        return;
    }

    switch (id)
    {
        case OBEX_CREATE_SESSION_CFM:
            handleGoepSessionCfm( state,
                                  (OBEX_CREATE_SESSION_CFM_T *)message );
            break;

        case OBEX_AUTH_REQ_IND:
            /* Ignore. PBAPC expectes OBEX_AUTH_CLG_IND followed by this */
            break;

        case OBEX_AUTH_CLG_IND:
            handleGoepAuthClgInd(state, 
                                  (OBEX_AUTH_CLG_IND_T *)message);
            break;        

        case OBEX_AUTH_RSP_CFM:
            handleGoepAuthRspCfm( state,
                                  ( OBEX_AUTH_RSP_CFM_T*)message);
            break;

        case OBEX_CONNECT_CFM:
            handleGoepConnectCfm(state, (OBEX_CONNECT_CFM_T *)message);        
            break;

        case OBEX_DELETE_SESSION_IND:
            handleObexDeleteSessionInd(state, 
                                       (OBEX_DELETE_SESSION_IND_T*)message);
            break;

        case OBEX_GET_CFM:
            handleGoepGetCfm( state, (OBEX_GET_CFM_T*) message );
            break;
        case OBEX_SET_PATH_CFM:
            handleGoepSetPathCfm(state, (OBEX_SET_PATH_CFM_T *)message);
            break;   

        default:
            break;
    }
}


