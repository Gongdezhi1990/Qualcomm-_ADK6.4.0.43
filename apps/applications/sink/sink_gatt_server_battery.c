/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_battery.c

DESCRIPTION
    Routines to handle messages sent from the GATT Battery Server Task.
*/
#include "sink_ble.h"
#include "sink_gatt_db.h"
#include "sink_gatt_server_battery.h"
#include "sink_gatt_common.h"
#include "sink_debug.h"
#include "sink_development.h"
#include "sink_gatt_client_battery.h"
#include "sink_gatt_hid_remote_control.h"
#include "sink_gatt_server.h"
#include "sink_powermanager.h"
#include "sink_peer.h"

#include <gatt_battery_server.h>

#include <csrtypes.h>
#include <message.h>


#ifdef GATT_BATTERY_SERVER

#ifdef DEBUG_GATT_BATTERY_SERVER
#define GATT_BATTERY_SERVER_INFO(x) DEBUG(x)
#define GATT_BATTERY_SERVER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_BATTERY_SERVER_INFO(x)
#define GATT_BATTERY_SERVER_ERROR(x)
#endif


/* Assigned numbers for GATT Bluetooth Namespace Descriptors */ 
#define NAMESPACE_BLUETOOTH_SIG             0x01        /* Bluetooth SIG Namespace */ 
#define DESCRIPTION_BATTERY_UNKNOWN         0x0000      /* Bluetooth SIG description "unknown" */
#define DESCRIPTION_BATTERY_LOCAL           0x010F      /* Bluetooth SIG description "internal" for local battery*/
#define DESCRIPTION_BATTERY_SECOND          0x0002      /* Bluetooth SIG description "Second" for remote battery*/
#define DESCRIPTION_BATTERY_THIRD           0x0003      /* Bluetooth SIG description "third" for peer battery */

#define ENABLE_NOTIFICATIONS                            /* Enable notifications on the battery server */

#ifdef ENABLE_NOTIFICATIONS
#define BATTERY_SERVER_ENABLE_NOTIFICATIONS TRUE
#else
#define BATTERY_SERVER_ENABLE_NOTIFICATIONS FALSE
#endif


/******************************************************************************
NAME
    sinkBleBatteryLevelReadSendAndRepeat

DESCRIPTION
    Checks and dispatches the battery levels required for each gatt connection.
	If required, it starts a repeat timer.

PARAMETERS
	NONE

RETURNS
    NONE
*/
void sinkBleBatteryLevelReadSendAndRepeat(void)
{
  uint16 index;
  uint16 updateNeedsSending = 0;
  uint8 battery_level = 0;
  
  for( index = 0;index < MAX_BLE_CONNECTIONS;index++ )
  {
      if( LOCAL_UPDATE_REQD( GATT_SERVER ) )
      {
          battery_level = (uint8) powerManagerBatteryLevelAsPercentage();
        
          updateNeedsSending++;
          GATT_BATTERY_SERVER_INFO(("Sending GATT_SERVER[%d] local level\n", index));
          GattBatteryServerSendLevelNotification( GATT_SERVER.bas_server_local, 
          1, &(GATT[index].cid), battery_level);
      }

#if defined(GATT_BATTERY_SERVER_PEER) && defined(ENABLE_PEER)
      if( PEER_UPDATE_REQD( GATT_SERVER ) )
      {
          battery_level = peerGetBatteryLevel();
          updateNeedsSending++;
          GATT_BATTERY_SERVER_INFO(("Sending GATT_SERVER[%d] peer level\n", index));
          GattBatteryServerSendLevelNotification( GATT_SERVER.bas_server_peer, 
          1, &(GATT[index].cid),  battery_level);
      }
#endif

#ifdef GATT_BATTERY_SERVER_REMOTE
      if( REMOTE_UPDATE_REQD( GATT_SERVER ) )
      {
          battery_level = gattBatteryClientGetCachedLevel(sinkGattHidRcGetRemoteControlCid());
          updateNeedsSending++;
          GATT_BATTERY_SERVER_INFO(("Sending GATT_SERVER[%d] remote level\n", index));
          GattBatteryServerSendLevelNotification( GATT_SERVER.bas_server_remote, 
          1, &(GATT[index].cid),  battery_level);
      }
#endif

  }
  if( updateNeedsSending > 0 )
  {
    MessageSendLater( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER, 0, GATT_SERVER_BATTERY_UPDATE_TIME );
  }
}

