/****************************************************************************
Copyright (c) 2005 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_manager.c        

DESCRIPTION
    Contains GATT Manager functionality.
*/

#include "sink_gatt_manager.h"

#include "sink_ble.h"
#include "sink_ble_sc.h"
#include "sink_ble_gap.h"
#include "sink_debug.h"
#include "sink_development.h"
#include "sink_gaia.h"
#include "sink_gatt_client.h"
#include "sink_gatt_client_battery.h"
#include "sink_gatt_client_ancs.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_client_hid.h"
#include "sink_gatt_client_dis.h"
#include "sink_gatt_client_gatt.h"
#include "sink_gatt_server_lls.h"
#include "sink_gatt_server.h"
#include "sink_gatt_server_battery.h"
#include "sink_gatt_server_gatt.h"
#include "sink_statemanager.h"
#include "sink_ba_receiver.h"
#include "sink_ba_broadcaster_association.h"
#include "sink_ba_ble_gap.h"
#include "sink_gatt_common.h"
#include "sink_scan.h"

#ifdef GATT_AMS_CLIENT
#include "sink_gatt_client_ams.h"
#endif

#ifdef GATT_AMA_SERVER
#include "sink_gatt_server_ama.h"
#endif

#ifdef GATT_BISTO_COMM_SERVER
#include "sink_gatt_server_bisto_comm.h"
#endif

#ifdef GATT_AMS_PROXY
#include "sink_gatt_server_ams_proxy.h"
#endif

#ifdef GATT_ANCS_PROXY
#include "sink_gatt_server_ancs_proxy.h"
#endif

#ifdef GATT_BMS
#include "sink_gatt_server_bms.h"
#endif

#include <gatt_manager.h>

#include <csrtypes.h>
#include <stdlib.h>
#include <vm.h>


#ifdef GATT_ENABLED


#ifdef DEBUG_GATT_MANAGER
#define GATT_MANAGER_INFO(x) DEBUG(x)
#define GATT_MANAGER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_MANAGER_INFO(x)
#define GATT_MANAGER_ERROR(x)
#endif


/*******************************************************************************
NAME
    handleGattManagerRegistrationCfm
    
DESCRIPTION
    Handle when GATT_MANAGER_REGISTER_WITH_GATT_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_MANAGER_REGISTER_WITH_GATT_CFM message
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
static bool handleGattManagerRegistrationCfm(GATT_MANAGER_REGISTER_WITH_GATT_CFM_T * cfm)
{
    GATT_MANAGER_INFO(("GATT_MANAGER_REGISTER_WITH_GATT_CFM status=[0x%x]\n", cfm->status));
    if (cfm)
    {
        if (cfm->status == gatt_manager_status_success)
        {
            MessageSend(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_INIT_COMPLETE, 0);
            return TRUE;
        }
    }
    return FALSE;
}

/*******************************************************************************
DESCRIPTION
    Accept or reject a GATT connection over BR/EDR
*/
static void handleGattManagerRemoteClientConnectInd(GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND_T * ind)
{
    GattManagerRemoteClientConnectResponse(ind->cid, ind->flags, TRUE);
}

/*******************************************************************************
NAME
    handleGattManagerRemoteClientConnectCfm
    
DESCRIPTION
    Handle when GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM message
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
static bool handleGattManagerRemoteClientConnectCfm(GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T * cfm)
{
    tp_bdaddr tp_addr;
    uint16 index = GATT_INVALID_INDEX;
    
    GATT_MANAGER_INFO(("GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM status=[0x%x]\n", cfm->status));
    if (cfm)
    {
        if (cfm->status == gatt_status_success)
        {
            /* Add the server connection */
            if (gattServerConnectionAdd(cfm->cid, &cfm->taddr))
            {
                if((BA_RECEIVER_MODE_ACTIVE) && gapBaGetAssociationInProgress())
                {
                    /* Update the receiver with the CID */
                    sinkReceiverHandleBroadcasterConnectCfm(cfm);
                }
                else
                {
                    /* There has been a link up, so check if PXP if alert has to be stopped */
                    sinkGattHandleLinkUpInd(cfm->cid);

                    /* Associate GAIA with the connection ID */
                    gaiaGattConnect(cfm->cid);

                    /* Write LE APT if LE link is Secure */
                    sinkBleWriteApt(cfm->cid);
                }

                if(VmGetBdAddrtFromCid(cfm->cid, &tp_addr))
                {
                    /* Find an empty slot in GAP conn_flags */
                    index = sinkBleGapFindGapConnFlagIndex(NULL);

                    sinkBleGapSetSecurityRequestSent(FALSE , &tp_addr.taddr, cfm->cid, index);
                }
                
                /* Send connection success event */
                sinkBleSlaveConnIndEvent(cfm->cid);

                /* Record this connection which stores device specific data */
                gattClientAdd(cfm->cid, ble_gap_role_peripheral);

#ifdef GATT_AMA_SERVER
                sinkAmaServerConnectEvent(cfm);
#endif
#ifdef GATT_BMS
                sinkBmsServerConnectEvent(cfm->cid);
#endif                

                return TRUE;
            }
            else
            {
                /* we could not add the connection, looks like this is not a bonded device
                  * So, disconnect the link */
                gattServerDisconnect(cfm->cid);
            }
        }
    }
           
    return FALSE;
}


