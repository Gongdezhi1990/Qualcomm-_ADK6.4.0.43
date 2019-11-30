/****************************************************************
* Copyright (C) 2017 Qualcomm Technologies International, Ltd
*
* imu_ppg_message_handler.c
* Date: 23/05/2017
* Revision: 1.0.0
*
*
* Usage: Message handlers for the IMU and PPG device library
*
****************************************************************/
#include "sport_health_driver.h"
#include "imu_ppg_message_handler.h"
#include "imu_ppg_private.h"
#include "sport_health_logging.h"

#include "macros.h"
#include "system_clock.h"
#include "i2c.h"
#include "hydra_log.h"
#include <message.h>
#include <stdlib.h>

/***********************************************************
* PRIVATE VARIABLE DECLARATION
***********************************************************/
static imu_t s_imu_info;
static ppg_t s_ppg_info;
static ppg_driver_sensor_data_t ppg_data;
static imu_driver_sensor_data_t  imu_d_data;
static imu_ppg_data_t imu_ppg_data;
static uint16 IMU_DRIVER_LP_SAMPLE_INTERVAL_MS_TABLE[IMU_NUM_LOW_POWER_ODR] = IMU_SUPPORTED_LOW_POWER_ODR;
static uint16 PPG_SAMPLE_RATE_TABLE[PPG_NUM_SAMPLE_RATE] = PPG_SPO2_SAMPLE_RATE;
static uint16 IMU_SETTLE_TIME_TABLE[NUM_IMU_RATES] = IMU_SETTLE_TIME_LOOK_UP;

/***********************************************************
* PRIVATE FUNCTION DECFINITION
***********************************************************/
/*!
 * @brief IMU interrupt handler
 *
 */
