/****************************************************************************
Copyright (c) 2015 - 2016 Qualcomm Technologies International, Ltd.

FILE NAME
    sink_gatt_client_ias.c

DESCRIPTION
    Routines to handle the GATT Imm Alert Service.
*/

#include <stdlib.h>
#include <connection.h>
#include <gatt.h>

#include "sink_gatt_client_ias.h"
#include "sink_ble.h"

#include "sink_debug.h"
#include "sink_gatt_client.h"
#include "sink_gatt_client_hid.h"
#include "sink_main_task.h"
#include "sink_events.h"
#include "sink_ble_scanning.h"
#include "sink_gatt_common.h"
#include "sink_configmanager.h"

#ifdef GATT_IAS_CLIENT
#include "config_definition.h"
#include "sink_gatt_common_ias_config_def.h"
#include <config_store.h>

static const uint8 ias_ble_advertising_filter[] = {GATT_SERVICE_UUID_IMMEDIATE_ALERT & 0xFF, GATT_SERVICE_UUID_IMMEDIATE_ALERT >> 8};

#ifdef DEBUG_GATT_IAS_CLIENT
#define GATT_IAS_CLIENT_DEBUG(x) DEBUG(x)
#else
#define GATT_IAS_CLIENT_DEBUG(x)
#endif

/* During alerting the phone, we are the peripheral device. Hence only IAS client is checked */
/* During alerting the remote, we are the central device and the alert needs to be sent on HID */

/* If the device to be alerted is a remote */
#define ALERT_FOR_REMOTE(type) (type == sink_gatt_ias_alert_remote)
#define ALERT_FOR_PHONE(type) (type == sink_gatt_ias_alert_phone)
/* Check for HID Client connection to alert */
#define HID_INSTANCE_AVAILABLE(service) ((service->hidc_instance1) || (service->hidc_instance2))
#define HID_CONNECTED(conn, service) ((gattClientIsServiceAvailable(conn, gatt_client_hid)) || HID_INSTANCE_AVAILABLE(service))
#define IS_REMOTE_TO_BE_ALERTED(type, conn, service) (ALERT_FOR_REMOTE(type) && HID_CONNECTED(conn, service))
/* If the device to be alerted is a phone */
#define IS_PHONE_TO_BE_ALERTED(type, conn, service) (ALERT_FOR_PHONE(type) && !HID_CONNECTED(conn, service))
/* Clearing the alert */
#define IS_CLEARING_ALERT(type) (type == sink_gatt_ias_alert_none)

/****************************STATIC FUNCTIONS************************************/

/*******************************************************************************
NAME
    sinkGattIasClientGetImmAlertStopTimer
    
DESCRIPTION
    Get Immediate Alert StopTimer_s value.
    
RETURNS    
    uint16  
*/
uint16 sinkGattIasClientGetImmAlertStopTimer(void)
{
    uint16 ret_val = 0;
    sink_gatt_common_ias_readonly_config_def_t *ias_client_r_config = NULL;

    if (configManagerGetReadOnlyConfig(SINK_GATT_COMMON_IAS_READONLY_CONFIG_BLK_ID, (const void **)&ias_client_r_config))
    {
        ret_val = ias_client_r_config->ImmediateAlertStopTimeout_client_s;
        configManagerReleaseConfig(SINK_GATT_COMMON_IAS_READONLY_CONFIG_BLK_ID);
    }
    return ret_val;
}

bool sinkGattIasClientSetImmAlertStopTimer(uint16 timeout)
{
    sink_gatt_common_ias_readonly_config_def_t *ias_client_r_config = NULL;

    if (configManagerGetWriteableConfig(SINK_GATT_COMMON_IAS_READONLY_CONFIG_BLK_ID, (void **)&ias_client_r_config, 0))
    {
        ias_client_r_config->ImmediateAlertStopTimeout_client_s = timeout;
        configManagerUpdateWriteableConfig(SINK_GATT_COMMON_IAS_READONLY_CONFIG_BLK_ID);
        return TRUE;
    }

    return FALSE;
}

