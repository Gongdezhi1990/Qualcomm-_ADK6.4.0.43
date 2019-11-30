/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#include <stdlib.h>
#include <string.h>
#include <message.h>

#include "gatt_manager_internal.h"

#include "gatt_manager_client.h"
#include "gatt_manager_data.h"

static bool clientParamsValid(const gatt_manager_client_registration_params_t *client)
{
    if ( (NULL == client) ||
         (NULL == client->client_task) ||
         (0 == client->cid) ||
         (0 == client->start_handle) ||
         (0 == client->end_handle) ||
         (client->start_handle >= client->end_handle))
    {
        return FALSE;
    }

    return TRUE;
}

static bool serverParamsValid(Task task,
                              const typed_bdaddr *taddr,
                              gatt_connection_type conn_type)
{
    if(NULL == task ||
       NULL == taddr ||
       (gatt_connection_bredr_master != conn_type &&
        gatt_connection_ble_master_directed != conn_type &&
        gatt_connection_ble_master_whitelist != conn_type) )
    {
        return FALSE;
    }

    return TRUE;
}

static bool gattClientDataValid(const gatt_manager_client_lookup_data_t * client_data,
                                uint16 handle)
{
    if(NULL == client_data ||
       handle < client_data->start_handle ||
       handle > client_data->end_handle)
    {
        return FALSE;
    }
    return TRUE;
}


/******************************************************************************
 *                      GATT MANAGER CLIENT PUBLIC API                        *
 ******************************************************************************/

gatt_manager_status_t GattManagerRegisterClient(const gatt_manager_client_registration_params_t *client)
{
    if (!gattManagerDataIsInit())
    {
        return gatt_manager_status_not_initialised;
    }

    if (gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_initialised)
    {
        return gatt_manager_status_wrong_state;
    }

    if (!clientParamsValid(client))
    {
        return gatt_manager_status_invalid_parameters;
    }

    if(!gattManagerDataAddClient(client))
    {
        return gatt_manager_status_failed;
    }

    return gatt_manager_status_success;
}

bool GattManagerGetClientData(const Task client, gatt_manager_client_service_data_t * service_data)
{
    const gatt_manager_client_lookup_data_t * client_lookup;

    if (!gattManagerDataIsInit() ||
        NULL == client ||
        NULL == service_data)
    {
        return FALSE;
    }

    client_lookup = gattManagerDataGetClientByTask(client);
    if (NULL == client_lookup)
    {
        return FALSE;
    }

    service_data->start_handle = client_lookup->start_handle;
    service_data->end_handle   = client_lookup->end_handle;
    return TRUE;
}

gatt_manager_status_t GattManagerUnregisterClient(Task client)
{
    if (!gattManagerDataIsInit())
    {
        return gatt_manager_status_not_initialised;
    }

    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_not_initialised)
    {
        return gatt_manager_status_wrong_state;
    }

    if(!gattManagerDataRemoveClient(client))
    {
        return gatt_manager_status_invalid_parameters;
    }

    return gatt_manager_status_success;
}

void GattManagerConnectToRemoteServer(const Task task,
                                      const typed_bdaddr *taddr,
                                      gatt_connection_type conn_type,
                                      bool conn_timeout)
{
    if (!gattManagerDataIsInit() ||
        gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_initialised)
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"));
    }

    if (gattManagerDataGetRemoteServerConnectHandler() != NULL)
    {
        GATT_MANAGER_PANIC(("GM: Remote server connect handler is already set!"));
    }

    if(!serverParamsValid(task, taddr, conn_type))
    {
        GATT_MANAGER_PANIC(("GM: Invalid parameters!"));
    }

    if(gattManagerDataGetAdvertisingState() == gatt_manager_advertising_state_requested)
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER);
        message->task = task;
        message->taddr = *taddr;
        message->conn_type = conn_type;
        message->conn_timeout = conn_timeout;
        MessageSendConditionally(gattManagerDataGetTask(),
                                 GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER,
                                 message, gattManagerDataGetAdvertisingRequestedFlag());
    }
    else
    {
        GattConnectRequest(gattManagerDataGetTask(), taddr, conn_type, conn_timeout);
        gattManagerDataSetRemoteServerConnectHandler(task);
    }
}


static void cancelRemoteServerConnectCfm(Task task, gatt_manager_status_t status)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_CANCEL_REMOTE_SERVER_CONNECT_CFM);
    message->status = status;
    MessageSend(task, GATT_MANAGER_CANCEL_REMOTE_SERVER_CONNECT_CFM, message);
}


