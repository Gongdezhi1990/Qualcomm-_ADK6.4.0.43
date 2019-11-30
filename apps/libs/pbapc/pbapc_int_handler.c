/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_int_handler.c
    
DESCRIPTION
    PhoneBook Access Profile Client Library - handler functions for 
    internal messages.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stream.h>
#include <source.h>

#include "pbapc_extern.h"
#include "pbapc_private.h"
#include "pbapc_util.h"


#define PBAPC_OBJ_LEN  21


/***************************************************************************
 * NAME    
 *  handleIntPullvCard
 *
 * DESCRIPTION
 *  internal handler to pull vcard
 ***************************************************************************/
static void handleIntPullvCard( pbapcState *state, 
                                PBAPC_INT_GET_VCARD_T *msg )
{
    uint8 *obj = NULL;
    Source src = 0;
    uint16 len = 0;
    bool ok;

    if (state->currCom != pbapc_com_none)
    { 
        /* Not idle, Send Error Message */
        pbapcMsgSendPullvCardErrCfm( state, pbapc_not_idle );
        return;
    }

       if ((msg->entry == 0) && (state->pb.currPb != pbap_pb ))
    {
        pbapcMsgSendPullvCardErrCfm(state, pbapc_vce_invalid_entry);
        return;
    }

    obj = (uint8*)malloc(PBAPC_OBJ_LEN);
    if( !obj )
    {
        pbapcMsgSendPullvCardErrCfm( state, pbapc_vce_no_param_resources );
        return;
    }

    /* The minimum OBEX packet length is 255. As per the specification
    in case of a multi packet request, all obex request messages shall be
    sent as one OBEX packet containing all of the headers */
    /* to make the code simpler we check the ok flag only at the end */
    /* Create Obex object */
    ok = !!ObexObjNew(state->handle); /* 1 if !0 */

    /* Add Name object */
    if( ok )
    {
        sprintf((char*)obj,"%lx.vcf",  msg->entry );
        len = strlen( (const char*) obj);
        src = StreamRegionSource( obj, len );
        ok &= (src &&
          ObexObjAddVariableHdr( state->handle, obex_name_hdr, len, src));
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }                        
    } 

    /* Add Type Object */
     if( ok ) 
    {
        const uint8* type = pbapcGetvCardMimeType( &len );
        src = StreamRegionSource( type, len );
        ok &= src &&
          ObexObjAddVariableHdr( state->handle, obex_type_hdr, len, src);
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    }

    /* Add Application parameters */ 
    if( ok )
    {
       len = pbapcFrameFilterFormat( msg->filterHi, msg->filterLo, 
                                     msg->format,  obj );
        if( len )
        {
            src = StreamRegionSource( obj, len );
            ok &= src &&
                  ObexObjAddVariableHdr( state->handle,
                                              obex_app_params, 
                                              len, src);
            
            if (src)
            {
                SourceEmpty(src);
                src = 0;
            }
        }
    }
 
    if ( ok )
    {
        ObexGetRequest( state->handle, TRUE );
        state->currCom =  pbapc_com_pull_vcard;
    }
    else
    {
        ObexObjDelete( state->handle );
        pbapcMsgSendPullvCardErrCfm( state,pbapc_vce_no_param_resources );
    }
    
    /* Free the object */
    if(obj) free( obj );

}
   

/***************************************************************************
 * NAME    
 *  handleIntPullvCardList
 *
 * DESCRIPTION
 *  internal handler to pull vcard List
 ***************************************************************************/
