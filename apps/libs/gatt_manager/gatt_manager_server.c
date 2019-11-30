/*******************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.

*/

#include <stdlib.h>
#include <string.h>
#include <message.h>

#include "gatt_manager_internal.h"

#include "gatt_manager_server.h"
#include "gatt_manager_data.h"

static __inline__ size_t gattAccessIndSize(const GATT_ACCESS_IND_T * ind)
{
    return ((ind->size_value) * sizeof(uint8));
}

static void serverAccessInd(const GATT_ACCESS_IND_T * ind, Task task, uint16 handle)
{
    MAKE_GATT_MANAGER_MESSAGE_WITH_LEN(GATT_MANAGER_SERVER_ACCESS_IND, gattAccessIndSize(ind));

    /* First copy the static content, and then the variable length */
    *message = *ind;
    memmove(message->value, ind->value, gattAccessIndSize(ind));

    message->handle = handle;

    MessageSend(task, GATT_MANAGER_SERVER_ACCESS_IND, message);
}

static bool serverRegistrationParamsValid(const gatt_manager_server_registration_params_t * server)
{
    if (NULL == server ||
        0 == server->end_handle ||
        0 == server->start_handle ||
        NULL == server->task)
    {
        return FALSE;
    }

    return TRUE;
}

static bool serverConnectionParamsValid(Task task, const typed_bdaddr *taddr,
                                        gatt_connection_type conn_type)
{
    if( (NULL == task) ||
        ((gatt_connection_ble_slave_directed == conn_type ||
          gatt_connection_ble_slave_directed_low_duty == conn_type) && NULL != taddr) ||
        ( gatt_connection_ble_slave_whitelist == conn_type ) ||
        ( gatt_connection_ble_slave_undirected == conn_type ))
    {
        return TRUE;
    }

    return FALSE;
}

static bool remoteClientValid(Task task, uint16 cid, uint16 handle)
{
    if(NULL != task ||
       0 != cid ||
       0!= handle)
    {
        return TRUE;
    }
    return FALSE;
}

static void remoteClientConnectCfmFail(Task task)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM);
    message->status = gatt_status_failure;
    MessageSend(task, GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM, message);
}

static void remoteClientConnectCfm(const GATT_CONNECT_CFM_T * cfm)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM);

    *message = *cfm;

    MessageSend(gattManagerDataGetRemoteClientConnectTask(),
                GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM, message);
}

static void cancelRemoteClientConnectCfm(Task task, gatt_manager_status_t status)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM);
    message->status = status;
    MessageSend(task, GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM, message);
}

static void remoteClientNotificationCfm(const GATT_NOTIFICATION_CFM_T * cfm, Task task,
                                        uint16 handle)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM);

    *message = *cfm;
    message->handle = handle;

    MessageSend(task, GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM, message);
}

static void remoteClientIndicationCfm(const GATT_INDICATION_CFM_T * cfm, Task task,
                                      uint16 handle)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM);

    *message = *cfm;
    message->handle = handle;

    MessageSend(task, GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM, message);
}

static void gattConnectCfmPanicOnWrongCid(const GATT_CONNECT_CFM_T * cfm)
{
    if(cfm->cid != gattManagerDataGetRemoteClientConnectCid())
    {
         GATT_MANAGER_PANIC(("GM: Unknown CID in Connect CFM!"));
    }
}

static void gattConnectCfmSuccess(const GATT_CONNECT_CFM_T * cfm)
{
    if (gattManagerDataIsCancelPending())
    {
        cancelRemoteClientConnectCfm(gattManagerDataGetRemoteClientConnectTask(),
                                     gatt_manager_status_failed);
    }

    gattManagerDataAdvertisingState_Idle();
    remoteClientConnectCfm(cfm);
}

static void gattConnectCfmFailure(const GATT_CONNECT_CFM_T * cfm)
{
    if (gattManagerDataIsCancelPending())
    {
        gattManagerDataPendingCancelled();
        cancelRemoteClientConnectCfm(gattManagerDataGetRemoteClientConnectTask(),
                                     gatt_manager_status_success);
    }
    else
    {
        remoteClientConnectCfm(cfm);
    }
    gattManagerDataAdvertisingState_Idle();
}

static void gattConnectCfmUnknown(const GATT_CONNECT_CFM_T * cfm)
{
    if (gattManagerDataIsCancelPending())
    {
        cancelRemoteClientConnectCfm(gattManagerDataGetRemoteClientConnectTask(),
                                     gatt_manager_status_failed);
    }
    else
    {
        remoteClientConnectCfm(cfm);
    }
    gattManagerDataAdvertisingState_Idle();
}