void GattManagerCancelConnectToRemoteServer(Task default_task)
{
    Task connection_task = gattManagerDataGetRemoteServerConnectHandler();
    gatt_manager_status_t request_status = gatt_manager_status_wrong_state;
    uint16 cid;

    if (!gattManagerDataIsInit() ||
        gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_initialised)
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"));
    }

    if (gattManagerDataGetRemoteServerConnectHandler())
    {
        cid = gattManagerDataGetRemoteServerConnectCid();
        gattManagerDataSetRemoteServerConnectHandler(NULL);

        if (INVALID_CID != cid)
        {
            GattDisconnectRequest(cid);
            request_status = gatt_manager_status_success;
        }
        else
        {
            /* When a request to connect is made, we first get a message
               saying initialising and giving the cid. If we have a task, but
               no CID, then we may have been called before getting this message */
            GATT_MANAGER_DEBUG_INFO(("GM: GattManagerCancelConnectToRemoteServer may have been requested in gap before init"));

            request_status = gatt_manager_status_failed;
        }
    }

    if (connection_task)
    {
        cancelRemoteServerConnectCfm(connection_task, request_status);
    }
    if (default_task != connection_task)
    {
        cancelRemoteServerConnectCfm(default_task, request_status);
    }
}


void GattManagerDiscoverAllCharacteristics(const Task client)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Discover All Characteristics"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
            NULL != client_data)
    {
        GattDiscoverAllCharacteristicsRequest(gattManagerDataGetTask(), client_data->cid,
                                              client_data->start_handle,
                                              client_data->end_handle );
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM, message);
    }
}

void GattManagerDiscoverAllCharacteristicDescriptors(const Task client,
                                                    uint16 start_handle,
                                                    uint16 end_handle)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Discover Characteristic Descriptors"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        NULL != client_data &&
        start_handle <= end_handle &&
        start_handle >= client_data->start_handle &&
        end_handle <= client_data->end_handle)
    {
        GattDiscoverAllCharacteristicDescriptorsRequest(gattManagerDataGetTask(),
                                                        client_data->cid,
                                                        start_handle,
                                                        end_handle);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM, message);
    }
}

void GattManagerReadCharacteristicValue(const Task client, uint16 handle)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in characteristic value request"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattReadCharacteristicValueRequest(gattManagerDataGetTask(), client_data->cid, handle);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void GattManagerReadUsingCharacteristicUuid(const Task client,
                                            uint16 start_handle,
                                            uint16 end_handle,
                                            gatt_uuid_type_t uuid_type,
                                            const gatt_uuid_t *uuid)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Read Using Characteristic UUID"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        NULL != client_data &&
        start_handle <= end_handle &&
        start_handle >= client_data->start_handle &&
        end_handle <= client_data->end_handle)
    {
        GattReadUsingCharacteristicUuidRequest(gattManagerDataGetTask(), client_data->cid,
                                               start_handle, end_handle,
                                               uuid_type, uuid);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM, message);
    }
}

void GattManagerReadLongCharacteristicValue(Task client, uint16 handle)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Read Long Characteristic Value"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattReadLongCharacteristicValueRequest(gattManagerDataGetTask(), client_data->cid, handle);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void GattManagerWriteWithoutResponse(const Task client,
                                     uint16 handle,
                                     uint16 size_value,
                                     uint8 *value)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Write Without Response"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattWriteWithoutResponseRequest(gattManagerDataGetTask(), client_data->cid,
                                        handle, size_value, value );
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM, message);
    }
}


void GattManagerSignedWriteWithoutResponse(const Task client,
                                           uint16 handle,
                                           uint16 size_value,
                                           uint8 *value)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Signed Write Without Response"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattSignedWriteWithoutResponseRequest(gattManagerDataGetTask(), client_data->cid,
                                              handle, size_value, value );
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM, message);
    }
}

void GattManagerWriteCharacteristicValue(const Task client,
                                         uint16 handle,
                                         uint16 size_value,
                                         const uint8 *value)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Write Characteristic Value"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattWriteCharacteristicValueRequest(gattManagerDataGetTask(), client_data->cid,
                                            handle, size_value, value );
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void GattManagerWriteLongCharacteristicValue(const Task client,
                                             uint16 handle,
                                             uint16 size_value,
                                             const uint8 *value)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Write Long Characteristic Value"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattWriteLongCharacteristicValueRequest(gattManagerDataGetTask(), client_data->cid,
                                                handle, size_value, value );
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void GattManagerReliableWritePrepare(const Task client,
                                     uint16 handle,
                                     uint16 offset,
                                     uint16 size_value,
                                     uint8 *value)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Reliable Write Prepare"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        gattClientDataValid(client_data, handle) == TRUE)
    {
        GattReliableWritePrepareRequest(gattManagerDataGetTask(), client_data->cid,
                                        handle, offset, size_value, value );
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM, message);
    }
}

void GattManagerReliableWriteExecute(const Task client, bool execute)
{
    const gatt_manager_client_lookup_data_t * client_data;

    GATT_MANAGER_PANIC_NULL(client, ("GM: NULL task in Reliable Write Execute"));

    client_data =  gattManagerDataGetClientByTask(client);
    if (gattManagerDataGetInitialisationState() == gatt_manager_initialisation_state_initialised &&
        NULL != client_data)
    {
        GattReliableWriteExecuteRequest(gattManagerDataGetTask(), client_data->cid, execute);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM);
        message->status = gatt_status_failure;
        MessageSend(client, GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM, message);
    }
}