static void handleIntPullvCardList( pbapcState *state, 
                                    PBAPC_INT_GET_VCARD_LIST_T *msg )
{
    uint16 len = WORD_APP_PARAM_LEN; /* for Max List Count size */
    uint8 *obj = NULL;
    bool ok;
    Source src = 0;

    if (state->currCom != pbapc_com_none)
    { 
        /* Not idle, Send Error Message */
        pbapcMsgSendPullvCardListErrCfm( state, pbapc_not_idle );
        return;
    }

    if(msg->phonebook == pbap_root ) msg->phonebook = state->pb.currPb;


    if( ( msg->phonebook != state->pb.currPb ) &&
        ( state->pb.currPb != pbap_telecom ) )
    {
        pbapcMsgSendPullvCardListErrCfm( state, pbapc_vcl_no_pbook_folder );
        return;
    } 


    if( msg->order ) len += BYTE_APP_PARAM_LEN;
    if( msg->srchAttr) len += BYTE_APP_PARAM_LEN;
    if( msg->srchValLen ) len += ( msg->srchValLen + APP_PARAM_HDR_LEN );
    if( msg->listStart ) len += WORD_APP_PARAM_LEN; 
 
    if( len )
    {
        obj =  malloc( len );
        if( !obj )
    {
        pbapcMsgSendPullvCardListErrCfm( state,pbapc_vcl_no_param_resources);
        return;
        }
    }

    /* The minimum OBEX packet length is 255. As per the specification
    in case of a multi packet request, all obex request messages shall be
    sent as one OBEX packet containing all of the headers */
    /* to make the code simpler we check the ok flag only at the end */
    /* Create Obex object */
    ok = !!ObexObjNew(state->handle); /* 1 if !0 */ 
 
    /* Add Name */
    if( ok )
    {
        len = 0;
        if( msg->phonebook != state->pb.currPb ) 
        {
            const uint8* name = pbapcGetPbNameFromID( msg->phonebook, &len );
            src = StreamRegionSource( name, len );
        }

        ok &= ObexObjAddVariableHdr( state->handle, obex_name_hdr, 
                                        len, src );
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    }

    /* Add Type object */
    if( ok ) 
    {   
        const uint8* type =  pbapcGetvCardListingMimeType( &len );
        src = StreamRegionSource( type, len );
        ok &= src &&
          ObexObjAddVariableHdr( state->handle, obex_type_hdr, len, src);
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    }
    /* Add application parameters */
    if( ok )
    {
        len = pbapcFramevCardListAppParams( msg->order,
                                            msg->srchVal,
                                            msg->srchValLen,
                                            msg->srchAttr,
                                            obj );
        len +=  pbapcFrameListAppParams( msg->maxList, 
                                         msg->listStart, &obj[len] );

        src = StreamRegionSource( obj, len );
        ok &= src &&
        ObexObjAddVariableHdr( state->handle, 
                               obex_app_params, len, src);
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    }

    if ( ok )
    {
        ObexGetRequest( state->handle, TRUE );
        state->currCom =  pbapc_com_pull_vcard_list;
    }
    else
    {
        ObexObjDelete( state->handle );
        pbapcMsgSendPullvCardListErrCfm( state,pbapc_vcl_no_param_resources);
    }

    /* Free the object */
    if(obj) free( obj );
}


/***************************************************************************
 * NAME    
 *  handleIntSetPb
 *
 * DESCRIPTION
 *  internal handler to set the phone book
 ***************************************************************************/
static void handleIntSetPb( pbapcState *state, 
                            PBAPC_INT_SET_PHONEBOOK_T *msg)
{
    pbapcPb pb = state->pb;

    if(msg->repository == pbap_current) msg->repository = pb.currRepos;
    if(msg->phonebook == pbap_current) msg->phonebook = pb.currPb;
    
     /* If already in the target folder, just send the confirm */
    if ( (pb.currRepos == msg->repository) && 
         (pb.currPb == msg->phonebook) )
    {
        pbapcMsgSendSetPbCfm( state, pbapc_success );
        return;
    }

    /* If not idle, Send Error Message */
    if( state->currCom != pbapc_com_none )
    { 
        pbapcMsgSendSetPbCfm( state, pbapc_not_idle );
        return;
    }

    /* If it does not support current repository, throw error */
    if( !( state->srvRepos & msg->repository ) )
    {
        pbapcMsgSendSetPbCfm(state, pbapc_spb_no_repository);
        return;
    }

    state->currCom = pbapc_com_set_phonebook;

    pb.targetRepos = msg->repository;
    pb.targetPb = msg->phonebook;
    state->pb = pb;
    state->setPbState  = pbapcPbNextState( pb );
    pbapcSetPb(state);
}