void imu_interrupt_handler(void)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;
    /* Toggle between ANY and NO MOTION on alternate interrupts because
     * both the ANY_MOTION and NO_MOTION interrupts in IMU are
     * mapped to same interrupt line (INTERRUPT_1).
    */
    imu_ppg_data.motion_detect_info ^= IMU_ANY_MOTION;
    if(imu_ppg_data.motion_detect_info == IMU_ANY_MOTION)
    {
        /* DISABLE any motion interrupt - x axis*/
        com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_X_ENABLE, IMU_DRIVER_DISABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* DISABLE any motion interrupt - y axis*/
        com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_Y_ENABLE, IMU_DRIVER_DISABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* DISABLE any motion interrupt - z axis*/
        com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_Z_ENABLE, IMU_DRIVER_DISABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable no motion interrupt - x axis*/
        com_rslt += imu_set_intr_enable_2(IMU_DRIVER_NOMOTION_X_ENABLE, IMU_DRIVER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable no motion interrupt - y axis*/
        com_rslt += imu_set_intr_enable_2(IMU_DRIVER_NOMOTION_Y_ENABLE, IMU_DRIVER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable no motion interrupt - z axis*/
        com_rslt += imu_set_intr_enable_2(IMU_DRIVER_NOMOTION_Z_ENABLE, IMU_DRIVER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable the FIFO header */
        com_rslt += imu_set_fifo_header_enable(FIFO_HEADER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /** Enable the FIFO accel */
        com_rslt += imu_set_fifo_accel_enable(FIFO_ACCEL_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    else
    {
        /* Disable the FIFO header */
        com_rslt += imu_set_fifo_header_enable(IMU_DRIVER_DISABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Disable the FIFO accel */
        com_rslt += imu_set_fifo_accel_enable(IMU_DRIVER_DISABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Flush the FIFO */
        com_rslt += imu_set_command_register(FIFO_FLUSH);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable any motion interrupt - x axis*/
        com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_X_ENABLE, IMU_DRIVER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable any motion interrupt - y axis*/
        com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_Y_ENABLE, IMU_DRIVER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /* Enable any motion interrupt - z axis*/
        com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_Z_ENABLE, IMU_DRIVER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }

    /* Send motion detect information to Hub */
    MAKE_IMUPPG_MESSAGE(IMU_MOTION_DETECT_IND);
    message->motion_detect_info = imu_ppg_data.motion_detect_info;
    MessageSend(pImuPpg->hub_task, IMU_MOTION_DETECT_IND, message);
}

/**
 * @brief This function is to configure motion detection ANY_MOTION and
 *        NO_MOTION in IMU sensor
 */
static uint8 imu_motion_detect_config(void)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /* Select interrupt 1 as output */
    com_rslt += imu_set_output_enable(IMU_DRIVER_INTR1_OUTPUT_ENABLE, IMU_DRIVER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Select active-low interrupt (open-drain). Connect to external voltage with a pull up resistor */
    com_rslt += imu_set_intr_level(IMU_DRIVER_INTR1_LEVEL, IMU_DRIVER_LEVEL_LOW);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    com_rslt += imu_set_intr_output_type(IMU_DRIVER_INTR1_LEVEL, IMU_DRIVER_OPEN_DRAIN);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    com_rslt += imu_set_intr_edge_ctrl(IMU_DRIVER_INTR1_LEVEL, IMU_DRIVER_EDGE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Map any motion interrupt to interrupt line 1 */
    com_rslt += imu_set_intr_any_motion(IMU_DRIVER_INTR1_MAP_ANY_MOTION, IMU_DRIVER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Map no motion interrupt to interrupt line 1 */
    com_rslt += imu_set_intr_nomotion(IMU_DRIVER_INTR1_MAP_NOMO, IMU_DRIVER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Set any motion interrupt duration */
    com_rslt += imu_set_intr_any_motion_durn(ANY_MOTION_INTERRUPT_TH);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Set no motion interrupt duration */
    com_rslt += imu_set_intr_slow_no_motion_durn(NO_MOTION_INTERRUPT_TH);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Select slow or no motion */
    com_rslt += imu_set_intr_slow_no_motion_select(NO_MOTION);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Enable any motion interrupt - x axis*/
    com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_X_ENABLE, IMU_DRIVER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Enable any motion interrupt - y axis*/
    com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_Y_ENABLE, IMU_DRIVER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /* Enable any motion interrupt - z axis*/
    com_rslt += imu_set_intr_enable_0(IMU_DRIVER_ANY_MOTION_Z_ENABLE, IMU_DRIVER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);

    return com_rslt;
}
/**
 * @brief This function is for handling when a proximity interrupt is detected.
 * A periodically proximity mode enable
 */
static void ppg_handle_in_ear_detected(void)
{
    SPORT_HEALTH_DRIVER_DEBUG_INFO("In ear detected. \n");
    imu_ppg_data.ppg_in_out_status = PPG_STATUS_IN;
    ppg_reset_out_counter();
}
/**
 * @brief This function is for handling when out of ear has been detected.
 * A periodically proximity mode enable
 */
static void ppg_handle_out_ear_detected(void)
{
    SPORT_HEALTH_DRIVER_DEBUG_INFO("Out ear detected. \n");
    imu_ppg_data.ppg_in_out_status = PPG_STATUS_OUT;
}
/**
 * @brief This function is for updating the in/out status based on the
 *  proximity detection algo. It also notify the hub of the in/out status
 *  if proximity mode is enabled.
 */
static void get_proximity_info(void)
{
    if (imu_ppg_data.ppg_in_out_status != imu_ppg_data.ppg_in_out_status_old)
    {
        switch (imu_ppg_data.ppg_in_out_status)
        {
        case PPG_STATUS_IN:
            ppg_handle_in_ear_detected();
            break;
        case PPG_STATUS_OUT:
            ppg_handle_out_ear_detected();
            break;
        default:
            break;
        }
        imu_ppg_data.ppg_in_out_status_old = imu_ppg_data.ppg_in_out_status;
    }

    if (imu_ppg_data.prox_mode_info != PROXIMITY_MODE_NONE)
    {
        MAKE_IMUPPG_MESSAGE(PROX_READ_FIFO_RSP);
        message->status = prox_success;
        message->in_out_status = imu_ppg_data.ppg_in_out_status;
        MessageSend(pImuPpg->hub_task, PROX_READ_FIFO_RSP, message);
    }
}

/*
*	@brief : The function initializes the IMU sensor data structure
*	@return
*	@param
*/
static void imu_d_sensor_data_init(imu_sensor_data_t *p_imu_sensor_data)
{
   /** Initialise the accelerometer */
   p_imu_sensor_data->accel.frame_count = 0;
   p_imu_sensor_data->accel.range = 0;
   p_imu_sensor_data->accel.last_sample_time = 0;
   p_imu_sensor_data->accel.sampling_interval = 0;

   /** Initialise the gyro */
   p_imu_sensor_data->gyro.frame_count = 0;
   p_imu_sensor_data->gyro.range = 0;
   p_imu_sensor_data->gyro.last_sample_time = 0;
   p_imu_sensor_data->gyro.sampling_interval = 0;

}
/**
*	@brief  The function initializes the PPG sensor data structure
*	@return
*	@param
*/
static void ppg_sensor_data_init(ppg_sensor_data_t *p_ppg_sensor_data)
{
   /** Initialise the red LED */
   p_ppg_sensor_data->red.frame_count = 0;
   p_ppg_sensor_data->red.range = 0;
   p_ppg_sensor_data->red.last_sample_time = 0;
   p_ppg_sensor_data->red.sampling_interval = 0;

   /** Initialise the IR LED */
   p_ppg_sensor_data->IR.frame_count = 0;
   p_ppg_sensor_data->IR.range = 0;
   p_ppg_sensor_data->IR.last_sample_time = 0;
   p_ppg_sensor_data->IR.sampling_interval = 0;
}

/**
 *	@brief This function is used for setting the multi led IR only mode.
 *  @return  Success/Failure (0/1)
 */
uint8 set_multi_led_ir_only_mode(void)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;

    /** Enable the operating mode to Multi LED */
    com_rslt += ppg_set_operating_mode(PPG_MULTI_LED_MODE);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);

    /** Set the current that will be used in multi-led mode */
    com_rslt += ppg_set_proximity_mode_led_pa(PPG_PROX_MODE_LED_PA_CURRENT_12D5mA);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);

    /** Enable IR only for multi led mode */
    com_rslt += ppg_set_multi_led_mode_control_slot1_2(PPG_MULTI_LED_MODE_IR_PA,
                                                       PPG_MULTI_LED_MODE_SLOT_DISABLED);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);

    if (com_rslt != 0)
    {
        SPORT_HEALTH_DRIVER_DEBUG_INFO(("set_multi_led_ir_only_mode failed: %d \n", com_rslt));
    }

    return com_rslt;
}

/**
 * @brief  This function (API) is used to set the configuration of
 *         PPG sensor in HR mode.
 *
 * @param v_sample_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static PPG_RETURN_FUNCTION_TYPE ppg_hr_config_set(uint8 v_sample_rate)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;

    /** Setting configurations for In/Out detection */
    com_rslt += ppg_set_proximity_mode_led_pa(PPG_PROX_MODE_LED_PA_CURRENT_12D5mA);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);

    /** Averaging required to support 25Hz */
    if(v_sample_rate == PPG_SAMPLE_RATE25)
    {
        /** Set the sample average to 8 in register 0x09 */
        com_rslt = ppg_set_sample_avg(PPG_SAMPLE_AVG8);
        s_ppg_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
        /** Set the SPO2 sample rate to 200Hz in the register 0x0A */
        com_rslt += ppg_set_spo2_sample_rate(PPG_SAMPLE_RATE200 -
                                                   PPG_SAMPLE_RATE_OFFSET);
        s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    }
    else
    {
        /** Set the SPO2 sample rate to 50Hz in the register 0x0A */
        com_rslt += ppg_set_spo2_sample_rate(v_sample_rate);
        s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    }
    /** Set the FIFO_A_FULL in register 0x0A */
    com_rslt += ppg_set_fifo_a_full(PPG_ZERO_NUM_SAMPLES_IN_FIFO);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the FIFO_ROLL_OVER_EN in register 0x0A */
    com_rslt += ppg_set_fifo_roll_over(PPG_FIFO_ROLL_OVER_EN);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the SPO2 ADC range to 16384 levels in the register 0x0A */
    com_rslt += ppg_set_spo2_adc_range(PPG_SPO2_ADC_RGE16384);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the SPO2 ADC resolution to 18 bits in the register 0x0A */
    com_rslt += ppg_set_led_pulse_width(PPG_SPO2_LED_PW18);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the LED1 current level to default = 12mA */
    com_rslt += ppg_set_led1_pa(PPG_LED_CURRENT_CONROL_12mA);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    
    /** Set the LED2 current level to default = 12mA */
    com_rslt += ppg_set_led2_pa(PPG_LED_CURRENT_CONROL_12mA);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    
    com_rslt += set_multi_led_ir_only_mode();
    return com_rslt;
}
/**
 * @brief  This function (API) is used to set the configuration of
 *         PPG sensor in HR mode.
 *
 * @param v_sample_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static PPG_RETURN_FUNCTION_TYPE ppg_spo2_config_set(uint8 v_sample_rate)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;

    /** Averaging required to support 25Hz */
    if(v_sample_rate == PPG_SAMPLE_RATE25)
    {
        /** Set the sample average to 8 in register 0x09 */
        com_rslt = ppg_set_sample_avg(PPG_SAMPLE_AVG8);
        s_ppg_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
        /** Set the SPO2 sample rate to 200Hz in the register 0x0A */
        com_rslt += ppg_set_spo2_sample_rate(PPG_SAMPLE_RATE200 -
                                                   PPG_SAMPLE_RATE_OFFSET);
        s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    }
    else
    {
        /** Set the SPO2 sample rate to 50Hz in the register 0x0A */
        com_rslt += ppg_set_spo2_sample_rate(v_sample_rate);
        s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    }
    /** Set the FIFO_A_FULL in register 0x0A */
    com_rslt += ppg_set_fifo_a_full(PPG_ZERO_NUM_SAMPLES_IN_FIFO);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the FIFO_ROLL_OVER_EN in register 0x0A */
    com_rslt += ppg_set_fifo_roll_over(PPG_FIFO_ROLL_OVER_EN);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the SPO2 ADC range to 16384 levels in the register 0x0A */
    com_rslt += ppg_set_spo2_adc_range(PPG_SPO2_ADC_RGE16384);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the SPO2 ADC resolution to 18 bits in the register 0x0A */
    com_rslt += ppg_set_led_pulse_width(PPG_SPO2_LED_PW18);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the LED1 current level to default = 12mA */
    com_rslt += ppg_set_led1_pa(PPG_LED_CURRENT_CONROL_12mA);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Set the LED2 current level to default = 12mA */
    com_rslt += ppg_set_led2_pa(PPG_LED_CURRENT_CONROL_12mA);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    /** Enable the operating mode to Multi LED */
    com_rslt += ppg_set_operating_mode(PPG_SPO2_MODE);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);

    return com_rslt;
}
/**
 * @brief  Get the led mode of the driver
 * @param ppg_mode: ppg mode
 * @param led_mode: Pointer to led mode
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static PPG_RETURN_FUNCTION_TYPE ppg_get_led_mode(ppg_mode_t ppg_mode, ppg_led_mode_t *led_mode)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;

    switch(ppg_mode)
    {
    case SPO2_ONLY:
        *led_mode = RED_IR_LED;
        break;
    case HR_ONLY:
    {
        uint8 p_mode_slot1 = PPG_INIT_VALUE;
        uint8 p_mode_slot2 = PPG_INIT_VALUE;
        com_rslt += ppg_get_multi_led_mode_control_slot1_2(&p_mode_slot1, &p_mode_slot2);
        s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
        uint8 slot1_2 = PPG_GET_REG_VALUE(p_mode_slot1, PPG_MULTI_LED_MODE_CONTROL1,
                                              p_mode_slot2, PPG_MULTI_LED_MODE_CONTROL2);
        if (com_rslt == ppg_success)
        {
            switch(slot1_2)
            {
            case PPG_MULTI_LED_MODE_RED_PA:
                *led_mode = RED_LED_ONLY;
                break;
            case PPG_MULTI_LED_MODE_IR_PA:
                *led_mode = IR_LED_ONLY;
                break;
            case PPG_MULTI_LED_MODE_RED_PILOT_PA:
                *led_mode = PILOT_RED_LED_ONLY;
                break;
            case PPG_MULTI_LED_MODE_IR_PILOT_PA:
                *led_mode = PILOT_IR_LED_ONLY;
                break;
            case PPG_MULTI_LED_MODE_IR_RED_PA:
            case PPG_MULTI_LED_MODE_RED_IR_PA:
            case PPG_MULTI_LED_MODE_IR_RED_PILOT_PA:
            case PPG_MULTI_LED_MODE_RED_IR_PILOT_PA:
                /* We shouldn't use both red and IR Leds for HR for power saving purpose */
            default:
                *led_mode = LED_MODE_NONE;
                break;
            }
        }
    }
        break;
    case PPG_MODE_NONE:
    default:
        break;
    }
    return com_rslt;
}
/**
 * @brief This function is to set the initial header value of the PPG sensor data before
 *        reading the FIFO (payload). Refer to sport_health_driver.h to get more information
 *        about the PPG header in PPG sensor data structure.
 * @param ppg_led_data: Pointer to the ppg data
 * @param p_ppg_sensor_data: Pointer to sensor data
 */
