/****************************************************************
* Copyright (C) 2017 Qualcomm Technologies International, Ltd
*
* max30102.h
* Date: 19/06/2017
* Revision: 1.0.0
*
*
* Usage: (MAX30102) PPG Sensor Device Driver include file
*
****************************************************************/
#ifndef SPORT_HEALTH_DRIVER_PPG_MAX30102_H
#define SPORT_HEALTH_DRIVER_PPG_MAX30102_H

#include "print.h"
#include "panic.h"

#define SH_DRIVER_DEBUG_INFO(x) {PRINT(("%s:%d - ", __FILE__, __LINE__)); PRINT(x);}
#define SH_DRIVER_DEBUG_PANIC(x) {SH_DRIVER_DEBUG_INFO(x); Panic();}

/****************************************************************/
/***\name	BUS READ AND WRITE FUNCTION POINTERS        */
/****************************************************************/
/**
 * @brief Define the calling convention for bus communication routine.
 *        This includes types of parameters. This example shows the
 *        configuration for an I2C bus link.
 *
 * For communication function that looks like this:
 * bus_write(uint8 device_addr, uint8 register_addr, uint8 * data, uint8 length);
 *
 * The PPG_WR_FUNC_PTR would equal:
 *
 * PPG_WR_FUNC_PTR int8 (* bus_write)(uint8, uint8, uint8 *, uint8)
 */
#define PPG_WR_FUNC_PTR int8 (*bus_write)(uint8, uint8, uint8 *, uint8)

/** @brief link macro between API function calls and bus write function
 *         The bus write function can change since this is a
 *         system dependante.
*/
#define PPG_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
                bus_write(dev_addr, reg_addr, reg_data, wr_len)

/**
 * @brief Define the calling convention for bus communication routine.
 *        This includes types of parameters. This example shows the
 *        configuration for an I2C bus link.
 *
 * For communication function that looks like this:
 * bus_read(uint8 device_addr, uint8 register_addr, uint8 * data, uint8 length);
 *
 * The PPG_RD_FUNC_PTR would equal:
 *
 * PPG_RD_FUNC_PTR int8 (* bus_read)(uint8, uint8, uint8 *, uint8)
 */
#define PPG_RD_FUNC_PTR int8 (*bus_read)(uint8, uint8, uint8 *, uint8)

#define PPG_BRD_FUNC_PTR int8 (*burst_read)(uint8, uint8, uint8 *, uint16)

/** @brief link macro between API function calls and bus write function
 *         The bus write function can change since this is a
 *         system dependante.
*/
#define PPG_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len)\
                bus_read(dev_addr, reg_addr, reg_data, r_len)

#define PPG_BURST_READ_FUNC(device_addr, register_addr, register_data, rd_len)\
                burst_read(device_addr, register_addr, register_data, rd_len)

#define PPG_MDELAY_DATA_TYPE                uint16

/** Use BITSERIAL as I2C master */
/*define BITSERIAL (1)*/
/** Enabled for ADK6.1 */
#define MAX_DATA_TRANSFER_LENGTH_PPG        (252)  /** Header FIFO frame = 7 bytes in max30102. Therefore, transfer length MOD 7 = 0 */

#define PPG_OUT_COUNTER_NUMBER         1

/**************************************************/
/**\name	FIFO FRAME COUNT DEFINITION   */
/*************************************************/
#define CONVERT_BYTES_TO_WORDS(x)           (x >> 2)
#define FIFO_DEPTH                          (MAX_DATA_TRANSFER_LENGTH_PPG)
#define FIFO_DEPTH_WORDS                    (CONVERT_BYTES_TO_WORDS(FIFO_DEPTH))

