/* Copyright (c) 2010 - 2015 Qualcomm Technologies International, Ltd. */
/*  

FILE NAME
	sppc_service_handler.c        

DESCRIPTION
	Functions used for starting up and also stopping an SPP Server.

*/

#include "spps_private.h"

void sppServiceHandler(Task task, MessageId id, Message message);

const TaskData sppsServiceTask = { sppServiceHandler };

static Task sppsClientTask = 0;


/*****************************************************************************/

static void sendSppStartServiceCfm(spp_start_status status)
{
    if (sppsClientTask)
    {
        MAKE_SPP_MESSAGE(SPP_START_SERVICE_CFM);
        message->status = status;
        MessageSend(sppsClientTask, SPP_START_SERVICE_CFM, message);
    }
#if SPP_DEBUG_LIB
    else
        SPP_DEBUG(("sppsClientTask is NULL!\n"));
#endif
}

/*****************************************************************************/

static void sppHandleRfcommRegisterCfm(const CL_RFCOMM_REGISTER_CFM_T *cfm)
{
    
#if SPP_DEBUG_LIB
    if (!sppsClientTask)
        SPP_DEBUG(("sppsClientTask is NULL!\n"));
#endif

    if (cfm->status != success)
    {
        sendSppStartServiceCfm(spp_start_rfc_chan_fail);
        sppsClientTask = 0;
    }
    else if (cfm->server_channel != SPP_DEFAULT_CHANNEL)
    {
        sendSppStartServiceCfm(spp_start_rfc_default_fail);
        sppsClientTask = 0;
    }
    else
    {
        ConnectionRegisterServiceRecord(
            (Task)&sppsServiceTask,
            sizeof(spp_service_record),
            (const uint8 *)&spp_service_record
            );
    }  
}

/*****************************************************************************/

static void sppHandleSdpRegisterCfm(CL_SDP_REGISTER_CFM_T *cfm)
{
    if (cfm->status == sds_status_success)
    {
        sendSppStartServiceCfm(spp_start_success);
        sppStoreServiceHandle(cfm->service_handle);
    }
    else if (cfm->status != sds_status_pending)
    {
        sendSppStartServiceCfm(spp_start_sdp_reg_fail);
        sppsClientTask = 0;
    }
}

/*****************************************************************************/

static void sppHandleRfcommConnectInd( CL_RFCOMM_CONNECT_IND_T *ind )
{
    MAKE_SPP_MESSAGE(SPP_CONNECT_IND);

#if SPP_DEBUG_LIB
    if (!sppsClientTask)
        SPP_DEBUG(("sppsClientTask is NULL!\n"));
#endif

    message->addr = ind->bd_addr;
    message->server_channel = ind->server_channel;
    message->sink = ind->sink;
    MessageSend(sppsClientTask, SPP_CONNECT_IND, message);
}

/*****************************************************************************/

static void sendSppStopServiceCfm(Task theClientTask, spp_stop_status status)
{
    if (theClientTask)
    {
        MAKE_SPP_MESSAGE(SPP_STOP_SERVICE_CFM);
        message->status = status;
        MessageSend(theClientTask, SPP_STOP_SERVICE_CFM, message);
    }
#if SPP_DEBUG_LIB
    else
        SPP_DEBUG(("theClientTask is NULL!\n"));
#endif
}

/*****************************************************************************/

static void sppHandleSdpUnregisterCfm(CL_SDP_UNREGISTER_CFM_T *cfm)
{
    if (cfm->status == sds_status_success)
    {
        /* Set the service handle to 0 now */
        sppStoreServiceHandle(0);

        /* Unallocate the RFCOMM channel for the SPP service. */
        ConnectionRfcommDeallocateChannel(
            (Task)&sppsServiceTask, 
            SPP_DEFAULT_CHANNEL
            );
    }
    else if (cfm->status != sds_status_pending)
    {
        sendSppStopServiceCfm(sppsClientTask, spp_stop_sdp_unreg_fail);
    }
}

/*****************************************************************************/

static void sppHandleRfcommUnregisterCfm(CL_RFCOMM_UNREGISTER_CFM_T *cfm)
{
    if (cfm->status == success)
    {
        sendSppStopServiceCfm(sppsClientTask, spp_stop_success);
        sppsClientTask = 0;
    }
    else
    {
        sendSppStopServiceCfm(sppsClientTask, spp_stop_rfc_chan_fail);
    }
}


/*****************************************************************************/

void sppServiceHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    if (sppsClientTask)
    {
        switch(id)
        {
        case CL_RFCOMM_REGISTER_CFM:  
            sppHandleRfcommRegisterCfm((CL_RFCOMM_REGISTER_CFM_T *)message);
            break;

        case CL_RFCOMM_UNREGISTER_CFM:
            sppHandleRfcommUnregisterCfm((CL_RFCOMM_UNREGISTER_CFM_T *)message);
            break;
            
        case CL_SDP_REGISTER_CFM: 
            sppHandleSdpRegisterCfm((CL_SDP_REGISTER_CFM_T *)message);
            break;

        case CL_SDP_UNREGISTER_CFM:
            sppHandleSdpUnregisterCfm((CL_SDP_UNREGISTER_CFM_T *)message);
            break;

        case CL_RFCOMM_CONNECT_IND:
            sppHandleRfcommConnectInd( (CL_RFCOMM_CONNECT_IND_T *)message);
            break;
            
        default:
            /* Received an unexpected message */
            SPP_DEBUG(("sppServiceHandler - unexpected msg type 0x%x\n", id));
            break;
        }
    }
#if SPP_DEBUG_LIB
    else
        SPP_DEBUG(("sppsClientTask is NULL!\n"));
#endif
}

    
/*****************************************************************************/

void SppStartService(Task theAppTask )
{
    /* Is there already an SPP service initiated?  */
    if (sppsClientTask)
    {
        sendSppStartServiceCfm(spp_start_already_started);
    }
    else
    {
        sppsClientTask = theAppTask;
        ConnectionRfcommAllocateChannel(
               (Task)&sppsServiceTask, 
               SPP_DEFAULT_CHANNEL
               );
    }
}

/*****************************************************************************/

void SppStopService(Task theAppTask)
{
    /* Is there an SPP service to stop? */
    if (!sppsClientTask)
    {
        sendSppStopServiceCfm(theAppTask, spp_stop_not_started);
    }
    else if (sppsClientTask != theAppTask)
    {
        sendSppStopServiceCfm(theAppTask, spp_stop_invalid_app_task);
    }
    else
    {
        ConnectionUnregisterServiceRecord(
            (Task)&sppsServiceTask,
            sppGetServiceHandle()
            );
    }
}

