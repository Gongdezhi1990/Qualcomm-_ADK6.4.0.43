/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    obex_profile_handler.c
    
DESCRIPTION
    Internal file for profile handling and session management.
*/

#include <stdlib.h>
#include <string.h>
#include <panic.h>
#include <connection.h>
#include <stream.h>
#include <source.h>
#include "obex_extern.h"
#include "obex_private.h"
#include <stdio.h>


/***************************************************************************
 * NAM
 *  obexHandleRfcommDisconnect
 *
 * DESCRIPTION 
 * Accept RFCOMM Disconnect request and delete the Task.
 *************************************************************************/
static void obexHandleRfcommDisconnect( Obex session )
{
    ConnectionRfcommDisconnectResponse( session->sink );

    if( !IsObexInIdle( session ) )
	{
        SET_OBEX_IN_IDLE( session );
        obexDeleteSessionInd( session );
    }
    else
    {
        SET_OBEX_IN_IDLE( session );
        obexDeleteSessionTask( session );
    }
}

/***************************************************************************
 * NAM
 *  obexDeleteTask
 *
 * DESCRIPTION 
 * Delete the task
 *************************************************************************/
static void obexDeleteTask( Obex session )
{
    /* Delete the Task */
    MessageFlushTask( session->theApp );
    MessageFlushTask( &session->task );
    free( session );    
    
    OBEX_INFO(("Obex free memory\n"));
}

/***************************************************************************
 * NAME
 *  obexDisconnectSession
 *
 * DESCRIPTION 
 * Disconnect RFCOMM or L2CAP connection
 *************************************************************************/
void obexDisconnectSession( Obex session )
{
    if( IsObexL2cap( session ) )
    {
        ConnectionL2capDisconnectRequest( &session->task, session->sink );
    }
    else
    {
        ConnectionRfcommDisconnectRequest( &session->task, session->sink );
    }
    SET_OBEX_IN_IDLE( session );
}

/***************************************************************************
 * NAME
 *  obexConnectionReady
 *
 * DESCRIPTION
 *  RFCOMM connection is ready for data transfer.
 **************************************************************************/
static void obexConnectionReady( Obex session )
{
    if( IsObexClient( session ) && IsObexInConnect( session ))
    {
        /* send OBEX Connect Non authenticated Request */
        obexConnectReq( session, TRUE );
    }
    else
    {
        /* Unblock incoming Data if session is not blocked */
        if(!session->srcUsed) obexSourceEmpty( session );
    }
}


/***************************************************************************
 * NAME
 *  obexHandleRfcConnectCfm
 *
 * DESCRIPTION
 *  Handle the RFCOMM Connection establishment
 **************************************************************************/
static void obexHandleRfcConnectCfm( Obex    session, 
                                     OBEX_SESSION_CFM_T* cfm)
{
    ObexStatus status = obex_failure;

    if( cfm->status == rfcomm_connect_pending ) 
    {
        /* Connection is still Pending */
        return;
    }

    else if( cfm->status ==  rfcomm_connect_success ||
             cfm->status ==  rfcomm_connect_channel_already_open )
    {
        /* Max OBEX packet depends on the payload size. 
           Use the best fit value */
        session->maxPktLen = ( OBEX_MAX_PACKET_SIZE / cfm->payload_size ) *
                               cfm->payload_size;

        session->sink = cfm->sink;
        status = obex_success;
    }

    obexHandleSessionCfm( session, status, &cfm->addr, cfm->server_channel );
}




/***************************************************************************
 * NAME:
 *  obexCreateSession
 *
 * DESCRIPTION
 *  Create a session Task for OBEX and the application 
 *
 * PARAMETRS
 *  apptaskData - The application Task Data
 *  sizeAppTask - Size required for the application Task
 *  role        - Role of OBEX session 
 *  sizeTarget  - Size of the Target
 *  target      - Target header
 **************************************************************************/
