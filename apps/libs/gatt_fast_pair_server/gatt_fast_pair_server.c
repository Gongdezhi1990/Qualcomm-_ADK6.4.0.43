/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/

#include "gatt_fast_pair_server_private.h"
#include "gatt_fast_pair_server_msg_handler.h"


/******************************************************************************/
bool GattFastPairServerInit(
        GFPS    *fast_pair_server,
        Task    app_task,
        uint16  start_handle,
        uint16  end_handle
        )
{
    gatt_manager_server_registration_params_t reg_params;
    
    if ((app_task == NULL) || (fast_pair_server == NULL ))
    {
        GATT_FAST_PAIR_SERVER_PANIC((
                    "GFPS: Invalid Initialization parameters"
                    ));
    }

    /* Reset all the service library memory */
    memset(fast_pair_server, 0, sizeof(GFPS));

    fast_pair_server->lib_task.handler = fpsServerMsgHandler;
    fast_pair_server->app_task = app_task;

    reg_params.start_handle = start_handle;
    reg_params.end_handle = end_handle;
    reg_params.task = &fast_pair_server->lib_task;

    if (GattManagerRegisterServer(&reg_params) == gatt_manager_status_success)
    {
        return TRUE;
    }
    return FALSE;
}


/******************************************************************************/
static bool gattFastPairServerWriteGenericResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result,
        uint16      handle
        )
{
    if (fast_pair_server == NULL)
    {
        GATT_FAST_PAIR_SERVER_DEBUG_PANIC((
                    "GFPS: Null instance!\n"
                    ));
    }
    else if (cid == 0)
    {
        return FALSE;
    }

    sendFpsServerAccessRsp(
        (Task)&fast_pair_server->lib_task,
        cid,
        handle,
        result,
        0,
        NULL
        );

    return TRUE;
}


/******************************************************************************/
bool GattFastPairServerWriteKeybasedPairingResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        )
{
    return gattFastPairServerWriteGenericResponse(
            fast_pair_server, 
            cid, 
            result,
            HANDLE_KEYBASED_PAIRING
            );
}


/******************************************************************************/
static bool gattFastPairServerGenericReadConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      client_config,
        uint16      handle
        )
{
    uint8 config_data[CLIENT_CONFIG_VALUE_SIZE];

    if (fast_pair_server == NULL)
    {
        GATT_FAST_PAIR_SERVER_DEBUG_PANIC((
                    "GFPS: Null instance!\n"
                    ));
    }
    else if (cid == 0)
    {
        return FALSE;
    }

    config_data[0] = (uint8)client_config & 0xFF;
    config_data[1] = (uint8)client_config >> 8;

    sendFpsServerAccessRsp(
        (Task)&fast_pair_server->lib_task,
        cid,
        handle,
        gatt_status_success,
        2,
        config_data
        );

    return TRUE;
}

/******************************************************************************/
bool GattFastPairServerReadKeybasedPairingConfigResponse(
        const GFPS  *fast_pair_server,
        uint16      cid,
        uint16      client_config
        )
{
    return gattFastPairServerGenericReadConfigResponse(
            fast_pair_server,
            cid,
            client_config,
            HANDLE_KEYBASED_PAIRING_CLIENT_CONFIG
            );
}


/******************************************************************************/
bool GattFastPairServerWriteKeybasedPairingConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        )
{
    return gattFastPairServerWriteGenericResponse(
            fast_pair_server, 
            cid, 
            result,
            HANDLE_KEYBASED_PAIRING_CLIENT_CONFIG
            );
}


/******************************************************************************/
static bool gattFastPairServerGenericNotification(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint8       value[FAST_PAIR_VALUE_SIZE],
        uint16      handle
        )
{
    if (fast_pair_server == NULL)
    {
        GATT_FAST_PAIR_SERVER_DEBUG_PANIC((
                    "GFPS: Null instance!\n"
                    ));
    }
    else if (cid == 0)
    {
        return FALSE;
    }

    GattManagerRemoteClientNotify(
        (Task)&fast_pair_server->lib_task,
        cid,
        handle,
        FAST_PAIR_VALUE_SIZE,
        value
        );

    return TRUE;
}


/******************************************************************************/
bool GattFastPairServerKeybasedPairingNotification(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint8       value[FAST_PAIR_VALUE_SIZE]
        )
{
    return gattFastPairServerGenericNotification(
            fast_pair_server, 
            cid, 
            value ,
            HANDLE_KEYBASED_PAIRING
            );
}
 

/******************************************************************************/
bool GattFastPairServerWritePasskeyResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        )
{
    return gattFastPairServerWriteGenericResponse(
            fast_pair_server, 
            cid, 
            result,
            HANDLE_PASSKEY
            );
}


/******************************************************************************/
bool GattFastPairServerReadPasskeyConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      client_config
        )
{
    return gattFastPairServerGenericReadConfigResponse(
            fast_pair_server, 
            cid, 
            client_config,
            HANDLE_PASSKEY_CLIENT_CONFIG
            );
}


/******************************************************************************/
bool GattFastPairServerWritePasskeyConfigResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        )
{
    return gattFastPairServerWriteGenericResponse(
            fast_pair_server, 
            cid, 
            result,
            HANDLE_PASSKEY_CLIENT_CONFIG
            );
}


/******************************************************************************/
bool GattFastPairServerPasskeyNotification(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint8       value[FAST_PAIR_VALUE_SIZE]
        )
{
    return gattFastPairServerGenericNotification(
            fast_pair_server, 
            cid, 
            value ,
            HANDLE_PASSKEY
            );
}


/******************************************************************************/
bool GattFastPairServerWriteAccountKeyResponse(
        const GFPS  *fast_pair_server, 
        uint16      cid, 
        uint16      result
        )
{
    return gattFastPairServerWriteGenericResponse(
            fast_pair_server, 
            cid, 
            result,
            HANDLE_ACCOUNT_KEY
            );
}
