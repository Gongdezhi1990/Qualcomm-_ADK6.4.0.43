/*****************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd

FILE NAME
    sport_health_driver_if.h

DESCRIPTION
    This file contains data private to the IMU and PPG device library.

NOTES

****************************************************************************/

#ifndef SPORT_HEALTH_DRIVER_H
#define SPORT_HEALTH_DRIVER_H

#include "types.h"
#include "message.h"
#include "bitserial_api.h"
#include "sport_health_driver_imu_bmi160.h"
#include "sport_health_driver_ppg_max30102.h"
#include <pio_common.h>
#include "pio.h"

/** Maximum storage limit for sensor results in Hub */
#define MAX_SENSOR_RSLT_BUFFER_SIZE (512) /** The size is in bytes */
#define MAX_SENSOR_RSLT_BUFFER_SIZE_WORDS (CONVERT_BYTES_TO_WORDS(MAX_SENSOR_RSLT_BUFFER_SIZE))

/** Values for deriving FIFO depth by mode */
#define IMU_ACC_BYTES (IMU_DRIVER_FIFO_A_LENGTH + IMU_DRIVER_FIFO_HEADER_LENGTH)
#define IMU_GYRO_BYTES (IMU_DRIVER_FIFO_G_LENGTH + IMU_DRIVER_FIFO_HEADER_LENGTH)
#define IMU_ACC_GYRO_BYTES (IMU_DRIVER_FIFO_AG_LENGTH + IMU_DRIVER_FIFO_HEADER_LENGTH)
/** allowing 1/8 headroom */
#define IMU_FIFO_DEPTH_BYTES (IMU_DRIVER_FIFO_DEPTH - 128)

/** FIFO depth by mode */
#define FIFO_DEPTH_MIN_MS        (0)
#define IMU_FIFO_DEPTH_ACC (IMU_FIFO_DEPTH_BYTES/IMU_ACC_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_ACC (MAX_SENSOR_RSLT_BUFFER_SIZE/IMU_ACC_BYTES)
#define IMU_FIFO_DEPTH_GYRO (IMU_FIFO_DEPTH_BYTES/IMU_GYRO_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_GYRO (MAX_SENSOR_RSLT_BUFFER_SIZE/IMU_GYRO_BYTES)
#define IMU_FIFO_DEPTH_ACC_GYRO (IMU_FIFO_DEPTH_BYTES/IMU_ACC_GYRO_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_ACC_GYRO (MAX_SENSOR_RSLT_BUFFER_SIZE/IMU_ACC_GYRO_BYTES)

/** Values for deriving FIFO depth by mode */
#define PPG_PPG_BYTES         (PPG_DRIVER_FIFO_HR_LENGTH)
#define PPG_SPO2_BYTES        (PPG_DRIVER_FIFO_IR_LENGTH)
#define PPG_PPG_SPO2_BYTES    (PPG_DRIVER_FIFO_MULTI_LED_LENGTH)
#define PPG_FIFO_DEPTH_BYTES  (PPG_DRIVER_FIFO_DEPTH_BYTES - 8)

/** FIFO depth by mode */
#define PPG_FIFO_DEPTH_PPG (PPG_FIFO_DEPTH_BYTES/PPG_PPG_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_PPG (MAX_SENSOR_RSLT_BUFFER_SIZE/PPG_PPG_BYTES)
#define PPG_FIFO_DEPTH_SPO2 (PPG_FIFO_DEPTH_BYTES/PPG_SPO2_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_SPO2 (MAX_SENSOR_RSLT_BUFFER_SIZE/PPG_SPO2_BYTES)
#define PPG_FIFO_DEPTH_PPG_SPO2 (PPG_FIFO_DEPTH_BYTES/PPG_PPG_SPO2_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_PPG_SPO2 (MAX_SENSOR_RSLT_BUFFER_SIZE/PPG_PPG_SPO2_BYTES)

#define PROX_FIFO_DEPTH              ((PROX_FIFO_DEPTH_BYTES - 8)/PROX_PROXIMITY_BYTES)
#define MAX_SENSOR_RSLT_DEPTH_PROX   (MAX_SENSOR_RSLT_BUFFER_SIZE/PROX_PROXIMITY_BYTES)

#define IMU_NUM_VAL_PER_SAMPLE   (3) /**< IMU sensor gives coordinates x,y and z per sample */

/* macro for GATT Library debug lib printfs */
#ifdef SPORT_HEALTH_DRIVER_DEBUG_LIB
#include <panic.h>
#include <stdio.h>
#include <print.h>
#define SPORT_HEALTH_DRIVER_DEBUG_INFO(x)  {PRINT (x);}
#define SPORT_HEALTH_DRIVER_DEBUG(x)       {PRINT (x);  Panic();}
#else
#define SPORT_HEALTH_DRIVER_DEBUG_INFO(x)
#define SPORT_HEALTH_DRIVER_DEBUG(x)
#endif

/**************************************************************/
/**\name	STRUCTURE DEFINITIONS                         */
/**************************************************************/
/**
 * @brief The IMU PPG device profile structure.
 */
typedef struct
{
    TaskData             task;
    Task                 app_task;
    Task                 hub_task;
}IMUPPG;

/** Provide a reference for all IMUPPG lib modules */
extern IMUPPG *pImuPpg;

/** Provide a reference to the working buffer for all IMUPPG lib modules */
extern uint8 *p_fifo_data;

extern bool bitSerialTransferComplete;


/***************************************************************/
/***\name	STRUCTURE DEFINITIONS                         */
/***************************************************************/

