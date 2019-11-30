/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include <string.h>
#include <stdio.h>

#include "gatt_battery_server_private.h"

#include "gatt_battery_server_access.h"


/****************************************************************************/
bool GattBatteryServerReadLevelResponse(const GBASS *battery_server, uint16 cid, uint8 battery_level)
{
    gatt_status_t status = gatt_status_failure;

    if (battery_server == NULL)
    {
        return FALSE;
    }

    if (battery_level <= 100)
    {
        status = gatt_status_success;
    }
    else
    {
        status = gatt_status_insufficient_resources;
    }
    sendBatteryLevelAccessRsp(battery_server, cid, battery_level, status);

    return TRUE;
}

/****************************************************************************/
bool GattBatteryServerReadClientConfigResponse(const GBASS *battery_server, uint16 cid, uint16 client_config)
{
    if (battery_server == NULL)
    {
        return FALSE;
    }

    sendBatteryConfigAccessRsp(battery_server, cid, client_config);

    return TRUE;
}

/****************************************************************************/
bool GattBatteryServerReadPresentationResponse(const GBASS *battery_server, uint16 cid, uint8 name_space, uint16 description)
{
    if (battery_server == NULL)
    {
        return FALSE;
    }

    sendBatteryPresentationAccessRsp(battery_server, cid, name_space, description);
    
    return TRUE;
}
