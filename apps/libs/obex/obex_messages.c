/****************************************************************************
Copyright (c) 2010 - 2016 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_message.c
    
DESCRIPTION
   All interface messages are despatched from here
*/

#include <panic.h>
#include <string.h>
#include "obex_extern.h"
#include "obex_private.h"

/*********************************************************************
 * NAME:
 *  obexCreateSessionCfm
 *
 * DESCRIPTION 
 *  Despatch OBEX_CREATE_SESSION_CFM Message 
 *********************************************************************/
void obexCreateSessionCfm(  Obex    session,
                            ObexStatus   status,
                            const bdaddr *addr)
{
    MAKE_OBEX_MESSAGE( OBEX_CREATE_SESSION_CFM );

    message->status = status;
    message->session = session;
    message->remoteAddr = *addr;
    message->channel =  session->channel;

    MessageSend( session->theApp, OBEX_CREATE_SESSION_CFM, message );
}


/**************************************************************************
 * NAME
 *  obexConnectCfm
 *
 * DESCRIPTION 
 * Despatch OBEX_CONNECT_CFM message.
 ***************************************************************************/
void obexConnectCfm( Obex         session,
                     ObexStatus   status )
{
    ObexMessageId id;
    MAKE_OBEX_MESSAGE( OBEX_CONNECT_CFM );

    message->session = session;
    message->status = status;
	message->maxPacketLen = session->maxPktLen;
    id = IsObexL2cap( session )? OBEX_L2CAP_CONNECT_CFM: OBEX_CONNECT_CFM;
    MessageSend(session->theApp, id, message);

    if( status == obex_remote_success )
    {
        SET_OBEX_CONNECTED( session );
    }
    else if( IsObexConnecting( session ) )
    {
        /* Connection failed. Delete the Session */
        obexDeleteSessionInd( session );
    }
}

/****************************************************************************
 * NAME
 *  obexDeleteSessionInd
 *
 * DESCRIPTION
 *  Dispatch a delete session Indication 
 **************************************************************************/
void obexDeleteSessionInd( Obex session )
{
    MAKE_OBEX_MESSAGE( OBEX_DELETE_SESSION_IND );

    if( IsObexReady (session )) SET_OBEX_IN_SESSION( session );
    message->session = session;
    message->connTask   = session->theApp;
    MessageSend( session->theApp, OBEX_DELETE_SESSION_IND, message );
}

/****************************************************************************
 * NAME
 *  obexStatusCfm
 *
 * DESCRIPTION
 *  Dispatch Obex Confirmation Message
 ****************************************************************************/
void obexStatusCfm( Obex        session, 
                    ObexStatus  status,
                    ObexMessageId id,
                    Source  src,
                    uint16  srcLen ) 
                            
{
    MAKE_OBEX_MESSAGE( OBEX_MESSAGE_STATUS );
    message->session = session;
    message->status = status;
    message->hdrs = src;
    message->hdrsLen = srcLen;
   
    MessageSend( session->theApp, id, message); 
}

/***************************************************************************
 * NAME
 *  obexCommandInd
 *
 * DESCRIPTION
 *  Dispatch a Indication message to the application.
 ***************************************************************************/
void obexCommandInd( Obex        session, 
                     ObexMessageId id,
                     bool          final,
                     Source        src,
                     uint16        srcLen ) 
                            
{
    MAKE_OBEX_MESSAGE( OBEX_MESSAGE_IND );
    message->session = session;
    message->hdrs = src;
    message->final = final;
    message->hdrsLen = srcLen;
   
    MessageSend( session->theApp, id, message); 
}

/***********************************************************************
 * NAME
 *  ObexAbortInd
 *
 * DESCRIPTION 
 *  Dispatch an indication message that the current operation has been 
 *  Aborted.
 ***********************************************************************/
void obexAbortInd( Obex session ) 
{
    MAKE_OBEX_MESSAGE( OBEX_ABORT_IND );
    message->session = session;
    MessageSend( session->theApp, OBEX_ABORT_IND, message );
}

/***********************************************************************
 * NAME
 *  ObexAuthReqInd
 *
 * DESCRIPTION 
 *  Dispatch an indication message to the application to start 
 *  Authentication of an ongoing connection.
 ***********************************************************************/
void obexAuthReqInd( Obex session )
{
    MAKE_OBEX_MESSAGE( OBEX_AUTH_REQ_IND );
    message->session = session;
    MessageSend( session->theApp, OBEX_AUTH_REQ_IND, message );
}

/***********************************************************************
 * NAME
 *  ObexAuthClgInd
 *
 * DESCRIPTION 
 *  Dispatch an indication message to the application to start 
 *  Authentication of an ongoing connection.
 *
 ***********************************************************************/
void obexAuthClgInd( Obex session, const uint8* pkt, uint16 pktLen )
{
    uint8 offset = OBEX_TLV_HDR_SIZE;
    MAKE_OBEX_MESSAGE( OBEX_AUTH_CLG_IND );
    message->session = session;
    message->options = 0;
    message->sizeRealm = 0;
    message->realm = NULL;

    memmove( message->nonce, &pkt[offset], OBEX_SIZE_DIGEST );
    offset += OBEX_SIZE_DIGEST;
  
    if( ( pktLen > offset + OBEX_TLV_HDR_SIZE ) &&
        ( pkt[offset] == OBEX_REQ_OPTIONS_TAG ) )  
    {
        offset += OBEX_TLV_HDR_SIZE;
        message->options = pkt[offset]; 
        offset++;
    }
    
    if( ( pktLen > offset + OBEX_TLV_HDR_SIZE ) &&
        ( pkt[offset] ==  OBEX_REQ_REALM_TAG ) )   
    {
        offset++;
        if( ( pktLen - offset ) >= pkt[offset] )
        {
            message->sizeRealm = pkt[offset++];
            message->realm = &pkt[offset];
        }
    }
     
    MessageSend(session->theApp, OBEX_AUTH_CLG_IND, message );
}

/***********************************************************************
 * NAME
 *  ObexAuthRspCfm
 *
 * DESCRIPTION 
 *  Dispatch the Authentication Response header to the application. 
 ***********************************************************************/
void obexAuthRspCfm( Obex session, const uint8* pkt, uint16 pktLen )
{
    uint16 size = 0;

    /* Find out User ID is present */
    if( ( pktLen > OBEX_TOTAL_DIGEST_SIZE + OBEX_TLV_HDR_SIZE) &&
        ( pkt[OBEX_TOTAL_DIGEST_SIZE] == OBEX_RSP_USRID_TAG) )
    {
         size  =  pkt[OBEX_TOTAL_DIGEST_SIZE+1] > OBEX_MAX_VAL_SIZE?   
                   OBEX_MAX_VAL_SIZE: pkt[OBEX_TOTAL_DIGEST_SIZE+1];
    }

    {
        uint8 offset = OBEX_TLV_HDR_SIZE;
        MAKE_OBEX_MESSAGE_WITH_LEN( OBEX_AUTH_RSP_CFM, size );         
        message->session = session;
        memmove( message->request, &pkt[offset], OBEX_SIZE_DIGEST );

        offset +=  OBEX_TOTAL_DIGEST_SIZE;

        if ( pktLen >= offset +  size )
        {
            message->sizeUserId  = size;
            memmove( message->userId, &pkt[offset], size );
        }
        else
        {
            message->sizeUserId  = 0;
        }
        
        MessageSend( session->theApp, OBEX_AUTH_RSP_CFM, message );
    }
}