/**
 * @brief  Struct used to store the configurations for
 *         sensor in IMU or PPG
 */
typedef struct{

    uint16  interrupt_pio;       /**< Interrupt PIO of the Sensor */
    uint8   imu_i2c_addr;        /**< device I2C address of IMU Sensor */
    uint8   ppg_i2c_addr;        /**< device I2C address of PPG Sensor */
}sensor_config_data_t;


/**
 * @brief   Struct used to store the data from each
 *         sensor in IMU or PPG
 */
typedef struct{
    uint32 last_sample_time;  /**< Last sample time in ms */
    uint16 frame_count;       /**< Number of samples from Accel */
    union{
        int16  **p_imu_data;
        uint16 **p_ppg_data;
    }data;
    uint16  range;             /**< Range of Accel */
    uint8  sampling_interval; /**< Sampling interval of Accel */
    bool saturation_flag;  /**< FIFO data saturation glag */
}sensor_data_t;
/**
 * @brief   Struct used to store all the FIFO data from
 *         IMU sensors
 */
typedef struct{
    sensor_data_t accel;
    sensor_data_t gyro;
    uint32 fifo_query_time;
 }imu_sensor_data_t;
/**
 * @brief   Struct used to store all the FIFO data from
 *         PPG sensors
 */
 typedef struct{
    ppg_led_mode_t led_mode;
    sensor_data_t IR;
    sensor_data_t red;
    sensor_data_t green;
}ppg_sensor_data_t;
/** @brief   PPG feedback indications from algorithms for PPG
 *         calibration for heart rate measurements
 */
typedef enum{
    HR_DATA_GOOD,
    HR_DATA_OK,
    HR_DATA_BAD,
    HR_DATA_VERY_BAD
}ppg_hr_feedback_t;

/**
 * @brief   IMU specific message IDs for IMU PPG task
 */
#define IMU_INTERNAL_MESSAGE_BASE_ADDR     (3200)

typedef enum
{
    /** Driver Library messages */
    IMU_CONFIG_REQ = IMU_INTERNAL_MESSAGE_BASE_ADDR,
    IMU_CONFIG_RSP,
    IMU_CONFIG_UPDATE_REQ,
    IMU_CONFIG_UPDATE_RSP,
    IMU_READ_FIFO_REQ,
    IMU_READ_FIFO_RSP,
    IMU_MOTION_DETECT_IND,
    IMU_INTERRUPT_IND,
    NUM_IMU_INTERNAL_MESSAGES
}imu_internal_messages_t;

/**
 * @brief   PPG specific message IDs for IMU PPG task
 */
#define PPG_INTERNAL_MESSAGE_BASE_ADDR     (NUM_IMU_INTERNAL_MESSAGES)
typedef enum
{
    /** Driver Library messages */
    PPG_CONFIG_REQ = PPG_INTERNAL_MESSAGE_BASE_ADDR,
    PPG_CONFIG_RSP,
    PPG_SUSPEND_REQ,
    PPG_SUSPEND_RSP,
    PPG_READ_FIFO_REQ,
    PPG_READ_FIFO_RSP,
    NUM_PPG_INTERNAL_MESSAGES
}ppg_internal_messages_t;

/**
 * @brief   Proximity specific message IDs for IMU PPG task
 */
#define PROX_INTERNAL_MESSAGE_BASE_ADDR     (NUM_PPG_INTERNAL_MESSAGES)
typedef enum
{
    /** Driver Library messages */
    PROX_CONFIG_REQ = PROX_INTERNAL_MESSAGE_BASE_ADDR,
    PROX_CONFIG_RSP,
    PROX_READ_FIFO_REQ,
    PROX_READ_FIFO_RSP
}prox_internal_messages_t;

typedef enum
{
    /** IMUPPG Library confirmation */
    IMUPPG_INIT_CFM = 0,
}imuppg_init_cfm_message_t;

/**
 * @brief   IMU specific mode of operations
 */

typedef enum
{
    ACCEL_ONLY = 1,
    GYRO_ONLY,
    ACCEL_GYRO_BOTH,
    IMU_MODE_NONE
}imu_mode_t;
/**
 * @brief   PPG specific mode of operations
 */
typedef enum
{
    HR_ONLY = 1,
    SPO2_ONLY,
    PPG_MODE_NONE
}ppg_mode_t;
/**
 * @brief   Proximity specific mode of operations
 */
typedef enum
{
    PROXIMITY_ONLY = 1,
    PROXIMITY_MODE_NONE
}prox_mode_t;
/**
 * @brief  Status code returned in messages from the IMU library
 *        This status code indicates the outcome of the request.
 */
typedef enum
{
    imu_success = 0,               /**< The operation succeeded. */
    imu_failure,                   /**< The operation failed. */
    imu_invalid_parameters,        /**< Invalid parameters supplied by the client. */
}imu_status_code_t;
/**
 * @brief   Motion detect modes in IMU
 */
typedef enum
{
    IMU_NO_MOTION = 0,
    IMU_ANY_MOTION
}motion_mode_t;
/**
 * @brief  Status code returned in messages from the PPG library
 *        This status code indicates the outcome of the request.
 *
 */
typedef enum
{
    ppg_success = 0,               /** The operation succeeded. */
    ppg_failure,                   /** The operation failed. */
    ppg_invalid_parameters,        /** Invalid parameters supplied by the client. */

}ppg_status_code_t;

