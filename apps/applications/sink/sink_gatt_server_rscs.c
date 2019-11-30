/****************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_rscs.c

DESCRIPTION
    Routines to handle messages sent from the GATT Running Speed and Cadence Sensor Task.
*/

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <util.h>
#include <stdlib.h>

#ifdef ACTIVITY_MONITORING
/* Library headers */
#include <gatt_running_speed_cadence_server.h>

/* Application headers */
#include "sink_private_data.h"
#include "sink_gatt_common.h"
#include "sink_debug.h"
#include "sink_gatt_db.h"
#include "sink_ble.h"
#include "sink_gatt_server.h"
#include "sink_gatt_server_rscs.h"
#include "sink_development.h"
#include "sink_sport_health.h"

#ifdef GATT_RSCS_SERVER

#ifdef DEBUG_GATT_RSCS_SERVER
#define GATT_RSCS_SERVER_INFO(x) DEBUG(x)
#define GATT_RSCS_SERVER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_RSCS_SERVER_INFO(x)
#define GATT_RSCS_SERVER_ERROR(x)
#endif

/*
    The running speed and cadence measurement notification size.
       --------------------------------------------------------------------------------------------------------------------
       | Flags | Instantaneous Speed | Instantaneous Cadence | Instantaneous Stride Length | Total Distance |-----------  
       --------------------------------------------------------------------------------------------------------------------
       Flags:                       1 byte
       Instantaneous Speed:         2 byte
       Instantaneous Cadence:       1 byte
       Instantaneous Stride Length: 2 byte (optional)
       Total Distance:              4 byte (optional)
*/
#define RUNNING_SPEED_AND_CADENCE_NOTIFICATION_SIZE    (10)//3 + INST_STRIDE_LENGTH_PRESENT + TOTAL_DISTANCE_PRESENT >> 1

/*
    The  running speed and cadence step extension notification size.
       --------------------------------------------------------------------------------------------------------------------
       | Flags | Step Count | Context | Total Distance | Norm Acceleration |  Energy Used |-----------
       --------------------------------------------------------------------------------------------------------------------
       Flags:                       1 byte
       Step count:                  2 byte (optional)
       Context:                     1 byte (optional)
       Total Distance:              2 byte (optional)
       Norm Acc:                    1 byte (optional)
       Energy Used:                 2 byte (optional)
*/
#define RUNNING_SPEED_AND_CADENCE_EXTENSION_NOTIFICATION_SIZE    (4)//3 + INST_STRIDE_LENGTH_PRESENT + TOTAL_DISTANCE_PRESENT >> 1

#define RSC_UPDATE_REQD(conex)  (((conex).client_config.running_speed_and_cadence) & 1)

/* Dummy data for use in measurement notifications. */
#define DUMMY_SPEED     0x06
#define DUMMY_CADENCE   0x40

/*******************************************************************************
NAME
    handleReadRSCExtensionClientConfig

DESCRIPTION
    Handle when a GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND message is received.

PARAMETERS
    ind Pointer to a GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleReadRSCExtensionClientConfig(const GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);
    uint16 client_config = 0;

    /* Return the current value of the client configuration descriptor for the device */
    GATT_RSCS_SERVER_INFO(("GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND rscs=[0x%p] cid=[0x%x]\n", (void *)ind->rscs, ind->cid));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->rscs == GATT_SERVER.rscs_server)
            client_config = GATT_SERVER.client_config.running_speed_and_cadence;
    }

    GattRSCExtensionServerReadClientConfigResponse(ind->rscs, ind->cid, client_config);
    GATT_RSCS_SERVER_INFO(("  client_config=[0x%x]\n", client_config));
    UNUSED(ind);
}

/*******************************************************************************
NAME
    handleWriteRSCExtensionClientConfig

DESCRIPTION
    Handle when a GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND message is received.

PARAMETERS
    ind Pointer to a GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleWriteRSCExtensionClientConfig(const GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /*
     * Check whether the remote device has enabled or disabled
     * notifications for the Heart rate Measurement characteristic. This value will need
     * to be stored as device attributes so they are persistent.
    */
    GATT_RSCS_SERVER_INFO(("GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND rscs=[0x%p] cid=[0x%x] value[0x%x]\n",
        (void *)ind->rscs, ind->cid, ind->config_value));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->rscs == GATT_SERVER.rscs_server)
        {
            GATT_SERVER.client_config.running_speed_and_cadence = ind->config_value;
            GATT_RSCS_SERVER_INFO((" client_config[0x%x]\n", GATT_SERVER.client_config.running_speed_and_cadence));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_running_speed_and_cadence);

            if(! RSC_UPDATE_REQD(GATT_SERVER))
            {
                /* Disable the RSC measurement notifications */
                MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_RSC_READ_TIMER );
            }
            else
            {
                sinkBleNotifyRSCMeasurements();
            }
        }
    }
    UNUSED(ind);
}

/*******************************************************************************
NAME
    handleReadRSCMeasurementClientConfig

DESCRIPTION
    Handle when a GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message is received.

PARAMETERS
    ind Pointer to a GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleReadRSCMeasurementClientConfig(const GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);
    uint16 client_config = 0;

    /* Return the current value of the client configuration descriptor for the device */
    GATT_RSCS_SERVER_INFO(("GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND rscs=[0x%p] cid=[0x%x]\n", (void *)ind->rscs, ind->cid));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->rscs == GATT_SERVER.rscs_server)
            client_config = GATT_SERVER.client_config.running_speed_and_cadence;
    }

    GattRSCServerReadClientConfigResponse(ind->rscs, ind->cid, client_config);
    GATT_RSCS_SERVER_INFO(("  client_config=[0x%x]\n", client_config));
    UNUSED(ind);
}

