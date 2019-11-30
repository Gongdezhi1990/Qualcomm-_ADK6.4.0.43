/****************************************************************************
Copyright (c) 2014 - 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_init.c        

DESCRIPTION
    Contains GATT initialisation routines.
*/

#include "sink_gatt_db.h"
#include "sink_gatt_init.h"

#include "sink_ble.h"
#include "sink_gatt_server.h"

#include "sink_debug.h"
#include "sink_gaia.h"
#include "sink_gatt_client_battery.h"
#include "sink_gatt_client_hid.h"
#include "sink_gatt_device.h"
#include "sink_gatt_manager.h"
#include "sink_gatt_server.h"
#include "sink_gatt_server_battery.h"
#include "sink_gatt_server_tps.h"
#include "sink_gatt_server_lls.h"
#include "sink_gatt_server_ias.h"
#include "sink_gatt_server_hrs.h"
#ifdef ACTIVITY_MONITORING
#include "sink_gatt_server_rscs.h"
#include "sink_gatt_server_logging.h"
#endif
#include "sink_gatt_server_dis.h"
#include "sink_gatt_server_gap.h"
#include "sink_gatt_server_gatt.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_server_ba.h"
#ifdef ENABLE_FAST_PAIR
#include "sink_gatt_server_fps.h"
#endif

#include <gatt_server.h>
#include <gatt_gap_server.h>
#include <gatt_battery_server.h>
#include <gatt_manager.h>
#include <connection.h>

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

#include <csrtypes.h>


#ifdef GATT_ENABLED


#ifdef DEBUG_GATT
#define GATT_DEBUG(x) DEBUG(x)
#else
#define GATT_DEBUG(x) 
#endif

extern const uint16 gattDatabase[];

/*******************************************************************************
NAME
    calculateOptionalGattServerSize
    
DESCRIPTION
    Calculates the size of memory required to hold the optional GATT servers.
    
PARAMETERS
    None
    
RETURNS
    The size of memory required to hold the optional GATT servers.
*/
static uint16 calculateOptionalGattServerSize(void)
{
    uint16 size = 0;
    
    /* Get size used by battery server */
    size += sinkGattBatteryServerCalculateSize();
    
    /* Calculates size of other optional GATT servers here */
    size += sinkGattLinkLossServerGetSize();       
    size += sinkGattTxPowerServerGetSize();       
    size += sinkGattImmAlertServerGetSize();
    size += sinkGattHeartRateServerGetSize();
#ifdef ACTIVITY_MONITORING
    size += sinkGattRSCServerGetSize();
    size += sinkGattLoggingServerGetSize();
#endif
    size += sinkGattDeviceInfoServerGetSize();
    size += sinkGattBAServerGetSize();
#ifdef ENABLE_FAST_PAIR
    size += sinkGattFastPairServerGetSize();
#endif
#ifdef GATT_AMA_SERVER
    size += sinkGattAmaServerCalculateSize();
#endif
#ifdef GATT_BISTO_COMM_SERVER
    size += sinkGattBistoCommServerCalculateSize();
#endif
#ifdef GATT_AMS_PROXY
    size += sinkGattAmsProxyServerCalculateSize();
#endif
#ifdef GATT_ANCS_PROXY
    size += sinkGattAncsProxyServerCalculateSize();
#endif

    return size;
}


/*******************************************************************************
NAME
    calculateCompleteGattServerSize
    
DESCRIPTION
    Calculates the size of memory required to hold all the GATT servers.
    
PARAMETERS
    None
    
RETURNS
    The size of memory required to hold all the GATT servers.
*/
static uint16 calculateCompleteGattServerSize(void)
{
    uint16 size = 0;
    GATT_DEBUG(("Calculate memory size for server tasks:\n"));
    
    /* GATT Server must always be included */
    size += sinkGattServerGetSize();
    GATT_DEBUG(("+GATT=[%d]\n", size));
    
    /* GAP Server must always be included */
    size += sinkGapServerGetSize();
    GATT_DEBUG(("+GAP=[%d]\n", size));
       
    /* Add any optional tasks that have been requested... */
    size += calculateOptionalGattServerSize();
    GATT_DEBUG(("+Optional=[%d]\n", size));
    
    return size;
}


