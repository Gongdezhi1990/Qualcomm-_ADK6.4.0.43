/* Copyright (c) 2014 - 2016 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_battery_server_private.h"

#include "gatt_battery_server_db.h"


/****************************************************************************/
bool GattBatteryServerSendLevelNotification(const GBASS *battery_server, uint16 number_cids, const uint16 *cids, uint8 battery_level)
{
    bool result = FALSE;
    uint16 index = 0;

    if ((battery_server != NULL) && (battery_level <= 100) && number_cids)
    {
        if (battery_server->notifications_enabled)
        {
            result = TRUE;

            for (index = 0; index < number_cids; index++)
            {
                if (cids[index] == 0)
                {
                    /* CID must be non-zero */
                    result = FALSE;
                }
                else
                {
                    /* Send notification to GATT Manager */
                    GattManagerRemoteClientNotify((Task)&battery_server->lib_task, cids[index], HANDLE_BATTERY_LEVEL, 1, &battery_level);
                }
            }
            
        }
        else
        {
            result = FALSE;
        }
    }

    return result;
}