/**
 * @brief Status code returned in messages from the PROXIMITY library
 *        This status code indicates the outcome of the request.
 *
 */
typedef enum
{
    prox_success = 0,               /** The operation succeeded. */
    prox_failure,                   /** The operation failed. */
    prox_invalid_parameters,        /** Invalid parameters supplied by the client. */

}prox_status_code_t;

#define IMU_ACCEL_OUTPUT_DATA_RATE_OFFSET     (6)
/**
 *  @brief  Supported output data rate for accelerometer low power mode in IMU sensor
*/
typedef enum
{
    IMU_OUTPUT_DATA_RATE_25HZ = 0,
    IMU_OUTPUT_DATA_RATE_50HZ,
    IMU_OUTPUT_DATA_RATE_100HZ,
    IMU_OUTPUT_DATA_RATE_200HZ,
    IMU_OUTPUT_DATA_RATE_400HZ,
    IMU_NUM_OUTPUT_DATA_RATE
}imu_output_data_rate_t;
/**
 * @brief  Supported ADC range in PPG sensor
*/
typedef enum
{
    PPG_ADC_RGE2048 = 0,
    PPG_ADC_RGE4096,
    PPG_ADC_RGE8192,
    PPG_ADC_RGE16384,
    PPG_NUM_ADC_RGE
}ppg_adc_range_t;
/**
 * @brief  Supported output data rate PPG sensor
*/
typedef enum
{
    PPG_SAMPLE_RATE25 = 0,
    PPG_SAMPLE_RATE50,
    PPG_SAMPLE_RATE100,
    PPG_SAMPLE_RATE200,
    PPG_SAMPLE_RATE400,
    PPG_NUM_SAMPLE_RATE,
    PPG_SAMPLE_RATE_OFFSET = PPG_SAMPLE_RATE50
}ppg_output_data_rate_t;
/**
 * @brief   This message is sent as a response to calling IMU_INIT_REQ.
 *         This message indicates the outcome of initialising the library.
 */
typedef struct
{
    Task             task;
    imu_status_code_t    imu_status;
    ppg_status_code_t    ppg_status;
} IMUPPG_INIT_CFM_T;
/** @brief   This message sent from scheduler is to set the output data
 *         rate of accelerometer in IMU sensor
 */
typedef struct
{
    uint8 v_output_data_rate;
    imu_mode_t imu_mode;
    bool v_motion_detect;
} IMU_CONFIG_REQ_T;
/** @brief   This message is sent to scheduler in response to IMU_CONFIG_REQ_T
 */
typedef struct
{
    imu_status_code_t    status;
} IMU_CONFIG_RSP_T;
/** @brief   This message is sent from scheduler to update the configuration of
 *         accelerometer in IMU sensor
 */
typedef struct
{
    imu_sensor_data_t *p_imu_sensor_data;
    uint8 v_range;
    uint8 v_output_data_rate;
    imu_mode_t imu_mode;
} IMU_CONFIG_UPDATE_REQ_T;
/** @brief   This message is sent to scheduler in response to IMU_CONFIG_UPDATE_REQ_T
 */
typedef struct
{
	uint16               settle_time;
    imu_status_code_t    status;
} IMU_CONFIG_UPDATE_RSP_T;
/**
 * @brief   This message is sent from scheduler to read the data in FIFO in IMU sensor
 */
typedef struct
{
    imu_sensor_data_t *p_imu_sensor_data;
    imu_mode_t imu_mode;
}IMU_READ_FIFO_REQ_T;
/**
 * @brief   This message is sent to scheduler in response to IMU_READ_FIFO_REQ_T
 */
typedef struct
{
    imu_status_code_t    status;
} IMU_READ_FIFO_RSP_T;
/**
 * @brief   This message is sent to scheduler to update the status of motion
 *          detection
 */
typedef struct
{
    motion_mode_t  motion_detect_info;
} IMU_MOTION_DETECT_IND_T;
/**
 * @brief   This message sent from scheduler is to set the sample
 *         rate of red LED in PPG sensor
 */
typedef struct
{
    uint8 v_sample_rate;
    ppg_mode_t ppg_mode;
} PPG_CONFIG_REQ_T;
/**
 * @brief   This message is sent to scheduler in response to PPG_CONFIG_REQ_T
 */
typedef struct
{
    ppg_status_code_t    status;
} PPG_CONFIG_RSP_T;
/**
 * @brief   This message sent from scheduler is to bring the PPG
 *         IN/OUT of suspend  mode
 */
typedef struct
{
    uint8 ppg_mode;
} PPG_SUSPEND_REQ_T;
/**
 * @brief   This message sent from scheduler is sent in response
 *         to PPG_SUSPEND_REQ
 */
typedef struct
{
    ppg_status_code_t    status;
} PPG_SUSPEND_RSP_T;
/**
 * @brief   This message is sent from scheduler to read the data in FIFO in PPG sensor
 */
typedef struct
{
    ppg_sensor_data_t *p_ppg_sensor_data;
    ppg_hr_feedback_t ppg_hr_feedback;
    ppg_mode_t ppg_mode;
}PPG_READ_FIFO_REQ_T;
/**
 * @brief   This message is sent to scheduler in response to PPG_READ_FIFO_REQ_T
 */
typedef struct
{
    ppg_status_code_t    status;
} PPG_READ_FIFO_RSP_T;
/** @brief This message sent from scheduler is to set the output data
 *         rate of proximity sensor
 */