/******************************************************************************
 *                      GATT MANAGER Client Internal API                      *
 ******************************************************************************/

void GattManagerConnectToRemoteServerInternal(const GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER_T *params)
{
    GATT_MANAGER_PANIC_NULL(params, ("GM: Connect T oRemote Server Internal params NULL!"));

    GattManagerConnectToRemoteServer(params->task, &params->taddr, params->conn_type,
                                     params->conn_timeout);
}

void gattManagerClientRemoteServerConnected(const GATT_CONNECT_CFM_T * cfm)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM);

    *message = *cfm;

    if(gattManagerDataGetRemoteServerConnectHandler())
    {
        MessageSend(gattManagerDataGetRemoteServerConnectHandler(),
                    GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM, message);
        gattManagerDataSetRemoteServerConnectHandler(NULL);
    }
}


void gattManagerClientConnecting(uint16 cid)
{
    gattManagerDataSetRemoteServerConnectCid(cid);
}


void gattManagerClientRemoteServerNotification(const GATT_NOTIFICATION_IND_T * not)
{
    Task task = gattManagerDataGetClientByCid(not->handle, not->cid);
    MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND,
                                       not->size_value);
    if (NULL == task)
    {
        task = gattManagerDataGetApplicationTask();
    }

    *message = *not;
    memmove(message->value,not->value,not->size_value * sizeof(uint8));

    MessageSend(task, GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND, message);
}

void gattManagerClientRemoteServerIndication(const GATT_INDICATION_IND_T * ind)
{
    Task task = gattManagerDataGetClientByCid(ind->handle, ind->cid);
    MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(GATT_MANAGER_REMOTE_SERVER_INDICATION_IND, ind->size_value);

    if (NULL == task)
    {
        task = gattManagerDataGetApplicationTask();
    }

    *message = *ind;
    memmove(message->value,ind->value ,ind->size_value * sizeof(uint8));

    MessageSend(task, GATT_MANAGER_REMOTE_SERVER_INDICATION_IND, message);
}

void gattManagerClientDiscoverAllCharacteristicsConfirm(const GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_DISCOVER_ALL_CHARACTERISTICS_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM, message);
    }
}

void gattManagerClientDiscoverAllCharacteristicsDescriptorsConfirm(const GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM, message);
    }
}

void gattManagerClientReadCharacteristicValueConfirm(const GATT_READ_CHARACTERISTIC_VALUE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_READ_CHARACTERISTIC_VALUE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM, cfm->size_value);

        *message = *cfm;
        memmove(message->value,cfm->value,cfm->size_value * sizeof(uint8));

        MessageSend(task, GATT_MANAGER_READ_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void gattManagerClientReadUsingCharacteristicUuidConfirm(const GATT_READ_USING_CHARACTERISTIC_UUID_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_READ_USING_CHARACTERISTIC_UUID_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM, cfm->size_value);

        *message = *cfm;
        memmove(message->value,cfm->value,cfm->size_value * sizeof(uint8));

        MessageSend(task, GATT_MANAGER_READ_USING_CHARACTERISTIC_UUID_CFM, message);
    }
}


void gattManagerClientReadLongCharacteristicValueConfirm(const GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM, cfm->size_value);
        
        *message = *cfm;
        memmove(message->value,cfm->value,cfm->size_value * sizeof(uint8));

        MessageSend(task, GATT_MANAGER_READ_LONG_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void gattManagerClientWriteWithoutResponseConfirm(const GATT_WRITE_WITHOUT_RESPONSE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_WRITE_WITHOUT_RESPONSE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_WRITE_WITHOUT_RESPONSE_CFM, message);
    }
}

void gattManagerClientSignedWriteWithoutResponseConfirm(const GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_SIGNED_WRITE_WITHOUT_RESPONSE_CFM, message);
    }
}

void gattManagerClientWriteCharacteristicValueConfirm(const GATT_WRITE_CHARACTERISTIC_VALUE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_WRITE_CHARACTERISTIC_VALUE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void gattManagerClientWriteLongCharacteristicValueConfirm(const GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_WRITE_LONG_CHARACTERISTIC_VALUE_CFM, message);
    }
}

void gattManagerClientReliableWritePrepareConfirm(const GATT_RELIABLE_WRITE_PREPARE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_RELIABLE_WRITE_PREPARE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM);

        *message = *cfm;

        MessageSend(task, GATT_MANAGER_RELIABLE_WRITE_PREPARE_CFM, message);
    }
}

void gattManagerClientReliableWriteExecuteConfirm(const GATT_RELIABLE_WRITE_EXECUTE_CFM_T * cfm)
{
    Task task = gattManagerDataGetClientByCid(cfm->handle, cfm->cid);
    if (NULL == task)
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Invalid client [Handle:%x, CID:%x] for GATT_RELIABLE_WRITE_EXECUTE_CFM\n",
                                 cfm->handle, cfm->cid));
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM);

        *message = *cfm;
        
        MessageSend(task, GATT_MANAGER_RELIABLE_WRITE_EXECUTE_CFM, message);
    }
}