static void get_ppg_data_header_before_payload(ppg_data_t *ppg_led_data, sensor_data_t *p_ppg_sensor_data)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    uint8 v_data = PPG_INIT_VALUE;
    uint8 v_sample_avg = PPG_INIT_VALUE;

    ppg_led_data->p_ppg_data = *(p_ppg_sensor_data->data.p_ppg_data);
    ppg_led_data->frame_count = 0;
    ppg_led_data->saturation_flag = 0;

    p_ppg_sensor_data->range = PPG_HR_ADC_RESOLUTION_16BIT;
    com_rslt += ppg_get_spo2_sample_rate(&v_data);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
    com_rslt += ppg_get_sample_avg(&v_sample_avg);
    s_imu_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);

    p_ppg_sensor_data->sampling_interval = PPG_SAMPLE_RATE_TABLE[(v_data + PPG_SAMPLE_RATE_OFFSET) >> v_sample_avg];

}
/**
 * @brief This function is to set the remaining header parameters of the PPG sensor data
 *        structure once the FIFO read and post processing is complete.Refer to sport_health_driver.h to
 *        get more information about the PPG header in PPG sensor data structure.
 * @param ppg_led_data: Pointer to the ppg data
 * @param p_ppg_sensor_data: Pointer to sensor data
 */
