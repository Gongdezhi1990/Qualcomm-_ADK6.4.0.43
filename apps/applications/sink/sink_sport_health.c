/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sport_health_app.c

DESCRIPTION
    Sample application for using sports and health algorithms.
*/
#ifdef ACTIVITY_MONITORING
#include "sink_sport_health.h"
#include "sport_health_driver.h"
#include "sport_health_hub.h"
#include "sport_health_algorithms.h"
#include "sport_health_logging.h"
#include <stdio.h>
#include <stdlib.h>
#include <vmtypes.h>
#include <os.h>
#include <panic.h>
#include <operator.h>
#include "hydra_log.h"
#include <util.h>
#include "sink_gatt_server_rscs.h"
#include "sink_gatt_server_hrs.h"
#include "sink_tap_ui.h"
#include "sink_tones.h"
#include "sink_configmanager.h"
#include "sink_activity_monitoring_config_def.h"

/** sport health app control structure */
sport_health_app_t sh_app;

/** @brief: Algorithm information data structure used while accessing
 * algorithm results.
 */
static sh_vm_algo_info_t algo_info = 0;
static uint32 algo_enable_default = ALGO_EN_STEP | ALGO_EN_DISTANCE | ALGO_EN_HR | ALGO_EN_TAP | ALGO_EN_TAP_RATES;

#define INTERRUPT_MASK(pio)      (1ul << (pio))
/* Set up a table to identify which switch matches a bitmask */
static uint32 switch_mask_table[2];

/*! Structure to hold information about the interrupts.
 * Only hold the last state of the PIOs at present, in
 * interruptState variable
 * */
static struct
{
    uint32      interruptState;
} interruptData;

/**
 * @brief This function is used to construct an uint32 value from an uint8 array
 * starting from a given index.
 * @param index The index in the given array to start with.
 * @param data  The pointer to the array that stores the data.
 */
static uint32 getUint32(uint8 index, uint8 *data)
{
    uint32 byte3 = data[index + 3];
    uint32 byte2 = data[index + 2];
    uint32 byte1 = data[index + 1];
    uint32 value = data[index];
    value |= ((byte3 << 24) | (byte2 << 16) | (byte1 << 8));
    return value;
}


/**
 * @brief This function is used to send the configuration required
 *        to enable the algorithms supported as part of sports and health.
 * @param algo_enable Bitfield which contains all the algorithms to be enabled
 */
void sh_seq_enable(uint32 algo_enable)
{
    sh_app.algo_enable = algo_enable;

    MAKE_SHAPP_MESSAGE(SH_VM_ENABLE_REQ);
    msg->enables     = algo_enable;
    msg->power_mode  = SH_VM_PWR_DEFAULT;
    msg->p_algo_info = &algo_info;
    MessageSend(sh_app.hub_task, SH_VM_ENABLE_REQ, msg);
}

/**
 * @brief This function is used to set the timing information based on
 *        bluetooth interval.
 * @param bt_interval_ms Bluetooth time interval in ms.
 * @param service_active Type of bluetooth service
 */
static void set_timing_info(uint32 bt_interval_ms, bool service_active)
{
    MAKE_SHAPP_MESSAGE(SH_VM_SET_TIMING_INFO_REQ);
    msg->bt_interval_ms = bt_interval_ms;
    msg->bt_mode = service_active ? SH_VM_BT_MODE_ACTIVE : SH_VM_BT_MODE_SNIFF;

    MessageSend(sh_app.hub_task, SH_VM_SET_TIMING_INFO_REQ, msg);
}

/**
 * @brief Message handler for SH_VM_SET_TIMING_INFO_CFM. This function
 *        sets the mode of application and creates the bitfield containing
 *        all the algorithms to be enabled.
 * @param pMessage
 */
static void sh_vm_set_timing_info_cfm(Message pMessage)
{
    /** not checking if driver reported succesful YET */
    UNUSED(pMessage);
    {
        sh_app.mode = SH_APP_TIMING_SET;
    }
    sh_seq_enable(algo_enable_default);
}

/**
 * @brief Test function to show the use of algorithm's result
 *        accessor functions. This function is a message handler
 *        for SH_VM_RESULTS_IND.
 *        This function on Crescendo displays the results on LCD
 *        display.
 */
