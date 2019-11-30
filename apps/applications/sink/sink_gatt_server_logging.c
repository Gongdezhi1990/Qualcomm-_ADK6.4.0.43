/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_logging.c

DESCRIPTION
    Routines to handle messages sent from the GATT Logging Task.
*/

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <util.h>

#ifdef ACTIVITY_MONITORING
/* Library headers */


/* Application headers */
#include "sink_private_data.h"
#include "sink_gatt_common.h"
#include "sink_gatt_db.h"
#include "sink_ble.h"
#include "sink_gatt_server.h"
#include "sink_gatt_server_logging.h"
#include "sink_development.h"
#include "sport_health_algorithms.h"
#include "sink_sport_health.h"

#ifdef GATT_LOGGING_SERVER
#include "sink_gatt_server_rscs.h"
#include "sink_gatt_server_hrs.h"
#include "gatt_logging_server.h"
#include "sink_debug.h"
#ifdef DEBUG_GATT_LOGGING_SERVER
#define GATT_LOGGING_SERVER_INFO(x) DEBUG(x)
#define GATT_LOGGING_SERVER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_LOGGING_SERVER_INFO(x)
#define GATT_LOGGING_SERVER_ERROR(x)
#endif


#include "sport_health_logging.h"
#include <stdarg.h>

static void GattLoggingNotify(const uint8 *p_data, uint16 size);

/*******************************************************************************
NAME
    handleReadLoggingControl

DESCRIPTION
    Handle when a GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message is received.

PARAMETERS
    ind Pointer to a GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND message.

RETURNS
    void
*/
static void handleReadLoggingControl(const GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND_T * ind)
{

    uint8 logging_control = 0;
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* Store the logging control value written by client to logging server */
    GATT_LOGGING_SERVER_INFO(("GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND_T logging=[0x%p] cid=[0x%x] value[0x%x]\n",
        (void *)ind->logging, ind->cid, ind->logging->logging_control));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->logging == GATT_SERVER.logging_server)
        {
            logging_control = GATT_SERVER.logging_server->logging_control;
            GattLoggingServerReadLoggingControlResponse(ind->logging, ind->cid, logging_control);
        }
    }
    SHG_DBG_MSG0(SH_LOG_DEBUG,"GATT Logging started");
    UNUSED(ind);
}

/*******************************************************************************
NAME
    handleWriteLoggingControl

DESCRIPTION
    Handle when a GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND message is received.

PARAMETERS
    ind Pointer to a GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND message.

RETURNS
    void
*/
static void handleWriteLoggingControl(const GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND_T * ind)
{

    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* Store the logging control value written by client to logging server */
    GATT_LOGGING_SERVER_INFO(("GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND_T logging=[0x%p] cid=[0x%x] value[0x%x]\n",
        (void *)ind->logging, ind->cid, ind->logging->logging_control));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->logging == GATT_SERVER.logging_server)
        {
            GATT_SERVER.logging_server->logging_control =  ind->logging->logging_control;
            GATT_LOGGING_SERVER_INFO((" Logging control[0x%x]\n", GATT_SERVER.logging_server->logging_control));
        }
    }

    UNUSED(ind);
}


/*******************************************************************************
NAME
    handleReadLoggingDebugConfiguration

DESCRIPTION
    Handle when a GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND message is received.

PARAMETERS
    ind Pointer to a GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND message.

RETURNS
    void
*/
static void handleReadLoggingDebugConfiguration(const GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND_T * ind)
{

    uint8 debug_configuration[LOGGING_DEBUG_CONFIGURATION_DATA_SIZE];
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* Store the logging control value written by client to logging server */
    GATT_LOGGING_SERVER_INFO(("GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND_T logging=[0x%p] cid=[0x%x] value[0x%x]\n",
        (void *)ind->logging, ind->cid, ind->logging->logging_control));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->logging == GATT_SERVER.logging_server)
        {
            uint16 logging_debug_config_len = sinkSportHealthGetLoggingDebugConfiguration(debug_configuration, LOGGING_DEBUG_CONFIGURATION_DATA_SIZE);
            if (logging_debug_config_len == 0) return;
            for( index = 0;index < MAX_BLE_CONNECTIONS;index++ )
            {
                /* Send the Logging Debug Configuration data. */
                GattLoggingServerReadLoggingDebugConfigurationResponse(ind->logging, ind->cid, (uint16)logging_debug_config_len, debug_configuration);
            }
        }
    }
    UNUSED(ind);
}

/*******************************************************************************
NAME
    handleWriteLoggingDebugConfiguration

DESCRIPTION
    Handle when a GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND message is received.

PARAMETERS
    ind Pointer to a GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND message.

RETURNS
    void
*/
static void handleWriteLoggingDebugConfiguration(const GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND_T * ind)
{

    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* Store the logging control value written by client to logging server */
    GATT_LOGGING_SERVER_INFO(("GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND_T logging=[0x%p] cid=[0x%x] value[0x%x]\n",
        (void *)ind->logging, ind->cid, ind->p_debug_configuration[0]));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->logging == GATT_SERVER.logging_server)
        {
            sinkSportHealthSetLoggingDebugConfiguration(ind->p_debug_configuration);
        }
    }

    UNUSED(ind);
}