/*******************************************************************************
NAME
    handleGattManagerRemoteServerConnectCfm
    
DESCRIPTION
    Handle when GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM message
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
static bool handleGattManagerRemoteServerConnectCfm(GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM_T * cfm)
{
    GATT_MANAGER_INFO(("GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM status=[0x%x]\n", cfm->status));
    if (cfm)
    {
        if (cfm->status == gatt_status_success)
        {
            if(BA_BROADCASTER_MODE_ACTIVE)
            {
                /* Ok, connected to receiver, now allow receiver to read the association data.
                    We are not interested in receiver's data, so shall not add the client structure to
                    the ble data */
                sinkBroadcasterHandleReceiverConnectCfm(cfm);
                /* We are done with connecting to one receiver, so stop scanning for more */
                sinkBleMasterConnCompleteEvent(cfm->cid);
            }
            else
            {
                /* Record the new connection */
                if (!gattClientAdd(cfm->cid, ble_gap_role_central))
                {
                    GATT_MANAGER_INFO(("Couldn't store client->server connection!\n"));

                    /* Remove GATT connection if it couldn't be stored */
                    GattManagerDisconnectRequest(cfm->cid);
                    return FALSE;
                }

                /* Write LE APT if LE link is Secure */
                sinkBleWriteApt(cfm->cid);
            }
            return TRUE;
        }
    }
    return FALSE;
}


/*******************************************************************************
NAME
    handleGattManagerDisconnectInd
    
DESCRIPTION
    Handle when GATT_MANAGER_DISCONNECT_IND message was received
    
PARAMETERS
    ind Pointer to a GATT_MANAGER_DISCONNECT_IND message
    
RETURNS
    TRUE if the message was handled, FALSE otherwise
*/
static bool handleGattManagerDisconnectInd(GATT_MANAGER_DISCONNECT_IND_T * ind)
{
    gatt_client_connection_t *client_connection = gattClientFindByCid(ind->cid);
    bool server_connection = gattServerConnectionFindByCid(ind->cid);
    
    GATT_MANAGER_INFO(("GATT_MANAGER_DISCONNECT_IND\n"));
    
    if (ind)
    { 
        /*Cancel relevant internal messages to avoid any race condition*/
        MessageCancelAll(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR);

        if (client_connection)
        {
            GATT_MANAGER_INFO(("    Client\n"));
            
            /* Remove client services */
            gattClientRemoveServices(client_connection);
            
            /* Finally remove client connection */
            if (!gattClientRemove(ind->cid))
            {
                GATT_MANAGER_ERROR(("Couldn't find client->server connection to remove!\n"));
            }
        }
        if (server_connection)
        {
            GATT_MANAGER_INFO(("    Server\n"));

            /* There has been a link loss, so check if PXP has to alerted */
            if(ind->status == gatt_status_link_loss)
            {
                sinkGattHandleLinkLossInd(ind->cid);
            }
            
            /* Server disconnect happened ,inform the disconnect to gaia */
            gaiaGattDisconnect(ind->cid);

            if (!gattServerConnectionRemove(ind->cid))
            {
                GATT_MANAGER_ERROR(("Couldn't find server->client connection to remove!\n"));
            }   

#ifdef GATT_AMA_SERVER
            sinkAmaServerDisconnectEvent(ind->cid, ind->status);
#endif
#ifdef GATT_BISTO_COMM_SERVER
            SinkBistoCommServerDisconnectEvent(ind->cid, ind->status);
#endif
#ifdef GATT_BMS
            sinkBmsServerDisconnectEvent(ind->cid);
#endif
        }

        /* This would have been a connection made in Central role, send event to inform of disconnection */
        sinkBleDisconnectionEvent(ind->cid);
        {
            sink_ba_app_mode_t ba_mode = sinkBroadcastAudioGetMode();
            switch (ba_mode)
            {
                case sink_ba_appmode_broadcaster:
                    /* could receiver disconnect for exiting GAIA link too */
                    if(sinkBroadcasterIsReceiverCid(ind->cid))
                        sinkBroadcasterHandleReceiverDisconnectInd(ind);
                break;

                case sink_ba_appmode_receiver:
                    /* could receiver disconnect for exiting GAIA link too */
                    if(sinkReceiverIsBroadcasterCid(ind->cid))
                        sinkReceiverHandleBroadcasterDisconnectInd(ind);
                break;

                default:
                break;
            }
        }
        return TRUE;
    }
    return FALSE;
}

