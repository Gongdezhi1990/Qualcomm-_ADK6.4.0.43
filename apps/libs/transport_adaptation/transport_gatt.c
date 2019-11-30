/****************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

*/

/*!
@file    transport_gatt.c
@brief   Routines for managing a transport channel over GATT.
*/

#include <panic.h>
#include <print.h>
#include <vm.h>

#include <gatt_manager.h>
#include "transport_gatt.h"
#include "transport_adaptation.h"
#include "transport_adaptation_common.h"

/***************************************************************************
NAME
    transportSendGattMessageMoreData

DESCRIPTION
    utility function for sending TRANSPORT_MESSAGE_MORE_DATA 
    message to the transport manager layer.
*/
static void transportSendGattMessageMoreData(uint16 handle,
                                                uint16 size_value,
                                                uint8 *value)
{
    MAKE_TA_MESSAGE_WITH_LEN(TRANSPORT_MESSAGE_MORE_DATA, size_value);
    PRINT(("TA GATT : transportSendGattMessageMoreData\n"));

    message->transport = TRANSPORT_GATT;
    message->handle = handle;
    message->data_len = size_value;
    memcpy(message->data ,value, size_value);
    TA_SEND_MESSAGE_MORE_DATA(message);
}

/******************************************************************************/
bool transportGattRegister(Task lib_task, uint16 start_handle, uint16 end_handle)
{
    bool ret_value = FALSE;
    gatt_manager_status_t status;
    gatt_manager_server_registration_params_t registration_params;
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_REGISTER_CFM, message);
    PRINT(("TA GATT : transportGattRegister start_handle -%d, end_handle - %d\n", start_handle, end_handle));

    /* Setup data required for transport GATT Service to be registered with the GATT Manager */
    registration_params.task = lib_task;
    registration_params.start_handle = start_handle;
    registration_params.end_handle = end_handle;

    /* Register with the GATT Manager and verify the result */
    status = GattManagerRegisterServer(&registration_params);

    if(status == gatt_manager_status_success)
    {
        message->status  = SUCCESS;
        ret_value = TRUE;
    }
    else
        message->status  = FAIL;

    message->transport = TRANSPORT_GATT;
    message->transport_id = start_handle;

    PRINT(("TL GATT:Send TRANSPORT_REGISTER_CFM  %d\n", message->status));
    TA_SEND_REGISTER_CFM(message);
    return ret_value;
}

/*****************************************************************************/
bool transportGattSendNotify(uint16 handle, uint16 size_value, const uint8 *value)
{
    uint16 cid = transportGetGattCid();
    PRINT(("TA GATT: transportGattSendNotify\n"));

    /* Validate the input parameters */
    if ((cid == TA_GATT_INVALID_CID) || (size_value == 0) || (value == NULL))
    {
        PRINT(("TA GATT: transportGattSendNotify Invalid parameters\n"));
        return FALSE;
    }

    GattManagerRemoteClientNotify(transportGetLibTask(), cid, handle, size_value, value);
    return TRUE;
}

/******************************************************************************/
void transportSendGattServerAccessRsp(uint16 handle,
                                      uint16 result,
                                      uint16 size_value,
                                      const uint8 *value)
{
    PRINT(("TA GATT : transportSendGattServerAccessRsp\n"));

    /* Validate Cid and transport handler task. */
    if ((transportGetLibTask() == NULL))
    {
        PRINT(("TA GATT : Handler not initialised\n"));
    }
    else
    {
        if (!GattManagerServerAccessResponse(transportGetLibTask(),
                                             transportGetGattCid(),
                                             handle, 
                                             result,
                                             size_value,
                                             value))
        {
            PRINT(("TA GATT : Couldn't send GATT access response\n"));
        }
    }
}

/******************************************************************************/
void handleGattMessage(MessageId id, Message message)
{
    PRINT(("TA GATT: handleGattMessage 0x%x\n", (unsigned)id));

    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* Handle server access indication message from GATT Manager library. */
            GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind = (GATT_MANAGER_SERVER_ACCESS_IND_T *)message;

            /* Check Read/Write Access Permission Flag. */
            if (access_ind->flags & ATT_ACCESS_WRITE)
            {
                /* Send TRANSPORT_MESSAGE_MORE_DATA to transport manager. */
                transportSendGattMessageMoreData(access_ind->handle,
                                                 access_ind->size_value,
                                                 access_ind->value);
            }
            else
            {
                PRINT(("TA GATT : Unknown handle 0x%x with flag 0x%x\n", access_ind->handle, access_ind->flags));
                transportSendGattServerAccessRsp(access_ind->handle, gatt_status_write_not_permitted, 0, NULL);
            }
        }
        break;

        case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
        {
            PRINT(("TA GATT: GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM\n"));
            /* Library just absorbs confirmation messages */
        }
        break;

        default:
        {
            /* Unrecognised message from GATT Manager.*/
            PRINT(("TA GATT: Unrecognised message from GATT Manager.\n"));
        }
        break;
    }
}

/******************************************************************************/
void transportHandleGattConnected(gatt_status_t status, uint16 cid)
{
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_CONNECT_CFM,msg);
    PRINT(("TA GATT:transportHandleGattConnected\n"));

    msg->transport = TRANSPORT_GATT;

    if((status == gatt_status_success) && (transportGetGattCid()== TA_GATT_INVALID_CID))
    {
        /* Store the Connection identifier of remote device. */
        transportSetGattCid(cid);
        VmGetBdAddrtFromCid(cid, &msg->addr);
        msg->status = SUCCESS;
    }
    else
    {
        msg->status = FAIL;
    }

    /* Inform the transport manager about GATT connection confirmation.*/
    PRINT(("TA GATT: Send TRANSPORT_CONNECT_CFM\n"));
    TA_SEND_CONNECT_CFM(msg);
}

/******************************************************************************/
void transportHandleGattDisconnected(gatt_status_t status, uint16 cid)
{
    MAKE_TA_CLIENT_MESSAGE(TRANSPORT_DISCONNECT_CFM, msg);
    UNUSED(cid);
    PRINT(("TA GATT:transportHandleGattDisconnected\n"));

    msg->transport = TRANSPORT_GATT;

    if(status == gatt_status_success)
    {
        msg->status = SUCCESS;
        transportSetGattCid(TA_GATT_INVALID_CID);
    }
    else
    {
        msg->status = FAIL;
    }

    /* Inform the transport manager about GATT Disconnection confirmation.*/
    PRINT(("TA GATT: Send TRANSPORT_CONNECT_CFM\n"));
    TA_SEND_DISCONNECT_CFM(msg);
}

