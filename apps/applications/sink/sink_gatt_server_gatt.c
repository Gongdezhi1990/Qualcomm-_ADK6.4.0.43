/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_gatt.c

DESCRIPTION
    Routines to handle messages sent from the GATT Server task.
*/
#include "sink_gatt_db.h"
#include "sink_gatt_server_gatt.h"
#include "sink_gatt_common.h"
#include "sink_ble.h"
#include "sink_gatt_server.h"
#include "sink_debug.h"
#include "sink_development.h"

#include <gatt_server.h>

#include <csrtypes.h>
#include <message.h>


#ifdef GATT_ENABLED


#ifdef DEBUG_GATT
#define GATT_INFO(x) DEBUG(x)
#define GATT_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else 
#define GATT_INFO(x)
#define GATT_ERROR(x) 
#endif


#define CLIENT_CONFIG_INDICATION 0x2 /* Client Characteristic Configuration Indication bits */

/******************************************************************************/
static void handle_read_client_config_indication(Message message)
{
    uint16 index;
    GATT_SERVER_READ_CLIENT_CONFIG_IND_T *msg;
    uint16 client_config = 0;

    msg = (GATT_SERVER_READ_CLIENT_CONFIG_IND_T*) PanicNull((void*)message);

    index = gattCommonConnectionFindByCid(msg->cid);
    if(GATT_INVALID_INDEX != index)
    {
        client_config = GATT_SERVER.client_config.gatt;
        if(!GattServerReadClientConfigResponse(GATT_SERVER.gatt_server,
                                                    msg->cid, msg->handle,
                                                    client_config))
        {
            GATT_ERROR(("GATT Server Read Client Config Response failed for CID[%x]\n", msg->cid));
        }
    }
    
    GATT_INFO(("Gatt service read, client_config[0x%x]\n", client_config));
}


/******************************************************************************/
static void handle_write_client_config_indication(Message message)
{
    uint16 index;
    GATT_SERVER_WRITE_CLIENT_CONFIG_IND_T *msg;

    msg = (GATT_SERVER_WRITE_CLIENT_CONFIG_IND_T*) PanicNull((void*)message);

    index = gattCommonConnectionFindByCid(msg->cid);
    if(GATT_INVALID_INDEX != index)
    {
       GATT_SERVER.client_config.gatt = msg->config_value;
       GATT_INFO(("  client_config[0x%x]\n", msg->config_value));
       gattServerStoreConfigAttributes(msg->cid, gatt_attr_service_gatt);
    }
    else
    {
        GATT_ERROR(("GATT Server task unknown CID[%x]\n", msg->cid));
    }
}


/*******************************************************************************/
bool sinkGattServerInitialiseTask(uint16 **ptr)
{
    if (ptr)
    {
        if (GattServerInit((GGATTS*)*ptr, sinkGetBleTask(),
                                                   HANDLE_GATT_SERVICE,
                                                   HANDLE_GATT_SERVICE_END) == gatt_server_status_success)
        {
            GATT_INFO(("GATT Server registered\n"));
            gattServerSetServicePtr(ptr, gatt_server_service_gatt);
            *ptr += ADJ_GATT_STRUCT_OFFSET(GGATTS);
            return TRUE;
        }
        else
        {
            GATT_INFO(("GATT Server init failed\n"));
        }
    }
    return FALSE;
}


/******************************************************************************/
void sinkGattServerMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    
    switch(id)
    {
        case GATT_SERVER_READ_CLIENT_CONFIG_IND:
        {
            DEBUG(("GATT_SERVER_READ_CLIENT_CONFIG_IND\n"));
            handle_read_client_config_indication(message);
        }
        break;
        case GATT_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            DEBUG(("GATT_SERVER_WRITE_CLIENT_CONFIG_IND\n"));
            handle_write_client_config_indication(message);
        }
        break;
        case GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM:
        {
            GATT_INFO(("GATT_SERVER_SERVICE_CHANGED_INDICATION_CFM\n"));
        }
        break;
        default:
        {
            GATT_ERROR(("GATT Server task unhandled msg[%x]\n", id));
        }
    }
}


/******************************************************************************/
void sinkGattServerSendServiceChanged(uint16 cid)
{
    uint16 index;
    
    index = gattCommonConnectionFindByCid(cid);
    if ((GATT_INVALID_INDEX != index) && (GATT_SERVER.client_config.gatt & CLIENT_CONFIG_INDICATION))
    {
        GattServerSendServiceChangedIndication(GATT_SERVER.gatt_server, cid);
    }
}


#endif /* GATT_ENABLED */
