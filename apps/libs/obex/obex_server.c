/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_server.c
    
DESCRIPTION
    This file defines all API functions for a OBEX server session.
    The library allows creation of multiple OBEX sessions to multiple 
    devices. It is the applications responsibility to limit the number of 
    sessions for optimal resource usage.
*/

#include "obex_extern.h"
#include <stdlib.h>

/****************************************************************************
 * NAME
 *  ObexConnectResponse
 * 
 * DESCRIPTION
 *  API to establish a OBEX server session 
 *
 * PARAMETERS
 *  Refer obex.h
 *
 * RETURNS
 **************************************************************************/
Task ObexConnectResponse( Sink					sink,
                          uint8                 rfcChannel,
                          bool                  accept,
                          const ObexConnParams* connParams)  
{
    ObexConnId connId;
    ObexChannel   channel;

    connId.u.sink = sink;
    channel.l2capObex = FALSE;
    channel.u.channel = rfcChannel;

    return obexConnectResponse( connId, channel, accept, connParams );     
}
   
/**************************************************************************
 * NAME
 *  ObexPutResponse
 * 
 * DESCRIPTION
 *  API to send a PUT response
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexPutResponse( Obex session, ObexResponse response )
{
    OBEX_ASSERT( session );
    obexSendResponse( session, response );
}

/**************************************************************************
 * NAME
 *  ObexGetResponse
 * 
 * DESCRIPTION
 *  API to send a GET response
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexGetResponse( Obex session, ObexResponse response )
{
    OBEX_ASSERT( session );
    obexSendResponse( session, response );
}

/**************************************************************************
 * NAME
 *  ObexSetPathResponse
 * 
 * DESCRIPTION
 *  API to send a SETPATH response
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexSetPathResponse( Obex session, ObexResponse response )
{
    OBEX_ASSERT( session );
    if(response == obex_continue ) response = obex_remote_success;

    obexSendResponse( session, response );
}


#ifdef GOEP_VERSION_2_0

/****************************************************************************
 * NAME
 *  ObexL2capConnectResponse
 * 
 * DESCRIPTION
 *  API to establish a OBEX server session over L2cap
 *
 * PARAMETERS
 *  Refer obex.h
 *
 * RETURNS
 **************************************************************************/
Task ObexL2capConnectResponse( uint16                psm,
                               uint8                 identifier,
                               uint16                connId,
                               bool                  accept,
                               const ObexConnParams* connParams ) 
{
    ObexConnId    id;
    ObexChannel   channel;

    id.u.l2cap.identifier = identifier;
    id.u.l2cap.connId = connId;
 
    channel.l2capObex = TRUE;
    channel.u.psm = psm;

    return obexConnectResponse( id, channel, accept, connParams ); 

}


#endif /* GOEP_VERSION_2_0 */