/*******************************************************************************
NAME
    gattIasClientFindConnection

DESCRIPTION
    Finds a client connection associated with a ias instance.

PARAMETERS
    giasc    The Imm Alert client instance pointer

RETURNS
    The client connection pointer associated with the ias instance. NULL if not found.

*/
static gatt_client_connection_t *gattIasClientFindConnection(const GIASC_T *giasc)
{
    uint16 index = 0;
    gatt_client_services_t *data = NULL;

    if (giasc == NULL)
    {
        return NULL;
    }

    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        data = gattClientGetServiceData(&GATT_CLIENT.connection);
        if (data && (data->iasc == giasc))
        {
            return &GATT_CLIENT.connection;
        }
    }

    return NULL;
}

/*******************************************************************************
NAME
    gattIasClientServiceInitialised

DESCRIPTION
    Called when the Imm Alert service is initialised.

PARAMETERS
    giasc    The Imm Alert client instance pointer

*/
static void gattIasClientServiceInitialised(const GIASC_T *giasc)
{
    gatt_client_connection_t *conn = gattIasClientFindConnection(giasc);

    if (conn != NULL)
    {
        gattClientDiscoveredServiceInitialised(conn);
    }
}

/****************************************************************************/
static uint16 sinkGattIasClientGetCachedLevel(uint16 cid, sink_gatt_ias_dev_type dev_type)
{
    uint16 index;
    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        if(GATT[index].cid == cid)
            return (dev_type == sink_gatt_ias_alert_remote) ? GATT_CLIENT.cache.iasc_remote_alert_level : \
                                    GATT_CLIENT.cache.iasc_phone_alert_level;
    }
    return IASC_INVALID_ALERT_LEVEL;
}

/*******************************************************************************
NAME
    gattIasClientSetCachedLevel

DESCRIPTION
    Sets the cached remote alert level

PARAMETERS
    level    The alert level to cache

*/
static void gattIasClientSetCachedLevel(uint16 cid, uint8 level, sink_gatt_ias_dev_type dev_type)
{
    uint16 index;
    GATT_IAS_CLIENT_DEBUG(("GATT Set IAS alert level cache=[%u]\n", level));
    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        /* First find the correct connection */
        if(cid == GATT[index].cid)
        {
            if(ALERT_FOR_REMOTE(dev_type))
            {
                /* Check if "find-my-phone" is not set to the same cid 
                  * Else during ias_init_cfm, there can be cofusion when both
                  * alert level set for the same cid */
                if(GATT_CLIENT.cache.iasc_phone_alert_level == IASC_INVALID_ALERT_LEVEL)
                {
                    GATT_CLIENT.cache.iasc_remote_alert_level = level;
                    break;
                }
            }
            else
            {
                if(GATT_CLIENT.cache.iasc_remote_alert_level == IASC_INVALID_ALERT_LEVEL)
                {
                    GATT_CLIENT.cache.iasc_phone_alert_level = level;
                    break;
                }
            }
        }
    }
}

/*******************************************************************************
NAME
    gattIasClientGetCachedLevelDevType

DESCRIPTION
    Gets the DevType and Level cached 

PARAMETERS
    cid    LE connection identifier
    dev_type To store the device type
    alert_level To store the cached alert level

*/
static void gattIasClientGetCachedLevelDevType(uint16 cid, sink_gatt_ias_dev_type *dev_type, gatt_imm_alert_set_level *alert_level)
{
    uint16 cached_level = IASC_INVALID_ALERT_LEVEL;
    if(dev_type == NULL || alert_level == NULL)
        return;
        
     if ((cached_level = sinkGattIasClientGetCachedLevel(cid, sink_gatt_ias_alert_remote)) != IASC_INVALID_ALERT_LEVEL)
     {
        *dev_type = sink_gatt_ias_alert_remote;
        *alert_level = (gatt_imm_alert_set_level)cached_level;
     }
     else if((cached_level = sinkGattIasClientGetCachedLevel(cid, sink_gatt_ias_alert_phone)) != IASC_INVALID_ALERT_LEVEL)
     {
        *dev_type = sink_gatt_ias_alert_phone;
        *alert_level = (gatt_imm_alert_set_level)cached_level;
     }
     else
     {
        *dev_type = sink_gatt_ias_alert_none;
        *alert_level = gatt_imm_alert_level_reserved;
     }
}