/*******************************************************************************
NAME
    sinkGattGetBatteryServerHandles

DESCRIPTION
    Handles for the given Battery server.

PARAMETERS
    start      - battery server start handle to return.
    end        - battery server end handle to return.
    battery_id - Battery server ID

RETURNS
    TRUE if the Battery server available, FALSE otherwise.
*/
static bool sinkGattGetBatteryServerHandles(uint16 *start, uint16 *end, gatt_server_battery_id battery_id)
{
    bool status = FALSE;

    switch(battery_id)
    {
        case GATT_BATTERY_SERVER_LOCAL_ID:
            *start = HANDLE_BATTERY_SERVICE1;
            *end = HANDLE_BATTERY_SERVICE1_END;
            status = TRUE;
            break;

        case GATT_BATTERY_SERVER_REMOTE_ID:
#ifdef GATT_BATTERY_SERVER_REMOTE
            *start = HANDLE_BATTERY_SERVICE2;
            *end = HANDLE_BATTERY_SERVICE2_END;
            status = TRUE;
#endif
            break;

        case GATT_BATTERY_SERVER_PEER_ID:
#ifdef GATT_BATTERY_SERVER_PEER
            *start = HANDLE_BATTERY_SERVICE3;
            *end = HANDLE_BATTERY_SERVICE3_END;
            status = TRUE;
#endif
            break;

        default:
            break;
    };

    return status;
}


/*******************************************************************************
NAME
    sinkGattGetBatteryServerUpdateRundata
    
DESCRIPTION
    Updates rundata for the given server.
    
PARAMETERS
    ptr        - ointer to allocated memory to store server tasks rundata.
    battery_id - Battery server ID
    
RETURNS
    TRUE if the Battery server available, FALSE otherwise.
*/
static bool sinkGattGetBatteryServerUpdateRundata(uint16 **ptr, gatt_server_battery_id battery_id)
{
    bool status = FALSE;

    switch(battery_id)
    {
        case GATT_BATTERY_SERVER_LOCAL_ID:
            gattServerSetServicePtr(ptr, gatt_server_service_bas_local);
            *ptr += ADJ_GATT_STRUCT_OFFSET(GBASS);
            status = TRUE;
            break;

        case GATT_BATTERY_SERVER_REMOTE_ID:
#ifdef GATT_BATTERY_SERVER_REMOTE
            gattServerSetServicePtr(ptr, gatt_server_service_bas_remote);
            *ptr += ADJ_GATT_STRUCT_OFFSET(GBASS);
            status = TRUE;
#endif
            break;

        case GATT_BATTERY_SERVER_PEER_ID:
#ifdef GATT_BATTERY_SERVER_PEER
            gattServerSetServicePtr(ptr, gatt_server_service_bas_peer);
            *ptr += ADJ_GATT_STRUCT_OFFSET(GBASS);
            status = TRUE;
#endif
            break;

        default:
            break;
    };

    return status;
}

/*******************************************************************************
NAME
    sinkGattBatteryServerInitTask

DESCRIPTION
    Initialise a Battery server task.
    NOTE: This function will modify *ptr.

PARAMETERS
    ptr - pointer to allocated memory to store server tasks rundata.
    remote_battery - TRUE if a remote battery, FALSE otherwise.

RETURNS
    TRUE if the Battery server task was initialised, FALSE otherwise.
*/
static bool sinkGattBatteryServerInitTask(uint16 **ptr, gatt_server_battery_id battery_id)
{
    gatt_battery_server_init_params_t params = {BATTERY_SERVER_ENABLE_NOTIFICATIONS};
    uint16 start_handle;
    uint16 end_handle;

    if (!ptr)
    {
        return FALSE;
    }

    if(!sinkGattGetBatteryServerHandles(&start_handle, &end_handle, battery_id))
    {
        return FALSE;
    }

    if(!GattBatteryServerInit((GBASS*)*ptr, sinkGetBleTask(), &params,
                                        start_handle, end_handle))
    {
        GATT_BATTERY_SERVER_INFO(("GATT Battery Server init failed\n"));
        return FALSE;
    }

    if(!sinkGattGetBatteryServerUpdateRundata(ptr, battery_id))
    {
        return FALSE;
    }

    GATT_BATTERY_SERVER_INFO(("GATT Battery Server [%u] initialised\n", battery_id));
    return TRUE;
}


/*******************************************************************************/
uint16 sinkGattBatteryServerCalculateSize(void)
{
    /* Local battery server size */
    uint16 size = sizeof(GBASS);

#ifdef GATT_BATTERY_SERVER_REMOTE
    size += sizeof(GBASS);
#endif

#ifdef GATT_BATTERY_SERVER_PEER
    size += sizeof(GBASS);
#endif

    GATT_BATTERY_SERVER_INFO(("BAS=[%d]\n", size));
    return size;
}


