/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    transport_manager_handler.c

DESCRIPTION
    Message handler for transport manager
*/
#include <stdlib.h>
#include <print.h>
#include <vmal.h>
#include <stream.h>
#include <sink.h>
#include <source.h>
#include<connection_no_ble.h>
#include <transport_adaptation.h>
#include <vmtypes.h>
#include "transport_manager_private.h"

/* Utility */
static transport_type_t transportMgrConvertManagerType(transport_mgr_type_t trans_type);
static transport_mgr_type_t transportMgrConvertAdaptationType(transport_type_t trans_type);

/*static handlers*/
static void transportMgrHandleRegisterCfm( TRANSPORT_REGISTER_CFM_T * message);
static void transportMgrHandleConnectCfm(TRANSPORT_CONNECT_CFM_T * msg);
static void transportMgrHandleDisconnectCfm(TRANSPORT_DISCONNECT_CFM_T * msg);
static void transportMgrHandleDeregisterCfm(TRANSPORT_DEREGISTER_CFM_T * msg);
static void transportMgrHandleGattMoreData(TRANSPORT_MESSAGE_MORE_DATA_T * msg);
static void transportMgrHandleMoreData(MessageMoreData * msg);
static void transportMgrHandleMoreSpace(MessageMoreSpace * msg);

/* Internal message handlers */
static void transportMgrHandleInternalRegisterReq(TRANSPORT_MGR_INTERNAL_REGISTER_REQ_T *msg);
static void transportMgrHandleInternalDeRegisterReq(TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ_T *msg);
static void transportMgrHandleInternalConnectReq(TRANSPORT_MGR_INTERNAL_CONNECT_REQ_T *msg);
static void transportMgrHandleInternalDisConnectReq(TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ_T *msg);

/******************************************************************************/
static transport_type_t transportMgrConvertManagerType(transport_mgr_type_t trans_type)
{
#define CASE(from, to) case (from): if((from) == (to)) goto coerce; else return (to);
    switch(trans_type)
    {
        CASE(transport_mgr_type_rfcomm,TRANSPORT_RFCOMM);
        CASE(transport_mgr_type_accessory,TRANSPORT_ACCESSORY);
        coerce: return (transport_type_t) trans_type;
        default: return 0xff;
    }
#undef CASE
}

/******************************************************************************/
static transport_mgr_type_t transportMgrConvertAdaptationType(transport_type_t trans_type)
{
#define CASE(from, to) case (from): if((from) == (to)) goto coerce; else return (to);
    switch(trans_type)
    {
        CASE(TRANSPORT_RFCOMM,transport_mgr_type_rfcomm);
        CASE(TRANSPORT_ACCESSORY,transport_mgr_type_accessory);
        coerce: return (transport_mgr_type_t) trans_type;
        default: return 0xff;
    }
#undef CASE
}

/***************************************************************************/
static void transportMgrHandleRegisterCfm(TRANSPORT_REGISTER_CFM_T * message)
{
    transport_mgr_link_data_t *trans_data = NULL;
    uint16 link_id_key = 0;
    PRINT(("transportMgrHandleRegisterCfm\n"));


    /* Get the trans link id  which is used as a key for searching the link instance from the data base. */
    link_id_key = ((transportMgrConvertAdaptationType(message->transport)) == transport_mgr_type_gatt) ? message->transport_id : INVALID_TRANS_LINK_ID;

    trans_data = transportMgrFindDataWithTransInfo(link_id_key,transportMgrConvertAdaptationType(message->transport));

    if(trans_data != NULL)
    {
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_REGISTER_CFM,msg);
        msg->status = message->status;
        if(trans_data->link_cfg->type != transport_mgr_type_gatt)
        {
            /* Update the trans link id to the data block*/
            trans_data->link_cfg->trans_info.non_gatt_trans.trans_link_id = message->transport_id;
        }
        /* Copy the data block to the confirmation message*/
        memcpy(&(msg->link_cfg),trans_data->link_cfg,sizeof(transport_mgr_link_cfg_t));
        /* Confirmation passed on to app task*/
        MessageSend(trans_data->application_task,TRANSPORT_MGR_REGISTER_CFM,msg);
        /* If the registration is failed we need to remove the reference from the list */
        if(message->status == FALSE)
        {
            transportMgrRemoveLinkRef(trans_data);
        }
    }
}

