/****************************************************************************
Copyright (c) 2004 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    rfc_handler.c        

DESCRIPTION
	This file implements all the RFCOMM Connection Entity handler functions
    for upstream RFC messages from Bluestack and all downstream messages.
*/


/****************************************************************************
	Header files
*/
#include    "connection.h"
#include 	"common.h"
#include    "connection_private.h"
#include	"common.h"
#include 	"dm_link_policy_handler.h"
#include    "rfc_handler.h"

#include    <bdaddr.h>
#include    <print.h>
#include    <string.h>
#include    <vm.h>
#include    <stream.h>
#include    <sink.h>
#include    <source.h>
#include    <vmal.h>
#include <message.h>


#ifndef CL_EXCLUDE_RFCOMM

#define OPTIMAL_START_MTU   (884)
#define INVALID_RFCOMM_CONNECT_CHANNEL (0xFF)
#define MODEM_SIGNAL_MASK   (0xCE)

/*****************************************************************************/
static void convertRfcPortnegValuesToPortPar(port_par *out, const RFC_PORTNEG_VALUES_T *in)
{
    out->baud_rate      = in->baud_rate;
    out->data_bits      = in->data_bits;
    out->stop_bits      = in->stop_bits;
    out->parity         = in->parity;
    out->parity_type    = in->parity_type;
    out->flow_ctrl_mask = in->flow_ctrl_mask;
    out->xon            = in->xon;
    out->xoff           = in->xoff;
    out->parameter_mask = in->parameter_mask;
}

/*****************************************************************************/
static void convertPortParToRfcPortnegValues(RFC_PORTNEG_VALUES_T *out, const port_par *in)
{
    out->baud_rate      = in->baud_rate;
    out->data_bits      = in->data_bits;
    out->stop_bits      = in->stop_bits;
    out->parity         = in->parity;
    out->parity_type    = in->parity_type;
    out->flow_ctrl_mask = in->flow_ctrl_mask;
    out->xon            = in->xon;
    out->xoff           = in->xoff;
    out->parameter_mask = in->parameter_mask;
}

/*****************************************************************************
    The break signal is 0 to 3-seconds, encoded as a 0-15 range of 200ms 
    increments. It is encoded (according to GSM 07.10) as follows:
       - bit 0:   1-break signal encoded, 0-no break signal
       - bit 1-2: reserved
       - bit 3-6: range 0-15, break signal in 200ms increments.
******************************************************************************/
#define BREAK_SIGNAL_RANGE_MASK 0x0F

static uint8 encode_break_signal(uint8 break_signal)
{
    if (break_signal & BREAK_SIGNAL_RANGE_MASK)
        return (break_signal & BREAK_SIGNAL_RANGE_MASK) << 3 | 0x01;
    else
        return 0;
}

/*****************************************************************************
    RFCOMM Register (Sequential)

    Triggers:
        ConnectionRfcommAllocateChannel
        ConnectionRfcommAllocateChannelLazy

    Message Sequence:
        Connection -> BlueStack         RFC_REGISTER_REQ
        Bluestack -> Connection         RFC_REGISTER_CFM

    Response:
        CL_RFCOMM_REGISTER_CFM

******************************************************************************/
void connectionHandleRfcommRegisterReq(const CL_INTERNAL_RFCOMM_REGISTER_REQ_T *req)
{
            /*  
        Create an entry in the connection map to enable incoming primitives on this
        server channel to be mapped to the correct task 
            */
    MAKE_PRIM_T(RFC_REGISTER_REQ);
    prim->phandle           = 0;
    prim->context           = CTX_FROM_TASK( req->theAppTask);
    prim->flags             = 0;
    prim->loc_serv_chan_req = req->suggested_server_channel;
    VmSendRfcommPrim(prim);
}

