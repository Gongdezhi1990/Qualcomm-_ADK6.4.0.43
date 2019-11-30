/*******************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.
 
*******************************************************************************/



#ifndef _SINK_GATT_CLIENT_H_
#define _SINK_GATT_CLIENT_H_

#include "sink_ble_gap.h"
#include "sink_gatt_hid_remote_control.h"

#include <bdaddr.h>
#include <csrtypes.h>

#ifdef GATT_ENABLED
#include <gatt_client.h>
#include <gatt_battery_client.h>
#include <gatt_apple_notification_client.h>
#include <gatt_imm_alert_client.h>
#include <gatt_hid_client.h>
#include <gatt_device_info_client.h>
#include <gatt_scan_params_client.h>
#include <gatt_heart_rate_client.h>
#include <gatt_gap_server.h>
#include <gatt_broadcast_client.h>
#ifdef GATT_AMS_CLIENT
#include <gatt_ams_client.h>
#endif

/* Define for Client variables */
#define GATT_CLIENT gBleData->ble.gatt[index].client

#define GATT_CLIENT_INVALID_CID (0)

/* Number of central configured */
#define GATT_CLIENT_MAX_CONN  sinkBleGapGetMaxCentralConnection()

/*
    Defines the bitfield used to identify which client tasks are used for the device.
*/
typedef enum
{
    gatt_client_start = 1,	/* NOT to be used directly */

    gatt_client_battery = gatt_client_start,
    gatt_client_ancs = 2,
    gatt_client_ias = 3,
    gatt_client_hid = 4,
    gatt_client_dis = 5,
    gatt_client_gatt = 6,
    gatt_client_spc = 7,
    gatt_client_hrs = 8,
    gatt_client_ba = 9,
#ifdef GATT_AMS_CLIENT
    gatt_client_ams = 10,
#endif
    /* Add more client services here not forgetting to 
       add a corresponding element to gatt_clients in sink_gatt_client.c */


     /* not particularly pleasant, but safe */
    gatt_client_terminator,
    gatt_client_num_services = gatt_client_terminator - gatt_client_start

} gatt_client_tasks_t;

#define MAX_ONE_INSTANCE 0x1
#define MAX_TWO_INSTANCE 0x2
#define NUM_DUAL_INSTANCE_GATT_SERVICES     1    														/* hid we allow 2 instance */
#define NUM_SINGLE_INSTANCE_GATT_SERVICES   (gatt_client_num_services - NUM_DUAL_INSTANCE_GATT_SERVICES)/* only 1 for all other services */

/* The maximum supported client service is derived from number of instances each client service
 * which we are interested. Currently we support one instance of BAS, ANCS, IAS, DIS, GATT and SPC
 * and two instance of HID. If number of instances changes then even this macro shall change accrodingly */
#define MAX_GATT_CLIENT_SERVICES   ((MAX_ONE_INSTANCE * NUM_SINGLE_INSTANCE_GATT_SERVICES) \
                                                           + (MAX_TWO_INSTANCE * NUM_DUAL_INSTANCE_GATT_SERVICES))

#define HAS_MAX_SERVICES_DISCOVERED(num_disc_service) (num_disc_service >= MAX_GATT_CLIENT_SERVICES)


/* Cache of remote values */
typedef struct __gatt_cache_t
{
    BITFIELD basc_remote_level:8;
    BITFIELD iasc_remote_alert_level:4;
    BITFIELD iasc_phone_alert_level:4;
} gatt_cache_t;

typedef struct __gatt_client_connection
{
    void *data;                                 /* Connection specific data */
    BITFIELD role:2;                            /* Records the GAP role for the client connection */
    BITFIELD service_changed:1;                 /* Records if the service definitions have changed  */
} gatt_client_connection_t;

/* Discovery data */
typedef struct __gatt_client_discovery
{
    gatt_client_tasks_t service;
    uint16 start_handle;
    uint16 end_handle;
} gatt_client_discovery_t;

/* List of remote services */
typedef struct __gatt_client_services
{    
    gatt_client_discovery_t *discovery;
#ifdef GATT_HID_REMOTE_CONTROL
    gattHidRcDevData_t *remote;
#endif
    BITFIELD number_discovered_services:8;
    BITFIELD current_discovered_service:8;
    GBASC *basc;
    GIASC_T *iasc;
    /* Insert more device service pointers here */
    GANCS *ancs;
#ifdef GATT_AMS_CLIENT
    GAMS *ams;
#endif
    GHIDC_T *hidc_instance1;
    GHIDC_T *hidc_instance2;
    GDISC    *disc;
    GGATTC *gattc;
    GSPC_T *spc;
    GHRSC_T *hrs;
#ifdef ENABLE_BROADCAST_AUDIO
    GBSC      *ba;
#endif
    uint32 size_client_data;
    uint16 client_data[1];
} gatt_client_services_t;