/***************************************************************************/
static void transportMgrHandleConnectCfm(TRANSPORT_CONNECT_CFM_T * msg)
{
    transport_mgr_link_data_t *trans_data = NULL;
    transport_mgr_type_t transport_type = transport_mgr_type_none;

    PRINT(("transportMgrHandleConnectCfm\n"));
    {
    MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_LINK_CREATED_CFM,message);
    message->status = msg->status;
    message->addr = msg->addr;
	
    transport_type = transportMgrConvertAdaptationType(msg->transport);
    if(transport_type != transport_mgr_type_gatt)
    {
        trans_data = transportMgrFindDataWithTransInfo(msg->transport_id,transport_type);
        /* Update the sink in the data block*/
        transportMgrUpdateSink(msg->transport_id, msg->sink,msg->transport);
        memcpy(&(message->link_cfg),trans_data->link_cfg,sizeof(transport_mgr_link_cfg_t));
        message->trans_sink = msg->sink;
    
        /* Associate Transport handler task with Transport Sink */
        if((msg->status) && SinkIsValid(msg->sink))
        {
            MessageStreamTaskFromSink(message->trans_sink, transportMgrGetTransportTask());
            /* Configure the stream to get all the related messages*/
            SourceConfigure(StreamSourceFromSink(message->trans_sink), VM_SOURCE_MESSAGES, VM_MESSAGES_ALL);
            SinkConfigure(message->trans_sink, VM_SINK_MESSAGES, VM_MESSAGES_ALL);
        }
        transportMgrListFindAndSendMessage(msg->transport_id,TRANSPORT_MGR_LINK_CREATED_CFM,message,transport_type);
    }
    else
    {
        UNUSED(trans_data);
        message->link_cfg.type = transport_type;
        /* When the transport type is GATT we would not know 
            about the start and end handle */
        transportMgrListBroadcastGattMessage(TRANSPORT_MGR_LINK_CREATED_CFM,message);
    }
    }

}

/***************************************************************************/
static void transportMgrHandleDisconnectCfm(TRANSPORT_DISCONNECT_CFM_T * msg)
{
    uint16 transport_link_id = 0;
    transport_mgr_link_data_t *trans_data = NULL;
    transport_mgr_type_t transport_type = transport_mgr_type_none;

    PRINT(("transportMgrHandleDisconnectCfm\n"));
    {
    MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_LINK_DISCONNECTED_CFM,message);
    transport_type = transportMgrConvertAdaptationType(msg->transport);
    message->status = msg->status;
    if(transport_type != transport_mgr_type_gatt)
    {
        transport_link_id = transportMgrFindTranslinkIdFromSink(msg->sink);
        trans_data = transportMgrFindDataWithTransInfo(transport_link_id,transportMgrConvertAdaptationType(msg->transport));
        message->trans_sink = msg->sink;
        transportMgrFlushInput(transport_link_id,transportMgrConvertAdaptationType(msg->transport));
        message->link_cfg = *trans_data->link_cfg;
        transportMgrListFindAndSendMessage(transport_link_id,TRANSPORT_MGR_LINK_DISCONNECTED_CFM,message,message->link_cfg.type);
    }
    else
    {
        UNUSED(trans_data);
        message->link_cfg.type = transport_type;
        /* When the transport type is GATT we would not know 
            about the start and end handle */
        transportMgrListBroadcastGattMessage(TRANSPORT_MGR_LINK_DISCONNECTED_CFM,message);
    }
    }
}

/***************************************************************************/
static void transportMgrHandleDeregisterCfm(TRANSPORT_DEREGISTER_CFM_T * msg)
{
    MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_DEREGISTER_CFM,message);

    PRINT(("transportMgrHandleDeregisterCfm\n"));

    /* Inform all the application regarding the deregistration*/
    message->status = msg->status;
    message->type = transportMgrConvertAdaptationType(msg->transport);
    message->trans_link_id = msg->transport_id;
    transportMgrListFindAndSendMessage(message->trans_link_id,TRANSPORT_MGR_DEREGISTER_CFM,message,message->type);
        	
    if(msg->status)
    {
        transport_mgr_link_data_t *temp = transportMgrFindDataWithTransInfo(msg->transport_id,transportMgrConvertAdaptationType(msg->transport));

        if (temp)
        {
            /* Remove all references to trans_link_id*/
            transportMgrRemoveLinkRef(temp);
        }
    }
}

