/****************************************************************************
Copyright (c) 2014 - 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_ble.c

DESCRIPTION
    BLE functionality
*/

#include "sink_ble.h"

#include "sink_ble_advertising.h"
#include "sink_ble_scanning.h"
#include "sink_debug.h"
#include "sink_development.h"
#include "sink_statemanager.h"
#include "sink_powermanager.h"
#include "sink_devicemanager.h"
#include "sink_ba.h"
#include "sink_ba_broadcaster_association.h"
#include "sink_ba_ble_gap.h"

#ifdef ENABLE_BROADCAST_AUDIO
#include "sink_a2dp.h"
#include <hfp.h>
#endif

#include <connection.h>
#include <vm.h>
#include <string.h>
#include <stdlib.h>

#ifdef GATT_ENABLED

#include <gatt.h>
#include <gatt_battery_client.h>
#include <gatt_battery_server.h>
#include <gatt_gap_server.h>
#include <gatt_manager.h>
#include <gatt_server.h>

#include "sink_gatt_common.h"
#include "sink_gatt.h"
#include "sink_gatt_client_battery.h"
#include "sink_gatt_client_gatt.h"
#include "sink_gatt_client_hid.h"
#include "sink_gatt_client_dis.h"
#include "sink_gatt_init.h"
#include "sink_gatt_manager.h"
#include "sink_gatt_server_battery.h"
#include "sink_gatt_server_ias.h"
#include "sink_gatt_client_ias.h"
#include "sink_gatt_server_lls.h"
#include "sink_gatt_server_hrs.h"
#include "sink_gatt_server_fps.h"


#include "sink_main_task.h"
#include "sink_sport_health_sm.h"

#include "sink_gatt_server_rscs.h"
#include "sink_gatt_server_logging.h"

#ifdef GATT_AMS_CLIENT
#include "sink_gatt_client_ams.h"
#endif
#include "sink_gatt_client_ancs.h"
#include "sink_gatt_client_spc.h"
#include "sink_gatt_client_hrs.h"
#include "sink_gatt_server_gap.h"
#include "sink_gatt_server_gatt.h"
#include "sink_gatt_client_ba.h"
#include "sink_gaia.h"

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

#ifdef DEBUG_BLE
#define BLE_INFO(x) DEBUG(x)
#define BLE_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define BLE_INFO(x)
#define BLE_ERROR(x)
#endif
/* single instance of ble global data */
sink_ble_global_data_t *gBleData;

#define DEVICE_IS_ON                        (stateManagerGetState() != deviceLimbo)
#define ENCRYPTION_RETRY_EVENT_DELAY_MS     500
#define LEGACY_SLOW_ADV_INTERVAL_MIN  1000
#define LEGACY_SLOW_ADV_INTERVAL_MAX 1200

/* The below values are considered from  ble_master_update_conn_params and 
     ble_slave_update_conn_params. */

#ifdef GATT_BISTO_COMM_SERVER
#define SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN				12		/* Slave minimum connection interval */
#define SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX             24		/* Slave maximum connection interval */
#else
#define SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN				72		/* Slave minimum connection interval */
#define SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX				88		/* Maximum connection interval */
#endif

#define SINK_BLE_CONN_PARAM_SUPERVISION_TIMEOUT					400		/* Supervision timeout */
#define SINK_BLE_CONN_PARAM_CE_LENGTH_MIN						0		/* Minimum length of the connection */
#define SINK_BLE_CONN_PARAM_CE_LENGTH_MAX						160		/* Maximum length of the connection */
#define SINK_BLE_CONN_PARAM_MASTER_CONN_INTERVAL_MIN			80		/* Master minimum connection interval */
#define SINK_BLE_CONN_PARAM_MASTER_CONN_INTERVAL_MAX			88		/* Master maximum connection interval */
#define SINK_BLE_CONN_PARAM_MASTER_CONN_LATENCY					8		/* Master connection latency */
#define SINK_BLE_CONN_PARAM_SLAVE_CONN_LATENCY					4		/* Slave connection latency */
#define SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_INTERVAL_MIN		24      /* Initial Minimum connection interval */
#define SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_INTERVAL_MAX		40    	/* Initial maximum connection interval */
#define SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_LATENCY        	0      	/* Initial connection latency */
#define SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_ATTEMPT_TIMEOUT	50   	/* Initial connection attempt timeout */
#define SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_LATENCY_MAX        64    	/* Maximum connection latency */
#define SINK_BLE_CONN_PARAM_MASTER_SUPERVISION_TIMEOUT_MIN      400   	/* Minimum supervision timeout */
#define SINK_BLE_CONN_PARAM_MASTER_SUPERVISION_TIMEOUT_MAX     	400   	/* Maximum supervision timeout */

/* Default BLE configuration */
static const ble_configuration_t ble_config = {

                                10,     /* Pairing Connection timeout (secs) */
                                60,     /* Bondable Timeout (secs) */

                                128,    /* Fast scan interval */
                                12,     /* Fast scan window */
                                120,    /* Gap mode switch timer */
                                10,     /* Time to scan for whitelist devices before reverting to general scanning, 
                                            if a private device has been paired with */
                                30,     /* Fast scan timer */
                                2048,   /* Slow scan interval */
                                18,     /* Slow scan window */
                                32,     /* Fast adv interval min */
                                48,     /* Fast adv interval max */
                                30,     /* Fast adv timer */
#ifdef GATT_AMA_SERVER
                                100,   /* AMA TODO  Slow adv interval min */
                                120,   /* AMA TODO Slow adv interval max */
#elif defined ENABLE_FAST_PAIR
                                200, /*Fast Pair silent pairing adv interval min */
                                250, /*Fast Pair silent pairing adv interval max */
#else
                                1000,   /* Slow adv interval min */
                                1200,   /* Slow adv interval max */
#endif /* GATT_AMA_SERVER */
#ifdef ENABLE_FAST_PAIR
                                 800   /* Fast Pair adv timer (secs), Fastpair spec recommends less than 15 minutes */
#endif                                
};


/* Default connection parameters when Master and initiating connection (parameters to allow quick connection/discovery of database) */
static const ble_connection_initial_parameters_t ble_master_initial_conn_params = {
    SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_INTERVAL_MIN,		/* Minimum connection interval */ 
    SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_INTERVAL_MAX,      /* Maximum connection interval */
    SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_LATENCY,           /* Connection latency */
    SINK_BLE_CONN_PARAM_SUPERVISION_TIMEOUT,                /* Supervision timeout */
    SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_ATTEMPT_TIMEOUT,   /* Connection attempt timeout */
    SINK_BLE_CONN_PARAM_MASTER_INIT_CONN_LATENCY_MAX,       /* Connection latency max */
    SINK_BLE_CONN_PARAM_MASTER_SUPERVISION_TIMEOUT_MIN,     /* Minimum supervision timeout */
    SINK_BLE_CONN_PARAM_MASTER_SUPERVISION_TIMEOUT_MAX      /* Maximum supervision timeout */
};


/* Default connection parameters when Master and the connection has been initiated (initial discovery has been performed,
   can relax parameters) */
