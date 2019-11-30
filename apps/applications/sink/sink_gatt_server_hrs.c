/****************************************************************************
Copyright (c) 2015 Qualcomm Technologies International, Ltd.


FILE NAME
    sink_gatt_server_hrs.c

DESCRIPTION
    Routines to handle messages sent from the GATT Heart Rate Sensor Task.
*/

#include <stdlib.h>

/* Firmware headers */
#include <csrtypes.h>
#include <message.h>
#include <util.h>

/* Library headers */
#include <gatt_heart_rate_server.h>

/* Application headers */
#include "sink_gatt_common.h"
#include "sink_gatt_db.h"
#include "sink_ble.h"
#include "sink_gatt_server.h"
#include "sink_gatt_server_hrs.h"
#include "sink_development.h"
#include "sink_events.h"
#include "sink_main_task.h"

#ifdef GATT_HRS_SERVER

#ifdef DEBUG_GATT_HRS_SERVER
#define GATT_HRS_SERVER_INFO(x) DEBUG(x)
#define GATT_HRS_SERVER_ERROR(x) DEBUG(x) TOLERATED_ERROR(x)
#else
#define GATT_HRS_SERVER_INFO(x)
#define GATT_HRS_SERVER_ERROR(x)
#endif

/*
    The simulated heart rate measurement notification size.
       -------------------------------------
       | Flags    |   HR meas Value |  RR Interval  
       ------------------------------------
       Flags:            1 byte
       HR Meas value:    1 byte
       RR interval:      2 bytes
*/
#define HEART_RATE_NOTIFICATION_SIZE    4

#define HR_UPDATE_REQD(conex)  (((conex).client_config.heart_rate) & 1)

/*----------------------------------------------------------------------------*
 *  NAME
 *      simulateHRMeasReading
 *
 *  DESCRIPTION
 *      This function formulates HR Measurement data in a format
 *      given by HR service specification. It simulates static
 *      HR measurement reading with RR interval and no enerygy expended field.
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/

static void simulateHRMeasReading(uint8 *p_hr_meas, uint8 *p_length, uint8 sensor_status)
{
    uint8  *p_hr_meas_flags = NULL;

    p_hr_meas_flags = &p_hr_meas[(*p_length)++];

    /*
     *  --------------------------------------
     *  | Flags      |   HR meas Value |  RR Intervals -----------  
     *  --------------------------------------
     *      Flags field: 
     *              Bit 0: Heart Rate Value Format bit, 0 - UINT8 Format, 1 - UINT16 Format
     *              Bit 1: Sensor Contact Status bit, 0 - No or Poor Contact, 1 - In contact
     *              Bit 2: Sensor Contact Support bit, 0 - Sensor Contact feature is supported, 1 - Sensor Contact feature not supported
     *              Bit 3: Energy Expended Status bit, 0 - Energy Expended field not present, 1 - Energy Expended field is present
     *              Bit 4: RR-Interval bit, 0 - RR-Interval values are not present, 1 - RR-Interval values present
     *              Bits 5-7: RFU (Resereved for future use must be set to 0)
     *
     *      Heart Rate Measurement Value Field:
     *              <= 255 bpm if Heart Rate Value Format bit set to 0, >255 bpm Heart Rate Value Format bit set to 1
     *      
     *      RR-Interval Field:
     *              One or mor RR-Interval values if  RR-Interval bit set to 1.
     */
     
    *p_hr_meas_flags = SENSOR_MEASUREVAL_FORMAT_UINT8 |
                       sensor_status |
                       RR_INTERVAL_PRESENT;

    /* 78 +/- 32 bpm */
    p_hr_meas[(*p_length)++] = HEART_RATE_IN_BPM + (32 - (int32)UtilRandom() % 16);   

    /* 480 +/- 32 */
    p_hr_meas[(*p_length)++] = LE8_L(RR_INTERVAL_IN_MS) + (32 - (int32)UtilRandom() % 16);
    p_hr_meas[(*p_length)++] = LE8_H(RR_INTERVAL_IN_MS);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      sinkBleStopHrSimulation
 *
 *  DESCRIPTION
 *      This function sends handles EventSysBleHrSensorNotInContact and sends final
 *      HR Measurement data with Sensor Contact Bit set to "NOT IN CONTACT"
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
static void sinkBleStopHrSimulation(void)
{
    uint16 index;
    
    for( index = 0;index < MAX_BLE_CONNECTIONS;index++ )
    {
        /* Check if the notifications are enabled */
        if( HR_UPDATE_REQD(GATT_SERVER) )
        {
            uint8 *p_hr_meas = malloc(HEART_RATE_NOTIFICATION_SIZE);
           
            /* Cancel the next scheduled Heart rate measurement */
            MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_HR_READ_TIMER );

            if(p_hr_meas != NULL)
            {
                uint8  hr_meas_len  = 0;

                 /* Read simulated HR measurement and send dummy data */
                simulateHRMeasReading(p_hr_meas, &hr_meas_len, SENSOR_NOT_IN_CONTACT);
                 
                /* Send the final Heart Rate Measurement notification with sensor in contact bit set to 0 */
                GattHrServerSendNotification( GATT_SERVER.hrs_server, 
                                          GATT[index].cid, hr_meas_len, p_hr_meas);
                GATT_HRS_SERVER_INFO(("GATT sinkBleStopHrSimulation() Heart Rate Measurement %d bpm\n", p_hr_meas[1] ));
                /* Free the memory */
                free(p_hr_meas);
            }
            else
            {
                /* Could not allocate memory */
                GATT_HRS_SERVER_INFO(("GATT sinkBleStopHrSimulation() Malloc Failed\n"));
            }
        }   
    }
}

