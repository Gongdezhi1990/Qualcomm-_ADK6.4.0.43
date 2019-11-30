/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_client_handler.c
    
DESCRIPTION
    This internal file handles all incoming and outgoing OBEX client messages
*/

#include "obex_extern.h"
#include "obex_private.h"
#include <source.h>
#include <sink.h>


/**************************************************************************
 * NAME
 *  obexConnectReq
 *
 * Description
 *   Issue a OBEX Connect Request 
 *
 * PARAMETERS
 *  session - OBEX session
 *  flush   - No more headers to add, send the packet.
 **************************************************************************/
bool obexConnectReq( Obex session, bool flush )
{
    /* Frame a Connect Packet */
    if(!obexFrameConnectPkt( session, OBEX_CONNECT_OPCODE ))
    {
        /* Send a failure Connect Confirmation Message */
        obexConnectCfm( session, obex_failure );
        return FALSE;
    }

    /* Flush the packet if requested */
    if( flush ) obexFlushPacket( session, OBEX_CONNECT_OPCODE );

    return TRUE;
}

/**************************************************************************
 * NAME
 *  obexPutReq
 *
 * Description
 *   Issue an OBEX Put Request 
 *
 * PARAMETERS
 *  session - OBEX session
 *  opcode - PUT or PUT_FINAL 
 **************************************************************************/
void obexPutReq( Obex session, uint8 opcode )
{
    /* Put is allowed if the session is free or already in a multi-put */
    if( !(IsObexConnected( session ) ||  IsObexInPut( session )))
    {
         OBEX_OBJ_DELETE( session );
         obexPutErrCfm( session, obex_invalid_command ); 
         return;     
    }

    /* check the PUT Request Object has at least an empty Body header */
    if(( IsObexConnected( session ) ) &&  
       ( session->pktLen < OBEX_MIN_PUT_REQ_SIZE ) )
    {
        /* Invalid parameters */
         OBEX_OBJ_DELETE( session );
         obexPutErrCfm( session, obex_invalid_parameters );
         return;
    }           

    if( IsObexInPut( session ) && ( session->pktLen == 0 ) )
    {
        obexNewPacket( session, 0, OBEX_PUT ); 
    }    

    /* send the packet */
    obexFlushPacket( session, opcode );

    /* set the state */        
    SET_OBEX_IN_PUT( session );

    if( IsObexSrmFlowGo ( session ) && (opcode == OBEX_PUT) )
    {
        /* Set the SRM lock to send more data */
        SET_OBEX_SRM_LOCK(session );
    } 
}

/**************************************************************************
 * NAME
 *  obexGetReq
 *
 * Description
 *   Issue an OBEX GET Request 
 *
 * PARAMETERS
 *  session - OBEX session
 *  opcode - GET or GET_FINAL 
 **************************************************************************/
void obexGetReq( Obex session, uint8 opcode )
{
    /* Get if accept if the session is free or in a multi-get state */
    if( !(IsObexConnected( session ) ||  IsObexInGet( session )))
    {
         OBEX_OBJ_DELETE( session );
         obexGetErrCfm( session, obex_invalid_command ); 
         return;     
    }

    /* There must be some headers for the first GET operation */
    if( (IsObexConnected( session ) && 
        (session->pktLen < OBEX_MIN_GET_REQ_SIZE )) )
    {
        /* Invalid parameters */
         OBEX_OBJ_DELETE( session );
         obexGetErrCfm( session, obex_invalid_parameters );
         return;
    }

    /* Create a new Object for an empty GET continue request */
    if( IsObexInGet( session ) && ( session->pktLen == 0 ) )
    {
        obexNewPacket( session, 0, OBEX_GET ); 
    }    

    /* send the packet */
    obexFlushPacket( session, opcode );

    /* set the state */        
    SET_OBEX_IN_GET( session );

    if( IsObexSrmFlowGo ( session ))
    {
        SET_OBEX_SRM_LOCK( session );
    } 
}


/**************************************************************************
 * NAME
 *  obexDisconnectReq
 *
 * Description
 * Issue a OBEX Disconnect Request and disconnect the session  
 *
 * PARAMETERS
 *  session - OBEX session
 **************************************************************************/