/*****************************************************************************/
void connectionHandleRfcommRegisterCfm(const RFC_REGISTER_CFM_T *cfm)
{
    if (cfm->context)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_REGISTER_CFM);
       
        if(cfm->accept)
        {
            message->status         = success;
            message->server_channel = cfm->loc_serv_chan;
        }
        else
        {
            message->status         = fail;
            message->server_channel = RFC_INVALID_SERV_CHANNEL;
        }
        MessageSend((Task)cfm->context, CL_RFCOMM_REGISTER_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("RFC_REGISTER_CFM has context 0x0\n"));
    }
#endif       
}

/*****************************************************************************
    RFCOMM Unregister

    Triggers:
        ConnectionRfcommDeallocateChannel
   
    Message Sequence:
        Connection -> BlueStack         RFC_UNREGISTER_REQ
        Bluestack -> Connection         RFC_UNREGISTER_CFM

    Response:
        CL_RFCOMM_UNREGISTER_CFM

******************************************************************************/
void connectionHandleRfcommUnregisterReq(const CL_INTERNAL_RFCOMM_UNREGISTER_REQ_T* req)
{
    MAKE_PRIM_T(RFC_UNREGISTER_REQ);
    prim->loc_serv_chan     = req->local_server_channel;
    VmSendRfcommPrim(prim);
}

/*****************************************************************************/
void connectionHandleRfcommUnregisterCfm(const RFC_UNREGISTER_CFM_T *cfm)
{
    if (cfm->context)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_UNREGISTER_CFM);

        if (cfm->status == RFC_SUCCESS)
            message->status = success;
        else
            message->status = fail;

        message->server_channel = cfm->loc_serv_chan;
        
        MessageSend((Task)cfm->context, CL_RFCOMM_UNREGISTER_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        /* This can happen for some error cases */
        CL_DEBUG_INFO(("RFC_UNREGISTER_CFM has context 0x0\n"));
    }
#endif   

}

/*****************************************************************************
    RFCOMM Client Connect Request

    Triggers:
        ConnectionRfcommConnectRequest

    Message Sequence:
        Connection -> BlueStack         RFC_CLIENT_CONNECT_REQ
        Bluestack -> Connection         RFC_CLIENT_CONNECT_CFM

    Response:
        CL_RFCOMM_CONNECT_CFM

******************************************************************************/
static void sendRfcommConnectionCfm(Task task, rfcomm_connect_status status, uint8 channel, uint16 payload, Sink sink, bdaddr *addr)
{
    MAKE_CL_MESSAGE(CL_RFCOMM_CLIENT_CONNECT_CFM);
    message->status = status;
    message->server_channel = channel;
    message->payload_size = payload;
    message->sink = sink;
    if (addr)
        message->addr = *addr;
    MessageSend(task, CL_RFCOMM_CLIENT_CONNECT_CFM, message);
}

/*****************************************************************************/
void connectionHandleRfcommConnectReq(const CL_INTERNAL_RFCOMM_CONNECT_REQ_T* req)
{
    if (req->security_channel)
    {
        /* send a client connect request */
        {
            MAKE_PRIM_T(RFC_CLIENT_CONNECT_REQ);
            BdaddrConvertVmToBluestack(&(prim->bd_addr), &(req->bd_addr));
            prim->rem_serv_chan         = req->remote_server_channel;
            prim->context               = CTX_FROM_TASK(req->theAppTask);
            prim->client_security_chan  = req->security_channel;
            prim->max_payload_size      = req->config.max_payload_size;

            /* Modem Status parameters to be used during connection
             */
            prim->modem_signal  = req->config.modem_signal & MODEM_SIGNAL_MASK;
            prim->break_signal  = encode_break_signal(req->config.break_signal);
            prim->msc_timeout   = req->config.msc_timeout;

            /* The following parameters are for AMP or have default values for 
             * streams.
            */
            prim->phandle               = 0;
            prim->flags                 = 0;
            prim->priority              = 0;
            prim->total_credits         = 0;
            prim->remote_l2cap_control  = 0;
            prim->local_l2cap_control   = 0;
            prim->reserved_length       = 0;
            prim->reserved              = 0;

            VmSendRfcommPrim(prim);
        }
    }
    else
    {
        sendRfcommConnectionCfm(
            req->theAppTask, 
            rfcomm_connect_security_not_set, 
            RFC_INVALID_SERV_CHANNEL, 
            0, 0, 0
            );
    }
}

