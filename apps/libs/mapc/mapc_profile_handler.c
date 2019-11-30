/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    mapc_profile_handler.c

DESCRIPTION
   This file implements the Task Management and Task handling routines.
*/

#include <panic.h>
#include <sink.h>
#include <stream.h>
#include <string.h>
#include <connection.h>
#include <obex.h>
#include "mapc_private.h"
#include "mapc_extern.h"

/* Mapc library Task for handling incoming connections */
const TaskData gMapcTaskData = { mapcProfileHandler };

/* Global to store the application Task for incoming MNS connections */
static MapcLibState gMnsState = { NULL, mapc_sdp_reg_none };

#define OBEX_REGISTER_CFM_T CL_RFCOMM_REGISTER_CFM_T

/***************************************************************************
 * NAME
 *  mapcSdpRegisterCfm
 *
 * DESCRIPTION
 *  Handle the result of SDP Record registration
 **************************************************************************/
static void  mapcSdpRegisterCfm( CL_SDP_REGISTER_CFM_T* sdpCfm )
{
    MapcStatus status;

    status = ( sdpCfm->status == success )? mapc_success : mapc_failure;

    if(sdpCfm->status == success)
    {
        /* Set SDP status */
        mapcSetSdpState( mapc_sdp_registered );
    }
    else
    {
        /* Deallocate the L2CAP Channel */
        ConnectionRfcommDeallocateChannel( mapcGetProfileTask() ,
                                           MAPC_DEFAULT_CHANNEL );

    }

    /* Despatch  Cfm message to the app */
    mapcMnsStartCfm( status, MAPC_DEFAULT_CHANNEL, sdpCfm->service_handle );
}


/***************************************************************************
 * NAME
 *  mapcObexRegisterCfm
 *
 * DESCRIPTION
 *   Handle the incoming OBEX registration confirmation message
 **************************************************************************/
static void mapcObexRegisterCfm( OBEX_REGISTER_CFM_T* regCfm )
{
    MapcStatus  status;

    status = ( regCfm->status == success )? mapc_success : mapc_failure;

    /* Start SDP record registration on Success */
    if( (mapcGetSdpState() == mapc_sdp_reg_pending) &&
        (status == success) )
    {
        mapcRegSdpRecord();
    }
    else
    {
        mapcMnsStartCfm( status, regCfm->server_channel, 0 );
    }
}

/*************************************************************************
 * NAME
 * mapcHandleSessionCfm
 *
 * DESCRIPTION
 *   Handle the MAS or MNS Session  connection
 *************************************************************************/
static void mapcHandleSessionCfm( Mapc      mapcTask,
                                  OBEX_CREATE_SESSION_CFM_T* cfm )
{
    MapcStatus status;

    if( cfm->status ==  obex_success )
    {
        status = mapc_pending;
        MAPC_CONNECT_PENDING( mapcTask );
        mapcTask->session = cfm->session;

    }
    else
    {
        status = mapc_failure;
        MAPC_IDLE( mapcTask );

    }

    /* Send Connect Cfm to the application */
    mapcConnectCfm( mapcTask,
                   status,
                   &cfm->remoteAddr,
                   cfm->channel);
}


/*************************************************************************
 * NAME
 *  mapcUnregisterMnsApp
 *
 * DESCRIPTION
 *  Unegister the MNS Service
 *
 * PARAMETERS
 *  recordHandle - The SDP Record Handle
 *  mnsChannel  - The mns Channel
 **************************************************************************/
void mapcUnregisterMnsApp( uint32 recordHandle, uint8 mnsChannel )
{
    /* Unregister the SDP record */
    gMnsState.eSdpState =  mapc_sdp_reg_none;
    ConnectionUnregisterServiceRecord( mapcGetProfileTask(), recordHandle );

    /* Deallocate the RFCOMM Server Channel */
    ConnectionRfcommDeallocateChannel( mapcGetProfileTask(), mnsChannel );
}


/*************************************************************************
 * NAME
 *  mapcRegisterMnsApp
 *
 * DESCRIPTION
 *  Register the Client Application for notifying the incoming MNS connection
 *  requests.
 *
 * PARAMETERS
 *  theAppTask - The Application Task
 *  popSdpRec  - Populate SDP Record
 *  mnsChannel - Server Channel
 ************************************************************************/
bool mapcRegisterMnsApp( Task theAppTask , bool popSdpRec , uint8 mnsChannel )
{
    UNUSED(popSdpRec);

    /* Return failure if MNS has already Registered */
    if(gMnsState.regApp) return FALSE;

    /* Register the application Task and the MNS channel*/
    gMnsState.regApp = theAppTask;
    gMnsState.eSdpState =  mapc_sdp_reg_pending;
    mapcRegisterChannel( mnsChannel );

    return TRUE;
}

