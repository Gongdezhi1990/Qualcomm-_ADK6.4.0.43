/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#include <stdlib.h>
#include <string.h>

#include "gatt_manager_internal.h"

#include "gatt_manager_handler.h"
#include "gatt_manager_data.h"
#include "gatt_manager_server.h"
#include "gatt_manager_client.h"

static void handleGattInitCfm(const GATT_INIT_CFM_T * cfm)
{
    if (gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_registering )
    {
        GATT_MANAGER_PANIC(("GM: Received GATT_INIT_CFM in wrong state[%x]\n",
                           gattManagerDataGetInitialisationState()));
    }

    if (gatt_status_success != cfm->status)
    {
        gattManagerDataInitialisationState_Registration();
        registerWithGattCfm(gatt_manager_status_failed);
    }
    else
    {
        gattManagerDataInitialisationState_Initialised();
        registerWithGattCfm(gatt_manager_status_success);
    }
}

static void handleGattConnectCfm(const GATT_CONNECT_CFM_T * cfm)
{
    if (gatt_status_initialising == cfm->status)
    {
        if(gattManagerDataGetAdvertisingState() == gatt_manager_advertising_state_requested)
        {
            gattManagerServerAdvertising(cfm->cid);
        }
        else
        {
            gattManagerClientConnecting(cfm->cid);
            GATT_MANAGER_DEBUG_INFO(("GM: Remote Server connection Initialising!"));
        }
    }
    else if (cfm->cid == gattManagerDataGetRemoteClientConnectCid())
    {
        gattManagerServerRemoteClientConnected(cfm);
    }
    else
    {
        gattManagerClientRemoteServerConnected(cfm);
    }
}

static void gattManageDisconnectInd(const GATT_DISCONNECT_IND_T * ind)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_DISCONNECT_IND);

    *message = *ind;

    MessageSend(gattManagerDataGetApplicationTask(), GATT_MANAGER_DISCONNECT_IND,
                message);
}

static void gattManagerExchangeMtuInd(const GATT_EXCHANGE_MTU_IND_T * ind)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_EXCHANGE_MTU_IND);

    *message = *ind;

    MessageSend(gattManagerDataGetApplicationTask(), GATT_EXCHANGE_MTU_IND,
                message);
}

static void gattManagerExchangeMtuCfm(const GATT_EXCHANGE_MTU_CFM_T * cfm)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_EXCHANGE_MTU_CFM);

    *message = *cfm;

    MessageSend(gattManagerDataGetApplicationTask(), GATT_EXCHANGE_MTU_CFM,
                message);
}

