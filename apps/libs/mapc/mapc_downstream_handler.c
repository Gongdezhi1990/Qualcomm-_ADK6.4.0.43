/****************************************************************************
Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd.
 

FILE NAME
    mapc_downstream_handler.c
    
DESCRIPTION
   This file implements the downstream interface with connection library 
   and Obex.
*/

#include <panic.h>
#include <connection.h>
#include <obex.h>
#include "mapc_extern.h"
#include "mapc_private.h"

/* Default Service Record for MNS Service */
static const uint8 serviceRecord[] = 
{
    0x09, 0x00, 0x01,           /* Service Class ID  */
    0x35, 0x03,
        0x19, 0x11, 0x33,       /* Message Notification Server UUID */

    0x09, 0x00, 0x04,           /* Protocol Descriptor List */
    0x35, 0x11,
        0x35, 0x03,
            0x19, 0x01, 0x00,   /* L2CAP UUID */
        0x35, 0x05,
            0x19, 0x00, 0x03,   /* RFCOMM UUID */
            0x08, MAPC_DEFAULT_CHANNEL,
        0x35, 0x03,
            0x19, 0x00, 0x08,   /* OBEX UUID */

    0x09, 0x01, 0x00,           /* Service Name */
    0x25, 0x3,
        'M','N','S',            /* Service Name is MNS */
    
    0x09, 0x00, 0x09,           /* Protocol Descriptor List */
    0x35, 0x08,
        0x35, 0x06,
            0x19, 0x11, 0x34,   /* Message Access Profile UUID */
            0x09, 0x01, 0x01,   /* Version 0x0101 */
};

/* Static data structures for SDP Attribute Request to MSE */
static const uint8 attrRequest[] =
{
    0x35,0x0C,                 
        0x09, 0x00, 0x04,       /* Protocol Descriptor List */
        0x09, 0x01, 0x00,       /* Service Name */
        0x09, 0x03, 0x15,       /* The MAS Instance ID */
        0x09, 0x03, 0x16        /* Supported Message Types */
};

 
/* Static data structures for SDP Search Request */
static const uint8 serviceRequest[] =
{
    0x35, 0x03,                 
        0x19, 0x11, 0x32      /* Message Access Server UUID */
};

/* MAS Target */
static const uint8 masTarget[] = 
{
    0xbb, 0x58, 0x2b, 0x40, 
    0x42, 0x0c, 
    0x11, 0xdb,
    0xb0, 0xde,
    0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

/* MNS Target */
static const uint8 mnsTarget[] = 
{
    0xbb, 0x58, 0x2b, 0x41,
    0x42, 0x0c,
    0x11, 0xdb,
    0xb0, 0xde,
    0x08, 0x00, 0x20, 0x0c, 0x9a, 0x66
};

/*************************************************************************
 * NAME
 *  mapcRegisterChannel
 *
 * DESCRIPTION 
 *  Register the RFCOMM Channel for accepting incoming connections 
 ************************************************************************/
void mapcRegisterChannel( uint8 mnsChannel )
{
   /* If the request is to populate a default SDP record, override
      the application proposed MNS channel with the default one */
    if( mapcGetSdpState() == mapc_sdp_reg_pending )
    {
         mnsChannel =  MAPC_DEFAULT_CHANNEL;
    } 

    /* Register the RFCOMM Channel for MNS Service */
    ConnectionRfcommAllocateChannel( mapcGetProfileTask(), mnsChannel );
}

/**************************************************************************
 * NAME
 *  mapcRegSdpRecord
 *
 * DESCRIPTION
 *  Register an SDP record for the MNS Service
 **************************************************************************/
void mapcRegSdpRecord( void )
{
    ConnectionRegisterServiceRecord( mapcGetProfileTask(), 
                                     sizeof( serviceRecord ),
                                     serviceRecord );

    mapcSetSdpState( mapc_sdp_reg_progress );
}

/***************************************************************************
 * NAME
 *  mapcSdpSearch
 *
 * DESCRIPTION 
 *  Initiate an SDP Service Attribute Search
 *
 * PARAMETERS 
 * theTask      - The task to receive the result
 * addr         - The Remote MSE device
 **************************************************************************/
void mapcSdpSearch( Task  theTask , const bdaddr* addr )
{
    ConnectionSdpServiceSearchAttributeRequest( theTask, 
                                                addr, 
                                                MAPC_MAX_SDP_ATTR_SIZE,
                                                sizeof(serviceRequest),
                                                serviceRequest,
                                                sizeof(attrRequest),
                                                attrRequest );
}

/************************************************************************
 * NAME
 *  mapcIntConnectReq
 *
 * DESCRIPTION 
 * Send a connect request to establish a OBEX session. 
 * 
 ************************************************************************/
void mapcIntConnectReq( MAPC_INT_CONNECT_REQ_T *req )
{
    ObexConnParams params;
    Mapc masTask;

    params.connTaskData.handler = mapcSessionHandler;
    params.sizeConnTask = sizeof(MAPC);
    params.target = masTarget;
    params.sizeTarget = sizeof(masTarget);
    params.auth = FALSE;

    masTask = (Mapc) ObexConnectRequest( (const bdaddr*)&(req->addr),  
                                         req->channel,
                                         &params );

    mapcTaskInitialize( masTask, req->theAppTask, mapc_mas_client );

}

/************************************************************************
 * NAME
 *  mapcIntConnectResp
 *
 * DESCRIPTION 
 * Send a connect response to establish the OBEX connection.
 * 
 ************************************************************************/
void mapcIntConnectResp( MAPC_INT_CONNECT_RESP_T *rsp )
{
    ObexConnParams params;
    Mapc mnsTask;

    params.connTaskData.handler = mapcSessionHandler;
    params.sizeConnTask = sizeof(MAPC);
    params.target = mnsTarget;
    params.sizeTarget = sizeof(mnsTarget);
    params.auth = FALSE;
   
    mnsTask = (Mapc) ObexConnectResponse(  rsp->sink,  
                                           rsp->channel,
                                           rsp->accept,
                                           &params ); 
    if( rsp->accept )
    {
        mapcTaskInitialize( mnsTask, rsp->theAppTask, mapc_mns_server );
    }
    else
    {
        MAKE_MAPC_MESSAGE(MAPC_CONNECT_CFM);
        message->status = mapc_connect_rejected;
        message->addr = rsp->addr;
        message->channel = rsp->channel;
        message->session = 0;
        MessageSend( rsp->theAppTask, MAPC_MNS_CONNECT_CFM, message );
    }
}

/************************************************************************
 * NAME
 *  mapcIntDeleteTask 
 *
 * DESCRIPTION 
 *  Delete the Session
 * 
 ************************************************************************/
void mapcIntDeleteTask( MAPC_INT_TASK_DELETE_T* req )
{
    Mapc mapcTask = req->session;

    if( IsMapcIdle( mapcTask ) )
    {
        ObexDeleteSessionResponse( mapcTask->session );
    }
    else if( IsMapcDisconnected( mapcTask ) ||
             IsMapcDisconnecting( mapcTask ) )
    {                                 
        /* Move it to Disconnected State. The library will delete the 
           session on receiving OBEX_DELETE_SESSION_IND Message from
           OBEX  */
        MAPC_IDLE( mapcTask );
        
    }
    else
    {
        /* Initiate a Disconnect of the Session before Deleting 
           the session */
        MAPC_DISCONNECTING( mapcTask );
        MAPC_RESET_FUNCTION( mapcTask );
        ObexDisconnectRequest( mapcTask->session );

    }
}