/*****************************************************************************/
void connectionHandleRfcommClientConnectCfm(const RFC_CLIENT_CONNECT_CFM_T* cfm)
{
    Task  appTask = (Task)cfm->context;
    
    if (appTask)
    {
        bdaddr                  addr;
        rfcomm_connect_status   status;
        Sink                    sink    = StreamRfcommSink(cfm->conn_id);;

        BdaddrConvertBluestackToVm(&addr, &(cfm->bd_addr));

        switch (cfm->status)
        {
            case RFC_CONNECTION_PENDING:

                /* Associate the task, passed back in the context field,
                   with the sink associated with the conn_id for this message.
                  */
                PanicNull( sink );      /* There MUST be a sink. */
                SourceConfigure( StreamSourceFromSink(sink), VM_SOURCE_MESSAGES, VM_MESSAGES_NONE);
                MessageStreamTaskFromSink(sink, appTask);

                status = rfcomm_connect_pending;
                break;
            case RFC_SUCCESS:
                PanicNull( sink );      /* There MUST be a sink. */
                SourceConfigure(StreamSourceFromSink(sink), VM_SOURCE_MESSAGES, VM_MESSAGES_ALL);
                status = rfcomm_connect_success;
                break;
            case RFC_CHANNEL_ALREADY_EXISTS:
                status = rfcomm_connect_channel_already_open;
                break;
            case RFC_REMOTE_REFUSAL:
                status = rfcomm_connect_rejected;
                break;
            case RFC_CONNECTION_REJ_SECURITY:
                status = rfcomm_connect_rejected_security;
                break;
            case RFC_RES_ACK_TIMEOUT:
                status = rfcomm_connect_res_ack_timeout;
                break;
            case RFC_INCONSISTENT_PARAMETERS:
                status = rfcomm_connect_inconsistent_parameters;
                break;
            default:
                if (cfm->status < RFCOMM_ERRORCODE_BASE)
                    status = connectionConvertL2capConnectStatus(cfm->status);
                else
                    status = rfcomm_connect_failed;
                break;
        }

        sendRfcommConnectionCfm(
            appTask,
            status,
            cfm->serv_chan,
            cfm->max_payload_size,
            sink,
            &addr
            );
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("RFC_CLIENT_CONNECT_CFM has context 0x0\n"));
    }
#endif       
}

/*****************************************************************************
    RFCOMM Server Connect Indication
				
    Triggers:
        RFC_SERVER_CONNECT_IND

    Message Sequence:
        Bluestack -> Connection         RFC_SERVER_CONNECT_IND
        Connection -> BlueStack         RFC_SERVER_CONNECT_RSP
        BlueStack -> Connection         RFC_SERVER_CONNECT_CFM

    Response:
        CL_RFCOMM_CONNECT_CFM

    NOTES: 
        RFCOMM stream is not active until the CFM message.

******************************************************************************/
void connectionHandleRfcommConnectInd(const RFC_SERVER_CONNECT_IND_T *ind)
{
    Task  appTask = (Task)ind->context;

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_CONNECT_IND);

        BdaddrConvertBluestackToVm(&(message->bd_addr), &(ind->bd_addr));

        /* Get the sink from the conn_id. */
        message->sink = (Sink) PanicNull( StreamRfcommSink(ind->conn_id) );

        message->server_channel = ind->loc_serv_chan;

        MessageSend(appTask, CL_RFCOMM_CONNECT_IND, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("RFC_CLIENT_CONNECT_IND has context 0x0\n"));
    }