/***************************************************************************
 * NAME
 *  mapcUnRegMnsApp
 *
 * DESCRIPTION
 *  Unregister the application
 **************************************************************************/
void mapcUnregMnsApp( void )
{
    gMnsState.regApp = NULL;
    gMnsState.eSdpState =  mapc_sdp_reg_none;

}

/***************************************************************************
 * NAME
 *  mapcGetprofileTask
 *
 * DESCRIPTION
 *  Return the profile Task. This is defined as a function instead of a
 *  Macro to ensure  the encapsulation of globals.
 **************************************************************************/
Task mapcGetProfileTask( void )
{
    return (Task)&gMapcTaskData;
}

/***************************************************************************
 * NAME
 *  mapcGetRegAppTask
 *
 * DESCRIPTION
 *  Return the registered application Task. This is defined as a function
 *  instead of a Macro for the encapsulation of globals.
 **************************************************************************/
Task mapcGetRegAppTask( void )
{
    return gMnsState.regApp;
}

/***************************************************************************
 * NAME
 *  mapcGetSdpState
 *
 * DESCRIPTION
 *  Return the SDP state. This is defined as a function
 *  instead of a Macro for the encapsulation of globals.
 **************************************************************************/
MapcState mapcGetSdpState( void )
{
    return gMnsState.eSdpState;
}

/***************************************************************************
 * NAME
 *  mapcSetSdpState
 *
 * DESCRIPTION
 *  Set the SDP state. This is defined as a function
 *  instead of a Macro for the encapsulation of globals.
 **************************************************************************/
void mapcSetSdpState( MapcState state )
{
    gMnsState.eSdpState = state;
}

/**************************************************************************
 * NAME
 *  mapcTaskInitialize
 *
 * DESCRIPTION
 *  Initialize the Mapc Session Task
 *************************************************************************/
void mapcTaskInitialize( Mapc mapcTask, Task appTask, MapcRole role)
{
    mapcTask->appTask = appTask;
    mapcTask->session = NULL;
    mapcTask->role = role;
    MAPC_DISCONNECTED( mapcTask );
    mapcTask->state = mapc_session_progress;
    mapcTask->function = mapc_no_function;
}


/*************************************************************************
 * NAME
 * mapcHandleDeleteSessionInd
 *
 * DESCRIPTION
 *   Handle the MAS or MNS Delete session Indication message from OBEX
 *************************************************************************/
void mapcHandleDeleteSessionInd( Mapc mapcTask,
                                 OBEX_DELETE_SESSION_IND_T *ind )
{
    UNUSED(ind);

    if( IsMapcIdle( mapcTask ) )
    {
       /* Respond immediately and no more access to this session */
        ObexDeleteSessionResponse( mapcTask->session );
    }
    else
    {
        /*  RFCOMM Channel got dropped */
        MAPC_IDLE( mapcTask );
        mapcDisconnectInd( mapcTask);
    }
}


/*************************************************************************
 * NAME
 * mapcHandleConnectCfm
 *
 * DESCRIPTION
 *   Handle the MAS or MNS Connect Complete Confirmation
 *************************************************************************/
void mapcHandleConnectCfm( Mapc mapcTask, OBEX_CONNECT_CFM_T* cfm )
{
    MapcStatus status;
    tp_bdaddr tpaddr;


    /* Handle this only if the connection is pending */
    if( !IsMapcConnecting( mapcTask) )
    {
        /* Just ignore */
        return;
    }

    if( (cfm->session != mapcTask->session ) )
    {
        MAPC_ERR(("Task corrupted\n"));
    }


    if( cfm->status == obex_remote_success )
    {
        status = mapc_success;
        mapcTask->maxPktLen = cfm->maxPacketLen;
        MAPC_CONNECTED( mapcTask );
    }
    else
    {
        status = mapc_failure;
        MAPC_IDLE( mapcTask ); /* Waiting for OBEX_DELETE_SESSION_IND */
        mapcTask->session = 0;
    }

    SinkGetBdAddr( ObexGetSink( mapcTask->session ), &tpaddr );

    /* Send Connect Cfm to the application */
    mapcConnectCfm( mapcTask,
                    status,
                    &tpaddr.taddr.addr,
                    ObexGetChannel( mapcTask->session));
}


/**************************************************************************
 * NAME:
 *  mapcProfileHandler
 *
 * DESCRIPTION
 *  The Map main task handler function. This is used only before each session
 *  creation.
 ************************************************************************/
