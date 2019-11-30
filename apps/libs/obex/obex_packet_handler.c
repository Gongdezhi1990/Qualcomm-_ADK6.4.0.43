/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_packet_handler.c

DESCRIPTION
    Internal function to encode and decode the OBEX packets
*/


#include "obex_extern.h"
#include "obex_private.h"
#include <sink.h>
#include <source.h>
#include <stream.h>

/***************************************************************************
* NAME
* obexSourceEmpty
*
* DESCRIPTION
* Empty the source to receive new incoming packets 
****************************************************************************/
void obexSourceEmpty( Obex session )
{
    Source source = StreamSourceFromSink( session->sink );

    /* Empty the source and unblock any incoming data */
    if( session->srcUsed ) 
    {
        if( SourceSize( source ) < session->srcUsed ) SourceEmpty( source ); 
        else  SourceDrop( source, session->srcUsed ); 

        session->srcUsed = 0;
    }

    /* Unblock incoming Data */
    if( SourceBoundary( source ) )
    {
        MessageSend( &session->task, OBEX_MESSAGE_MORE_DATA, 0 );
    }
}

/***************************************************************************
* NAME
* obexSinkSlack
*
* DESCRIPTION
* Verify available length
****************************************************************************/
uint16 obexSinkSlack( Obex session )
{
    uint16 maxLen = SinkSlack(session->sink) + SinkClaim(session->sink, 0);

    if( maxLen > session->maxPktLen) maxLen = session->maxPktLen;

    return ( maxLen - session->pktLen );

}
/***************************************************************************
* NAME
* obexSinkClaim
*
* DESCRIPTION
* Claim the OBEX sink for outgoing packet
****************************************************************************/
bool obexSinkClaim( Obex session, uint16 len )
{
    uint16 space = obexSinkSpace(session) ;

    if(obexSinkSpace(session) >= len ) return TRUE;

    if( len > obexSinkSlack( session )) return FALSE;

    if( SinkClaim( session->sink, len - space) == OBEX_INVALID_UINT16)
    {
        return FALSE;
    } 
    return TRUE;
}


/***************************************************************************
* NAME
*   obexNewPacket
*
* DESCRIPTION
*   Create a New Obex Packet of size. 
*
* RETURNS
*   Size allocated for the packet. Return 0 on failure.
****************************************************************************/
uint16 obexNewPacket( Obex session , uint16 size, uint8 opcode )
{
    uint16 hdrLen;
    uint16 allocLen;

    /* Return 0 if an Outgoing packet is already in the Sink */
    if( session->pktLen ) return 0;

    /* Calculate the size of OBEX packet header */
    if( opcode == OBEX_SET_PATH)
    {
         hdrLen = OBEX_PKT_SETPATH_HDR_SIZE;
    }
    else if( opcode == OBEX_CONNECT_OPCODE )
    {
         hdrLen =  OBEX_PKT_CONNECT_HDR_SIZE;
    }
    else
    {
         hdrLen = OBEX_PKT_HDR_SIZE;
    }

     /* Add the size required for ConnectID if exists */
    if( session->connID != OBEX_INVALID_UINT32)
    {
        hdrLen += OBEX_UINT_HDR_SIZE;
    }

    /* Check Sink has space for the packet */
    if( (allocLen = obexSinkSlack( session )) < (hdrLen + size)) return 0;

    /* Claim sink space only for the OBEX packet header + Connection ID */
    if( !obexSinkClaim( session, hdrLen )) return 0;
    
    /* Fill(virtually) the header */
    session->pktLen = hdrLen;
   
    /* Fill the Connection ID header if exists */
    if( session->connID != OBEX_INVALID_UINT32)
    {
        /* Length will be corrected after adding the header */
        session->pktLen = hdrLen - OBEX_UINT_HDR_SIZE;

        obexAddUint32Header( session,
                             OBEX_CONNECTID_HDR,
                             session->connID);
    }
    else
    {
        session->pktLen = hdrLen;
    }

    /* Return the available length for the remaining headers */
    return allocLen - hdrLen;
}


/***************************************************************************
* NAME
*  obexFrameConnectPkt
*
* DESCRIPTION
*  Frame a Connect Request or Response packet
*
* RETURNS
*  returns 0 on failure
****************************************************************************/
bool obexFrameConnectPkt( Obex session, uint8 opcode )
{
    uint8* dest; 
    uint16 dataLen = 0;

    /* Include Who header to a success connect response and a connect 
       request if a Target header is associated with this session */
    if( (session->targetWho ) &&
        ( (opcode == obex_remote_success) || 
          (opcode == OBEX_CONNECT_OPCODE)))
    {
        dataLen = OBEX_PKT_HDR_SIZE + session->sizeTargetWho;
    } 

    /* Get a New Obex Header for Connect Request */
    if( !obexNewPacket( session , dataLen, OBEX_CONNECT_OPCODE  ) ) 
    {
        return FALSE;
    }

    /* 
     * Frame the packets for Connect Header Do not fill the length field now.
     *   ----------------------------------------------------------------
     *   | Opcode/  |          |          |       | Maximum  | 
     *   | response | Length   | Version  | Flags | OBEX Len |   Headers
     *   ----------------------------------------------------------------
     *         0         1-2        3           4      5-6
     */
    dest = SinkMap(session->sink);
    dest[0] = opcode;
    dest[3] = OBEX_OBEX_VERSION;
    dest[4] = 0;
    OBEX_UINT16_TO_UINT8( dest, session->maxPktLen, 5);

    /* Add the  who/Target Header */
    if( dataLen )
    {
        uint8 local_opcode = IsObexServer( session )? OBEX_WHO_HDR:OBEX_TARGET_HDR;
        Source src = StreamRegionSource( session->targetWho ,
                                         session->sizeTargetWho );
        /* Add the Target/who header */
         if(!obexAddSeqHeader( session, local_opcode, session->sizeTargetWho, src))
         {
            if (src)
            {
                SourceEmpty( src );
            }
            return FALSE;
         }
         
         if (src)
         {
            SourceEmpty( src );
        }
    }

    return TRUE;
}    

