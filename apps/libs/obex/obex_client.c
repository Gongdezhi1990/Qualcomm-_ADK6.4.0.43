/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_client.c
    
DESCRIPTION
    This file defines all API functions for a OBEX client session.
    The library allows creation of multiple OBEX session to multiple 
    devices. It is the applications responsibility to limit the number of 
    sessions for optimal resource usage.
*/

#include "obex_extern.h"
#include <source.h>
#include <stream.h>
#include <stdlib.h>

/****************************************************************************
 * NAME
 *  ObexConnectRequest
 * 
 * DESCRIPTION
 *  API to establish a OBEX client session 
 *
 * PARAMETERS
 *  Refer obex.h
 *
 * RETURNS
 *  The connection context task to which it will notify on session establishment.
 **************************************************************************/
Task ObexConnectRequest( const bdaddr          *addr,
                        uint8                 rfcChannel,
                        const ObexConnParams  *connParams )
{
    ObexChannel channel;
    if( !connParams) return NULL;

    channel.l2capObex = FALSE;
    channel.u.channel = rfcChannel;

    return obexConnectRequest( addr, channel, connParams );
}

/**************************************************************************
 * NAME
 *  ObexPutRequest
 * 
 * DESCRIPTION
 *  API to send a PUT request
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexPutRequest( Obex session, bool final )
{
    uint8 opcode= OBEX_PUT;

    OBEX_ASSERT( session ); 

    if( final ) opcode = OBEX_PUT_FINAL;
    
    obexPutReq(session, opcode );
}
                     
/**************************************************************************
 * NAME
 *  ObexGetRequest
 * 
 * DESCRIPTION
 *  API to send a GET request
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexGetRequest( Obex session, bool final )
{
    uint8 opcode= OBEX_GET;

    OBEX_ASSERT( session ); 

    if( final ) opcode = OBEX_GET_FINAL;
    
    obexGetReq(session, opcode );
}

/*************************************************************************
 * NAME
 *  ObexAbort
 *
 * DESCRIPTION
 *  API to send Abort request
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexAbort( Obex session )
{
    OBEX_ASSERT( session ); 
    obexAbortReq( session );
}

/**************************************************************************
 * NAME
 *  ObexDeleteRequest
 * 
 * DESCRIPTION
 *  API to send Delete request
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexDeleteRequest(Obex session, uint16 objLen , const uint8* objName)
{
    Source name=0;
    OBEX_ASSERT( session ); 
    OBEX_ASSERT( objName );

    /* Create a new Object for PUT. Delete is PUT with no Body */ 
    if( !obexNewPacket ( session, objLen * 2 , OBEX_PUT) )
    {
        obexDeleteCfm( session, obex_failure );
        return;
    }

    /* Create the name header source */
    name = StreamRegionSource( objName, objLen  );

    /* Add Name header to the request and send the request */
    if(ObexObjAddVariableHdr( session, obex_name_hdr, objLen, name))
    {
       obexDeleteReq( session ); 
    }
    else
    {
        obexDeleteCfm( session, obex_failure );
        ObexObjDelete( session );
    }
    
    if (name)
    {
        SourceEmpty( name );
    }
}
 
/**************************************************************************
 * NAME
 *  ObexSetPathRequest
 * 
 * DESCRIPTION
 *  API to send a SETPATH request
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexSetPathRequest( Obex session, 
                         ObexFolderPath flags, 
                         uint16 folderLen, 
                         const uint8* folderName)
{
    Source name=0;
    OBEX_ASSERT( session ); 

    /* An empty name header in the setpath sets the folder to root */
    if(flags == obex_folder_root)
    {
        folderName = NULL;
        folderLen = 0;
        flags = obex_folder_in;
    }
   
    /* Create a new Object for Setpath */ 
    if( !obexNewPacket ( session, folderLen * 2 , OBEX_SET_PATH) )
    {
        obexSetPathCfm( session, obex_failure );
        return;
    }

    /* Create the name header source */
    if( folderLen )  name = StreamRegionSource(folderName, folderLen);

    /* Add Name header to the Setpath request and send the request */
    if(ObexObjAddVariableHdr( session, obex_name_hdr, folderLen, name))
    {
       obexSetPathReq( session, flags ); 
    }
    else
    {
        obexSetPathCfm( session, obex_failure );
        ObexObjDelete( session );
    }
    
    if (name)
    {
        SourceEmpty( name );
    }
}

/**************************************************************************
 * NAME
 *  ObexDisconnectRequest
 * 
 * DESCRIPTION
 *  API to disconnect the OBEX session.
 *
 * PARAMETERS
 *  Refer obex.h
 **************************************************************************/
void ObexDisconnectRequest( Obex session )
{
    OBEX_ASSERT( session ); 

    
    /* send the packet */
    obexDisconnectReq( session );
}


#ifdef GOEP_VERSION_2_0
/****************************************************************************
 * NAME
 *  ObexL2capConnectRequest
 * 
 * DESCRIPTION
 *  API to establish a OBEX L2CAP client session 
 *
 * PARAMETERS
 *  Refer obex.h
 *
 * RETURNS
 *  The connection context task to which it will notify on session establishment.
 **************************************************************************/
Task ObexL2capConnectRequest( const bdaddr          *addr,
                              uint16                 psm,
                              const ObexConnParams  *connParams )
{
    ObexChannel   channel;
    if( !connParams) return NULL;

    channel.l2capObex = TRUE;
    channel.u.psm = psm;
    return obexConnectRequest( addr, channel, connParams ); 
}

#endif /* GOEP_VERSION_2_0 */