/*****************************************************/
/***\name	REGISTER DEFINITIONS       */
/****************************************************/
#define PPG_PROX_INT_STATUS_ADDR            (0x00)
#define PPG_PROX_INT_EN_ADDR                (0x02)
#define PPG_FIFO_WR_PTR_ADDR                (0x04)
#define PPG_FIFO_RD_PTR_ADDR                (0x06)
#define PPG_FIFO_DATA_REG_ADDR              (0x07)
#define PPG_FIFO_CONFIG_ADDR                (0x08)
#define PPG_MODE_CONFIG_ADDR                (0x09)
#define PPG_SPO2_CONFIG_ADDR                (0x0A)
#define PPG_LED1_PA_ADDR                    (0x0C)
#define PPG_LED2_PA_ADDR                    (0x0D)
#define PPG_PROX_MODE_LED_PA_ADDR           (0x10)
#define PPG_MULTI_LED_MODE_CONTROL12_ADDR   (0x11)
#define PPG_MULTI_LED_MODE_CONTROL34_ADDR   (0x12)
#define PPG_PROX_INT_THRESH_ADDR            (0x30)

/********************************************/
/***\name	CONSTANTS        */
/*******************************************/
#define  PPG_INIT_VALUE     	            (0)
#define  PPG_GEN_READ_WRITE_DATA_LENGTH     (1)

