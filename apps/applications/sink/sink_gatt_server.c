/****************************************************************************
Copyright (c) 2005 - 2015 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_server.c

DESCRIPTION
    Routines to handle the GATT Servers.
*/

#include "sink_ble.h"
#include "sink_gatt_server.h"
#include "sink_gatt_common.h"
#include "sink_gatt_server_battery.h"

#include "sink_debug.h"
#include "sink_devicemanager.h"

#include <vm.h>
#include <gatt_manager.h>


#ifdef GATT_ENABLED

#ifdef DEBUG_GATT_SERVER
#define GATT_SERVER_DEBUG(x) DEBUG(x)
#else
#define GATT_SERVER_DEBUG(x) 
#endif


/****************************************************************************/
bool gattServerConnectionAdd(uint16 cid, const typed_bdaddr *client_taddr)
{
    sink_attributes attributes;
    typed_bdaddr public_taddr;
    uint16 index;
    bool is_bonded_device = sinkBleCheckBdAddrtIsBonded(client_taddr, &public_taddr);
    
    /* If we are trying to connect to a device in connectable
     * state, then we should be in proper state to allow the connection
     */
    if(!sinkBleGapIsConnectable())
        return FALSE;

    /* So, we are in connectable state, now check if we still have slot for connection
     * in peripheral role */
    if(gattCommonAddConnections(cid, ble_gap_role_peripheral))
    {
        index = gattCommonConnectionFindByCid(cid);

        /* Set client service configuration to default value */
        memset(&GATT_SERVER.client_config, 0, sizeof(gatt_ccd_attributes_t));

        if (is_bonded_device)
        {
            /* Restore client configuration from PS */
            deviceManagerClearAttributes(&attributes);
            if (deviceManagerGetAttributes(&attributes, &public_taddr.addr))
            {
                GATT_SERVER.client_config = attributes.gatt_client;
                GATT_SERVER_DEBUG(("GATT Client: Read client config addr[%x:%x:%lx] config_gatt[0x%x] config_batt_l[0x%x] config_batt_r[0x%x] config_batt_p[0x%x]\n",
                                   public_taddr.addr.nap,
                                   public_taddr.addr.uap,
                                   public_taddr.addr.lap,
                                   GATT_SERVER.client_config.gatt,
                                   GATT_SERVER.client_config.battery_local,
                                   GATT_SERVER.client_config.battery_remote,
                                   GATT_SERVER.client_config.battery_peer
                                   ));
            }
        }

        /* check for repeated updates */
        if( LOCAL_UPDATE_REQD(GATT_SERVER) ||
            REMOTE_UPDATE_REQD(GATT_SERVER) ||
            PEER_UPDATE_REQD(GATT_SERVER) )
        {
            MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER );
            MessageSend( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER, 0 );
        }

        return TRUE;
    }
    return FALSE;
}


/****************************************************************************/
bool gattServerConnectionFindByCid(uint16 cid)
{
    return (gattCommonConnectionFindByCid(cid) != GATT_INVALID_INDEX);
}


/****************************************************************************/
bool gattServerConnectionRemove(uint16 cid)
{
    return gattCommonRemoveConnections(cid);
}


/****************************************************************************/
bool gattServerDisconnectAll(void)
{
    /* Call into gatt_manager to remove connections */
    return gattCommonDisconnectAll(FALSE);
}

/****************************************************************************/
void gattServerDisconnect(uint16 cid)
{
    GattManagerDisconnectRequest(cid);
}

/***************************************************************************/
bool gattServerIsFullyDisconnected(void)
{
    uint16 conn_index = 0;
    for(conn_index=0;conn_index < MAX_BLE_CONNECTIONS ;conn_index++)
    {
        /* check if the connection is purely peripheral */
        if(GATT[conn_index].cid != 0)
        {
            if(GATT[conn_index].client.connection.role != ble_gap_role_central)
                return FALSE;
        }
    }
    return TRUE;
}