/* Client data */
typedef struct __gatt_client
{
    gatt_client_connection_t connection;
    gatt_cache_t cache;
} gatt_client_t;


/****************************************************************************
NAME    
    gattClientAdd
    
DESCRIPTION
    Stores connection details when a remote server device connects.
    
PARAMETERS
    cid             The connection ID
    client_taddr    Pointer to the connecting LE device
    client_gap_role GAP Role for the client connection
    
RETURNS    
    TRUE if the connection was successfully added, FALSE otherwise.
*/
bool gattClientAdd(uint16 cid, ble_gap_role_t client_gap_role);


/****************************************************************************
NAME    
    gattClientFindByCid
    
DESCRIPTION
    Finds the GATT client connection by connection ID.
    
PARAMETERS
    cid             The connection ID
    
RETURNS    
    The GATT connection instance if found, NULL otherwise.
*/
gatt_client_connection_t *gattClientFindByCid(uint16 cid);


/****************************************************************************
NAME    
    gattClientRemove
    
DESCRIPTION
    Frees resource for a GATT client connection.
    
PARAMETERS
    cid             The connection ID

RETURNS    
    TRUE if the connection was found and removed. FALSE otherwise.
*/
bool gattClientRemove(uint16 cid);


/*******************************************************************************
NAME
    gattClientGetServicePtr
    
DESCRIPTION
    Finds the offset within client_data[] for the particular client GATT service for the specified connection.
    
PARAMETERS
    connection      The GATT client connection instance.
    size_service    The size of the service to be added.
    
RETURNS
    Pointer to the memory that can be used by the new service if successsful.
    Will return NULL if the connection was NULL. 
*/
uint16 *gattClientGetServicePtr(gatt_client_connection_t *connection, uint16 size_service);


/*******************************************************************************
NAME
    gattClientStoreDiscoveredService
    
DESCRIPTION
    Store services discovered during the "Discover All Primary Services" procedure.
    
PARAMETERS
    cid             The connection ID.
    uuid_type    To indicate if UUID is 16/32/64/128 bit
    uuid            Pointer to 16 bit/128 UUID of the discovered service.
    start           The start handle of the service.
    end             The end handle of the service.
    more            Indicates if discovery is still in progress.
    
*/
void gattClientStoreDiscoveredService(uint16 cid, uint16 uuid_type, uint32 *uuid, uint16 start, uint16 end, bool more);


/*******************************************************************************
NAME
    gattClientGetServiceData
    
DESCRIPTION
    Returns the service data for the specified connection.
    
PARAMETERS
    connection      The GATT client connection instance.
    
RETURNS
    Pointer to the service data for the client connection. 
*/
gatt_client_services_t *gattClientGetServiceData(gatt_client_connection_t *connection);


/****************************************************************************
NAME    
    gattClientDiscoveredServiceInitialised
    
DESCRIPTION
    Called when a primary service has been fully initialised.
    The next primary service can then be initialised, or the procedure ends.
    
PARAMETERS
    connection             The GATT connection

*/
void gattClientDiscoveredServiceInitialised(gatt_client_connection_t *connection);

/****************************************************************************
NAME    
    gattClientRemoveDiscoveredService
    
DESCRIPTION
    Called when a discovered service library failed to initialize.
    
PARAMETERS
    connection             The GATT connection
    service                  The client service that needs to be de-removed

 NOTE: DO NOT call this function other than the case where client library init fails. If called in
other situation then the entire client_data shall be corrupted.

*/
bool gattClientRemoveDiscoveredService(gatt_client_connection_t *connection, gatt_client_tasks_t service);

/****************************************************************************
NAME    
    gattClientInitialiseDiscoveredServices
    
DESCRIPTION
    Initialise known primary services after being discovered.
    
PARAMETERS
    connection             The GATT connection

*/
void gattClientInitialiseDiscoveredServices(gatt_client_connection_t *connection);

/****************************************************************************
NAME    
    gattDiscoverPrimaryServices
    
DESCRIPTION
    Discovers GATT primary services supported on the remote device.
    
PARAMETERS
    connection             The GATT connection

*/
void gattDiscoverPrimaryServices(gatt_client_connection_t *connection);

/****************************************************************************
NAME    
    gattClientGetDiscoveredServices
    
DESCRIPTION
    Gets a list of the discovered primary services.
    
PARAMETERS
    connection                  The GATT connection
    number_discovered_services  Set by the function to the current number of discovered services

*/
gatt_client_discovery_t *gattClientGetDiscoveredServices(gatt_client_connection_t *connection);

#endif /* GATT_ENABLED */


/****************************************************************************
NAME    
    gattClientDisconnect
    
DESCRIPTION
    Disconnects the client role connection identified by the CID. 
    
PARAMETERS
    cid                     The connection ID
    
RETURNS    
    None
*/
#ifdef GATT_ENABLED
void gattClientDisconnect(uint16 cid);
#else
#define gattClientDisconnect(cid) ((void)(0))
#endif