/************************************************************************
 * NAME
 *  ObexFlushPacket
 *
 * DESCRIPTION
 *  Send the request or response packet 
 *************************************************************************/
void obexFlushPacket( Obex session, uint8 opcode )
{
    uint8* pkt = SinkMap ( session->sink ); /* Sink must be valid */

    /* Empty the source and unblock any incoming data */
    obexSourceEmpty( session );

    /* Fill the Request opcode or response */
    pkt[0] = opcode;
    
    /* Populate the correct length of the packet */
    OBEX_UINT16_TO_UINT8( pkt, session->pktLen, 1);

    /* Send the OBEX Object */
    obexObjFlush( session, TRUE );
}

/****************************************************************************
 * NAME
 *  ObexHandleIncomingPacket 
 *
 * DESCRIPTION 
 *  Handle incoming OBEX Packet 
 ****************************************************************************/
void obexHandleIncomingPacket( Obex session )
{
    Source source = StreamSourceFromSink( session->sink );
    uint16 srcData;

    while( ((srcData =  SourceBoundary( source )) >= OBEX_PKT_HDR_SIZE) && 
            (session->srcUsed == 0) )
    {
        const uint8* pkt = SourceMap( source );
        uint16 pktLen;

        /* Validate the Length of the packet */
        OBEX_UINT8_TO_UINT16(pktLen, pkt, 1);

        if( pktLen > session->maxPktLen) 
        {
            OBEX_INFO((" Remote sent invalid length pkt\n"));  
            SourceEmpty( source );

            /*  Force Disconnect  for Client */
            if( IsObexClient( session ) ) obexDisconnectReq( session );
            else obexSendErrorResponse( session, obex_bad_request );
            break;
        }

        /* Keep the data in the Source until receiving a complete packet */
        if( pktLen > srcData ) break;

        if( IsObexServer( session ) )
        {
            session->srcUsed = obexHandleCommand( session, source, pktLen );
        }
        else
        {
            session->srcUsed = obexHandleResponse( session, source, pktLen );
        }

        /* Drop the unblocked source Data */
        SourceDrop( source, pktLen - session->srcUsed );
    }   
}


/***************************************************************************
 * NAME
 *  ObexStoreConnectionID
 *
 * DESCRIPTION 
 *  Store the Connection ID extracted from Connect Packet 
 *
 * PARAMETERS
 *  session -   Session Handle
 *  pkt     -   Connect Request or Response pkt
 *  len     -   Length of the Connect Packet
 **************************************************************************/
bool obexStoreConnectionID( Obex session, const uint8* pkt, uint16 len )
{
    /* Extract the Connection ID Header from the Connect packet */
    session->connID  = obexGetUint32Header( pkt, len, OBEX_CONNECTID_HDR );

    return (  session->connID ==  OBEX_INVALID_UINT32 )? FALSE:TRUE;
}

/***************************************************************************
* NAME
*  obexValidateConnectionID
*
* DESCRIPTION
*   Validate the Connection ID. 
*
* PARAMETERS
*   source      : Source containing the OBEX packet.
*   len (INOUT) : Length of the packet as input and length of the unprocessed
*                 packet as output.
*************************************************************************/
bool obexValidateConnectionID(Obex session, Source source, uint16 *len)
{
    const uint8* pkt = SourceMap(source);
    uint32 connID;

    if( pkt[0] == OBEX_SET_PATH )
    {
        if( *len < OBEX_PKT_SETPATH_HDR_SIZE) return FALSE;

        pkt +=  OBEX_PKT_SETPATH_HDR_SIZE;
        *len -=  OBEX_PKT_SETPATH_HDR_SIZE;
    }
    else
    {
        pkt  += OBEX_PKT_HDR_SIZE;
        *len -=  OBEX_PKT_HDR_SIZE;
    }

    connID = obexGetUint32Header( pkt , *len , OBEX_CONNECTID_HDR ); 
    
    if( connID !=  OBEX_INVALID_UINT32 ) 
    {
        *len -= OBEX_UINT_HDR_SIZE;
    }

    if(session->connID == connID) return TRUE;
    return FALSE; 
}

/***************************************************************************
* NAME
* obexSendAuthPacket
*
* DESCRIPTION
* Send a Authentication Request or Response packet
***************************************************************************/
void obexSendAuthPacket( Obex session, uint8 hdr, uint16 size, Source src )
{
    /* Add the Digest header */
    if( size && !obexAddSeqHeader( session, hdr, size, src ) )
    {
        SourceEmpty( src );
        obexConnectCfm( session, obex_failure );
        return;
    }

    if( hdr == OBEX_AUTH_CHALLENGE )
    {
        /* Added Authentication Challenge and waiting for response*/
        SET_OBEX_IN_AUTH( session );  
    }
    else
    {
        /* Added Authentication Response header. incoming auth completed */
        session->auth = FALSE;
    }

    if( IsObexServer( session ) )
    {
        uint8 response = ( session->auth)? OBEX_UNAUTHORIZED_RSP:
                              OBEX_SUCCESS;  
        obexFlushPacket( session, response ); 
        if(response == OBEX_SUCCESS) obexConnectCfm( session, response );
    }
    else if( IsObexInAuthConnect ( session ) && !session->auth )
    {
        /* Flush only after adding both headers as client */
        obexFlushPacket( session, OBEX_CONNECT_OPCODE );
    }
}