typedef struct
{
    uint8 v_sample_rate;
    prox_mode_t prox_mode;
} PROX_CONFIG_REQ_T;
/** @brief   This message is sent to scheduler in response to PROX_CONFIG_REQ_T
 */
typedef struct
{
    prox_status_code_t    status;
} PROX_CONFIG_RSP_T;
/**
 * @brief   This message is sent from hub to read the data in FIFO in proximity sensor
 */
typedef struct
{
    prox_mode_t prox_mode;
}PROX_READ_FIFO_REQ_T;
/**
 * @brief   This message is sent to scheduler in response to PPG_READ_FIFO_REQ_T
 */
typedef struct
{
    bool in_out_status; /**< flag to indicate IN or OUT status of proximity sensor */
    prox_status_code_t    status;
} PROX_READ_FIFO_RSP_T;
/**************************************************************/
/**\name	LOOK UP TABLE FOR IMU SETTLE TIME    */
/**************************************************************/
#define IMU_SETTLE_TIME_LOOK_UP \
{IMU_SETTLE_TIME_TRANS_TO_LO_RATE_MS,\
 IMU_SETTLE_TIME_TRANS_TO_LO_RATE_MS,\
 IMU_SETTLE_TIME_TRANS_TO_HI_RATE_MS,\
 IMU_SETTLE_TIME_TRANS_TO_HI_RATE_MS,\
 IMU_SETTLE_TIME_TRANS_TO_HI_RATE_MS\
}
#define NUM_IMU_RATES       (5)

/**************************************************************/
/**\name	LOOK UP TABLES FOR ODR AND RANGE    */
/**************************************************************/
#define IMU_SUPPORTED_LOW_POWER_ODR \
                {40, 20, 10, 5, 2}
#define PPG_SPO2_ADC_RGE \
                {2048, 4096, 8192, 16384}
#define PPG_SPO2_SAMPLE_RATE \
                {40, 20, 10, 5, 2}

#define IMU_NUM_LOW_POWER_ODR  (IMU_NUM_OUTPUT_DATA_RATE)
#define PPG_NUM_ADC_RGE        (PPG_NUM_ADC_RGE)
#define PPG_NUM_SAMPLE_RATE    (PPG_NUM_SAMPLE_RATE)

/**
 * @brief Initialises the IMU and PPG sensor
 * @param client_task Task Id for app task
 * @param sensor_configuration the configuration data for sensors
 * @return IMUPGG task ID
 */
Task SportHealthDriverImuPpgInitTask(Task client_task, sensor_config_data_t* sensor_configuration);

/**
 * @brief Register the hub task id to the IMUPPG task
 * @param clientTask [in] Task Id for the hub task
 */
void SportHealthDriverImuPpgHubCfg(Task client_task);

/**
 * @brief Delete the IMUPPG task as application has triggered
 *        teardown
 */
void imu_ppg_delete(void);