#endif    
}

/*****************************************************************************/
void connectionHandleRfcommConnectRes(const CL_INTERNAL_RFCOMM_CONNECT_RES_T* res)
{

    MAKE_PRIM_T(RFC_SERVER_CONNECT_RSP);
    prim->conn_id = PanicZero(SinkGetRfcommConnId(res->sink));
     SourceConfigure( StreamSourceFromSink(res->sink), VM_SOURCE_MESSAGES, VM_MESSAGES_NONE);
     MessageStreamTaskFromSink(res->sink, res->theAppTask);
  

    if (res->response)
    {
        prim->response = RFC_ACCEPT_SERVER_CONNECTION;
    }
    else
    {
        prim->response = RFC_DECLINE_SERVER_CONNECTION;
    } 
    prim->max_payload_size      = res->config.max_payload_size;

    /* Modem Status parameters to be used during connection
     */
    prim->modem_signal  = res->config.modem_signal & MODEM_SIGNAL_MASK;
    prim->break_signal  = encode_break_signal(res->config.break_signal);
    prim->msc_timeout   = res->config.msc_timeout;

    /* The following parameters are for AMP or have default values for 
     * streams.
     */
    prim->flags = 0;
    prim->priority = 0;
    prim->total_credits = 0;
    prim->remote_l2cap_control = 0;
    prim->local_l2cap_control = 0;
    VmSendRfcommPrim(prim);
}

/*****************************************************************************/
void connectionHandleRfcommServerConnectCfm(const RFC_SERVER_CONNECT_CFM_T* cfm)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(cfm->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    rfcomm_connect_status status;

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_SERVER_CONNECT_CFM);

        switch (cfm->status)
        {   
            case RFC_SUCCESS:
                status = rfcomm_connect_success;
                SourceConfigure(StreamSourceFromSink(sink), VM_SOURCE_MESSAGES, VM_MESSAGES_ALL);
                break;
            case RFC_INVALID_CHANNEL:
                status = rfcomm_connect_channel_not_registered;
                break;
            case RFC_DECLINE_SERVER_CONNECTION:
                status = rfcomm_connect_declined;
                break;
            case RFC_CONNECTION_REJ_SECURITY:
                status = rfcomm_connect_rejected_security;
                break;
            case RFC_INCONSISTENT_PARAMETERS:
                status = rfcomm_connect_inconsistent_parameters;
                break;
            default:
                if (cfm->status < RFCOMM_ERRORCODE_BASE)
                    status = connectionConvertL2capConnectStatus(cfm->status);
                    else
                    status = rfcomm_connect_failed;
                break;
        }

        message->status = status;
        message->server_channel = cfm->serv_chan;
        message->payload_size = cfm->max_payload_size;
        message->sink = sink;
        BdaddrConvertBluestackToVm(&(message->addr), &(cfm->bd_addr));
        MessageSend(appTask, CL_RFCOMM_SERVER_CONNECT_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Invalid context returned in RFC_SERVER_CONNECT_CFM\n"));
    }
#endif       
}


/*****************************************************************************
    RFCOMM Disconnect Req

    Triggers:
        ConnectionRfcommDisconnectRequest

    Message Sequence:
        Connection -> BlueStack         RFC_DISCONNECT_REQ
        Bluestack -> Connection         RFC_DISCONNECT_CFM
        BlueStack -> Connection         RFC_DISCONNECT_IND
        Connection -> BlueStack         RFC_DISCONNECT_RSP  (stream destroyed)

    Response:
        CL_RFCOMM_DISCONNECT_CFM

******************************************************************************/
static void sendRfcommDisconnectCfm(Task appTask, Sink sink, rfcomm_disconnect_status status)
{
    MAKE_CL_MESSAGE(CL_RFCOMM_DISCONNECT_CFM);
    message->status = status;
    message->sink = sink;
    MessageSend(appTask, CL_RFCOMM_DISCONNECT_CFM, message);
}