/***************************************************************************
 * NAME    
 *  handleIntContinue
 *
 * DESCRIPTION
 *  Continue the current PULL opertaion 
 ***************************************************************************/
static void handleIntPullContinue( pbapcState *state, 
                                   PBAPC_INT_GET_CONTINUE_T* msg )
{
    bool isPull;    
    /* Not connected */
    if( !state->handle )
    {
        PBAPC_DEBUG(("Not Connected\n"));
        return;
    }

    isPull = ( state->currCom == pbapc_com_pull_phonebook ) ||
             ( state->currCom ==  pbapc_com_pull_vcard_list) ||
             ( state->currCom == pbapc_com_pull_vcard );
    
    if( msg->proceed && isPull )
    {
       ObexGetRequest( state->handle, TRUE ); 
    }
    else
    {
        ObexSourceDrop( state->handle );
        if( isPull ) ObexAbort( state->handle );  
    }

}
    
/***************************************************************************
 * NAME    
 *  handleIntPullPb
 *
 * DESCRIPTION
 *  internal handler to initiate Pull phone book function
 ***************************************************************************/
static void handleIntPullPb( pbapcState *state, 
                             PBAPC_INT_GET_PHONEBOOK_T *msg)
{
    uint16 len = 0;
    uint8 *obj = NULL;
    const uint8* type = NULL;
    Source src = 0;
    bool ok;

    if (state->currCom != pbapc_com_none)
    { 
        /* Not idle, Send Error Message */
        pbapcMsgSendPullPhonebookErrCfm( state, pbapc_not_idle );
        return;
    }

    if( msg->repository == pbap_current )
    {
        if( !(state->pb.currRepos & state->srvRepos) )  
        {
            pbapcMsgSendPullPhonebookErrCfm( state, pbapc_spb_no_repository);
            return;
        }

        msg->repository = state->pb.currRepos;
    } 


    obj =  malloc(PBAPC_OBJ_LEN);
    if(!obj)
    {
        pbapcMsgSendPullPhonebookErrCfm( state, pbapc_ppb_no_name_resources);
        return;
    }

    /* The minimum OBEX packet length is 255. As per the specification
    in case of a multi packet request, all obex request messages shall be
    sent as one OBEX packet containing all of the headers */
    /* to make the code simpler we check the ok flag only at the end */
    /* Create Obex object */
    ok = !!ObexObjNew(state->handle); /* 1 if !0 */
    
    /* Add Name object */
    if( ok )
    {
        len = pbapcGetPbPathFromID( (msg->repository == pbap_sim1), 
                                     msg->phonebook, obj ); 
        src = StreamRegionSource( obj, len );
        ok &= (src &&
          ObexObjAddVariableHdr( state->handle, obex_name_hdr, len, src));
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    }

    /* Add Type object */
    if( ok ) 
    {
        type = pbapcGetPhonebookMimeType( &len );
        src = StreamRegionSource( type, len );
        ok &= src &&
          ObexObjAddVariableHdr( state->handle, obex_type_hdr, len, src);
        
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    }

    /* Add Application parameters */ 
    if( ok )
    {
        len = pbapcFrameFilterFormat( msg->filterHi, msg->filterLo, 
                                     msg->format,  obj );
        len += pbapcFrameListAppParams( msg->maxList, 
                                       msg->listStart, &obj[len] );

        src = StreamRegionSource( obj, len );
        ok &= src &&
              ( ObexObjAddVariableHdr( state->handle, 
                                              obex_app_params, 
                                              len, src) );
       
        if (src)
        {
            SourceEmpty(src);
            src = 0;
        }
    } 

    if ( ok )
    {
        ObexGetRequest( state->handle, TRUE );
        state->currCom =  pbapc_com_pull_phonebook;
    }
    else
    {
        ObexObjDelete( state->handle );
        pbapcMsgSendPullPhonebookErrCfm(state,pbapc_ppb_no_param_resources);
    }

    /* Free the object */
    free( obj );
}

/***************************************************************************
 * NAME    
 *  handleIntAuthClg
 *
 * DESCRIPTION
 *  internal handler to send Authentication Challenge
 ***************************************************************************/
