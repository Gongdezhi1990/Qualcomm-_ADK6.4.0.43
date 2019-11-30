/****************************************************************************
Copyright (c) 2004 - 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    l2cap_handler.c        

DESCRIPTION
    This file contains the guts of the L2CAP connection manager 
    Connection library will ALWAYS use BlueStack L2CAP AutoConfig and only 
    support the following L2CAP modes:
        Basic
        Enhanced Retransmission
        Streaming

NOTES

*/

/****************************************************************************
    Header files
*/
#include "connection.h"
#include "common.h"
#include "connection_private.h"
#include "dm_link_policy_handler.h"
#include "l2cap_handler.h"

#include <panic.h>
#include <print.h>
#include <vm.h>

#include <stdio.h>

#include <bdaddr.h>
#include <sink.h>
#include <stream.h>
#include <source.h>
#include <vmal.h>
#include <message.h>

#ifndef CL_EXCLUDE_L2CAP

/* Init QoS struct to default values */
static void setQosToDefault(qos_flow *qos)
{
    qos->service_type = L2CA_QOS_DEFAULT_SERVICE_TYPE;
    qos->token_rate = L2CA_QOS_DEFAULT_TOKEN_RATE;
    qos->token_bucket = L2CA_QOS_DEFAULT_TOKEN_BUCKET;
    qos->peak_bw = L2CA_QOS_DEFAULT_PEAK_BW;
    qos->latency = L2CA_QOS_DEFAULT_LATENCY;
    qos->delay_var = L2CA_QOS_DEFAULT_DELAY_VAR;
}

static void convertToQosFlow_t(const L2CA_QOS_T *in, qos_flow *out)
{
    out->service_type = in->service_type;
    out->token_rate = in->token_rate;
    out->token_bucket = in->token_bucket;
    out->peak_bw = in->peak_bw;
    out->latency = in->latency;
    out->delay_var = in->delay_var;
}

/* Create a connect cfm message and send it to the supplied task */
static void sendL2capConnectCfm(
        Task appTask,
        l2cap_connect_status status,
        uint16 psm,
        Sink sink,
        uint16 mtu,
        uint16 timeout,
        const qos_flow *qos,
        uint8 mode,
        l2ca_cid_t cid,
        bdaddr *addr
        )
{
    CL_DEBUG_INFO(("CL_L2CAP_CONNECT_CFM status 0x%x psm 0x%x sink 0x%p\n", 
                status,
                psm,
                (void*)sink));

    {
        MAKE_CL_MESSAGE(CL_L2CAP_CONNECT_CFM);
        message->status = status;
        message->psm_local = psm;
        message->sink = sink;
        message->connection_id = cid;
        if (addr)
            message->addr = *addr;
        message->mtu_remote = mtu;
        message->flush_timeout_remote = timeout;
        message->mode = mode;

        if (qos)
            message->qos_remote = *qos;
        else
            setQosToDefault(&message->qos_remote);

        MessageSend(appTask, CL_L2CAP_CONNECT_CFM, message);
    }
}

/* Create a LE COC connect cfm message and send it to the supplied task */
static void sendL2capTpConnectCfm(
        Task appTask,
        l2cap_tp_connect_status status,
        uint16 psm,
        Sink sink,
        uint16 mtu,
        uint16 timeout,
        const qos_flow *qos,
        uint8 mode,
        l2ca_cid_t cid,
        tp_bdaddr *addr
        )
{
    CL_DEBUG_INFO((
                "CL_L2CAP_TP_CONNECT_CFM status 0x%x psm 0x%x sink 0x%x\n", 
                status,
                psm,
                (uint16)sink)
            );

    {
        MAKE_CL_MESSAGE(CL_L2CAP_TP_CONNECT_CFM);
        message->status = status;
        message->psm_local = psm;
        message->sink = sink;
        message->connection_id = cid;
        if (addr)
            message->taddr = *addr;
        message->mtu_remote = mtu;
        message->flush_timeout_remote = timeout;
        message->mode = mode;

        if (qos)
            message->qos_remote = *qos;
        else
            setQosToDefault(&message->qos_remote);

        MessageSend(appTask, CL_L2CAP_TP_CONNECT_CFM, message);
    }
}

/* Create a LE COC add credit cfm message and send it to the supplied task */
static void sendL2capAddCreditCfm(
        Task appTask,
        uint16 status,
        uint16 credits,
        l2ca_cid_t cid
        )
{
    CL_DEBUG_INFO((
                "CL_L2CAP_ADD_CREDIT_CFM status 0x%x credits 0x%x\n", 
                (uint16)status,
                credits)
            );

    {
        MAKE_CL_MESSAGE(CL_L2CAP_ADD_CREDIT_CFM);
        
        message->status = (L2CA_RESULT_T)status;
        message->credits = credits;
        message->connection_id = cid;

        MessageSend(appTask, CL_L2CAP_ADD_CREDIT_CFM, message);
    }
}