/*******************************************************************************
NAME
    handleWriteRSCMeasurementClientConfig

DESCRIPTION
    Handle when a GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND message is received.

PARAMETERS
    ind Pointer to a GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleWriteRSCMeasurementClientConfig(const GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* 
     * Check whether the remote device has enabled or disabled 
     * notifications for the Heart rate Measurement characteristic. This value will need
     * to be stored as device attributes so they are persistent.
    */
    GATT_RSCS_SERVER_INFO(("GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND rscs=[0x%p] cid=[0x%x] value[0x%x]\n", 
        (void *)ind->rscs, ind->cid, ind->config_value));

    if (index != GATT_INVALID_INDEX)
    {      
        if(ind->rscs == GATT_SERVER.rscs_server)
        {
            GATT_SERVER.client_config.running_speed_and_cadence = ind->config_value;
            GATT_RSCS_SERVER_INFO((" Sensor client_config[0x%x]\n", GATT_SERVER.client_config.running_speed_and_cadence));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_running_speed_and_cadence);

            if(! RSC_UPDATE_REQD(GATT_SERVER))
            {
                /* Disable the RSC measurement notifications */
                MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_RSC_READ_TIMER );
            }
            else
            {
                sinkBleNotifyRSCMeasurements();
            }
        }
    }
    UNUSED(ind);
}

/******************************************************************************/
static void notifyRSCMeasurements(void)
{
    uint8 rsc_meas_data [RUNNING_SPEED_AND_CADENCE_NOTIFICATION_SIZE];
    uint16 index;
    /* Actual RSC measurements received from RSC sensor,  The mechanism of retrieving
       RSC measurements are to be implemented by the customer based on their requirements.
    */
    uint16  rsc_meas_len = getRSCMeasReading(rsc_meas_data, RUNNING_SPEED_AND_CADENCE_NOTIFICATION_SIZE);
    if (rsc_meas_len == 0) return;
    for( index = 0;index < MAX_BLE_CONNECTIONS;index++ )
    {
        if( RSC_UPDATE_REQD(GATT_SERVER))
        {
            /* Send the Running Speed and Cadence Measurement notification. */
            GattRSCServerSendNotification( GATT_SERVER.rscs_server, GATT[index].cid, (uint16)rsc_meas_len, rsc_meas_data);
            GATT_RSCS_SERVER_INFO(("GATT Running Speed and Cadence Measurement %d cadence\n", rsc_meas_data[1] ));
        }
    }
}

/******************************************************************************/
static void notifyRSCExtensions(void)
{
    uint8 rsc_meas_data [RUNNING_SPEED_AND_CADENCE_EXTENSION_NOTIFICATION_SIZE];
    uint16 index;
    /* Actual RSC measurements received from RSC sensor,  The mechanism of retrieving
       RSC measurements are to be implemented by the customer based on their requirements.
    */
    uint16 rsc_meas_len = getRSCExtensionReading(rsc_meas_data, RUNNING_SPEED_AND_CADENCE_EXTENSION_NOTIFICATION_SIZE);
    if (rsc_meas_len == 0) return;
    for( index = 0;index < MAX_BLE_CONNECTIONS;index++ )
    {
        if( RSC_UPDATE_REQD(GATT_SERVER))
        {
            /* Send the Running Speed and Cadence Extension notification. */
            GattRSCServerSendNotificationExtension( GATT_SERVER.rscs_server, GATT[index].cid, (uint16)rsc_meas_len, rsc_meas_data);
            GATT_RSCS_SERVER_INFO(("GATT Running Speed and Cadence Extension %d steps\n", rsc_meas_data[1] ));
        }
    }
}

/*******************************************************************************/
void sinkBleNotifyRSCMeasurements(void)
{
    notifyRSCMeasurements();

    #ifdef ACTIVITY_MONITORING
        notifyRSCExtensions();
    #endif

    MessageSendLater( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_RSC_READ_TIMER, 0, GATT_SERVER_RSC_UPDATE_TIME );

}

/*******************************************************************************/
bool sinkGattRSCServerInitialise(uint16 **ptr)
{
    if (GattRSCServerInit( sinkGetBleTask(),  (GRSCS_T *)*ptr, HANDLE_RUNNING_SPEED_AND_CADENCE_SERVICE,
                                     HANDLE_RUNNING_SPEED_AND_CADENCE_SERVICE_END))
    {
        GATT_RSCS_SERVER_INFO(("GATT Running Speed and Cadence Sensor initialized\n"));
        gattServerSetServicePtr(ptr, gatt_server_service_rscs);
        /* The size of RSCS is also calculated and memory is allocated.
         * So advance the ptr so that the next Server while initializing.
         * shall not over write the same memory */
        *ptr += ADJ_GATT_STRUCT_OFFSET(GRSCS_T);
        return TRUE;
    }
    
    /* Running Speed and Cadence Service library initialization failed */
    GATT_RSCS_SERVER_INFO(("GATT Running Speed and Cadence Sensor init failed \n"));
    return FALSE;
}

/******************************************************************************/
void sinkGattRSCServerMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
        case GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND:
        {
            handleReadRSCMeasurementClientConfig((const GATT_RSC_SERVER_READ_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            handleWriteRSCMeasurementClientConfig((const GATT_RSC_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND:
        {
            handleReadRSCExtensionClientConfig((const GATT_RSC_EXTENSION_SERVER_READ_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            handleWriteRSCExtensionClientConfig((const GATT_RSC_EXTENSION_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        default:
        {
            /* Unhandled messages are not critical errors */
            GATT_RSCS_SERVER_ERROR(("GATT Unhandled msg id[%x]\n", id));
        }
    }
}
#endif

#endif /* GATT_RSCS_SERVER */