void mapcProfileHandler( Task task, MessageId id, Message message )
{
    UNUSED(task);

    switch( id )
    {

        case CL_SDP_REGISTER_CFM:
            mapcSdpRegisterCfm( (CL_SDP_REGISTER_CFM_T*) message );
            break;

        case CL_SDP_UNREGISTER_CFM:
            /* Just ignore */
            break;

        case CL_RFCOMM_UNREGISTER_CFM:
            mapcMnsShutdownCfm(((
                  (CL_L2CAP_UNREGISTER_CFM_T*)message)->status == success )?
                   mapc_success: mapc_failure );
            break;

        case CL_RFCOMM_REGISTER_CFM:
            mapcObexRegisterCfm( (CL_RFCOMM_REGISTER_CFM_T*) message );
            break;

        case OBEX_CONNECT_IND:
            mapcMnsConnectInd( (OBEX_CONNECT_IND_T*) message );
            break;

        case MAPC_INT_CONNECT_REQ:
            mapcIntConnectReq( (MAPC_INT_CONNECT_REQ_T*) message );
            break;

        case MAPC_INT_CONNECT_RESP:
            mapcIntConnectResp( (MAPC_INT_CONNECT_RESP_T*) message );
            break;

        case MAPC_INT_TASK_DELETE:
            mapcIntDeleteTask( (MAPC_INT_TASK_DELETE_T*) message );
            break;

        case MAPC_INT_SET_NOTIFICATION:
            mapcMasIntSetNotification( (MAPC_INT_SET_NOTIFICATION_T*)message );
            break;

        case MAPC_INT_GET_FOLDER:
            mapcMasIntGetFolder( (MAPC_INT_GET_FOLDER_T*) message );
            break;

        case MAPC_INT_GET_MSG_LIST:
            mapcMasIntGetMsgList( (MAPC_INT_GET_MSG_LIST_T*) message );
            break;

        case MAPC_INT_GET_MSG:
            mapcMasIntGetMsg( (MAPC_INT_GET_MSG_T*) message );
            break;

        case MAPC_INT_SEND_EVENT_RESP:
            mapcMnsIntSendEventResp( (MAPC_INT_SEND_EVENT_RESP_T*) message);
            break;

        case MAPC_INT_SET_FOLDER:
            mapcMasIntSetFolder( (MAPC_INT_SET_FOLDER_T*) message );
            break;

        case MAPC_INT_GET_CONTINUE:
            mapcMasIntGetContinue( (MAPC_INT_GET_CONTINUE_T*) message );
            break;

        case MAPC_INT_UPDATE_INBOX:
            mapcMasIntUpdateInbox( (MAPC_INT_UPDATE_INBOX_T*) message );
            break;

        case MAPC_INT_SET_MSG_STATUS:
            mapcMasIntSetMessage( (MAPC_INT_SET_MSG_STATUS_T*) message );
            break;

        case MAPC_INT_PUT_MSG:
            mapcMasIntPutMessage( (MAPC_INT_PUT_MSG_T*) message );
            break;

        case MAPC_INT_PUT_CONTINUE:
            mapcMasIntPutContinue((MAPC_INT_PUT_CONTINUE_T*) message);
            break;

        default:
            /* unexpected */
            MAPC_ERR(("Unexpected %d\n", id));
            break;

    }


}

/**************************************************************************
 * NAME:
 *  mapcSessionHandler
 *
 * DESCRIPTION
 *  The Map session handler function
 ************************************************************************/
void mapcSessionHandler(Task task, MessageId id, Message message)
{
    Mapc session = (MAPC*) task;

    switch( id )
    {
        case OBEX_CREATE_SESSION_CFM:
            mapcHandleSessionCfm( session,
                      (OBEX_CREATE_SESSION_CFM_T*) message );
            break;


        case OBEX_CONNECT_CFM:
            mapcHandleConnectCfm( session,
                                  ( OBEX_CONNECT_CFM_T* ) message );
            break;

        case OBEX_DELETE_SESSION_IND:
            mapcHandleDeleteSessionInd( session,
                              ( OBEX_DELETE_SESSION_IND_T* )message );
            break;

        case OBEX_PUT_CFM: /* MAS Message */
        case OBEX_GET_CFM: /* MAS Message */
            mapcMasHandleGetPutCfm( session, (OBEX_GETPUT_CFM_T*) message );
            break;

        case OBEX_SET_PATH_CFM:
            mapcMasHandleSetPathCfm( session, (OBEX_SET_PATH_CFM_T*) message);
            break;

        case OBEX_PUT_IND: /* MNS Message */
            mapcMnsHandlePutInd( session, (OBEX_PUT_IND_T*) message );
            break;

        case MESSAGE_SOURCE_EMPTY:
        case MESSAGE_MORE_SPACE:
            break;

        default:
            /* unexpected */
            MAPC_ERR(("Unexpected %d\n", id));
            break;
    }

}

