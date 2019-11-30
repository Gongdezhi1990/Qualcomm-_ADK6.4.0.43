/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    transport_accessory.c
    
DESCRIPTION
    Handles adaptation of the Accessory transport.
*/

#include <panic.h>
#include <print.h>


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

