/* Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/*  */

#include <vm.h>

#include "gatt_fast_pair_server_private.h"
#include "gatt_fast_pair_server_msg_handler.h"

#define KEYBASED_PAIRING_MIN_SIZE   (16)
#define KEYBASED_PAIRING_MAX_SIZE   (80)


/******************************************************************************/
static bool attBearerIsBLE(uint16 cid)
{
    tp_bdaddr tpaddr;

    if (VmGetBdAddrtFromCid(cid, &tpaddr) &&
        tpaddr.transport == TRANSPORT_BLE_ACL)
    {
        return TRUE;
    }
    return FALSE;
}

/******************************************************************************/
void sendFpsServerAccessRsp(
        Task task,
        uint16 cid,
        uint16 handle,
        uint16 result,
        uint16 size_value,
        const uint8 *value
        )
{
    if (
            !GattManagerServerAccessResponse(
                task, 
                cid, 
                handle, 
                result, 
                size_value, 
                value
                )
       )
    {
        GATT_FAST_PAIR_SERVER_DEBUG_PANIC((
                    "Couldn't send GattManagerServerAccessRsp\n"
                    ));

    }
}


/******************************************************************************/

#define sendFpsServerAccessErrorRsp(fast_pair_server, access_ind, error) \
    sendFpsServerAccessRsp( \
            (Task)&fast_pair_server->lib_task, \
            access_ind->cid, \
            access_ind->handle, \
            error, \
            0, \
            NULL \
            )


/******************************************************************************/
static void fpsHandleKeybasedPairingAccess(
        const GFPS *fast_pair_server,
        const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind
        )
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        sendFpsServerAccessErrorRsp(
                fast_pair_server,
                access_ind,
                gatt_status_read_not_permitted
                );
    }
    /* If ONLY flag is ATT_ACCESS_PERMISSION */
    else if (access_ind->flags == ATT_ACCESS_PERMISSION)
    {
        /* There are no Access Permissions so just respond with success. */
        sendFpsServerAccessRsp(
                (Task)&fast_pair_server->lib_task,
                access_ind->cid,
                access_ind->handle,
                gatt_status_success,
                0,
                NULL
                );

    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Value size must be 16 or 80-octets. */
        if (access_ind->size_value != KEYBASED_PAIRING_MIN_SIZE &&
            access_ind->size_value != KEYBASED_PAIRING_MAX_SIZE)
        {
            sendFpsServerAccessErrorRsp(
                    fast_pair_server, 
                    access_ind,
                    gatt_status_invalid_length
                    );
        }
        else
        {
            MAKE_FPS_MESSAGE_WITH_LEN(
                    GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND,
                    access_ind->size_value
                    );

            message->fast_pair_server = fast_pair_server;
            message->cid = access_ind->cid;
            message->size_value = access_ind->size_value;
            memmove(
                    message->value, 
                    access_ind->value, 
                    sizeof(uint8) * access_ind->size_value
                   );

            MessageSend(
                    fast_pair_server->app_task,
                    GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_IND,
                    message
                    );
        }
    }
    else
    {
        sendFpsServerAccessErrorRsp(
                fast_pair_server, 
                access_ind,
                gatt_status_request_not_supported
                );
    }
}


/******************************************************************************/
static void fpsHandleReadClientConfigAccess(
        const GFPS *fast_pair_server,
        const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind,
        MessageId id
        )
{
    /* Same structure for GATT_FAST_PAIR_SERVER_READ_PASSKEY_CONFIG_IND */
    MAKE_FPS_MESSAGE(
            GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND
            );

    message->fast_pair_server = fast_pair_server;
    message->cid = access_ind->cid;

    MessageSend(
            fast_pair_server->app_task,
            id,
            message);
}