/*******************************************************************************
NAME
    initialiseOptionalServerTasks
    
DESCRIPTION
    Initialise each of the optional server tasks that have been configured.
    
PARAMETERS
    ptr     pointer to allocated memory to store server tasks rundata.
    
RETURNS
    TRUE if all requested server tasks were initialised, FALSE otherwise.
*/
static bool initialiseOptionalServerTasks(uint16 **ptr)
{
    if (ptr)
    {
        /* DON'T change the sequence of BA server initialization. 
            Else it won't be compactible with older BA receivers */
        if(!sinkGattBAServerInitialise(ptr))
        {
            return FALSE;
        }
        
        if (!sinkGattBatteryServerInitialise(ptr))
        {
            return FALSE;
        }

        /* Initialize the Link Loss server. */
        if(!sinkGattLinkLossServerInitialiseTask(ptr))
        {
            return FALSE;
        }

        /* Initialize the Tx Power server */
        if(!sinkGattTxPowerServerInitialiseTask(ptr))
        {
            return FALSE;
        }

        /* Initialize the Immediate Alert server */
        if (!sinkGattImmAlertServerInitialise(ptr))
        {
            return FALSE;
        }

        /* Initialize the Heart Rate server */
        if (!sinkGattHeartRateServerInitialise(ptr))
        {
            return FALSE;
        }

#ifdef ACTIVITY_MONITORING
        /* Initialize the Running Speed and Cadence server */
        if (!sinkGattRSCServerInitialise(ptr))
        {
            return FALSE;
        }
        /* Initialize the Logging server */
        if (!sinkGattLoggingServerInitialise(ptr))
        {
            return FALSE;
        }
#endif

        /* Initialize the Device Information server */
        if(!sinkGattDeviceInfoServerInitialise(ptr))
        {
            return FALSE;
        }
#ifdef ENABLE_FAST_PAIR
        /* Initialize the Fast Pair Server */
        if(!sinkGattFastPairServerInitialise(ptr))
        {
            return FALSE;
        }
#endif

#ifdef GATT_AMA_SERVER
        /* Initialize the Device Information server */
        if(!sinkGattAmaServerInitialise(ptr))
        {
            return FALSE;
        }
#endif
#ifdef GATT_BISTO_COMM_SERVER
        /* Initialize the Device Information server */
        if(!sinkGattBistoCommServerInitialise(ptr))
        {
            return FALSE;
        }
#endif
#ifdef GATT_AMS_PROXY
        /* Initialize the Apple Proxy server */
        if(!sinkGattAmsProxyServerInitialise(ptr))
        {
            return FALSE;
        }
#endif
#ifdef GATT_ANCS_PROXY
        /* Initialize the Apple Proxy server */
        if(!sinkGattAncsProxyServerInitialise(ptr))
        {
            return FALSE;
        }
#endif
#ifdef GATT_BMS
        /* Initialize the Bisto Media Service (BMS) server */
        if(!sinkGattBmsServerInitialise())
        {
            return FALSE;
        }
#endif
        /* Initialise GAIA server */
        gaiaGattServerInitialise();
        
        /* Initialise other optional GATT Servers here */
        
        /* All requested server tasks were registered successfully */
        return TRUE;
    }
    
    return FALSE;
}

/*******************************************************************************
NAME
    initialiseGattServers
    
DESCRIPTION
    Prepare the GATT Manager for server registration, then Initialise each of
    the GATT Server tasks that have been requested and finally register the
    GATT database for the device.
    
PARAMETERS
    None
    
RETURNS
    TRUE if the initialisation was successful, FALSE otherwise.
*/
static bool initialiseGattServers(void)
{
    uint16 index = 0;
    uint16 server_malloc_size = calculateCompleteGattServerSize();
    
    GATT_SERVER.servers_ptr = calloc(1, server_malloc_size);

    if (GATT_SERVER.servers_ptr)
    {
        uint16 *ptr = (uint16*)GATT_SERVER.servers_ptr;
        /* GATT Service MUST always be included as per the Bluetooth spec */
        if (sinkGattServerInitialiseTask(&ptr))
        {
            /* GAP Server must always be included as per the Bluetooth spec */
            if (sinkGattGapServerInitialiseTask(&ptr))
            {                
                /* Initialise Optional services */
                if (initialiseOptionalServerTasks(&ptr))
                {
                    /* All server tasks have been initialised */
                    return TRUE;
                }
            }
        }
        
        /* Free allocated memory used to store rundata for the GATT Server libraries */
        free(GATT_SERVER.servers_ptr);
    }
    
    return FALSE;
}

static void registerGattSdpRecords(void)
{
    uint8 *sdp;
    uint16 size_sdp;
    uint16 i;

    for (i = 0; i < gatt_sdp_last; i++)
    {
        sdp = GattGetServiceRecord(i, &size_sdp);
        ConnectionRegisterServiceRecord(sinkGetBleTask(), size_sdp, sdp);
    }
}


/*******************************************************************************
NAME
    initialiseGattWithServers
    
DESCRIPTION
    Function to initialise GATT for the device when server roles have been
    requested.
    
PARAMETERS
    None
    
RETURNS
    TRUE if the initialisation request was successful, FALSE otherwise.
*/
static bool initialiseGattWithServers(void)
{
    GATT_DEBUG(("Initialise GATT Manager - with servers\n"));
    if (GattManagerInit(sinkGetBleTask()))
    {
        if (GattManagerRegisterConstDB(&gattDatabase[0], GattGetDatabaseSize()/sizeof(uint16)))
        {
            if (initialiseGattServers())
            {
                registerGattSdpRecords();
                GattManagerRegisterWithGatt();
                
                return TRUE;
            }
        }
    }
        
    return FALSE;
}


/*******************************************************************************
NAME
    initialiseGattClient
    
DESCRIPTION
    Function to initialise GATT for client role.
    
PARAMETERS
    None
    
RETURNS
    None
*/
static void initialiseGattClient(void)
{
    /* Init of client services */
    gattBatteryClientInit();
    sinkGattIasClientInit();
    sinkGattHidClientInit();
}


/******************************************************************************/
bool sinkGattInitInitialiseDevice(void)
{
    /*Init sink GATT database*/
    memset(&GATT, 0, sizeof(gatt_data_t));
    
    /* Client role init */
    initialiseGattClient();
            
    /* Server role init */   
    return initialiseGattWithServers();
}
#endif /* GATT_ENABLED */

