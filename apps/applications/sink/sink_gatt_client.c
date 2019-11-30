/****************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client.c

DESCRIPTION
    Routines to handle the GATT Clients.
*/

#include "sink_gatt_client.h"

#include "sink_gatt.h"
#include "sink_ble.h"
#include "sink_debug.h"
#include "sink_gatt_client_battery.h"
#ifdef GATT_AMS_CLIENT
#include "sink_gatt_client_ams.h"
#endif
#include "sink_gatt_client_ancs.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_client_hid.h"
#include "sink_gatt_client_dis.h"
#include "sink_gatt_client_gatt.h"
#include "sink_gatt_client_spc.h"
#include "sink_gatt_client_hrs.h"
#include "sink_gatt_client_ba.h"
#include "sink_gatt_device.h"
#include "sink_gatt_common.h"
#include "sink_ba_receiver.h"
#include "sink_ba_ble_gap.h"

#include <vm.h>
#include <gatt_manager.h>
#include <gatt_broadcast_server_uuids.h>
#include <message.h>

#include <stdlib.h>

#ifdef GATT_ENABLED

#ifdef DEBUG_GATT_CLIENT
#define GATT_CLIENT_DEBUG(x) DEBUG(x)
const char * const gatt_clients[] = {
    "BAS Service",
    "ANCS Service",
    "IAS Service",
    "HID Service",
    "DIS Service",
    "GATT Service",
    "SPC Service",
    "HRS Service",
    "BA Service"
#ifdef GATT_AMS_CLIENT
    ,
    "AMS_SERVICE"
#endif
};

#else
#define GATT_CLIENT_DEBUG(x) 
#endif


#define GATT_CLIENT_DISCOVERY_ALLOC_MEM(ptr) \
{ \
    ptr = PanicUnlessMalloc(sizeof(gatt_client_discovery_t) * MAX_GATT_CLIENT_SERVICES); \
    memset(ptr, 0, sizeof(gatt_client_discovery_t) * MAX_GATT_CLIENT_SERVICES); \
    GATT_CLIENT_DEBUG(("Mem Alloc (Gatt Client - Discovery): size[%u] addr[%p]\n", \
                       sizeof(gatt_client_discovery_t) * MAX_GATT_CLIENT_SERVICES, \
                       (void *)ptr)); \
}

#define GATT_CLIENT_SERVICES_ALLOC_MEM(ptr) \
{ \
    ptr = PanicUnlessMalloc(sizeof(gatt_client_services_t)); \
    memset(ptr, 0, sizeof(gatt_client_services_t)); \
    GATT_CLIENT_DEBUG(("Mem Alloc (Gatt Client - Services): size[%u] addr[%p]\n", \
                       sizeof(gatt_client_services_t), \
                       (void *)ptr)); \
}

#define GATT_CLIENT_FREE_MEM(ptr) \
{ \
    GATT_CLIENT_DEBUG(("Mem Free (Gatt Client): addr[%p]\n", \
                       (void *)ptr)); \
    free(ptr); \
    ptr = NULL; \
}


/****************************************************************************/
static gatt_client_services_t* gattClientReserveMemory(gatt_client_connection_t *connection, uint16 size)
{
    if (connection)
    {
        gatt_client_services_t *services = connection->data;
        uint16 size_client_data = sizeof(gatt_client_services_t) + services->size_client_data + size;
        gatt_client_services_t *data = realloc(connection->data, size_client_data);
        GATT_CLIENT_DEBUG(("Mem Re-Alloc (Gatt Client - Services): size[%u] addr[%p]\n", size_client_data, (void*)data));
        
        if (data)
        {
            data->size_client_data = size_client_data - sizeof(gatt_client_services_t);
            connection->data = data;
            return connection->data;
        }
    }
    return NULL;
}

/****************************************************************************
NAME    
    gattClientDiscoveryComplete
    
DESCRIPTION
    Called when discovery is complete.
    
PARAMETERS
    connection  The GATT connection
    
NOTE
    This function MUST be called with a non-NULL *connection argument.
*/
static void gattClientDiscoveryComplete(gatt_client_connection_t *connection)
{
    gatt_client_services_t *data = gattClientGetServiceData(connection);

    /* Panic if Connection is NULL */
    PanicNull(connection);
    
    GATT_CLIENT_DEBUG(("GATT Client Discovery Complete\n"));
    
    if (data)
    {
        GATT_CLIENT_FREE_MEM(data->discovery);
    }
    
    if(connection->role == ble_gap_role_central)
    {
        /* Connection is now complete after setting security */
        sinkBleMasterConnCompleteEvent(gattClientFindByConnection(connection));
    }
    else
    {
        /* in case of peripheral, we need to post slave connected complete */
        sinkBleSlaveConnIndEvent(gattClientFindByConnection(connection));
    }
}


