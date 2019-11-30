/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client.h"
#include "gatt_apple_notification_client_msg_handler.h"
#include "gatt_apple_notification_client_private.h"
#include <gatt_manager.h>
#include <string.h>

static bool registerWithGattManager(GANCS *ancs, uint16 cid, uint16 start_handle, uint16 end_handle)
{
    gatt_manager_client_registration_params_t registration_params;

    registration_params.client_task = &ancs->lib_task;
    registration_params.cid = cid;
    registration_params.start_handle = start_handle;
    registration_params.end_handle = end_handle;

    return (GattManagerRegisterClient(&registration_params) == gatt_manager_status_success);
}

static void initAncsClient(GANCS *ancs, Task app_task, uint16 cid)
{
    memset(ancs, 0, sizeof(GANCS));
    ancs->lib_task.handler = appleNotificationClientMsgHandler;
    ancs->app_task = app_task;
    ancs->cid = cid;
    ancs->notification_source = GATT_ANCS_INVALID_HANDLE;
    ancs->data_source = GATT_ANCS_INVALID_HANDLE;
    ancs->control_point = GATT_ANCS_INVALID_HANDLE;
    ancs->ns_ccd = GATT_ANCS_INVALID_HANDLE;
    ancs->ds_ccd = GATT_ANCS_INVALID_HANDLE;
}

gatt_ancs_status_t GattAncsInit(GANCS *ancs, Task app_task, uint16 cid, uint16 start_handle, uint16 end_handle)
{
    if ((app_task == NULL) || (ancs == NULL))
        return gatt_ancs_status_invalid_parameter;

    initAncsClient(ancs, app_task, cid);

    if (registerWithGattManager(ancs, cid, start_handle, end_handle) == FALSE)
        return gatt_ancs_status_failed;

    GattManagerDiscoverAllCharacteristics(&ancs->lib_task);
    ancs->pending_cmd = ancs_pending_discover_all_characteristics;

    PRINT(("ANCS: Initialised instance [%p] with cid [0x%02x]\n", (void *) ancs, ancs->cid));

    return gatt_ancs_status_initiating;
}