/***************************************************************************/
static void add_data_ref(uint16 transport_id, uint16 size, uint8 *data)
{
    transport_mgr_data_t *trans_data = PanicNull(calloc(1, sizeof(transport_mgr_data_t) + size));
    trans_data->data_id = transport_id;
    trans_data->data_size = size;
    memcpy(trans_data->data, data, size);
    transportMgrAddRef((void *)trans_data, transport_mgr_data);
}

/***************************************************************************/
static void transportMgrHandleAccessoryMoreData(TRANSPORT_MESSAGE_MORE_DATA_T * msg)
{
    MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_MORE_DATA, m);
    
    add_data_ref(msg->handle, msg->data_len, msg->data);
    
    m->type = transport_mgr_type_accessory;
    m->trans_link_info = msg->handle;
    
    transportMgrListFindAndSendMessage(m->trans_link_info, TRANSPORT_MGR_MORE_DATA, m, m->type);
}


/***************************************************************************/
static void transportMgrHandleGattMoreData(TRANSPORT_MESSAGE_MORE_DATA_T * msg)
{
    gatt_status_t result_for_response = gatt_status_invalid_handle;

    PRINT(("transportMgrHandleGattMoreData\n"));

    /* Check whether the message recieved is with a valid handle*/
    if(transportMgrFindDataWithTransInfo(msg->handle,transport_mgr_type_gatt) != NULL)
    {
        result_for_response = gatt_status_success;
        if(transportMgrFindSizeOfDataList() < MAX_MESSAGE_MORE_DATA)
        {
            MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_MORE_DATA,message);
            /* Allocate memory for transport manager data block and add it to the list*/
            add_data_ref(msg->handle, msg->data_len, msg->data);

            message->type = transport_mgr_type_gatt;
            message->trans_link_info = msg->handle;
            transportMgrListFindAndSendMessage(msg->handle,TRANSPORT_MGR_MORE_DATA,message,transport_mgr_type_gatt);
        }
        /* Else just drop the MORE DATA message*/
    }

    /* Respond with access response in any case */
    TransportGattSendServerAccessRsp(msg->handle,result_for_response, 0, NULL);

}


/***************************************************************************/
static void transportMgrHandleMsgMoreData(TRANSPORT_MESSAGE_MORE_DATA_T * msg)
{
    switch (msg->transport)
    {
    case TRANSPORT_ACCESSORY:
        transportMgrHandleAccessoryMoreData(msg);
        break;
    
    case TRANSPORT_GATT:
        transportMgrHandleGattMoreData(msg);
        break;

    default:
        break;
    }
}


/***************************************************************************/
static void transportMgrHandleMoreData(MessageMoreData * msg)
{
    transport_mgr_link_data_t *transData = transportMgrFindTransDataFromSink(StreamSinkFromSource(msg->source));
    PRINT(("transportMgrHandleMoreData\n"));

    if(transData != NULL)
    {
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_MORE_DATA,m);
        m->trans_link_info = transData->link_cfg->trans_info.non_gatt_trans.trans_link_id;
        m->type          = transData->link_cfg->type;
        MessageSend(transData->application_task,TRANSPORT_MGR_MORE_DATA,m);
    }
}

/***************************************************************************/
static void transportMgrHandleMoreSpace(MessageMoreSpace * msg)
{
    transport_mgr_link_data_t *transData = transportMgrFindTransDataFromSink(msg->sink);

    PRINT(("transportMgrHandleMoreSpace\n"));

    if(transData != NULL)
    {
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_MORE_SPACE,m);
        m->trans_link_id = transData->link_cfg->trans_info.non_gatt_trans.trans_link_id;
        m->type          = transData->link_cfg->type;
        MessageSend(transData->application_task,TRANSPORT_MGR_MORE_SPACE,m);
    }
}