/*******************************************************************************
NAME
    handleGattManagerCancelRemoteClientConnectCfm
    
DESCRIPTION
    Handle when GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM message was received
    
PARAMETERS
    cfm Pointer to a GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM message
    
RETURNS
    Nothing
*/
static void handleGattManagerCancelRemoteClientConnectCfm(GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM_T* cfm)
{
    GATT_MANAGER_INFO(("GattMgr: Cancel advertising cfm status=[%x]\n", cfm->status));
    /* Only if we were able to successfully stop advertisement */
    if(cfm->status == gatt_manager_status_success)
    {
        /* Generate a cancel advertising which can restart advertising in a different mode */
        sinkBleCancelAdvertisingEvent();
        sinkDisableGattConnectable();
    }
}


/******************************************************************************/
void sinkGattManagerStartAdvertising(void)
{
    GATT_MANAGER_INFO(("GattMgr: Start advertising\n"));
    
    GattManagerWaitForRemoteClient(sinkGetBleTask(), NULL, gatt_connection_ble_slave_undirected);
    sinkEnableGattConnectable();
}


/******************************************************************************/
void sinkGattManagerStopAdvertising(void)
{
    GATT_MANAGER_INFO(("GattMgr: Send stop advertising request\n"));
    GattManagerCancelWaitForRemoteClient();
    /* Wait for GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM to indicate outcome of
     * the request.
     */
}


/******************************************************************************/
void sinkGattManagerMsgHandler( Task task, MessageId id, Message message )
{
    UNUSED(task);
    
    switch(id)
    {
        case GATT_MANAGER_REGISTER_WITH_GATT_CFM:
        {
            if (!handleGattManagerRegistrationCfm((GATT_MANAGER_REGISTER_WITH_GATT_CFM_T*)message))
            {
                FATAL_ERROR(("handleGattManagerRegistrationCfm failed\n"));
            }
        }
        break;
        case GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND:
        {
            handleGattManagerRemoteClientConnectInd((GATT_MANAGER_REMOTE_CLIENT_CONNECT_IND_T*)message);
        }
        break;
        case GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM:
        {
            handleGattManagerRemoteClientConnectCfm((GATT_MANAGER_REMOTE_CLIENT_CONNECT_CFM_T*)message);
        }
        break;
        case GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM:
        {
            if (!handleGattManagerRemoteServerConnectCfm((GATT_MANAGER_REMOTE_SERVER_CONNECT_CFM_T*)message))
            {
                sinkBleMasterConnCompleteEvent(GATT_CLIENT_INVALID_CID);
            }
        }
        break;
        case GATT_MANAGER_DISCONNECT_IND:
        {
            handleGattManagerDisconnectInd((GATT_MANAGER_DISCONNECT_IND_T*)message);
        }
        break;
        case GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM:
        {
            handleGattManagerCancelRemoteClientConnectCfm((GATT_MANAGER_CANCEL_REMOTE_CLIENT_CONNECT_CFM_T*)message);
        }
        break;
        case GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND:
        {
            /* Ignore any notifications received before the client libraries have been initialised */
            GATT_MANAGER_INFO(("GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND ignored cid=[%u] handle=[%u]\n",
                               ((GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T*)message)->cid,
                               ((GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T*)message)->handle));
        }
        break;
        case GATT_MANAGER_REMOTE_SERVER_INDICATION_IND:
        {
            /* Respond to any indications received before the client libraries have been initialised */
            GATT_MANAGER_INFO(("GATT_MANAGER_REMOTE_SERVER_INDICATION_IND ignored cid=[%u] handle=[%u]\n",
                               ((GATT_MANAGER_REMOTE_SERVER_INDICATION_IND_T*)message)->cid,
                               ((GATT_MANAGER_REMOTE_SERVER_INDICATION_IND_T*)message)->handle));
            GattManagerIndicationResponse(((GATT_MANAGER_REMOTE_SERVER_NOTIFICATION_IND_T*)message)->cid);
        }
        break;
        case GATT_MANAGER_SERVER_ACCESS_IND:
        {
            /* No server handle exists for this access request, so just reject. */
            GattAccessResponse(((GATT_MANAGER_REMOTE_SERVER_INDICATION_IND_T*)message)->cid,
                               ((GATT_MANAGER_REMOTE_SERVER_INDICATION_IND_T*)message)->handle, 
                               gatt_status_request_not_supported, 
                               0, 
                               NULL);
            
            /* Inform the remote device that it has invalid handles, and should start a new discovery */
            sinkGattServerSendServiceChanged(((GATT_MANAGER_REMOTE_SERVER_INDICATION_IND_T*)message)->cid);
        }
        break;
        case GATT_MANAGER_REMOTE_CLIENT_NOTIFICATION_CFM:
        case GATT_MANAGER_REMOTE_CLIENT_INDICATION_CFM:
        default:
        {
            GATT_MANAGER_ERROR(("GATT Manager unhandled msg[%x]\n", id));
        }
        break;
    }
}


/******************************************************************************/
void sinkGattManagerStartConnection(const typed_bdaddr *addr)
{
    GATT_MANAGER_INFO(("GattMgr: Start Connection\n"));
    
    GattManagerConnectToRemoteServer(sinkGetBleTask(),
                        addr,
                        gatt_connection_ble_master_directed,
                        TRUE);
}


#endif /* GATT_ENABLED */