static const ble_connection_update_parameters_t ble_master_update_conn_params = {
    SINK_BLE_CONN_PARAM_MASTER_CONN_INTERVAL_MIN,	/* Minimum connection interval */
    SINK_BLE_CONN_PARAM_MASTER_CONN_INTERVAL_MAX,   /* Maximum connection interval */
    SINK_BLE_CONN_PARAM_MASTER_CONN_LATENCY,        /* Connection latency */
    SINK_BLE_CONN_PARAM_SUPERVISION_TIMEOUT,        /* Supervision timeout */
    SINK_BLE_CONN_PARAM_CE_LENGTH_MIN,              /* Minimum length of the connection */
    SINK_BLE_CONN_PARAM_CE_LENGTH_MAX               /* Maximum length of the connection */
};


/* Default connection update parameters when Slave and the connection has been established */
static const ble_connection_update_parameters_t ble_slave_update_conn_params = {
    SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN,	/* Minimum connection interval */
    SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX,    /* Maximum connection interval */
    SINK_BLE_CONN_PARAM_SLAVE_CONN_LATENCY,         /* Connection latency */
    SINK_BLE_CONN_PARAM_SUPERVISION_TIMEOUT,        /* supervision timeout */
    SINK_BLE_CONN_PARAM_CE_LENGTH_MIN,              /* Minimum length of the connection */
    SINK_BLE_CONN_PARAM_CE_LENGTH_MAX               /* Maximum length of the connection */
};

/*Retry for Conn Parameter Update for Upgrade until CI is max 60ms (48*1.25) */
#define SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MAX 48

/*******************************************************************************
NAME
    handleInitComplete
    
DESCRIPTION
    Handle when BLE_INTERNAL_MESSAGE_INIT_COMPLETE message was received
    
PARAMETERS
    None
    
RETURNS
    None
*/
static void handleInitComplete(void)
{
 #ifdef ENABLE_RANDOM_ADDR
    ConnectionDmBleConfigureLocalAddressReq(ble_local_addr_generate_resolvable, NULL);
#endif
    /* After initialisation decide if scanning/advertising needs to start */
    if ((!sinkBroadcastAudioIsActive() && sinkBleIsActiveOnPowerOff()) || DEVICE_IS_ON)
    { 
        /* Power on BLE */
        sinkBlePowerOnEvent();     
    }
}

/*******************************************************************************
NAME
    handleEncryptionRetry
    
DESCRIPTION
    Handle when BLE_INTERNAL_MESSAGE_ENCRYPTION_RETRY_TIMER message was received
    
PARAMETERS
    payload The message payload
    
RETURNS
    None
*/
static void handleEncryptionRetry(Message message)
{
    ble_gap_event_t event;
    ble_gap_event_args_t args;
    uint16 cid = *(uint16 *) message;
    tp_bdaddr tpaddrt;
    bool active_conn = FALSE;

    memset(&tpaddrt, 0, sizeof(tpaddrt));
    
    /* Send GAP event to retry encryption */
    if (cid != 0)
        active_conn = VmGetBdAddrtFromCid(cid, &tpaddrt);
    
    if (active_conn)
    {
        event.id = ble_gap_event_retry_encryption;
        args.encryption_retry.taddr = tpaddrt.taddr;
        event.args = &args;
        sinkBleGapEvent(event);
    }
}

/*******************************************************************************
NAME
    ConnectionDmBleConnectionParameterUpdateCfm
    
DESCRIPTION
    Handle received CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM as a result of BLE Connection Parameter procedure.
    
PARAMETERS
    CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM_T
    
RETURNS
    None.
*/
static void ConnectionDmBleConnectionParameterUpdateCfm(const CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM_T* cfm)
{
    BLE_INFO(("CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM [0x%x]\n", cfm->status));
    sinkBleHandleConnParamFailures(cfm->status);
}


/*******************************************************************************
NAME
    ConnectionDmBleConnectionUpdateCompleteInd
    
DESCRIPTION
    Handle received CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND as a result of BLE Connection Parameter procedure.
    
PARAMETERS
    CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T
    
RETURNS
    None.
*/
void ConnectionDmBleConnectionUpdateCompleteInd(const CL_DM_BLE_CONNECTION_UPDATE_COMPLETE_IND_T* ind)
{
    BLE_INFO(("CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_IND [0x%x]\n", ind->status));
    sinkBleHandleConnParamFailures(ind->status);
}


/*******************************************************************************
NAME
    sinkBleHandleCLMessage
    
DESCRIPTION
    Connection library messages that are sent to the BLE message handler.
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    payload The message payload
    
RETURNS
    None
*/
static void sinkBleHandleCLMessage(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch (id)
    {
        case CL_DM_LOCAL_NAME_COMPLETE:
        {
            sinkBleGapReadLocalNameComplete((CL_DM_LOCAL_NAME_COMPLETE_T*)message);
        }
        break;
        case CL_DM_BLE_SECURITY_CFM:
        {
            sinkBleGapSetSecurityCfm((CL_DM_BLE_SECURITY_CFM_T*)message);
        }
        break;
        case CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM:
        {
             ConnectionDmBleConnectionParameterUpdateCfm((const CL_DM_BLE_CONNECTION_PARAMETERS_UPDATE_CFM_T*)message);
        }
        break;
        default:
        {
            BLE_ERROR(("Unhandled BLE connection msg [0x%x]\n", id));
        }
        break;
    }
}


/*******************************************************************************
NAME
    bleInternalMsgHandler
    
DESCRIPTION
    Internal BLE messages that are sent to this message handler.
    
PARAMETERS
    task    The task the message is delivered
    id      The ID for the GATT message
    payload The message payload
    
RETURNS
    None
*/
static void bleInternalMsgHandler(Task task, MessageId id, Message message)
{
    ble_gap_event_t event;
    UNUSED(task);
    
    switch (id)
    {
        case BLE_INTERNAL_MESSAGE_INIT_COMPLETE:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_INIT_COMPLETE\n"));
            
            handleInitComplete();
        }
        break;
        case BLE_INTERNAL_MESSAGE_EVENT_NO_CONNECTIONS:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_EVENT_NO_CONNECTIONS\n"));
        
            /* Send GAP event to indicate there are no BLE connections */
            event.id = ble_gap_event_no_connections;
            event.args = NULL;
            sinkBleGapEvent(event);
        }
        break;
        case BLE_INTERNAL_MESSAGE_WHITELIST_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_WHITELIST_TIMER\n"));
            
            /* Send GAP event to indicate whitelist timer */
            event.id = ble_gap_event_whitelist_timeout;
            event.args = NULL;
            sinkBleGapEvent(event);
        }
        break;
        case BLE_INTERNAL_MESSAGE_FAST_SCAN_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_FAST_SCAN_TIMER\n"));
            
            /* Send GAP event to indicate the fast scan has timed out */
            event.id = ble_gap_event_fast_scan_timeout;
            event.args = NULL;
            sinkBleGapEvent(event);
        }
        break;
        case BLE_INTERNAL_MESSAGE_FAST_ADV_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_FAST_ADV_TIMER\n"));
            
            /* Send GAP event to indicate the fast scan has timed out */
            event.id = ble_gap_event_fast_adv_timeout;
            event.args = NULL;
            sinkBleGapEvent(event);
        }
        break;
        case BLE_INTERNAL_MESSAGE_ENCRYPTION_RETRY_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_ENCRYPTION_RETRY_TIMER\n"));
            handleEncryptionRetry(message);
        }
        break;
        case BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES\n"));
            gattClientRediscoverServices(((BLE_INTERNAL_MESSAGE_REDISCOVER_SERVER_SERVICES_T *)message)->cid);
        }
        break;
        case BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED\n"));
            gattClientDiscoveredServiceInitialised(gattClientFindByCid(((BLE_INTERNAL_MESSAGE_GATT_CLIENT_SERVICE_INITIALISED_T *)message)->cid));
        }
        break;
        case BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_BATTERY_READ_TIMER\n"));
            sinkBleBatteryLevelReadSendAndRepeat();
        }
        break;
        case BLE_INTERNAL_MESSAGE_HR_READ_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_HR_READ_TIMER\n"));
            sinkBleNotifyHrMeasurements();
        }
        break;
