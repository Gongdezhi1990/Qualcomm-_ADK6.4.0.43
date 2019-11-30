/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    dm_sync_handler.c

DESCRIPTION
    This file contains the functions responsible for managing the setting up
    and tearing down of Synchronous connections.

NOTES

*/


/****************************************************************************
    Header files
*/
#include "connection.h"
#include "connection_private.h"
#include "common.h"
#include "dm_link_policy_handler.h"
#include "dm_sync_handler.h"

#include <bdaddr.h>
#include <vm.h>
#include <sink.h>
#include <stream.h>

#ifndef CL_EXCLUDE_SYNC

typedef struct 
{
    uint8   link_type;
    uint8   tesco;
    uint8   wesco;
    uint16  rx_packet_length;
    uint16  tx_packet_length;
    Sink    sink;
    uint16  sco_handle;     /* LMP SCO Handle. */
} link_params;



static void sendSyncConnectCfmToClient(
        Task appTask,
        Task clientTask,
        bdaddr* bd_addr,
        hci_status status,
        const link_params *params
        )
{
    if (clientTask)
    {
        MAKE_CL_MESSAGE(CL_DM_SYNC_CONNECT_CFM);
        /* Initially assume an unknown link type and refine later */
        message->link_type = sync_link_unknown;
        message->audio_sink = 0;
        message->rx_bandwidth = 0;
        message->tx_bandwidth = 0;
        message->wesco = params->wesco;

        /* Fill in bd_addr if valid one could be obtained */
        if(bd_addr != NULL)
            message->bd_addr = *bd_addr;
        else
            BdaddrSetZero(&message->bd_addr);
        
        /* Only attempt to fill in link_type, audio_sink and bandwidths if
         * connection attempt was successful 
         */
        if (!status && params)
        {
            message->audio_sink = params->sink;        
            message->sco_handle = params->sco_handle;
            
            if ( params->link_type==0x00 )
            {
                message->link_type = sync_link_sco;
                message->rx_bandwidth = 8000;  /* SCO will always be 8kB/s */
                message->tx_bandwidth = 8000;
            }
            else if ( params->link_type==0x02 )
            {
                message->link_type = sync_link_esco;
                if ( params->tesco )
                {   
                    /* Calculate eSCO bandwidths - formula can be found in 
                     * BT v1.2 spec 
                     */
                    message->rx_bandwidth = 
                        (1600UL * params->rx_packet_length) / params->tesco;
                    message->tx_bandwidth = 
                        (1600UL * params->tx_packet_length) / params->tesco;
                }
            }
        }
        
        message->status = status;
        
        if (!status && appTask)
        {
            const msg_filter *msgFilter = connectionGetMsgFilter();

            if (msgFilter[0] & msg_group_sync_cfm)
            {
                CL_DM_SYNC_CONNECT_CFM_T *appcopy 
                    = PanicUnlessNew(CL_DM_SYNC_CONNECT_CFM_T);
                COPY_CL_MESSAGE(message, appcopy);
                MessageSend(appTask, CL_DM_SYNC_CONNECT_CFM, appcopy);
            }
        }

        MessageSend(clientTask, CL_DM_SYNC_CONNECT_CFM, message);
    }
}