/***************************************************/
/***\name	BIT SLICE GET AND SET FUNCTIONS  */
/**************************************************/
#define PPG_GET_BITSLICE(regvar, bitname)\
        ((regvar & bitname##__MSK) >> bitname##__POS)

#define PPG_SET_BITSLICE(regvar, bitname, val)\
        ((regvar & ~bitname##__MSK) | \
        ((val<<bitname##__POS)&bitname##__MSK))

#define PPG_GET_REG_VALUE(val1, bitname1, val2, bitname2)\
        (((val1<<bitname1##__POS)&bitname1##__MSK) |\
        ((val2<<bitname2##__POS)&bitname2##__MSK))

/*************************************************/
/***\name	PPG PROX INT STATUS/EN POINTER  */
/*************************************************/
#define PPG_PROX_INT_STATUS_PTR__POS        (4)
#define PPG_PROX_INT_STATUS_PTR__LEN        (1)
#define PPG_PROX_INT_STATUS_PTR__MSK        (0x10)
#define PPG_PROX_INT_STATUS_PTR__REG        (PPG_PROX_INT_STATUS_ADDR)

#define PPG_PROX_INT_EN_PTR__POS            (4)
#define PPG_PROX_INT_EN_PTR__LEN            (1)
#define PPG_PROX_INT_EN_PTR__MSK            (0x10)
#define PPG_PROX_INT_EN_PTR__REG            (PPG_PROX_INT_EN_ADDR)

#define PPG_PROX_INT_EN_VALUE               (1)
#define PPG_PROX_INT_EN_DISTABLE_VALUE      (0)
#define PPG_PROX_INT_STATUS_LOW_VALUE       (0)
#define PPG_PROX_INT_STATUS_HIGH_VALUE      (1)

/*************************************************/
/***\name	PPG PROX MODE LED PA POINTER  */
/*************************************************/
#define PPG_PROX_MODE_LED_PA__REG           (PPG_PROX_MODE_LED_PA_ADDR)

#define PPG_PROX_MODE_LED_PA_CURRENT_0mA    (0x00)
#define PPG_PROX_MODE_LED_PA_CURRENT_3D1mA  (0x0F)
#define PPG_PROX_MODE_LED_PA_CURRENT_6D4mA  (0x1F)
#define PPG_PROX_MODE_LED_PA_CURRENT_12D5mA (0x3F)
#define PPG_PROX_MODE_LED_PA_CURRENT_25D4mA (0x7F)
#define PPG_PROX_MODE_LED_PA_CURRENT_50mA   (0xFF)

/********************************************/
/***\name	MAX30102 MULTI LED MODE CONTROL SLOT1,2  */
/*******************************************/
#define PPG_MULTI_LED_MODE_CONTROL1__POS    (0)
#define PPG_MULTI_LED_MODE_CONTROL1__LEN    (3)
#define PPG_MULTI_LED_MODE_CONTROL1__MSK    (0x07)
#define PPG_MULTI_LED_MODE_CONTROL1__REG    (PPG_MULTI_LED_MODE_CONTROL12_ADDR)
#define PPG_MULTI_LED_MODE_CONTROL2__POS    (4)
#define PPG_MULTI_LED_MODE_CONTROL2__LEN    (3)
#define PPG_MULTI_LED_MODE_CONTROL2__MSK    (0x70)
#define PPG_MULTI_LED_MODE_CONTROL2__REG    (PPG_MULTI_LED_MODE_CONTROL12_ADDR)

#define PPG_MULTI_LED_MODE_SLOT_DISABLED    (0x00)
#define PPG_MULTI_LED_MODE_RED_PA           (0x01)
#define PPG_MULTI_LED_MODE_IR_PA            (0x02)
#define PPG_MULTI_LED_MODE_RED_PILOT_PA     (0x05)
#define PPG_MULTI_LED_MODE_IR_PILOT_PA      (0x06)
#define PPG_MULTI_LED_MODE_IR_RED_PA        (0x12)
#define PPG_MULTI_LED_MODE_RED_IR_PA        (0x21)
#define PPG_MULTI_LED_MODE_IR_RED_PILOT_PA  (0x56)
#define PPG_MULTI_LED_MODE_RED_IR_PILOT_PA  (0x65)

#define PPG_MULTI_LED_MODE_SLOT1_2          (0)
#define PPG_MULTI_LED_MODE_SLOT3_4          (1)

/********************************************/
/***\name	PPG MULTI LED MODE CONTROL SLOT3,4  */
/*******************************************/
#define PPG_MULTI_LED_MODE_CONTROL3__POS    (0)
#define PPG_MULTI_LED_MODE_CONTROL3__LEN    (3)
#define PPG_MULTI_LED_MODE_CONTROL3__MSK    (0x07)
#define PPG_MULTI_LED_MODE_CONTROL3__REG    (PPG_MULTI_LED_MODE_CONTROL34_ADDR)
#define PPG_MULTI_LED_MODE_CONTROL4__POS    (4)
#define PPG_MULTI_LED_MODE_CONTROL4__LEN    (3)
#define PPG_MULTI_LED_MODE_CONTROL4__MSK    (0x70)
#define PPG_MULTI_LED_MODE_CONTROL4__REG    (PPG_MULTI_LED_MODE_CONTROL34_ADDR)

/*************************************************/
/***\name	PPG PROX INT THRESHOLD POINTER  */
/*************************************************/
#define PPG_PROX_INT_THRESH__REG            (PPG_PROX_INT_THRESH_ADDR)
#define PPG_PROX_INT_THRESH_VALUE           (0x19)


/********************************************/
/***\name	PPG FIFO POINTER  */
/*******************************************/
#define PPG_FIFO_WR_PTR__POS                (0)
#define PPG_FIFO_WR_PTR__LEN                (5)
#define PPG_FIFO_WR_PTR__MSK                (0x1F)
#define PPG_FIFO_WR_PTR__REG                (PPG_FIFO_WR_PTR_ADDR)

#define PPG_FIFO_RD_PTR__POS                (0)
#define PPG_FIFO_RD_PTR__LEN                (5)
#define PPG_FIFO_RD_PTR__MSK                (0x1F)
#define PPG_FIFO_RD_PTR__REG                (PPG_FIFO_RD_PTR_ADDR)

/********************************************/
/***\name	PPG FIFO DATA REGISTER  */
/*******************************************/
#define PPG_FIFO_DATA__REG                  (PPG_FIFO_DATA_REG_ADDR)

/********************************************/
/***\name	PPG FIFO ROLL OVER  */
/*******************************************/
#define PPG_ROLL_OVER__POS                  (4)
#define PPG_ROLL_OVER__LEN                  (1)
#define PPG_ROLL_OVER__MSK                  (0x10)
#define PPG_ROLL_OVER__REG                  (PPG_FIFO_CONFIG_ADDR)
#define PPG_FIFO_ROLL_OVER_EN               (1)

/********************************************/
/***\name	PPG FIFO A FULL  */
/*******************************************/
#define PPG_FIFO_A_FULL__POS                (0)
#define PPG_FIFO_A_FULL__LEN                (4)
#define PPG_FIFO_A_FULL__MSK                (0x0F)
#define PPG_FIFO_A_FULL__REG                (PPG_FIFO_CONFIG_ADDR)
#define PPG_ZERO_NUM_SAMPLES_IN_FIFO        (0)

/********************************************/
/***\name	PPG SAMPLE AVERAGING  */
/*******************************************/
#define PPG_SAMPLE_AVG__POS                 (5)
#define PPG_SAMPLE_AVG__LEN                 (3)
#define PPG_SAMPLE_AVG__MSK                 (0xE0)
#define PPG_SAMPLE_AVG__REG                 (PPG_FIFO_CONFIG_ADDR)

#define PPG_SAMPLE_NO_AVG                   (0)
#define PPG_SAMPLE_AVG2                     (1)
#define PPG_SAMPLE_AVG4                     (2)
#define PPG_SAMPLE_AVG8                     (3)
#define PPG_SAMPLE_AVG16                    (4)
#define PPG_SAMPLE_AVG32                    (5)

/********************************************/
/***\name	PPG MODE CONTROL  */
/*******************************************/
#define PPG_MODE_CONFIG__POS                (0)
#define PPG_MODE_CONFIG__LEN                (3)
#define PPG_MODE_CONFIG__MSK                (0x07)
#define PPG_MODE_CONFIG__REG                (PPG_MODE_CONFIG_ADDR)

#define PPG_HEART_RATE_MODE                 (0x02)
#define PPG_SPO2_MODE                       (0x03)
#define PPG_MULTI_LED_MODE                  (0x07)

/********************************************/
/***\name	PPG RESET CONTROL  */
/*******************************************/
#define PPG_CHIP_RESET__POS                 (6)
#define PPG_CHIP_RESET__LEN                 (1)
#define PPG_CHIP_RESET__MSK                 (0x40)
#define PPG_CHIP_RESET__REG                 (PPG_MODE_CONFIG_ADDR)

#define PPG_CHIP_RESET_EN                   (1)
#define PPG_CHIP_RESET_DELAY                (60)

/********************************************/
/***\name	PPG SHUTDOWN CONTROL  */
/*******************************************/
#define PPG_SHDN_CONTROL__POS               (7)
#define PPG_SHDN_CONTROL__LEN               (1)
#define PPG_SHDN_CONTROL__MSK               (0x80)
#define PPG_SHDN_CONTROL__REG               (PPG_MODE_CONFIG_ADDR)

#define PPG_SHDN_CONTROL_DISABLE            (0)
#define PPG_SHDN_CONTROL_EN                 (1)

/********************************************/
/***\name	PPG SPO2 SAMPLE RATE
 *          CONFIGURATION  */
/*******************************************/
#define PPG_SPO2_SAMPLE_RATE__POS           (2)
#define PPG_SPO2_SAMPLE_RATE__LEN           (3)
#define PPG_SPO2_SAMPLE_RATE__MSK           (0x1C)
#define PPG_SPO2_SAMPLE_RATE__REG           (PPG_SPO2_CONFIG_ADDR)

#define PPG_SPO2_SAMPLE_RATE50              (0)
#define PPG_SPO2_SAMPLE_RATE100             (1)
#define PPG_SPO2_SAMPLE_RATE200             (2)
#define PPG_SPO2_SAMPLE_RATE400             (3)
#define PPG_SPO2_SAMPLE_RATE800             (4)
#define PPG_SPO2_SAMPLE_RATE1000            (5)
#define PPG_SPO2_SAMPLE_RATE1600            (6)
#define PPG_SPO2_SAMPLE_RATE3200            (7)
#define PPG_NUM_SPO2_SAMPLE_RATE            (8)

/********************************************/
/***\name	PPG SPO2 ADC RANGE
 *          CONFIGURATION  */
/*******************************************/
#define PPG_SPO2_ADC_RGE__POS               (5)
#define PPG_SPO2_ADC_RGE__LEN               (2)
#define PPG_SPO2_ADC_RGE__MSK               (0x60)
#define PPG_SPO2_ADC_RGE__REG               (PPG_SPO2_CONFIG_ADDR)

#define PPG_SPO2_ADC_RGE2048                (0)
#define PPG_SPO2_ADC_RGE4096                (1)
#define PPG_SPO2_ADC_RGE8192                (2)
#define PPG_SPO2_ADC_RGE16384               (3)
#define PPG_NUM_SPO2_ADC_RGE                (4)

#define PPG_HR_ADC_RESOLUTION_16BIT         (65535)

/********************************************/
/***\name	PPG SPO2 ADC LED PULSE
 *          WIDTH  */
/*******************************************/
#define PPG_SPO2_LED_PW__POS                (0)
#define PPG_SPO2_LED_PW__LEN                (2)
#define PPG_SPO2_LED_PW__MSK                (0x03)
#define PPG_SPO2_LED_PW__REG                (PPG_SPO2_CONFIG_ADDR)

#define PPG_SPO2_LED_PW15                   (0)
#define PPG_SPO2_LED_PW16                   (1)
#define PPG_SPO2_LED_PW17                   (2)
#define PPG_SPO2_LED_PW18                   (3)

/********************************************/
/***\name	PPG LED PULSE AMPLITUDE */
/*******************************************/
#define PPG_LED1_PA__REG                    (PPG_LED1_PA_ADDR)
#define PPG_LED2_PA__REG                    (PPG_LED2_PA_ADDR)

#define PPG_CURRENT_DECREMENT_STEP          (1) /** Decrementing the value in register by 1 reduces the current by 0.2 mA */
#define PPG_CURRENT_INCREMENT_STEP          (1) /** Incrementing the value in register by 1 increases the current by 0.2 mA */

/********************************************/
/***\name	PPG CHIP ID  */
/*******************************************/
#define PPG_USER_CHIP_ID__REG               (0xFF)
#define PPG_CHIP_ID                         (0x15)

/*****************************************************/
/***\name	ERROR CODES       */
/****************************************************/

#define E_PPG_NULL_PTR                      ((int8)-127)
#define E_PPG_OUT_OF_RANGE                  ((int8)-2)
#define E_PPG_BUSY                          ((int8)-3)
#define	E_PPG_SUCCESS                       ((uint8)0)
#define	E_PPG_ERROR                         ((int8)-1)

/*****************************************************/
/***\name	CONSTANTS       */
/****************************************************/
#define PPG_NULL                            (0)
#define PPG_SINGLE_LED_TOTAL_BYTES          (3)
#define PPG_DATA_BUFFER_PTR_INCREMENT       (1) /**< Increment by 1 since the storage buffer pointer is 16bit */
#define PPG_BOTH_LED_TOTAL_BYTES            (6)
#define PPG_FIFO_X_MSB_DATA                 (0)
#define PPG_FIFO_X_MID_DATA                 (1)
#define PPG_FIFO_X_LSB_DATA                 (2)
#define PPG_FIFO_LENGTH                     (192)
#define PPG_FIFO_DEPTH                      (32)

#define PPG_SHIFT_BIT_POSITION_BY_16_BITS   (16)
#define PPG_SHIFT_BIT_POSITION_BY_14_BITS   (14)
#define PPG_SHIFT_BIT_POSITION_BY_06_BITS   (6)
#define PPG_SHIFT_BIT_POSITION_BY_08_BITS   (8)
#define PPG_SHIFT_BIT_POSITION_BY_02_BITS   (2)

#define PPG_ADC_16BIT_SATURATION_VALUE      (65536)
#define PPG_LED_CURRENT_CONROL_5mA          (0x1F)
#define PPG_LED_CURRENT_CONROL_12mA         (0x3C)

#define PPG_FRAME_COUNT                     (1)
#define PPG_DRIVER_FIFO_HR_LENGTH           (2)
#define PPG_DRIVER_FIFO_IR_LENGTH           (2)
#define PPG_DRIVER_FIFO_MULTI_LED_LENGTH    (4)
#define PPG_DRIVER_FIFO_DEPTH_BYTES         (192)

/* PPG sensor in multi led mode used as proximity sensors */
#define PROX_FIFO_DEPTH_BYTES               (PPG_DRIVER_FIFO_DEPTH_BYTES)
#define PROX_PROXIMITY_BYTES                (PPG_DRIVER_FIFO_MULTI_LED_LENGTH)

/**This refers PPG return type as int8 */
#define PPG_RETURN_FUNCTION_TYPE            int8
#define PPG_MODE_SWITCHING_DELAY            (1)
#define PPG_GEN_READ_WRITE_DELAY            (1)

/** Refers to sensor in/out detection result */
#define PPG_STATUS_IN                       (0)
#define PPG_STATUS_OUT                      (1)

/***************************************************************/
/***\name	STRUCTURE DEFINITIONS                         */
/***************************************************************/
/**
*	@brief ppg structure
*	This structure holds all relevant information about max30102
*/
typedef struct{
uint8 chip_id;/**< chip id of PPG */
uint8 dev_addr;/**< device address of PPG */
PPG_WR_FUNC_PTR;/**< i2c bus write function pointer */
PPG_RD_FUNC_PTR;/**< i2c bus read function pointer */
PPG_BRD_FUNC_PTR;/**< i2c burst write function pointer */
void (*delay_msec)(PPG_MDELAY_DATA_TYPE);/**< delay function pointer */
}ppg_t;
/**
 * @brief  Struct used to store the data from each
 *         sensor in IMU or PPG
 */
typedef struct{
    uint16 frame_count;     /**< Number of samples from Accel */
    uint16 *p_ppg_data;
    bool saturation_flag;   /**< FIFO data saturation glag */
}ppg_data_t;
/**
 * @brief  Struct used to store all the FIFO data from
 *         PPG sensors
 */
typedef struct{
    ppg_data_t IR;
    ppg_data_t red;
    ppg_data_t green;
}ppg_driver_sensor_data_t;

/*****************************************************/
/***\name	OPERATING MODES       */
/****************************************************/
typedef enum
{
    HR_MODE_E = 2,          /**< Red LED      */
    SPO2_MODE_E = 3,        /**< IR + Red LED */
    MULTI_LED_MODE_E = 7,   /**< IR + Red LED */
    NUM_PPG_MODES_E
}ppg_operating_mode_t;

/*****************************************************/
/***\name	LED OPERATING MODES, used to indicates which LED PULSE AMPLITUDE SETTING is used */
/****************************************************/
typedef enum
{
    RED_LED_ONLY = 0,       /**< Red LED only                       */
    IR_LED_ONLY,            /**< IR LED only                        */
    RED_IR_LED,             /**< IR + Red LED                       */
    PILOT_RED_LED_ONLY,     /**< Red LED using PILOT_PA             */
    PILOT_IR_LED_ONLY,      /**< IR LED using PILOT_PA              */
    LED_MODE_NONE,          /**< Indicates any unexpected led mode  */
}ppg_led_mode_t;


#endif /* SPORT_HEALTH_DRIVER_PPG_PPG_H */