static void clearRemoteClientConnectData(void)
{
    gattManagerDataSetRemoteClientConnectTask(NULL);
    gattManagerDataSetRemoteClientConnectCid(0);
}

/******************************************************************************
 *                      GATT MANAGER SERVER PUBLIC API                        *
 ******************************************************************************/

gatt_manager_status_t GattManagerRegisterServer(const gatt_manager_server_registration_params_t * server)
{
    if (!gattManagerDataIsInit())
    {
        return gatt_manager_status_not_initialised;
    }

    if (gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_registration)
    {
        return gatt_manager_status_wrong_state;
    }

    if (!serverRegistrationParamsValid(server))
    {
        return gatt_manager_status_invalid_parameters;
    }

    if (gattManagerDataGetDB() == NULL)
    {
        return gatt_manager_status_failed;
    }

    if (!gattManagerDataAddServer(server))
    {
        return gatt_manager_status_invalid_parameters;
    }

    return gatt_manager_status_success;
}

void GattManagerWaitForRemoteClient(Task task,
                                    const typed_bdaddr *taddr,
                                    gatt_connection_type conn_type)
{
    if (!gattManagerDataIsInit())
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"))
    }

    if (gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_initialised)
    {
        GATT_MANAGER_PANIC(("GM: In wrong Initialisation state!"))
    }

    if(!serverConnectionParamsValid(task, taddr, conn_type))
    {
        GATT_MANAGER_PANIC(("GM: Invalid parameters!"))
    }

    if(gattManagerDataServerCount() == 0)
    {
        GATT_MANAGER_PANIC(("GM: No Servers registered!"))
    }

    if(gattManagerDataGetAdvertisingState() == gatt_manager_advertising_state_idle)
    {
        gattManagerDataSetRemoteClientConnectTask(task);

        if(gattManagerDataGetRemoteServerConnectHandler())
        {
            MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT);
            message->task = task;
            if(NULL != taddr)
            {
                message->taddr_not_null = TRUE;
                message->taddr = *taddr;
            }
            message->conn_type = conn_type;
            MessageSendConditionallyOnTask(gattManagerDataGetTask(),
                                           GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT,
                                           message, gattManagerDataGetPointerToRemoteServerConnectHandler());
            gattManagerDataAdvertisingState_Queued();
        }
        else
        {
            GattConnectRequest(gattManagerDataGetTask(), taddr, conn_type, FALSE);
            gattManagerDataAdvertisingState_Requested();
        }
    }
    else
    {
        remoteClientConnectCfmFail(task);
    }
}

void GattManagerCancelWaitForRemoteClient(void)
{
    if (!gattManagerDataIsInit())
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"));
    }

    if (gattManagerDataGetInitialisationState() != gatt_manager_initialisation_state_initialised)
    {
        GATT_MANAGER_PANIC(("GM: In wrong Initialisation state!"));
    }

    if(gattManagerDataServerCount() == 0)
    {
        GATT_MANAGER_PANIC(("GM: No Servers registered!"))
    }

    if (gattManagerDataIsCancelPending())
    {
        GATT_MANAGER_DEBUG_INFO(("GM: Repeat Disconnect Request Ignored!"))
        return;
    }

    switch(gattManagerDataGetAdvertisingState())
    {
        case gatt_manager_advertising_state_requested:
            /* Can't issue a disconnect until the GATT_CONNECT_CFM returns as
             * the CID is not known at this point. So this request must be
             * queued until the GATT_CONNECT_CFM message indicating the pending
             * advertising request has been received.
             * */
            MessageCancelFirst(gattManagerDataGetTask(),
                               GATT_MANAGER_INTERNAL_MSG_CANCEL_ADVERTISING);
            MessageSendConditionally(gattManagerDataGetTask(),
                                     GATT_MANAGER_INTERNAL_MSG_CANCEL_ADVERTISING,
                                     0, gattManagerDataGetAdvertisingRequestedFlag());
            break;

        case gatt_manager_advertising_state_advertising:
            gattManagerDataCancelPending();
            GattDisconnectRequest(gattManagerDataGetRemoteClientConnectCid());
            break;

        case gatt_manager_advertising_state_queued:
            if(MessageCancelAll(gattManagerDataGetTask(), GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT) > 0)
            {
                cancelRemoteClientConnectCfm(gattManagerDataGetRemoteClientConnectTask(),
                                             gatt_status_success);
                clearRemoteClientConnectData();
                gattManagerDataAdvertisingState_Idle();
            }
            else
            {
                GATT_MANAGER_PANIC(("GM: No WaitFor Remote Client Queued!"))
            }
            break;

        case gatt_manager_advertising_state_idle:
            cancelRemoteClientConnectCfm(gattManagerDataGetApplicationTask(),
                                         gatt_manager_status_wrong_state);
            break;

        default:
            GATT_MANAGER_PANIC(("GM: In Impossible state!"));
            break;
    }
}