/****************************************************************************
NAME
    connectionHandleSyncRegisterReq
    
DESCRIPTION
    Register the task as utilising Synchronous connections. This registers it
    with BlueStack. On an incoming Synchronous connection the task will be
    asked whether its willing to accept it. All tasks wishing to use
    Synchronous connections must call this register function.

RETURNS
    void
*/
void connectionHandleSyncRegisterReq(const CL_INTERNAL_SYNC_REGISTER_REQ_T *req)
{
    /* 
        Send a register request to BlueStack it will keep track of the task id.
        This is sent by each task wishing to use Synchronous connections.
    */
    MAKE_PRIM_T(DM_SYNC_REGISTER_REQ);
    prim->phandle = 0;
    prim->pv_cbarg = (context_t)req->theAppTask;

    VmSendDmPrim(prim);    
    
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND);
        message->theAppTask = req->theAppTask;
        MessageSendLater(
                connectionGetCmTask(),
                CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND,
                message,
                (uint32)SYNC_REGISTER_TIMEOUT
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncRegisterCfm
    
DESCRIPTION
    Task has been sucessfully registered for receiving Synchronous connection
    notifications - inform the client.
    
RETURNS
    void
*/
void connectionHandleSyncRegisterCfm(const DM_SYNC_REGISTER_CFM_T *cfm)
{
    /* Cancel the message checking we got a register cfm from BlueStack */
    (void) MessageCancelFirst(
            connectionGetCmTask(),
            CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND
            );
    
    if (cfm->pv_cbarg)
    {
        MAKE_CL_MESSAGE(CL_DM_SYNC_REGISTER_CFM);
        message->status = success;
        MessageSend(
                (Task) (cfm->pv_cbarg),
                CL_DM_SYNC_REGISTER_CFM,
                message
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncRegisterTimeoutInd
    
DESCRIPTION
    Task has not been registered for receiving Synchronous connection 
    notifications - inform the client.
    
RETURNS
    void
*/
void connectionHandleSyncRegisterTimeoutInd(
        const CL_INTERNAL_SYNC_REGISTER_TIMEOUT_IND_T *ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_SYNC_REGISTER_CFM);
    message->status = fail;
    MessageSend(
            ind->theAppTask,
            CL_DM_SYNC_REGISTER_CFM,
            message
            );
}


/****************************************************************************
NAME    
    connectionHandleSyncUnregisterReq

DESCRIPTION
    Unregister task with BlueStack indicating it is no longer interested in
    being notified about incoming Synchronous connections.

RETURNS
    void
*/
void connectionHandleSyncUnregisterReq(
        const CL_INTERNAL_SYNC_UNREGISTER_REQ_T *req
        )
{
    /* Send an unregister request to BlueStack */
    MAKE_PRIM_T(DM_SYNC_UNREGISTER_REQ);
    prim->phandle = 0;
    prim->pv_cbarg = (context_t)req->theAppTask;
    VmSendDmPrim(prim);
    
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND);
        message->theAppTask = req->theAppTask;
        MessageSendLater(
                connectionGetCmTask(),
                CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND,
                message,
                (uint32)SYNC_UNREGISTER_TIMEOUT
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncUnregisterCfm

DESCRIPTION
    Task has been sucessfully unregistered from receiving Synchronous connection
    notifications - inform the client.

RETURNS
    void
*/
void connectionHandleSyncUnregisterCfm(const DM_SYNC_UNREGISTER_CFM_T *cfm)
{
    /* Cancel the message checking we got a register cfm from BlueStack */
    (void) MessageCancelFirst(
            connectionGetCmTask(),
            CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND
            );
    
    if (cfm->pv_cbarg)
    {
        MAKE_CL_MESSAGE(CL_DM_SYNC_UNREGISTER_CFM);
        message->status = success;
        MessageSend(
                (Task) (cfm->pv_cbarg),
                CL_DM_SYNC_UNREGISTER_CFM,
                message
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncUnregisterTimeoutInd

DESCRIPTION
    Task has not been unregistered from receiving Synchronous connection
    notifications - inform the client.

RETURNS
    void
*/
void connectionHandleSyncUnregisterTimeoutInd(
        const CL_INTERNAL_SYNC_UNREGISTER_TIMEOUT_IND_T *ind
        )
{
    MAKE_CL_MESSAGE(CL_DM_SYNC_UNREGISTER_CFM);
    message->status = fail;
    MessageSend(
            ind->theAppTask,
            CL_DM_SYNC_UNREGISTER_CFM,
            message
            );
}


/****************************************************************************
NAME
    connectionHandleSyncConnectReq

DESCRIPTION
    Request to open a Synchronous connection to a remote device.

RETURNS
    void
*/
void connectionHandleSyncConnectReq(const CL_INTERNAL_SYNC_CONNECT_REQ_T *req)
{
    tp_bdaddr    tpaddr;

    /* 
       We are assuming that the client requests a Synchronous packet type that
       the remote end supports. If it doesn't support the type requested the
       Synchronous will not be opened and the cfm will have the appropriate
       error code set, this will be passed on to the client. The client should
       use the function for reading the remote supported features if it needs
       to determine the Synchronous packet types supported by the remote end.
       */    
    if (SinkGetBdAddr(req->sink, &tpaddr))
    {
        DM_SYNC_CONFIG_T* config 
            = (DM_SYNC_CONFIG_T*)PanicUnlessMalloc(
                    sizeof(DM_SYNC_CONFIG_T)
                    );

        MAKE_PRIM_T(DM_SYNC_CONNECT_REQ);
        prim->phandle  = 0;
        prim->pv_cbarg = (context_t) req->theAppTask;
        prim->length   = 0;

        BdaddrConvertVmToBluestack(&prim->bd_addr, &tpaddr.taddr.addr);

        config->tx_bdw = req->config_params.tx_bandwidth;
        config->rx_bdw = req->config_params.rx_bandwidth;
        config->voice_settings = req->config_params.voice_settings;
        config->max_latency = req->config_params.max_latency;
        config->retx_effort = req->config_params.retx_effort;
        config->packet_type 
            = (hci_pkt_type_t)req->config_params.packet_type;

        prim->u.config = VmGetHandleFromPointer(config);

        VmSendDmPrim(prim);
    }
    else
    {
        /* Addr not valid send an error to the client */
        sendSyncConnectCfmToClient(
                0,
                req->theAppTask,
                NULL,
                hci_error_no_connection,
                0
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncConnectCfm

DESCRIPTION

    Response to the Synchronous connect request indicating either that an
    Synchronous has been opened or that the attempt has failed.

RETURNS
    void
*/
void connectionHandleSyncConnectCfm(
        Task theAppTask,
        const DM_SYNC_CONNECT_CFM_T *cfm
        )
{
    bdaddr addr;
    
    link_params params;
    params.link_type = cfm->link_type;
    params.tesco = cfm->tx_interval;
    params.wesco = cfm->wesco;
    params.rx_packet_length = cfm->rx_packet_length;
    params.tx_packet_length = cfm->tx_packet_length;
    params.sco_handle = cfm->handle;
    params.sink = StreamScoSink(cfm->handle);

    BdaddrConvertBluestackToVm(&addr, &cfm->bd_addr);
    
    /* Tell the client task that we have a cfm for the Sync request */
    sendSyncConnectCfmToClient(
            theAppTask,
            (Task) (cfm->pv_cbarg),
            &addr,
            cfm->status,
            &params
            );
}


/****************************************************************************
NAME
    connectionHandleSyncConnectInd

DESCRIPTION
    Indication that the remote device wishes to open an Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncConnectInd(const DM_SYNC_CONNECT_IND_T *ind)
{
    if (ind->pv_cbarg)
    {
        MAKE_CL_MESSAGE(CL_DM_SYNC_CONNECT_IND);
        BdaddrConvertBluestackToVm(&message->bd_addr, &ind->bd_addr);
        MessageSend(
                (Task) (ind->pv_cbarg),
                CL_DM_SYNC_CONNECT_IND,
                message
                );
    }
}


/****************************************************************************
NAME    
    connectionHandleSyncConnectCompleteInd

DESCRIPTION
    Indication that the remote devices wish to open an Synchronous connection
    has completed.

RETURNS
    void
*/
void connectionHandleSyncConnectCompleteInd(
        Task theAppTask,
        const DM_SYNC_CONNECT_COMPLETE_IND_T *ind
        )
{
    bdaddr addr;

    link_params params;
    params.link_type = ind->link_type;
    params.tesco = ind->tx_interval;
    params.wesco = ind->wesco;
    params.rx_packet_length = ind->rx_packet_length;
    params.tx_packet_length = ind->tx_packet_length;
    params.sco_handle = ind->handle;
    params.sink = StreamScoSink(ind->handle);

    BdaddrConvertBluestackToVm(&addr, &ind->bd_addr);
    
    /* Tell the client task that we have a cfm for the Sync request */
    sendSyncConnectCfmToClient(
            theAppTask,
            (Task) (ind->pv_cbarg),
            &addr,
            params.sink ? ind->status : hci_error_no_connection,
            &params
            );
}



/****************************************************************************
NAME
    connectionHandleSyncConnectRes

DESCRIPTION
    Response accepting (or not) an incoming Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncConnectRes(const CL_INTERNAL_SYNC_CONNECT_RES_T *res)
{
    MAKE_PRIM_T(DM_SYNC_CONNECT_RSP);
    
    BdaddrConvertVmToBluestack(&prim->bd_addr, &res->bd_addr);

    prim->config.tx_bdw = res->config_params.tx_bandwidth;
    prim->config.rx_bdw = res->config_params.rx_bandwidth;
    prim->config.voice_settings = res->config_params.voice_settings;
    prim->config.max_latency = res->config_params.max_latency;
    prim->config.retx_effort = res->config_params.retx_effort;
    prim->config.packet_type = (hci_pkt_type_t)res->config_params.packet_type;
    
    if ( res->response )
    {
        prim->response = hci_success;
    }
    else
    {
        /* 
         * Reason must be one of:
         *      hci_error_rej_by_remote_no_res
         *      hci_error_rej_by_remote_sec 
         *      hci_error_rej_by_remote_pers
         */
        prim->response = hci_error_rej_by_remote_no_res;
    }
    
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    connectionHandleSyncDisconnectReq

DESCRIPTION
    Request to disconnect an existing Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncDisconnectReq(
        const CL_INTERNAL_SYNC_DISCONNECT_REQ_T *req
        )
{
    /* Send a SCO disconnect request to BlueStack */
    MAKE_PRIM_T(DM_SYNC_DISCONNECT_REQ);
    prim->handle = PanicZero(SinkGetScoHandle(req->audio_sink));;
    prim->reason = (uint16)req->reason;
    VmSendDmPrim(prim);
}


/****************************************************************************
NAME
    connectionHandleSyncDisconnectInd

DESCRIPTION

    Indication that the Synchronous connection has been disconnected. The
    disconnect will have been initiated by the remote device.

RETURNS
    void
*/
void connectionHandleSyncDisconnectInd(const DM_SYNC_DISCONNECT_IND_T *ind)
{
    if (ind->pv_cbarg)
    {

        /* Indication that the Synchronous connection has been disconnected -
         * tell the relevant task 
         */
        MAKE_CL_MESSAGE(CL_DM_SYNC_DISCONNECT_IND);

        message->audio_sink = StreamScoSink(ind->handle);
        message->reason = connectionConvertHciStatus(ind->reason);

        /* Status is meaningless here for incoming Disconnect. Set to success.
         */
        
        message->status = hci_success;
        MessageSend(
                (Task) (ind->pv_cbarg),
                CL_DM_SYNC_DISCONNECT_IND, 
                message
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncDisconnectCfm

DESCRIPTION
    Confirmation that the Synchronous connection has been disconnected. The
    discconect will have been initiated by the local device.

RETURNS
    void
*/
void connectionHandleSyncDisconnectCfm(const DM_SYNC_DISCONNECT_CFM_T *cfm)
{
    if (cfm->pv_cbarg)
    {

        /* Indication that the Synchronous connection has been disconnected -
         * tell the relevant task. */
        MAKE_CL_MESSAGE(CL_DM_SYNC_DISCONNECT_IND);

        /* TODO - Connection handle has to be given to App instead of just Sink
         */

        message->audio_sink = StreamScoSink(cfm->handle);
        message->status = connectionConvertHciStatus(cfm->status);
        
        /* Disconnect initiated by local device so reason is meaningless really.
         */ 
        message->reason = hci_success; 
        
        MessageSend(
                (Task) (cfm->pv_cbarg),
                CL_DM_SYNC_DISCONNECT_IND,
                message
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncRenegotiateReq

DESCRIPTION
    Request to change the connection parameters of an existing Synchronous
    connection.

RETURNS
    void
*/
void connectionHandleSyncRenegotiateReq(
        const CL_INTERNAL_SYNC_RENEGOTIATE_REQ_T *req
        )
{
    if ( !req->audio_sink )
    {
        /* Sink not valid send an error to the client */
        sendSyncConnectCfmToClient(
                0,
                req->theAppTask,
                NULL,
                hci_error_no_connection,
                0
                );
    }
    else
    {
        DM_SYNC_CONFIG_T* config 
            = (DM_SYNC_CONFIG_T*) PanicUnlessMalloc(sizeof(DM_SYNC_CONFIG_T)); 
        MAKE_PRIM_T(DM_SYNC_RENEGOTIATE_REQ);

        prim->length = 0;
        prim->handle = PanicZero(SinkGetScoHandle(req->audio_sink));
    
        config->max_latency = req->config_params.max_latency;
        config->retx_effort = req->config_params.retx_effort;
        config->voice_settings = req->config_params.voice_settings;
        config->packet_type = (hci_pkt_type_t)req->config_params.packet_type;
        config->rx_bdw = req->config_params.rx_bandwidth;
        config->tx_bdw = req->config_params.tx_bandwidth;
        prim->u.config = VmGetHandleFromPointer(config);
        
        /* EDR bits use inverted logic at HCI interface */
        /* prim->packet_type ^= sync_all_edr_esco;*/
        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME
    connectionHandleSyncRenegotiateInd

DESCRIPTION
    Indication that remote device has changed the connection parameters of an
    existing Synchronous connection.

RETURNS
    void
*/
void connectionHandleSyncRenegotiateInd(const DM_SYNC_RENEGOTIATE_IND_T *ind)
{
    if (ind->pv_cbarg)
    {
        MAKE_CL_MESSAGE(CL_DM_SYNC_RENEGOTIATE_IND);

        message->audio_sink = StreamScoSink(ind->handle);
        message->status = connectionConvertHciStatus(ind->status);
        MessageSend(
                (Task) (ind->pv_cbarg),
                CL_DM_SYNC_RENEGOTIATE_IND,
                message
                );
    }
}


/****************************************************************************
NAME
    connectionHandleSyncRenegotiateCfm

DESCRIPTION

    Confirmation of local device's attempt to change the connection parameters
    of an existing Synchronous connection.


RETURNS
    void
*/
void connectionHandleSyncRenegotiateCfm(const DM_SYNC_RENEGOTIATE_CFM_T *cfm)
{
    if (cfm->pv_cbarg)
    {
        MAKE_CL_MESSAGE(CL_DM_SYNC_RENEGOTIATE_IND);

        message->audio_sink = StreamScoSink(cfm->handle);
        message->status = connectionConvertHciStatus(cfm->status);
        MessageSend(
                (Task) (cfm->pv_cbarg),
                CL_DM_SYNC_RENEGOTIATE_IND,
                message
                );
    }
}


#endif
