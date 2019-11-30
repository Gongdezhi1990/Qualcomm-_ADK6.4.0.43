/* Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client.h"
#include "gatt_apple_notification_client_private.h"
#include "gatt_apple_notification_client_ready_state.h"
#include <gatt_manager.h>

gatt_ancs_status_t GattAncsDestroy(GANCS *ancs)
{
    if (ancs == NULL)
        return gatt_ancs_status_invalid_parameter;

    if (GattManagerUnregisterClient(&ancs->lib_task) != gatt_manager_status_success)
        return gatt_ancs_status_failed;

    gattAncsReadyStateUpdate(ancs, FALSE);

    MessageFlushTask(&ancs->lib_task);

    PRINT(("ANCS: Destroyed instance [%p] with cid [0x%02x]\n", (void *) ancs, ancs->cid));

    return gatt_ancs_status_success;
}