/******************************************************************************/
static void fpsHandleWriteClientConfigAccess(
        const GFPS *fast_pair_server,
        const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind,
        MessageId id
        )
{
    if (access_ind->size_value != CLIENT_CONFIG_VALUE_SIZE)
    {
        sendFpsServerAccessErrorRsp(
                fast_pair_server, 
                access_ind,
                gatt_status_invalid_length
                );
    }
    else
    {
        /* Same structure for 
         * GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_CONFIG_IND 
         */
        MAKE_FPS_MESSAGE(
                GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND
                );

        message->fast_pair_server = fast_pair_server;
        message->cid = access_ind->cid;
        message->config_value = 
            access_ind->value[0] | access_ind->value[1] << 8;

        MessageSend(
                fast_pair_server->app_task,
                id,
                message
                );
    }
}


/******************************************************************************
 * Both have the same structure and value size
 ******************************************************************************/
static void fpsHandlePasskeyOrAccountKeyAccess(
        const GFPS *fast_pair_server,
        const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind,
        MessageId id
        )
{
    if (access_ind->flags & ATT_ACCESS_READ)
    {
        sendFpsServerAccessErrorRsp(
                fast_pair_server,
                access_ind,
                gatt_status_read_not_permitted
                );
    }
    /* If ONLY flag is ATT_ACCESS_PERMISSION */
    else if (access_ind->flags == ATT_ACCESS_PERMISSION)
    {
        /* There are no Access Permissions so just respond with success. */
        sendFpsServerAccessRsp(
                (Task)&fast_pair_server->lib_task,
                access_ind->cid,
                access_ind->handle,
                gatt_status_success,
                0,
                NULL
                );

    }
    else if (access_ind->flags & ATT_ACCESS_WRITE)
    {
        /* Passkey size is always 16-octets. */
        if (access_ind->size_value != FAST_PAIR_VALUE_SIZE)
        {
            sendFpsServerAccessErrorRsp(
                    fast_pair_server, 
                    access_ind,
                    gatt_status_invalid_length
                    );
        }
        else
        {
            MAKE_FPS_MESSAGE(
                    GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND
                    );

            message->fast_pair_server = fast_pair_server;
            message->cid = access_ind->cid;
            memmove(
                    message->value, 
                    access_ind->value, 
                    sizeof(uint8) * FAST_PAIR_VALUE_SIZE
                   );

            MessageSend(
                    fast_pair_server->app_task,
                    id,
                    message
                    );
        }
    }
    else
    {
        sendFpsServerAccessErrorRsp(
                fast_pair_server, 
                access_ind,
                gatt_status_request_not_supported
                );
    }
}