/***************************************************************************/
static void transportMgrHandleInternalRegisterReq(TRANSPORT_MGR_INTERNAL_REGISTER_REQ_T *message)
{
    transport_mgr_link_data_t *trans_data = NULL;
    bool retVal = FALSE;

    switch(message->link_cfg.type)
    {
        case transport_mgr_type_gatt:
            /* Registration procedure for GATT transport*/
            retVal = TransportGattRegisterReq(message->link_cfg.trans_info.gatt_trans.start_handle,
                                                            message->link_cfg.trans_info.gatt_trans.end_handle);
            break;
        case transport_mgr_type_rfcomm:
        case transport_mgr_type_accessory:
            /* Registration procedure for Non - GATT related*/
            retVal = TransportRegisterReq(transportMgrConvertManagerType(message->link_cfg.type), 
                                                        message->link_cfg.trans_info.non_gatt_trans.trans_link_id);
            break;
        default:
            break;
    }

    /* Register with the transport adaptation layer for further processing*/
    if (TRUE == retVal)
    {
        transportMgrSetBusy(BLOCKED);
        /* Allocate memory for transport manager block and add it to the list*/
        trans_data = (transport_mgr_link_data_t *)PanicNull(calloc(1, sizeof(transport_mgr_link_data_t)));
        trans_data->link_cfg = (transport_mgr_link_cfg_t *)PanicNull( calloc(1,  sizeof(transport_mgr_link_cfg_t) ) );
        trans_data->application_task = message->application_task;
        memcpy(trans_data->link_cfg,&(message->link_cfg),sizeof(transport_mgr_link_cfg_t));
        if(message->link_cfg.type != transport_mgr_type_gatt)
        {
            trans_data->link_cfg->trans_info.non_gatt_trans.trans_link_id= INVALID_TRANS_LINK_ID;
        }
        trans_data->next =NULL;
        transportMgrAddRef((void *)trans_data,transport_mgr_link_data);
    }
    else
    {
        /* Inform the app task about the status of the registration*/
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_REGISTER_CFM,msg);
        msg->status = transport_mgr_status_fail;
        memcpy(&(msg->link_cfg) , &(message->link_cfg),sizeof(transport_mgr_link_cfg_t));
        MessageSend(message->application_task,TRANSPORT_MGR_REGISTER_CFM,msg);
    }

}

/***************************************************************************/
static void transportMgrHandleInternalDeRegisterReq(TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ_T *message)
{
    if(!(TransportDeregisterReq(transportMgrConvertManagerType(message->type), message->trans_link_id) ))
    {
        /* Inform the app task about the status of the registration*/
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_DEREGISTER_CFM,msg);
        msg->status = transport_mgr_status_fail;
        msg->type = message->type;
        msg->trans_link_id = message->trans_link_id;
        MessageSend(message->application_task,TRANSPORT_MGR_DEREGISTER_CFM,msg);
    }
}

/***************************************************************************/
static void transportMgrHandleInternalConnectReq(TRANSPORT_MGR_INTERNAL_CONNECT_REQ_T *msg)
{
    transport_mgr_link_data_t *trans_data = NULL;

    if(!(TransportConnectReq(transportMgrConvertManagerType(msg->trans_type), msg->bd_addr,msg->link_id, msg->remote_link_id)))
    {
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_LINK_CREATED_CFM,message);
        trans_data = transportMgrFindDataWithTransInfo(msg->link_id, msg->trans_type);
        message->status = transport_mgr_status_fail;
        memcpy(&(message->link_cfg),trans_data->link_cfg,sizeof(transport_mgr_link_cfg_t));
        message->trans_sink = NULL;
        transportMgrListFindAndSendMessage(msg->link_id,TRANSPORT_MGR_LINK_CREATED_CFM,message,msg->trans_type);
    }
    UNUSED(trans_data);

}