static void sh_vm_results_ind_test(void)
{
    /** Test logic to validate the delete functionality */
    /*sh_seq_enable(ALGO_EN_TAP);*/
     #ifndef SPORTS_HEALTH_LOGGING_DISABLE
     uint32 dist_cm = 0;
     uint16 step_cnt = 0;
     uint16  hr_cnt = 0;
     sh_tap_type_t tap_cnt = 0;


     if(sh_app.algo_enable & ALGO_EN_STEP)
     {
         step_cnt = SportHealthAlgorithmsGetStepNumber(algo_info.step_info);
     }
     if(sh_app.algo_enable & ALGO_EN_TAP)
     {
         tap_cnt = SportHealthAlgorithmsGetTapType(algo_info.tap_info);
     }
     if(sh_app.algo_enable & ALGO_EN_DISTANCE)
     {
         dist_cm = SportHealthAlgorithmsGetDistance(algo_info.distance_info);
     }
     if(sh_app.algo_enable & ALGO_EN_HR)
     {
         hr_cnt = SportHealthAlgorithmsGetHeartRate(algo_info.heart_rate_info);
     }
     #endif
     #ifndef SPORTS_HEALTH_LOGGING_DISABLE
     L0_DBG_MSG1("Step count: %5d", step_cnt);
     L0_DBG_MSG1("Tap  count: %d",  tap_cnt);
     L0_DBG_MSG1("Dist count: %5d", dist_cm);
     L0_DBG_MSG1("HR   count: %2d", hr_cnt);
     #endif
     /* Code demonstrating the use of tap detects for user interface */
     if(sh_app.algo_enable & ALGO_EN_TAP)
     {
         sh_tap_type_t tap_type = SportHealthAlgorithmsGetTapType(algo_info.tap_info);
         #ifdef TEST_TAP
         /* Tap test code to calibrate the tap latency. Based on
          * detected tap type, a corresponding single/double tone
          * is played on headphones.
         */
         if(tap_type == SH_TAP_TYPE_SINGLE_TAP)
         {
             /* Play a single tone */
             TonesPlayTone(SINGLE_TONE, FALSE);
         }
         if(tap_type == SH_TAP_TYPE_DOUBLE_TAP)
         {
             /* Play a double tone */
             TonesPlayTone(DOUBLE_TONE, FALSE);
         }
         #else
         tapUiTaskData * theTapUiTask = sinkGetTapUiTask();
         /* Send the tap type to tap UI task */
         MessageSend(&theTapUiTask->task, tap_type, 0);
         #endif
     }
}

static void sh_vm_enable_cfm(Message pMessage)
{
    /** not checking if driver reported succesful YET */
    UNUSED(pMessage);
    {
        sh_app.enabled = TRUE;
        sh_app.mode = SH_APP_ACTIVE;
    }
}

/**
 * @brief Message handler for sports and health app task
 * @param pTask Task ID
 * @param pId Message ID
 * @param pMessage Message content
 */
static void sport_health_app_message_handler ( Task pTask, MessageId pId, Message pMessage )
{
    UNUSED(pTask);

    switch ((sh_vm_message_t)pId)
    {
        case SH_VM_SET_TIMING_INFO_CFM:

            sh_vm_set_timing_info_cfm(pMessage);
            break;

        case SH_VM_ENABLE_CFM:
            sh_vm_enable_cfm(pMessage);
            break;

        case SH_VM_RESULTS_IND:
            sh_vm_results_ind_test();
            break;

        case SH_VM_IN_OUT_IND:
            break;

        default:
            break;

    }
}
/**
 * @brief This function is a bitserial initialise function in which
 *        bitserial I2C PIOs and corresponding functionalities are
 *        configured.
 * @param pio: PIO to be configured
 * @param func: Corresponding function to be configured
 * @return Initialisation result: TRUE/FALSE
 */