static void gattMessageHandler(MessageId id, Message payload)
{
    GATT_MANAGER_PANIC_NULL(payload, ("GM: Message payload is NULL!"));

    switch(id)
    {
        /* GATT Messages handled by the GATT Manager CONNECTION module */
        case GATT_INIT_CFM:
            handleGattInitCfm( (const GATT_INIT_CFM_T*)payload );
            break;

        case GATT_CONNECT_CFM:
            handleGattConnectCfm( (const GATT_CONNECT_CFM_T*)payload );
            break;

        case GATT_DISCONNECT_IND:
            gattManageDisconnectInd((const GATT_DISCONNECT_IND_T*)payload);
            break;

        case GATT_EXCHANGE_MTU_CFM:
            gattManagerExchangeMtuCfm((const GATT_EXCHANGE_MTU_CFM_T*)payload);
            break;

        case GATT_EXCHANGE_MTU_IND:
            gattManagerExchangeMtuInd((const GATT_EXCHANGE_MTU_IND_T*)payload);
            break;

        /* GATT Messages that are forwarded to the client task */
        case GATT_NOTIFICATION_IND:
            gattManagerClientRemoteServerNotification((const GATT_NOTIFICATION_IND_T*)payload);
            break;

        case GATT_INDICATION_IND:
            gattManagerClientRemoteServerIndication((const GATT_INDICATION_IND_T*)payload);
            break;

        case GATT_DISCOVER_ALL_CHARACTERISTICS_CFM:
            gattManagerClientDiscoverAllCharacteristicsConfirm((const GATT_DISCOVER_ALL_CHARACTERISTICS_CFM_T *)payload);
            break;

        case GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM:
            gattManagerClientDiscoverAllCharacteristicsDescriptorsConfirm((const GATT_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *)payload);
            break;

        case GATT_READ_CHARACTERISTIC_VALUE_CFM:
            gattManagerClientReadCharacteristicValueConfirm((const GATT_READ_CHARACTERISTIC_VALUE_CFM_T *)payload);
            break;

        case GATT_READ_USING_CHARACTERISTIC_UUID_CFM:
            gattManagerClientReadUsingCharacteristicUuidConfirm((const GATT_READ_USING_CHARACTERISTIC_UUID_CFM_T *)payload);
            break;

        case GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM:
            gattManagerClientReadLongCharacteristicValueConfirm((const GATT_READ_LONG_CHARACTERISTIC_VALUE_CFM_T *)payload);
            break;

        case GATT_WRITE_WITHOUT_RESPONSE_CFM:
            gattManagerClientWriteWithoutResponseConfirm((const GATT_WRITE_WITHOUT_RESPONSE_CFM_T *)payload);
            break;

        case GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM:
            gattManagerClientSignedWriteWithoutResponseConfirm((const GATT_SIGNED_WRITE_WITHOUT_RESPONSE_CFM_T *)payload);
            break;

        case GATT_WRITE_CHARACTERISTIC_VALUE_CFM:
            gattManagerClientWriteCharacteristicValueConfirm((const GATT_WRITE_CHARACTERISTIC_VALUE_CFM_T *)payload);
            break;

        case GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM:
            gattManagerClientWriteLongCharacteristicValueConfirm((const GATT_WRITE_LONG_CHARACTERISTIC_VALUE_CFM_T *)payload);
            break;

        case GATT_RELIABLE_WRITE_PREPARE_CFM:
            gattManagerClientReliableWritePrepareConfirm((const GATT_RELIABLE_WRITE_PREPARE_CFM_T *)payload);
            break;

        case GATT_RELIABLE_WRITE_EXECUTE_CFM:
            gattManagerClientReliableWriteExecuteConfirm((const GATT_RELIABLE_WRITE_EXECUTE_CFM_T *)payload);
            break;

        /* GATT Messages handled by the GATT Manager Server module */
        case GATT_CONNECT_IND:
            gattManagerServerConnectInd((const GATT_CONNECT_IND_T*)payload);
            break;

        case GATT_ACCESS_IND:
            gattManagerServerAccessInd((const GATT_ACCESS_IND_T*)payload );
            break;

        case GATT_NOTIFICATION_CFM:
            gattManagerServerNotificationCfm((const GATT_NOTIFICATION_CFM_T*)payload);
            break;

        case GATT_INDICATION_CFM:
            gattManagerServerIndicationCfm((const GATT_INDICATION_CFM_T*)payload);
            break;

        default:
            GATT_MANAGER_DEBUG_PANIC(("GM: Un-handled GATT message [%x]\n", id));
            break;
    }
}

static void gattManagerInternalMessageHandler(MessageId id, Message payload)
{
    switch(id)
    {
        case GATT_MANAGER_INTERNAL_MSG_CANCEL_ADVERTISING:
            GattManagerCancelWaitForRemoteClient();
            break;

        case GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT:
            gattManagerWaitForRemoteClientInternal((const GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT_T *)payload);
            break;

        case GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER:
            GattManagerConnectToRemoteServerInternal((const GATT_MANAGER_INTERNAL_MSG_CONNECT_TO_REMOTE_SERVER_T *)payload);
            break;

        default:
            GATT_MANAGER_DEBUG_PANIC(("ERROR: Un-handled Internal GATT Manager message [%x]\n", id));
            break;
    }
}

/******************************************************************************
 *                      GATT MANAGER Internal API                             *
 ******************************************************************************/

void registerWithGattCfm(gatt_manager_status_t status)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REGISTER_WITH_GATT_CFM);
    message->status = status;
    MessageSend(gattManagerDataGetApplicationTask(),
                GATT_MANAGER_REGISTER_WITH_GATT_CFM, message);
}

void gattManagerMessageHandler(Task task, MessageId id, Message payload)
{
    UNUSED(task);

    if (!gattManagerDataIsInit())
    {
        GATT_MANAGER_PANIC(("GM: Not Initialised but received a message!"));
    }

    if ((id >= GATT_MESSAGE_BASE) && (id < GATT_MESSAGE_TOP))
    {
        gattMessageHandler(id, payload);
    }
    else if ((id >= GATT_MANAGER_INTERNAL_MSG_BASE) &&
             (id < GATT_MANAGER_INTERNAL_MSG_TOP))
    {
        gattManagerInternalMessageHandler(id, payload);
    }
    else
    {
        GATT_MANAGER_DEBUG_PANIC(("ERROR: Un-handled message [%x]\n", id));
    }
}