static void handleIntAuthClg( pbapcState *state , PBAPC_INT_AUTH_CLG_T* msg)
{
    if( state->currCom == pbapc_com_connect )
    {
        const uint8* nonce = (msg)?msg->nonce:NULL;

        ObexAuthReqResponse( state->handle, nonce,
                             obex_auth_no_option, 0, NULL );
    }
    else
    {
        PBAPC_DEBUG(("Invalid State\n"));
    }
}

/***************************************************************************
 * NAME    
 *  handleIntAuthResp
 *
 * DESCRIPTION
 *  internal handler to send Authentication Response
 **************************************************************************/
static void handleIntAuthResp( pbapcState *state, PBAPC_INT_AUTH_RESP_T* msg)
{
    if( state->currCom == pbapc_com_connect )
    {
        ObexAuthClgResponse( state->handle, msg->digest, 
                             msg->sizeUserId, msg->userId );
    }
    else
    {
        PBAPC_DEBUG(("Invalid State\n"));
    }
}
/***************************************************************************
 * NAME    
 *  handleIntDisconnect
 *
 * DESCRIPTION
 *  internal handler to handle disconnect 
 **************************************************************************/
static void handleIntDisconnect(pbapcState *state)
{
    if( state->handle )
    {
        /* Request disconnect */
        ObexDisconnectRequest( state->handle );
        
        /* If PBAP is calling disconnect before completing
            the connect, do not wait for the disconnect confirmation.
            OBEX will automatically delete the session */
        if( state->currCom != pbapc_com_connect )
        {
            /* Await confirmation of disconnect*/
            state->currCom = pbapc_com_disconnect;
            pbapcMsgSendDisconnectCfm( state, pbapc_failure );
            return;
        }
    }
    
    /* Immediately send confirmation */
    state->currCom = pbapc_com_none;
    pbapcMsgSendDisconnectCfm( state, pbapc_failure );
}


/***************************************************************************
 * NAME    
 *  pbapcIntHandler
 *
 * DESCRIPTION
 *   Handler for messages received by the PBABC Task.
 ***************************************************************************/
void pbapcIntHandler( Task task, MessageId id, Message message)
{
    pbapcState *state  = (pbapcState*) task;

    switch ( id )
    {
        case PBAPC_INT_REG_SDP:
            pbapcRegisterSdpRecord( state );
            break;
        case PBAPC_INT_CONNECT:
            pbapcInitConnection(state, 
                              &((PBAPC_INT_CONNECT_T *)message)->bdAddr);
            break;
        case PBAPC_INT_TASK_DELETE:
            MessageFlushTask(task);
            free(task);
            break;
        case PBAPC_INT_AUTH_CLG:
            handleIntAuthClg(state, (PBAPC_INT_AUTH_CLG_T*)message);
            break;
        case PBAPC_INT_AUTH_RESP:
            handleIntAuthResp(state, (PBAPC_INT_AUTH_RESP_T *)message);
            break;
        case PBAPC_INT_DISCONNECT:
            handleIntDisconnect(state);
            break;
        case PBAPC_INT_GET_PHONEBOOK:
            handleIntPullPb( state, 
                             (PBAPC_INT_GET_PHONEBOOK_T *)message);
            break;
        case PBAPC_INT_SET_PHONEBOOK:
            handleIntSetPb(state,(PBAPC_INT_SET_PHONEBOOK_T*)message);
            break;
        case PBAPC_INT_GET_VCARD_LIST:
            handleIntPullvCardList( state, 
                                    (PBAPC_INT_GET_VCARD_LIST_T *)message);
            break;
        case PBAPC_INT_GET_VCARD:
            handleIntPullvCard( state, (PBAPC_INT_GET_VCARD_T *)message); 
            break;
        case PBAPC_INT_GET_CONTINUE:
            handleIntPullContinue(state,(PBAPC_INT_GET_CONTINUE_T*)message);
            break;
        default:
            PBAPC_DEBUG(("Unexpected id - %x\n", id));
            break;
    }
}