/***************************************************************************/
void gattServerStoreConfigAttributes(uint16 cid, gatt_attribute_service_t ccd_service)
{
    sink_attributes attributes;
    tp_bdaddr tpaddr;
    typed_bdaddr public_taddr;
    bool attr_changed = FALSE;
    uint16 index = gattCommonConnectionFindByCid(cid);
   
    /* Store client configuration to PS */
    if ((index != GATT_INVALID_INDEX) && VmGetBdAddrtFromCid(cid, &tpaddr))
    {
        if (sinkBleCheckBdAddrtIsBonded(&tpaddr.taddr, &public_taddr))
        {
            deviceManagerClearAttributes(&attributes);
            if (!deviceManagerGetAttributes(&attributes, &public_taddr.addr))
            {
                /* No attributes exists for this device so write defaults for BLE - currently no default attributes. */
                attr_changed = TRUE;
            }

            switch(ccd_service)
            {
                case gatt_attr_service_battery_local:
                    if (attributes.gatt_client.battery_local != GATT_SERVER.client_config.battery_local)
                    {
                        attributes.gatt_client.battery_local = GATT_SERVER.client_config.battery_local;
                        attr_changed = TRUE;
                    }
                    break;
                case gatt_attr_service_battery_remote:
                    if (attributes.gatt_client.battery_remote != GATT_SERVER.client_config.battery_remote)
                    {
                        attributes.gatt_client.battery_remote = GATT_SERVER.client_config.battery_remote;
                        attr_changed = TRUE;
                    }
                    break;
                case gatt_attr_service_battery_peer:
                    if (attributes.gatt_client.battery_peer != GATT_SERVER.client_config.battery_peer)
                    {
                        attributes.gatt_client.battery_peer = GATT_SERVER.client_config.battery_peer;
                        attr_changed = TRUE;
                    }
                    break;
                case gatt_attr_service_heart_rate:
                    if (attributes.gatt_client.heart_rate != GATT_SERVER.client_config.heart_rate)
                    {
                        attributes.gatt_client.heart_rate = GATT_SERVER.client_config.heart_rate;
                        attr_changed = TRUE;
                    }
                    break;
                case gatt_attr_service_gatt:
                    if (attributes.gatt_client.gatt != GATT_SERVER.client_config.gatt)
                    {
                        attributes.gatt_client.gatt = GATT_SERVER.client_config.gatt;
                        attr_changed = TRUE;
                    }
                    break;
                case gatt_attr_service_all:
                    attributes.gatt_client = GATT_SERVER.client_config;
                    attr_changed = TRUE;
                    break;
                default:
                    /* No client config attributes handled for this service */
                    break;
            }
            
            GATT_SERVER_DEBUG(("GATT Server: Update client config; service[%u] changed[%u]\n", ccd_service, attr_changed));
                               
            if (attr_changed)
            {
                deviceManagerStoreAttributes(&attributes, &public_taddr.addr);
                GATT_SERVER_DEBUG(("GATT Server: Store client config addr[%x:%x:%lx] config_gatt[0x%x] config_batt_l[0x%x] config_batt_r[0x%x] config_batt_p[0x%x]\n",
                               public_taddr.addr.nap,
                               public_taddr.addr.uap,
                               public_taddr.addr.lap,
                               GATT_SERVER.client_config.gatt,
                               GATT_SERVER.client_config.battery_local,
                               GATT_SERVER.client_config.battery_remote,
                               GATT_SERVER.client_config.battery_peer
                               ));
            }
        }
    }
}

/***************************************************************************/
void gattServerSetServicePtr(uint16 **ptr, gatt_server_service_t service)
{
    uint16 index;
    for(index = 0; index < MAX_BLE_CONNECTIONS; index ++)
    {
        switch(service)
        {
            case gatt_server_service_gatt:
                GATT_SERVER.gatt_server = (GGATTS*)*ptr;
            break;
            case gatt_server_service_gap:
                GATT_SERVER.gap_server = (GGAPS*)*ptr;
            break;
            case gatt_server_service_bas_local:
                GATT_SERVER.bas_server_local = (GBASS*)*ptr;
            break;
            case gatt_server_service_bas_remote:
                GATT_SERVER.bas_server_remote = (GBASS*)*ptr;
            break;
            case gatt_server_service_bas_peer:
                GATT_SERVER.bas_server_peer = (GBASS*)*ptr;
            break;
            case gatt_server_service_hrs:
                GATT_SERVER.hrs_server = (GHRS_T*)*ptr;
            break;
#ifdef ACTIVITY_MONITORING
            case gatt_server_service_rscs:
                GATT_SERVER.rscs_server = (GRSCS_T*)*ptr;
            break;
            case gatt_server_service_logging:
                GATT_SERVER.logging_server = (GLOG_T*)*ptr;
            break;
#endif
            case gatt_server_service_ba:
                GATT_SERVER.ba_server = (GBSS *)*ptr;
            break;
#ifdef GATT_FP_SERVER
            case gatt_server_service_fps:
                GATT_SERVER.fps_server = (GFPS *)*ptr;
            break;
#endif
#ifdef GATT_AMA_SERVER
            case gatt_server_service_ama:
                GATT_SERVER.ama_server = (GAMASS*)*ptr;
            break;
#endif
#ifdef GATT_BISTO_COMM_SERVER
            case gatt_server_service_bisto_comm:
                GATT_SERVER.bisto_comm = (GBISTO_COMM_SS*)*ptr;
            break;
#endif
            default:
            break;
        }
    }
}

#endif /* GATT_ENABLED */