#ifdef ACTIVITY_MONITORING
        case BLE_INTERNAL_MESSAGE_RSC_READ_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_RSC_READ_TIMER\n"));
            sinkBleNotifyRSCMeasurements();
        }
        break;
#endif

#ifdef ENABLE_FAST_PAIR
        case BLE_INTERNAL_MESSAGE_FAST_PAIR_ADV_TIMER:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_FAST_PAIR_ADV_TIMER"));
                
            /* Send GAP event to indicate the fast pair advertisement has timed out */
            event.id = ble_gap_event_fast_pair_adv_timeout;
            event.args = NULL;
            sinkBleGapEvent(event);
        }
        break;
#endif
        case BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR\n"));
            gattClientDiscoveryError(((BLE_INTERNAL_MESSAGE_GATT_CLIENT_DISCOVERY_ERROR_T *)message)->connection);
        }
        break;
        case BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_ASSOCIATION_SEARCH_TIMEOUT\n"));
            
            /* Send GAP event to indicate the association search has timed out */
            event.id = ble_gap_event_association_timeout;
            event.args = NULL;
            sinkBleGapEvent(event);
        }
        break;
        case BLE_INTERNAL_MESSAGE_START_ADVERTISING_ON_ASSOCIATION_END:
        {
            BLE_INFO(("BLE_INTERNAL_MESSAGE_START_ADVERTISING_ON_ASSOCIATION_END\n"));
            /* Send GAP event to start standalone connectable adverts */
            sinkBleStartBroadcastEvent();
        }
        break;
        default:
        {
            BLE_ERROR(("Unhandled BLE internal msg [0x%x]\n", id));
        }
        break;
    }
}


/******************************************************************************/
void sinkBleInitialiseDevice(void)
{
    BLE_INFO(("Initialise BLE...\n"));

    gBleData = PanicUnlessMalloc(sizeof(sink_ble_global_data_t));
    memset(gBleData, 0, sizeof(sink_ble_global_data_t));

    /* Setup BLE Message handler */
    memset(&BLE_DATA, 0, sizeof(ble_data_t));
    BLE_DATA.task.handler = sinkBleMsgHandler;

    /* Setup whitelist from Paired Device List on initialisation */
    ConnectionDmBleAddTdlDevicesToWhiteListReq(TRUE);
    
    /* Initialise GATT */
    if (!sinkGattInitInitialiseDevice())
    {
        FATAL_ERROR(("GATT failed to initialise!\n"));
    }
    
    /* Initialise GAP */
    sinkBleGapInitialise();
    /* Add the scan filters */
    bleAddScanFilters();
}

/******************************************************************************/
void sinkBleMsgHandler(Task task, MessageId id, Message message)
{
    if ( (id >= CL_MESSAGE_BASE) && (id < CL_MESSAGE_TOP) )
    {
        sinkBleHandleCLMessage(task, id,  message); 
    }
    else if ( (id >= GATT_MESSAGE_BASE) && (id < GATT_MESSAGE_TOP))
    {
        sinkGattMsgHandler(task, id, message);
    }
    else if ((id >= GATT_MANAGER_MESSAGE_BASE) && (id < GATT_MANAGER_MESSAGE_TOP))
    {
        sinkGattManagerMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_SERVER_MESSAGE_BASE) && (id < GATT_SERVER_MESSAGE_TOP))
    {
        sinkGattServerMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_GAP_SERVER_MESSAGE_BASE) && (id < GATT_GAP_SERVER_MESSAGE_TOP))
    {
        if (sinkGattGapServerMsgHandler(task, id, message) == gap_msg_read_name_required)
        {
            sinkBleGapStartReadLocalName(ble_gap_read_name_gap_server);
        }
    }
    else if ( (id >= GATT_BATTERY_SERVER_MESSAGE_BASE) && (id < GATT_BATTERY_SERVER_MESSAGE_TOP))
    {
        sinkGattBatteryServerMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_BATTERY_CLIENT_MESSAGE_BASE) && (id < GATT_BATTERY_CLIENT_MESSAGE_TOP))
    {
        gattBatteryClientMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_IMM_ALERT_SERVER_MESSAGE_BASE) && (id < GATT_IMM_ALERT_SERVER_MESSAGE_TOP))
    {
        sinkGattImmAlertServerMsgHandler(task, id, message);
    }
    else if( (id >= GATT_LINK_LOSS_SERVER_MESSAGE_BASE) && (id < GATT_LLS_ALERT_SERVER_MESSAGE_TOP))
    {
        sinkGattLinkLossServerMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_IMM_ALERT_CLIENT_MESSAGE_BASE) && (id < GATT_IMM_ALERT_CLIENT_MESSAGE_TOP))
    {
        sinkGattIasClientMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_HR_SERVER_MESSAGE_BASE) && (id < GATT_HR_SERVER_MESSAGE_TOP))
    {
        sinkGattHeartRateServerMsgHandler(task, id, message);
    }
#ifdef ACTIVITY_MONITORING
    else if ( (id >= GATT_RSC_SERVER_MESSAGE_BASE) && (id < GATT_RSC_SERVER_MESSAGE_TOP))
    {
        sinkGattRSCServerMsgHandler(task, id, message);
    }
    else if ( (id >= GATT_LOGGING_SERVER_MESSAGE_BASE) && (id < GATT_LOGGING_SERVER_MESSAGE_TOP))
    {
        sinkGattLoggingServerMsgHandler(task, id, message);
    }
#endif
#ifdef ENABLE_FAST_PAIR
    else if( (id >= GATT_FAST_PAIR_SERVER_MESSAGE_BASE) && (id < GATT_FAST_PAIR_SERVER_MESSAGE_TOP))
    {
        sinkGattFastPairServerMsgHandler(task, id, message);
    }
