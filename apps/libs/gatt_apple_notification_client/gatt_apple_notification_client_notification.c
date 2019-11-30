/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client_notification.h"
#include "gatt_apple_notification_client_private.h"
#include "gatt_apple_notification_client_write.h"
#include "gatt_apple_notification_client_external_msg_send.h"
#include "gatt_apple_notification_client_discover.h"

static bool isPendingCmdWriteControlPointCharacteristic(const GANCS *ancs)
{
    ancs_pending_cmd_t pending_cmd = ancs->pending_cmd;

    return (pending_cmd == ancs_pending_write_cp_attr) ||
           (pending_cmd == ancs_pending_write_cp_app_attr) ||
           (pending_cmd == ancs_pending_write_cp);
}

static bool isStringNullTerminated(const uint8 *string, unsigned string_size)
{
    unsigned i;

    for(i = 0; i < string_size; i++)
    {
        if (string[i] == 0)
            return TRUE;
    }

    return FALSE;
}

static bool isAppIdValid(const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *ind)
{
    return isStringNullTerminated(&ind->value[1], ind->size_value - 1);
}

static uint16 getCharacteristicHandle(GANCS *ancs, uint8 characteristic)
{
    switch(characteristic)
    {
        case GATT_APPLE_NOTIFICATION_NS:
            return ancs->notification_source;
        case GATT_APPLE_NOTIFICATION_DS:
            return ancs->data_source;
        case GATT_APPLE_NOTIFICATION_CP:
            return ancs->control_point;
        default:
            PANIC(("ANCS: Unknown characteristic [0x%04x]\n", characteristic));
            return GATT_ANCS_INVALID_HANDLE;
    }
}

static bool discoverCharacteristicDescriptors(GANCS *ancs, uint8 characteristic)
{
    uint16 startHandle, endHandle;
    gatt_manager_client_service_data_t client_data;

    client_data.start_handle = GATT_ANCS_INVALID_HANDLE;
    client_data.end_handle = GATT_ANCS_INVALID_HANDLE;

    if (!GattManagerGetClientData(&ancs->lib_task, &client_data))
        PANIC(("ANCS: Could not get client data\n"));

    startHandle = MIN(getCharacteristicHandle(ancs, characteristic) + 1, client_data.end_handle);
    endHandle = findEndHandleForCharDesc(ancs, startHandle, client_data.end_handle, characteristic);

    if(startHandle && endHandle)
        return ancsDiscoverAllCharacteristicDescriptors(ancs, startHandle, endHandle);
    else
        return FALSE;
}

void handleAncsNotification(GANCS *ancs, const GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T *ind)
{
    /* first validate the payload */
    if (ind->size_value) 
    {
        /* This is notification for NS */
        if(ind->handle == ancs->notification_source)
        {
            /* Check if application has set interest in this category */
            if(CHECK_CATEGORY(ancs->ns_notification_mask, ind->value[2]))
            {
                uint32 notification_uid = ((uint32)ind->value[4] << 24) | ((uint32)ind->value[5] << 16) | ((uint32)ind->value[6] << 8) | ((uint32)ind->value[7]);
                gattAncsSendNotificationSourceInd(ancs, ind->value[0], ind->value[1], ind->value[2], ind->value[3], notification_uid);
            }
        }
        /* This is notification for DS */
       /* NOTE: According to Apple Notification Service Client Specification Version:1.1
        * " As with a response to a Get Notification Attributes command, if the response to a Get App Attributes command
            is larger than the negotiated GATT Maximum Transmission Unit (MTU), it is split into multiple fragments by
            the NP. The NC must recompose the response by splicing each fragment. The response is complete when the
            complete tuples for each requested attribute has been received. "
            Here the parsing is done with the assumtion that each of the multiple fragmented packet shall still have the 
            header information like command_id, App Identifier or command_id, NotificationUID followed by the fragmented part.
            This assumption has to be taken because, none of the packet tells the total length of the entire indication.
            */
        else if(ind->handle == ancs->data_source)
        {
            if(isPendingCmdWriteControlPointCharacteristic(ancs))
                gattAncsSendWriteControlPointResponse(ancs, gatt_status_success);

            /* The notification can be either for attribute or application attribute */
            switch(ind->value[0])
            {
                case gatt_ancs_notification_attr:
                {
                    uint32 notification_uid = ((uint32)ind->value[1] << 24) | ((uint32)ind->value[2] << 16) | ((uint32)ind->value[3] << 8) | ((uint32)ind->value[4]);
                    gattAncsSendDataSourceAttributesInd(ancs, notification_uid, &ind->value[5], ind->size_value - 5);
                }
                break;

                case gatt_ancs_notification_app_attr:
                {
                    if(isAppIdValid(ind))
                        gattAncsSendDataSourceAppAttributesInd(ancs, ind->size_value - 1, &ind->value[1]);
                }
                break;

                default:
                break;
            }
        }
    }
}

void ancsSetNotificationRequest(GANCS *ancs, bool notifications_enable, uint8 characteristic)
{
    switch(characteristic)
    {
        case GATT_APPLE_NOTIFICATION_NS:
            if (CHECK_VALID_HANDLE(ancs->notification_source))
            {
                /* First check if ccd handle is found, else find it */
                if (CHECK_VALID_HANDLE(ancs->ns_ccd))
                {
                    PRINT(("ANCS: Write Notification Source config\n"));
                    writeClientConfigNotifyValue(ancs, notifications_enable, ancs->ns_ccd);
                    ancs->pending_cmd = ancs_pending_write_ns_cconfig;
                }
                else
                {
                    PRINT(("ANCS: DiscoverAllCharacteristicDescriptors for Notification Source\n"));
                    if (discoverCharacteristicDescriptors(ancs, GATT_APPLE_NOTIFICATION_NS))
                    {
                        if (notifications_enable)
                            ancs->pending_cmd = ancs_pending_set_ns_notify_enable;
                        else
                            ancs->pending_cmd = ancs_pending_set_ns_notify_disable;
                    }
                    else
                        gattAncsSendSetNotificationSourceNotificationResponse(ancs, gatt_status_failure);
                }
            }
            else
                gattAncsSendSetNotificationSourceNotificationResponse(ancs, gatt_status_request_not_supported);
        break;

        case GATT_APPLE_NOTIFICATION_DS:
            if (CHECK_VALID_HANDLE(ancs->data_source))
            {
                /* First check if the ccd handle is found, else find it */
                if (CHECK_VALID_HANDLE(ancs->ds_ccd))
                {
                    PRINT(("ANCS: Write Data Source config\n"));
                    writeClientConfigNotifyValue(ancs, notifications_enable, ancs->ds_ccd);
                    ancs->pending_cmd = ancs_pending_write_ds_cconfig;
                }
                else
                {
                    PRINT(("ANCS: DiscoverAllCharacteristicDescriptors for Data Source\n"));
                    if (discoverCharacteristicDescriptors(ancs, GATT_APPLE_NOTIFICATION_DS))
                    {
                        if (notifications_enable)
                            ancs->pending_cmd = ancs_pending_set_ds_notify_enable;
                        else
                            ancs->pending_cmd = ancs_pending_set_ds_notify_disable;
                    }
                    else
                        gattAncsSendSetDataSourceNotificationResponse(ancs, gatt_status_failure);
                }
            }
            else
                gattAncsSendSetDataSourceNotificationResponse(ancs, gatt_status_request_not_supported);
        break;

        default:
            PANIC(("ANCS: Not a valid characteristic for notifications"));
        break;
    }
}