/******************************************************************************/
static void fpsHandleAccessIndication(
        const GFPS *fast_pair_server, 
        const GATT_MANAGER_SERVER_ACCESS_IND_T *access_ind
        )
{
    /* Service characteristics and descriptors cannot be accessed over BR/EDR */
    if ( !attBearerIsBLE(access_ind->cid) )
    {
        sendFpsServerAccessErrorRsp(
                fast_pair_server,
                access_ind,
                gatt_status_application_error
                );
    }
    else
    {
        switch (access_ind->handle)
        {
            case HANDLE_KEYBASED_PAIRING:
                fpsHandleKeybasedPairingAccess(fast_pair_server, access_ind);
                break;

            case HANDLE_KEYBASED_PAIRING_CLIENT_CONFIG:
                if (access_ind->flags & ATT_ACCESS_READ)
                {
                    fpsHandleReadClientConfigAccess(
                        fast_pair_server, 
                        access_ind,
                        GATT_FAST_PAIR_SERVER_READ_KEYBASED_PAIRING_CONFIG_IND
                        );
                }
                else if (access_ind->flags & ATT_ACCESS_WRITE)
                {
                    fpsHandleWriteClientConfigAccess(
                        fast_pair_server, 
                        access_ind,
                        GATT_FAST_PAIR_SERVER_WRITE_KEYBASED_PAIRING_CONFIG_IND
                        );
                }
                else
                { 
                    sendFpsServerAccessErrorRsp(
                            fast_pair_server, 
                            access_ind,
                            gatt_status_request_not_supported
                            );
                }
                break;

            case HANDLE_PASSKEY:
                fpsHandlePasskeyOrAccountKeyAccess(
                        fast_pair_server, 
                        access_ind,
                        GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_IND
                        );
                break;

            case HANDLE_PASSKEY_CLIENT_CONFIG:
                if (access_ind->flags & ATT_ACCESS_READ)
                {
                    fpsHandleReadClientConfigAccess(
                            fast_pair_server, 
                            access_ind,
                            GATT_FAST_PAIR_SERVER_READ_PASSKEY_CONFIG_IND
                            );
                }
                else if (access_ind->flags & ATT_ACCESS_WRITE)
                {
                    fpsHandleWriteClientConfigAccess(
                            fast_pair_server, 
                            access_ind,
                            GATT_FAST_PAIR_SERVER_WRITE_PASSKEY_CONFIG_IND
                            );
                }
                else
                { 
                    sendFpsServerAccessErrorRsp(
                            fast_pair_server, 
                            access_ind,
                            gatt_status_request_not_supported
                            );
                }
                break;

            case HANDLE_ACCOUNT_KEY:
                fpsHandlePasskeyOrAccountKeyAccess(
                        fast_pair_server, 
                        access_ind,
                        GATT_FAST_PAIR_SERVER_WRITE_ACCOUNT_KEY_IND
                        );
                break;

            default:
                sendFpsServerAccessErrorRsp(
                        fast_pair_server,
                        access_ind,
                        gatt_status_invalid_handle
                        );
                break;
        } /* switch */
    } /* endif */
}

/******************************************************************************/
static void fpsSendNotificationCfm(
        const GFPS *fast_pair_server, 
        const GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM_T *notification_cfm,
        MessageId id
        )
{
    /* Both GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM and
     * GATT_FAST_PAIR_SERVER_PASSKEY_NOTIFICATION_CFM have the same structure.
     */
    MAKE_FPS_MESSAGE(
            GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM
            );

    message->fast_pair_server = fast_pair_server;
    message->cid = notification_cfm->cid;
    message->status = notification_cfm->status;

    MessageSend(
            fast_pair_server->app_task,
            id,
            message
            );
}
/******************************************************************************/
static void fpsHandleNotificationcfm(
        const GFPS *fast_pair_server, 
        const GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM_T *notification_cfm
        )
{
    switch (notification_cfm->handle)
    {
        case HANDLE_KEYBASED_PAIRING:
            fpsSendNotificationCfm(
                    fast_pair_server,
                    notification_cfm,
                    GATT_FAST_PAIR_SERVER_KEYBASED_PAIRING_NOTIFICATION_CFM
                    );
            break;

        case HANDLE_PASSKEY:
            fpsSendNotificationCfm(
                    fast_pair_server,
                    notification_cfm,
                    GATT_FAST_PAIR_SERVER_PASSKEY_NOTIFICATION_CFM
                    );
             break;

        default:
            GATT_FAST_PAIR_SERVER_DEBUG_PANIC((
                    "Unrecognised handle 0x%04x in Notification Cfm\n",
                    notification_cfm->handle
                    ));
    }
}


/******************************************************************************/
void fpsServerMsgHandler(Task task, MessageId id, Message msg)
{
    GFPS *fast_pair_server = (GFPS *)task;

    switch (id)
    {
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            fpsHandleAccessIndication(
                    fast_pair_server, 
                    (GATT_MANAGER_SERVER_ACCESS_IND_T *)msg
                    );
        }
        break;

        case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
        {
            fpsHandleNotificationcfm(
                    fast_pair_server,
                    (GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM_T *)msg
                    );
        }
        break;

        default:
        {
            GATT_FAST_PAIR_SERVER_DEBUG_PANIC((
                        "GFPS: GATT Manager message 0x%04x not handled\n",
                        id
                        ));
            break;
        }
    } /* switch */

}