/***************************************************/
/***\name	 FUNCTION DECLARATIONS FOR IMU  */
/**************************************************/
/**
*	@brief  This API is used to perform initialization steps
*          for imu sensor. This include
*          1. Trigger a reset including a reboot of imu sensor.
*          2. Read and save the CHIP_ID
*          3. Prevent wake up of GYRO
*  @param p_imu_info Pointer to imu sensor device specific
*         information
*	@return  Results of bus communication function
*
*
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_init(imu_t *p_imu_info);
/*******************************************************************/
/***\name	 FUNCTIONS FOR ACCEL AND GYRO POWER MODE STATUS  */
/******************************************************************/
/**
*	@brief  This API reads the Gyro power mode
*
*   @param p_gyro_power_mode_stat :	Stores the value of Accel
*         power mode
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_power_mode_stat(uint8 *p_gyro_power_mode_stat);
/**
*	@brief  This API reads the Accel power mode
*
*   @param p_accel_power_mode_stat :	Stores the value of Accel
*         power mode
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_power_mode_stat(uint8 *p_accel_power_mode_stat);

#ifdef FIFO_ENABLE
/***************************************************/
/***\name	 FUNCTION FOR FIFO LENGTH AND FIFO DATA READ */
/**************************************************/
/**
*	@brief  This API reads the FIFO length of the sensor
*
*   @param p_fifo_length  The value of FIFO byte counter
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_d_fifo_length(uint16 *v_fifo_length);
/**
*	@brief  This API reads the FIFO data of the sensor
*
*   @param p_fifodata  Pointer holding the FIFO data
*   @param v_fifo_length  Number of bytes to be read from
*         FIFO. The maximum size of FIFO is 1024 bytes.
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_d_fifo_data(uint8 *p_fifodata, uint16 v_fifo_length);
#endif
/***************************************************/
/***\name	 FUNCTION FOR ACCEL CONFIGURATIONS */
/**************************************************/
/**
*	@brief  This API is used to get the Accel output date rate
*
*  @param   p_output_data_rate  Stores the value of Accel output
*           date rate
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_output_data_rate(uint8 *p_output_data_rate);
/**
*	@brief  This API is used to set the Accel output date rate
*
*   @param   v_output_data_rate :The value of Accel output date rate
*   @param  v_accel_bw :The value of selected Accel bandwidth
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_output_data_rate(uint8 v_output_data_rate, uint8 v_accel_bw);
/**
*	@brief  This API is used to get the Accel bandwidth
*
*   @param  p_bw  Stores the value of Accel bandwidth
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_bw(uint8 *p_bw);
/**
*	@brief  This API is used to set the Accel bandwidth
*
*   @param  v_bw  The value to be set of Accel bandwidth
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_bw(uint8 v_bw);
/**
*	@brief  This API is used to get the Accel under sampling parameter
*
*	@param  p_accel_under_sampling  Stores the value of Accel
*          under sampling
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_under_sampling_parameter(uint8 *p_accel_under_sampling);
/**
*	@brief  This API is used to set the Accel under sampling parameter
*
*	@param  v_accel_under_sampling  The value of Accel under sampling
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_under_sampling_parameter(
uint8 v_accel_under_sampling);
/**
*	@brief  This API is used to read the range (g values) of the
*          Accel
*
*   @param p_range  The value of Accel g range
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_accel_range(uint8 *p_range);
/**
*	@brief  This API is used to set the range (g values) of the
*          Accel
*
*   @param v_range  The value of Accel g range
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_accel_range(uint8 v_range);
/***************************************************/
/***\name	 FUNCTION FOR GYRO CONFIGURATIONS */
/**************************************************/
/**
*	@brief This API is used to read the Gyro output data rate
*
*   @param  p_output_data_rate  Stores the value of gyro output data rate
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_output_data_rate(uint8 *p_output_data_rate);
/**
*	@brief This API is used to set the Gyro output data rate
*
*   @param  v_output_data_rate :The value of gyro output data rate
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_gyro_output_data_rate(uint8 v_output_data_rate);
/**
*	@brief This API is used to read the bandwidth of gyro
*
*   @param  p_bw  Stores the value of gyro bandwidth
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_bw(uint8 *p_bw);
/**
*	@brief This API is used to set the bandwidth of gyro
*
*   @param  v_bw  Stores The value of gyro bandwidth
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_gyro_bw(uint8 v_bw);
/**
*	@brief This API reads the range of Gyro
*
*   @param  p_range  Stores the value of gyro range
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_gyro_range(uint8 *v_range);
/**
*	@brief This API sets the range of Gyro
*
*   @param  v_range  The value of gyro range
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_gyro_range(uint8 v_range);
#ifdef FIFO_ENABLE
/***************************************************/
/***\name	 FUNCTION FOR FIFO CONFIGURATIONS */
/**************************************************/
/**
*	@brief This API reads FIFO frame header enable
*
*  @param p_fifo_header  Stores the value of FIFO header
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_fifo_header_enable(uint8 *p_fifo_header);
/**
*	@brief This API sets FIFO frame header enable
*
*  @param v_fifo_header  The value of FIFO header
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_fifo_header_enable(uint8 v_fifo_header);
/**
*	@brief  This API is used to check whether Accel data is stored
*          in FIFO (all 3 axes) or not
*
*  @param p_fifo_accel  Stores the value of FIFO Accel enable
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_fifo_accel_enable(uint8 *p_fifo_accel);
/**
*	@brief  This API is used to enable Accel data in FIFO (all 3 axes)
*
*  @param v_fifo_accel  Stores the value of FIFO Accel enable
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_fifo_accel_enable(uint8 v_fifo_accel);
/**
*	@brief This API is used to check whether gyro data is stored in FIFO (all 3 axes)
*
*   @param p_fifo_gyro  The value of FIFO Gyro enable
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_fifo_gyro_enable(uint8 *p_fifo_gyro);
/**
*	@brief  This API is used to enable Gyro data in FIFO (all 3 axes)
*
*  @param v_fifo_gyro  Stores the value of FIFO Gyro enable
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_fifo_gyro_enable(uint8 v_fifo_gyro);
#endif
/*!
 *	@brief This API reads the Any motion interrupt which is mapped to interrupt1
 *  and interrupt2 pin. For more details refer function definition.
 *
 *	@param v_channel: The value of any motion selection
 *	@param v_intr_any_motion : The value of any motion enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_any_motion(uint8 v_channel, uint8 *v_intr_any_motion);
/*!
 *	@brief This API writes the Any motion interrupt to be mapped to interrupt1
 *	and interrupt2. For more details refer function definition.
 *
 *	@param v_channel: The value of any motion selection
 *	@param v_intr_any_motion : The value of any motion enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_any_motion(uint8 v_channel, uint8 v_intr_any_motion);
/*!
 *	@brief This API reads the No motion interrupt which is  mapped to interrupt1
 *	and interrupt2 pin. For more details refer function definition.
 *
 *	@param v_channel: The value of no motion selection
 *	@param v_intr_nomotion : The value of no motion enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_nomotion(uint8 v_channel, uint8 *v_intr_nomotion);
/*!
 *	@brief This API configures the No motion interrupt to be mapped to interrupt1
 *	and interrupt2 pin. For more details refer function definition.
 *
 *	@param v_channel: The value of no motion selection
 *	@param v_intr_nomotion : The value of no motion enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_nomotion(uint8 v_channel, uint8 v_intr_nomotion);
/*!
 *	@brief This API reads any motion duration. For more details refer function definition.
 *
 *  @param v_any_motion_durn : The value of any motion duration
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_any_motion_durn(uint8 *v_any_motion_durn);
/*!
 *	@brief This API writes any motion duration. For more details refer function definition.
 *
 *  @param v_any_motion_durn : The value of any motion duration
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_any_motion_durn(uint8 v_any_motion_durn);
/*!
 *	@brief This API reads Slow/no-motion interrupt trigger delay duration.
 *  For more details refer function definition.
 *
 *  @param v_slow_no_motion :The value of slow no motion duration
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_slow_no_motion_durn(uint8 *v_slow_no_motion);
 /*!
 *	@brief This API writes Slow/no-motion interrupt trigger delay duration.
 *  For more details refer function definition.
 *
 *  @param v_slow_no_motion :The value of slow no motion duration
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_slow_no_motion_durn(uint8 v_slow_no_motion);
/*!
 *	@brief This API is used to read threshold definition for the any-motion interrupt
 *	For more details refer function definition.
 *
 *  @param  v_any_motion_thres : The value of any motion threshold
 *
 *	@note any motion threshold changes according to Accel g range
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_any_motion_thres(uint8 *v_any_motion_thres);
/*!
 *	@brief This API is used to write threshold definition for  any-motion interrupt
 *	For more details refer function definition.
 *
 *  @param  v_any_motion_thres : The value of any motion threshold
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_any_motion_thres(uint8 v_any_motion_thres);
 /*!
 *	@brief This API is used to read threshold for the slow/no-motion interrupt
 *	For more details refer function definition.
 *
 *  @param v_slow_no_motion_thres : The value of slow no motion threshold
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_slow_no_motion_thres(uint8 *v_slow_no_motion_thres);
 /*!
 *	@brief This API is used to write threshold for the slow/no-motion interrupt.
 *  For more details refer function definition.
 *
 *  @param v_slow_no_motion_thres : The value of slow no motion threshold
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_slow_no_motion_thres(uint8 v_slow_no_motion_thres);
 /*!
 *	@brief This API is used to read
 *	the slow/no-motion selection from sensor. For more details refer function definition.
 *  @param  v_intr_slow_no_motion_select :
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_intr_slow_no_motion_select(uint8 *v_intr_slow_no_motion_select);
 /*!
 *	@brief This API is used to write the slow/no-motion selection to the IMU sensor
 *  For more details refer function definition.
 *
 *  @param  v_intr_slow_no_motion_select :
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_slow_no_motion_select(uint8 v_intr_slow_no_motion_select);
/*!
 *	@brief  This API is used to set the available interrupt enable bitfield.
 *  For more details refer function definition.
 *
 *	@param v_enable : Value which selects the interrupt
 *	@param v_intr_enable_zero : The interrupt enable value
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_enable_0(uint8 v_enable, uint8 v_intr_enable_zero);
/*!
 *	@brief  This API is used to set the available interrupt enable bitfield.
 *  For more details refer function definition.
 *
 *	@param v_enable: The value of interrupt enable
 *	@param v_intr_enable_2 : The interrupt enable value
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_enable_2(uint8 v_enable, uint8 v_intr_enable_2);
/*!
*	@brief This API reads the any motion interrupt status from the sesnor related
*   with a specific interrupt function. For more details refer function definition.
*
*  @param v_any_motion_intr : The status of any-motion interrupt
*
*  @return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_stat0_any_motion_intr(uint8 *v_any_motion_intr);
/*!
 *	@brief This API reads data ready no motion interrupt status
 *	from the sensor. For more details refer function definition.
 *
 *  @param v_nomotion_intr : The status of no motion interrupt
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_get_stat1_nomotion_intr(uint8 *v_nomotion_intr);
/*!
 *	@brief This API is used to set the Output enable for interrupt1
 *	and interrupt2 pin. For more details refer function definition.
 *
 *  @param v_channel: The value of output enable selection
 *	@param v_output_enable : The value of output enable of interrupt enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_output_enable(uint8 v_channel, uint8 v_output_enable);
/*!
 *	@brief  This API is used to set the configure level condition of
 *	interrupt1 and interrupt2 pin. For more details refer function definition.
 *
 *  @param v_channel: The value of level condition selection
 *	@param v_intr_level : The value of level of interrupt enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_level(uint8 v_channel, uint8 v_intr_level);
/*!
 *	@brief  This API configures trigger condition of interrupt1
 *	and interrupt2 pin. For more details refer function definition
 *
 *  @param v_channel: The value of edge trigger selection
 *	@param v_intr_edge_ctrl : The value of edge trigger enable
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_edge_ctrl(uint8 v_channel, uint8 v_intr_edge_ctrl);
/*!
 *	@brief  This API is used to set output type on interrupt1
 *	and interrupt2 pin. For more details refer function definition.
 *
 *  @param v_channel: The value of output type enable selection
 *	@param v_intr_output_type :	The value of output type of interrupt enable
 *
 *	@return results of bus communication function
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_intr_output_type(uint8 v_channel, uint8 v_intr_output_type);
/****************************************************/
/***\name	FUNCTION FOR STEP COMMAND REGISTER WRITE */
/****************************************************/
/**
* @brief This API writes the value to the command register
*
* @return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_set_command_register(uint8 v_command_reg);

#ifdef FIFO_ENABLE
/****************************************************/
/***\name	FUNCTIONS FOR FIFO DATA READ */
/****************************************************/
/**
*	@brief  This function is used to read the fifo data when
*  configured in header mode
*
*  @param p_imu_d_sensor_data Pointer to 3D sensor data structure
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_read_fifo_header_data(
imu_driver_sensor_data_t *p_imu_d_sensor_data, uint16 v_fifo_length);
/**
*	@brief  This function is used to read the fifo data in header
*          mode for user defined length
*
*  @param p_fifo_header_data Pointer to 3D sensor data structure
*  @param v_fifo_user_length Number of bytes to be read
*
*	@return  DRIVER_SUCCESS/Failure (0/1)
*/
IMU_DRIVER_RETURN_FUNCTION_TYPE imu_read_fifo_header_data_user_defined_length(
uint16 v_fifo_user_length, imu_driver_sensor_data_t *fifo_header_data);
#endif /* FIFO_ENABLE */



