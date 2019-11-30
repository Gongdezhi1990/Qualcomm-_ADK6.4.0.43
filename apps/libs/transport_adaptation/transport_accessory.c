/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    transport_accessory.c
    
DESCRIPTION
    Handles adaptation of the Accessory transport.
*/

#include <panic.h>
#include <print.h>

#ifdef TA_HAVE_ACCESSORY_TRANSPORT
#include <iap2.h>
#endif

#include "transport_accessory.h"
#include "transport_adaptation.h"
#include "transport_adaptation_common.h"

static void sendRegisterCfm(bool status, uint16 transport_id)
{
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_REGISTER_CFM, msg);
    
    msg->status = status;
    msg->transport = TRANSPORT_ACCESSORY;
    msg->transport_id = transport_id;
    
    TA_SEND_REGISTER_CFM(msg);    
}


static void sendDeegisterCfm(bool status, uint16 transport_id)
{
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_DEREGISTER_CFM, msg);
    
    msg->status = status;
    msg->transport = TRANSPORT_ACCESSORY;
    msg->transport_id = transport_id;
    
    TA_SEND_DEREGISTER_CFM(msg);
}

#ifdef TA_HAVE_ACCESSORY_TRANSPORT

static void handleRegisterHandler(IAP2_EA_REGISTER_HANDLER_CFM_T *cfm)
{
    sendRegisterCfm(cfm->status == iap2_status_success, cfm->protocol_id);
}


static void handleUnegisterHandler(IAP2_EA_UNREGISTER_HANDLER_CFM_T *cfm)
{
    sendDeegisterCfm(cfm->status == iap2_status_success, cfm->protocol_id);
}


static void handleSessionStart(IAP2_EA_SESSION_START_IND_T *ind)
{
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_CONNECT_CFM, cfm);

    cfm->transport    = TRANSPORT_ACCESSORY;
    cfm->sink = (Sink) ind->link;
    cfm->transport_id = ind->protocol_id;
    cfm->status  = SUCCESS;
    TA_SEND_CONNECT_CFM(cfm);
}


static void handleSessionStop(IAP2_EA_SESSION_STOP_IND_T *ind)
{
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_DISCONNECT_CFM, cfm);

    cfm->transport    = TRANSPORT_ACCESSORY;
    cfm->sink = (Sink) ind->link;
    cfm->transport_id = ind->protocol_id;
    cfm->status  = SUCCESS;
    TA_SEND_DISCONNECT_CFM(cfm);
}


static void handleSessionData(IAP2_EA_SESSION_DATA_IND_T *ind)
{
    MAKE_TA_MESSAGE_WITH_LEN(TRANSPORT_MESSAGE_MORE_DATA, ind->size_payload);
    message->transport = TRANSPORT_ACCESSORY;
    message->handle = ind->protocol_id;
    message->data_len = ind->size_payload;
    memcpy(message->data, ind->payload, ind->size_payload);
    TA_SEND_MESSAGE_MORE_DATA(message);
}


void transportAccessoryRegisterReq(Task task, uint8 transport_id)
{
    Iap2EARegisterHandler(task, transport_id);
}


uint16 TransportAccessoryRegisterProtocol(char *name)
{
    return Iap2EARegisterProtocol(name, iap2_app_match_optional);
}


void transportAccessoryDeregisterReq(Task task, uint8 transport_id)
{
    Iap2EAUnregisterHandler(task, transport_id);
}

void handleAccessoryMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(message);

    switch (id)
    {
    case IAP2_EA_REGISTER_HANDLER_CFM:
        handleRegisterHandler((IAP2_EA_REGISTER_HANDLER_CFM_T *) message);
        break;
        
    case IAP2_EA_UNREGISTER_HANDLER_CFM:
        handleUnegisterHandler((IAP2_EA_UNREGISTER_HANDLER_CFM_T *) message);
        break;
        
    case IAP2_IDENTIFICATION_CFM:
        break;

    case IAP2_EA_SESSION_START_IND:
        handleSessionStart((IAP2_EA_SESSION_START_IND_T *) message);
        break;

    case IAP2_EA_SESSION_STOP_IND:
        handleSessionStop((IAP2_EA_SESSION_STOP_IND_T *) message);
        break;
        
    case IAP2_EA_SESSION_DATA_IND:
        handleSessionData((IAP2_EA_SESSION_DATA_IND_T *) message);
        break;

    default:
        break;
    }
}


bool TransportAccessorySendData(uint16 transport_id, uint16 size_payload, uint8 *payload)
{
    return Iap2EASendData(transport_id, payload, size_payload);
}


uint16 TransportAccessoryGetAvailableSpace(uint16 transport_id)
{
    return Iap2EAGetSlackForProtocol(transport_id);
}


#else /* TA_HAVE_ACCESSORY_TRANSPORT */

/* The Accessory transport is included in an add-on installer. */
uint16 TransportAccessoryRegisterProtocol(char *name)
{
    UNUSED(name);

    return 0;
}


void transportAccessoryRegisterReq(Task task, uint8 transport_id)
{
    UNUSED(task);
    UNUSED(transport_id);

    sendRegisterCfm(FALSE, transport_id);
}


void transportAccessoryDeregisterReq(Task task, uint8 transport_id)
{
    UNUSED(task);
    UNUSED(transport_id);

    sendDeegisterCfm(FALSE, transport_id);
}


void handleAccessoryMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);
    UNUSED(id);
    UNUSED(message);
}


bool TransportAccessorySendData(uint16 transport_id, uint16 size_payload, uint8 *payload)
{
    UNUSED(transport_id);
    UNUSED(size_payload);
    UNUSED(payload);

    return FALSE;
}


uint16 TransportAccessoryGetAvailableSpace(uint16 transport_id)
{
    UNUSED(transport_id);
    return 0;
}

#endif /* TA_HAVE_ACCESSORY_TRANSPORT */