static bool sport_health_set_i2c_pio(uint16 pio, pin_function_id func)
{
    uint32 status;
    uint16 bank = PBANK(pio);
    uint32 mask = POFFM(pio);

    status = PioSetMapPins32Bank(bank, mask, 0);
    if (status)
    {
        SINK_SPORT_HEALTH_DEBUG_INFO("Bitserial I2C pio setup failed (PioSetMapPins32Bank)");
        return FALSE;
    }
    status = PioSetFunction(pio, func);
    if (status == FALSE)
    {
        SINK_SPORT_HEALTH_DEBUG_INFO("Bitserial I2C pio setup failed (PioSetFunction)");
        return FALSE;
    }
    status = PioSetDir32Bank(bank, mask, 0);
    if (status)
    {
        SINK_SPORT_HEALTH_DEBUG_INFO("Bitserial I2C pio setup failed (PioSetDir32Bank)");
        return FALSE;
    }
    status = PioSet32Bank(bank, mask, mask);
    if (status)
    {
        SINK_SPORT_HEALTH_DEBUG_INFO("Bitserial I2C pio setup failed (PioSet32Bank)");
        return FALSE;
    }
    status = PioSetStrongBias32Bank(bank, mask, mask);
    if (status)
    {
        SINK_SPORT_HEALTH_DEBUG_INFO("Bitserial I2C pio setup failed (PioSetStrongBias32Bank)");
        return FALSE;
    }
    return TRUE;
}
/**
 * @brief Function to get I2C PIOs for BitSerial.
 * @param sensor_configuration the configuration data for sensors
 * @return: bool: False if read fails/True on success read
*/
static bool sport_health_config_i2c_pio(sensor_config_data_t* sensor_configuration)
{
    activity_monitoring_config_def_t *p_i2c_pio_config_data = NULL;
    bool ret_val = FALSE;

    if(configManagerGetReadOnlyConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID,
                         (const void **)&p_i2c_pio_config_data) > 0)
    {
       sensor_configuration->interrupt_pio = p_i2c_pio_config_data->interrupt_line_pio;
       /* Configure I2C PIOs for each BitSerial functionality */
       ret_val = sport_health_set_i2c_pio(p_i2c_pio_config_data->i2c_scl_pio, BITSERIAL_0_CLOCK_OUT);
       if(ret_val)
       {
           ret_val = sport_health_set_i2c_pio(p_i2c_pio_config_data->i2c_scl_pio, BITSERIAL_0_CLOCK_IN);
       }
       if(ret_val)
       {
           ret_val = sport_health_set_i2c_pio(p_i2c_pio_config_data->i2c_sda_pio, BITSERIAL_0_DATA_OUT);
       }
       if(ret_val)
       {
           ret_val = sport_health_set_i2c_pio(p_i2c_pio_config_data->i2c_sda_pio, BITSERIAL_0_DATA_IN);
       }
       configManagerReleaseConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID);
    }
    return ret_val;
}
/**
 * @brief Function to get I2C Address for.
 *
 * @return: bool: False if read fails/True on success read
*/
static bool sport_health_config_i2c_addr(sensor_config_data_t* sensor_configuration)
{
    activity_monitoring_config_def_t *p_i2c_addr_config_data = NULL;
    bool ret_val = FALSE;

    if(configManagerGetReadOnlyConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID,
                         (const void **)&p_i2c_addr_config_data) > 0)
    {
       sensor_configuration->imu_i2c_addr = p_i2c_addr_config_data->imu_i2c_addr;
       sensor_configuration->ppg_i2c_addr = p_i2c_addr_config_data->ppg_i2c_addr;
       configManagerReleaseConfig(ACTIVITY_MONITORING_CONFIG_BLK_ID);
    }
    return ret_val;
}

/*!
 * @brief Initialise interrupt handling for the form factor board
 *
 * @param cb  The application call back function
 */
static void sport_health_initialise_interrupts(uint16 interrupt_pio)
{
    uint32 pio_map = PioGetMapPins32Bank(PBANK(interrupt_pio));
    uint32 mask = pio_map | INTERRUPT_MASK(interrupt_pio);
    switch_mask_table[1] = INTERRUPT_MASK(interrupt_pio);
    /* Enable the PIOs with connected switches */
    PioSetMapPins32Bank(0, mask, mask);
    /* Request monitoring of the interrupt PIOs */
    PioDebounce32Bank(0,mask,4,15);
    PioCommonSetPio(interrupt_pio, pio_pull, 1);
    interruptData.interruptState = INTERRUPT_MASK(interrupt_pio);
}

/**
 * @brief Function to enable the algorithms supported by default.
*/
void sinkSportHealthEnableDefault(void)
{
    sh_seq_enable(algo_enable_default);
}

/**
 * @brief Initialises the sports and health app task
 * @return sports and health app task ID
 */
Task sport_health_app_init(void)
{
    sh_app.sh_app_task.handler = sport_health_app_message_handler;

    sensor_config_data_t *sensor_configuration = NULL;

    /* setup driver and hub tasks, and pass all task ids around */
    bool result = sport_health_config_i2c_pio(sensor_configuration);
    result += sport_health_config_i2c_addr(sensor_configuration);
    if(result)
    {
        sh_app.driver_task = SportHealthDriverImuPpgInitTask(&(sh_app.sh_app_task), sensor_configuration);
        sh_app.hub_task = SportHealthHubInit(&(sh_app.sh_app_task));
        sport_health_initialise_interrupts(sensor_configuration->interrupt_pio);
        SportHealthDriverImuPpgHubCfg(sh_app.hub_task);
        SportHealthHubCfg(sh_app.driver_task);

        sh_app.enabled = 0;
        sh_app.algo_enable = 0;
        sh_app.mode = SH_APP_UNINITIALISED;

        set_timing_info(500, FALSE);
    }

    return(&(sh_app.sh_app_task));
}
/*!
 * @brief Handler for messages from the system about PIO changes
 *
 * This handler only handles the MESSAGE_PIO_CHANGED event from
 * the system.
 *
 * @param pTask Task data associated with events
 * @param pId The ID of the message we have received
 * @param pMessage Pointer to the message content (if any)
 */