static void sendL2capDisconnectCfm(Task appTask, l2cap_disconnect_status status, Sink sink)
{
    /* Sink return_sink; */
    MAKE_CL_MESSAGE(CL_L2CAP_DISCONNECT_CFM);
    message->sink = sink;
    message->status = status;
    MessageSend(appTask, CL_L2CAP_DISCONNECT_CFM, message);

    CL_DEBUG_INFO(("CL_L2CAP_DISCONNECT_CFM status 0x%x\n", status));
}

static l2cap_disconnect_status convertDisconnectResult(l2ca_disc_result_t disc_result)
{
    /* Convert the BlueStack status into the connection lib return type */
    switch(disc_result)
    {
        case L2CA_DISCONNECT_NORMAL:
            return l2cap_disconnect_successful;
        case L2CA_DISCONNECT_LINK_LOSS:
            return l2cap_disconnect_link_loss;
        case L2CA_DISCONNECT_TIMEOUT:
            return l2cap_disconnect_timed_out;
        default:    
            return l2cap_disconnect_error;
    }
}

/****************************************************************************
NAME    
    connectionHandleL2capRegisterReq

DESCRIPTION
    Used by client tasks to register a PSM so remote devices can connect to 
    it. 

    Connection library will only support the following flow and error control 
    modes:
        Basic
        Enhanced Retransmission
        Flow

    Connection library only uses the Auto L2CAP API.

RETURNS
	void
*/
void connectionHandleL2capRegisterReq(
        const CL_INTERNAL_L2CAP_REGISTER_REQ_T *req
        )
{
    MAKE_PRIM_T(L2CA_REGISTER_REQ);
    prim->psm_local     = req->app_psm;
    prim->phandle       = 0;
    if(req->flags & (L2CA_REGFLAG_FOR_LE_TP |
                   L2CA_REGFLAG_USE_TP_PRIMS))
    {
        prim->mode_mask = 0;
    }
    else
    {
    prim->mode_mask     = L2CA_MODE_MASK_BASIC | 
                          L2CA_MODE_MASK_ENHANCED_RETRANS | 
                          L2CA_MODE_MASK_STREAMING;
    }
    prim->flags         = req->flags;
    prim->reg_ctx       = CTX_FROM_TASK(req->clientTask);
    
    /* Reserved parameters - intialise to 0 */
    prim->connection_oriented.buffer_size_sink      = 0;
    prim->connection_oriented.buffer_size_source    = 0;
    prim->connectionless.buffer_size_sink           = 0;
    prim->connectionless.buffer_size_source         = 0;

    VmSendL2capPrim(prim);
}