/***************************************************/
/***\name	 FUNCTION DECLARATIONS  */
/**************************************************/
/**
 *	@brief  This API is used to perform initialization steps
 *          for ppg sensor.
 *
 *  @param p_ppg_info Pointer to ppg sensor device specific
 *         information
 *
 *	@return  Results of bus communication function
 */
PPG_RETURN_FUNCTION_TYPE ppg_init(ppg_t *p_ppg_info);
/**
 *	@brief  This API is used to RESET the ppg sensor
 *
 *  @param  v_reset The value to be set in reset field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_reset(uint8 v_reset);
/**
 *	@brief  This API is used to SHUTDOWN the ppg sensor.
 *
 *  @param  v_shdn The value to be set in shutdown field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_shutdown(uint8 v_shdn);

/**
 *	@brief  This API is used to set the multi-LED mode control for slot1 and 2
 *          in ppg sensor.
 *
 *  @param  slot1_mode The value to be set in mode field for slot1
 *  @param  slot2_mode The value to be set in mode field for slot2
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_multi_led_mode_control_slot1_2(uint8 slot1_mode,
                                                                uint8 slot2_mode);
/**
 *	@brief  This API is used to get the multi-LED mode control for slot1 and 2
 *          in ppg sensor.
 *
 *  @param  p_slot1_mode Stores value set in mode field for slot1
 *  @param  p_slot2_mode Stores value set in mode field for slot2
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_multi_led_mode_control_slot1_2(uint8 *p_slot1_mode,
                                                                uint8 *p_slot2_mode);
/**
 *	@brief  This API is used to set the multi-LED mode control for slot3 and 4
 *          in ppg sensor.
 *
 *  @param  slot3_mode The value to be set in mode field for slot3
 *  @param  slot4_mode The value to be set in mode field for slot4
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_multi_led_mode_control_slot3_4(uint8 slot3_mode,
                                                                uint8 slot4_mode);
/**
 *	@brief  This API is used to get the multi-LED mode control for slot3 and 4
 *          in ppg sensor.
 *
 *  @param  p_slot3_mode Stores value set in mode field for slot3
 *  @param  p_slot4_mode Stores value set in mode field for slot4
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_multi_led_mode_control_slot3_4(uint8 *p_slot3_mode,
                                                                uint8 *p_slot4_mode);

/**
 *	@brief  This API is used to get the interrupt status for proximity in ppg sensor.
 *
 *  @param  p_status Stores value set in status field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_interrupt_status(uint8 *p_status);

/**
 *	@brief  This API is used to set the interrupt enable for proximity in ppg sensor.
 *
 *  @param  v_enable The value to be set in enable field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_proximity_interrupt_enable(uint8 v_enable);
/**
 *	@brief  This API is used to get the interrupt enable for proximity in ppg sensor.
 *
 *  @param  p_enable Stores value set in enable field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_interrupt_enable(uint8 *p_enable);
/**
 *	@brief  This API writes Proximity Mode LED PA to ppg sensor
 *
 *  @param  v_pilot_pa The value to write to Proximity Mode LED PA
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_proximity_mode_led_pa(uint8 v_pilot_pa);
/**
 *	@brief  This API reads Proximity Mode LED PA value from ppg sensor
 *
 *  @param  v_pilot_pa Stores the value of Proximity Mode LED PA
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_mode_led_pa(uint8 *v_pilot_pa);
/**
 *	@brief  This API writes Proximity Interrupt Threshold to ppg sensor
 *
 *  @param  v_prox_int_threshhold The value to write to Proximity Interrupt Threshold
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_proximity_interrupt_threshold(uint8 v_prox_int_threshhold);
/**
 *	@brief  This API reads Proximity Interrupt Threshold value from ppg sensor
 *
 *  @param  v_prox_int_threshhold Stores the value of Proximity Interrupt Threshold
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_proximity_interrupt_threshold(uint8 *v_prox_int_threshhold);

/**
 *	@brief  This API is used to set the operating state in ppg sensor.
 *
 *  @param  v_mode The value to be set in mode field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_operating_mode(uint8 v_mode);
/**
 *	@brief  This API is used to get the operating state in ppg sensor.
 *
 *  @param  p_mode Stores value set in mode field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_operating_mode(uint8 *p_mode);
/**
 *	@brief  This API writes LED PA
 *
 *  @param  v_led_pa The value to write to LED PA
 *
 *	@return  Success/Failure (0/1)
*/
PPG_RETURN_FUNCTION_TYPE ppg_set_led1_pa(uint8 v_led_pa);
PPG_RETURN_FUNCTION_TYPE ppg_set_led2_pa(uint8 v_led_pa);
/**
 *	@brief  This API reads LED PA
 *
 *  @param  p_led_pa Stores the value of LED1 PA
 *
 *	@return  Success/Failure (0/1)
*/
PPG_RETURN_FUNCTION_TYPE ppg_get_led1_pa(uint8 *p_led_pa);
PPG_RETURN_FUNCTION_TYPE ppg_get_led2_pa(uint8 *p_led_pa);
/**
 *	@brief  This API is used to set the sample averaging in ppg sensor.
 *
 *  @param  v_sample_avg The value to be set in sample average field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_sample_avg(uint8 v_sample_avg);
/**
 *	@brief  This API is used to get the sample averaging in ppg sensor.
 *
 *  @param  p_sample_avg Stores value to set in sample average field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_sample_avg(uint8 *p_sample_avg);
/**
 *	@brief  This API is used to set the fifo full water mark in ppg sensor.
 *
 *  @param  v_fifo_a_full Number of empty spaces in fifo before triggering
 *                        fifo full interrupt
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_fifo_a_full(uint8 v_fifo_a_full);
/**
 *	@brief  This API is used to set the fifo roll over in ppg sensor.
 *
 *  @param  v_roll_over_en The value to be set in fifo roll over field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_fifo_roll_over(uint8 v_roll_over_en);
/**
 *	@brief  This API is used to set the SPO2 sensor ADC full
 *          scale range in ppg sensor.
 *
 *  @param  v_adc_range The value to be set in mode field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_spo2_adc_range(uint8 v_adc_range);
/**
 *	@brief  This API is used to get the SPO2 sensor ADC full
 *          scale range in ppg sensor.
 *
 *  @param  p_adc_range Stores value set in ADC range field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_spo2_adc_range(uint8 *p_adc_range);
/**
 *	@brief  This API is used to set the SPO2 sensor sample rate in ppg sensor.
 *
 *  @param  v_sample_rate The value to be set in sample rate field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_spo2_sample_rate(uint8 v_sample_rate);
/**
 *	@brief  This API is used to get the SPO2 sensor sample rate
 *          in ppg sensor.
 *
 *  @param  p_sample_rate Stores value set in sample rate field
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_spo2_sample_rate(uint8 *p_sample_rate);
/**
 *	@brief  This API is used to set the SPO2 sensor LED pulse width
 *          and ADC resolution in ppg sensor.
 *
 *  @param  v_pulse_width The value to be set in LED pulse width field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_set_led_pulse_width(uint8 v_pulse_width);
/**
 *	@brief  This API is used to set the SPO2 sensor LED pulse width
 *          and ADC resolution in ppg sensor.
 *
 *  @param  p_pulse_width Stores value set in LED pulse width field
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_get_led_pulse_width(uint8 *p_pulse_width);
/**
 *	@brief  This API gets the length of FIFO of in ppg sensor.
 *
 *  @param p_fifo_length The value of FIFO byte counter
 *  @param led_mode Enum indicates the led being used
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE PPG_FIFO_length(uint16 *p_fifo_length, ppg_led_mode_t led_mode);
/**
 *	@brief  This API reads the FIFO data in ppg sensor.
 *
 *  @param p_fifodata Pointer holding the FIFO data
 *  @param v_fifo_length Number of bytes to be read from FIFO.
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE PPG_FIFO_data(uint8 *p_fifodata, uint16 v_fifo_length);
/**
 *	@brief  This API reads the FIFO data in ppg sensor as per the user defined length
 *          and reorganizes the data according to LED types.
 *
 *  @param p_ppg_sensor_data Pointer holding the FIFO data defined as per algorithm
 *                              interface.
 *  @param v_fifo_length Number of bytes to be read from FIFO.
 *                             The maximum size of FIFO is 192 bytes.
 *  @param led_mode Enum indicates the led being used
 *  @param in_out pointer to store in and out status
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_read_fifo_data_user_defined_length(
        ppg_driver_sensor_data_t *p_ppg_sensor_data, uint16 v_fifo_length,
        ppg_led_mode_t led_mode, uint8 *in_out);
/**
 *	@brief  This API handles the data saturation in SPO2/HR photodiode
 *          If a data saturation is detected, the current level is reduced by 0.02 mA.
 *
 *  @param   p_ppg_sensor_data Pointer to store PPG sensor data.
 *           v_fifo_length Number of bytes to be read from FIFO.
 *                             The maximum size of FIFO is 192 bytes.
 *
 *	@return  saturation_flag_b: True/False
 */