void sport_health_interrupt_handler ( const void* pMessage )
{
    const MessagePioChanged * lMessage;
    unsigned interrupt;

    lMessage = ( const MessagePioChanged * ) (pMessage ) ;
    uint32 newPioState = (uint32)lMessage->state | ((uint32)lMessage->state16to31)<<16;
    uint32 change = interruptData.interruptState ^ newPioState;

    /* Only action when changed, with 1 bit set and only 1 bit changed.
         * The active-low interrupt pin is pulled into its low state by the MAX30102
         * whenever an interrupt is triggered until the interrupt is cleared.
         */
    if (change && !(change & (change-1)) && (change & interruptData.interruptState))
    {
        for (interrupt = 1; interrupt < sizeof(switch_mask_table)/sizeof(switch_mask_table[0]); interrupt++)
        {
            if (change == switch_mask_table[interrupt])
            {
                sh_interrupt_notify();
            }
        }
    }
    /* save the state regardless */
    interruptData.interruptState = newPioState;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      getRSCMeasReading
 *
 *  DESCRIPTION
 *      This function formulates RSC Measurement data in a format
 *      given by RSC service specification.
 *
 *  RETURNS
 *      length of data
 *
 *---------------------------------------------------------------------------*/

uint16 getRSCMeasReading(uint8 *p_rsc_meas, uint8 max_length)
{
    uint16 length = 0;
    if (max_length < 10) return 0;
    /*
     *  --------------------------------------------------------------------------------------------------------------------
     *  | Flags | Instantaneous Speed | Instantaneous Cadence | Instantaneous Stride Length | Total Distance |-----------
     *  --------------------------------------------------------------------------------------------------------------------
     *      Flags field:
     *              Bit 0:  Instantaneous Stride Length Field Present,
                            0 - Not Present, 1 - Present
     *              Bit 1:  Total Distance Field Present,
                            0 - Not Present, 1 - Present
     *              Bit 2:  Walking or Running Feature, 0 - Not Supported or
                            user is walking, 1 - User is running
     *              Bit 3 - 7: RFU (Reserved for future use must be set to 0)
     *
     *      Instantaneous Speed Field:
     *              The instantaneous speed of the user, as measured by the RSC sensor. This is a mandatory field.
     *
     *      Instantaneous Cadence Field:
     *              This field represents the number of times per minute a foot fall occurs. This is a mandatory field.
     *
     *      Instantaneous Stride Length:
     *              This field represents the distance between two successive
     *              contacts of the same foot to the ground. This field is only
     *              present if Bit 0 of the Flags field is 1.
     */


    if(sh_app.algo_enable & ALGO_EN_DISTANCE)
    {
        uint32 inst_speed = SportHealthAlgorithmsGetDistanceSpeed(algo_info.distance_info);
        uint32 stride_length = SportHealthAlgorithmsGetDistanceStrideLength(algo_info.distance_info);
        uint32 total_distance = (SportHealthAlgorithmsGetDistance(algo_info.distance_info)/10); /* Convert cm to decimeters */
        p_rsc_meas[length++] = WALK_OR_RUN_PRESENT | TOTAL_DISTANCE_PRESENT | INST_STRIDE_LENGTH_PRESENT;

        /* Setting the instantaneous speed */
        p_rsc_meas[length++] = inst_speed & 0xFF;
        p_rsc_meas[length++] = inst_speed >> 8;

        /* Setting the instantaneous cadence */
        p_rsc_meas[length++] = 0x40;

        /* Setting the instantaneous stride length */
        p_rsc_meas[length++] = stride_length & 0xFF;
        p_rsc_meas[length++] = stride_length >> 8;

        /* Setting the total distance */
        p_rsc_meas[length++] = total_distance & 0xFF;
        p_rsc_meas[length++] = total_distance >> 8;
        p_rsc_meas[length++] = total_distance >> 16;
        p_rsc_meas[length++] = total_distance >> 24;
    }
    else {
        p_rsc_meas[length++] = 0x00;
        /* Setting the instantaneous speed */
        p_rsc_meas[length++] = 0;
        p_rsc_meas[length++] = 0;
        /* Setting the instantaneous cadence */
        p_rsc_meas[length++] = 0;        
    }
    return length;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      getRSCExtensionReading
 *
 *  DESCRIPTION
 *      This function formulates RSC extension data in a format
 *      given by RSC service specification.
 *
 *  RETURNS
 *      length of data
 *
 *---------------------------------------------------------------------------*/

uint16 getRSCExtensionReading(uint8 *p_rsc_meas, uint8 max_length)
{
    /*
       --------------------------------------------------------------------------------------------------------------------
       | Flags | Step Count | Context | Total Distance | Norm Acceleration |  Energy Used |-----------
       --------------------------------------------------------------------------------------------------------------------
     *      Flags field:
     *              Bit 0:  Step count data,
                            0 - Not Present, 1 - Present
     *              Bit 1:  Context data,
                            0 - Not Present, 1 - Present
     *              Bit 2:  Speed data,
                            0 - Not Present, 1 - Present
     *              Bit 3:  Total distance
                            0 - Not Present, 1 - Present
     *              Bit 4:  Acceleration present
                            0 - Not Present, 1 - Present
     *              Bit 5:  Calories present
                            0 - Not Present, 1 - Present
     *              Bit 6 - 7: RFU (Reserved for future use must be set to 0)
     *
     *      Step Count Field:
     *              The step count of the user, as measured by the RSC sensor.
     *
     *      Context Field:
     *              This field represents the context of the user i.e., walking/running/stationary
     *
     */

    uint16 length = 0;
    if (max_length < 3) return 0;
    if(sh_app.algo_enable & ALGO_EN_STEP)
    {
        uint16 step_cnt = SportHealthAlgorithmsGetStepNumber(algo_info.step_info);
        
        p_rsc_meas[length++] = STEP_COUNT_PRESENT;
        /* Setting the step count */
        p_rsc_meas[length++] = step_cnt & 0xFF;
        p_rsc_meas[length++] = step_cnt >> 8;

    }
    else {
        p_rsc_meas[length++] = 0;
    }
    return length;

}

/*----------------------------------------------------------------------------*
 *  NAME
 *      getHRMeasReading
 *
 *  DESCRIPTION
 *      This function formulates HR Measurement data in a format
 *      given by HR service specification.
 *  *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/

void getHRMeasReading(uint8 *p_hr_meas, uint8 *p_length, uint8 sensor_status)
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

    uint16 hr_bpm = SportHealthAlgorithmsGetHeartRate(algo_info.heart_rate_info);
    /* 78 +/- 32 bpm */
    p_hr_meas[(*p_length)++] = hr_bpm & 0xFF;

    /* 480 +/- 32 */
    p_hr_meas[(*p_length)++] = LE8_L(RR_INTERVAL_IN_MS) + (32 - (int32)UtilRandom() % 16);
    p_hr_meas[(*p_length)++] = LE8_H(RR_INTERVAL_IN_MS);
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      sinkSportHealthGetLoggingDebugConfiguration
 *
 *  DESCRIPTION
 *      This function is used to get the bitmask for logging debug configuration
 *      in the algorithms.
 *
 *  PARAMETERS
 *      p_logging_debug_configuration   Pointer to the debug configuration data.
 *      max_length                      Maximum length of this data
 *
 *  RETURNS
 *      uint16                          Actual length of the data
 *
 *---------------------------------------------------------------------------*/
uint16 sinkSportHealthGetLoggingDebugConfiguration(uint8 *p_logging_debug_configuration, uint8 max_length)
{
    uint32 bitmask = SportHealthLoggingGetBleLogging();
    if (bitmask)
    {
        uint8 index = 0;
        while (index < max_length)
        {
            p_logging_debug_configuration[index] = (bitmask >> (8 * index)) & 0xFF;
            index ++;
        }
        return max_length;
    }

    return 0;
}

/*----------------------------------------------------------------------------*
 *  NAME
 *      sinkSportHealthSetLoggingDebugConfiguration
 *
 *  DESCRIPTION
 *      This function is used to set the bitmask of the debug configuration for
 *      different logging levels in the algorithms
 *
 *  PARAMETERS
 *      p_logging_debug_configuration   Pointer to the debug configuration data.
 *
 *  RETURNS
 *      None
 *
 *---------------------------------------------------------------------------*/
void sinkSportHealthSetLoggingDebugConfiguration(uint8 *p_logging_debug_configuration)
{
    uint32 bitmask = getUint32(0, p_logging_debug_configuration);
    SportHealthLoggingSetBleLogging(bitmask);
}

#endif