/*******************************************************************************
NAME
    gattIasClientInitCfm

DESCRIPTION
    Handle the GATT_IMM_ALERT_CLIENT_INIT_CFM message

PARAMETERS
    cfm    The GATT_IMM_ALERT_CLIENT_INIT_CFM message
*/
static void gattIasClientInitCfm(const GATT_IMM_ALERT_CLIENT_INIT_CFM_T *cfm)
{
    sink_gatt_ias_dev_type dev_type;
    gatt_imm_alert_set_level alert_level;
    GATT_IAS_CLIENT_DEBUG(("GATT_IMM_ALERT_CLIENT_INIT_CFM status[%u] cid=[0x%x] \n", cfm->status, cfm->cid));

    /* The service initialisation is complete */
    gattIasClientServiceInitialised(cfm->imm_alert_client);

    if ((cfm->status == gatt_imm_alert_client_status_success) &&
        (gattIasClientFindConnection(cfm->imm_alert_client) != NULL))
    {
        /* Initialisation - Set alert level */
       gattIasClientGetCachedLevelDevType(cfm->cid, &dev_type, &alert_level);

        if(alert_level == gatt_imm_alert_level_reserved)
        {
            GATT_IAS_CLIENT_DEBUG(("IAS Client initialization done \n"));
        }
        else  /* Case where remote client (eg phone) wants to alert remote peripheral (eg remote control) - Set alert level */
                /* Case where user triggers alert on Soundbar and remote is not connected*/
        {
            gatt_client_connection_t *connection = gattClientFindByCid(cfm->cid);
            gatt_client_services_t *services = gattClientGetServiceData(connection);

            if(services)
            {
                if(IS_REMOTE_TO_BE_ALERTED(dev_type, connection, services)
                    || IS_PHONE_TO_BE_ALERTED(dev_type, connection, services))
                {
                    GATT_IAS_CLIENT_DEBUG(("IAS Client Remote alert \n"));

                    /* The cached alert must be stopped after timeout */
                    MessageSendLater(&theSink.task, EventSysImmAlertTimeout, 0, D_SEC(sinkGattIasClientGetImmAlertStopTimer()));

                    /* Set alert level */
                    GattImmAlertClientSetAlertLevel(cfm->imm_alert_client, alert_level);
                }

                /* Just reset it */
                gattIasClientSetCachedLevel(cfm->cid, IASC_INVALID_ALERT_LEVEL, dev_type);
            }
        }
    }
}


/*******************************************************************************
NAME
    gattIasClientSetAlertCfm

DESCRIPTION
    Handle the GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM message

PARAMETERS
    cfm    The GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM message

*/
static void gattIasClientSetAlertCfm(const GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM_T *cfm)
{
    UNUSED(cfm);
    GATT_IAS_CLIENT_DEBUG(("GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM status[%u] \n",
                   cfm->status));
}


/****************************GLOBAL FUNCTIONS***********************************/

/****************************************************************************/
void sinkGattIasClientSetupAdvertisingFilter(void)
{
    GATT_IAS_CLIENT_DEBUG(("GattIas: Add IAS scan filter\n"));
    ConnectionBleAddAdvertisingReportFilter(ble_ad_type_more_uuid16, sizeof(ias_ble_advertising_filter), sizeof(ias_ble_advertising_filter), ias_ble_advertising_filter);
    ConnectionBleAddAdvertisingReportFilter(ble_ad_type_complete_uuid16, sizeof(ias_ble_advertising_filter), sizeof(ias_ble_advertising_filter), ias_ble_advertising_filter);
}

/****************************************************************************/
bool sinkGattIasClientAddService(uint16 cid, uint16 start, uint16 end)
{
    gatt_client_services_t *client_services = NULL;
    gatt_client_connection_t *connection = gattClientFindByCid(cid);
    uint16 *service = gattClientGetServicePtr(connection, sizeof(GIASC_T));

    GATT_IAS_CLIENT_DEBUG(("Add Imm Alert Client Service cid=[0x%x] \n", cid));

    if (service)
    {
        GATT_IMM_ALERT_CLIENT_INIT_PARAMS_T params;
        GIASC_T *giasc = NULL;

        client_services = gattClientGetServiceData(connection);
        client_services->iasc = (GIASC_T *)service;
        giasc = client_services->iasc;

        params.cid = cid;
        params.start_handle = start;
        params.end_handle = end;

        return (GattImmAlertClientInit(sinkGetBleTask(), giasc, &params, NULL));
    }

    return FALSE;
}

