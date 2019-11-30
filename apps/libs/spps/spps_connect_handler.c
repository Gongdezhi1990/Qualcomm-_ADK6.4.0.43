/* Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd. */
/*  

FILE NAME
	spps_connect_handler.c        

DESCRIPTION
	Functions for accepting an SPP server connection (device B, acceptor).

*/

#include <stdlib.h>
#include "spps_private.h"

/* SPP server task data */

struct __SPP 
{
    SPP_COMMON      c;
    uint8           server_channel;
};


#define sppsSendConnectCfm(spp,status) sppSendConnectCfm(spp,SPPS_SERVER_CONNECT_CFM,status)

/*****************************************************************************/

static void sppsHandleRfcommServerConnectCfm(SPP *spp, const CL_RFCOMM_SERVER_CONNECT_CFM_T *cfm)
{
#ifdef SPP_DEBUG_LIB
    if (sppGetServiceHandle() == 0)
        SPP_DEBUG(("Managed to get a connection with out an SDP service record!\n"));
#endif

    if (cfm->status == rfcomm_connect_success)
    {
        spp->c.sink = cfm->sink;
        spp->c.max_payload_size = cfm->payload_size;

        /* Unregister the SDP Service Record so that no other SPP connection can 
         * be made to this device 
         */
        ConnectionUnregisterServiceRecord(
            &spp->c.task,
            sppGetServiceHandle()
            );
        
        SPP_STATE(sppConnected);
    }
    sppsSendConnectCfm(spp, sppConvertRfcommConnectStatus(cfm->status));
}

/*****************************************************************************/

static void sppsHandleSdpUnregisterCfm(SPP *spp, const CL_SDP_UNREGISTER_CFM_T *cfm)
{
    UNUSED(spp);
    if (cfm->status == sds_status_success)
    {
        /* Set the service handle to 0 */
        sppStoreServiceHandle(0);
    }
    
#ifdef SPP_DEBUG_LIB
    else if (cfm->status != sds_status_pending)
        SPP_DEBUG(("Unexpected SDP Unregister status: %d\n", cfm->status));
#endif 
}

/*****************************************************************************/

static void sppsHandleSdpRegisterCfm(SPP *spp, const CL_SDP_REGISTER_CFM_T *cfm)
{
    if (cfm->status == sds_status_success)
    {
        sppStoreServiceHandle(cfm->service_handle);
        /* Once the SDP Service record is restored, the server task can be 
         * deleted.
         */
        MessageSend(&spp->c.task, SPP_INTERNAL_TASK_DELETE_REQ, 0);
    }
    
#ifdef SPP_DEBUG_LIB
    else if (cfm->status != sds_status_pending)
        SPP_DEBUG(("Unexpected SDP Unregister status: %d\n", cfm->status));
#endif 
}


/*****************************************************************************/