/*******************************************************************************/
bool sinkGattBatteryServerInitialise(uint16 **ptr)
{
    /* Initialise battery server for local battery */
    if (!sinkGattBatteryServerInitTask(ptr, GATT_BATTERY_SERVER_LOCAL_ID))
    {
        return FALSE;
    }

#ifdef GATT_BATTERY_SERVER_REMOTE
    if (!sinkGattBatteryServerInitTask(ptr, GATT_BATTERY_SERVER_REMOTE_ID))
    {
        return FALSE;
    }
#endif

#ifdef GATT_BATTERY_SERVER_PEER
    if (!sinkGattBatteryServerInitTask(ptr, GATT_BATTERY_SERVER_PEER_ID))
    {
        return FALSE;
    }
#endif
    return TRUE;
}


/*******************************************************************************
NAME
    handleReadBatteryLevel

DESCRIPTION
    Handle when a GATT_BATTERY_SERVER_READ_LEVEL_IND message is recieved.

PARAMETERS
    ind Pointer to a GATT_BATTERY_SERVER_READ_LEVEL_IND message.

RETURNS
    void
*/
static void handleReadBatteryLevel(const GATT_BATTERY_SERVER_READ_LEVEL_IND_T * ind)
{
    uint8 battery_level = 0;
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    GATT_BATTERY_SERVER_INFO(("GATT_BATTERY_SERVER_READ_LEVEL_IND bas=[0x%p] cid=[0x%x]\n", (void *)ind->battery_server, ind->cid));

    if(index != GATT_INVALID_INDEX)
    {
        if (ind->battery_server == GATT_SERVER.bas_server_local)
        {
            /* Read internal battery level */
            battery_level = (uint8) powerManagerBatteryLevelAsPercentage();
        }

#ifdef GATT_BATTERY_SERVER_REMOTE
        else if (ind->battery_server == GATT_SERVER.bas_server_remote)
        {
            /* Read cached remote battery level */
            battery_level = gattBatteryClientGetCachedLevel(sinkGattHidRcGetRemoteControlCid());
        }
#endif

#if defined(GATT_BATTERY_SERVER_PEER) && defined(ENABLE_PEER)
        else if (ind->battery_server == GATT_SERVER.bas_server_peer)
        {
            /* Read the cached peer battery level */
            battery_level = peerGetBatteryLevel();
        }
#endif
    }
    GATT_BATTERY_SERVER_INFO(("    Return: level=[%u]\n", battery_level));

    /* Return requested battery level */
    GattBatteryServerReadLevelResponse(ind->battery_server, ind->cid,  battery_level);
}


/*******************************************************************************
NAME
    handleReadBatteryLevelClientConfig

DESCRIPTION
    Handle when a GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND message is recieved.

PARAMETERS
    ind Pointer to a GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleReadBatteryLevelClientConfig(const GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    uint16 client_config = 0;
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* Return the current value of the client configuration descriptor for the device */
    GATT_BATTERY_SERVER_INFO(("GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND bas=[0x%p] cid=[0x%x]\n", (void *)ind->battery_server, ind->cid));

    if(index != GATT_INVALID_INDEX)
    {
        if (ind->battery_server == GATT_SERVER.bas_server_local)
        {
            client_config = GATT_SERVER.client_config.battery_local;
            GATT_BATTERY_SERVER_INFO((" battery local\n"));
        }
        else if (ind->battery_server == GATT_SERVER.bas_server_remote)
        {
            client_config = GATT_SERVER.client_config.battery_remote;
            GATT_BATTERY_SERVER_INFO((" battery remote\n"));
        }
        else if (ind->battery_server == GATT_SERVER.bas_server_peer)
        {
            client_config = GATT_SERVER.client_config.battery_peer;
            GATT_BATTERY_SERVER_INFO((" battery peer\n"));
        }
    }

    GattBatteryServerReadClientConfigResponse(ind->battery_server, ind->cid, client_config);
    GATT_BATTERY_SERVER_INFO(("  client_config=[0x%x]\n", client_config));
}