void obexDisconnectReq( Obex session )
{
    /* Check the current state */
    if( (IsObexReady ( session )))
    {
        OBEX_OBJ_DELETE( session );
        SET_OBEX_IN_DISCONNECT( session );
        /* Set the mode into disconnecting */
        if( obexNewPacket( session, 0, OBEX_DISCONNECT) )
        {
            /* send the packet */
            obexFlushPacket( session, OBEX_DISCONNECT );
            return;
        }
    }

    /* OBEX disconnect hasn't been sent. Disconnect the RFCOMM */
    obexDisconnectSession( session );

}

/**************************************************************************
 * NAME
 *  obexSetPathReq
 *
 * Description
 *   Issue a OBEX Setpath Request 
 *
 * PARAMETERS
 *  session - OBEX session
 *  flags   - Setpath flags
 **************************************************************************/
void obexSetPathReq( Obex session, uint8 flag )
{
    uint8* pkt = SinkMap( session->sink );

   /* Check the current state */
    if( !(IsObexConnected( session )))
    {
        /* Invalid state */
         OBEX_OBJ_DELETE( session );
         obexSetPathCfm( session, obex_invalid_command ); 
         return;     
    }

    /* 3rd byte of setpath is flag and 4th byte reserved */
    pkt[3] = flag;
    pkt[4] = 0;  

    /* send the packet */
    obexFlushPacket( session, OBEX_SET_PATH );

    /* set the state */        
    SET_OBEX_IN_SETPATH( session );
}

/**************************************************************************
 * NAME
 *  obexAbortReq
 *
 * Description
 *   Issue a OBEX Abort Request
 *
 * PARAMETERS
 *  session - OBEX session
 **************************************************************************/
void obexAbortReq( Obex session )
{
    /* Handle this request only for multi packet GET or PUT */
    if( ( IsObexInGet( session ) || IsObexInPut ( session ) ) &&
        ( obexNewPacket( session, 0, OBEX_ABORT ) ) )
    {
       /* send the packet */
        obexFlushPacket( session, OBEX_ABORT);
        SET_OBEX_IN_ABORT( session );
    }
}

/**************************************************************************
 * NAME
 *  obexDeleteReq
 *
 * Description
 *   Issue a OBEX Delete Request 
 *
 * PARAMETERS
 *  session - OBEX session
 **************************************************************************/
void obexDeleteReq( Obex session )
{
   /* Check the current state */
    if( !(IsObexConnected( session )))
    {
        /* Invalid state */
        OBEX_OBJ_DELETE( session );
        obexDeleteCfm( session, obex_invalid_command );
        return; 
    }

    /* send the packet */
    obexFlushPacket( session, OBEX_PUT_FINAL );

    /* set the state */        
    SET_OBEX_IN_DELETE( session );
}

/**************************************************************************
 * NAME
 *  obexHandleConnectResponse
 *
 * DESCRIPTION
 *  Handle the OBEX CONNECT Response packet 
 *
 * RETURNS
 *  Length of unprocessed packet. 
 ***************************************************************************/
