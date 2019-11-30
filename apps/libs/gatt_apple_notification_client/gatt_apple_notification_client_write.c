/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client_write.h"
#include "gatt_apple_notification_client_external_msg_send.h"

static void handleWriteClientConfigResp(GANCS *ancs, const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *write_cfm)
{
    switch(ancs->pending_cmd)
    {
        case ancs_pending_write_ns_cconfig:
            gattAncsSendSetNotificationSourceNotificationResponse(ancs, write_cfm->status);
            break;

        case ancs_pending_write_ds_cconfig:
            gattAncsSendSetDataSourceNotificationResponse(ancs, write_cfm->status);
            break;

        default:
        break;
    }
}

static void writeClientValue(GANCS *ancs, uint16 handle, uint16 size_value, const uint8* value)
{
    GattManagerWriteCharacteristicValue(&ancs->lib_task, handle, size_value, value);
}

void writeClientConfigNotifyValue(GANCS *ancs, bool notifications_enable, uint16 handle)
{
    uint8 value[2];
    
    value[1] = 0;
    value[0] = notifications_enable ? gatt_client_char_config_notifications_enabled : 0;
    
    writeClientValue(ancs, handle, sizeof(value), value);
}

void ancsWriteCharValue(GANCS *ancs, const ANCS_INTERNAL_MSG_WRITE_CP_CHARACTERISTIC_T* req, uint16 handle)
{
    writeClientValue(ancs, handle, req->size_command_data, req->command_data);
}

void handleAncsWriteValueResp(GANCS *ancs, const GATT_MANAGER_WRITE_CHARACTERISTIC_VALUE_CFM_T *write_cfm)
{
    switch (ancs->pending_cmd)
    {
        case ancs_pending_write_ns_cconfig:
        case ancs_pending_write_ds_cconfig:
        {
            handleWriteClientConfigResp(ancs, write_cfm);
        }
        break;

        case ancs_pending_write_cp_attr:
        case ancs_pending_write_cp_app_attr:
        case ancs_pending_write_cp:
            gattAncsSendWriteControlPointResponse(ancs, write_cfm->status);
            break;

        case ancs_pending_none:
            PRINT(("ANCS: Write value response not expected [0x%x]\n", ancs->pending_cmd));

            break;
            
        default:
        {
            /* No other pending write values expected */
            DEBUG_PANIC(("ANCS: Write value response not expected [0x%x]\n", ancs->pending_cmd));
        }
        break;
    }
}
