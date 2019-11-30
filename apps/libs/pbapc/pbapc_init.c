/****************************************************************************
Copyright (c) 2004 - 2016 Qualcomm Technologies International, Ltd.


FILE NAME
    pbapc_init.c
    
DESCRIPTION
    PBAP Client initalization routines including the handling of SDP 
    registration and SDP search.
*/
#include <connection.h>
#include <sdp_parse.h>
#include <stdlib.h>
#include <string.h>

#include "pbapc_extern.h"
#include "pbapc_private.h"

static const uint8 serviceRecordPbapc[] =
    {            
        0x09,0x00,0x01,        /* Service class ID list */
        0x35,0x03,        
        0x19,0x11, 0x2E,    /* UUID = Phonebook Access Client */
                    
        0x09,0x01,0x00,        /* Service name */
        0x25,0x0B,            /* 11 byte string - PBAP Client */
        'P','B','A','P',' ','C','l','i','e','n','t',

    
        0x09,0x00,0x09,        /* profile descriptor list */        
        0x35,0x08,        
        0x35,0x06,            /* 6 bytes in total DataElSeq */
        0x19,0x11,0x30,      /* UUID = OBEXPhonebookAccess */
        0x09,0x01,0x01,        /* 2 byte uint, version = 101 */
    };

/* OBEX PBAP service */
static const uint8 servRequestPbapc[] =
{
    0x35, 0x03, 
    0x19, 0x11, 0x2F /* UUID16, Phone book access Server */
};

/* Static data structures for SDP Attribute Request to PSE */
static const uint8 attrRequestPbapc[] =
{
    0x35,0x06,                 
        0x09, 0x00, 0x04,       /* Protocol Descriptor List */
        0x09, 0x03, 0x14        /* Supported Message Types */
};


/****************************************************************************
 *NAME    
 *  handleSDPRegisterCfm
 *
 *DESCRIPTION
 * Handle the SDP Record register confirmation 
 ***************************************************************************/
static void handleSDPRegisterCfm( pbapcState *state, 
                                  CL_SDP_REGISTER_CFM_T *msg )
{
    PbapcLibStatus status = (msg->status == success)?
                             pbapc_success: pbapc_sdp_failure_bluestack;

    if( state->currCom == pbapc_com_reg_sdp )
    {
        pbapcMsgInitCfm( state->theAppTask, msg->service_handle, status );

        /* Free the state now */
        MessageSend(&state->task,PBAPC_INT_TASK_DELETE, 0);
    }
    else
    {
        PBAPC_DEBUG(("State invalid\n"));    
    }
}



/****************************************************************************
 *NAME    
 *  handleSDPServSrchAttrCfm
 *
 *DESCRIPTION
 * Handle the SDP response and proceed with the connection
 ***************************************************************************/
static void handleSDPServSrchAttrCfm( pbapcState *state, 
                                  CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T *msg)
{
    uint8 chan;
    uint8 *chans = &chan;
    uint8 found;
    uint8 repos;
    uint16 device_id = state->device_id;
    
    if( state->currCom != pbapc_com_connect ) 
    {
        MessageFlushTask((Task)state);
        /* Delete the task which is allocated during connect initiation with remote device */
        MessageSendLater(&state->task,PBAPC_INT_TASK_DELETE, 0, PBAPC_TASK_DELETE_DELAY);        
        return;
    }

    if ( (msg->status==success) &&
          SdpParseGetMultipleRfcommServerChannels(
            msg->size_attributes, msg->attributes,
            1, &chans, &found) )
    {
        SdpParseGetPbapRepos(msg->size_attributes, msg->attributes, &repos);
        state->srvRepos = repos;

        /* Initiate a Connection attempt */
        pbapcGoepConnect( state, &msg->bd_addr, chan ); 
    }
    else
    {
        state->currCom = pbapc_com_none;
        MessageFlushTask((Task)state);
        pbapcMsgSendConnectCfm( state->theAppTask, 
                                NULL,
                                &msg->bd_addr, 
                                pbapc_failure, 0, 0 );
        
        /* No pbapc supported, reset to NULL */
        Pbapc[device_id] = NULL;
        
        /* Delete the task after PBAPC_TASK_DELETE_DELAY, This is to handle messages directed
           to the task with in that time. the only possible message is 
           any continuation responses for this SDP attribute search. */
        MessageSendLater(&state->task,PBAPC_INT_TASK_DELETE, 0, PBAPC_TASK_DELETE_DELAY);
    }
}
/****************************************************************************
 * NAME
 *  pbapcCreateTask
 *
 * DESCRIPTION
 *  Create the PBAP task
 **************************************************************************/
Task pbapcCreateTask( Task theAppTask )
{
    pbapcState *state;

    state = malloc(sizeof(pbapcState));
    if( !state )
        return NULL;

    memset(state, 0, sizeof( pbapcState ) );
    state->task.handler = pbapcInitHandler;
    state->theAppTask = theAppTask;

    return &state->task;
}
/****************************************************************************
 * NAME
 *  pbapcRegisterSdpRecord
 *
 * DESCRIPTION
 *  Register the Client Side SDP Record with Bluestack 
 **************************************************************************/
void pbapcRegisterSdpRecord( pbapcState *state)
{
    ConnectionRegisterServiceRecord( &state->task,
                                     sizeof( serviceRecordPbapc ),
                                     serviceRecordPbapc );

    state->currCom = pbapc_com_reg_sdp;
}

/****************************************************************************
 * NAME
 *  pbapcInitConnection
 *
 * DESCRIPTION
 *  Initiate the PBAP Connection by starting an SDP search first.
 **************************************************************************/
void pbapcInitConnection( pbapcState *state, const bdaddr *bdAddr )
{
    /* search for remote channel */
    ConnectionSdpServiceSearchAttributeRequest(
        &state->task,
        bdAddr,
        30,
        sizeof(servRequestPbapc), servRequestPbapc,
        sizeof(attrRequestPbapc), attrRequestPbapc);

    state->currCom= pbapc_com_connect;
}

/****************************************************************************
 *NAME    
 *  pbapcGetSupportedRepositories
 *
 *DESCRIPTION
 *  Get the supported repositories. reurn 0xFF on error
 ***************************************************************************/
uint8 pbapcGetSupportedRepositories( pbapcState *state )
{
    return (state->handle)?(state->srvRepos): 0;
}

/****************************************************************************
 *NAME    
 *  pbapcGetApptask
 *
 *DESCRIPTION
 *  Get the application task
 ***************************************************************************/
Task pbapcGetAppTask( pbapcState *state )
{
    return state->theAppTask;
}

/****************************************************************************
 *NAME    
 *  pbapcInitHandler
 *
 *DESCRIPTION
 * Initialization Handler for messages received by the PBAPC Task.
 ***************************************************************************/
void  pbapcInitHandler( Task task, MessageId id, Message message)
{
    pbapcState *state = (pbapcState*) task;

    if( id <  PBAPC_INT_ENDOFLIST )
    {
        pbapcIntHandler( task, id, message );
		return;
    }

    switch (id)
    {
        /* Messages from the connection library */
        case CL_SDP_REGISTER_CFM:
            handleSDPRegisterCfm(state, (CL_SDP_REGISTER_CFM_T*)message);
            break;

        case CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM:
            handleSDPServSrchAttrCfm(state, 
                            (CL_SDP_SERVICE_SEARCH_ATTRIBUTE_CFM_T*)message);
            break;
        
        default:
           PBAPC_LOG(("Unhandled Init message : 0x%X\n",id));
           break;
    }
}