bool GattManagerServerAccessResponse(Task task,
                                     uint16 cid,
                                     uint16 handle,
                                     uint16 result,
                                     uint16 size_value,
                                     const uint8 *value)
{
    uint16 adjusted_handle;

    if (!gattManagerDataIsInit())
    {
        return FALSE;
    }

    if(!remoteClientValid(task, cid, handle))
    {
        return FALSE;
    }

    if (handle == 0)
    {
        GATT_MANAGER_DEBUG_INFO(("Access Rsp Handle=0 Task[0x%p] cid[0x%x]\n", 
                                 (void*)task,
                                 cid));
        
        /* Write execute response */
        if (!gattManagerDataServerGetPrepareWriteFlag(task))
        {
            GATT_MANAGER_DEBUG_INFO(("   No execute writes to process\n"));
            /* we are not expecting write execute response from this task */
            return FALSE;
        }
        if (result != gatt_status_success)
        {
            GATT_MANAGER_DEBUG_INFO(("   Set execute write failure[%u]\n", result));
            gattManagerDataServerSetExecuteWriteResult(result);
        }
        /* clear the flag */
        if (gattManagerDataServerClearPrepareWriteFlag(task))
        {
            /* all flags are now cleared, can send the response back to Gatt library */
            adjusted_handle = 0;
            result = gattManagerDataServerGetExecuteWriteResult();
            GATT_MANAGER_DEBUG_INFO(("   Execute write result[%u]\n", result));
        }
        else
        {
            /* waiting for responses from other tasks */
            GATT_MANAGER_DEBUG_INFO(("   Waiting for more write responses\n"));
            return TRUE;
        }
    }
    else
    {
        adjusted_handle = gattManagerDataGetServerDatabaseHandle(task, handle);
        
        if (adjusted_handle == 0)
        {
            adjusted_handle = handle;
        }
        GATT_MANAGER_DEBUG_INFO(("Access Rsp Handle[0x%x] Task[0x%p] cid[0x%x] result[%u]\n", 
                                 adjusted_handle, 
                                 (void*)task, 
                                 cid,
                                 result));
    }

    GattAccessResponse(cid, adjusted_handle, result, size_value, value);

    return TRUE;
}

void GattManagerRemoteClientNotify(Task   task,
                                   uint16 cid,
                                   uint16 handle,
                                   uint16 size_value,
                                   const uint8 *value)
{
    uint16 adjusted_handle;

    if (!gattManagerDataIsInit())
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"));
    }

    if(!remoteClientValid(task, cid, handle))
    {
        GATT_MANAGER_PANIC(("GM: Remote Client Notify Invalid parameters!"));
    }

    adjusted_handle = gattManagerDataGetServerDatabaseHandle(task, handle);
    if (adjusted_handle)
    {
        GattNotificationRequest(gattManagerDataGetTask(), cid, adjusted_handle,
                                size_value, value);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM);
        message->status = gatt_status_failure;
        MessageSend(task, GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM, message);
    }
}

void GattManagerRemoteClientIndicate(Task   task,
                                     uint16 cid,
                                     uint16 handle,
                                     uint16 size_value,
                                     const uint8 *value)
{
    uint16 adjusted_handle;

    if (!gattManagerDataIsInit())
    {
        GATT_MANAGER_PANIC(("GM: Not initialised!"));
    }

    if(!remoteClientValid(task, cid, handle))
    {
        GATT_MANAGER_PANIC(("GM: Remote Client Notify Invalid parameters!"));
    }

    adjusted_handle = gattManagerDataGetServerDatabaseHandle(task, handle);
    if (adjusted_handle)
    {
        GattIndicationRequest(gattManagerDataGetTask(), cid, adjusted_handle,
                              size_value, value);
    }
    else
    {
        MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM);
        message->status = gatt_status_failure;
        MessageSend(task, GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM, message);
    }
}

/******************************************************************************
 *                      GATT MANAGER Server Internal API                      *
 ******************************************************************************/

void gattManagerWaitForRemoteClientInternal(const GATT_MANAGER_INTERNAL_MSG_WAIT_FOR_REMOTE_CLIENT_T *params)
{
    GATT_MANAGER_PANIC_NULL(params, ("GM: Wait for Remote Client Internal params NULL!"));

    gattManagerDataAdvertisingState_Idle();
    GattManagerWaitForRemoteClient(params->task,
                                   ((TRUE == params->taddr_not_null) ? &params->taddr : NULL),
                                   params->conn_type);
}