PPG_RETURN_FUNCTION_TYPE PPG_FIFO_check_data_saturation(uint16 v_fifo_length);
/**
 *	@brief  This API handles the current control of LED based on feedback from algorithms
 *          If in the request, algos report HR_DATA_BAD status current is incremented by
 *          0.2mA and FIFO is flushed. If HR_DATA_GOOD, FIFO is read and current is
 *          decremented by 0.2mA. If HR_OK, current level is unchanged and FIFO is read.
 *
 *  @param  p_ppg_sensor_data Pointer to store PPG sensor data.
 *  @param  ppg_header_feedback HR data quality status from algorithms.
 *  @param  led_mode Led mode value
 *  @param  in_out Pointer to store in and out status.
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE ppg_led_current_control(ppg_driver_sensor_data_t *p_ppg_sensor_data,
                                                 uint8 ppg_header_feedback, ppg_led_mode_t led_mode,
                                                 uint8 *in_out);
/**
 *	@brief  This APIis used to flush the FIFO.
 *
 *  @param
 *
 *	@return
 */
PPG_RETURN_FUNCTION_TYPE ppg_flush_fifo(void);

/**
 *	@brief  This API is used to reset the out of ear counter to zero.
 *
 *  @param
 *
 *	@return
 */
void ppg_reset_out_counter(void);

/**
 *	@brief  This API reads the FIFO data in PPG sensor as per the user
 *  defined length and reorganizes the data according to LED types.
 *
 *  @param led_mode mode of led in multi led configuration.
 *  @param in_out pointer to store in and out status
 *
 *	@return  Success/Failure (0/1)
 */
PPG_RETURN_FUNCTION_TYPE prox_read_fifo_data(ppg_led_mode_t led_mode, uint8 *in_out);

#endif /* SPORT_HEALTH_DRIVER_H */