static Obex obexCreateSession( TaskData     appTaskData, 
                               uint16       sizeAppTask, 
                               ObexRole     role,
                               uint16       sizeTarget,
                               const uint8* target )
{
    Obex sessionTask;
    uint16 taskSize = sizeof(OBEX) + sizeAppTask;

    /* Create Session Task for OBEX and the application */
    sessionTask = (Obex) PanicUnlessMalloc ( taskSize );
    memset(sessionTask, 0, taskSize );
    
    OBEX_INFO(("Obex create memory\n"));

    sessionTask->task.handler = obexProfileHandler;
    sessionTask->theApp = (Task) ((uint8*)sessionTask + sizeof(OBEX));
    sessionTask->role = role; 
    sessionTask->state = obex_session;
    sessionTask->sizeTargetWho= sizeTarget;
    sessionTask->srcUsed = 0;
    sessionTask->targetWho = target;
    sessionTask->theApp->handler = appTaskData.handler;
    sessionTask->connID = OBEX_INVALID_UINT32;
    sessionTask->maxPktLen = OBEX_MAX_PACKET_SIZE;
    obexResetSrmParams( sessionTask );
    return sessionTask;
}

/**************************************************************************
 * NAME
 *  obexSessionReq
 * 
 * DESCRIPTION
 *  Establish the session 
 *
 * PARAMETERS
 * Obex     - OBEX Session handle
 * addr     - BD ADDR of the remote device
 * channel  - The transport information 
 *
 * RETURNS
 *  The Application task to which it will notifiy on Session Establishment.
 ***************************************************************************/
static Task obexSessionReq( Obex sessionTask,
                            const bdaddr* addr,
                            ObexChannel trans)
{
    uint16 channel = obexGetChannel( trans );

    if( IsObexL2capChannel( trans ) )
    {
        obexL2capConnectReq( sessionTask, addr, channel );
    }
    else
    {
        /* Create the RFCOMM Connect Request */    
        ConnectionRfcommConnectRequest( &sessionTask->task, 
                                        addr,
                                        (uint8)channel,
                                        (uint8)channel,
                                        OBEX_MAX_RFC_FRAME_SIZE);
    }

    return sessionTask->theApp;
}

/**************************************************************************
 * NAME
 *  obexSessionResp
 * 
 * DESCRIPTION
 *  Response to a session request.
 *
 * PARAMETERS
 * Obex     - OBEX Session handle
 * accept   - TRUE to accept the connection
 * auth     - OBEX authentication is required
 * sink     - Associated sink value
 * channel  - RFCOMM channel.
 *
 * RETURNS
 *  The Application task to which it will notifiy on Session Establishment.
 ***************************************************************************/
static Task obexSessionResp(Obex sessionTask, 
                            bool accept,
                            bool auth,
                            ObexConnId id,
                            ObexChannel trans )
{
    Task task = NULL;
    uint16 channel = obexGetChannel( trans );

    if( accept )
    {
        /* Session Task must be valid */
        task = &sessionTask->task;
        sessionTask->sink = id.u.sink;
        sessionTask->channel = channel;
        sessionTask->auth = auth;
        /* Set unique session ID for each session */
        if( sessionTask->targetWho ) sessionTask->connID =  (context_t) task;
    }   
   
    if( IsObexL2capChannel( trans ) )
    {
        obexL2capConnectResp( sessionTask, accept, channel, id );
    }
    else
    {
        /* Create the RFCOMM Connect Response */
        ConnectionRfcommConnectResponse( task, 
                                         accept, 
                                         id.u.sink, 
                                         (uint8)channel, 
                                         OBEX_MAX_RFC_FRAME_SIZE );
    }

    return (sessionTask)? sessionTask->theApp: NULL;
}

/***************************************************************************
 * NAME
 *  obexHandleSessionCfm
 *
 * DESCRIPTION
 *  Handle the RFCOMM Connection establishment
 **************************************************************************/