void gattManagerServerConnectInd(const GATT_CONNECT_IND_T * ind)
{
    MAKE_GATT_MANAGER_MESSAGE(GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND);
    *message = *ind;
    MessageSend(gattManagerDataGetApplicationTask(), GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND, message);
}

void GattManagerRemoteClientConnectResponse(uint16 cid, uint16 flags, bool accept)
{
    GattConnectResponse(gattManagerDataGetTask(), cid, flags, accept);
}

void gattManagerServerIndicationCfm(const GATT_INDICATION_CFM_T * cfm)
{
    gatt_manager_resolve_server_handle_t discover;
    discover.handle = cfm->handle;

    if (gattManagerDataResolveServerHandle(&discover))
    {
        remoteClientIndicationCfm(cfm, discover.task, discover.adjusted);
    }
    else
    {
        remoteClientIndicationCfm(cfm, gattManagerDataGetApplicationTask(), cfm->handle);
    }
}

void gattManagerServerNotificationCfm(const GATT_NOTIFICATION_CFM_T * cfm)
{
    gatt_manager_resolve_server_handle_t discover;
    discover.handle = cfm->handle;

    if (gattManagerDataResolveServerHandle(&discover))
    {
        remoteClientNotificationCfm(cfm, discover.task, discover.adjusted);
    }
    else
    {
        remoteClientNotificationCfm(cfm, gattManagerDataGetApplicationTask(), cfm->handle);
    }
}

void gattManagerServerAccessInd(const GATT_ACCESS_IND_T * ind)
{
    bool pending_write = FALSE;
    
    if (ind->handle != 0)
    {
        gatt_manager_resolve_server_handle_t discover;
        discover.handle = ind->handle;

        /* Track Writes sent to servers. This is to handle the Reliable Write sub-procedure to write 
            a Characteristic Value, as an Execute Write Request with handle 0 must be sent to all
            servers with a pending Write Request. */
        pending_write = (ind->flags & (ATT_ACCESS_WRITE | ATT_ACCESS_WRITE_COMPLETE)) == ATT_ACCESS_WRITE;

        if(gattManagerDataResolveServerHandle(&discover))
        {
            if (pending_write)
            {
                gattManagerDataSetServerPendingWriteFlag(discover.handle);
            }
            GATT_MANAGER_DEBUG_INFO(("Access Ind Server : PendingWrite[%u] Handle[0x%x] Task[0x%p] cid[0x%x]\n",
                                     pending_write,
                                     discover.handle, 
                                     (void*)discover.task,
                                     ind->cid));
            serverAccessInd(ind, discover.task, discover.adjusted);
        }
        else
        {
            GATT_MANAGER_DEBUG_INFO(("Access Ind App : PendingWrite[%u] Handle[0x%x] Task[0x%p] cid[0x%x]\n", 
                                         pending_write,
                                         discover.handle, 
                                         (void*)gattManagerDataGetApplicationTask(),
                                         ind->cid));
            if (pending_write)
            {
                gattManagerDataSetApplicationPendingWriteFlag();
            }
            serverAccessInd(ind, gattManagerDataGetApplicationTask(), ind->handle);
        }
    }
    else
    {
        gatt_manager_data_iterator_t iter;
        gatt_manager_server_lookup_data_t server;

        if(gattManagerDataServerIteratorStart(&iter))
        {
            while(gattManagerDataServerIteratorPrepareWriteFlagsNext(&server, &iter))
            {
                /* Send Execute Write command (with handle 0) to all servers that 
                   were sent Prepare Write Requests.
                */
                GATT_MANAGER_DEBUG_INFO(("Access Ind : Handle[0x%x] Task[0x%p] cid[0x%x]\n", 
                                         ind->handle, 
                                         (void*)server.task,
                                         ind->cid));
                serverAccessInd(ind, server.task, ind->handle);
            }
        }
    }
}

void gattManagerServerAdvertising(uint16 cid)
{
    gattManagerDataSetRemoteClientConnectCid(cid);
    gattManagerDataAdvertisingState_Advertising();
}

void gattManagerServerRemoteClientConnected(const GATT_CONNECT_CFM_T * cfm)
{
    gattConnectCfmPanicOnWrongCid(cfm);

    if(cfm->status == gatt_status_success)
    {
        gattConnectCfmSuccess(cfm);
    }
    else if (cfm->status == gatt_status_failure)
    {
        gattConnectCfmFailure(cfm);
    }
    else
    {
        gattConnectCfmUnknown(cfm);
    }

    clearRemoteClientConnectData();
}
