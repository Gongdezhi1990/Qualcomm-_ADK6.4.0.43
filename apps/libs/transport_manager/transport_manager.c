	/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.


FILE NAME
    transport_manager.c

DESCRIPTION
    Functions to implement transport manager
*/
#include <stdlib.h>
#include <panic.h>
#include <bdaddr.h>
#include <transport_adaptation.h>
#include "transport_manager.h"
#include "transport_manager_private.h"
#include "print.h"


/******************************************************************************/
void  TransportMgrInit(void)
{
    PRINT(("TransportMgrInit\n"));

    /* Initialize the private data block */
    transportMgrInitPrivateData();

    /* Initialize the adaptation layer */
    TransportInit((Task)&(transportMgrGetPrivateData()->transport_mgr_task));
    
}

/******************************************************************************/
transport_mgr_status_t TransportMgrRegisterAccessory(uint16 *id, char *name)
{
    if (id && name)
    {
        uint16 protocol_id = TransportAccessoryRegisterProtocol(name);
        
        if (protocol_id)
        {
            *id = protocol_id;
            return transport_mgr_status_success;
        }
    }
    
    return transport_mgr_status_not_registered;
}
 
/******************************************************************************/
transport_mgr_status_t TransportMgrRegisterTransport(Task app_task,transport_mgr_link_cfg_t *trans_link_cfg)
{

    PRINT(("TransportMgrRegisterTransport\n"));
    /* NULL parameter check for link_config*/
    TRANSPORT_MGR_ASSERT_NULL(trans_link_cfg, ("TM: NULL for link config while registering"));

    /* Parameter Validation */
    if((!isTransportTypeValid(trans_link_cfg->type)) || (app_task == NULL))
    {
        return transport_mgr_status_invalid_params;
    }
    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    {
        /* This looks to be a new registration , we can start the internal message now */
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_INTERNAL_REGISTER_REQ,message);
        message->application_task = app_task;
        memcpy(&(message->link_cfg),trans_link_cfg,sizeof(transport_mgr_link_cfg_t));
        MessageSendConditionally(transportMgrGetTransportTask(), TRANSPORT_MGR_INTERNAL_REGISTER_REQ, message, isTransportMgrBusy());
        return transport_mgr_status_success;
    }
}

/******************************************************************************/
transport_mgr_status_t TransportMgrDeRegisterTransport(Task app_task,transport_mgr_type_t trans_type, uint16 link_id)
{

    PRINT(("TransportManUnRegisterTransport\n"));

    /* Parameter Validation*/
    if((!isTransportTypeValid(trans_type)) || (app_task == NULL))
    {
        return transport_mgr_status_invalid_params;
    }
    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    /* Check whether this link_id is already registered*/
    if(!(transportMgrFindDataWithTransInfo(link_id,trans_type)))
    {
        return transport_mgr_status_not_registered;
    }

    {
        /* Create the internal message for deregistration*/
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ, msg);
        msg->type = trans_type;
        msg->trans_link_id = link_id;
        msg->application_task = app_task;
        MessageSend(transportMgrGetTransportTask(),TRANSPORT_MGR_INTERNAL_DEREGISTER_REQ,msg);
    }
    return transport_mgr_status_success;
}


/******************************************************************************/
transport_mgr_status_t TransportMgrConnect(transport_mgr_type_t type, uint16 trans_link_id,uint8 remote_trans_link_id, tp_bdaddr *remote_addr)
{
    PRINT(("TransportMgrConnect\n"));


    /* Parameter validation */
    if((!isTransportTypeValid(type)) ||(BdaddrTpIsEmpty(remote_addr))
        || (type == transport_mgr_type_gatt))
    {
        return transport_mgr_status_invalid_params;
    }

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT


    /* Check whether this trans_link_id is already registered*/
    if(!(transportMgrFindDataWithTransInfo(trans_link_id,type)))
    {
        return transport_mgr_status_not_registered;
    }


    {
        /* Trigger the intenal message for Connection*/
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_INTERNAL_CONNECT_REQ, msg);
        msg->trans_type = type;
        msg->link_id= trans_link_id;
        msg->remote_link_id= remote_trans_link_id;
        msg->bd_addr=remote_addr;
        MessageSend(transportMgrGetTransportTask(),TRANSPORT_MGR_INTERNAL_CONNECT_REQ,msg);
    }
    return transport_mgr_status_success;


}

/******************************************************************************/
transport_mgr_status_t TransportMgrDisconnect(transport_mgr_type_t type, Sink disconnect_sink)
{
    PRINT(("TransportManConnect\n"));

    /* Parameter validation */
    if((!isTransportTypeValid(type)) || (disconnect_sink ==NULL))
    {
        return transport_mgr_status_invalid_params;
    }

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT


    /* Check whether this trans_link_id is already registered */
    if(!(transportMgrFindTranslinkIdFromSink(disconnect_sink)))
    {
        return transport_mgr_status_not_registered;
    }

    {
       /* Trigger the intenal message for Connection*/
        MAKE_TRANS_MGR_MESSAGE(TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ, msg);
        msg->trans_type = type;
        msg->sink= disconnect_sink;
        MessageSend(transportMgrGetTransportTask(),TRANSPORT_MGR_INTERNAL_DISCONNECT_REQ,msg);
    }
    
    return transport_mgr_status_success;
}

/******************************************************************************/
void TransportMgrClientConnected(gatt_status_t status,uint16 cid)
{
    PRINT(("TransportMgrClientConnected\n"));

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    /* Validation for registration could not be done here as the apptask
         calling this APi does not know about start and end handles. 
         Hence passing on the message to adaptation as it is */
    TransportGattConnected(status, cid);
}

/******************************************************************************/
void TransportMgrClientDisconnected(gatt_status_t status,uint16 cid)
{
    PRINT(("TransportMgrClientDisconnected\n"));

    /* Validate whether transport manager is initialized or not*/
    TRANSPORT_MGR_ASSERT_INIT

    /* Validation for registration could not be done here as the apptask
         calling this APi does not know about start and end handles. 
         Hence passing on the message to adaptation as it is */
    TransportGattDisconnected(status, cid);
}