/*****************************************************************************/
void connectionHandleRfcommDisconnectReq(const CL_INTERNAL_RFCOMM_DISCONNECT_REQ_T* req)
{
    uint16 conn_id = SinkGetRfcommConnId(req->sink);
    
    if (req->sink && conn_id)
    {
        MAKE_PRIM_T(RFC_DISCONNECT_REQ);
        prim->conn_id = conn_id;
        VmSendRfcommPrim(prim);
    }
    else
    {
        sendRfcommDisconnectCfm(
            req->theAppTask,
            req->sink,
            rfcomm_disconnect_unknown_sink
            );
    }
}

/*****************************************************************************/
void connectionHandleRfcommDisconnectCfm(const RFC_DISCONNECT_CFM_T *cfm)
{
    /* If there is a cross-over between an RFC_DISCONNECT_IND from BlueStack
       and an RFC_DISCONNECT_REQ from the VM, then the RFC_DISCONNECT_CFM can
       come back indicating an Invalid Channel. In this case, swallow the
       CFM.
    */
    if (cfm->status != RFC_INVALID_CHANNEL)
    {
        /* Get the Sink from the Conn_id and then the app task from the Sink. */
        Sink sink       = (Sink) PanicNull( StreamRfcommSink(cfm->conn_id) );
        Task appTask    = MessageStreamGetTaskFromSink(sink);

        if (appTask)
        {
            sendRfcommDisconnectCfm(
                appTask ,
                sink,
                connectionConvertRfcommDisconnectStatus(cfm->status)
                );
        }
#ifdef CONNECTION_DEBUG_LIB
        else
        {
            CL_DEBUG(("conn_id in RFC_DISCONNECT_CFM is not mapped to a sink/task\n"));
        }
#endif
    }
}

/*****************************************************************************
    RFCOMM Disconnect Ind

    Triggers:
        RFC_DISCONNECT_IND
                    
    Message Sequence:
        Bluestack -> Connection         RFC_DISCONNECT_IND
        Connection -> BlueStack         RFC_DISCONNECT_RSP (stream destroyed)
                    
    Response:
        ConnectionRfcommDisconnectResponse
                    
******************************************************************************/
void connectionHandleRfcommDisconnectInd(const RFC_DISCONNECT_IND_T *ind)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(ind->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        /* Send the disconnect indication to the application */
        MAKE_CL_MESSAGE(CL_RFCOMM_DISCONNECT_IND);
        message->sink = sink;
        message->status = connectionConvertRfcommDisconnectStatus(ind->reason);
        MessageSend(appTask, CL_RFCOMM_DISCONNECT_IND, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("conn_id in RFC_DISCONNECT_IND is not mapped to a sink/task \n"));
    }
#endif
}

/*****************************************************************************/
void connectionHandleRfcommDisconnectRsp(const CL_INTERNAL_RFCOMM_DISCONNECT_RSP_T *rsp)
{
    uint16 conn_id = SinkGetRfcommConnId(rsp->sink);
    /* The conn_id could be 0 if there is an RFCOMM disconnect cross-over, in
     * which case sending the response is redundant.
     */
    if (conn_id)
    {
        MAKE_PRIM_T(RFC_DISCONNECT_RSP);
        prim->conn_id = conn_id;
        VmSendRfcommPrim(prim);
    }
}
	
/*****************************************************************************
    RFCOMM Remote Port Negotiation Required

    Triggers:
        ConnectionRfcommPortNegRequest

    Message Sequence:
        Connection -> BlueStack         RFC_PORTNEG_REQ
        Bluestack -> Connection         RFC_PORTNEG_CFM
    
    Response:
        CL_RFCOMM_PORTNEG_CFM

******************************************************************************/
void connectionHandleRfcommPortNegReq(const CL_INTERNAL_RFCOMM_PORTNEG_REQ_T* req)
{
    int16 conn_id = (uint16) PanicZero( SinkGetRfcommConnId(req->sink) );

    MAKE_PRIM_T(RFC_PORTNEG_REQ);
    prim->conn_id = conn_id;
    prim->request = req->request;
    convertPortParToRfcPortnegValues(&prim->port_pars, &req->port_params);
        VmSendRfcommPrim(prim);
}