static void get_ppg_data_header_after_payload(ppg_data_t *ppg_led_data, sensor_data_t *p_ppg_sensor_data)
{
    p_ppg_sensor_data->last_sample_time = SportHealthLoggingGetSystemTimeMs();
    p_ppg_sensor_data->frame_count = ppg_led_data->frame_count;
    p_ppg_sensor_data->saturation_flag = ppg_led_data->saturation_flag;
}
/**
 * @brief  This function (API) is used to read the data from FIFO in PPG
 *         sensor.
 *
 * @param  p_ppg_sensor_data Pointer to store PPG sensor data.
 * @param  ppg_header_feedback HR data quality status from algorithms.
 * @param  ppg_mode ppg mode value
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static PPG_RETURN_FUNCTION_TYPE ppg_read_fifo_data(ppg_sensor_data_t *p_ppg_sensor_data, uint8 ppg_header_feedback, ppg_mode_t ppg_mode)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    ppg_led_mode_t led_mode = RED_LED_ONLY;

    UNUSED(ppg_mode);

    ppg_get_led_mode(ppg_mode, &led_mode);

    /** Initialise the sensor data structure before the next fetch */
    ppg_sensor_data_init(p_ppg_sensor_data);

    if (ppg_mode != PPG_MODE_NONE)
    {
        switch(led_mode)
        {
        case RED_LED_ONLY:
        case PILOT_RED_LED_ONLY:
            get_ppg_data_header_before_payload(&(ppg_data.red), &(p_ppg_sensor_data->red));
            ppg_led_current_control(&ppg_data, ppg_header_feedback, led_mode, &imu_ppg_data.ppg_in_out_status);
            get_ppg_data_header_after_payload(&(ppg_data.red), &(p_ppg_sensor_data->red));
            break;
        case IR_LED_ONLY:
        case PILOT_IR_LED_ONLY:
            get_ppg_data_header_before_payload(&(ppg_data.IR), &(p_ppg_sensor_data->IR));
            ppg_led_current_control(&ppg_data, ppg_header_feedback, led_mode, &imu_ppg_data.ppg_in_out_status);
            get_ppg_data_header_after_payload((&ppg_data.IR), &(p_ppg_sensor_data->IR));
            break;
        case RED_IR_LED:
            get_ppg_data_header_before_payload(&(ppg_data.red), &(p_ppg_sensor_data->red));
            get_ppg_data_header_before_payload(&(ppg_data.IR), &(p_ppg_sensor_data->IR));
            ppg_led_current_control(&ppg_data, ppg_header_feedback, led_mode, &imu_ppg_data.ppg_in_out_status);
            get_ppg_data_header_after_payload(&(ppg_data.red), &(p_ppg_sensor_data->red));
            get_ppg_data_header_after_payload((&ppg_data.IR), &(p_ppg_sensor_data->IR));
            break;
        default:
            break;
        }
    }
    get_proximity_info();
    return com_rslt;
}
/**
 * @brief  This function (API) is used to set the configuration of
 *         accelerometer. It sets the default range (+-2G) for aceelerometer.
 *
 * @param v_output_data_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static IMU_DRIVER_RETURN_FUNCTION_TYPE imu_d_accel_config_set(uint8 v_output_data_rate, uint8 v_motion_detect)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /**Set the accel mode as LOWPOWER write in the register 0x7E*/
    com_rslt = imu_set_command_register(ACCEL_LOWPOWER);
    s_imu_info.delay_msec(IMU_DRIVER_MODE_SWITCHING_DELAY);
    /**Set the gyro mode as SUSPEND write in the register 0x7E*/
    com_rslt += imu_set_command_register(GYRO_MODE_SUSPEND);
    s_imu_info.delay_msec(IMU_DRIVER_MODE_SWITCHING_DELAY);
    /** Set the accel bandwidth as number of averaing = 1
     * Note: When in low power mode subtract the IMU_DRIVER_ACCEL_RES_OFFSET
     * from bandwidth of interest.
    */
    com_rslt += imu_set_accel_bw(IMU_DRIVER_ACCEL_RES_NO_AVG - IMU_DRIVER_ACCEL_RES_OFFSET);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Set accel output data rate to default 50Hz */
    com_rslt += imu_set_accel_output_data_rate(
        v_output_data_rate + IMU_ACCEL_OUTPUT_DATA_RATE_OFFSET, IMU_DRIVER_ACCEL_RES_NO_AVG);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    if (v_motion_detect)
    {
        /** Configure the wake on motion interrupts */
        com_rslt += imu_motion_detect_config();
    }
    else
    {
        /** Enable the FIFO header */
        com_rslt += imu_set_fifo_header_enable(FIFO_HEADER_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        /** Enable the FIFO accel */
        com_rslt += imu_set_fifo_accel_enable(FIFO_ACCEL_ENABLE);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    }
    return com_rslt;
}
/**
 * @brief  This function (API) is used to set the configuration of
 *         Gyro. It sets the default range (+-2G) for aceelerometer.
 *
 * @param v_output_data_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static IMU_DRIVER_RETURN_FUNCTION_TYPE imu_gyro_config_set(uint8 v_output_data_rate)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /**Set the accel mode as SUSPEND write in the register 0x7E*/
    com_rslt = imu_set_command_register(ACCEL_SUSPEND);
    s_imu_info.delay_msec(IMU_DRIVER_MODE_SWITCHING_DELAY);
    /**Set the gyro mode as SUSPEND write in the register 0x7E*/
    com_rslt += imu_set_command_register(GYRO_MODE_NORMAL);
    s_imu_info.delay_msec(IMU_DRIVER_MODE_SWITCHING_DELAY);
    /** Set the gryo bandwidth as Normal */
    com_rslt += imu_set_gyro_bw(IMU_DRIVER_GYRO_NORMAL_MODE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Set accel output data rate to default 50Hz */
    com_rslt += imu_set_gyro_output_data_rate(v_output_data_rate);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Enable the FIFO header */
    com_rslt += imu_set_fifo_header_enable(FIFO_HEADER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Enable the FIFO accel */
    com_rslt += imu_set_fifo_gyro_enable(FIFO_GYRO_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);

    return com_rslt;
}
/**
 * @brief  This function (API) is used to set the configuration of
 *         Accel and Gyro. It sets the default range (+-2G) for aceelerometer.
 *
 * @param v_output_data_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static IMU_DRIVER_RETURN_FUNCTION_TYPE imu_d_accel_gyro_config_set(uint8 v_output_data_rate)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /**Set the accel mode as LOWPOWER write in the register 0x7E*/
    com_rslt = imu_set_command_register(ACCEL_LOWPOWER);
    s_imu_info.delay_msec(IMU_DRIVER_MODE_SWITCHING_DELAY);
    /**Set the gyro mode as normal write in the register 0x7E*/
    com_rslt += imu_set_command_register(GYRO_MODE_NORMAL);
    s_imu_info.delay_msec(IMU_DRIVER_MODE_SWITCHING_DELAY);
    /** Set the accel bandwidth as number of averaing = 1
     * Note: When in low power mode subtract the IMU_DRIVER_ACCEL_RES_OFFSET
     * from bandwidth of interest.
    */
    com_rslt += imu_set_accel_bw(IMU_DRIVER_ACCEL_RES_NO_AVG - IMU_DRIVER_ACCEL_RES_OFFSET);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Set the gryo bandwidth as Normal */
    com_rslt += imu_set_gyro_bw(IMU_DRIVER_GYRO_NORMAL_MODE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Set accel output data rate to default 50Hz */
    com_rslt += imu_set_accel_output_data_rate(v_output_data_rate + IMU_ACCEL_OUTPUT_DATA_RATE_OFFSET, IMU_DRIVER_ACCEL_RES_NO_AVG);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Set gyro output data rate to default 50Hz */
    com_rslt += imu_set_gyro_output_data_rate(v_output_data_rate);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Enable the FIFO header */
    com_rslt += imu_set_fifo_header_enable(FIFO_HEADER_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Enable the FIFO accel and gyro */
    com_rslt += imu_set_fifo_gyro_enable(FIFO_ACCEL_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    com_rslt += imu_set_fifo_gyro_enable(FIFO_GYRO_ENABLE);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);

    return com_rslt;
}
/**
 * @brief  This function (API) is used to update the configuration of
 *         accelerometer. Accelerometer range and output data rate are
 *         provided as update option.
 *
 * @param v_range Range to be set
 * @param v_output_data_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static IMU_DRIVER_RETURN_FUNCTION_TYPE IMU_DRIVER_ACCEL_config_update(uint8 v_range,uint8 v_output_data_rate)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /** Update the accel range  */
    com_rslt += imu_set_accel_range(v_range);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Update accel output data rate. Number of averaging cycle
     * is set by default to 1 (IMU_DRIVER_ACCEL_OSR4_AVG1) */
    com_rslt += imu_set_accel_output_data_rate(
        v_output_data_rate + IMU_ACCEL_OUTPUT_DATA_RATE_OFFSET, IMU_DRIVER_ACCEL_RES_NO_AVG);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);

    return com_rslt;
}
/**
 * @brief  This function (API) is used to update the configuration of
 *         Gyro. Gyro range and output data rate are
 *         provided as update option.
 *
 * @param v_range Range to be set
 * @param v_output_data_rate Sampling rate/output data to be set.
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static IMU_DRIVER_RETURN_FUNCTION_TYPE imu_gyro_config_update(uint8 v_range,uint8 v_output_data_rate)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /** Update the gyro range  */
    com_rslt += imu_set_gyro_range(v_range);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
    /** Update gyro output data rate.*/
    com_rslt += imu_set_gyro_output_data_rate(v_output_data_rate);
    s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);

    return com_rslt;
}
/**
 * @brief  This function (API) is used to read the data from FIFO.
 *         If there is an update in accelerometer configuration,
 *         the data is flushed from FIFO following the read based on
 *         input_config_frame type .
 *
 * @param  p_imu_sensor_data IMU sensor data
 * @param  imu_mode Configured IMU Mode (ACCEL/GYRO/BOTH)
 * @return  DRIVER_SUCCESS/Failure (0/1)
 */
