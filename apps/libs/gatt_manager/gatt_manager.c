/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#include <stdlib.h>
#include <string.h>

#include "gatt_manager_internal.h"
#include "gatt_manager_handler.h"
#include "gatt_manager_data.h"


/******************************************************************************
 *                      GATT MANAGER PUBLIC API                               *
 ******************************************************************************/

bool GattManagerInit(Task task)
{
    if (NULL == task)
    {
        return FALSE;
    }

    gattManagerDataInit(gattManagerMessageHandler, task);
    gattManagerDataInitialisationState_Registration();
    return TRUE;
}

void GattManagerDeInit(void)
{
    gattManagerDataDeInit();
}

bool GattManagerRegisterConstDB(const uint16* db_ptr, uint16 size)
{
    if (!gattManagerDataIsInit() ||
        gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_registration ||
        gattManagerDataGetDB() != NULL ||
        NULL == db_ptr ||
        0 == size)
    {
        return FALSE;
    }

    gattManagerDataSetConstDB(db_ptr, size);
    return TRUE;
}

void GattManagerRegisterWithGatt(void)
{
    if (!gattManagerDataIsInit())
    {
        registerWithGattCfm(gatt_manager_status_not_initialised);
    }

    if (gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_registration)
    {
        registerWithGattCfm(gatt_manager_status_wrong_state);
    }

    gattManagerDataInitialisationState_Registering();
    /* When using the const DB discard the const qualifier as the GattInit API
       doesn't support it. We could have discarded the qualifier earlier but it
       makes more sense to discard it at the point where we are forced to do
       so. */
    GattInit(gattManagerDataGetTask(), gattManagerDataGetDBSize(),
             (uint16 *)gattManagerDataGetDB());
}

void GattManagerDisconnectRequest(uint16 cid)
{
    /* CID should not be 0, so its better to check before sending the disconnect req */
    if(cid == 0)
    {
        GATT_MANAGER_PANIC(("GM: Invalid CID!"));
    }
    if (!gattManagerDataIsInit())
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"));
    }

    /* Everything looks fine, now we can proceed with the disconnect request */
    GattDisconnectRequest(cid);
}

void GattManagerIndicationResponse(uint16 cid)
{
    if (gattManagerDataIsInit())
    {
        GattIndicationResponse(cid);
    }
}

uint16 GattManagerGetServerDatabaseHandle(Task task, uint16 handle)
{
    return gattManagerDataGetServerDatabaseHandle(task, handle);
}
