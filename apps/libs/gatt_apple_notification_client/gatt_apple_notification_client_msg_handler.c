/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client_msg_handler.h"
#include "gatt_apple_notification_client_private.h"
#include "gatt_apple_notification_client_discover.h"
#include "gatt_apple_notification_client_notification.h"
#include "gatt_apple_notification_client_external_msg_send.h"
#include "gatt_apple_notification_client_write.h"

static void handleSetNsNotificationMsg(GANCS *ancs, const ANCS_INTERNAL_MSG_SET_NS_NOTIFICATION_T *notif)
{
    ancs->task_pending_cfm = notif->task_pending_cfm;
    ancs->ns_notification_mask = notif->notifications_mask;
    ancsSetNotificationRequest(ancs, notif->notifications_enable, GATT_APPLE_NOTIFICATION_NS);
}

static void handleSetDsNotificationMsg(GANCS *ancs, const ANCS_INTERNAL_MSG_SET_DS_NOTIFICATION_T *notif)
{
    ancs->task_pending_cfm = notif->task_pending_cfm;
    ancsSetNotificationRequest(ancs, notif->notifications_enable, GATT_APPLE_NOTIFICATION_DS);
}

static void handleWriteControlPointCharacteristicMsg(GANCS *ancs, const ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC_T *charact)
{
    ancs->task_pending_cfm = charact->task_pending_cfm;
    ancs->pending_cmd = charact->pending_cmd;

    if(CHECK_VALID_HANDLE(ancs->control_point))
        ancsWriteCharValue(ancs, charact, ancs->control_point);
    else
        gattAncsSendWriteControlPointResponse(ancs, gatt_status_request_not_supported);
}

static void handleGattManagerMsg(GANCS *ancs, MessageId id, Message payload)
{
    switch (id)
    {
        case GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND:
            handleAncsNotification(ancs, (GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *) payload);
        break;

        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM:
            handleAncsDiscoverAllCharacteristicsResp(ancs, (GATT_MANAGER_DISCOVER_ALL_CHARACTERISTICS_CFM_T *) payload);
        break;

        case GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM:
            handleAncsDiscoverAllCharacteristicDescriptorsResp(ancs, (GATT_MANAGER_DISCOVER_ALL_CHARACTERISTIC_DESCRIPTORS_CFM_T *) payload);
        break;

        case GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM:
            handleAncsWriteValueResp(ancs, (GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *) payload);
        break;

        default:
            DEBUG_PANIC(("ANCS: Client GattMgr Msg not handled [0x%x]\n", id));
        break;
    }
}

static void handleInternalAncsMsg(GANCS *ancs, MessageId id, Message payload)
{
    switch (id)
    {
        case ANCS_INTERNAL_MSG_SET_NS_NOTIFICATION:
            handleSetNsNotificationMsg(ancs, (ANCS_INTERNAL_MSG_SET_NS_NOTIFICATION_T *) payload);
        break;

        case ANCS_INTERNAL_MSG_SET_DS_NOTIFICATION:
            handleSetDsNotificationMsg(ancs, (ANCS_INTERNAL_MSG_SET_DS_NOTIFICATION_T *) payload);
        break;

        case ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC:
            handleWriteControlPointCharacteristicMsg(ancs, (ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC_T *) payload);
        break;

        default:
            DEBUG_PANIC(("ANCS: Client Internal Msg not handled [0x%x]\n", id));
        break;
    }
}

void appleNotificationClientMsgHandler(Task task, MessageId id, Message payload)
{
    GANCS *ancs = (GANCS *) task;

    if ((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
        handleGattManagerMsg(ancs, id, payload);
    else
        handleInternalAncsMsg(ancs, id, payload);
}