/*****************************************************************************/
void connectionHandleRfcommPortNegCfm(const RFC_PORTNEG_CFM_T* cfm)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(cfm->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_PORTNEG_CFM);

        if (cfm->status == RFC_SUCCESS)
            message->status = rfcomm_portneg_success;
            else
            message->status = rfcomm_portneg_failed;
        
        message->sink = sink;

        convertRfcPortnegValuesToPortPar(&message->port_params, &cfm->port_pars);
        MessageSend(appTask, CL_RFCOMM_PORTNEG_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("conn_id in RFC_PORTNEG_CFM is not mapped to a Sink/Task\n"));
    }
#endif
}

/*****************************************************************************
    RFCOMM Remote Port Negotiation Indication

    Triggers:
        RFC_PORTNEG_IND

    Message Sequence:
        Bluestack -> Connection         RFC_PORTNEG_IND
        Connection -> BlueStack         RFC_PORTNEG_RSP

    Response:
        ConnectionRfcommPortNegResponse

******************************************************************************/
void connectionHandleRfcommPortNegInd(const RFC_PORTNEG_IND_T* ind)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(ind->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_PORTNEG_IND);
        message->sink = sink;
        message->request = ind->request;
        convertRfcPortnegValuesToPortPar(&message->port_params, &ind->port_pars);
        MessageSend(appTask, CL_RFCOMM_PORTNEG_IND, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("conn_id in RFC_PORTNEG_IND is not mapped to a Sink/task\n"));
    }
#endif
}

/*****************************************************************************/
void connectionHandleRfcommPortNegRsp(const CL_INTERNAL_RFCOMM_PORTNEG_RSP_T* rsp)
{
    int16 conn_id = (uint16) PanicZero( SinkGetRfcommConnId(rsp->sink) );
    MAKE_PRIM_T(RFC_PORTNEG_RSP);
    prim->conn_id = conn_id;
    convertPortParToRfcPortnegValues(&prim->port_pars, &rsp->port_params);
    VmSendRfcommPrim(prim);
}

/*****************************************************************************
    RFCOMM Modem Status Required

    Triggers:
        ConnectionRfcommControlSignalRequest
                
    Message Sequence:
        Connection -> BlueStack         RFC_MODEM_STATUS_REQ
        Bluestack -> Connection         RFC_MODEM_STATUS_CFM

    Response:
        CL_RFCOMM_CONTROL_CFM

******************************************************************************/
void connectionHandleRfcommControlReq(const CL_INTERNAL_RFCOMM_CONTROL_REQ_T* req)
{
    uint16 conn_id = (uint16) PanicZero( SinkGetRfcommConnId(req->sink) );

    MAKE_PRIM_T(RFC_MODEM_STATUS_REQ);
    prim->conn_id      = conn_id;
    prim->modem_signal = req->modem_signal & MODEM_SIGNAL_MASK;
    prim->break_signal = encode_break_signal(req->break_signal);
    VmSendRfcommPrim(prim);
}

/*****************************************************************************/
void connectionHandleRfcommControlCfm(const RFC_MODEM_STATUS_CFM_T* cfm)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(cfm->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_CONTROL_CFM);
        message->sink = sink;
        switch( cfm->status )
        {
            case RFC_SUCCESS:
                message->status = rfcomm_control_success;
                break;
            case RFC_INCONSISTENT_PARAMETERS:
                message->status = rfcomm_control_invalid_modem_signal;
                break;
            default:
                message->status = rfcomm_control_failed;
                break;
        }
        MessageSend(appTask, CL_RFCOMM_CONTROL_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("conn_id in RFC_MODEM_STATUS_CFM is not mapped to a sink/task\n"));
    }