#endif
    else if( (id >= GATT_ANCS_MESSAGE_BASE) && (id < GATT_ANCS_MESSAGE_TOP))
    {
        sinkGattAncsClientMsgHandler(task, id, message);
    }    
    else if((id >= GATT_HID_CLIENT_MESSAGE_BASE) && (id < GATT_HID_CLIENT_MESSAGE_TOP))
    {
        sinkGattHidClientMsgHandler(task, id, message);
    }
    else if((id >= GATT_DEVICE_INFO_CLIENT_MESSAGE_BASE) && (id < GATT_DEVICE_INFO_CLIENT_MESSAGE_TOP))
    {
        sinkGattDisClientMsgHandler(task, id, message);
    }
    else if((id >= GATT_SCAN_PARAMS_CLIENT_MESSAGE_BASE) && (id < GATT_SCAN_PARAMS_CLIENT_MESSAGE_TOP))
    {
        sinkGattSpClientMsgHandler(task, id, message);
    }
    else if((id >= GATT_HEART_RATE_CLIENT_MESSAGE_BASE) && (id<= GATT_HEART_RATE_CLIENT_MESSAGE_TOP))
    {
        sinkGattHrsClientMsgHandler(task, id, message);
    }
    else if((id >= GATT_CLIENT_MESSAGE_BASE) && (id < GATT_CLIENT_MESSAGE_TOP))
    {
        sinkGattClientServiceMsgHandler(task, id, message);
    }
    else if((id >= GATT_BROADCAST_CLIENT_MESSAGE_BASE) && ( id <= GATT_BROADCAST_CLIENT_INIT_CFM))
    {
        sinkGattBAClientMsgHandler(task, id, message);
    }
    else if ( (id >= BLE_INTERNAL_MESSAGE_BASE) && (id < BLE_INTERNAL_MESSAGE_TOP))
    {
        bleInternalMsgHandler(task, id, message);
    }
#ifdef GATT_AMA_SERVER
    else if ( (id >= GATT_AMA_SERVER_MESSAGE_BASE) && (id < GATT_AMA_SERVER_MESSAGE_TOP))
    {
        sinkGattAmaServerMsgHandler(task, id, message);
    }
#endif
#ifdef GATT_BISTO_COMM_SERVER
    else if ( (id >= GATT_BISTO_COMM_SERVER_MESSAGE_BASE) && (id < GATT_BISTO_COMM_SERVER_MESSAGE_TOP))
    {
        sinkGattBistoCommServerMsgHandler(task, id, message);
    }
#endif
#ifdef GATT_AMS_PROXY
    else if ( (id >= GATT_AMS_PROXY_SERVER_MESSAGE_BASE) && (id < GATT_AMS_PROXY_SERVER_MESSAGE_TOP))
    {
        sinkGattAmsProxyServerMsgHandler(task, id, message);
    }
#endif
#ifdef GATT_AMS_CLIENT
    else if( (id >= GATT_AMS_CLIENT_MESSAGE_BASE) && (id < GATT_AMS_CLIENT_MESSAGE_TOP))
    {
        sinkGattAmsClientMsgHandler(task, id, message);
    }    
#endif
#ifdef GATT_ANCS_PROXY
    else if( (id >= GATT_ANCS_PROXY_SERVER_MESSAGE_BASE) && (id < GATT_ANCS_PROXY_SERVER_MESSAGE_TOP))
    {
        sinkGattAncsProxyMsgHandler(task, id, message);
    }
#endif
    else
    {
        BLE_ERROR(("Unhandled BLE msg [0x%x]\n", id));
    }
}

