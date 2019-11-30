/* Copyright (c) 2015 Qualcomm Technologies International, Ltd. */
/*  */

#include "gatt_client_private.h"

#include <string.h>
#include <stdio.h>

#include <gatt_manager.h>


/****************************************************************************/
gatt_client_status_t GattClientDestroy(GGATTC *const gatt_client, uint16 cid)
{
    gatt_client_status_t result = gatt_client_status_failed;

    UNUSED(cid);

    /* Check parameters */
    if (gatt_client == NULL)
    {
        return gatt_client_status_invalid_parameter;
    }
    
    /* Register with the GATT Manager and verify the result */
    if (GattManagerUnregisterClient(&gatt_client->lib_task) == gatt_manager_status_success)
    {
        result = gatt_client_status_success;
    }
    
    /* Clear pending messages */
    MessageFlushTask((Task)&gatt_client->lib_task);
    
    return result;
}
