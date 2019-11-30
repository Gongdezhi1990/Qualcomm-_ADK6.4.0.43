/* Copyright (c) 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client_external_msg_send.h"
#include "gatt_apple_notification_client_private.h"
#include <stdlib.h>
#include <string.h>

static void sendResponseToCaller(const GANCS *ancs, MessageId id, void *message)
{
    Task caller_task = ancs->task_pending_cfm;

    if (caller_task)
        MessageSend(caller_task, id, message);
    else
        free(message);
}

static gatt_ancs_status_t getAncsStatusFromGattStatus(gatt_status_t gatt_status)
{
    switch(gatt_status)
    {
        case gatt_status_success:
            return gatt_ancs_status_success;
        case gatt_status_request_not_supported:
            return gatt_ancs_status_not_supported;
        case 0xA0:
            return gatt_ancs_status_unknown_command;
        case 0xA1:
            return gatt_ancs_status_invalid_command;
        case 0xA2:
            return gatt_ancs_status_invalid_parameter;
        case 0xA3:
            return gatt_ancs_status_action_failed;
        default:
            return gatt_ancs_status_failed;
    }
}

static void setClientToNoPendingCommandState(GANCS *ancs)
{
    ancs->pending_cmd = ancs_pending_none;
}

static gatt_ancs_command_id getAncsCommandId(ancs_pending_cmd_t pending_cmd)
{
    switch (pending_cmd)
    {
        case ancs_pending_write_cp_attr:
            return gatt_ancs_notification_attr;
        case ancs_pending_write_cp_app_attr:
            return gatt_ancs_notification_app_attr;
        /* We do not know the command Id the caller used (callers responsibility) */
        case ancs_pending_write_cp:
            return gatt_ancs_reserved;
        default:
            return gatt_ancs_notification_action;
    }
}

void gattAncsSendSetNotificationSourceNotificationResponse(GANCS *ancs, gatt_status_t gatt_status)
{
     MAKE_APPLE_NOTIFICATION_MESSAGE(GATT_ANCS_SET_NS_NOTIFICATION_CFM);
     memset(message, 0, sizeof(GATT_ANCS_SET_NS_NOTIFICATION_CFM_T));
     message->ancs = ancs;
     message->cid  = ancs->cid;
     message->status = getAncsStatusFromGattStatus(gatt_status);
     message->gatt_status = gatt_status;
     sendResponseToCaller(ancs, GATT_ANCS_SET_NS_NOTIFICATION_CFM, message);

     setClientToNoPendingCommandState(ancs);
}

void gattAncsSendSetDataSourceNotificationResponse(GANCS *ancs, gatt_status_t gatt_status)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE(GATT_ANCS_SET_DS_NOTIFICATION_CFM);
    memset(message, 0, sizeof(GATT_ANCS_SET_DS_NOTIFICATION_CFM_T));
    message->ancs = ancs;
    message->cid  = ancs->cid;
    message->status = getAncsStatusFromGattStatus(gatt_status);
    message->gatt_status = gatt_status;
    sendResponseToCaller(ancs, GATT_ANCS_SET_DS_NOTIFICATION_CFM, message);

    setClientToNoPendingCommandState(ancs);
}

void gattAncsSendWriteControlPointResponse(GANCS *ancs, gatt_status_t gatt_status)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE(GATT_ANCS_WRITE_CP_CFM);
    memset(message, 0, sizeof(GATT_ANCS_WRITE_CP_CFM_T));
    message->ancs = ancs;
    message->cid  = ancs->cid;
    message->command_id = getAncsCommandId(ancs->pending_cmd);
    message->status = getAncsStatusFromGattStatus(gatt_status);
    message->gatt_status = gatt_status;
    sendResponseToCaller(ancs, GATT_ANCS_WRITE_CP_CFM, message);

    setClientToNoPendingCommandState(ancs);
}

void gattAncsSendNotificationSourceInd(GANCS *ancs, uint8 event_id, uint8 event_flag, uint8 category_id, uint8 category_count, uint32 notification_uid)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE(GATT_ANCS_NS_IND);
    memset(message, 0, sizeof(GATT_ANCS_NS_IND_T));
    message->ancs = ancs;
    message->event_id = event_id;
    message->event_flag = event_flag;
    message->category_id = category_id;
    message->category_count = category_count;
    message->notification_uid = notification_uid;
    MessageSend(ancs->app_task, GATT_ANCS_NS_IND, message);
}

void gattAncsSendDataSourceAttributesInd(GANCS *ancs, uint32 notification_uid, const uint8 *attributes_data, uint16 attributes_data_size)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE_WITH_LEN(GATT_ANCS_DS_ATTR_IND, attributes_data_size);
    message->ancs = ancs;
    message->notification_uid = notification_uid;
    message->size_value = attributes_data_size;
    memcpy(message->value, attributes_data, attributes_data_size);
    MessageSend(ancs->app_task, GATT_ANCS_DS_ATTR_IND, message);
}

void gattAncsSendDataSourceAppAttributesInd(GANCS *ancs, uint16 value_size, const uint8 *value)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE_WITH_LEN(GATT_ANCS_DS_APP_ATTR_IND, value_size);
    message->ancs = ancs;
    message->size_value = value_size;
    memcpy(message->value, value, value_size);
    MessageSend(ancs->app_task, GATT_ANCS_DS_APP_ATTR_IND, message);
}

void gattAncsSendInitResponse(GANCS *ancs, gatt_status_t gatt_status)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE(GATT_ANCS_INIT_CFM);
    memset(message, 0, sizeof(GATT_ANCS_INIT_CFM_T));
    message->ancs = ancs;
    message->status = getAncsStatusFromGattStatus(gatt_status);
    MessageSend(ancs->app_task, GATT_ANCS_INIT_CFM, message);

    setClientToNoPendingCommandState(ancs);
}