/****************************************************************************
NAME    
    initialiseGattClientService
    
DESCRIPTION
    Called to initialise the GATT service before all other services.
    This is so Service Changed indications can be captured.
    
PARAMETERS
    connection  The connection pointer

*/
static void initialiseGattClientService(gatt_client_connection_t *connection)
{
    gatt_client_discovery_t *discover = gattClientGetDiscoveredServices(connection);
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    bool gatt_service_found = FALSE;

    if (discover && data)
    {
        gatt_service_found = gattClientIsServiceAvailable(connection, gatt_client_gatt);
        if(gatt_service_found)
        {
            gatt_service_found = sinkGattClientServiceAdd(gattClientFindByConnection(connection), 
                                                                                discover->start_handle, discover->end_handle);
            GATT_CLIENT_DEBUG(("Add gatt client service; success[%u] cid[0x%x] start[0x%x] end[0x%x]\n", 
                               gatt_service_found,
                               gattClientFindByConnection(connection), 
                               discover->start_handle,
                               discover->end_handle));
        }
        
        if (!gatt_service_found)
        {
            /* An error has occurred trying to process GATT service.
               Attempt to continue operation and check for security requirements. */
            gattClientProcessSecurityRequirements(connection, data);
        }
    }
}

/****************************************************************************
NAME    
    gattClientIsServiceAvailable
    
DESCRIPTION
    This function is used to find out if the client connection supports the particular service.
    
PARAMETERS
    connection  The connection pointer
    service The service which needs to be checked for 

RETURNS
    TRUE if the service is available, FALSE otherwise

*/
bool gattClientIsServiceAvailable(gatt_client_connection_t *connection, gatt_client_tasks_t service)
{
    gatt_client_discovery_t *discover = gattClientGetDiscoveredServices(connection);
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    uint16 service_count = 0;

    if (discover && data)
    {
        for (service_count = 0 ; service_count < data->number_discovered_services ; service_count++)
        {
            if ((discover) && (discover->service == service))
            {
                return TRUE;
            }
            /* Increment to next discovery data */
            discover +=1;
        }
    }
    return FALSE;
}

/****************************************************************************
NAME    
    gattClientConnection
    
DESCRIPTION
    Return GATT Client Connection
    
PARAMETERS
    Index of GATT Connection

RETURNS
    Connection pointer to client connection

*/
gatt_client_connection_t *gattClientConnection(uint16 index)
{
    return &(GATT_CLIENT.connection);
}

/****************************************************************************
NAME    
    gattDiscoverPrimaryServices
    
DESCRIPTION
    Discovers GATT primary services supported on the remote device.
    
PARAMETERS
    connection             The GATT connection

*/
void gattDiscoverPrimaryServices(gatt_client_connection_t *connection)
{
    if(BA_RECEIVER_MODE_ACTIVE && gapBaGetAssociationInProgress())
    {
        /* In receiver mode, we are interested only in BA service. 
            However, receiver needs to support 2 different BA UUIDs
            So, first check for 0xFE8F, if failed check if the broadcaster supports 0xFE06
            */
        gatt_uuid_t ba_uuid = UUID_BROADCAST_SERVICE & 0x0000FFFF;
        GattDiscoverPrimaryServiceRequest(sinkGetBleTask(), 
                                                              gattClientFindByConnection(connection), 
                                                              gatt_uuid16,
                                                              &ba_uuid);
    }
    else
    {
        GattDiscoverAllPrimaryServicesRequest(sinkGetBleTask(), gattClientFindByConnection(connection));
    }
}

/****************************************************************************/
gatt_client_discovery_t *gattClientGetDiscoveredServices(gatt_client_connection_t *connection)
{
    gatt_client_services_t *service = NULL;
    
    if (connection)
    {
        service = gattClientGetServiceData(connection);
        
        if (service)
        {
            return service->discovery;
        }
    }
    
    return NULL;
}