/*******************************************************************************
NAME
    handleReadHrMeasurementClientConfig

DESCRIPTION
    Handle when a GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message is recieved.

PARAMETERS
    ind Pointer to a GATT_HR_SERVER_READ_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleReadHrMeasurementClientConfig(const GATT_HR_SERVER_READ_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);
    uint16 client_config = 0;

    /* Return the current value of the client configuration descriptor for the device */
    GATT_HRS_SERVER_INFO(("GATT_HR_SERVER_READ_CLIENT_CONFIG_IND hrs=[0x%p] cid=[0x%x]\n", (void *)ind->hr_sensor, ind->cid));

    if (index != GATT_INVALID_INDEX)
    {
        if(ind->hr_sensor == GATT_SERVER.hrs_server)
            client_config = GATT_SERVER.client_config.heart_rate;
    }

    GattHrServerReadClientConfigResponse(ind->hr_sensor, ind->cid, client_config);
    GATT_HRS_SERVER_INFO(("  client_config=[0x%x]\n", client_config));
}

/*******************************************************************************
NAME
    handleWriteHrMeasurementClientConfig

DESCRIPTION
    Handle when a GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND message is recieved.

PARAMETERS
    ind Pointer to a GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND message.

RETURNS
    void
*/
static void handleWriteHrMeasurementClientConfig(const GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND_T * ind)
{
    uint16 index = gattCommonConnectionFindByCid(ind->cid);

    /* 
     * Check whether the remote device has enabled or disabled 
     * notifications for the Heart rate Measurement characteristic. This value will need
     * to be stored as device attributes so they are persistent.
    */
    GATT_HRS_SERVER_INFO(("GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND hrs=[0x%p] cid=[0x%x] value[0x%x]\n", 
        (void *)ind->hr_sensor, ind->cid, ind->config_value));

    if (index != GATT_INVALID_INDEX)
    {      
        if(ind->hr_sensor == GATT_SERVER.hrs_server)
        {
            GATT_SERVER.client_config.heart_rate = ind->config_value;
            GATT_HRS_SERVER_INFO((" Heart rate Sensor client_config[0x%x]\n", GATT_SERVER.client_config.heart_rate));
            gattServerStoreConfigAttributes(ind->cid, gatt_attr_service_heart_rate);

            if(! HR_UPDATE_REQD(GATT_SERVER))
            {
                /* Disable the HR measurement notifications */
                MessageCancelFirst( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_HR_READ_TIMER );
            }
            else
            {
                sinkBleNotifyHrMeasurements();
            }
        }
    }

}