static void sppsConnectionHandler(Task task, MessageId id, Message message)
{
   	SPP* spp = (SPP*) task;

    switch(spp->c.state)
    {
    case sppConnecting:
        switch(id)
        {
        case CL_RFCOMM_SERVER_CONNECT_CFM:
            sppsHandleRfcommServerConnectCfm(
                spp,
                (CL_RFCOMM_SERVER_CONNECT_CFM_T *)message
                );
            break;
        
        case CL_RFCOMM_CONTROL_IND:
            sppForwardRfcommControlInd(spp, (CL_RFCOMM_CONTROL_IND_T*)message);
            break;

        case CL_RFCOMM_PORTNEG_IND:
            sppHandleRfcommPortnegInd(spp, (CL_RFCOMM_PORTNEG_IND_T*)message);
            break;
            
        default:
            sppHandleUnexpectedMsg(spp->c.state, id);
            break;
        }
        break;

    case sppConnected:
        switch(id)
        {
        case CL_SDP_UNREGISTER_CFM: 
                sppsHandleSdpUnregisterCfm(
                    spp, 
                    (CL_SDP_UNREGISTER_CFM_T *)message
                    );
                break;
            
        case CL_RFCOMM_DISCONNECT_IND:
            sppHandleRfcommDisconnectInd(
                spp,
                (CL_RFCOMM_DISCONNECT_IND_T *)message
                );
            break;
            
        case CL_RFCOMM_CONTROL_IND:
            sppForwardRfcommControlInd(spp, (CL_RFCOMM_CONTROL_IND_T*)message);
            break;
        
        case CL_SM_ENCRYPTION_CHANGE_IND:
            sppForwardSmEncryptionChangeInd(
                spp,
                (CL_SM_ENCRYPTION_CHANGE_IND_T *)message
                );
            break;

        case CL_RFCOMM_PORTNEG_IND:
            sppHandleRfcommPortnegInd(spp, (CL_RFCOMM_PORTNEG_IND_T*)message);
            break;

        case MESSAGE_MORE_DATA:
            sppForwardMessageMoreData(spp, (MessageMoreData *)message);
            break;
            
        case MESSAGE_MORE_SPACE:
            sppForwardMessageMoreSpace(spp, (MessageMoreSpace *)message);
            break;


        /* Ignored messages */
        case MESSAGE_STREAM_DISCONNECT:
        case MESSAGE_SOURCE_EMPTY:
            break;

        default:
            sppHandleUnexpectedMsg(spp->c.state, id);
            break;
        }
        break;

    case sppDisconnecting:
        switch(id)
        {
        case CL_RFCOMM_DISCONNECT_CFM:
            sppHandleRfcommDisconnectCfm(
                spp, 
                (CL_RFCOMM_DISCONNECT_CFM_T *)message
                );
            break;

        case SPP_INTERNAL_DISCONNECT_REQ:
            /* If the Service record is unregistered */
            if (!sppGetServiceHandle())
                /* Restore the SPP service record again. */
                ConnectionRegisterServiceRecord(
                    &spp->c.task,
                    SSP_SERVICE_RECORD_SIZE,
                    (const uint8 *)&spp_service_record
                );
            else
                /* Otherwise go straight to deleting the task. */
                MessageSend(&spp->c.task, SPP_INTERNAL_TASK_DELETE_REQ, 0);
        
            break;

       case CL_SDP_REGISTER_CFM: 
            sppsHandleSdpRegisterCfm(spp,(CL_SDP_REGISTER_CFM_T *)message);
            break;
            
        case SPP_INTERNAL_TASK_DELETE_REQ:
            sppDeleteTask(spp);
            break;    

        /* Ignored messages */
        case MESSAGE_STREAM_DISCONNECT:
        case MESSAGE_SOURCE_EMPTY:
            break;

        default:
            sppHandleUnexpectedMsg(spp->c.state, id);
            break;
        }
        break;

    default:
        SPP_DEBUG(("Unexpected SPP Server state %d\n", spp->c.state));
        break;
    }
}

/*****************************************************************************/

void SppConnectResponse(Task theAppTask, const bdaddr *bd_addr, const bool response, const Sink sink, const uint8 local_server_channel, const uint16 max_payload_size)
{
    /* Create the Sppc task */
    SPP *spp = PanicUnlessNew(SPP);

    rfcomm_config_params rfcomm_config = 
        {
        RFCOMM_DEFAULT_PAYLOAD_SIZE,
        RFCOMM_DEFAULT_MODEM_SIGNAL,
        RFCOMM_DEFAULT_BREAK_SIGNAL,
        RFCOMM_DEFAULT_MSC_TIMEOUT
        };

    SPP_PRINT(("SPP Server Task Created.\n"));
    
    spp->c.task.handler= sppsConnectionHandler;
    spp->c.client_task = theAppTask;
    spp->c.sink = sink;
    spp->c.bd_addr = *bd_addr;
    spp->c.max_payload_size = max_payload_size;
    spp->c.state = sppConnecting;
    spp->server_channel = local_server_channel;

    /* If the response is negative, then the CL_RFCOM_SERVER_CONNECT_CFM will 
     * indicate that the connection was unsuccessful and put the SPP Server 
     * into the disconnecting state to tidy up
     */
    if (spp->c.max_payload_size)
        rfcomm_config.max_payload_size = spp->c.max_payload_size;

    ConnectionRfcommConnectResponse(
        &spp->c.task,
        response,
        sink,
        local_server_channel,
        &rfcomm_config
        );
}