uint16 obexHandleConnectResponse( Obex session, Source source, uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    uint16 maxPktLen;
    uint8 response = pkt[0];

    /* Validate the PktLen */
    if( pktLen < OBEX_PKT_CONNECT_HDR_SIZE )
    {
        /* Send Failure Connect Cfm Message */
        obexConnectCfm( session,  obex_invalid_response );   
        return 0;
    }

    /* Not interested in OBEX Version and Flags now. 
       Extract the maximum  packet length from bytes 5,6 */
    OBEX_UINT8_TO_UINT16( maxPktLen, pkt, 5);

    pktLen -= OBEX_PKT_CONNECT_HDR_SIZE;
    pkt  +=  OBEX_PKT_CONNECT_HDR_SIZE; 

    /* To work with PTS v4.0, change the error code if authentication 
       was not requested */
    if( ( response ==  obex_unauthorized ) &&
         !(IsObexInAuthConnect( session )) ) 
    {
         response = OBEX_UNAUTHORIZED_RSP;
    } 

    if( response == OBEX_UNAUTHORIZED_RSP )
    {
        /* Frame another Connect Request for authenticated Connect */  
        if( obexConnectReq( session , FALSE ) )
        {
            /* Initiate local authentication */
            session->auth = TRUE;

            if( !obexAuthenticateSession( session, pkt, &pktLen ) )
            {
                obexConnectCfm( session, obex_invalid_response );
            }                            
        } 
    }
    else if( response == OBEX_SUCCESS )
    {
        if( session->targetWho && 
            !( obexStoreConnectionID( session, pkt, pktLen) &&
               obexValidateSession( session, pkt, pktLen) ) )
        {
            obexConnectCfm( session, obex_local_unauth );
            return 0;
        }

        /* Do not reject the connection if server is not sending the 
           Authentication response. This is to interop with PTS v4.0 */  
        obexAuthenticateSession( session, pkt, &pktLen );
  
        /* Update the maxPktLen on Success */
        if( maxPktLen < session->maxPktLen ) session->maxPktLen = maxPktLen;
         
        obexConnectCfm( session,  obex_remote_success );
    }
    else
    {
        /* Send a Failure Confirmation Message */
        obexConnectCfm( session, obex_failure );
		/* Return 0 to drop the entire packet in case of failure*/
		return 0;
    }
    
    return pktLen;
}

/**************************************************************************
 * NAME
 *  obexHandleGetPutResponse
 *
 * DESCRIPTION
 *  Handle the GET or the PUT response packet
 *
 * RETURNS
 *  Length of unprocessed packet. 
 ***************************************************************************/
uint16 obexHandleGetPutResponse( Obex session, Source source, uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    uint16 hdrLen = pktLen;
    uint8 status = pkt[0];
    ObexMessageId id; 

    /* Validation of Connection ID is not required. But just extract
        it if exist */
    obexValidateConnectionID( session, source, &hdrLen );

    /* Send the Confirmation message */
    id = IsObexInPut( session )?  OBEX_PUT_CFM : OBEX_GET_CFM;
    obexStatusCfm( session, status, id, source, hdrLen );
    
    /* Do not change the state in case of multi packet transaction */
    if( status == obex_continue )
    {
        obexHandleSrm( session, IsObexInPut(session)?OBEX_PUT:OBEX_GET,
                       source, hdrLen );
    }
    else
    {
        SET_OBEX_CONNECTED( session );
        obexHandleSrm( session, 0, source, hdrLen );
    }

    return hdrLen;
}

/**************************************************************************
 * NAME
 *  obexHandleAbortResponse
 *
 * DESCRIPTION
 *  Handle the Abort response packet
 *
 ***************************************************************************/
void obexHandleAbortResponse( Obex session, ObexStatus status )
{
    if( status == obex_remote_success) status = obex_local_abort;

    if( session->state == obex_abort_put )
    {
        obexPutErrCfm( session, status );
    }
    else
    {
        obexGetErrCfm( session, status );
    }
    SET_OBEX_CONNECTED( session );

    /* Disconnect the OBEX connection on remote error */
    if( status != obex_local_abort) obexDisconnectReq( session );
}

/**************************************************************************
 * NAME
 *  obexHandleResponse
 *
 * DESCRIPTION
 *  Handle the OBEX Response packet 
 *
 * RETURNS
 *  Length of unprocessed packet
 ***************************************************************************/
uint16 obexHandleResponse( Obex session, Source source, uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    uint16  rspLen = 0;
    ObexStatus status = pkt[0];

    switch( session->state )
    {
        case obex_connect:
        case obex_auth_connect: /* Fall Through */
            rspLen = obexHandleConnectResponse( session, source, pktLen );
            break;

        case obex_disconnect:
            obexDeleteSessionInd( session );
            break;

        case obex_abort_get:
        case obex_abort_put:
            obexHandleAbortResponse( session, status );
            break;

        case obex_delete:
            SET_OBEX_CONNECTED( session );
            obexDeleteCfm( session, status );
            break;
            
        case obex_setpath:
            SET_OBEX_CONNECTED( session );
            obexSetPathCfm( session, status );
            break;

        case obex_put:
        case obex_get: /* fall through */
           rspLen = obexHandleGetPutResponse( session, source, pktLen );
           break;

        default:
            break;
    }
    
    return rspLen;
}             
  