/*******************************************************************************
NAME
    handleReadLoggingReportClientConfig

DESCRIPTION
    Handle when a GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message is received.

PARAMETERS
    ind Pointer to a GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleReadLoggingReportClientConfig(const GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);
    uint16 client_config = 0;

    /* Return the current value of the client configuration descriptor for the device */
    GATT_LOGGING_SERVER_INFO(("GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND logging=[0x%p] cid=[0x%x]\n", (void *)ind->rsc_sensor, ind->cid));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->logging == GATT_SERVER.logging_server)
            client_config = GATT_SERVER.client_config.logging;
    }

    GattLoggingServerReadClientConfigResponse(ind->logging, ind->cid, client_config);
    GATT_LOGGING_SERVER_INFO(("  client_config=[0x%x]\n", client_config));

}

/*******************************************************************************
NAME
    handleWriteLoggingReportClientConfig

DESCRIPTION
    Handle when a GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND message is received.

PARAMETERS
    ind Pointer to a GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleWriteLoggingReportClientConfig(const GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /*
     * Check whether the remote device has enabled or disabled
     * notifications for the logging report characteristic. This value will need
     * to be stored as device attributes so they are persistent.
    */
    GATT_LOGGING_SERVER_INFO(("GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND logging=[0x%p] cid=[0x%x] value[0x%x]\n",
        (void *)ind->logging, ind->cid, ind->config_value));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->logging == GATT_SERVER.logging_server)
        {
            GATT_SERVER.client_config.logging = ind->config_value;
            GATT_LOGGING_SERVER_INFO((" Logging client_config[0x%x]\n", GATT_SERVER.client_config.logging));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_logging);
            if ((GATT_SERVER.client_config.logging) & 1) { // Logging on - disable other notifications
                /* Disable the RSC & HR measurement notifications */
                MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_LOGGING_READ_TIMER );
                MessageCancelFirst( sinkGetBleTask(), GATT_SERVER_HR_UPDATE_TIME);
                // Register the notification callback
                SportHealthLoggingRegisterNotifyCallback(GattLoggingNotify);
            }
            else
            { // Enable RSC + HR notifications, disable logging
                SportHealthLoggingRegisterNotifyCallback(NULL);
                sinkBleNotifyRSCMeasurements();
                sinkBleNotifyHrMeasurements();
            }
        }
    }
}

// Send notifications on all open connections with the supplied data
static void GattLoggingNotify(const uint8 *p_data, uint16 size)
{
    uint16 index;
    for( index = 0;index < MAX_BLE_CONNECTIONS;index++ ) {
        GattLoggingServerSendNotification(GATT_SERVER.logging_server,
                                          GATT[index].cid, size, (uint8 *)p_data);
    }
}

/*******************************************************************************/
bool sinkGattLoggingServerInitialise(uint16 **ptr)
{
    if (GattLoggingServerInit( sinkGetBleTask(),  (GLOG_T *)*ptr, HANDLE_LOGGING_SERVICE,
                                     HANDLE_LOGGING_SERVICE_END))
    {
        GATT_LOGGING_SERVER_INFO(("GATT Logging initialized\n"));
        gattServerSetServicePtr(ptr, gatt_server_service_logging);
        /* The size of logging is also calculated and memory is allocated.
         * So advance the ptr so that the next Server while initializing.
         * shall not over write the same memory */
        *ptr += ADJ_GATT_STRUCT_OFFSET(GLOG_T);
        return TRUE;
    }

    /* Logging Service library initialization failed */
    GATT_LOGGING_SERVER_INFO(("GATT logging server init failed \n"));
    return FALSE;
}

/******************************************************************************/
void sinkGattLoggingServerMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
        case GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND:
        {
            handleReadLoggingReportClientConfig((const GATT_LOGGING_SERVER_READ_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            handleWriteLoggingReportClientConfig((const GATT_LOGGING_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND:
        {
            handleReadLoggingControl((const GATT_LOGGING_SERVER_READ_LOGGING_CONTROL_IND_T*)message);
        }
        break;
        case GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND:
        {
            handleWriteLoggingControl((const GATT_LOGGING_SERVER_WRITE_LOGGING_CONTROL_IND_T*)message);
        }
        break;
        case GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND:
        {
            handleReadLoggingDebugConfiguration((const GATT_LOGGING_SERVER_READ_DEBUG_CONFIGURATION_IND_T*)message);
        }
        break;
        case GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND:
        {
            handleWriteLoggingDebugConfiguration((const GATT_LOGGING_SERVER_WRITE_DEBUG_CONFIGURATION_IND_T*)message);
        }
        break;
        default:
        {
            /* Unhandled messages are not critical errors */
            GATT_LOGGING_SERVER_ERROR(("GATT Unhandled msg id[%x]\n", id));
        }
    }
}

#endif /* GATT_LOGGING_SERVER */
#endif /* ACTIVITY_MONITORING */