void obexHandleSessionCfm( Obex       session,
                           ObexStatus status,
                           const bdaddr* addr,
                           uint16  channel )
{
    
    session->channel = channel;

    /* Despatch Create Session Confirmation Message */
    obexCreateSessionCfm( session, status, addr );

    if( status == obex_success )
    {
        SET_OBEX_IN_CONNECT( session );
        
        obexConnectionReady( session );
    }
    else
    {
        SET_OBEX_IN_IDLE( session );

        /* Send a failure Connect Confirmation Message */
        obexConnectCfm( session, obex_failure );

        /* Session is not connected. Delete the Task */
        obexDeleteSessionTask( session );
    }
}

/****************************************************************************
 * NAME
 *  obexConnectRequest
 * 
 * DESCRIPTION
 * Request  establish a OBEX client session 
 *
 * PARAMETERS
 *  addr    - Remote address
 *  channel  - Obex transport
 *  connParams - Connection parameters
 *
 * RETURNS
 *  The connection context task to which it will notify on session establishment.
 **************************************************************************/
Task obexConnectRequest( const bdaddr          *addr,
                         ObexChannel           channel,
                        const ObexConnParams  *connParams )
{
    Obex sessionTask;
    ObexRole role = obexGetClientRole(channel);

    /* Create a OBEX session Task */
    sessionTask = obexCreateSession( connParams->connTaskData,
                                     connParams->sizeConnTask,
                                     role,
                                     connParams->sizeTarget,
                                     connParams->target );

    return obexSessionReq( sessionTask, addr, channel );
}

/****************************************************************************
 * NAME
 *  obexConnectResponse
 * 
 * DESCRIPTION
 * Response for establishisg a OBEX server session 
 *
 * PARAMETERS
 *  connId  - Connection Id
 *  channel  - Obex transport
 *  accept  - Accept connection
 *  connParams - Connection parameters
 *
 * RETURNS
 *  The connection context task to which it will notify on session establishment.
 **************************************************************************/
Task obexConnectResponse( ObexConnId            connId,
                          ObexChannel           channel,
                          bool                  accept,
                          const ObexConnParams* connParams) 
{
    Obex sessionTask;
    ObexRole role = obexGetServerRole(channel);

    if( accept && connParams )
    {    
        /* Create OBEX session Task */
        sessionTask = obexCreateSession( connParams->connTaskData,
                                         connParams->sizeConnTask,
                                         role,
                                         connParams->sizeTarget,
                                         connParams->target );

        return obexSessionResp( sessionTask, TRUE, 
                                connParams->auth, connId,
                                channel );
    }
    else
    {
        return obexSessionResp( NULL, FALSE, FALSE, connId, channel );
    } 

}



/***************************************************************************
 * NAM
 *  ObexDeleteSessionTask
 *
 * DESCRIPTION 
 *  Delete the session task.
 *************************************************************************/
void obexDeleteSessionTask( Obex session )
{
    if( IsObexInIdle( session ))
    {
        /* Delete the after 50 ms */
        MessageSendLater( &session->task, OBEX_MESSAGE_DELETE_TASK, 0, 500);  
    }
    else if( ( IsObexServer( session) ) &&
             ( IsObexDisconnected( session ) ) )
    {
        /* waiting for remote RFCOMM disconnection */ 
        SET_OBEX_IN_IDLE( session );
    }
    else
    {
        /* Disconnect RFCOMM channel */
        obexDisconnectSession( session );
    }
}

/***********************************************************************
 * NAME
 *  obexAuthenticateSession
 *
 * DESCRIPTION 
 *  Authenticate the OBEX connect packet - OBEX authentication is not 
 *  supported in this version. This function is just a place holder.
 *
 * RETURN
 *  TRUE on success and FALSE on failure
 ***********************************************************************/