/******************************************************************************/
void sinkBleHRSensorInContact(void)
{
    sinkBleNotifyHrMeasurements();            
    /* Start a 10 second simulation timer after which the heart rate measurement notifications shall be stopped */
    MessageSendLater( &theSink.task, EventSysBleHrSensorNotInContact , 0, STOP_HR_NOTIFICATION_TIME);
}

/******************************************************************************/
void sinkBleHRSensorNotInContact(void)
{
    /* Sensor not in contact stop sending notifications */
    sinkBleStopHrSimulation();
}

/******************************************************************************/
void sinkBleNotifyHrMeasurements(void)
{
    uint8 *p_hr_meas_data = NULL;
    uint16 index;

    for( index = 0;index < MAX_BLE_CONNECTIONS;index++ )
    {
        if(HR_UPDATE_REQD(GATT_SERVER))
        {
            /* Actual HR measurements received from HR sensor,  The mechanism of retreiving
                HR measurements are to be implemented by the customer based on their requirements.
                In which case simulateHRMeasReading() function needs to be removed.
            */
            p_hr_meas_data = malloc(HEART_RATE_NOTIFICATION_SIZE);

            if(p_hr_meas_data)
            {
                uint8   hr_meas_len = 0;
                #ifdef ACTIVITY_MONITORING
                /* Read HR measurement and send dummy data */
                getHRMeasReading(p_hr_meas_data, &hr_meas_len, SENSOR_IN_CONTACT);
                #else
                /* Read simulated HR measurement and send dummy data */
                simulateHRMeasReading(p_hr_meas_data, &hr_meas_len, SENSOR_IN_CONTACT);
                #endif
                /* Send the Heart Rate Measurement notification. */
                GattHrServerSendNotification( GATT_SERVER.hrs_server, 
                                          GATT[index].cid, (uint16)hr_meas_len, p_hr_meas_data);
                GATT_HRS_SERVER_INFO(("GATT Heart Rate Measurement %d bpm\n", p_hr_meas_data[1] ));
                /* Free the memory */
                free(p_hr_meas_data);
            }
            else
            {
                /* Could not allocate memory */
                GATT_HRS_SERVER_INFO(("GATT Malloc Failed\n"));
            }
        }
    }
    MessageSendLater( sinkGetBleTask(), BLE_INTERNAL_MESSAGE_HR_READ_TIMER, 0, GATT_SERVER_HR_UPDATE_TIME );
}

/*******************************************************************************/
bool sinkGattHeartRateServerInitialise(uint16 **ptr)
{
    if (GattHrServerInit( sinkGetBleTask(),  (GHRS_T *)*ptr, HANDLE_HEART_RATE_SERVICE,
                                     HANDLE_HEART_RATE_SERVICE_END))
    {
        GATT_HRS_SERVER_INFO(("GATT Heart Rate Sensor initialised\n"));
        gattServerSetServicePtr(ptr, gatt_server_service_hrs);
        /* The size of HRS is also calculated and memory is alocated.
         * So advance the ptr so that the next Server while initializing.
         * shall not over write the same memory */
        *ptr += ADJ_GATT_STRUCT_OFFSET(GHRS_T);
        return TRUE;
    }
    
    /* Heart Rate Service library initialization failed */
    GATT_HRS_SERVER_INFO(("GATT Heart Rate Sensor init failed \n"));
    return FALSE;
}

/******************************************************************************/
void sinkGattHeartRateServerMsgHandler(Task task, MessageId id, Message message)
{
    UNUSED(task);
    switch(id)
    {
        case GATT_HR_SERVER_READ_CLIENT_CONFIG_IND:
        {
            handleReadHrMeasurementClientConfig((const GATT_HR_SERVER_READ_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        case GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND:
        {
            handleWriteHrMeasurementClientConfig((const GATT_HR_SERVER_WRITE_CLIENT_CONFIG_IND_T*)message);
        }
        break;
        default:
        {
            /* Un-handled messages are not critical errors */
            GATT_HRS_SERVER_ERROR(("GATT Un-handled msg id[%x]\n", id));
        }
    }
}

#endif /* GATT_HRS_SERVER */

bool sinkGattHeartRateServiceEnabled(void)
{
#ifdef GATT_HRS_SERVER
    return TRUE;
#else
    return FALSE;
#endif
}