/******************************************************************************/
void sinkBleBondableEvent(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event to become bondable */
    event.id = ble_gap_event_bondable;
    event.args = NULL;

    /* send only if bondable event is allowed */
    if(sinkBleGapAllowBonding())
        sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleAssociationEvent(void)
{
    ble_gap_event_t event;

    /* Allow this event only if broadcast mode is active */
    if(!sinkBroadcastAudioIsActive())
        return;
    
    /* Send GAP event to start association */
    event.id = ble_gap_event_start_association;
    event.args = NULL;
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleCancelAssociationEvent(void)
{
    ble_gap_event_t event;

    /* Allow this event only if broadcast mode is active */
    if(!sinkBroadcastAudioIsActive())
        return;

    /* Send GAP event to cancel association */
    event.id = ble_gap_event_cancel_association;
    event.args = NULL;
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleStartBroadcastEvent(void)
{
    ble_gap_event_t event;

    /* Allow this event only if broadcast mode is active */
    if(!sinkBroadcastAudioIsActive())
        return;
    
    /* Send GAP event to start broadcast */
    event.id = ble_gap_event_start_broadcast;
    event.args = NULL;
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleStopBroadcastEvent(void)
{
    ble_gap_event_t event;

    /* This event can come even when there is mode switch to normal */
    
    /* Send GAP event to start broadcast */
    event.id = ble_gap_event_stop_broadcast;
    event.args = NULL;
    sinkBleGapEvent(event);
}


/******************************************************************************/
void sinkBleConnectionPairingTimeoutEvent(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event to exit connection */
    event.id = ble_gap_event_pairing_timeout;
    event.args = NULL;
    
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleBondableTimeoutEvent(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event to exit bondable connection mode */
    event.id = ble_gap_event_bondable_timeout;
    event.args = NULL;
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleMasterConnCompleteEvent(uint16 cid)
{
    gatt_client_connection_t *connection = NULL;
    tp_bdaddr *tpaddrt = NULL;
    ble_gap_event_t event;
    ble_gap_event_args_t args;
    bool active_conn = FALSE;

    connection = gattClientFindByCid(cid);
    
    if (connection != NULL)
    {
        tpaddrt = PanicUnlessMalloc(sizeof(tp_bdaddr));
        active_conn = VmGetBdAddrtFromCid(cid, tpaddrt);
    }
    
    /* Send GAP event when central connection has complete, which can be used to restart scanning */
    event.id = ble_gap_event_central_conn_complete;
    if (!active_conn)
    {
        event.args = NULL;
    }
    else
    {
        args.master_conn_complete.taddr = (*tpaddrt).taddr;
        event.args = &args;
    }

    if(tpaddrt)
    {
        free(tpaddrt);
    }
    
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleDisconnectionEvent(uint16 cid)
{
    ble_gap_event_t event;
    ble_gap_event_args_t args;

    sinkSportHealthSMDeviceSetConnectionStateBle(FALSE);
    /* Send GAP event for disconnection linked to central role */
    event.id = ble_gap_event_disconn_ind;
    args.connection_id= cid;
    event.args = &args;
    sinkBleGapEvent(event);
}


/******************************************************************************/
void sinkBleCancelAdvertisingEvent(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event when advertising is cancelled */
    event.id = ble_gap_event_cancelled_advertising;
    event.args = NULL;
    sinkBleGapEvent(event);
}


/******************************************************************************/
void sinkBleSlaveConnIndEvent(uint16 cid)
{    
    ble_gap_event_t event;
    ble_gap_event_args_t args;
    sinkSportHealthSMDeviceSetConnectionStateBle(TRUE);

    /* Remove no connections message from queue, it no longer applies */
    MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_EVENT_NO_CONNECTIONS);
        
    /* Send GAP event when remote connection succeeded */
    event.id = ble_gap_event_peripheral_conn_ind;
    args.connection_id= cid;
    event.args = &args;
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleCheckNoConnectionsEvent(void)
{
    if (!gattClientHasClients() && gattServerIsFullyDisconnected())
    {
        /* If no connections exist, send an event to report this.
           so that the application can start advertising/scanning. */
        MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_EVENT_NO_CONNECTIONS);
        MessageSend(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_EVENT_NO_CONNECTIONS, NULL);
    }
}

/******************************************************************************/
void sinkBleRetryEncryptionEvent(uint16 cid)
{
    /* If there is pairing in progress then encryption request needs to be retried as Bluestack returns host busy status 
    */
    uint16 *message = PanicUnlessNew(uint16);
    MessageCancelFirst(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ENCRYPTION_RETRY_TIMER);
    *message = cid;
    MessageSendLater(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_ENCRYPTION_RETRY_TIMER, message, ENCRYPTION_RETRY_EVENT_DELAY_MS);
}

/******************************************************************************/
void sinkBleEncryptionCompleteEvent(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event to power on */
    event.id = ble_gap_event_encyption_complete;
    event.args = NULL;
    sinkBleGapEvent(event);    
}

/******************************************************************************/
void sinkBlePowerOnEvent(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event to power on */
    event.id = ble_gap_event_power_on;
    event.args = NULL;
    sinkBleGapEvent(event);    
}


/******************************************************************************/
void sinkBlePowerOffEvent(void)
{
    ble_gap_event_t event;

    /* Send GAP event to power off */
    event.id = ble_gap_event_power_off;
    event.args = NULL;
    sinkBleGapEvent(event);
}

/******************************************************************************/
void sinkBleClearWhiteListEvent(void)
{
    ble_gap_event_t event;

    /* Send GAP event to power off */
    event.id = ble_gap_event_clear_white_list;
    event.args = NULL;
    sinkBleGapEvent(event);
}

#ifdef ENABLE_FAST_PAIR
/******************************************************************************/
void sinkBleExitDiscoverableModeEvent(void)
{
    ble_gap_event_t event;

    /* Send GAP event on exiting discoverable mode (BR/EDR) */
    event.id = ble_gap_event_exit_discoverable_mode;
    event.args = NULL;
    sinkBleGapEvent(event);
}
#endif

/******************************************************************************/
const ble_configuration_t *sinkBleGetConfiguration(void)
{
    return &ble_config;
}

/******************************************************************************/
void sinkBleSetAdvertisingParamsDefault(uint16 adv_interval_min, uint16 adv_interval_max)
{
    ble_adv_params_t params;
    bool random_addr = FALSE;
#ifdef ENABLE_RANDOM_ADDR
    random_addr = TRUE;
#endif  

    params.undirect_adv.adv_interval_min = adv_interval_min;
    params.undirect_adv.adv_interval_max = adv_interval_max;
    params.undirect_adv.filter_policy = ble_filter_none;

#ifdef ENABLE_FAST_PAIR
    sinkGattManagerStopAdvertising();    
#endif

    BLE_INFO(("Set BLE Default Advertising Params rnd=%d\n", random_addr));

    ConnectionDmBleSetAdvertisingParamsReq(ble_adv_ind, random_addr, BLE_ADV_CHANNEL_ALL, &params);
}

/******************************************************************************/
void sinkBleSetMasterConnectionParamsDefault(uint16 scan_interval, uint16 scan_window)
{
    ble_connection_params params;

    params.scan_interval = scan_interval;
    params.scan_window = scan_window;
    params.conn_interval_min = ble_master_initial_conn_params.conn_interval_min;
    params.conn_interval_max = ble_master_initial_conn_params.conn_interval_max;
    params.conn_latency = ble_master_initial_conn_params.conn_latency;
    params.supervision_timeout = ble_master_initial_conn_params.supervision_timeout;
    params.conn_attempt_timeout = ble_master_initial_conn_params.conn_attempt_timeout;
    params.conn_latency_max = ble_master_initial_conn_params.conn_latency_max;
    params.supervision_timeout_min = ble_master_initial_conn_params.supervision_timeout_min;
    params.supervision_timeout_max = ble_master_initial_conn_params.supervision_timeout_max;
#ifdef ENABLE_RANDOM_ADDR
    params.own_address_type = TYPED_BDADDR_RANDOM;
#else
    params.own_address_type = TYPED_BDADDR_PUBLIC;
#endif
    
    BLE_INFO(("Set BLE Default Connection Params\n"));
    
    ConnectionDmBleSetConnectionParametersReq(&params);
}


/******************************************************************************/
void sinkBleSetMasterConnectionParamsUpdate(typed_bdaddr *taddr)
{
    BLE_INFO(("Set BLE Updated Master Connection Params\n"));
    
    ConnectionDmBleConnectionParametersUpdateReq(
                sinkGetBleTask(),
                taddr,
                ble_master_update_conn_params.conn_interval_min,
                ble_master_update_conn_params.conn_interval_max,
                ble_master_update_conn_params.conn_latency,
                ble_master_update_conn_params.supervision_timeout,
                ble_master_update_conn_params.ce_length_min,
                ble_master_update_conn_params.ce_length_max
                );
}


/******************************************************************************/
void sinkBleSetSlaveConnectionParamsUpdate(typed_bdaddr *taddr)
{
    BLE_INFO(("Set BLE Updated Slave Connection Params\n"));

    ConnectionDmBleConnectionParametersUpdateReq(
                sinkGetBleTask(),
                taddr,
                ble_slave_update_conn_params.conn_interval_min,
                ble_slave_update_conn_params.conn_interval_max,
                ble_slave_update_conn_params.conn_latency,
                ble_slave_update_conn_params.supervision_timeout,
                ble_slave_update_conn_params.ce_length_min,
                ble_slave_update_conn_params.ce_length_max
                );
}

/******************************************************************************/
void sinkBleGetAdvertisingParameters(bool fast_adv, ble_advertising_parameters_t *adv_params)
{
    if (fast_adv)
    {
        adv_params->interval_min = sinkBleGetConfiguration()->adv_interval_min_fast;
        adv_params->interval_max = sinkBleGetConfiguration()->adv_interval_max_fast;
    }
    else
    {
         if(stateManagerGetState() != deviceLimbo)
        {
            adv_params->interval_min = sinkBleGetConfiguration()->adv_interval_min_slow;
            adv_params->interval_max = sinkBleGetConfiguration()->adv_interval_max_slow;        
        }
        else
        {
            /*Limbo state  legacy slow connection params used*/
            adv_params->interval_min = LEGACY_SLOW_ADV_INTERVAL_MIN;
            adv_params->interval_max = LEGACY_SLOW_ADV_INTERVAL_MAX;
        }
    }
}


/******************************************************************************/
void sinkBleGetScanningParameters(bool fast_scan, ble_scanning_parameters_t *scan_params)
{
    if (fast_scan)
    {
        scan_params->interval = sinkBleGetConfiguration()->scan_interval_fast;
        scan_params->window = sinkBleGetConfiguration()->scan_window_fast;
    }
    else
    {
        scan_params->interval = sinkBleGetConfiguration()->scan_interval_slow;
        scan_params->window = sinkBleGetConfiguration()->scan_window_slow;
    }
}

/******************************************************************************/
void sinkBleDeleteMarkedDevices(void)
{
    if (BdaddrIsZero(&GAP.security_failed_device.addr))
    {
        /* No device is marked for deletion */
        return;
    }
    /* Delete the device from TDL */
    ConnectionSmDeleteAuthDeviceReq(GAP.security_failed_device.type, &GAP.security_failed_device.addr);
    BLE_INFO(("Marked LE Device deleted...\n"));
    /* clear the marked device */
    BdaddrTypedSetEmpty(&GAP.security_failed_device);
}

/******************************************************************************/
void sinkBleSimplePairingCompleteInd(const CL_SM_BLE_SIMPLE_PAIRING_COMPLETE_IND_T *ind)
{
    uint16 cid = GattGetCidForBdaddr(&ind->tpaddr.taddr);

    /* A client might have written Client Characteristic Configuration data before bonding.
       If the device is now bonded, this configuration should persist for future connections. */
    if (cid && (ind->status == success))
    {
        if(!(sinkBroadcastAudioIsActive() && sinkBroadcasterIsReceiverCid(cid)))
        {
            gattServerStoreConfigAttributes(cid, gatt_attr_service_all);

            sinkGattHidAddPriorityDevicesList(cid);
            /* Make sure that priority devices are looked after in the PDL.*/
            deviceManagerUpdatePriorityDevices();
        }
        else
        {
            /* Broadcaster is successfully associated with receiver. If remote BD address is RANDOM then store receiver BD address 
            so that it can be removed from PDL post association */
            if(ind->tpaddr.taddr.type == TYPED_BDADDR_RANDOM)
                sinkBroadcasterSetReceiverAddress((const bdaddr *)&ind->permanent_taddr.addr);
        }
    }
}

/******************************************************************************/
bool SinkBleConnectionParameterIsOutOfRange(const CL_DM_BLE_ACCEPT_CONNECTION_PAR_UPDATE_IND_T *ind)
{
    bool outOfRange = FALSE;
    gatt_client_connection_t *connection = NULL;
    uint16 cid = GattGetCidForBdaddr(&ind->taddr);

    connection = gattClientFindByCid(cid);

    /* Accept or reject the connection parameter update request; will be rejected if Cid is invalid
     * or the latency, minimum and maximum requested values are not in the allowed range.
     */
    if(!connection)
    {
       outOfRange = TRUE;
    }
    else if(connection->role == ble_gap_role_central)
    {
        if(ind->conn_interval_min < SINK_BLE_CONN_PARAM_MASTER_CONN_INTERVAL_MIN)
        {
            outOfRange = TRUE;
        }
    }
    else if (connection->role == ble_gap_role_peripheral)
    {
        if (gaiaIsGattUpgradeInProgress())
        {
            if (ind->conn_interval_max > SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MAX)
            {
                outOfRange = TRUE;
            }
        }
        else if(ind->conn_interval_min < SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN || 
            ind->conn_interval_max > SINK_BLE_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX ||
            ind->conn_latency < SINK_BLE_CONN_PARAM_SLAVE_CONN_LATENCY)
        {
            outOfRange = TRUE;
        }
    }

    return outOfRange;
}

/******************************************************************************/
bool sinkBleCheckBdAddrtIsBonded(const typed_bdaddr *client_taddr, typed_bdaddr *public_taddr)
 {

    tp_bdaddr current_addr;
    tp_bdaddr public_addr;
 
     BLE_INFO(("gattServerBdAddrtIsBonded\n"));
 
    if(client_taddr == NULL || client_taddr->type == TYPED_BDADDR_INVALID)
        return FALSE;

    memset(&public_addr, 0, sizeof(tp_bdaddr));
    
    current_addr.transport = TRANSPORT_BLE_ACL;
    current_addr.taddr = *client_taddr;

    /* Retrieve permanent address if this is a random address */
    if(current_addr.taddr.type == TYPED_BDADDR_RANDOM)
    {
        VmGetPublicAddress(&current_addr, &public_addr);
    }
    else
    {
        /* Provided address is PUBLIC address, copy it */
        public_addr.transport = TRANSPORT_BLE_ACL;
        public_addr.taddr = *client_taddr;
    }
 
    *public_taddr = public_addr.taddr;
 
    BLE_INFO((" Public Addr[%x:%x:%lx]\n",
        public_taddr->addr.nap,
        public_taddr->addr.uap,
        public_taddr->addr.lap));
 
    return ConnectionSmGetAttributeNow(0, &public_taddr->addr, 0, NULL);
 }

#ifdef ENABLE_BROADCAST_AUDIO

/*****************************************************************************
   Utility function to know if A2DP is connected & playing
*/
static bool isA2dpNotConnectedOrNotPlaying(const bdaddr *dev_addr)
{
    uint16 index;

    if(!getA2dpIndexFromBdaddr(dev_addr, &index))
    {
        return TRUE;
    }
    else if(!getA2dpStatusFlag(PLAYING, index))
    {
        return TRUE;
    }
    return FALSE;
}

/*****************************************************************************
   Utility function to get the GATT cid of non-streaming A2DP AG
*/
static uint16 getNotStreamingAgCid(void)
{
    uint16 index;
    uint16 cid = GATT_INVALID_INDEX;
    
    /* First check if their is a2dp link existing */
    for_all_a2dp(index)
    {   /* Don't disturb the playing AG */
        if(!getA2dpStatusFlag(PLAYING, index))
        {
            cid = gattCommonGetCidForAddr(getA2dpLinkBdAddr(index));
            if (cid != GATT_INVALID_INDEX)
                return cid;
        }
    }
    return cid;
}

/*****************************************************************************
   Utility function to get the GATT cid of connected HFP
*/
static uint16 getConnectedHfpAgCid(void)
{
    hfp_link_priority hfp;
    bdaddr dev_addr;
    uint16 cid = GATT_INVALID_INDEX;
    
    /* if a2dp is not connected, check if hfp link is connected */
    for( hfp = hfp_primary_link; hfp <= hfp_secondary_link; hfp++)
    {
        /* Only valid if we have HFP connect */
        if(HfpLinkGetBdaddr(hfp, &dev_addr))
        { /* And if there is corresponding a2dp link, it should not be playing */
            if(isA2dpNotConnectedOrNotPlaying(&dev_addr))
                cid = gattCommonGetCidForAddr(&dev_addr);
            
            /* If get correct CID, return it */
            if(cid != GATT_INVALID_INDEX)
                return cid;
        }    
    }
    return cid;
}

/*****************************************************************************
   Utility function to get the GATT cid of connected lone LE 
*/
static uint16 getConnectedLoneGattCid(void)
{
    uint16 index;
    tp_bdaddr current_bd_addr;
    tp_bdaddr per_bd_addr;
    uint16 cid = GATT_INVALID_INDEX;
    
    /* It could be that 
      * 1. Both links have only LE connection
      * 2. One link has a2dp connected but streaming and other is lone LE connection 
      */
    for(index=0;index < MAX_BLE_CONNECTIONS; index++)
    {  
        if(VmGetBdAddrtFromCid(GATT[index].cid, &current_bd_addr))
        {
             per_bd_addr = current_bd_addr;
             if(current_bd_addr.taddr.type == TYPED_BDADDR_RANDOM) 
                  VmGetPublicAddress(&current_bd_addr, &per_bd_addr);
            /* dont disturb the playing link */
            if(isA2dpNotConnectedOrNotPlaying(&per_bd_addr.taddr.addr))
                cid = GATT[index].cid;
            /* If get correct CID, return it */
            if( cid != GATT_INVALID_INDEX)
                return GATT[index].cid;
        }
    }
    return cid;
}

/*****************************************************************************
   Utility function to get the cid of the AG which has LE link
*/
static uint16 getConnectedAgCid(void)
{    
    uint16 cid = GATT_INVALID_INDEX;

    /* First check if we have connect AG with A2DP connection
        which is not streaming */
    cid = getNotStreamingAgCid();
    if(cid != GATT_INVALID_INDEX)
        return cid;
    
    /* Check if HFP is connected to AG and its corresponding A2DP is 
        not playing */
    cid = getConnectedHfpAgCid();
    if(cid != GATT_INVALID_INDEX)
        return cid;

    /* Check if there is lone LE link or LE link with A2DP which is not 
    playing */
    cid = getConnectedLoneGattCid();
    if(cid != GATT_INVALID_INDEX)
        return cid;
    
    return GATT_INVALID_INDEX;
}

/*****************************************************************************
   Utility function to disconnect the peripheral link 
*/
static void bleDisconnectPeripheralLELink(void)
{
    uint16 cid;
    
    if(!gattServerIsFullyDisconnected())
    {
        /* Ok,there is some peripheral link. Now the priority for disconnection is
            1. The LE link of AG which isn't streaming & has LE connection
            2. The LE link of AG which has just HFP & LE connection
            3. The LE link of any AG which has just LE connection */
        cid = getConnectedAgCid();
        if(cid != GATT_INVALID_INDEX)
        {
            /* Finally got hold of the peripheral link to get disconnected */
            gattServerDisconnect(cid);
        }
    }
}

/******************************************************************************/
bool sinkBleDisconnectCentralLELink(void)
{
    uint16 cid;
    /* Check if there is central LE connection */
    if(gattClientHasClients())
    {
        /* first disconnect the central link */
        cid = gattCommonGetCentralConnCid();
        if(cid != GATT_INVALID_INDEX)
        {
            gattClientDisconnect(cid);
            return TRUE;
        }
    }
    return FALSE;
}

/******************************************************************************/
void sinkBleDisconnectOneLELink(void)
{
    /* Central connection is the least priority and so can 
        be disconnected for reserving the link for association */
    if(!sinkBleDisconnectCentralLELink())
    {
        /* In BA mode it is possible that BLE state machine moves to fully
        connected state after connecting one peripheral link since we reserve one
        LE link for association. So make sure that we don't disconnect that single 
        peripheral link */
        if(gattCommonHasMaxConnections())
            /* Find the peripheral link to be disconnected */
            bleDisconnectPeripheralLELink();
    }
}

/******************************************************************************/
void sinkBleStartAdvertisingOnAssociationEnd(void)
{
    MessageSendConditionally(sinkGetBleTask(), BLE_INTERNAL_MESSAGE_START_ADVERTISING_ON_ASSOCIATION_END,
        NULL, gapBaGetReferenceOfAssociationInProgress());
}

#endif /* ENABLE_BROADCAST_AUDIO */


#if defined ENABLE_GAIA && defined ENABLE_UPGRADE
/* Connection update parameters when BLE Upgrade in progress */
#define SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MIN 12
#define SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_LATENCY  0
#define SINK_BLE_CONN_PARAM_UPGRADE_SUPERVISION_TIMEOUT 400
#define SINK_BLE_CONN_PARAM_UPGRADE_CE_LENGTH_MIN 0
#define SINK_BLE_CONN_PARAM_UPGRADE_CE_LENGTH_MAX 160

/* Request for 15 ms CI (12*1.25), so min and max are same */
static const ble_connection_update_parameters_t ble_slave_update_conn_params_for_upgrade = {
    SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MIN,    /* Minimum connection interval */
    SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MIN,   /* Maximum connection interval */
    SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_LATENCY,         /* Connection latency */
    SINK_BLE_CONN_PARAM_UPGRADE_SUPERVISION_TIMEOUT,        /* supervision timeout */
    SINK_BLE_CONN_PARAM_UPGRADE_CE_LENGTH_MIN,              /* Minimum length of the connection */
    SINK_BLE_CONN_PARAM_UPGRADE_CE_LENGTH_MAX               /* Maximum length of the connection */
};

 typedef enum __ble_retry_conn_parameters
{
    BLE_NO_RETRY_CONN_PARAMS = 0,
    BLE_RETRY_CONN_PARAMS_APPLY,
    BLE_RETRY_CONN_PARAMS_REVERT

} ble_retry_conn_parameters;

typedef struct __ble_upgrade_data_t
{
    ble_retry_conn_parameters   retry_conn_params; /*To retry connection parameters in case of failure/collision during BLE Upgrade*/
    uint16                                  max_conn_int; /*Retry connection parameter request with incremental connection interval for upgrade use case*/
}ble_upgrade_data_t;

static ble_upgrade_data_t ble_upgrade_data = {BLE_NO_RETRY_CONN_PARAMS, 0};

/*******************************************************************************
NAME
    bleResetRetryParams
    
DESCRIPTION
    Reset the retry parameters
    
PARAMETERS
    void
    
RETURNS
    None
*/
static void bleResetRetryParams(void)
{
    ble_upgrade_data.retry_conn_params = BLE_NO_RETRY_CONN_PARAMS;
}

/*******************************************************************************
NAME
    bleSetRetryParams
    
DESCRIPTION
    Set the retry parameters
    
PARAMETERS
    retry params to apply lower connection interval or revert to default higher connection interval
    
RETURNS
    None
*/
static void bleSetRetryParams(ble_retry_conn_parameters params)
{
    ble_upgrade_data.retry_conn_params = params;
}

/*******************************************************************************
NAME
    bleSetMaxConnInt
    
DESCRIPTION
    Set max connection interval parameters. This is in incremental values of 15ms, 30ms, 45ms 
    in case the previous request is rejected/failed
    
PARAMETERS
    maximum connection interval to set
    
RETURNS
    None
*/
static void bleSetMaxConnInt(uint16 max_ci)
{
    ble_upgrade_data.max_conn_int = max_ci;
}

/*******************************************************************************
NAME
    bleGetMaxConnInt
    
DESCRIPTION
    Get max connection interval parameters.
    
PARAMETERS
    void
    
RETURNS
    None
*/
static uint16 bleGetMaxConnInt(void)
{
    return (ble_upgrade_data.max_conn_int);
}

/*******************************************************************************
NAME
    bleResetMaxConnInt
    
DESCRIPTION
    Reset the max connection interval before starting upgrade or on a successful BLE connection parameter update

PARAMETERS
    void
    
RETURNS
    None
*/
static void bleResetMaxConnInt(void)
{
    ble_upgrade_data.max_conn_int = 0;
}

/*******************************************************************************
NAME
    bleSetIncrementalCI
    
DESCRIPTION
    Set the BLE connection parameters max value in incremental fashion.
    The first try is with 15ms CI, retry once again with 15, then with 30ms, 45ms and 60ms.
    Used only when Upgrade is in progress
    
PARAMETERS
    void
    
RETURNS
    None
*/
static void bleSetIncrementalCI(void)
{
    uint16 next_ci = bleGetMaxConnInt();
    
    /*retry with 15ms, 30ms, 45ms, 60ms CI*/
    next_ci += SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MIN;

    BLE_INFO(("bleSetIncrementalCI next_ci [0x%x] \n", next_ci));

    bleSetMaxConnInt(next_ci);
}


/******************************************************************************/
void sinkBleResetParamsForUpgrade(void)
{
    bleResetMaxConnInt();
    bleResetRetryParams();
}

/******************************************************************************/
void sinkBleHandleConnParamFailures(uint16 status)
{
    if (gaiaIsGattUpgradeInProgress())
    {    
        if (status==hci_success)
        {
            bleResetMaxConnInt();
        }
        else
        {
            ble_retry_conn_parameters retry_params = ble_upgrade_data.retry_conn_params;
            BLE_INFO(("sinkBleHandleConnParamFailures retry [0x%x] max_ci [%d] \n", retry_params, bleGetMaxConnInt()));

            /*Make sure the loop is not repeated for CFM and IND*/
            if  (retry_params != BLE_NO_RETRY_CONN_PARAMS)
            {
                bool sendUpgradeEvent = FALSE;

                if (bleGetMaxConnInt()==SINK_BLE_CONN_PARAM_UPGRADE_SLAVE_CONN_INTERVAL_MAX)
                /*Make sure the retry loop ends*/
                {
                    bleResetMaxConnInt();
                    MessageCancelAll(&theSink.task, EventSysUpgradeOptimisation);
                }
                else  if (status==hci_error_different_transaction_collision)
                    /*For collision case, retry with same CI as before*/
                {
                    sendUpgradeEvent = TRUE;
                }
                else if ((status==hci_error_qos_unacceptable_parameter) ||
                    (status==hci_error_invalid_lmp_parameters) ||
                    (status==hci_error_qos_rejected) ||
                    (status==hci_error_unspecified) ||
                    (status==hci_error_qos_not_supported))
                    /*For Reject cases, retry with incremented CI*/
                {                       
                    /*Conn parameters retry only for Upgrade use case*/
                    if (retry_params == BLE_RETRY_CONN_PARAMS_APPLY)
                    {
                        bleSetIncrementalCI();
                    }
                    sendUpgradeEvent = TRUE;
                }

                if (sendUpgradeEvent)
                {
                    /*Conn parameters retry only for Upgrade use case*/
                    if (retry_params == BLE_RETRY_CONN_PARAMS_APPLY)
                    {
                        sinkGaiaSendUpgradeOptimisationEvent(TRUE);
                    }
                    else if (retry_params == BLE_RETRY_CONN_PARAMS_REVERT)
                    {
                        sinkGaiaSendUpgradeOptimisationEvent(FALSE);
                    }
                }
            }
            
            bleResetRetryParams();
        }
    }
}

/******************************************************************************/
static void sinkBleSetSlaveConnectionParamsForUpgrade(typed_bdaddr *taddr)
{
    uint16 max_interval = bleGetMaxConnInt();
    if (max_interval==0)
    {
        max_interval = ble_slave_update_conn_params_for_upgrade.conn_interval_max;
    }
    
    BLE_INFO(("Set BLE Updated Slave Connection Params For Upgrade max_conn_interval [0x%x]\n", max_interval));

    ConnectionDmBleConnectionParametersUpdateReq(
                sinkGetBleTask(),
                taddr,
                ble_slave_update_conn_params_for_upgrade.conn_interval_min,
                max_interval,
                ble_slave_update_conn_params_for_upgrade.conn_latency,
                ble_slave_update_conn_params_for_upgrade.supervision_timeout,
                ble_slave_update_conn_params_for_upgrade.ce_length_min,
                ble_slave_update_conn_params_for_upgrade.ce_length_max
                );
}


/******************************************************************************/
void sinkBleApplyParamsForUpgrade(uint16 cid, bool apply)
{
    tp_bdaddr *tpaddrt = PanicUnlessMalloc(sizeof(tp_bdaddr));

    BLE_INFO(("sinkBleApplyParamsForUpgrade cid 0x%x Apply %d \n", cid, apply));

    if ((cid!=INVALID_CID) && (VmGetBdAddrtFromCid(cid, tpaddrt)))
    {
        if (apply)
        {
            bleSetRetryParams(BLE_RETRY_CONN_PARAMS_APPLY);
            sinkBleSetSlaveConnectionParamsForUpgrade(&tpaddrt->taddr);
        }
        else
        {
            bleSetRetryParams(BLE_RETRY_CONN_PARAMS_REVERT);
            sinkBleSetSlaveConnectionParamsUpdate(&tpaddrt->taddr);
        }        
    }
      
    free(tpaddrt);
}

#endif /*ENABLE_GAIA && ENABLE_UPGRADE*/

#ifdef ENABLE_FAST_PAIR
/******************************************************************************/
void sinkBleIndicateSCOStatusChanged(void)
{
    ble_gap_event_t event;
        
    /* Send GAP event to indicate SCO link is up */
    event.id = ble_gap_event_sco_status_changed;
    event.args = NULL;
    sinkBleGapEvent(event);
}
#endif /* ENABLE_FAST_PAIR*/

#ifdef GATT_BISTO_COMM_SERVER
/* Connection update parameters if a Bisto voice capture is in progress */
#define SINK_BLE_BISTO_QUERY_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN 12
#define SINK_BLE_BISTO_QUERY_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX 12
#define SINK_BLE_BISTO_QUERY_PARAM_SLAVE_CONN_LATENCY           4
#define SINK_BLE_BISTO_QUERY_PARAM_SUPERVISION_TIMEOUT          200
#define SINK_BLE_BISTO_QUERY_PARAM_CE_LENGTH_MIN                0
#define SINK_BLE_BISTO_QUERY_PARAM_CE_LENGTH_MAX                80

static const ble_connection_update_parameters_t ble_slave_update_conn_params_for_voice_capture =
{
    SINK_BLE_BISTO_QUERY_CONN_PARAM_SLAVE_CONN_INTERVAL_MIN,            /* Minimum connection interval */
    SINK_BLE_BISTO_QUERY_CONN_PARAM_SLAVE_CONN_INTERVAL_MAX,            /* Maximum connection interval */
    SINK_BLE_BISTO_QUERY_PARAM_SLAVE_CONN_LATENCY,                      /* Connection latency */
    SINK_BLE_BISTO_QUERY_PARAM_SUPERVISION_TIMEOUT,                     /* supervision timeout */
    SINK_BLE_BISTO_QUERY_PARAM_CE_LENGTH_MIN,                           /* Minimum length of the connection */
    SINK_BLE_BISTO_QUERY_PARAM_CE_LENGTH_MAX                            /* Maximum length of the connection */
};

/******************************************************************************/
static void sinkBleSetSlaveConnectionParamsForVoiceCapture(typed_bdaddr *taddr)
{
    BLE_INFO(("Set BLE Updated Slave Connection Params For Voice Capture\n"));

    ConnectionDmBleConnectionParametersUpdateReq(
                sinkGetBleTask(),
                taddr,
                ble_slave_update_conn_params_for_voice_capture.conn_interval_min,
                ble_slave_update_conn_params_for_voice_capture.conn_interval_max,
                ble_slave_update_conn_params_for_voice_capture.conn_latency,
                ble_slave_update_conn_params_for_voice_capture.supervision_timeout,
                ble_slave_update_conn_params_for_voice_capture.ce_length_min,
                ble_slave_update_conn_params_for_voice_capture.ce_length_max
                );
}

/******************************************************************************/
void sinkBleApplyConnectionParamsForVoiceCapture(uint16 cid, bool apply)
{
    tp_bdaddr tpaddrt = {0};

    BLE_INFO(("sinkBleApplyParamsForVoiceCapture cid 0x%x Apply %d \n", cid, apply));

    if ((cid!=INVALID_CID) && (VmGetBdAddrtFromCid(cid, &tpaddrt)))
    {
        if (apply)
        {
            sinkBleSetSlaveConnectionParamsForVoiceCapture(&tpaddrt.taddr);
        }
        else
        {
            sinkBleSetSlaveConnectionParamsUpdate(&tpaddrt.taddr);
        }
    }
}
#endif /* GATT_BISTO_COMM_SERVER */

#endif /* GATT_ENABLED */

