/*******************************************************************************
Copyright (c) 2014 - 2019 Qualcomm Technologies International, Ltd.


FILE NAME
    gatt_gatt_server_access.c

DESCRIPTION
    The GATT "GATT Server" access routines.
    
NOTES

*/


/* Firmware headers */
#include <stdlib.h>

/* "GATT Server" headers */
#include "gatt_server_private.h"

#include "gatt_server_access.h"
#include "gatt_server_api.h"
#include "gatt_server_db.h"

/* External lib headers */
#include "gatt_manager.h"
#include "gatt.h"


/*******************************************************************************
NAME
    handleGattServiceAccessRequest
    
DESCRIPTION
    Handle GATT_MANAGER_SERVER_ACCESS_IND message when it is for the Service of the
    "GATT Service"
    
PARAMETERS
    gatt_task   The "GATT Server" task.
    ind         Pointer to a GATT_MANAGER_SERVER_ACCESS_IND message that needs to be handled.
    
RETURN
    TRUE if the message was handled, FALSE otherwise.
*/
static bool handleGattServiceAccessRequest(Task gatt_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    if (gatt_task && ind)
    {
        if (ind->flags & ATT_ACCESS_READ)
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_success, 0, 0);
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_write_not_permitted, 0, 0);
        }
        else
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_request_not_supported, 0, 0);
        }
        return TRUE;
    }
    return FALSE;
}


/*******************************************************************************
NAME
    handleGattServiceChangedAccessRequest
    
DESCRIPTION
    Handle GATT_MANAGER_SERVER_ACCESS_IND message when it is for the Service Changed 
    characteristic of the "GATT Service"
    
PARAMETERS
    gatt_task   The "GATT Server" task.
    ind         Pointer to a GATT_MANAGER_SERVER_ACCESS_IND message that needs to be handled.
    
RETURN
    TRUE if the message was handled, FALSE otherwise.
*/
static bool handleGattServiceChangedAccessRequest(Task gatt_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    if (ind && gatt_task)
    {
        if (ind->flags & ATT_ACCESS_READ)
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_read_not_permitted, 0, 0);
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_write_not_permitted, 0, 0);
        }
        else
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_request_not_supported, 0, 0);
        }
        return TRUE;
    }
    return FALSE;
}


/*******************************************************************************
NAME
    handleGattServiceChangedCCfgAccessRequest
    
DESCRIPTION
    Handle GATT_MANAGER_SERVER_ACCESS_IND message when it is for the Service Changed 
    client configuration descriptor of the "GATT Service"
    
PARAMETERS
    gatt_task   The "GATT Server" task.
    ind         Pointer to a GATT_MANAGER_SERVER_ACCESS_IND message that needs to be
                handled.
    
RETURN
    TRUE if the message was handled, FALSE otherwise.
*/
static bool handleGattServiceChangedCCfgAccessRequest(Task gatt_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    if (ind && gatt_task)
    {
        /* Pass the read request to the GATT library for handling. */
        if (ind->flags & ATT_ACCESS_READ)
        {
            uint8 val = 0;
            
            val = GattHandleServChangedIndRead(ind->cid);
            
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_success,
                                           CLIENT_CONFIG_ACCESS_SIZE, &val);
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            if(ind->size_value == CLIENT_CONFIG_ACCESS_SIZE)
            {
                uint16 value = ind->value[0];
                
                /* This characteristic only allows indications, not notifications. */
                if(value == 2 || value == 0)
                {
                    GattHandleServChangedIndWrite(value, ind->cid);
                    
                    GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_success, 0, NULL);
                }
                else
                {
                    GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_value_not_allowed, 0, NULL);
                }        
            }
            else
            {
            	GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_invalid_length, 0, NULL);
            }
        }
        else
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_request_not_supported, 0, NULL);
        }
        return TRUE;
    }
    return FALSE;
}

#ifdef GATT_CACHING
/*******************************************************************************
NAME
    handleGattClientSupportFeaturesAccessRequest
    
DESCRIPTION
    Handle GATT_MANAGER_SERVER_ACCESS_IND message when it is for the Client Supported 
    Features characteristic of the "GATT Service"
    
PARAMETERS
    gatt_task   The "GATT Server" task.
    ind         Pointer to a GATT_MANAGER_SERVER_ACCESS_IND message that needs to be
                handled.
    
RETURN
    TRUE if the message was handled, FALSE otherwise.
*/
static bool handleGattClientSupportFeaturesAccessRequest(Task gatt_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    if (ind && gatt_task)
    {
        uint8 val;

        /* Pass the read request to the application, the application must respond with
         * GattServerReadClientConfigResponse()
         * */
        if (ind->flags & ATT_ACCESS_READ)
        {            
            GattHandleCsfRead(ind->cid, &val);

            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_success, CLIENT_SUPPORTED_FEATURES_SIZE, &val);
        }
        else if (ind->flags & ATT_ACCESS_WRITE)
        {
            /* This check should be <=, but since only 0 is below 1, it's == for now. */
            if(ind->size_value == CLIENT_SUPPORTED_FEATURES_SIZE)
            {
                if (GattHandleCsfWrite(ind->value[0], ind->cid))
                {
                    GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_success, 0, NULL);
                }
                else
                {
                    GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_value_not_allowed, 0, NULL);
                }
            }
            else
            {
            	GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_invalid_length, 0, NULL);
            }
        }
        else
        {
            GattManagerServerAccessResponse(gatt_task, ind->cid, ind->handle, gatt_status_request_not_supported, 0, NULL);
        }
        return TRUE;
    }
    return FALSE;
}
#endif

/******************************************************************************/
bool gattServerHandleGattManagerAccessInd(Task gatt_task, const GATT_MANAGER_SERVER_ACCESS_IND_T *ind)
{
    if (ind)
    {
        switch(ind->handle)
        {
            case HANDLE_GATT_SERVICE:
                return handleGattServiceAccessRequest(gatt_task, ind);

            case HANDLE_GATT_SERVICE_CHANGED:
                return handleGattServiceChangedAccessRequest(gatt_task, ind);

            case HANDLE_GATT_SERVICE_CHANGED_CLIENT_CONFIG:
                return handleGattServiceChangedCCfgAccessRequest(gatt_task, ind);
#ifdef GATT_CACHING                
            case HANDLE_GATT_CLIENT_SUPPORTED_FEATURES:
                return handleGattClientSupportFeaturesAccessRequest(gatt_task, ind);
#endif
            default:
                /* ERROR */
                break;
        }
    }
    return FALSE;
}

/******************************************************************************/
bool GattServerReadClientConfigResponse(GGATTS *gatt_server, uint16 cid, uint16 handle, uint16 config)
{
    uint8 config_resp[CLIENT_CONFIG_ACCESS_SIZE];

    config_resp[0] = config & 0xFF;
    config_resp[1] = (config >> 8) & 0xFF;

    PanicNull((void*)gatt_server);
    return GattManagerServerAccessResponse(&gatt_server->lib_task, cid,
                                           handle, gatt_status_success,
                                           CLIENT_CONFIG_ACCESS_SIZE, config_resp);
}