/*******************************************************************************
NAME
    handleWriteBatteryLevelClientConfig

DESCRIPTION
    Handle when a GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND message is recieved.

PARAMETERS
    ind Pointer to a GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleWriteBatteryLevelClientConfig(const GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* 
     * Check whether the remote device has enabled or disabled 
     * notifications for the Battery Level characteristic. This value will need
     * to be stored as device attributes so they are persistent.
    */
    GATT_BATTERY_SERVER_INFO(("GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND bas=[0x%p] cid=[0x%x] value[0x%x]\n", 
        (void *)ind->battery_server, ind->cid, ind->config_value));

    if (index != GATT_INVALID_INDEX)
    {
        unsigned short updateNeedsSending = 0;
        if (ind->battery_server == GATT_SERVER.bas_server_local)
        {
            GATT_SERVER.client_config.battery_local = ind->config_value;
            GATT_BATTERY_SERVER_INFO((" battery local client_config[0x%x]\n", GATT_SERVER.client_config.battery_local));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_battery_local);
            
            if( LOCAL_UPDATE_REQD(GATT_SERVER) )
            {
                updateNeedsSending++;
            }
        }
        else if (ind->battery_server == GATT_SERVER.bas_server_remote)
        {
            GATT_SERVER.client_config.battery_remote = ind->config_value;
            GATT_BATTERY_SERVER_INFO((" battery remote client_config[0x%x]\n", GATT_SERVER.client_config.battery_remote));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_battery_remote);
            
            if( REMOTE_UPDATE_REQD(GATT_SERVER) )
            {
                updateNeedsSending++;
            }
        }
        else if (ind->battery_server == GATT_SERVER.bas_server_peer)
        {
            GATT_SERVER.client_config.battery_peer = ind->config_value;
            GATT_BATTERY_SERVER_INFO((" battery peer client_config[0x%x]\n", GATT_SERVER.client_config.battery_peer));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_battery_peer);
            
            if( PEER_UPDATE_REQD(GATT_SERVER) )
            {
                updateNeedsSending++;
            }
        }

        if( updateNeedsSending > 0 )
        {
            MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER );
            MessageSend( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER, 0 );
        }
    }
}


/*******************************************************************************
NAME
    handleReadBatteryPresentation

DESCRIPTION
    Handle when a GATT_BATTERY_SERVER_READ_PRESENTATION_IND message is recieved.

PARAMETERS
    ind Pointer to a GATT_BATTERY_SERVER_READ_PRESENTATION_IND message.

RETURNS
    void
*/
static void handleReadBatteryPresentation(const GATT_BATTERY_SERVER_READ_PRESENTATION_IND_T * ind)
{
    uint16 description = DESCRIPTION_BATTERY_UNKNOWN;
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    GATT_BATTERY_SERVER_INFO(("GATT_BATTERY_SERVER_READ_PRESENTATION_IND bas=[0x%p] cid=[0x%x]\n", (void *)ind->battery_server, ind->cid));
    if(index != GATT_INVALID_INDEX)
    {
        if (ind->battery_server == GATT_SERVER.bas_server_local)
        {
            description = DESCRIPTION_BATTERY_LOCAL;
        }
        else if (ind->battery_server == GATT_SERVER.bas_server_remote)
        {
            description = DESCRIPTION_BATTERY_SECOND;
        }
        else if (ind->battery_server == GATT_SERVER.bas_server_peer)
        {
            description = DESCRIPTION_BATTERY_THIRD;
        }
    }

    GattBatteryServerReadPresentationResponse(ind->battery_server,
                                                  ind->cid,
                                                  NAMESPACE_BLUETOOTH_SIG,
                                                  description);

    GATT_BATTERY_SERVER_INFO(("   Return: desc=[0x%x]\n", description));
}


/******************************************************************************/
void sinkGattBatteryServerMsgHandler(Task task, MessageId id, Message message)
{
   UNUSED(task);
    switch(id)
    {
        case GATT_BATTERY_SERVER_READ_LEVEL_IND:
        {
            handleReadBatteryLevel((const GATT_BATTERY_SERVER_READ_LEVEL_IND_T*)message);
        }
        break;
        case GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND:
        {
            handleReadBatteryLevelClientConfig((const GATT_BATTERY_SERVER_READ_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            handleWriteBatteryLevelClientConfig((const GATT_BATTERY_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_BATTERY_SERVER_READ_PRESENTATION_IND:
        {
            handleReadBatteryPresentation((const GATT_BATTERY_SERVER_READ_PRESENTATION_IND_T*)message);
        }
        break;
        default:
        {
            /* Un-handled messages are not critical errors */
            GATT_BATTERY_SERVER_ERROR(("GATT Un-handled msg id[%x]\n", id));
        }
    }
}

#endif /* GATT_BATTERY_SERVER */

bool sinkGattBatteryServiceEnabled(void)
{
#ifdef GATT_BATTERY_SERVER
    return TRUE;
#else
    return FALSE;
#endif
}