/***************************************************************************/
static void transportMgrHandleInternalDisConnectReq(TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ_T *msg)
{
    transport_mgr_link_data_t *trans_data = NULL;
    if(!(TransportDisconnectReq(transportMgrConvertManagerType(msg->trans_type), msg->sink)))
    {
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_LINK_DISCONNECTED_CFM,message);
        trans_data = transportMgrFindDataWithTransInfo(transportMgrFindTranslinkIdFromSink(msg->sink), msg->trans_type);
        message->status = transport_mgr_status_fail;
        message->trans_sink= msg->sink;
        memcpy(&(message->link_cfg),trans_data->link_cfg,sizeof(transport_mgr_link_cfg_t));
        transportMgrListFindAndSendMessage(transportMgrFindTranslinkIdFromSink(msg->sink),TRANSPORT_MGR_LINK_DISCONNECTED_CFM,message,msg->trans_type);
    }
    UNUSED(trans_data);
}

/***************************************************************************
DESCRIPTION
    Message Handler for Transport Manager task
 
PARAMS
    Task -  Task associated with the message
    MessageId -  Identifier
    Message - Message itself
 
RETURNS
    
*/

void transportMgrMessageHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    PRINT(("transportMgrMessageHandler\n"));

    switch(id)
    {
        case MESSAGE_MORE_DATA:
        {
            MessageMoreData *m = (MessageMoreData *) message;
            PRINT(("TM:MESSAGE_MORE_DATA\n")); 
            transportMgrHandleMoreData(m);
        }
        break;

        case MESSAGE_MORE_SPACE:
        {
            MessageMoreSpace *m = (MessageMoreSpace *) message;
            PRINT(("TM:MESSAGE_MORE_SPACE\n"));
            transportMgrHandleMoreSpace(m);
        }
        break;

        case TRANSPORT_REGISTER_CFM:
        {
            PRINT(("TRANSPORT_REGISTER_CFM\n"));
            
            /* Release the synchronization lock */
            transportMgrSetBusy(UNBLOCKED);
            transportMgrHandleRegisterCfm((TRANSPORT_REGISTER_CFM_T *) message);
        }
        break;

        case TRANSPORT_DEREGISTER_CFM:
        {
            PRINT(("TRANSPORT_DEREGISTER_CFM\n"));
            transportMgrHandleDeregisterCfm((TRANSPORT_DEREGISTER_CFM_T *) message);
        }
        break;

        case TRANSPORT_CONNECT_CFM:
        {
            PRINT(("TRANSPORT_CONNECT_CFM\n"));
            transportMgrHandleConnectCfm((TRANSPORT_CONNECT_CFM_T *) message);
        }
        break;
        case TRANSPORT_DISCONNECT_CFM:
        {
            PRINT(("TRANSPORT_DISCONNECT_CFM\n"));
            transportMgrHandleDisconnectCfm((TRANSPORT_DISCONNECT_CFM_T *) message);
        }
        break;

        case TRANSPORT_MESSAGE_MORE_DATA:
        {
            PRINT(("TRANSPORT_MESSAGE_MORE_DATA_T\n"));
            transportMgrHandleMsgMoreData((TRANSPORT_MESSAGE_MORE_DATA_T *) message);
        }
        break;


        /* Handling all internal messages */

        case TRANSPORT_MGR_INTERNAL_REGISTER_REQ:
        {
            PRINT(("TRANSPORT_MGR_INTERNAL_REGISTER_REQ\n"));
            transportMgrHandleInternalRegisterReq((TRANSPORT_MGR_INTERNAL_REGISTER_REQ_T *) message);
        }
        break;

        case TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ:
        {
            PRINT(("TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ\n"));
            transportMgrHandleInternalDeRegisterReq((TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ_T *) message);
        }
        break;

        case TRANSPORT_MGR_INTERNAL_CONNECT_REQ:
        {
            PRINT(("TRANSPORT_MGR_INTERNAL_CONNECT_REQ\n"));
            transportMgrHandleInternalConnectReq((TRANSPORT_MGR_INTERNAL_CONNECT_REQ_T *) message);
        }
        break;

        case TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ:
        {
            PRINT(("TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ\n"));
            transportMgrHandleInternalDisConnectReq((TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ_T *) message);
        }
        break;
        /* silently ignore */
        case MESSAGE_SOURCE_EMPTY:
        {
            PRINT(("TM:MESSAGE_SOURCE_EMPTY\n"));   
        }
        break;

        default:
        {
            TransportHandleMessage(task, id, message);
        }
        break;
    }
}
