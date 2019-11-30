/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gap_server_access.c

DESCRIPTION
    The GATT "GAP Server" access routines.
    
NOTES

*/


/* Firmware headers */
#include <csrtypes.h>
#include <stdlib.h>

/* "GATT Server" headers */
#include "gatt_gap_server_private.h"

#include "gatt_gap_server_access.h"
#include "gatt_gap_server_api.h"
#include "gatt_gap_server_db.h"

/* External lib headers */
#include "gatt_manager.h"
#include "gatt.h"


/******************************************************************************/
static bool handleGapServiceAccessRequest(Task gap_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    gatt_status_t status = gatt_status_failure;
    
    if (gap_task && ind)
    {
        if (ind->flags & ATT_ACCESS_READ)
        {
            status = gatt_status_success;
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            status = gatt_status_write_not_permitted;
        }
        else
        {
            status = gatt_status_request_not_supported;
        }
        
        GattManagerServerAccessResponse(gap_task, ind->cid, ind->handle, status, 0, 0);
        
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
static bool handleDeviceNameAccessRequest(Task gap_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    gatt_status_t status = gatt_status_failure;
    
    if (gap_task && ind)
    {
        if (ind->flags & ATT_ACCESS_READ)
        {
            if (gattGapServerSendReadDeviceNameInd((GGAPS*)gap_task, ind->cid, ind->offset))
            {
                status = gatt_status_success;
            }
            else
            {
                status = gatt_status_unlikely_error;
            }
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            status = gatt_status_write_not_permitted;
        }
        else
        {
            status = gatt_status_request_not_supported;
        }
        
        if (status != gatt_status_success)
        {
            GattManagerServerAccessResponse(gap_task, ind->cid, ind->handle, status, 0, 0);
        }
        
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
static bool handleDeviceAppearanceAccessRequest(Task gap_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    gatt_status_t status = gatt_status_failure;
    
    if (gap_task && ind)
    {
        if (ind->flags & ATT_ACCESS_READ)
        {
            if (gattGapServerSendReadAppearanceInd((GGAPS*)gap_task, ind->cid))
            {
                status = gatt_status_success;
            }
            else
            {
                status = gatt_status_unlikely_error;
            }
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            status = gatt_status_write_not_permitted;
        }
        else
        {
            status = gatt_status_request_not_supported;
        }
        
        if (status != gatt_status_success)
        {
            GattManagerServerAccessResponse(gap_task, ind->cid, ind->handle, status, 0, 0);
        }
        
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
bool gattGapServerHandleGattManagerAccessInd(Task gap_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    if (ind)
    {
        switch(ind->handle)
        {
            case HANDLE_GAP_SERVICE:
                return handleGapServiceAccessRequest(gap_task, ind);

            case HANDLE_DEVICE_NAME:
                return handleDeviceNameAccessRequest(gap_task, ind);

            case HANDLE_DEVICE_APPEARANCE:
                return handleDeviceAppearanceAccessRequest(gap_task, ind);

            default:
                /* ERROR */
                break;
        }
    }
    return FALSE;
}


/******************************************************************************/
gatt_gap_server_status_t GattGapServerReadDeviceNameResponse(GGAPS *gap_server,
                                                             uint16 cid,
                                                             uint16 size,
                                                             uint8 * data)
{
    gatt_status_t   gatt_status = gatt_status_success;
    
    if (!gap_server || !cid)
    {
        return gatt_gap_server_status_invalid_parameter;
    }

    /* Update parameters if we have no response to send */
    if (!size)
    {
        data = NULL;
        gatt_status = gatt_status_invalid_offset;
    }

    if (GattManagerServerAccessResponse(&gap_server->lib_task, 
                                        cid, 
                                        HANDLE_DEVICE_NAME, 
                                        gatt_status, 
                                        size, 
                                        data))
    {
        return gatt_gap_server_status_success;
    }

    return gatt_gap_server_status_failed;
}


/******************************************************************************/
gatt_gap_server_status_t GattGapServerReadAppearanceResponse(GGAPS *gap_server,
                                                             uint16 cid,
                                                             uint16 size,
                                                             uint8 * data)
{
    if (gap_server && cid)
    {
        if (GattManagerServerAccessResponse(&gap_server->lib_task, cid, 
                                            HANDLE_DEVICE_APPEARANCE, gatt_status_success, 
                                            size, data))
        {
            return gatt_gap_server_status_success;
        }
        return gatt_gap_server_status_failed;
    }
    return gatt_gap_server_status_invalid_parameter;
}
