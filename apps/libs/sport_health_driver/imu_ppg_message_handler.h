/*****************************************************************************
Copyright (c) 2017 - 2018 Qualcomm Technologies International, Ltd

FILE NAME
    imu_ppg_message_handler.h

DESCRIPTION
    Header file for the message handler for the IMU and PPG device
    library.

NOTES

***************************************************************************/
#ifndef IMU_PPG_MESSAGE_HANDLER_H
#define IMU_PPG_MESSAGE_HANDLER_H

#include "sport_health_driver.h"
#include "sport_health_hub.h"

#define SYSTEM_CLOCKS_PER_MILLISECOND               ((uint32)1000)

/**!
 * @brief struct used for assign the value for
 *	gyro sleep configuration
 */
struct gyro_sleep_setting {
    uint8 sleep_trigger;/***< gyro sleep trigger configuration*/
    uint8 wakeup_trigger;/***< gyro wakeup trigger configuration*/
    uint8 sleep_state;/***< gyro sleep state configuration*/
    uint8 wakeup_int;/***< gyro wakeup interrupt configuration*/
};

/*********************************/
/** I2C Input buffer length */
/********************************/
#define I2C_BUFFER_LEN (8)

/*********************************/
/** BITSERIAL CONFIGURATION */
/********************************/
#define BITSERIAL_I2C_SCLK_FREQ_NORMAL_MODE         (100)
#define BITSERIAL_I2C_SCLK_FREQ_FAST_MODE           (400)

/*********************************/
/** IMU POWER MODES DEFINITION */
/********************************/
#define ACCEL_MODE_NORMAL                           (0x11)
#define GYRO_MODE_NORMAL                            (0x15)
#define	ACCEL_LOWPOWER                              (0X12)
#define MAG_SUSPEND_MODE                            (1)
#define IMU_DRIVER_MODE_SWITCHING_DELAY             (30)

/*********************************/
/** RETURN TYPE */
/********************************/
#define IMU_DRIVER_RETURN_FUNCTION_TYPE int8

/*********************************/
/***\name RUNNING MODE DEFINITIONS */
/********************************/
#define ACCEL_FIFO_CONFIG                           (0)
#define ACCEL_FIFO_CONFIG_UPDATE                    (1)
#define READ_FIFO_HEADER_MODE                       (2)

/***************************************************/
/**	ACCEL FULL RANGE          */
/**************************************************/
#define IMU_DRIVER_ACCEL_FULL_RANGE_SHIFT           (1)

/***************************************************************/
/***\name	ACCEL DATA REQUEST HEADER LENGTH, POSITION AND MASK*/
/***************************************************************/
/** Acc_Data_Request_Header ODR Description - Bit --> 0...3 */
#define IMU_DRIVER_ACCEL_CONFIG_OUTPUT_DATA_RATE__POS    (0)
#define IMU_DRIVER_ACCEL_CONFIG_OUTPUT_DATA_RATE__LEN    (4)
#define IMU_DRIVER_ACCEL_CONFIG_OUTPUT_DATA_RATE__MSK    (0x0F)
/***************************************************************/
/***\name	ACCEL DATA REQUEST HEADER LENGTH, POSITION AND MASK*/
/***************************************************************/
/** Acc_Data_Request_Header Range Description - Bit --> 4...5 */
#define IMU_DRIVER_ACCEL_RANGE__POS                      (4)
#define IMU_DRIVER_ACCEL_RANGE__LEN                      (2)
#define IMU_DRIVER_ACCEL_RANGE__MSK                      (0x30)
/*********************************/
/***\name MAG INTERFACE */
/********************************/
#define	C_IMU_DRIVER_BYTE_COUNT                          (2)
#define IMU_DRIVER_SLEEP_STATE                           (0x00)
#define IMU_DRIVER_WAKEUP_INTR                           (0x00)
#define IMU_DRIVER_SLEEP_TRIGGER                         (0x04)
#define IMU_DRIVER_WAKEUP_TRIGGER                        (0x02)
#define IMU_DRIVER_ENABLE_FIFO_WM                        (0x02)
#define	IMU_DRIVER_MAG_INTERFACE_OFF_PRIMARY_ON          (0x00)
#define	IMU_DRIVER_MAG_INTERFACE_ON_PRIMARY_ON           (0x02)