/****************************************************************************
NAME    
    connectionHandleL2capRegisterCfm

DESCRIPTION
    Confirmation that a PSM has been registered with the L2CAP layer of
    BlueStack.

RETURNS
    void
*/
void connectionHandleL2capRegisterCfm(const L2CA_REGISTER_CFM_T *cfm)
{
    if (cfm->reg_ctx)
    {
        connection_lib_status result = success;
        Task t = (Task)cfm->reg_ctx;

        MAKE_CL_MESSAGE(CL_L2CAP_REGISTER_CFM);

        if (cfm->result) 
        {
            result = fail;
        }
        message->status = result;
        message->psm = cfm->psm_local;
        MessageSend(t, CL_L2CAP_REGISTER_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG_INFO(("L2CAP_REGISTER_CFM reg_ctx is 0x0\n"));
    }
#endif   
}

/****************************************************************************
NAME    
    connectionHandleL2capUnregisterReq

DESCRIPTION
    Request to unregister a particular psm.

RETURNS
    void
*/
void connectionHandleL2capUnregisterReq(const CL_INTERNAL_L2CAP_UNREGISTER_REQ_T *req)
{
    MAKE_PRIM_T(L2CA_UNREGISTER_REQ);
    prim->psm_local = req->app_psm;
    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME
    connectionHandleL2capUnregisterCfm

DESCRIPTION
    Confirmation from Bluestack that the PSM has been un-registered. This is
    passed on to the application.

RETURNS 
    void
*/
void connectionHandleL2capUnregisterCfm(const L2CA_UNREGISTER_CFM_T *cfm)
{
    if (cfm->reg_ctx)
    {
        MAKE_CL_MESSAGE(CL_L2CAP_UNREGISTER_CFM);
            
        message->psm = cfm->psm_local;
        if (cfm->result == L2CA_RESULT_SUCCESS)
        {
            message->status = success;
        }
        else
        {
            message->status = fail;
        }
        MessageSend((Task)cfm->reg_ctx, CL_L2CAP_UNREGISTER_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG_INFO(("L2CAP_UNREGISTER_CFM reg_ctx is 0x0\n"));
    }
#endif   
}

/****************************************************************************
NAME    
    connectionHandleL2capConnectReq

DESCRIPTION
    Request to initiate an L2CAP connection.

RETURNS
    void
*/
void connectionHandleL2capConnectReq(const CL_INTERNAL_L2CAP_CONNECT_REQ_T *req)
{
    /* Attempt to connect */
    MAKE_PRIM_T(L2CA_AUTO_CONNECT_REQ);

    prim->con_ctx = CTX_FROM_TASK(req->theAppTask);

    prim->cid = 0;                      /* Only used for reconfigurations */
    prim->psm_local = req->psm_local;
    BdaddrConvertVmToBluestack(&(prim->bd_addr), &(req->bd_addr));
    prim->psm_remote = req->psm_remote;
    prim->remote_control = 0;
    prim->local_control = 0;
    prim->conftab_length = req->conftab_length;
    if (req->conftab_length)
    {
        prim->conftab = (uint16 *) VmGetHandleFromPointer( (void *)req->conftab );
    }
    else
    {
        prim->conftab = 0;
    }
    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleL2capTpConnectReq

DESCRIPTION
    Request to initiate an L2CAP LE COC connection.

RETURNS
    void
*/
void connectionHandleL2capTpConnectReq(const CL_INTERNAL_L2CAP_TP_CONNECT_REQ_T *req)
{
    /* Attempt to connect */
    MAKE_PRIM_T(L2CA_AUTO_TP_CONNECT_REQ);

    prim->con_ctx = CTX_FROM_TASK(req->theAppTask);
    prim->cid = 0;                      /* Only used for reconfigurations */
    prim->psm_local = req->psm_local;
    BdaddrConvertTpVmToBluestack(&(prim->tp_addrt), &(req->taddr));
    prim->psm_remote = req->psm_remote;
    prim->remote_control = 0;
    prim->local_control = 0;
    prim->conftab_length = req->length;    
    if (req->length)
    {
        prim->conftab = (uint16 *) VmGetHandleFromPointer( req->data );
    }
    else
    {
        prim->conftab = 0;
    }
    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleL2capAddCreditReq

DESCRIPTION
    Request to extend credit to remote device.

RETURNS
    void
*/
void connectionHandleL2capAddCreditReq(const CL_INTERNAL_L2CAP_ADD_CREDIT_REQ_T *req)
{
    /* Attempt to connect */
    MAKE_PRIM_T(L2CA_ADD_CREDIT_REQ);

    prim->cid = req->connection_id;
    prim->context = CTX_FROM_TASK(req->theAppTask);
    prim->credits = req->credits;
    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleL2capConnectCfm

DESCRIPTION
    Response to an L2CAP connect request

RETURNS
    void
*/
void connectionHandleL2capConnectCfm(const L2CA_AUTO_CONNECT_CFM_T *cfm)
{
    bdaddr              addr;
    Task                appTask = (Task)cfm->con_ctx;

#ifdef CONNECTION_DEBUG_LIB
    if (!appTask)
        CL_DEBUG(("L2CA_AUTO_CONNECT_CFM con_ctx is 0x0\n"));
#endif

    BdaddrConvertBluestackToVm(&addr, &(cfm->bd_addr));

    if (cfm->result == L2CA_CONNECT_SUCCESS)
    {
        Sink                sink = StreamL2capSink(cfm->cid);
        qos_flow            qos;
        qos_flow            *p_qos = 0;
        l2ca_flow_mode_t    mode = L2CA_FLOW_MODE_BASIC;

        /* these values are passed up in the CFM message. The option bit
           only indicates that they were negotiated */
        l2ca_mtu_t          remote_mtu = cfm->config.mtu;
        l2ca_timeout_t      flush_to = cfm->config.flush_to;

        if (cfm->config.options & L2CA_SPECIFY_QOS) 
        {
            L2CA_QOS_T *l2ca_qos = VmGetPointerFromHandle(cfm->config.qos);
            convertToQosFlow_t(l2ca_qos, &qos);
            p_qos = &qos;
            free(l2ca_qos);
        }
        if (cfm->config.options & L2CA_SPECIFY_FLOW) {
            L2CA_FLOW_T *l2ca_flow = (L2CA_FLOW_T *)VmGetPointerFromHandle(cfm->config.flow);
            mode = l2ca_flow->mode;
            free(l2ca_flow);
        }
        
        /* Send connect cfm to client */
        sendL2capConnectCfm(
            appTask, 
            l2cap_connect_success, 
            cfm->psm_local, sink, 
            remote_mtu, 
            flush_to, 
            p_qos,
            mode,
            cfm->cid,
            &addr
            );
        
        /* Associate the task with its sink */
        MessageStreamTaskFromSink(sink, appTask);

    }
    else if (cfm->result == L2CA_CONNECT_PENDING)
    {
        /* Send a confirmation to the App, so that it can cancel
           using the CID, before the connection is complete. */
        sendL2capConnectCfm(
            appTask, 
            l2cap_connect_pending, 
            cfm->psm_local, 
            0,          /* Streams are not active yet so no Sink */
            0,          /* No remote MTU */
            0,          /* No remote Flush timeout */
            0,          /* No QoS */
            0,          /* No mode */
            cfm->cid,   /* The CID is assigned though */
            &addr
            );
    }
    /* Anything else apart from INITIATING and RETRYING is an error. */
    else if (cfm->result != L2CA_CONNECT_INITIATING &&
             cfm->result != L2CA_CONNECT_RETRYING )
    {
        /* There are corner cases where the appTask may be Null (0) such as
         * when the stack is rejecting an incomming L2CAP_AUTO_CONNECT_IND due
         * to a lack of resource. Since the app didn't receive the IND, there
         * is no context and no appTask to route the error message to.
         * Consume the error without reporting. 
         */
        if (appTask)
        {
            /* Tell the client task the connect attempt failed */
            sendL2capConnectCfm(
                    appTask, 
                    connectionConvertL2capConnectStatus(cfm->result),
                    cfm->psm_local,
                    0, 
                    0, 
                    0, 
                    0, 
                    0, 
                    cfm->cid,
                    &addr
                    );
        }
    }

    /* Free CFM message pointers, even if they are not indicated, just in 
       case. 
     */
    free(VmGetPointerFromHandle(cfm->config.unknown));
}

/****************************************************************************
NAME    
    connectionHandleL2capTpConnectCfm

DESCRIPTION
    Response to an L2CAP LE COC connect request

RETURNS
    void
*/
void connectionHandleL2capTpConnectCfm(const L2CA_AUTO_TP_CONNECT_CFM_T *cfm)
{
    tp_bdaddr           addr;
    Task                appTask = (Task)cfm->con_ctx;

#ifdef CONNECTION_DEBUG_LIB
    if (!appTask)
        CL_DEBUG(("L2CA_AUTO_TP_CONNECT_CFM con_ctx is 0x0\n"));
#endif

    BdaddrConvertTpBluestackToVm(&addr, &(cfm->tp_addrt));

    if (cfm->result == L2CA_LE_CONNECT_SUCCESS)
    {
        Sink                sink = StreamL2capSink(cfm->cid);
        qos_flow            qos;
        qos_flow            *p_qos = 0;
        l2ca_flow_mode_t    mode = L2CA_FLOW_MODE_BASIC;

        /* these values are passed up in the CFM message. The option bit
           only indicates that they were negotiated */
        l2ca_mtu_t          remote_mtu = cfm->config.remote_mtu;
        l2ca_timeout_t      flush_to = cfm->config.flush_to;

        if (cfm->config.options & L2CA_SPECIFY_QOS) 
        {
            L2CA_QOS_T *l2ca_qos = VmGetPointerFromHandle(cfm->config.qos);
            convertToQosFlow_t(l2ca_qos, &qos);
            p_qos = &qos;
            free(l2ca_qos);
        }
        if (cfm->config.options & L2CA_SPECIFY_FLOW) {
            L2CA_FLOW_T *l2ca_flow = (L2CA_FLOW_T *)VmGetPointerFromHandle(cfm->config.flow);
            mode = l2ca_flow->mode;
            free(l2ca_flow);
        }
        
        /* Send connect cfm to client */
        sendL2capTpConnectCfm(
            appTask, 
            l2cap_tp_connect_success, 
            cfm->psm_local, sink, 
            remote_mtu, 
            flush_to, 
            p_qos,
            mode,
            cfm->cid,
            &addr
            );
        
        /* Associate the task with its sink */
        MessageStreamTaskFromSink(sink, appTask);

    }
    else if (cfm->result == L2CA_CONNECT_PENDING)
    {
        /* Send a confirmation to the App, so that it can cancel
           using the CID, before the connection is complete. */
        sendL2capTpConnectCfm(
            appTask, 
            l2cap_tp_connect_pending, 
            cfm->psm_local, 
            0,          /* Streams are not active yet so no Sink */
            0,          /* No remote MTU */
            0,          /* No remote Flush timeout */
            0,          /* No QoS */
            0,          /* No mode */
            cfm->cid,   /* The CID is assigned though */
            &addr
            );
    }
    /* Anything else apart from INITIATING and RETRYING is an error. */
    else if (cfm->result != L2CA_CONNECT_INITIATING &&
             cfm->result != L2CA_CONNECT_RETRYING )
    {
        /* There are corner cases where the appTask may be Null (0) such as
         * when the stack is rejecting an incomming L2CAP_AUTO_CONNECT_IND due
         * to a lack of resource. Since the app didn't receive the IND, there
         * is no context and no appTask to route the error message to.
         * Consume the error without reporting. 
         */
        if (appTask)
        {
            /* Tell the client task the connect attempt failed */
            sendL2capTpConnectCfm(
                    appTask, 
                    connectionConvertL2capTpConnectStatus(cfm->result),
                    cfm->psm_local,
                    0, 
                    0, 
                    0, 
                    0, 
                    0, 
                    cfm->cid,
                    &addr
                    );
        }
    }

    /* Free CFM message pointers, even if they are not indicated, just in 
       case. 
     */
    free(VmGetPointerFromHandle(cfm->config.unknown));
}

/****************************************************************************
NAME    
    connectionHandleL2capAddCreditCfm

DESCRIPTION
    Response to an L2CAP LE COC add credit request

RETURNS
    void
*/
void connectionHandleL2capAddCreditCfm(const L2CA_ADD_CREDIT_CFM_T *cfm)
{
    Task                appTask = (Task)cfm->context;

    /* Send connect cfm to client */
    sendL2capAddCreditCfm(
        appTask,
        cfm->result, 
        cfm->credits, 
        cfm->cid
        );
}

/****************************************************************************
NAME    
    connectionHandleL2capConnectInd

DESCRIPTION
    Indication that a remote device is trying to connect to this device.

RETURNS
    void
*/
void connectionHandleL2capConnectInd(const L2CA_AUTO_CONNECT_IND_T *ind)
{   
    bdaddr addr;

    if (ind->reg_ctx)
    {
        Task t = (Task)ind->reg_ctx;

        MAKE_CL_MESSAGE(CL_L2CAP_CONNECT_IND);

        CL_DEBUG_INFO(("CONNECT IND psm 0x%x cid 0x%x\n",
                       ind->psm_local, ind->cid));

        BdaddrConvertBluestackToVm(&addr, &(ind->bd_addr));

        message->identifier = ind->identifier;
        message->bd_addr = addr;
        message->psm = ind->psm_local;
        message->connection_id = ind->cid;
        MessageSend(t, CL_L2CAP_CONNECT_IND, message);
    }
}

/****************************************************************************
NAME    
    connectionHandleL2capTpConnectInd

DESCRIPTION
    Indication that a remote device is trying to connect to this device.

RETURNS
    void
*/
void connectionHandleL2capTpConnectInd(const L2CA_AUTO_TP_CONNECT_IND_T *ind)
{   
    tp_bdaddr addr;

    if (ind->reg_ctx)
    {
        MAKE_CL_MESSAGE(CL_L2CAP_TP_CONNECT_IND);     

        CL_DEBUG_INFO(("TP CONNECT IND psm 0x%x cid 0x%x\n",
                       ind->psm_local, ind->cid));

        BdaddrConvertTpBluestackToVm(&addr, &(ind->tp_addrt));

        message->identifier = ind->identifier;
        message->taddr = addr;
        message->psm = ind->psm_local;
        message->connection_id = ind->cid;
        MessageSend((Task)ind->reg_ctx, CL_L2CAP_TP_CONNECT_IND, message);
    }
}

/****************************************************************************
NAME    
    connectionHandleL2capConnectRes

DESCRIPTION
    Handle a response from the client task telling us whether to proceed
    with establishing the L2CAP connection. 

RETURNS
    void
*/
void connectionHandleL2capConnectRes(const CL_INTERNAL_L2CAP_CONNECT_RES_T *res)
{

    MAKE_PRIM_T(L2CA_AUTO_CONNECT_RSP);
    
    prim->con_ctx = CTX_FROM_TASK(res->theAppTask);
    prim->identifier = res->identifier;
    prim->cid = res->connection_id;
    

    if (res->response)
    {
        prim->response = L2CA_CONNECT_SUCCESS;
        prim->conftab_length = res->conftab_length;

        if (res->conftab_length)
            prim->conftab = (uint16 *) VmGetHandleFromPointer((void *)res->conftab);
        else
            prim->conftab = 0;
    }
    else
    {
        prim->response = L2CA_CONNECT_REJ_RESOURCES;

#ifdef CONNECTION_DEBUG_LIB
        if (res->conftab_length)
            CL_DEBUG_INFO(("conftab_length should be 0 if rejecting a connection!\n"));
#endif
        prim->conftab = 0;
        prim->conftab_length = 0;
    }

    VmSendL2capPrim(prim);
}    

/****************************************************************************
NAME    
    connectionHandleL2capTpConnectRes

DESCRIPTION
    Handle a response from the client task telling us whether to proceed
    with establishing the L2CAP LE COC connection. 

RETURNS
    void
*/
void connectionHandleL2capTpConnectRes(const CL_INTERNAL_L2CAP_TP_CONNECT_RES_T *res)
{

    MAKE_PRIM_T(L2CA_AUTO_TP_CONNECT_RSP);
    
    prim->con_ctx = CTX_FROM_TASK(res->theAppTask);
    prim->identifier = res->identifier;
    prim->cid = res->connection_id;
    

    if (res->response)
    {
        prim->response = L2CA_LE_CONNECT_SUCCESS;
        prim->conftab_length = res->length;

        if (res->length)
            prim->conftab = (uint16 *) VmGetHandleFromPointer(res->data);
        else
            prim->conftab = 0;
    }
    else
    {
        prim->response = L2CA_LE_CONNECT_REJ_RESOURCES;

#ifdef CONNECTION_DEBUG_LIB
        if (res->length)
            CL_DEBUG_INFO(("conftab_length should be 0 if rejecting a connection!\n"));
#endif
        prim->conftab = 0;
        prim->conftab_length = 0;
    }

    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleL2capDisconnectReq

DESCRIPTION
    Request by the local device to disconnect the L2CAP connection

RETURNS
    void
*/
void connectionHandleL2capDisconnectReq(const CL_INTERNAL_L2CAP_DISCONNECT_REQ_T *req)
{
    if (req->sink && SinkIsValid(req->sink))
    {
        MAKE_PRIM_T(L2CA_DISCONNECT_REQ);
        
        prim->cid = (l2ca_cid_t) PanicZero( SinkGetL2capCid(req->sink) );
        VmSendL2capPrim(prim);
    }
    else
    {
        sendL2capDisconnectCfm(
            req->theAppTask, 
            l2cap_disconnect_no_connection, 
            req->sink
            );
    }
}


/****************************************************************************
NAME    
    connectionHandleL2capDisconnectCfm

DESCRIPTION
    L2CAP connection has been disconnected by the local end.

RETURNS
    void
*/
void connectionHandleL2capDisconnectCfm(const L2CA_DISCONNECT_CFM_T *cfm)
{
    if (cfm->con_ctx)
    {
        sendL2capDisconnectCfm(
            (Task)cfm->con_ctx, 
            convertDisconnectResult(cfm->result), 
            StreamL2capSink(cfm->cid)
            );
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG_INFO(("L2CAP_DISCONNECT_CFM reg_ctx is 0x0\n"));
    }
#endif   

}


/****************************************************************************
NAME    
    connectionHandleL2capDisconnectInd

DESCRIPTION
    L2CAP connection has been disconnected by the remote end.

RETURNS
    void
*/
void connectionHandleL2capDisconnectInd(const L2CA_DISCONNECT_IND_T *ind)
{
    if (ind->con_ctx)
    {
        MAKE_CL_MESSAGE(CL_L2CAP_DISCONNECT_IND);
        message->identifier = ind->identifier;
        message->status = convertDisconnectResult(ind->reason);
        message->sink = StreamL2capSink(ind->cid);
        
        MessageSend(
            (Task)ind->con_ctx, 
            CL_L2CAP_DISCONNECT_IND, 
            message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("L2CA_DISCONNECT_IND con_ctx is 0x0\n"));
    }
#endif
}


/****************************************************************************
NAME    
    connectionHandleL2capDisconnectRes

DESCRIPTION
    Response by the local device to a L2CA_DISCONNECT_IND.

RETURNS
    void
*/
void connectionHandleL2capDisconnectRes(const CL_INTERNAL_L2CAP_DISCONNECT_RSP_T* rsp)
{
    /* There can be cross-over cases where the sink has already been closed by
    ** a disconnect req from this device 
    */
    if (rsp->sink)
    {
        l2ca_cid_t get_cid_val = (l2ca_cid_t)SinkGetL2capCid(rsp->sink);

        if (get_cid_val)
        {
            MAKE_PRIM_T(L2CA_DISCONNECT_RSP);
            prim->identifier = rsp->identifier;
            prim->cid = get_cid_val;
            VmSendL2capPrim(prim);
        }
    }
}


/****************************************************************************
NAME    
    connectionHandleQosSetupCfm

DESCRIPTION
    Confirmation of QOS parameters.  There is an interoperability issue with
    some older baseband chipsets.  This chipset will set the latency to 14
    slots (8.75mS).  This is not appropriate for piconet/scatternet operation as
    this will starve links of bandwidth. Therefore if this condition is
    detected then the latency is set back to a preferred default value

RETURNS
    void
*/
void connectionHandleQosSetupCfm(const DM_HCI_QOS_SETUP_CFM_T* cfm)
{
#define MINIMUM_LATENCY ((uint32)8750)
#define PREFERRED_LATENCY   ((uint32)25000)
    
    if(cfm->latency < MINIMUM_LATENCY)
    {
        MAKE_PRIM_C(DM_HCI_QOS_SETUP_REQ);
        prim->bd_addr = cfm->bd_addr;
        prim->service_type = cfm->service_type;
        prim->token_rate = cfm->token_rate;
        prim->peak_bandwidth = cfm->peak_bandwidth;
        prim->latency = PREFERRED_LATENCY;
        prim->delay_variation = cfm->delay_variation;       
        VmSendDmPrim(prim);
    }
}


/****************************************************************************
NAME    
    connectionHandleL2capTimeoutInd

DESCRIPTION
    Time out can occur during connection or disconnection transactions. 

RETURNS
    void
*/
void connectionHandleL2capTimeout(const L2CA_TIMEOUT_IND_T *ind)
{
    Task    appTask = 0;
    
    if (ind->con_ctx)
        appTask = (Task)ind->con_ctx;
    else if (ind->reg_ctx)
        appTask = (Task)ind->reg_ctx;
    
#ifdef CONNECTION_DEBUG_LIB
    else
        CL_DEBUG(("L2CA_TIMEOUT_IND reg_ctx and con_ctx are 0x0\n"));
#endif

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_L2CAP_TIMEOUT_IND);
        /* This could be 0 if no Sink has yet been established */
        message->sink = StreamL2capSink(ind->cid);
        MessageSend(
            appTask, 
            CL_L2CAP_TIMEOUT_IND, 
            message);
    }
}

/****************************************************************************
NAME    
    sendL2capMapFixedCidReq

DESCRIPTION
    Send the actual L2CA_MAP_FIXED_CID_REQ Prim. Set the state to the App Task,
    regardless of what it was before.

RETURNS
    void
*/
static void sendL2capMapFixedCidReq(connectionL2capState *state, const CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T* req)
{
    MAKE_PRIM_T(L2CA_MAP_FIXED_CID_REQ);

    /* Set the Lock to be the same as the App Task.*/
    state->mapLock = req->theAppTask;

    prim->addrt.type = TBDADDR_PUBLIC;
    BdaddrConvertVmToBluestack(&prim->addrt.addr, &req->bd_addr);

    prim->fixed_cid = L2CA_CID_CONNECTIONLESS;
    prim->cl_local_psm = req->psm_local;
    prim->cl_remote_psm = req->psm_remote;
    prim->con_ctx = CTX_FROM_TASK(req->theAppTask);
    
    if (req->type == l2cap_connectionless_data_broadcast)
        prim->flags = L2CA_CONFLAG_BROADCAST;
    else
        prim->flags = L2CA_CONFLAG_LOCK_ACL;

    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleL2capMapConnectionlessReq

DESCRIPTION
    Map connectionless data to a remote device; Broadcast or Unicast. For 
    Unicast, nail up the ACL, but don't for Broadcast.

RETURNS
    void
*/
void connectionHandleL2capMapConnectionlessReq(connectionL2capState *state, const CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T* req)
{
    /* If the lock for this scenario is free, then proceed. */
    if (!state->mapLock)
    {
        /* If mapping Unicast, check that it is supported first. */
        if (req->type == l2cap_connectionless_data_unicast)
        {
            BD_ADDR_T bd_addr;
            CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T *message =
                PanicUnlessNew(CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T);
            COPY_CL_MESSAGE(req, message);

            /* Stuff the req into the Lock, so it can be used in the next stage. */
            state->mapLock = (Task)message;

            BdaddrConvertVmToBluestack(&bd_addr, &req->bd_addr);

            /* Send an L2CA_GETINFO_REQ */
            {
                MAKE_PRIM_T(L2CA_GETINFO_REQ);
                prim->phandle = 0;
                prim->bd_addr = bd_addr;
                prim->info_type = L2CA_GETINFO_TYPE_EXT_FEAT;
                prim->req_ctx = CTX_FROM_TASK(req->theAppTask);
                prim->flags = 0;
                VmSendL2capPrim(prim);
            }
        }
        else
        {
            sendL2capMapFixedCidReq(state, req);
        }
    }
    /* Stick the message back in the task queue conditionalised on the lock.
    */
    else
    {
        MAKE_CL_MESSAGE(CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ);
        COPY_CL_MESSAGE(req, message);
        MessageSendConditionallyOnTask(
            connectionGetCmTask(),
            CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ,
            message,
            &state->mapLock
            );
    }
}

/****************************************************************************
NAME    
    sendL2capMapConnectionlessCfm

DESCRIPTION
    Send the CL_L2CAP_MAP_CONNECTIONLESS_CFM message to the App Task 
    indicating the outcome of the request.

RETURNS
    void
*/
static void sendL2capMapConnectionlessCfm(Task theAppTask, l2cap_map_connectionless_status status, uint16 local_psm, Sink sink, const bdaddr *addr, uint16 fixed_cid)
{
    MAKE_CL_MESSAGE(CL_L2CAP_MAP_CONNECTIONLESS_CFM);
    message->status = status;

    if (addr)
        message->addr = *addr;
    
    message->fixed_cid = fixed_cid;
    message->local_psm = local_psm;
    message->sink = sink;
    MessageSend(theAppTask, CL_L2CAP_MAP_CONNECTIONLESS_CFM, message);
}


/****************************************************************************
NAME    
    connectionHandleL2capGetInfoCfm

DESCRIPTION
    Part of the Map Connectionless Data message scenario. Response indicating
    if the remote device supports Connectionless Data.

RETURNS
    void
*/
void connectionHandleL2capGetInfoCfm(connectionL2capState *state, const L2CA_GETINFO_CFM_T* cfm)
{
    if (state->mapLock)
    {
        CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T* req = 
            (CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_REQ_T *)state->mapLock;
        
        if (
            cfm->result     == L2CA_GETINFO_SUCCESS && 
            cfm->info_type  == L2CA_GETINFO_TYPE_EXT_FEAT
           )
        {
            uint8* info_data = VmGetPointerFromHandle(cfm->info_data);
            uint16 ext_feat = (uint16)(info_data[1]<<8) | (uint16)info_data[0];
            free(info_data);

            /* Is Unicast Connectionless Data supported by the Remote device! */
            if (ext_feat & L2CA_EXT_FEAT_UCD_RECEPTION)
            {
                sendL2capMapFixedCidReq(state, req );
            }
            else
            {
                sendL2capMapConnectionlessCfm(
                    req->theAppTask,
                    l2cap_map_connectionless_unicast_unsupported,
                    req->psm_local,
                    0,                              /* Sink */
                    &req->bd_addr,
                    0                               /* Fixed CID */
                    );
                
                /* Reset the state. */
                state->mapLock = 0;
            }
        }
        else
        {
            sendL2capMapConnectionlessCfm(
                req->theAppTask,
                l2cap_map_connectionless_failed,
                req->psm_local,
                0,                              /* Sink */
                &req->bd_addr,
                0                               /* Fixed CID */
                );
            
            /* Reset the state. */
            state->mapLock = 0;
        }
        free(req);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("L2CA_GETINFO_CFM received out of state!\n"));
    }
#endif
}

/****************************************************************************
NAME    
    connectionHandleL2capMapFixedCidCfm

DESCRIPTION
    Handle the reception of the L2CA_MAP_FIXED_CID_CFM primitive.

RETURNS
    void
*/
void connectionHandleL2capMapFixedCidCfm(connectionL2capState *state, const L2CA_MAP_FIXED_CID_CFM_T* cfm)
{
    if (state->mapLock)
    {
        /* The inital state may be initiating, ignore that state. */
        if (cfm->result != L2CA_MISC_INITIATING)
        {
            Sink sink = 0;
            bdaddr addr;
            Task theAppTask = (Task)PanicZero(cfm->con_ctx);

            if (cfm->result == L2CA_MISC_SUCCESS)
            {
                sink = PanicNull(StreamL2capSink(cfm->cid));

                /* Associate the Sink to the Task */
                MessageStreamTaskFromSink(sink, theAppTask);
            }
            
            BdaddrConvertBluestackToVm(&addr, &cfm->addrt.addr);

            sendL2capMapConnectionlessCfm(
                theAppTask,
                connectionConvertL2capMapFixedCidResult(cfm->result),
                cfm->cl_local_psm,
                sink,
                &addr,
                cfm->fixed_cid
                );

            /* Reset the state. */
            state->mapLock = 0;
        }
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("L2CA_MAP_FIXED_CID_CFM received out of state!\n"));
    }
#endif    
}


/****************************************************************************
NAME    
    connectionHandleL2capMapFixedCidInd

DESCRIPTION
    Handle the reception of the L2CA_MAP_FIXED_CID_IND primitive. T

RETURNS
    void
*/
void connectionHandleL2capMapFixedCidInd(const L2CA_MAP_FIXED_CID_IND_T* ind)
{
    if (ind->reg_ctx)
    {
        Source source = (Source) PanicNull(StreamL2capSource(ind->cid));
        MAKE_CL_MESSAGE(CL_L2CAP_MAP_CONNECTIONLESS_IND);

        BdaddrConvertBluestackToVm(&message->addr, &ind->addrt.addr);

        /* Configure the Source to hold data until the app has responded. */
        SourceConfigure(source, VM_SOURCE_MESSAGES, VM_MESSAGES_NONE);

        message->fixed_cid = ind->fixed_cid;
        message->local_psm = ind->cl_local_psm;
        message->source = source;
        if (ind->flags & L2CA_CONFLAG_BROADCAST)
            message->type = l2cap_connectionless_data_broadcast;
        else
            message->type = l2cap_connectionless_data_unicast;
        
        MessageSend(
            (Task)ind->reg_ctx, 
            CL_L2CAP_MAP_CONNECTIONLESS_IND, 
            message
            );
    }
}

/****************************************************************************
NAME    
    connectionHandleL2capMapConnectionlessRsp

DESCRIPTION
    Send the L2CA_MAP_FIXED_CID_RES, setting the app task for the scenario.

RETURNS
    void
*/
void connectionHandleL2capMapConnectionlessRsp(const CL_INTERNAL_L2CAP_MAP_CONNECTIONLESS_RES_T *res)
{
    Sink sink = StreamSinkFromSource(res->source);
    MAKE_PRIM_T(L2CA_MAP_FIXED_CID_RSP);

    /* Associate the Stream with the task */
    MessageStreamTaskFromSink(sink, res->theAppTask);
    /* Let the data through. */
    SourceConfigure(res->source, VM_SOURCE_MESSAGES, VM_MESSAGES_ALL);

    prim->cid = SinkGetL2capCid(sink);
    prim->ucd_remote_psm = 0;
    prim->con_ctx = CTX_FROM_TASK(res->theAppTask);

    /* If this is a Unicast data Source then nail up 
    ** the connection using the LOCK ACL flag. The ACL will stay up until 
    ** the source is unmapped.
    */
    if (res->type == l2cap_connectionless_data_unicast)
        prim->flags = L2CA_CONFLAG_LOCK_ACL;

    VmSendL2capPrim(prim);    
}

/****************************************************************************
NAME    
    connectionHandleL2capUnmapConnectionlessReq

DESCRIPTION
    Send the L2CA_UNMAP_FIXED_CID_REQ, for the Connectionless data.

RETURNS
    void
*/
void connectionHandleL2capUnmapConnectionlessReq(const CL_INTERNAL_L2CAP_UNMAP_CONNECTIONLESS_REQ_T *req)
{
    MAKE_PRIM_T(L2CA_UNMAP_FIXED_CID_REQ);
    prim->cid = SinkGetL2capCid(req->sink);
    VmSendL2capPrim(prim);
}

/****************************************************************************
NAME    
    connectionHandleL2capUnmapFixedCidInd

DESCRIPTION
    Handle the reception of the L2CA_MAP_FIXED_CID_IND primitive. T

RETURNS
    void
*/
void connectionHandleL2capUnmapFixedCidInd(const L2CA_UNMAP_FIXED_CID_IND_T* ind)
{
    if (ind->con_ctx)
    {
        MAKE_CL_MESSAGE(CL_L2CAP_UNMAP_CONNECTIONLESS_IND);

        message->sink = StreamL2capSink(ind->cid);
        message->status = convertDisconnectResult(ind->reason);

        MessageSend(
            (Task)ind->con_ctx,
            CL_L2CAP_UNMAP_CONNECTIONLESS_IND,
            message);
    }
}

#endif /* !CL_EXCLUDE_L2CAP */

/* End-of-File */
