/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
Part of BlueLab 5.0.0.30

FILE NAME
    obex_server_handler.c
    
DESCRIPTION
    This internal file handles all incoming and outgoing OBEX server messages
*/

#include "obex_extern.h"
#include "obex_private.h"
#include <source.h>



/**************************************************************************
 * NAME
 *  obexHandleAbort
 *
 * Description
 *  Handle the Abort
 *
 * PARAMETERS
 *  session - OBEX session
 **************************************************************************/
static void obexHandleAbortRequest( Obex session )
{

    /* indicate the application that the current operation has been aborted */
    if( IsObexInPut( session ) || IsObexInGet( session ))
    {
        obexAbortInd( session );
    }

    /* Respond to Abort */
    obexSendResponse( session, obex_remote_success );

}

/**************************************************************************
 * NAME
 *  obexSendResponse
 *
 * Description
 *   Issue a OBEX Response
 *
 * PARAMETERS
 *  session - OBEX session
 *  response - response code
 **************************************************************************/
void obexSendResponse( Obex session, ObexResponse response )
{
    if( response == obex_success ) response = obex_remote_success;
    else if( response < obex_continue ) response = obex_unknown_error;
    else if( IsObexSrmFlowEnabled(session) ) 
    {
        obexSourceEmpty( session );
        return;
    }      

    /* Assuming the response for PUT. It can change later */
    if( session->pktLen ||  obexNewPacket( session , 0, OBEX_PUT  ) )
    {
        obexFlushPacket( session, response );
        if( !IsObexSrmEnabled(session) ) 
        {
            obexChangeSrmMode( session, obex_srm_deactivate );
        }
    }
    else
    {
        OBEX_ERR(("No Resource\n"));
    }

    /* Keep the same state on continue */
    if( response == obex_continue )
    {
        if( IsObexSrmFlowGo( session ) && IsObexInGet( session) )
        {
            SET_OBEX_SRM_LOCK( session );
        }
    }
    else
    {
         SET_OBEX_CONNECTED( session );
         obexChangeSrmMode( session, obex_srm_deactivate ); 
    }
}

/**************************************************************************
 * NAME
 *  obexHandleDisconnectRequest
 *
 * Description
 *  Handle the incoming Disconnect request 
 *
 * PARAMETERS
 *  session - OBEX session.
 *  source  - The Source buffer.
 *  pktLen  - Length of data in the Source.
 **************************************************************************/
static void obexHandleDisconnectRequest( Obex session,
                                         Source source, 
                                         uint16 pktLen )
{
    if( obexValidateConnectionID( session, source, &pktLen ) ) 
    {
        obexSendResponse( session, obex_remote_success );

        /* Set the mode into disconnecting */
        OBEX_OBJ_DELETE( session );
        obexDeleteSessionInd( session );
    }
    else
    {
        obexSendErrorResponse( session, obex_srv_unavailable );
    }
}

/**************************************************************************
 * NAME
 *  obexHandleConnectRequest
 *
 * DESCRIPTION
 *  Handle the OBEX CONNECT Request packet 
 *
 * RETURNS
 *  Length of unprocessed packet. 
 **************************************************************************/
static uint16 obexHandleConnectRequest( Obex session, 
                                        Source source, 
                                        uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    ObexResponse response = obex_remote_success;

    if( pktLen < OBEX_PKT_CONNECT_HDR_SIZE )
    {
        /* Validate the PktLen */
        response = obex_bad_request;
    }
    else if( (IsObexConnecting( session ) ))
    {
        uint16 maxPktLen;

        /* Not interested in OBEX Version and Flags now. 
           Extract the maximum  packet length from bytes 5,6 */
        OBEX_UINT8_TO_UINT16( maxPktLen, pkt, 5);

        if( session->targetWho  )
        {
            pkt += OBEX_PKT_CONNECT_HDR_SIZE;
            pktLen -= OBEX_PKT_CONNECT_HDR_SIZE;

            /* The Connect request must have a valid Target */ 
            if( obexValidateSession( session, pkt, pktLen) )
            { 
                if( maxPktLen < session->maxPktLen ) 
                {
                    session->maxPktLen = maxPktLen;
                }

                if(!obexAuthenticateSession(session, pkt , &pktLen ))
                {
                    response = IsObexInAuthConnect( session )? 
                               obex_unauthorized: OBEX_UNAUTHORIZED_RSP;
                } 
            }
            else
            {
                response = obex_not_implemented;
            }
        }
    }
    else
    {
        response = obex_precon_fail;
    }

    /* Frame a Connect Packet */
    if( obexFrameConnectPkt( session, response ) )
    {
        if( response == OBEX_UNAUTHORIZED_RSP || 
          ( (response == obex_remote_success) && session->auth )) return 0; 
        obexFlushPacket( session, response );
    }
    else
    {
        response = obex_failure; 
    }

    /* set in session since remote client will disconnect */
    if( response != obex_remote_success ) SET_OBEX_IN_SESSION( session );
    obexConnectCfm( session, response );  
    return 0;
}
/**************************************************************************
 * NAME
 *  obexHandleSetPathRequest
 *
 * DESCRIPTION
 *  Handle the OBEX SET PATH request packet
 *
 * RETURNS
 *  Length of unprocessed packet. 
 **************************************************************************/
