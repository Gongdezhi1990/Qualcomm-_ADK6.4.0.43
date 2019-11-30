/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Routines for managing all the underlying transports 
 */

#include <connection.h>
#include <sink.h>
#include <print.h>
#include <panic.h>
#include <vm.h>
#include "transport_accessory.h"
#include <gatt_manager.h>
#include "transport_adaptation.h"
#include "transport_rfcomm.h"
#include "transport_gatt.h"
#include "transport_adaptation_common.h"

/******************************************************************************
Main handler for Transport adaptation module. This handles all the 
communication with underlying transports.
*/
void TransportHandleMessage(Task task, MessageId id, Message message)
{
    PRINT(("TransportHandleMessage\n"));

    if((id >= CL_RFCOMM_REGISTER_CFM) && (id<=CL_RFCOMM_LINE_STATUS_CFM))
    {
        handleRfcommMessage(task, id, message);
    }
    else if ((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        handleGattMessage(id, message);
    }
    else
    {
        handleAccessoryMessage(task, id, message);
    }
}

/******************************************************************************/
bool TransportInit(Task app_task)
{
    PRINT(("TA: TransportInit\n"));

    if((app_task == NULL) ||(transportIsInitialised() == TRUE))
    {
        /* Either Application task is NULL or TA task already intialised. */
        return FALSE;
    }
    else
    {
        /* Initailase the transport manager data base. */
        transportInitialise(app_task);
        return TRUE;
    }
}

/******************************************************************************/
bool TransportRegisterReq(transport_type_t transport, uint16 transport_id)
{
    PRINT(("TA: TransportRegisterReq\n"));

    if(transportIsInitialised())
    {
        if(transport== TRANSPORT_RFCOMM)
        {
            transportRfcommRegister(transportGetLibTask(), (uint8)transport_id);
            return TRUE;
        }
        else if(transport == TRANSPORT_ACCESSORY)
        {
            transportAccessoryRegisterReq(transportGetLibTask(), transport_id);
            return TRUE;
        }
    }
    
    return FALSE;
}

/******************************************************************************/
bool TransportDeregisterReq(transport_type_t transport, uint16 transport_id)
{
    PRINT(("TA: TransportDeregisterReq\n"));

    if(transportIsInitialised())
    {
        if(transport == TRANSPORT_RFCOMM)
        {
            transportRfcommDeregister(transportGetLibTask(), (uint8)transport_id);
            return TRUE;
        }
        else if(transport == TRANSPORT_ACCESSORY)
        {
            transportAccessoryDeregisterReq(transportGetLibTask(), transport_id);
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************/
bool TransportConnectReq(transport_type_t transport,
                         const tp_bdaddr *tpaddr,
                         uint16 transport_id,
                         uint16 remote_transport_id)
{
    PRINT(("TA: TransportConnectReq\n"));

    if(transportIsInitialised())
    {
        if(tpaddr == NULL)
            return FALSE;

        if(transport == TRANSPORT_RFCOMM)
         {
            transportRfcommConnect((Task)transportGetLibTask(),
                                   &tpaddr->taddr.addr,
                                   transport_id,
                                   (uint8)remote_transport_id);
             return TRUE;
         }
    }
     return FALSE;
}

/******************************************************************************/
bool TransportDisconnectReq(transport_type_t transport, Sink sink)
{
    PRINT(("TA: TransportDisconnectReq\n"));

    if(transportIsInitialised())
    {
        if(!SinkIsValid(sink))
            return FALSE;

        if(transport == TRANSPORT_RFCOMM)
         {
            transportRfcommDisconnect(transportGetLibTask(), sink);
             return TRUE;
         }
    }
     return FALSE;
}

/******************************************************************************/
bool TransportGattRegisterReq(uint16 start_handle, uint16 end_handle)
{
    PRINT(("TA GATT:TransportGattRegisterReq\n"));

    /* Validate the parameters. */
    if ((transportGetLibTask() == NULL) || IS_GATT_HANDLES_NOT_VALID(start_handle, end_handle))
    {
        PRINT(("TA GATT : TransportGattRegisterReq Invalid parameters\n"));
        return FALSE;
    }
    else
    {
        return (transportGattRegister(transportGetLibTask(), start_handle, end_handle));
    }
}

/******************************************************************************/
bool TransportGattSendData(uint16 handle, uint16 size_payload, uint8 *payload)
{
    /* Send notification to the remote device.*/
    return (transportGattSendNotify(handle, size_payload, payload));
}

/******************************************************************************/
void TransportGattConnected(gatt_status_t status, uint16 cid)
{
    PRINT(("TA GATT:TransportGattConnected\n"));
    transportHandleGattConnected(status, cid);
}

/******************************************************************************/
void TransportGattDisconnected(gatt_status_t status, uint16 cid)
{
    PRINT(("TA GATT:TransportGattDisconnected\n"));
    transportHandleGattDisconnected(status, cid);
}

/******************************************************************************/
void TransportGattSendServerAccessRsp(uint16 handle,
                                            uint16 result,
                                            uint16 size_value,
                                            const uint8 *value)
{
    PRINT(("TA GATT : TransportSendServerAccessRsp\n"));
    transportSendGattServerAccessRsp(handle, result, size_value, value);
}