bool obexAuthenticateSession( Obex session, const uint8* pkt, uint16 *len)
{
    const uint8* digest;
    uint16 pktLen = 0;


    /*
     *  session->auth   State   outcome
     *  -------------   -----   ------
     *  FALSE        Connect     No Authentication required.
     *  TRUE         Connect     Expecting challenge from remote and the 
     *                           local device to send its challenge.
     *  FALSE        AutoConnect Expecting Response from the remote.
     *  TRUE         AuthConnect Expected Challenge and Response from remote.
     *  
     */                                        
    if( IsObexInConnect( session ) && !session->auth )
    {
        *len = 0;
        return TRUE;
    }

    if( session->auth )
    {
        pktLen = *len;
        /*  Request the application to initiate the authentication */ 
          
        if( IsObexInConnect( session ) ) obexAuthReqInd( session );

        digest = obexGetDigest( pkt, &pktLen, OBEX_AUTH_CHALLENGE );
        if(!digest) return FALSE;

        obexAuthClgInd( session, digest, pktLen ); 
        
        /* Calculate the unprocessed length */
        pktLen = *len - (digest - pkt);
    }
     
    if( IsObexInAuthConnect( session ) )
    {
        /* Local device sent the Challenge  and waiting for the Response */
        digest =  obexGetDigest( pkt, len, OBEX_AUTH_RESPONSE );
        if( !digest ) return FALSE;

        /* Send a OBEX_AUTH_RSP_CFM message to the application */
        obexAuthRspCfm( session, digest, *len );

    }

    *len = pktLen;
    return TRUE;
}

/***************************************************************************
 * NAME
 *  obexValidateSession 
 *
 * DESCRIPTION
 *  Validate the session by comparing the Target/Who header 
 *************************************************************************/
bool obexValidateSession( Obex session, const uint8* pkt, uint16 len )
{
    uint8* target;
    uint16 opcode;

    opcode = ( IsObexClient( session ))? OBEX_WHO_HDR: OBEX_TARGET_HDR;

    target =  obexGetSeqHeader( pkt, &len, opcode );

    if( (len == session->sizeTargetWho) && target &&
        (memcmp(target, session->targetWho, session->sizeTargetWho) == 0) )
    {
        return TRUE;
    }
    return FALSE;
}

/**************************************************************************
 * NAME
 *  obexHandler
 *
 * DESCRIPTION
 *  handler function for obex session.
 *************************************************************************/
void obexProfileHandler( Task task, MessageId id, Message message )
{
    Obex  session = (OBEX*) task;

    switch( id )
    {
        case OBEX_MESSAGE_MORE_DATA:
        case MESSAGE_MORE_DATA:     /* Fall through */
            obexHandleIncomingPacket( session );
            break;

        case MESSAGE_MORE_SPACE:
            obexHandleSrmSpace( session );
            break;

        case OBEX_MESSAGE_DELETE_TASK:
            obexDeleteTask( session );
            break;

        case CL_RFCOMM_CLIENT_CONNECT_CFM:
        case CL_RFCOMM_SERVER_CONNECT_CFM:
            obexHandleRfcConnectCfm( session ,
                                    (OBEX_SESSION_CFM_T*) message);
            break;

        case CL_RFCOMM_DISCONNECT_IND: 
            obexHandleRfcommDisconnect( session );
            break;
    
        case CL_L2CAP_DISCONNECT_IND:
            obexHandleL2capDisconnect( session, 
                    ((CL_L2CAP_DISCONNECT_IND_T*)message)->identifier );
            break; 

        case CL_RFCOMM_DISCONNECT_CFM: 
            if( IsObexInIdle( session ) ) obexDeleteSessionTask( session );
            break;

        case CL_RFCOMM_PORTNEG_IND:
        {
            CL_RFCOMM_PORTNEG_IND_T* msg = (CL_RFCOMM_PORTNEG_IND_T*)message;
            ConnectionRfcommPortNegResponse( task,
                                             msg->sink, 
                                             &msg->port_params);
            break;
        }

        case CL_RFCOMM_CONTROL_IND:
            if( IsObexReady (session ) || IsObexInConnect( session) )
            {
                /* Remote renegotiating the MODEM parameters */
                ConnectionRfcommControlSignalRequest( &session->task,       
                                                       session->sink,
                                                        0x00, 0x8C ); 
            }
            break;

        case CL_RFCOMM_CONTROL_CFM:
            break;

        case CL_L2CAP_CONNECT_CFM:
            obexHandleL2capConnectCfm( session, message );
            break;
                                      
        default:
           OBEX_INFO(("Unhandled - %x\n", id));
           break; 
    }

}