static uint16 obexHandleSetPathRequest( Obex session, 
                                        Source source, 
                                        uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    ObexResponse response = obex_remote_success;
    ObexFolderPath path = 0;

    if( IsObexReady( session ) ) 
    {
        if(obexValidateConnectionID( session, source, &pktLen) )
        {
            /* Check the flags in last 2 bits of byte 3*/
            path = pkt[0] & 0x03;
            
            if( path == obex_folder_out )
            {
                pktLen  = 0;
            }
            else if( ( !obexGetSeqHeader( pkt, &pktLen, obex_name_hdr ) ) ||
                     ( pktLen == 0 ) )
            { 
                /* This must happen only for folder root */
                path = obex_folder_root;
                pktLen = 0;
            }
        }
        else
        {
            response = obex_bad_request;
        }  
    }
     else
    {
        response = obex_srv_unavailable;
    }

    if( response == obex_remote_success )
    {
        SET_OBEX_IN_SETPATH(session);
        obexCommandInd( session, OBEX_SET_PATH_IND, path , source, pktLen );
    }
    else
    {
        obexSendResponse( session, response );
        pktLen = 0;
    }

    return pktLen;
}


/**************************************************************************
 * NAME
 *  obexHandlePutGetRequest
 *
 * DESCRIPTION
 *  Handle the OBEX transaction PUT/GET request packet
 *
 * RETURNS
 *  Length of unprocessed packet. 
 **************************************************************************/
static uint16 obexHandlePutGetRequest( Obex session, 
                                      Source source, 
                                      uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    ObexResponse response = obex_remote_success;
    uint8 cmd = pkt[0];
    
    if( IsObexReady( session ) )
    {
        if( IsObexConnected ( session ) )
        {
            if( ! obexValidateConnectionID( session, source, &pktLen))
            {
                 obexSendResponse( session,   obex_srv_unavailable ); 
                 return 0;
            }
        } 
        else
        {
            pktLen -= OBEX_PKT_HDR_SIZE;
        }

        /* Only allowed client states are obex_put and obex_get */
        if( IsObexInPut( session ) && 
            (cmd == OBEX_GET || cmd == OBEX_GET_FINAL ))
        {
            response = obex_bad_request ;
        }
        
    }
    else
    {
        response = obex_srv_unavailable;
    }

    if( response == obex_remote_success )
    {
        ObexMessageId id = OBEX_PUT_IND;

        if( cmd == OBEX_GET || cmd == OBEX_GET_FINAL)
        {
            id = OBEX_GET_IND;
            SET_OBEX_IN_GET(session);
        }
        else
        {
            SET_OBEX_IN_PUT(session);
        }
        obexHandleSrm( session, cmd , source, pktLen );

        obexCommandInd( session, id, IsObexFinalCmd(cmd), source, pktLen );
    }
    else
    {
        obexHandleSrm( session, 0, source, pktLen );

        obexSendResponse( session, response );
   
        pktLen = 0;
    }
   
    return pktLen;
}

/**************************************************************************
 * NAME
 *  obexSendErrorResponse
 *
 * DESCRIPTION
 *  Send a error response
 *
 * PARAMETERS
 *  session - OBEX session
 *  response - Response Code
 ************************************************************************/
void obexSendErrorResponse( Obex session, ObexResponse response )
{
    SET_OBEX_CONNECTED( session ); /* put the state back to Connected */
    if( obexNewPacket( session , OBEX_PKT_HDR_SIZE, OBEX_PUT )) 
    {
        obexFlushPacket( session, response );
    }
}


/**************************************************************************
 * NAME
 *  obexHandleCommand
 *
 * DESCRIPTION
 *  Handle the OBEX Command packet 
 *
 * RETURNS
 *  Length of unprocessed packet
 ***************************************************************************/
uint16 obexHandleCommand( Obex session, Source source, uint16 pktLen )
{
    const uint8* pkt = SourceMap( source );
    uint16  rspLen = 0;
    
    switch( pkt[0] )
    {
        case OBEX_CONNECT_OPCODE:
            rspLen = obexHandleConnectRequest( session, source, pktLen );
            break;

        case OBEX_DISCONNECT:
            obexHandleDisconnectRequest( session, source, pktLen ); 
            break;

        case OBEX_ABORT:
            obexHandleAbortRequest( session ); 
            break;

        case OBEX_SET_PATH:
            rspLen = obexHandleSetPathRequest( session, source, pktLen );
            break;

        case OBEX_PUT:
        case OBEX_PUT_FINAL:
        case OBEX_GET:
        case OBEX_GET_FINAL: /* fall through */
           rspLen = obexHandlePutGetRequest( session, source, pktLen ); 
           break;

        default:
            obexSendErrorResponse( session, obex_not_implemented );
            break;
    }
    
    return rspLen;
} 