#endif
}

/*****************************************************************************
    RFCOMM Modem Status Indication

    Triggers:
        RFC_MODEM_STATUS_IND

    Message Sequence:
        Bluestack -> Connection         RFC_MODEM_STATUS_IND

    Response:
        None

******************************************************************************/
void connectionHandleRfcommControlInd(const RFC_MODEM_STATUS_IND_T* ind)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(ind->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_CONTROL_IND);
        message->sink         = sink;
        message->modem_signal = ind->modem_signal;

        /* Decode the break_signal back into range 0-15. */
        if (ind->break_signal)
            message->break_signal = 
                ind->break_signal >> 3 & BREAK_SIGNAL_RANGE_MASK;
        else
            message->break_signal = 0;
        
        MessageSend(appTask, CL_RFCOMM_CONTROL_IND, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Context in RFC_MODEM_STATUS_IND is not mapped to a sink/task\n"));
    }
#endif   
}
        
/*****************************************************************************
    RFCOMM Line Status Required

    Triggers:
        ConnectionRfcommLineStatusRequest

    Message Sequence:
        Connection -> BlueStack         RFC_LINESTATUS_REQ
        Bluestack -> Connection         RFC_LINESTATUS_CFM
      
    Response:
        CL_RFCOMM_LINESTATUS_CFM

******************************************************************************/
void connectionHandleRfcommLineStatusReq(const CL_INTERNAL_RFCOMM_LINE_STATUS_REQ_T* req)
{
    uint16 conn_id = (uint16) PanicZero( SinkGetRfcommConnId(req->sink) );
    MAKE_PRIM_T(RFC_LINESTATUS_REQ);
    prim->conn_id      = conn_id;
    prim->error_flag   = (uint8) req->error;
    prim->line_status  = (uint8) req->lines_status;
    VmSendRfcommPrim(prim);
}

/*****************************************************************************/
void connectionHandleRfcommLineStatusCfm(const RFC_LINESTATUS_CFM_T* cfm)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(cfm->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_LINE_STATUS_CFM);
        message->sink = sink;
        switch( cfm->status )
        {
            case RFC_SUCCESS:
                message->status = rfcomm_line_status_success;
                break;
            case RFC_INCONSISTENT_PARAMETERS:
                message->status = rfcomm_line_status_invalid_error;
                break;
            default:
                message->status = rfcomm_line_status_failed;
                break;
        }
        MessageSend(appTask, CL_RFCOMM_LINE_STATUS_CFM, message);
    }
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("conn_id in RFC_LINESTATUS_CFM is not mapped to a sink/task\n"));
    }
#endif
}


/*****************************************************************************
    RFCOMM Line Status Indication
	
    Triggers:
        RFC_LINESTATUS_IND

    Message Sequence:
        Bluestack -> Connection         RFC_LINESTATUS_IND

    Response:
        None

******************************************************************************/
void connectionHandleRfcommLineStatusInd(const RFC_LINESTATUS_IND_T* ind)
{
    /* Get the Sink from the Conn_id and then the app task from the Sink. */
    Sink sink       = (Sink) PanicNull( StreamRfcommSink(ind->conn_id) );
    Task appTask    = MessageStreamGetTaskFromSink(sink);

    if (appTask)
    {
        MAKE_CL_MESSAGE(CL_RFCOMM_LINE_STATUS_IND);
        message->sink           = sink;
        message->error          = (bool) ind->error_flag;
        message->line_status    = (rfcomm_line_status_error) ind->line_status;
        MessageSend(appTask, CL_RFCOMM_LINE_STATUS_IND, message);
    }       
#ifdef CONNECTION_DEBUG_LIB
    else
    {
        CL_DEBUG(("Context in RFC_LINESTATUS_IND is not mapped to a Sink/task\n"));
    }
#endif   
}



#endif
