/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_server_indication.c

DESCRIPTION
    The "GATT Server" Indication routines; for sending indications
    on the GATT_SERVICE_CHANGED characteristic.
    
NOTES

*/

/* Firmware headers */
#include <stdlib.h>

/* "GATT Server" headers */
#include "gatt_server_private.h"

#include "gatt_server_indication.h"
#include "gatt_server_api.h"
#include "gatt_server_db.h"

/* External lib headers */
#include "gatt.h"
#include "gatt_manager.h"



/******************************************************************************/
bool GattServerSendServiceChangedIndication(GGATTS *gatt_server, uint16 cid)
{
    if (gatt_server)
    {
        /* Service changed, set the "changed range" to the full database */
        uint8 value[4] = {0x00, 0x00, 0xFF, 0xFF};
        GattManagerRemoteClientIndicate(&gatt_server->lib_task, cid, HANDLE_GATT_SERVICE_CHANGED, 4, value);
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
bool gattServerHandleGattManagerIndicationCfm(Task gatt_task, const GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM_T * cfm)
{
    if (gatt_task && cfm)
    {
        GGATTS *gatt_server = (GGATTS*)gatt_task;
        return gattServerSendServiceChangedIndicationCfm(gatt_server->app_task, cfm->cid, cfm->status);
    }
    return FALSE;
}