/****************************************************************************/
void sinkGattIasClientRemoveService(GIASC_T *giasc)
{
    GattImmAlertClientDestroy(giasc);
}

/****************************************************************************/
void sinkGattIasClientInit(void)
{
    uint16 index = 0;
    GATT_IAS_CLIENT_DEBUG(("GATT Immediate Alert Client Service initialised \n"));

    /* Initialise cached values */
    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
         /* Initialise cached values */
        GATT_CLIENT.cache.iasc_remote_alert_level = IASC_INVALID_ALERT_LEVEL;
        GATT_CLIENT.cache.iasc_phone_alert_level = IASC_INVALID_ALERT_LEVEL;
    }
}

/****************************************************************************/
ias_alert_status_t sinkGattIasClientSetAlert(gatt_imm_alert_set_level alert_level, sink_gatt_ias_dev_type dev_type)
{
    gatt_client_services_t *client_services = NULL;
    gatt_client_connection_t *connection = NULL;
    bool alert_sent = FALSE;
    uint16 index = 0;

    GATT_IAS_CLIENT_DEBUG(("Set Alert level on remote device \n"));

     if((dev_type == sink_gatt_ias_alert_remote) && (!sinkGattHidClientEnabled()))
        return ias_alert_not_supported;

    if(alert_level > alert_level_high)
        return ias_alert_param_invalid;

    /*Check if the remote device is connected*/
    for (index = 0; index < MAX_BLE_CONNECTIONS; index++)
    {
        if (GATT[index].cid)
        {
            connection = gattClientFindByCid(GATT[index].cid);
            /* Get Client services */
            client_services = gattClientGetServiceData(connection);

            if ((client_services != NULL) && (client_services->iasc))
            {
                /* Alert the phone/remote */
                if(IS_CLEARING_ALERT(dev_type) || IS_REMOTE_TO_BE_ALERTED(dev_type, connection, client_services)
                    || IS_PHONE_TO_BE_ALERTED(dev_type, connection, client_services))
                {
                    MessageCancelAll(&theSink.task, EventSysImmAlertTimeout);

                    GATT_IAS_CLIENT_DEBUG(("IASC Set Alert \n"));
                    /* Set Alert Level */
                    GattImmAlertClientSetAlertLevel((GIASC_T *)client_services->iasc, alert_level);

                    alert_sent = TRUE;
                    /*Set Timeout Event*/
                    if (alert_level != alert_level_no)
                    {
                        MessageSendLater(&theSink.task, EventSysImmAlertTimeout, 0, D_SEC(sinkGattIasClientGetImmAlertStopTimer()));
                    }
                }
            }
        }
    }

    /* In case the remote device is not connected Switch to appropriate mode if the device type is known */
    if ((alert_sent == FALSE) && (dev_type != sink_gatt_ias_alert_none))
    {
        /* Check if total connection slots have been used, if so then there is no
         * way to allow alert the remote server, so give an error beep and drop the request */
         if(!gattCommonHasMaxConnections())
         {
             /* Store alert level: This will be used init_cfm */
             gattIasClientSetCachedLevel(0, alert_level, dev_type);
         }
         else
         {
            /* Already the system is connected to its maximum capacity, so ignore the request ImmAlertErrorBeep */
            MessageCancelAll(&theSink.task, EventSysImmAlertErrorBeep);
            MessageSend(&theSink.task, EventSysImmAlertErrorBeep, 0);
            return ias_alert_not_supported;
         }
    }
    return ias_alert_success;
}

/******************************************************************************/
void sinkGattIasClientMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);

    switch(id)
    {
        case GATT_IMM_ALERT_CLIENT_INIT_CFM:
        {
            gattIasClientInitCfm((const GATT_IMM_ALERT_CLIENT_INIT_CFM_T*)message);
        }
        break;
        case GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM:
        {
            gattIasClientSetAlertCfm((const GATT_IMM_ALERT_CLIENT_SET_ALERT_CFM_T*)message);
        }
        break;
        default:
        {
            GATT_IAS_CLIENT_DEBUG(("Unhandled IAS Client msg[%x]\n", id));
        }
        break;
    }
}



#endif /* GATT_IAS_CLIENT */

