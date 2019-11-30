/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client.h"
#include "gatt_apple_notification_client_private.h"
#include <string.h>

void GattAncsSetNSNotificationEnableRequest(const GANCS *ancs, bool notifications_enable, uint16 notification_mask)
{
    GattAncsSetNotificationSourceNotificationEnableRequest(ancs->app_task, ancs, notifications_enable, notification_mask);
}

void GattAncsSetNotificationSourceNotificationEnableRequest(Task task, const GANCS *ancs, bool notifications_enable, uint16 notification_mask)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE(ANCS_INTERNAL_MSG_SET_NS_NOTIFICATION);

    PRINT(("ANCS: Notification Source, enable notifications request = %u, notifications mask [0x%04x]\n", notifications_enable, notification_mask));

    memset(message, 0, sizeof(ANCS_INTERNAL_MSG_SET_NS_NOTIFICATION_T));
    message->notifications_enable = notifications_enable;
    message->notifications_mask = notification_mask;
    message->task_pending_cfm = task;
    MessageSendConditionally((Task) &ancs->lib_task, ANCS_INTERNAL_MSG_SET_NS_NOTIFICATION, message, &ancs->pending_cmd);
}

void GattAncsSetDSNotificationEnableRequest(const GANCS *ancs, bool notifications_enable)
{
    GattAncsSetDataSourceNotificationEnableRequest(ancs->app_task, ancs, notifications_enable);
}

void GattAncsSetDataSourceNotificationEnableRequest(Task task, const GANCS *ancs, bool notifications_enable)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE(ANCS_INTERNAL_MSG_SET_DS_NOTIFICATION);

    PRINT(("ANCS: Data Source, enable notifications request = %u\n", notifications_enable));

    memset(message, 0, sizeof(ANCS_INTERNAL_MSG_SET_DS_NOTIFICATION_T));
    message->notifications_enable = notifications_enable;
    message->task_pending_cfm = task;
    MessageSendConditionally((Task) &ancs->lib_task, ANCS_INTERNAL_MSG_SET_DS_NOTIFICATION, message, &ancs->pending_cmd);
}

void GattAncsGetNotificationAttributes(const GANCS *const ancs, uint32 notification_uid, uint16 size_attribute_list, uint8 *attribute_list)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE_WITH_LEN(ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, CALCULATE_SIZEOF_GET_NOTIFICATION_ATTRIBUTES(size_attribute_list));
    message->task_pending_cfm = ancs->app_task;
    message->pending_cmd = ancs_pending_write_cp_attr;
    message->size_command_data = CALCULATE_SIZEOF_GET_NOTIFICATION_ATTRIBUTES(size_attribute_list);
    message->command_data[0] = gatt_ancs_notification_attr;
    message->command_data[4] = notification_uid & 0xFF;
    message->command_data[3] = (notification_uid >> 8) & 0xFF;
    message->command_data[2] = (notification_uid >> 16) & 0xFF;
    message->command_data[1] = (notification_uid >> 24) & 0xFF;
    memcpy(&message->command_data[5], attribute_list, size_attribute_list);
    MessageSendConditionally((Task) &ancs->lib_task, ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, message, &ancs->pending_cmd);
}

void GattAncsGetAppAttributes(const GANCS *const ancs, uint16 size_app_id, uint8 * app_id, uint16 size_attribute_list, uint8 *attribute_list)
{
    /* Only if valid app_id */
    if(size_app_id)
    {
        MAKE_APPLE_NOTIFICATION_MESSAGE_WITH_LEN(ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, CALCULATE_SIZEOF_GET_APP_ATTRIBUTES(size_app_id, size_attribute_list));
        message->task_pending_cfm = ancs->app_task;
        message->pending_cmd = ancs_pending_write_cp_app_attr;
        message->size_command_data = CALCULATE_SIZEOF_GET_APP_ATTRIBUTES(size_app_id, size_attribute_list);
        message->command_data[0] = gatt_ancs_notification_app_attr;
        memcpy(&message->command_data[1], app_id, size_app_id);
        memcpy(&message->command_data[1+size_app_id], attribute_list, size_attribute_list);
        MessageSendConditionally((Task) &ancs->lib_task, ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, message, &ancs->pending_cmd);
    }
}

void GattAncsPerformNotificationAction(const GANCS *const ancs, uint32 notification_uid, gatt_ancs_action_id action_id)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE_WITH_LEN(ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, CALCULATE_SIZEOF_PERFORM_NOTIFICATION_ACTION);
    message->task_pending_cfm = ancs->app_task;
    message->pending_cmd = ancs_pending_none;
    message->size_command_data = CALCULATE_SIZEOF_PERFORM_NOTIFICATION_ACTION;
    message->command_data[0] = gatt_ancs_notification_action;
    message->command_data[4] = notification_uid & 0xFF;
    message->command_data[3] = (notification_uid >> 8) & 0xFF;
    message->command_data[2] = (notification_uid >> 16) & 0xFF;
    message->command_data[1] = (notification_uid >> 24) & 0xFF;
    message->command_data[5] = action_id;
    MessageSendConditionally((Task) &ancs->lib_task, ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, message, &ancs->pending_cmd);
}

void GattAncsWriteControlPoint(Task task, const GANCS *ancs, const uint8 *value, uint16 value_size)
{
    MAKE_APPLE_NOTIFICATION_MESSAGE_WITH_LEN(ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, value_size);
    message->task_pending_cfm = task;
    message->pending_cmd = ancs_pending_write_cp;
    message->size_command_data = value_size;
    memcpy(message->command_data, value, value_size);
    MessageSendConditionally((Task) &ancs->lib_task, ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC, message, &ancs->pending_cmd);
}

uint16 GattAncsGetConnectionId(const GANCS *ancs)
{
    return ancs->cid;
}

uint16 GattAncsGetNotificationSourceHandle(const GANCS *ancs)
{
    return ancs->notification_source;
}

uint16 GattAncsGetDataSourceHandle(const GANCS *ancs)
{
    return ancs->data_source;
}