/***************************************************************/
/***\name	STRUCTURE DEFINITIONS                         */
/***************************************************************/
/**!
*	@brief imu_ppg structure
*	This structure holds all relevant information about bmi160
*/
typedef struct{
    ppg_mode_t ppg_mode_info;
    imu_mode_t imu_mode_info;
    prox_mode_t prox_mode_info;
    motion_mode_t motion_detect_info;
    uint8 ppg_in_out_status;
    uint8 ppg_in_out_status_old;
}imu_ppg_data_t;

/**
 *	@brief This function is used for setting the multi led IR only mode.
 *  @return  Success/Failure (0/1)
 */
uint8 set_multi_led_ir_only_mode(void);
/**
 *	@brief This function is to initialise imu_ppg_data_t
 *         data structure.
 *
 *	@return
 */
void imu_ppg_data_init(void);
/**
 *	@brief This function used for initialize the sensor
 *
 *  @param sensor_configuration the configuration data for sensors
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
uint8 imu_initialize_sensor(sensor_config_data_t* sensor_configuration);

/**
 *	@brief This function used for initialize the sensor
 *
 *  @param sensor_configuration the configuration data for sensors
 *	@return results of bus communication function
 *	@retval 0 -> Success
 *	@retval 1 -> Error
 *
 */
uint8 ppg_initialize_sensor(sensor_config_data_t* sensor_configuration);
/**
 *	@brief This function is an example for delay
 *	@param msec: delay in milli seconds
 *	@return communication result
 */
void delay_ms(uint16 msec);
/**
 *	@brief  Used to initialise the I2C bus and map:
 *          read/write, delay_ms and
 *	        i2c device address.
 *  @param  dev_addr, I2C device address
 *  @return  Success/Failure (0/1)
*/
int8 imu_i2c_routine(uint8 dev_addr);
/**
 *	@brief  Used to initialise the I2C bus and map:
 *          read/write, delay_ms and
 *	        i2c device address.
 *  @param  dev_addr, I2C device address
 *  @return  Success/Failure (0/1)
*/
int8 ppg_i2c_routine(uint8 dev_addr);
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
int8 i2c_bus_read(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint8 cnt);
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
int8 i2c_burst_read(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint16 cnt);
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
int8 i2c_bus_write(uint8 dev_addr, uint8 reg_addr, uint8 *reg_data, uint8 cnt);
/**
 * @brief This function is a handler to IMU_CONFIG_REQ_T message.
 * @param  Pointer to received message
 */
void imu_handle_config_req(const IMU_CONFIG_REQ_T *req);
/**
 * @brief This function is a handler to IMU_CONFIG_UPDATE_REQ message.
 * @param  Pointer to received message
 */
void imu_handle_config_update_req(const IMU_CONFIG_UPDATE_REQ_T *req);
/**
 * @brief This function is a handler to IMU_READ_FIFO_REQ message.
 * @param  Pointer to received message
 */
void imu_handle_read_fifo_header_mode_data_req(const IMU_READ_FIFO_REQ_T *req);
/**
 * @brief IMU interrupt handler
 */
void imu_interrupt_handler(void);
/**
 * @brief This function is a handler to PPG_CONFIG_REQ message.
 * @param  Pointer to received message
 */
void ppg_handle_config_req(const PPG_CONFIG_REQ_T *req);
/**
 * @brief This function is a handler to PPG_READ_FIFO_REQ message.
 * @param  Pointer to received message
 */
void ppg_handle_read_fifo_header_mode_data_req(const PPG_READ_FIFO_REQ_T *req);
/**
 * @brief This function is a handler to PPG_SUSPEND_REQ message.
 * @param  Pointer to received message
 */
void ppg_handle_suspend_req(const PPG_SUSPEND_REQ_T *req);
/**
 * @brief This function is a handler to SH_VM_IN_OUT_IND message.
 * @param  Pointer to received message
 */
void ppg_handle_in_out_ear_ind(const SH_VM_IN_OUT_IND_T *ind);

/**
 * @brief This function is a handler to PROX_CONFIG_REQ message.
 * @param  Pointer to received message
 */
void prox_handle_config_req(const PROX_CONFIG_REQ_T *req);
/**
 * @brief This function is a handler to PROX_READ_FIFO_REQ message.
 * @param  Pointer to received message
 */
void prox_handle_read_fifo_data_req(const PROX_READ_FIFO_REQ_T *req);

#endif /* IMU_PPG_MESSAGE_HANDLER_H */