static IMU_DRIVER_RETURN_FUNCTION_TYPE imu_read_fifo_header_mode_data(imu_sensor_data_t *p_imu_sensor_data, imu_mode_t imu_mode)
{
    uint16 v_fifo_length = IMU_DRIVER_INIT_VALUE;
    uint8 v_data  =  IMU_DRIVER_INIT_VALUE;
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    /** Initialise the sensor data structure before the next fetch */
    imu_d_sensor_data_init(p_imu_sensor_data);
    switch(imu_mode)
    {
    case ACCEL_ONLY:
        imu_d_data.accel.frame_count = 0;
        imu_d_data.accel.p_imu_data = *(p_imu_sensor_data->accel.data.p_imu_data);
        com_rslt += imu_get_accel_range(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        switch(v_data)
        {
        case IMU_DRIVER_ACCEL_RANGE0:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE0;
            break;
        case IMU_DRIVER_ACCEL_RANGE1:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE1;
            break;
        case IMU_DRIVER_ACCEL_RANGE2:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE2;
            break;
        case IMU_DRIVER_ACCEL_RANGE3:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE3;
            break;
        }
        com_rslt += imu_get_accel_output_data_rate(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        p_imu_sensor_data->accel.sampling_interval = IMU_DRIVER_LP_SAMPLE_INTERVAL_MS_TABLE[v_data - IMU_ACCEL_OUTPUT_DATA_RATE_OFFSET];
        break;
    case GYRO_ONLY:
        imu_d_data.gyro.frame_count = 0;
        imu_d_data.gyro.p_imu_data = *(p_imu_sensor_data->gyro.data.p_imu_data);
        com_rslt += imu_get_gyro_range(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        p_imu_sensor_data->gyro.range = (v_data << IMU_DRIVER_ACCEL_FULL_RANGE_SHIFT);
        com_rslt += imu_get_gyro_output_data_rate(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        p_imu_sensor_data->gyro.sampling_interval = IMU_DRIVER_LP_SAMPLE_INTERVAL_MS_TABLE[v_data];
    case ACCEL_GYRO_BOTH:
        imu_d_data.accel.frame_count = 0;
        imu_d_data.accel.p_imu_data = *(p_imu_sensor_data->accel.data.p_imu_data);
        imu_d_data.gyro.frame_count = 0;
        imu_d_data.gyro.p_imu_data = *(p_imu_sensor_data->gyro.data.p_imu_data);
        com_rslt += imu_get_accel_range(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        switch(v_data)
        {
        case IMU_DRIVER_ACCEL_RANGE0:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE0;
            break;
        case IMU_DRIVER_ACCEL_RANGE1:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE1;
            break;
        case IMU_DRIVER_ACCEL_RANGE2:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE2;
            break;
        case IMU_DRIVER_ACCEL_RANGE3:
            p_imu_sensor_data->accel.range = IMU_DRIVER_ACCEL_FULL_RANGE3;
            break;
        }
        com_rslt += imu_get_accel_output_data_rate(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        p_imu_sensor_data->accel.sampling_interval = IMU_DRIVER_LP_SAMPLE_INTERVAL_MS_TABLE[v_data - IMU_ACCEL_OUTPUT_DATA_RATE_OFFSET];
        com_rslt += imu_get_gyro_range(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        p_imu_sensor_data->gyro.range = (v_data << IMU_DRIVER_ACCEL_FULL_RANGE_SHIFT);
        com_rslt += imu_get_gyro_output_data_rate(&v_data);
        s_imu_info.delay_msec(IMU_DRIVER_GEN_READ_WRITE_DELAY);
        p_imu_sensor_data->gyro.sampling_interval = IMU_DRIVER_LP_SAMPLE_INTERVAL_MS_TABLE[v_data];
    case IMU_MODE_NONE:
        break;
    }
    /** Read the FIFO if FIFO Length greater than 0 */
    com_rslt += imu_d_fifo_length(&v_fifo_length);
    if (v_fifo_length > MAX_SENSOR_RSLT_BUFFER_SIZE)
    {
        SH_DRIVER_DEBUG_PANIC("FIFO length greater than result buffer size");
    }
    p_imu_sensor_data->fifo_query_time = SportHealthLoggingGetSystemTimeMs();
    if(v_fifo_length)
    {
        com_rslt +=  imu_read_fifo_header_data(&imu_d_data, v_fifo_length);
    }
    switch(imu_mode)
    {
    case ACCEL_ONLY:
        p_imu_sensor_data->accel.last_sample_time = SportHealthLoggingGetSystemTimeMs();
        p_imu_sensor_data->accel.frame_count = imu_d_data.accel.frame_count;
        break;
    case GYRO_ONLY:
        p_imu_sensor_data->gyro.last_sample_time = SportHealthLoggingGetSystemTimeMs();
        p_imu_sensor_data->gyro.frame_count = imu_d_data.gyro.frame_count;
        break;
    case ACCEL_GYRO_BOTH:
        p_imu_sensor_data->accel.last_sample_time = SportHealthLoggingGetSystemTimeMs();
        p_imu_sensor_data->accel.frame_count = imu_d_data.accel.frame_count;
        p_imu_sensor_data->gyro.last_sample_time = SportHealthLoggingGetSystemTimeMs();
        p_imu_sensor_data->gyro.frame_count = imu_d_data.gyro.frame_count;
        break;
    case IMU_MODE_NONE:
        break;
    }
    return com_rslt;
}
/***********************************************************
* PUBLIC FUNCTION DECFINITION
***********************************************************/
/**
 *	@brief This function is to initialise imu_ppg_data_t
 *         data structure.
 *
 *	@return
 */
void imu_ppg_data_init(void)
{
    imu_ppg_data.ppg_mode_info = PPG_MODE_NONE;
    imu_ppg_data.imu_mode_info = IMU_MODE_NONE;
    imu_ppg_data.prox_mode_info = PROXIMITY_MODE_NONE;
    imu_ppg_data.motion_detect_info = IMU_NO_MOTION;
    imu_ppg_data.ppg_in_out_status = PPG_STATUS_OUT;
    imu_ppg_data.ppg_in_out_status_old = PPG_STATUS_OUT;
}
/**
 *	@brief This function is initialize function for imu device
 *         driver
 *  @param sensor_configuration the configuration data for sensors
 *	@return  Result (0/1) of driver communication
 *           functions.
 */
uint8 imu_initialize_sensor(sensor_config_data_t* sensor_configuration)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    com_rslt = imu_i2c_routine(sensor_configuration->imu_i2c_addr);
    com_rslt += imu_init(&s_imu_info);

    if(com_rslt)
    {
        com_rslt = imu_failure;
    }
    else
    {
        /** IMU is present */
        com_rslt = (s_imu_info.chip_id == IMU_DRIVER_CHIP_ID)? imu_success : imu_failure;
    }

    return com_rslt;
}
/**
 *	@brief This function is test function for device
 *         driver
 *  @param sensor_configuration the configuration data for sensors
 *	@return  Result (0/1) of driver communication
 *           functions.
 */
uint8 ppg_initialize_sensor(sensor_config_data_t* sensor_configuration)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;

    com_rslt = ppg_i2c_routine(sensor_configuration->ppg_i2c_addr);
    com_rslt += ppg_init(&s_ppg_info);

    if(com_rslt)
    {
        com_rslt = ppg_failure;
    }
    else
    {
        /** PPG is present */
        com_rslt = (s_ppg_info.chip_id == PPG_CHIP_ID)? ppg_success : ppg_failure;
    }
    return com_rslt;
}
 /**
 *	@brief The function is used as I2C bus read
 *
 *	@param v_dev_addr The i2c device address of the sensor
 *	@param v_reg_addr Address of the register from where data
 *                         is going to be read
 *	@param p_reg_data Store the data read from the sensor
 *	@param v_cnt Number of bytes of data to be read
 *
 *	@return Status of the I2C read
 */
int8 i2c_bus_read(uint8 v_dev_addr, uint8 v_reg_addr, uint8 *p_reg_data, uint8 v_cnt)
{
    int8 ierror = IMU_DRIVER_INIT_VALUE;
    uint8 result = IMU_DRIVER_INIT_VALUE;
    uint8 a_i2c_array[I2C_BUFFER_LEN];
    bitserial_config bitserial_config_info;
    bitserial_handle handle;

    a_i2c_array[0] = v_reg_addr;
    /** Initialise the BITSERIAL config for I2C master with SCLK Normal Frequency = 100KHz */
    bitserial_config_info.mode = BITSERIAL_MODE_I2C_MASTER;
    bitserial_config_info.clock_frequency_khz = BITSERIAL_I2C_SCLK_FREQ_NORMAL_MODE;
    bitserial_config_info.u.i2c_cfg.i2c_address = v_dev_addr;
    bitserial_config_info.u.i2c_cfg.flags = BITSERIAL_I2C_ACT_ON_NAK_STOP;
    handle = BitserialOpen(BITSERIAL_BLOCK_0, &bitserial_config_info);
    result = BitserialTransfer(handle, 0, a_i2c_array, (uint16)v_cnt, p_reg_data, (uint16)v_cnt);
    BitserialClose(handle);
    ierror = result? DRIVER_ERROR : DRIVER_SUCCESS;
    return ierror;
}
/**
 *	@brief The function is used as I2C bus read in burst mode
 *
 *	@param v_dev_addr The i2c device address of the sensor
 *	@param v_reg_addr Start address from where the data is going to be read
 *	@param p_reg_data Start address of the buffer storing the read data
 *	@param v_cnt Number of bytes of data to be read
 *
 *  @return Status of the I2C read
 */
int8 i2c_burst_read(uint8 v_dev_addr, uint8 v_reg_addr, uint8 *p_reg_data, uint16 v_cnt)
{
    int8 ierror = IMU_DRIVER_INIT_VALUE;
    uint16 result = IMU_DRIVER_INIT_VALUE;
    uint8 a_i2c_array[I2C_BUFFER_LEN];
    bitserial_config bitserial_config_info;
    bitserial_handle handle;
    a_i2c_array[0] = v_reg_addr;

    /** Initialise the BITSERIAL config for I2C master with SCLK Normal Frequency = 100KHz */
    bitserial_config_info.mode = BITSERIAL_MODE_I2C_MASTER;
    bitserial_config_info.clock_frequency_khz = BITSERIAL_I2C_SCLK_FREQ_NORMAL_MODE;
    bitserial_config_info.u.i2c_cfg.i2c_address = v_dev_addr;
    bitserial_config_info.u.i2c_cfg.flags = BITSERIAL_I2C_ACT_ON_NAK_STOP;
    handle = BitserialOpen(BITSERIAL_BLOCK_0, &bitserial_config_info);
    result += BitserialWrite(handle, 0, a_i2c_array, 1, BITSERIAL_FLAG_NO_STOP);
    result += BitserialRead(handle, 0, p_reg_data, v_cnt, BITSERIAL_FLAG_BLOCK);
    BitserialClose(handle);
    ierror = result? DRIVER_ERROR : DRIVER_SUCCESS;
    if(ierror == DRIVER_ERROR)
    {
        SH_DRIVER_DEBUG_PANIC("I2C burst transfer error");
    }
    return ierror;
}
 /**
 *	@brief The function is used as I2C bus write
 *
 *	@param v_dev_addr The i2c device address of the sensor
 *	@param v_reg_addr Address of the register where the
 *	                  data is going to be written
 *	@param p_reg_data Stores the value that needs to be
 *                      written into the register
 *	@param v_cnt Number of bytes of data to be read
 *
 *	@return Status of the I2C write
 */
int8 i2c_bus_write(uint8 v_dev_addr, uint8 v_reg_addr, uint8 *p_reg_data, uint8 v_cnt)
{
    int8 ierror = IMU_DRIVER_INIT_VALUE;
    uint8 result = IMU_DRIVER_INIT_VALUE;
    uint8 a_i2c_array[I2C_BUFFER_LEN];
    uint8 v_stringpos = IMU_DRIVER_INIT_VALUE;
    bitserial_config bitserial_config_info;
    bitserial_handle handle;

    /** Build the I2C bus write frame according to defined format:
     * slave address | register address | register data
    */
    a_i2c_array[0] = v_reg_addr;
    for (v_stringpos = IMU_DRIVER_INIT_VALUE; v_stringpos < v_cnt; v_stringpos++)
        a_i2c_array[v_stringpos + IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH] = *(p_reg_data + v_stringpos);

    /** Initialise the BITSERIAL config for I2C master with SCLK Normal Frequency = 100KHz */
    bitserial_config_info.mode = BITSERIAL_MODE_I2C_MASTER;
    bitserial_config_info.clock_frequency_khz = BITSERIAL_I2C_SCLK_FREQ_NORMAL_MODE;
    bitserial_config_info.u.i2c_cfg.i2c_address = v_dev_addr;
    bitserial_config_info.u.i2c_cfg.flags = BITSERIAL_I2C_ACT_ON_NAK_STOP;
    handle = BitserialOpen(BITSERIAL_BLOCK_0, &bitserial_config_info);
    result = BitserialTransfer(handle, 0, a_i2c_array, (uint16)(v_cnt + 1), 0, 0);
    BitserialClose(handle);
    ierror = result? DRIVER_ERROR : DRIVER_SUCCESS;
    return ierror;
}
/**
 *	@brief  Used to initialise the I2C bus and map:
 *          read/write, delay_ms and
 *	        i2c device address.
 *  @param  dev_addr, I2C device address
 *  @return  DRIVER_SUCCESS/Failure (0/1)
*/
int8 imu_i2c_routine(uint8 dev_addr)
{
    /** Initialise I2C info for imu */
    s_imu_info.bus_write  = i2c_bus_write;
    s_imu_info.bus_read   = i2c_bus_read;
    s_imu_info.burst_read = i2c_burst_read;
    s_imu_info.delay_msec = delay_ms;
    s_imu_info.dev_addr   = dev_addr;
    return IMU_DRIVER_INIT_VALUE;
}
/**
 *	@brief  Used to initialise the I2C bus and map:
 *          read/write, delay_ms and
 *	        i2c device address.
 *  @param  dev_addr, I2C device address
 *  @return  DRIVER_SUCCESS/Failure (0/1)
*/
int8 ppg_i2c_routine(uint8 dev_addr)
{
    /** Initialise I2C info for PPG */
    s_ppg_info.bus_write = i2c_bus_write;
    s_ppg_info.bus_read = i2c_bus_read;
    s_ppg_info.burst_read = i2c_burst_read;
    s_ppg_info.delay_msec = delay_ms;
    s_ppg_info.dev_addr = dev_addr;

    return PPG_INIT_VALUE;
}
/**
 *	@brief  This function is used to model the delay
 *          in (ms)
 *	@param v_msec_16 Required delay in milli seconds
 *	@return
 */
void delay_ms(uint16 v_msec_16)
{
    /** add one to the delay to make sure we don't return early */
    uint32 v_delay = SystemClockGetTimerTime() + (v_msec_16 * 1000) + 1;

    while (((int32)(SystemClockGetTimerTime() - v_delay)) < 0);
}
/**
 * @brief  This function is a handler to IMU_CONFIG_REQ message.
 * @param  Pointer to received message
 */
void imu_handle_config_req(const IMU_CONFIG_REQ_T *req)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving IMU_CONFIG_REQ"));

    imu_ppg_data.imu_mode_info = req->imu_mode;
    switch(req->imu_mode)
    {
    case ACCEL_ONLY:
        com_rslt += imu_d_accel_config_set(req->v_output_data_rate, req->v_motion_detect);
        break;
    case GYRO_ONLY:
        com_rslt +=  imu_gyro_config_set(req->v_output_data_rate);
        break;
    case ACCEL_GYRO_BOTH:
        com_rslt +=  imu_d_accel_gyro_config_set(req->v_output_data_rate);
        break;
    case IMU_MODE_NONE:
        /** RESET the imu */
        com_rslt += imu_set_command_register(RESET_IMU_DRIVER);
        break;
    }

    MAKE_IMUPPG_MESSAGE(IMU_CONFIG_RSP);
    message->status = com_rslt? imu_failure : imu_success;
    MessageSend(pImuPpg->hub_task, IMU_CONFIG_RSP, message);
}
/**
 * @brief  This function is a handler to IMU_CONFIG_UPDATE_REQ message.
 * @param  Pointer to received message
 */
void imu_handle_config_update_req(const IMU_CONFIG_UPDATE_REQ_T *req)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;

    switch(req->imu_mode)
    {
    case ACCEL_ONLY:
        /** Update the accel configuration */
        com_rslt += IMU_DRIVER_ACCEL_config_update(req->v_range, req->v_output_data_rate);
        /** FIFO Read in next wake up */
        break;
    case GYRO_ONLY:
        /** Update the gyro configuration */
         com_rslt += imu_gyro_config_update(req->v_range, req->v_output_data_rate);
        /** Read the old data from FIFO, and FLUSH the FIFO on reading
           FIFO_input_config_frame */
        com_rslt += imu_read_fifo_header_mode_data(req->p_imu_sensor_data, GYRO_ONLY);
        break;
     case ACCEL_GYRO_BOTH:
        break;
     case IMU_MODE_NONE:
        break;
    }

    MAKE_IMUPPG_MESSAGE(IMU_CONFIG_UPDATE_RSP);
    message->status = com_rslt? imu_failure : imu_success;
	message->settle_time = IMU_SETTLE_TIME_TABLE[req->v_output_data_rate];
    MessageSend(pImuPpg->hub_task, IMU_CONFIG_UPDATE_RSP, message);
}
/**
 * @brief  This function is a handler to IMU_READ_FIFO_REQ message.
 * @param  Pointer to received message
 */
void imu_handle_read_fifo_header_mode_data_req(const IMU_READ_FIFO_REQ_T *req)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = IMU_DRIVER_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving IMU_DRIVER_READ_FIFO_REQ"));
    com_rslt += imu_read_fifo_header_mode_data(req->p_imu_sensor_data, req->imu_mode);

    MAKE_IMUPPG_MESSAGE(IMU_READ_FIFO_RSP);
    message->status = com_rslt? imu_failure : imu_success;
    MessageSend(pImuPpg->hub_task, IMU_READ_FIFO_RSP, message);
}
/**
 * @brief  This function is a handler to PPG_CONFIG_REQ message.
 * @param  Pointer to received message
 */
void ppg_handle_config_req(const PPG_CONFIG_REQ_T *req)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving PPG_CONFIG_REQ"));

    imu_ppg_data.ppg_mode_info = req->ppg_mode;
    switch(imu_ppg_data.ppg_mode_info)
    {
    case HR_ONLY:
        com_rslt += ppg_hr_config_set(req->v_sample_rate);
        break;
    case SPO2_ONLY:
        com_rslt += ppg_spo2_config_set(req->v_sample_rate);
        break;
    case PPG_MODE_NONE:
        /** RESET the PPG for dormant mode*/
        com_rslt = ppg_reset(PPG_CHIP_RESET_EN);
        s_ppg_info.delay_msec(PPG_GEN_READ_WRITE_DELAY);
        com_rslt += ppg_set_operating_mode(0x80);
        /** Free the memory allocated for FIFO data buffer */
        if((p_fifo_data != NULL) && (imu_ppg_data.imu_mode_info == IMU_MODE_NONE))
        {
            /** RESET the imu for dormant mode */
            imu_set_command_register(RESET_IMU_DRIVER);
        }
        break;
    }

    MAKE_IMUPPG_MESSAGE(PPG_CONFIG_RSP);
    message->status = com_rslt? ppg_failure : ppg_success;
    MessageSend(pImuPpg->hub_task, PPG_CONFIG_RSP, message);
}
/**
 * @brief  This function is a handler to PPG_READ_FIFO_REQ message.
 * @param  Pointer to received message
 */
void ppg_handle_read_fifo_header_mode_data_req(const PPG_READ_FIFO_REQ_T *req)
{
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving PPG_READ_FIFO_REQ"));

    com_rslt += ppg_read_fifo_data(req->p_ppg_sensor_data, req->ppg_hr_feedback, req->ppg_mode);

    MAKE_IMUPPG_MESSAGE(PPG_READ_FIFO_RSP);
    message->status = com_rslt? ppg_failure : ppg_success;
    MessageSend(pImuPpg->hub_task, PPG_READ_FIFO_RSP, message);
}

/**
 * @brief  This function is a handler to PPG_SUSPEND_REQ message.
 * @param  Pointer to received message
 */
void ppg_handle_suspend_req(const PPG_SUSPEND_REQ_T *req)
{
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving PPG_SUSPEND_REQ"));

    switch(req->ppg_mode)
    {
    case PPG_MODE_NONE:
        com_rslt += ppg_shutdown(PPG_SHDN_CONTROL_EN);
        break;
    case HR_ONLY:
    case SPO2_ONLY:
        com_rslt += ppg_shutdown(PPG_SHDN_CONTROL_DISABLE);
        break;
    }

    MAKE_IMUPPG_MESSAGE(PPG_SUSPEND_RSP);
    message->status = com_rslt? ppg_failure : ppg_success;
    MessageSend(pImuPpg->hub_task, PPG_SUSPEND_RSP, message);
}
/**
 * @brief This function is a handler to SH_VM_IN_OUT_IND message.
 * @param  Pointer to received message
 */
void ppg_handle_in_out_ear_ind(const SH_VM_IN_OUT_IND_T *ind)
{
    if (imu_ppg_data.ppg_in_out_status != imu_ppg_data.ppg_in_out_status_old)
    {
        switch(ind->tap_type)
        {
        case SH_IN_OUT_TYPE_IN:
            ppg_handle_in_ear_detected();
            break;
        case SH_IN_OUT_TYPE_OUT:
            ppg_handle_out_ear_detected();
            break;
        default:
            break;
        }
        imu_ppg_data.ppg_in_out_status_old = imu_ppg_data.ppg_in_out_status;
    }
}

/**
 * @brief This function is a handler to PROX_CONFIG_REQ message.
 * @param  Pointer to received message
 */
void prox_handle_config_req(const PROX_CONFIG_REQ_T *req)
{
    UNUSED(req);
    IMU_DRIVER_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving PROX_CONFIG_REQ"));
    if(imu_ppg_data.ppg_mode_info == PPG_MODE_NONE)
    {
        com_rslt += ppg_hr_config_set(req->v_sample_rate);
    }
    imu_ppg_data.prox_mode_info = req->prox_mode;

    MAKE_IMUPPG_MESSAGE(PROX_CONFIG_RSP);
    message->status = com_rslt? prox_failure : prox_success;
    MessageSend(pImuPpg->hub_task, PROX_CONFIG_RSP, message);
}
/**
 * @brief This function is a handler to PROX_READ_FIFO_REQ message.
 * @param  Pointer to received message
 */
void prox_handle_read_fifo_data_req(const PROX_READ_FIFO_REQ_T *req)
{
    UNUSED(req);
    PPG_RETURN_FUNCTION_TYPE com_rslt = PPG_INIT_VALUE;
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("IMUPPG Task: Receiving PROX_READ_FIFO_REQ"));
    ppg_led_mode_t led_mode = IR_LED_ONLY;

    com_rslt += prox_read_fifo_data(led_mode, &imu_ppg_data.ppg_in_out_status);
    if(com_rslt)
    {
        SH_DRIVER_DEBUG_INFO("Proximity fifo read failure");
    }
    SPORT_HEALTH_DRIVER_DEBUG_INFO(("in_out_status: %d, old: %d \n", imu_ppg_data.ppg_in_out_status, imu_ppg_data.ppg_in_out_status_old));
    get_proximity_info();
}