/****************************************************************************/
void gattClientInitialiseDiscoveredServices(gatt_client_connection_t *connection)
{
    gatt_client_discovery_t *discover = gattClientGetDiscoveredServices(connection);
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    uint16 cid = gattClientFindByConnection(connection);
    bool error_flag = TRUE;

    if (discover && data)
    {
        /* Advance the discover pointer based on the current discovered service */
        discover += data->current_discovered_service;
        if(discover)
        {
            GATT_CLIENT_DEBUG(("GATT Discovered Service: cid[0x%x] index[%u]\n", cid, data->current_discovered_service));
            
            GATT_CLIENT_DEBUG(("    service[%u] start[0x%x] end[0x%x]\n", discover->service, discover->start_handle, discover->end_handle));
            switch (discover->service)
            {
                case gatt_client_battery:
                {
                    if (gattBatteryClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;
                case gatt_client_ancs:
                {
                    if (sinkGattAncsClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;
#ifdef GATT_AMS_CLIENT
                case gatt_client_ams:
                {
                    if (sinkGattAmsClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;
#endif
                case gatt_client_ias:
                {
                    if (sinkGattIasClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;
                case gatt_client_hid:
                {
                    if (sinkGattHidClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;
                case gatt_client_dis:
                {
                    if (sinkGattDisClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;

                case gatt_client_spc:
                {
                    if (sinkGattSpClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;

                case gatt_client_hrs:
                {
                    if (sinkGattHrsClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;

                case gatt_client_ba:
                {
                    if(sinkGattBAClientAddService(cid, discover->start_handle, discover->end_handle))
                    {
                        error_flag = FALSE;
                    }
                }
                break;
                
                case gatt_client_gatt:
                {
                    /* Gatt service will already have been initialised before other services.
                       Send internal message to process next service */
                    MESSAGE_MAKE(message, BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED_T);
                    message->cid = cid;
                    MessageSend(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED, message);
                    error_flag = FALSE;
                }
                break;
                default:
                {
                    GATT_CLIENT_DEBUG(("GATT unknown discovered service\n"));
                }
                break;
            }
        }
    }
    if (error_flag)
    {
        MESSAGE_MAKE(message, BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR_T);
        /*message requires the pointer to gatt_client_connection_t as cid is  found using this address in gattClientFindByConnection() later on*/
        message->connection = connection;
        MessageSend(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR, message);
    }
} 


/****************************************************************************/
bool gattClientAdd(uint16 cid, ble_gap_role_t client_gap_role)
{
    gatt_client_connection_t *connection = NULL;
    gatt_client_services_t *services = NULL;
    tp_bdaddr *tp_addr = NULL;
    uint16 index = GATT_INVALID_INDEX;

    if(client_gap_role == ble_gap_role_central)
    {
        /* Master connect flag shall be set as soon as a connection has been triggered in central role
         * However @ that moment "cid" is not available, so its now time to update
         * the "cid". After this, when there is disconnect, we can reset the conn flags
         * using the cid 
         * Please note that for gap_role_peripheral, the conn flag for master connection attempt
         * is not set, because there is no trigger of master_connection_attempt. So in peripheral 
         * role, this flag shall not be set */
        tp_addr = PanicUnlessMalloc(sizeof(tp_bdaddr));
        if(VmGetBdAddrtFromCid(cid, tp_addr))
        {
            index = sinkBleGapFindGapConnFlagIndex((const typed_bdaddr*)&tp_addr->taddr);
        }

        /*In case of central, check and update the  connection id*/
        if(!gattCommonAddConnections(cid, ble_gap_role_central))
        {
            /* So, could not add the connection, then reset the connection attempt flag */
            sinkBleGapSetCentralConnAttempt(FALSE, NULL, 0, index);
             /* Free the allocated memory */
            free(tp_addr);
            tp_addr = NULL;
            
            return FALSE;
        }
        /* Okay, was able to add the  connection, now if we run out of memory while allocation for
            discovery pointer, then the caller shall trigger disconnect of the link, on recieving disconnect 
            indication we shall reset the connection attempt flag */
        sinkBleGapSetCentralConnAttempt(TRUE, &tp_addr->taddr, cid, index);
        /* Free the allocated memory */
        free(tp_addr);
        tp_addr = NULL;
    }

    /* Get the index to point to the correct GATT_CLIENT */
    index = gattCommonConnectionFindByCid(cid);
    connection = &(GATT_CLIENT.connection);

    GATT_CLIENT_SERVICES_ALLOC_MEM(services);

                
    if (services)
    {   
        /* Record details of the services */
        connection->data = services;

        /* Set the connection role */
        connection->role = client_gap_role;
        
        GATT_CLIENT_DISCOVERY_ALLOC_MEM(services->discovery);
        
        if (services->discovery)
        {
            /* Set MTU and kick off discovery once MTU configured */
            GattExchangeMtuRequest(sinkGetBleTask(), cid, sinkGattGetMinimumTxMtu());
            return TRUE;
        }
        else
        {
            GATT_CLIENT_FREE_MEM(services);
        }
    }
    return FALSE;
}


/****************************************************************************/
gatt_client_connection_t *gattClientFindByCid(uint16 cid)
{
    uint16 index = 0;

    index =  gattCommonConnectionFindByCid(cid);
    if(index != GATT_INVALID_INDEX)
    {
        GATT_CLIENT_DEBUG(("(Gatt Client - Connection): addr[%p]\n",  (void*)&(GATT_CLIENT.connection)));
        return &(GATT_CLIENT.connection);
    }
    return NULL;
}

/****************************************************************************/
bool gattClientRemove(uint16 cid)
{
    gatt_client_connection_t *connection = gattClientFindByCid(cid);

    if (connection && BLE_DATA.number_connections)
    {
        /* if central role, then reset the cid */
        if(connection->role == ble_gap_role_central)
        {
            gattCommonRemoveConnections(cid);
        }
        
        connection->role = ble_gap_role_unknown;
        if (connection->data)
        {
            gatt_client_discovery_t *discover = gattClientGetDiscoveredServices(connection);
            if (discover)
            {
                GATT_CLIENT_FREE_MEM(discover);
            }
            GATT_CLIENT_FREE_MEM(connection->data);
        }
        return TRUE;
    }
    
    return FALSE;
}

/****************************************************************************/
uint16 *gattClientGetServicePtr(gatt_client_connection_t *connection, uint16 size_service)
{
    if ((connection) && (connection->data))
    {
        gatt_client_services_t *services = connection->data;
        uint16 size_client_data = sizeof(gatt_client_services_t) + services->size_client_data + size_service;
        uint16 *end = NULL;

        end = services->client_data + (services->size_client_data/sizeof(uint16));
        services->size_client_data = size_client_data - sizeof(gatt_client_services_t);
        GATT_CLIENT_DEBUG(("(Gatt Client - Size of Services): size[%lu] addr[%p]\n", services->size_client_data, (void*)end));
        return end;
    }
    return NULL;
}

/****************************************************************************\
 * NOTE: This utility function should be called only in case initialization of client library fails. If called in
 * other situation then the entire client_data shall be corrupted.
*****************************************************************************/
static void gattClientDeleteLastService(gatt_client_connection_t *connection, uint16 size_service)
{
    if (connection)
    {
        gatt_client_services_t *services = connection->data;
        uint16 size_client_data = sizeof(gatt_client_services_t) + services->size_client_data - size_service;
        services->size_client_data = size_client_data - sizeof(gatt_client_services_t);
    }
}

/****************************************************************************
NAME    
    gattClientGetNumServiceInstance
    
DESCRIPTION
    This function is used to find out the number of instance of a particular service
    
PARAMETERS
    connection  The connection pointer
    service The service which needs to be checked for 

RETURNS
    Number of instances

*/
static uint16 gattClientGetNumServiceInstance(gatt_client_connection_t *connection, gatt_client_tasks_t service)
{
    gatt_client_discovery_t *discover = gattClientGetDiscoveredServices(connection);
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    uint16 service_count = 0;
    uint16 inst_count = 0;

    if (discover && data)
    {
        for (service_count = 0 ; service_count < data->number_discovered_services ; service_count++)
        {
            if ((discover) && (discover->service == service))
            {
                inst_count++;
            }
            /* Increment to next discovery data */
            discover +=1;
        }
    }
    return inst_count;
}

/****************************************************************************
NAME    
    gattClientCanAddClientService
    
DESCRIPTION
    This function is used to find out if the particular client service can be added or not based on
    number of instances
    
PARAMETERS
    connection  The connection pointer
    service The service which needs to be checked for 

RETURNS
    TRUE if possible, else FALSE

*/
static bool gattClientCanAddClientService(gatt_client_connection_t *connection, gatt_client_tasks_t service)
{
    switch(service)
    {
        /* Follow through cases which allows only one instance */
        case gatt_client_battery:
        case gatt_client_ancs:
        case gatt_client_ias:
        case gatt_client_dis:
        case gatt_client_gatt:
        case gatt_client_spc:
        case gatt_client_hrs:
        case gatt_client_ba:
#ifdef GATT_AMS_CLIENT
        case gatt_client_ams:
#endif
        {
            if(gattClientGetNumServiceInstance(connection, service) < MAX_ONE_INSTANCE)
                return TRUE;
        }
        break;

        /* For hid we allow 2 instances */
        case gatt_client_hid:
        {
            if(gattClientGetNumServiceInstance(connection, service) < MAX_TWO_INSTANCE)
                return TRUE;
        }
        break;
        default:
        break;
    }
    return FALSE;
}
/****************************************************************************
NAME : sinkGattClientUpdateSlaveConenctionParams

DESCRIPTION: We are in peripheral role, and found that the remote client doesn't
             support any service. However the remote client shall be using our services
             so we need to update the connection parameter 
***************************************************************************/

static void sinkGattClientUpdateSlaveConenctionParams (uint16 cid)
{
    tp_bdaddr *tpaddrt = PanicUnlessMalloc(sizeof(tp_bdaddr));
    if(VmGetBdAddrtFromCid(cid, tpaddrt))
    {
        sinkBleSetSlaveConnectionParamsUpdate(&tpaddrt->taddr);
    }
    free(tpaddrt);
}

/****************************************************************************/
static bool gattClientAddService(gatt_client_services_t *data, 
                                                       gatt_client_discovery_t *discover, gatt_client_connection_t *conn, 
                                                       gatt_client_tasks_t service, uint16 size, uint16 start, uint16 end) 
{
    if(gattClientCanAddClientService(conn, service))
    {
        GATT_CLIENT_DEBUG(("Gatt Client Storing %s handles\n", gatt_clients[service-1]));
        data = gattClientReserveMemory(conn, size);
        if(data)
        {
            /* Add the service to the discovery pointer */
            discover->service = service;
            discover->start_handle = start;
            discover->end_handle = end;
            
            data->number_discovered_services++;
            return TRUE;
        }
    }

    return FALSE;
}

/****************************************************************************/
void gattClientStoreDiscoveredService(uint16 cid, uint16 uuid_type, uint32 *uuid, uint16 start, uint16 end, bool more)
{
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    gatt_client_discovery_t *discover = NULL;
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    bool error_flag = TRUE;
    tp_bdaddr *tp_addr = NULL;

    GATT_CLIENT_DEBUG(("Gatt Client Store Discovered Service \n"));

    if (connection && data)
    {
        discover = gattClientGetDiscoveredServices(connection);
        if (discover)
        {
            /* The discover poiter has to be advanced by the number of services found to update the new service */
            discover += data->number_discovered_services;
            if (discover)
            {
                /* Now check if we have not yet discovered max services */
                if(!HAS_MAX_SERVICES_DISCOVERED(data->number_discovered_services))
                {
                    if(uuid_type == gatt_uuid16)
                    {
                        switch (uuid[0])
                        {
                            case GATT_SERVICE_UUID_BATTERY_SERVICE:
                            {
                                /* As per CS-326396-DD connection spec, in peripheral role, 
                                    app should initialize only ANCS and IAS client */
                                if(sinkGattBatteryClientEnabled() && (ble_gap_role_central == connection->role))
                                {
                                    /* Ok, found the battery service in remote device, reserve memory for this service 
                                        On successful memory reservation, update the discovery pointer with the service
                                        so that the particular service library is initialized later. The same holds good with other
                                        services too */
                                    gattClientAddService(data, discover, connection, gatt_client_battery, sinkGattBatteryClientGetSize(), start, end);
                                }
                            }
                            break;

                            case GATT_SERVICE_UUID_HUMAN_INTERFACE_DEVICE:
                            {
                                if(sinkGattHidClientEnabled() && (ble_gap_role_central == connection->role))
                                {
                                    /* If we were able to add HID instance, then we can think of adding remote control instance */
                                    if((gattClientAddService(data, discover, connection, gatt_client_hid, sinkGattHidClientGetSize(), start, end)) && 
                                       (gattClientGetNumServiceInstance(connection, gatt_client_hid) == MAX_ONE_INSTANCE))
                                    {
                                        /* Ok, adding the first instance of HID, so reserve the memory for remote control instance */
                                        gattClientReserveMemory(connection, sinkGattRemoteControlGetSize());
                                    }
                                }
                            }
                            break;
                            
                            case GATT_SERVICE_UUID_IMMEDIATE_ALERT:
                            {
                                if(sinkGattIasClientEnabled())
                                {
                                    gattClientAddService(data, discover, connection, gatt_client_ias, sinkGattIasClientGetSize(), start, end);
                                }
                            }
                            break;

                            case GATT_SERVICE_UUID_DEVICE_INFORMATION:
                            {   
                                if(sinkGattDisClientEnabled())
                                {
                                    gattClientAddService(data, discover, connection, gatt_client_dis, sinkGattDisClientGetSize(), start, end);
                                }
                            }
                            break;
                            case GATT_SERVICE_UUID_GENERIC_ATTRIBUTE:
                            {  
                                gattClientAddService(data, discover, connection, gatt_client_gatt, sinkGattClientGetSize(), start, end);
                            }
                            break;

                            case GATT_SERVICE_UUID_SCAN_PARAMETERS:
                            {   
                                if(sinkGattSpClientEnabled())
                                {
                                    gattClientAddService(data, discover, connection, gatt_client_spc, sinkGattSpClientGetSize(), start, end);
                                }
                            }
                            break;

                            case GATT_SERVICE_UUID_HEART_RATE:
                            {      
                                if(sinkGattHrsClientEnabled() && (ble_gap_role_central == connection->role))
                                {
                                    gattClientAddService(data, discover, connection, gatt_client_hrs, sinkGattHrsClientGetSize(), start, end);
                                }
                            }
                            break;
                            /* Follow thru cases */                            
                            case UUID_BROADCAST_SERVICE:
                            case UUID_BROADCAST_SERVICE_2:
                            {
                                if(BA_RECEIVER_MODE_ACTIVE && sinkReceiverIsBroadcasterCid(cid))
                                {
                                    gattClientAddService(data, discover, connection, gatt_client_ba, sinkGattBAClientGetSize(), start, end);
                                }
                            }
                            break;
                            /* Handle other services that client is interested in */
                            default:
                            {
                                /* Ignore unknown services */
                            }
                            break;
                        }
                    }
                    else if(uuid_type == gatt_uuid128)
                    {  
                        if(CHECK_ANCS_SERVICE_UUID(uuid))
                        {
                            gattClientAddService(data, discover, connection, gatt_client_ancs, sinkGattAncsClientGetSize(), start, end);
                        }
#ifdef GATT_AMS_CLIENT
                        if(CHECK_AMS_SERVICE_UUID(uuid))
                        {
                            gattClientAddService(data, discover, connection, gatt_client_ams, sinkGattAmsClientGetSize(), start, end);
                        }
#endif
                    }
                }

                if (!more)
                {
                    /* Just get the update data pointer (if at all it has changed) */
                    data = gattClientGetServiceData(connection);
                    /* We are done with getting the primary service
                       reset the service discovery flag*/
                    if(connection->role == ble_gap_role_peripheral)
                    {
                        tp_addr = PanicUnlessMalloc(sizeof(tp_bdaddr));

                        VmGetBdAddrtFromCid(cid, tp_addr);

                        sinkBleGapSetDiscoveryInProgress(FALSE, &tp_addr->taddr, cid, sinkBleGapFindGapConnFlagIndexByCid(cid));

                        /* Free the allocated memory */
                        free(tp_addr);
                        tp_addr = NULL;
                    }
                    
                    if (data && data->number_discovered_services)
                    {
                        /* Ok, we are now going to start initializing server-by-service in the order in which they were
                           discovered. As we have already reserved the memory for the services, we need to reset the
                           client size member because using this member we can find the offset of each service ptr within client_data */
                        data->size_client_data = 0;
                        /* Initialise GATT service first to handle any Service Changed indications */
                        initialiseGattClientService(connection);
                    }
                    else
                    {
                        if(connection->role == ble_gap_role_central)
                        {
                            /* In case the client was connected in central role, then this client connection is not required
                             * because remote device does not have any service of our interest */
                             gattClientDisconnect(cid);
                        }
                        else
                        {
                            if(BA_RECEIVER_MODE_ACTIVE && sinkReceiverIsBroadcasterCid(cid))
                            {
                                /* The remote connected device is actually not broadcaster. We are expecting
                                    broadcaster to connect for association. So, no point in continuing the connection */
                                gattClientDisconnect(cid);
                            }
                            else
                            {
                                /* We are in peripheral role, and found that the remote client doesn't
                                 * support any service. However the remote client shall be using our services
                                 * so we need to update the connection parameter */
                                sinkGattClientUpdateSlaveConenctionParams(cid);
                                /* In case the client was connected in peripheral role, then just remove the client
                                 * Because server might be still connected */
                                gattClientRemove(cid);
                            }
                        }
                    }
                }
                error_flag = FALSE;
            }
        }
    }
    
    if (error_flag)
    {
        gattClientDiscoveryError(connection);
    }
}


/****************************************************************************/
gatt_client_services_t *gattClientGetServiceData(gatt_client_connection_t *connection)
{
    if (connection)
    {
        return connection->data;
    }
    
    return NULL;
}


/****************************************************************************/
void gattClientDiscoveredServiceInitialised(gatt_client_connection_t *connection)
{
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    
    if (connection && connection->service_changed)
    {
        if (data)
        {
            GATT_CLIENT_FREE_MEM(data->discovery);
        }
        gattClientServiceChanged(gattClientFindByConnection(connection));
        
        /* Return early as discovery will be restarted */
        return;
    }
    
    if (data)
    {
        data->current_discovered_service++;
        if (data->current_discovered_service >= data->number_discovered_services)
        {
            GATT_CLIENT_DEBUG(("GATT Primary Services Initialised\n"));
            /* Discovery of primary services is now complete */
            gattClientDiscoveryComplete(connection);
        }
        else
        {
            gattClientInitialiseDiscoveredServices(connection);
        }
    }
    else
    {
        gattClientDiscoveryError(connection);
    }
}


/****************************************************************************/
bool gattClientRemoveDiscoveredService(gatt_client_connection_t *connection, 
                                       gatt_client_tasks_t service)
{
    bool retVal = FALSE;
    gatt_client_services_t *data = gattClientGetServiceData(connection);
    if (data)
    {
        /* Move to the last discovered service */
        switch(service)
        {
            case gatt_client_ancs:
            {
                if(data->ancs)
                {
                    sinkGattAncsClientRemoveService(data->ancs);
                    gattClientDeleteLastService(connection, sizeof(GANCS));
                    if(data && data->ancs)
                    {
                        data->ancs = NULL;
                    }
                    retVal = TRUE;
                }
            }
            break;
#ifdef GATT_AMS_CLIENT
            case gatt_client_ams:
            {
                if(data->ams)
                {
                    sinkGattAmsClientRemoveService(data->ams);
                    gattClientDeleteLastService(connection, sizeof(GAMS));
                    if(data && data->ams)
                    {
                        data->ams = NULL;
                    }
                    retVal = TRUE;
                }
            }
            break;
#endif
            case gatt_client_spc:
            {
                if(data->spc)
                {
                    sinkGattSpClientRemoveService(data->spc);
                    gattClientDeleteLastService(connection, sizeof(GSPC_T));
                    if(data && data->spc)
                    {
                        data->spc = NULL;
                    }
                    retVal = TRUE;
                }
            }
            break;
            
            case gatt_client_hrs:
            {
                if(data->hrs)
                {
                    sinkGattHrsClientRemoveService(data->hrs);
                    gattClientDeleteLastService(connection, sizeof(GHRSC_T));
                    if(data && data->hrs)
                    {
                        data->hrs = NULL;
                    }
                    retVal = TRUE;
                }
            }
            break;
            
            default:
            break;
        }
    }

    return retVal;
}

/****************************************************************************/
void gattClientDisconnect(uint16 cid)
{
    if (gattClientFindByCid(cid))
    {
        GattManagerDisconnectRequest(cid);
    }
}


/****************************************************************************/
bool gattClientDisconnectAll(void)
{
    return gattCommonDisconnectAll(TRUE);
}


/***************************************************************************/
void gattClientDiscoveryError(gatt_client_connection_t *connection)
{
    tp_bdaddr *tp_addr = NULL;
    GATT_CLIENT_DEBUG(("GATT Client Discovery Error!\n"));

    if (connection)
    {
        /* Reset the primary service discovery flag */
        if(connection->role == ble_gap_role_peripheral)
        {
            uint16 cid = gattClientFindByConnection(connection);
            tp_addr = PanicUnlessMalloc(sizeof(tp_bdaddr));

            VmGetBdAddrtFromCid(cid, tp_addr);

            sinkBleGapSetDiscoveryInProgress(FALSE, &tp_addr->taddr, cid, sinkBleGapFindGapConnFlagIndexByCid(cid));

            /* Free the allocated memory */
            free(tp_addr);
            tp_addr = NULL;
        }
        gattClientDiscoveryComplete(connection);
    }
}

/***************************************************************************/
static uint16 gattClientGetNumberOfCentralConnected(void)
{
    uint16 central_conn = 0;
    uint16 index =0;

    /* Find the number of central device connected */
    for(index = 0; index < MAX_BLE_CONNECTIONS;index++)
    {
        if(GATT_CLIENT.connection.role == ble_gap_role_central)
            central_conn++;
    }
    return central_conn;
}

/***************************************************************************/
bool gattClientHasMaxCentralClients(void)
{
    /* If max_central_conn is 0, then we should not allow any central connection */
    if(GATT_CLIENT_MAX_CONN)
    {
        /* If already all the client connection is used, then don't allow any more central connection.
          * Need to drop one of the client connection to allow any central connection */
        if(!gattCommonHasMaxConnections())
        {
            /* Get the total number of central devices connected */
            return (gattClientGetNumberOfCentralConnected() >= GATT_CLIENT_MAX_CONN);
        }
    }
    return TRUE;
}

/***************************************************************************/
bool gattClientHasClients(void)
{
    return (gattClientGetNumberOfCentralConnected() != 0);
}


/***************************************************************************/
void gattClientRemoveServices(gatt_client_connection_t *client_connection)
{
    gatt_client_services_t *services = gattClientGetServiceData(client_connection);
    uint16 cid = gattClientFindByConnection(client_connection);
    
    UNUSED(cid);
    UNUSED(services);

    /* Remove services if used */
    if (services)
    {
        if (services->basc)
        {
            gattBatteryClientRemoveService(services->basc, cid);
        }
        if(services->ancs)
        {
            sinkGattAncsClientRemoveService(services->ancs);
        }
#ifdef GATT_AMS_CLIENT
        if(services->ams)
        {
            sinkGattAmsClientRemoveService(services->ams);
        }
#endif
        if(services->hidc_instance1)
        {
            sinkGattHIDClientRemoveService(services->hidc_instance1, cid);
        }
        if(services->hidc_instance2)
        {
            sinkGattHIDClientRemoveService(services->hidc_instance2, cid);
        }
        if(services->disc)
        {
            sinkGattDISClientRemoveService(services->disc, cid);
        }
        if(services->iasc)
        {
            sinkGattIasClientRemoveService(services->iasc);
        }
        if (services->gattc)
        {
            sinkGattClientServiceRemove(services->gattc, cid);
        }
        if(services->spc)
        {
            sinkGattSpClientRemoveService(services->spc);
        }
        if(services->hrs)
        {
            sinkGattHrsClientRemoveService(services->hrs);
        }
#ifdef ENABLE_BROADCAST_AUDIO
        if(services->ba)
        {
            sinkGattBAClientRemoveService(services->ba);
        }
#endif
        /* Remove more services here */
    }            
}


/***************************************************************************/
void gattClientServiceChanged(uint16 cid)
{
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    gatt_client_services_t *services = gattClientGetServiceData(connection);
    
    if (connection)
    {
        connection->service_changed = TRUE;
        
        /* If service discovery is in progress then wait for discovery to complete
           before re-discovering services.
           Otherwise re-discover services immediately.
        */
        if (services && (services->discovery == NULL))
        {
            MESSAGE_MAKE(message, BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES_T);
            
            /* Remove all services from client */
            gattClientRemoveServices(connection);
            GATT_CLIENT_FREE_MEM(connection->data);
            
            /* Send message to discover services again */
            message->cid = cid;
            MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES);
            MessageSend(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES, message);
        }
    }
}


/***************************************************************************/
void gattClientRediscoverServices(uint16 cid)
{
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    gatt_client_services_t *services = NULL;
    bool error_flag = TRUE;
    
    if (connection == NULL)
        return;
    
    GATT_CLIENT_SERVICES_ALLOC_MEM(services);
    
    if (services)
    {
        /* Record details of the services */
        connection->data = services;
        
        /* Discover services again for this connection */
        GATT_CLIENT_DISCOVERY_ALLOC_MEM(services->discovery);
            
        if (services->discovery)
        {
            /* Discover primary services on this connection */
            gattDiscoverPrimaryServices(connection);
            error_flag = FALSE;
        }
    }
    
    if (error_flag)
    {
        /* Disconnect on error condition */
        gattClientDisconnect(cid);
    }
    
    connection->service_changed = FALSE;
}


/***************************************************************************/
void gattClientProcessSecurityRequirements(gatt_client_connection_t *connection, gatt_client_services_t *data)
{
   tp_bdaddr current_bd_addr;
   uint16 service_count= 0;
   bool is_security_required = FALSE;
   gatt_client_discovery_t *discover = gattClientGetDiscoveredServices(connection);
   uint16 cid = gattClientFindByConnection(connection);


    /* Here only pairing or encryption in case of central role is taken care and
    in case of peripheral role wait for bonding timeout to occur there by allowing
    for remote central device to pair and if remote device does not pair within the timeout 
    initiate encryption */

    for (service_count = 0 ; service_count < data->number_discovered_services ; service_count++)
    {
        /* Check any services required security */
        if(gattClientIsEncryptionForServiceRequired(discover))
        {
            is_security_required = TRUE;
            break;
        }
        /* Get the next discovered service */
        discover+=1;
    }
     GATT_CLIENT_DEBUG(("gattClientProcessSecurityRequirements: is_security_required = %d\n", is_security_required));
    if(is_security_required)
    {
        if(connection->role == ble_gap_role_central)
        {
            if(VmGetBdAddrtFromCid(cid, &current_bd_addr))
            {
                /* Initiate Encryption request */
                GATT_CLIENT_DEBUG(("Gatt Initiate Encryption request \n"));
                
                ConnectionDmBleSecurityReq(sinkGetBleTask(), 
                                            (const typed_bdaddr *)&current_bd_addr.taddr, 
                                            ble_security_encrypted_bonded,
                                            ble_connection_master_directed
                                            );
            }
        }
        else
        {
            /* In case of already bonded peripheral device we shall not have bond timer to send encryption,
             * therefore send remote_connect_success event which shall trigger encryption.
             * On encryption success/failure we shall initialize client library accordingly */
            sinkBleSlaveConnIndEvent(cid);
        }
    }
    else
    {
        /* We are in peripheral role, and found that the remote client doesn't
         * support any service which require encryption. However the remote client shall be 
         * using our services so we need to update the connection parameter */
        if(connection->role == ble_gap_role_peripheral)
        {
           sinkGattClientUpdateSlaveConenctionParams(cid);
        }
        /* If no security required i.e. discovered service does not require encryption 
            initialize discovered services?
        */
        gattClientInitialiseDiscoveredServices(connection);
    }
}

/****************************************************************************/
uint16 gattClientFindByConnection(gatt_client_connection_t *conn)
{
    uint16 index;
    for(index=0;index < MAX_BLE_CONNECTIONS ;index++)
    {
        if(&GATT_CLIENT.connection == conn)
        {
           return GATT[index].cid;
        }
    }
    return GATT_CLIENT_INVALID_CID;
}

/*******************************************************************************/
bool gattClientIsEncryptionForServiceRequired(const gatt_client_discovery_t *discovered_service)
{
    bool encryption_required = (discovered_service) &&  (
                                                       discovered_service->service == gatt_client_ancs
                                                    || discovered_service->service == gatt_client_ias
                                                    || discovered_service->service == gatt_client_hid
                                                    || discovered_service->service == gatt_client_ba
#ifdef GATT_AMS_CLIENT
                                                    || discovered_service->service == gatt_client_ams
#endif
                                                        );
    return encryption_required;
}


#endif /* GATT_ENABLED */