/****************************************************************************
NAME    
    gattClientDisconnectAll
    
DESCRIPTION
    Disconnects all client role connections.
    
PARAMETERS
    None
    
RETURNS    
    TRUE if a disconnect request was sent. FALSE otherwise.
*/
#ifdef GATT_ENABLED
bool gattClientDisconnectAll(void);
#else
#define gattClientDisconnectAll() (FALSE)
#endif


/****************************************************************************
NAME    
    gattClientDiscoveryError
    
DESCRIPTION
    Capture discovery errors.
    
PARAMETERS
    connection      The GATT connection

*/
#ifdef GATT_ENABLED
void gattClientDiscoveryError(gatt_client_connection_t *connection);
#else
#define gattClientDiscoveryError(connection) ((void)(0))
#endif


/****************************************************************************
NAME    
    gattClientHasMaxCentralClients
    
DESCRIPTION
    Checks if the maximum central device connections has been reached.
    
PARAMETERS
    None
    
RETURNS
    TRUE if reached the maximum connections. FALSE otherwise.

*/
#ifdef GATT_ENABLED
bool gattClientHasMaxCentralClients(void);
#else
#define gattClientHasMaxCentralClients() (TRUE)
#endif

/****************************************************************************
NAME    
    gattClientHasClients
    
DESCRIPTION
    This function check for number of central connected device return TRUE 
	if found else FALSE.
    
PARAMETERS
    None 
    
RETURNS
    TRUE if any connections. FALSE otherwise.

*/
#ifdef GATT_ENABLED
bool gattClientHasClients(void);
#else
#define gattClientHasClients() (FALSE)
#endif


/****************************************************************************
NAME    
    gattClientRemoveServices
    
DESCRIPTION
    Remove all services associated with the supplied connection.
    
PARAMETERS
    None
    
RETURNS
    None

*/
#ifdef GATT_ENABLED
void gattClientRemoveServices(gatt_client_connection_t *client_connection);
#else
#define gattClientRemoveServices(client_connection) ((void)(0))
#endif


/****************************************************************************
NAME    
    gattClientServiceChanged
    
DESCRIPTION
    Notifies the GATT client that a Service Changed indication has been received.
    
PARAMETERS
    None
    
RETURNS
    None

*/
#ifdef GATT_ENABLED
void gattClientServiceChanged(uint16 cid);
#else
#define gattClientServiceChanged(cid) ((void)(0))
#endif


/****************************************************************************
NAME    
    gattClientRediscoverServices
    
DESCRIPTION
    Called to rediscover all services on the supplied connection.
    
PARAMETERS
    cid  The client connection

*/
#ifdef GATT_ENABLED
void gattClientRediscoverServices(uint16 cid);
#else
#define gattClientRediscoverServices(cid) ((void)(0))
#endif


/****************************************************************************
NAME    
    gattClientProcessSecurityRequirements
    
DESCRIPTION
    Process security requirement for this client connection.
    
PARAMETERS
    connection pointer to this client connection
    data pointer to client services

*/
#ifdef GATT_ENABLED
void gattClientProcessSecurityRequirements(gatt_client_connection_t *connection, gatt_client_services_t *data);
#else
#define gattClientProcessSecurityRequirements(connection, data) ((void)(0))
#endif

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
#ifdef GATT_ENABLED
bool gattClientIsServiceAvailable(gatt_client_connection_t *connection, gatt_client_tasks_t service);
#else
#define gattClientIsServiceAvailable( connection, service) (FALSE)
#endif

/****************************************************************************
NAME    
    gattClientConnection
    
DESCRIPTION
    Return GATT Client Connection
    
PARAMETERS
    index Index of GATT Connection

RETURNS
    Connection pointer to client connection

*/
#ifdef GATT_ENABLED
gatt_client_connection_t *gattClientConnection(uint16 index);
#else
#define gattClientConnection(index) ((void)(0))
#endif

/****************************************************************************
NAME    
    gattClientFindByConnection
    
DESCRIPTION
    Utility function to get the corresponding GATT cid for the provided client connection
    
PARAMETERS
    conn Pointer to the instance of the client connection

RETURNS
    cid of the corresponding LE link

*/
#ifdef GATT_ENABLED
uint16 gattClientFindByConnection(gatt_client_connection_t *conn);
#else
#define gattClientFindByConnection(conn) (INVALID_CID)
#endif

/*******************************************************************************
NAME
    requireEncryptionForService

DESCRIPTION
    Check if encryption is required for a given service

PARAMETERS
    discovered_service

RETURNS
    True if encryption required, False otherwise
*/
#ifdef GATT_ENABLED
bool gattClientIsEncryptionForServiceRequired(const gatt_client_discovery_t *discovered_service);
#else
#define gattClientIsEncryptionForServiceRequired(discovered_service) ((void)(0))
#endif

#endif /* _SINK_GATT_CLIENT_H_ */

