/* Copyright (c) 2019 Qualcomm Technologies International, Ltd. */

#include "gatt_apple_notification_client_ready_state.h"

static gatt_ancs_ready_state_observer_t observer_list;

void gattAncsReadyStateUpdate(const GANCS *ancs, bool is_ready)
{
    if (observer_list)
        observer_list(ancs, is_ready);
}

bool GattAncsAddReadyStateObserver(gatt_ancs_ready_state_observer_t observer)
{
    if ((observer_list != NULL) || (observer == NULL))
        return FALSE;

    observer_list = observer;

    return TRUE;
}
