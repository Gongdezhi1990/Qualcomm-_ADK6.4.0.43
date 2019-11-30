 /****************************************************************
* Copyright (C) 2017 Qualcomm Technologies International, Ltd
*
* bmi160.h
* Date 23/05/2017
* Revision 1.0.0
*
*
* Usage (BMI160) IMU Sensor Device Driver include file
*
****************************************************************/
#ifndef SPORT_HEALTH_DRIVER_IMU_BMI160_H
#define SPORT_HEALTH_DRIVER_IMU_BMI160_H
#include "types.h"

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
 * The IMU_DRIVER_WR_FUNC_PTR would equal:
 *
 * IMU_DRIVER_WR_FUNC_PTR int8 (* bus_write)(uint8, uint8, uint8 *, uint8)
 */
#define IMU_DRIVER_WR_FUNC_PTR int8 (*bus_write)(uint8, uint8, uint8 *, uint8)
 /**
  * @brief link macro between API function calls and bus write function
  *        The bus write function can change since this is a
  *        system dependante.
  */
#define IMU_DRIVER_BUS_WRITE_FUNC(dev_addr, reg_addr, reg_data, wr_len)\
				bus_write(dev_addr, reg_addr, reg_data, wr_len)

 /**
 * @brief Define the calling convention for bus communication routine.
 *        This includes types of parameters. This example shows the
 *        configuration for an I2C bus link.
 *
 * For communication function that looks like this:
 * bus_read(uint8 device_addr, uint8 register_addr, uint8 * data, uint8 length);
 *
 * The IMU_DRIVER_RD_FUNC_PTR would equal:
 *
 * IMU_DRIVER_RD_FUNC_PTR int8 (* bus_read)(uint8, uint8, uint8 *, uint8)
 */
#define IMU_DRIVER_RD_FUNC_PTR int8 (*bus_read)(uint8, uint8, uint8 *, uint8)
#define IMU_DRIVER_BRD_FUNC_PTR int8 (*burst_read)(uint8, uint8, uint8 *, uint16)

 /**
  * @brief link macro between API function calls and bus write function
  *        The bus write function can change since this is a
  *        system dependante.
  */
#define IMU_DRIVER_BUS_READ_FUNC(dev_addr, reg_addr, reg_data, r_len)\
                 bus_read(dev_addr, reg_addr, reg_data, r_len)

#define IMU_DRIVER_BURST_READ_FUNC(device_addr, register_addr, register_data, rd_len)\
                 burst_read(device_addr, register_addr, register_data, rd_len)

 /** enable the macro for FIFO functionalities when FIFO is used*/
#define FIFO_ENABLE
 /** enable the macro for when only ACCEL is used*/
#define ONLY_ACCEL
#define IMU_DRIVER_MDELAY_DATA_TYPE              uint16

/** Use BITSERIAL as I2C master */
/*define BITSERIAL (1)*/
/** Enabled for ADK6.1 */
#define MAX_DATA_TRANSFER_LENGTH (252)  /** Header FIFO frame = 7 bytes in BMI160. Therefore, transfer length MOD 7 = 0 */

 /****************************************************************/
 /***\name	Trigger reset including reboot        */
 /****************************************************************/
#define RESET_IMU_DRIVER                         (0xB6)
 /****************************************************************/
 /***\name	CHIP_ID for BMI160        */
 /****************************************************************/
#define IMU_DRIVER_CHIP_ID                       (0xD1)
 /********************************************/
 /***\name	CONSTANTS        */
 /*******************************************/
#define  IMU_DRIVER_INIT_VALUE                   (0)
#define  IMU_DRIVER_ASSIGN_DATA                  (1)
#define  IMU_DRIVER_GEN_READ_WRITE_DATA_LENGTH   (1)
#define  IMU_DRIVER_MAXIMUM_TIMEOUT              (10)

 /** output data rate condition check*/
#define  IMU_DRIVER_OUTPUT_DATA_RATE0            (0)
#define  IMU_DRIVER_OUTPUT_DATA_RATE1            (1)
#define  IMU_DRIVER_OUTPUT_DATA_RATE2            (2)
#define  IMU_DRIVER_OUTPUT_DATA_RATE3            (3)
#define  IMU_DRIVER_OUTPUT_DATA_RATE4            (4)
#define  IMU_DRIVER_OUTPUT_DATA_RATE5            (5)
#define  IMU_DRIVER_OUTPUT_DATA_RATE6            (14)
#define  IMU_DRIVER_OUTPUT_DATA_RATE7            (15)

 /** Accel range check*/
#define IMU_DRIVER_ACCEL_RANGE0                  (3)
#define IMU_DRIVER_ACCEL_RANGE1                  (5)
#define IMU_DRIVER_ACCEL_RANGE2                  (8)
#define IMU_DRIVER_ACCEL_RANGE3                  (12)

#define IMU_DRIVER_ACCEL_FULL_RANGE0             (4)
#define IMU_DRIVER_ACCEL_FULL_RANGE1             (8)
#define IMU_DRIVER_ACCEL_FULL_RANGE2             (16)
#define IMU_DRIVER_ACCEL_FULL_RANGE3             (32)

 /** condition check for reading and writing data*/
#define	IMU_DRIVER_MAX_VALUE_FIFO_HEADER         (1)
#define	IMU_DRIVER_MAX_VALUE_FIFO_ACCEL          (1)
#define	IMU_DRIVER_MAX_VALUE_FIFO_GYRO           (1)
#define	IMU_DRIVER_MAX_VALUE_NO_MOTION           (1)

#define IMU_DRIVER_MAX_GYRO_WAKEUP_TRIGGER       (3)
#define IMU_DRIVER_MAX_GYRO_BW                   (3)
#define IMU_DRIVER_MAX_ACCEL_BW                  (7)

#define IMU_DRIVER_MAX_GYRO_RANGE                (4)
#define IMU_DRIVER_MAX_GYRO_SLEEP_TRIGGER        (7)
#define IMU_DRIVER_MAX_UNDER_SAMPLING            (1)
#define IMU_DRIVER_MAX_ACCEL_OUTPUT_DATA_RATE    (12)

#ifdef FIFO_ENABLE
 /** FIFO index definitions*/
#define IMU_DRIVER_FIFO_X_LSB_DATA               (0)
#define IMU_DRIVER_FIFO_X_MSB_DATA               (1)
#define IMU_DRIVER_FIFO_Y_LSB_DATA               (2)
#define IMU_DRIVER_FIFO_Y_MSB_DATA               (3)
#define IMU_DRIVER_FIFO_Z_LSB_DATA               (4)
#define IMU_DRIVER_FIFO_Z_MSB_DATA               (5)
 /** FIFO Gyro and Accel  definition*/
#define IMU_DRIVER_GA_FIFO_G_X_LSB               (0)
#define IMU_DRIVER_GA_FIFO_G_X_MSB               (1)
#define IMU_DRIVER_GA_FIFO_G_Y_LSB               (2)
#define IMU_DRIVER_GA_FIFO_G_Y_MSB               (3)
#define IMU_DRIVER_GA_FIFO_G_Z_LSB               (4)
#define IMU_DRIVER_GA_FIFO_G_Z_MSB               (5)
#define IMU_DRIVER_GA_FIFO_A_X_LSB               (6)
#define IMU_DRIVER_GA_FIFO_A_X_MSB               (7)
#define IMU_DRIVER_GA_FIFO_A_Y_LSB               (8)
#define IMU_DRIVER_GA_FIFO_A_Y_MSB               (9)
#define IMU_DRIVER_GA_FIFO_A_Z_LSB               (10)
#define IMU_DRIVER_GA_FIFO_A_Z_MSB               (11)
 /** FIFO length definitions*/
#define IMU_DRIVER_FIFO_HEADER_LENGTH            (1)
#define IMU_DRIVER_FIFO_A_LENGTH                 (6)
#define IMU_DRIVER_FIFO_G_LENGTH                 (6)
#define IMU_DRIVER_FIFO_AG_LENGTH                (12)
#define	IMU_DRIVER_FIFO_LENGTH_LSB_BYTE          (0)
#define	IMU_DRIVER_FIFO_LENGTH_MSB_BYTE          (1)

#endif

 /** bus read and write length for Accel and Gyro*/
#define IMU_DRIVER_GYRO_DATA_LENGTH              (2)
#define IMU_DRIVER_GYRO_XYZ_DATA_LENGTH          (6)
#define IMU_DRIVER_ACCEL_DATA_LENGTH             (2)
#define IMU_DRIVER_ACCEL_XYZ_DATA_LENGTH         (6)
#define IMU_DRIVER_FIFO_DATA_LENGTH              (2)
 /** Delay definitions*/
#define IMU_DRIVER_GEN_READ_WRITE_DELAY          (1)
#define IMU_DRIVER_CHIP_RESET_DELAY              ((uint16)60)

 /*****************************************************/
 /***\name	ARRAY SIZE DEFINITIONS      */
 /****************************************************/
#define	IMU_DRIVER_ACCEL_X_DATA_SIZE             (2)
#define	IMU_DRIVER_ACCEL_Y_DATA_SIZE             (2)
#define	IMU_DRIVER_ACCEL_Z_DATA_SIZE             (2)
#define	IMU_DRIVER_ACCEL_XYZ_DATA_SIZE           (6)

#define	IMU_DRIVER_GYRO_X_DATA_SIZE              (2)
#define	IMU_DRIVER_GYRO_Y_DATA_SIZE              (2)
#define	IMU_DRIVER_GYRO_Z_DATA_SIZE              (2)
#define	IMU_DRIVER_GYRO_XYZ_DATA_SIZE            (6)

#define	IMU_DRIVER_FIFO_DATA_SIZE                (2)
 /*****************************************************/
 /***\name	ARRAY PARAMETER DEFINITIONS      */
 /****************************************************/
#define IMU_DRIVER_GYRO_X_LSB_BYTE               (0)
#define IMU_DRIVER_GYRO_X_MSB_BYTE               (1)
#define IMU_DRIVER_GYRO_Y_LSB_BYTE               (0)
#define IMU_DRIVER_GYRO_Y_MSB_BYTE               (1)
#define IMU_DRIVER_GYRO_Z_LSB_BYTE               (0)
#define IMU_DRIVER_GYRO_Z_MSB_BYTE               (1)
#define IMU_DRIVER_DATA_FRAME_GYRO_X_LSB_BYTE    (0)
#define IMU_DRIVER_DATA_FRAME_GYRO_X_MSB_BYTE    (1)
#define IMU_DRIVER_DATA_FRAME_GYRO_Y_LSB_BYTE    (2)
#define IMU_DRIVER_DATA_FRAME_GYRO_Y_MSB_BYTE    (3)
#define IMU_DRIVER_DATA_FRAME_GYRO_Z_LSB_BYTE    (4)
#define IMU_DRIVER_DATA_FRAME_GYRO_Z_MSB_BYTE    (5)

#define IMU_DRIVER_ACCEL_X_LSB_BYTE              (0)
#define IMU_DRIVER_ACCEL_X_MSB_BYTE              (1)
#define IMU_DRIVER_ACCEL_Y_LSB_BYTE              (0)
#define IMU_DRIVER_ACCEL_Y_MSB_BYTE              (1)
#define IMU_DRIVER_ACCEL_Z_LSB_BYTE              (0)
#define IMU_DRIVER_ACCEL_Z_MSB_BYTE              (1)
#define IMU_DRIVER_DATA_FRAME_ACCEL_X_LSB_BYTE   (0)
#define IMU_DRIVER_DATA_FRAME_ACCEL_X_MSB_BYTE   (1)
#define IMU_DRIVER_DATA_FRAME_ACCEL_Y_LSB_BYTE   (2)
#define IMU_DRIVER_DATA_FRAME_ACCEL_Y_MSB_BYTE   (3)
#define IMU_DRIVER_DATA_FRAME_ACCEL_Z_LSB_BYTE   (4)
#define IMU_DRIVER_DATA_FRAME_ACCEL_Z_MSB_BYTE   (5)
 /*****************************************************/
 /***\name	ERROR CODES       */
 /****************************************************/

#define E_IMU_DRIVER_NULL_PTR                    ((int8)-127)
#define E_IMU_DRIVER_COMM_RES                    ((int8)-1)
#define E_IMU_DRIVER_OUT_OF_RANGE                ((int8)-2)
#define E_IMU_DRIVER_BUSY                        ((int8)-3)
#define	DRIVER_SUCCESS                           ((int8)0)
#define	DRIVER_ERROR                             ((int8)-1)

 /** Constants */
#define IMU_DRIVER_NULL                          (0)
#define IMU_DRIVER_DELAY_SETTLING_TIME           (5)
 /** This refers BMI160 return type as int8 */
#define IMU_DRIVER_RETURN_FUNCTION_TYPE          int8
 /*****************************************************/
 /***\name	REGISTER DEFINITIONS       */
 /****************************************************/
 /********************/
 /***\name CHIP ID */
 /********************/
#define IMU_DRIVER_USER_CHIP_ID_ADDR             (0x00)
 /********************/
 /***\name ERROR STATUS */
 /********************/
#define IMU_DRIVER_USER_ERROR_ADDR               (0X02)
 /********************/
 /***\name POWER MODE STATUS */
 /********************/
#define IMU_DRIVER_USER_PMU_STAT_ADDR			 (0X03)
#define IMU_DRIVER_ACCEL_GYRO_PMU_MASK			 (0X3C)
#define IMU_DRIVER_NORMAL_MODE                   (0X14)
 /********************/
 /***\name FIFO REGISTERS */
 /********************/
#define IMU_DRIVER_USER_FIFO_LENGTH_0_ADDR       (0X22)
#define IMU_DRIVER_USER_FIFO_LENGTH_1_ADDR		 (0X23)
#define IMU_DRIVER_USER_FIFO_DATA_ADDR			 (0X24)
 /****************************************************/
 /***\name ACCEL CONFIG REGISTERS  FOR ODR, BANDWIDTH AND UNDERSAMPLING*/
 /*******************************************************/
#define IMU_DRIVER_USER_ACCEL_CONFIG_ADDR        (0X40)
 /********************/
 /***\name ACCEL RANGE */
 /********************/
#define IMU_DRIVER_USER_ACCEL_RANGE_ADDR         (0X41)
 /****************************************************/
 /***\name GYRO CONFIG REGISTERS  FOR ODR AND BANDWIDTH */
 /*******************************************************/
#define IMU_DRIVER_USER_GYRO_CONFIG_ADDR         (0X42)
 /********************/
 /***\name GYRO RANGE */
 /********************/
#define IMU_DRIVER_USER_GYRO_RANGE_ADDR          (0X43)
#ifdef FIFO_ENABLE
 /****************************************************/
 /***\name FIFO CONFIG REGISTERS*/
 /*******************************************************/
#define IMU_DRIVER_USER_FIFO_CONFIG_0_ADDR       (0X46)
#define IMU_DRIVER_USER_FIFO_CONFIG_1_ADDR       (0X47)
#endif
 /****************************************************/
 /***\name GYRO POWER MODE TRIGGER REGISTER */
 /*******************************************************/
#define IMU_DRIVER_USER_PMU_TRIGGER_ADDR         (0X6C)
 /****************************************************/
 /***\name COMMAND REGISTER*/
 /*******************************************************/
#define IMU_DRIVER_CMD_COMMANDS_ADDR             (0X7E)

 /*****************************************************/
 /***\name	SHIFT VALUE DEFINITION       */
 /****************************************************/
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_01_BIT  (1)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_02_BITS (2)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_03_BITS (3)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_04_BITS (4)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_05_BITS (5)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_06_BITS (6)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_07_BITS (7)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_08_BITS (8)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_09_BITS (9)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_12_BITS (12)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_13_BITS (13)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_14_BITS (14)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_15_BITS (15)
#define IMU_DRIVER_SHIFT_BIT_POSITION_BY_16_BITS (16)

/*****************************************************/
/***\name	SETTLE TIME VALUE DEFINITION       */
/****************************************************/
#define IMU_SETTLE_TIME_TRANS_TO_HI_RATE_MS  (130)
#define IMU_SETTLE_TIME_TRANS_TO_LO_RATE_MS  (260)

 /*****************************************************/
 /***\name	BMM150 DEFINITION */
 /****************************************************/
#define IMU_DRIVER_FIFO_FRAME_CNT                (148)  /** Word aligned */
#define	IMU_DRIVER_FRAME_COUNT                   (1)
/***************************************************/
/**\name
INTERRUPT ENABLE 0 */
/******************************************************/
#define IMU_DRIVER_ANY_MOTION_X_ENABLE       (0)
#define IMU_DRIVER_ANY_MOTION_Y_ENABLE       (1)
#define IMU_DRIVER_ANY_MOTION_Z_ENABLE       (2)
/***************************************************/
/**\name
INTERRUPT ENABLE 2 */
/******************************************************/
#define  IMU_DRIVER_NOMOTION_X_ENABLE	(0)
#define  IMU_DRIVER_NOMOTION_Y_ENABLE	(1)
#define  IMU_DRIVER_NOMOTION_Z_ENABLE	(2)
/***************************************************/
/**\name
INTERRUPT MAPS */
/******************************************************/
#define IMU_DRIVER_INTR1_MAP_ANY_MOTION		(0)
#define IMU_DRIVER_INTR2_MAP_ANY_MOTION		(1)
#define IMU_DRIVER_INTR1_MAP_NOMO			(0)
#define IMU_DRIVER_INTR2_MAP_NOMO			(1)
/***************************************************/
/**\name
INTERRUPT THRESHOLD, DURATION CONFIGURATION REGISTERS*/
/******************************************************/
#define IMU_DRIVER_USER_INTR_MOTION_0_ADDR			(0X5F)
#define IMU_DRIVER_USER_INTR_MOTION_1_ADDR			(0X60)
#define IMU_DRIVER_USER_INTR_MOTION_2_ADDR			(0X61)
#define IMU_DRIVER_USER_INTR_MOTION_3_ADDR			(0X62)
/***************************************************/
/**\name MAP INTERRUPT 1 and 2 REGISTERS*/
/******************************************************/
#define IMU_DRIVER_USER_INTR_MAP_0_ADDR				(0X55)
#define IMU_DRIVER_USER_INTR_MAP_1_ADDR				(0X56)
#define IMU_DRIVER_USER_INTR_MAP_2_ADDR				(0X57)
/***************************************************/
/**\name INTERRUPT ENABLE REGISTERS*/
/******************************************************/
#define IMU_DRIVER_USER_INTR_ENABLE_0_ADDR			(0X50)
#define IMU_DRIVER_USER_INTR_ENABLE_1_ADDR			(0X51)
#define IMU_DRIVER_USER_INTR_ENABLE_2_ADDR			(0X52)
#define IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR			(0X53)
/***************************************************/
/**\name INTERRUPT STATUS REGISTERS*/
/******************************************************/
#define IMU_DRIVER_USER_INTR_STAT_0_ADDR			(0X1C)
#define IMU_DRIVER_USER_INTR_STAT_1_ADDR			(0X1D)
#define IMU_DRIVER_USER_INTR_STAT_2_ADDR			(0X1E)
#define IMU_DRIVER_USER_INTR_STAT_3_ADDR			(0X1F)
/***************************************************/
/**\name IN OUT CONTROL*/
/******************************************************/
#define IMU_DRIVER_INTR1_EDGE_CTRL			(0)
#define IMU_DRIVER_INTR2_EDGE_CTRL			(1)
#define IMU_DRIVER_INTR1_LEVEL				(0)
#define IMU_DRIVER_INTR2_LEVEL				(1)
#define IMU_DRIVER_INTR1_OUTPUT_TYPE		(0)
#define IMU_DRIVER_INTR2_OUTPUT_TYPE		(1)
#define IMU_DRIVER_INTR1_OUTPUT_ENABLE		(0)
#define IMU_DRIVER_INTR2_OUTPUT_ENABLE		(1)
/**************************************************/
/**\name	INTERRUPT EDGE TRIGGER ENABLE    */
/*************************************************/
#define IMU_DRIVER_EDGE		(0x01)
#define IMU_DRIVER_LEVEL	(0x00)
/**************************************************/
/**\name	INTERRUPT LEVEL ENABLE    */
/*************************************************/
#define IMU_DRIVER_LEVEL_LOW		(0x00)
#define IMU_DRIVER_LEVEL_HIGH		(0x01)
/**************************************************/
/**\name	INTERRUPT OUTPUT TYPE    */
/*************************************************/
#define IMU_DRIVER_OPEN_DRAIN	(0x01)
#define IMU_DRIVER_PUSH_PULL	(0x00)
/**************************************************/
/**\name	INTERRUPT OUTPUT ENABLE    */
/*************************************************/
#define IMU_DRIVER_INPUT	(0x00)
#define IMU_DRIVER_OUTPUT	(0x01)
/**************************************************/
/**\name	SLOW MOTION/ NO MOTION SELECT   */
/*************************************************/
#define SLOW_MOTION		(0x00)
#define NO_MOTION		(0x01)
/**************************************************/
/**\name	ANY/NO MOTION INTERRUPT THRESHOLD   */
/*************************************************/
#define ANY_MOTION_INTERRUPT_TH		(0x01)   /**< ANY_MOTION_INTERRUPT_TH + 1 = 2 data points */
#define NO_MOTION_INTERRUPT_TH   	(0x10)   /**< NO_MOTION_INTERRUPT_TH * 1.28 sec = 20.48 sec */
 /***************************************************************/
 /***\name	STRUCTURE DEFINITIONS                         */
 /***************************************************************/
 /**!
*	@brief bmi160 structure
*	This structure holds all relevant information about bmi160
*/
typedef struct{
uint8 chip_id; /**< chip id of BMI160 */
uint8 dev_addr; /**< device address of BMI160 */
int8 mag_manual_enable; /**< used to check the Mag manual/auto mode status */
IMU_DRIVER_WR_FUNC_PTR; /**< bus write function pointer */
IMU_DRIVER_RD_FUNC_PTR; /**< bus read function pointer */
IMU_DRIVER_BRD_FUNC_PTR; /**< burst read function pointer */
void (*delay_msec)(IMU_DRIVER_MDELAY_DATA_TYPE); /**< delay function pointer */
}imu_t;
 /***************************************************************/
 /***\name	USER DATA REGISTERS DEFINITION START    */
 /***************************************************************/

/**************************************************************/
/**\name	ANY MOTION XYZ AXIS ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_En_0 Description - Reg Addr --> 0x50, Bit -->0 */
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__POS               (0)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__MSK               (0x01)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_X_ENABLE__REG	              \
(IMU_DRIVER_USER_INTR_ENABLE_0_ADDR)

/* Int_En_0 Description - Reg Addr --> 0x50, Bit -->1 */
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__POS               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__MSK               (0x02)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Y_ENABLE__REG	          \
(IMU_DRIVER_USER_INTR_ENABLE_0_ADDR)

/* Int_En_0 Description - Reg Addr --> 0x50, Bit -->2 */
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__POS               (2)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__MSK               (0x04)
#define IMU_DRIVER_USER_INTR_ENABLE_0_ANY_MOTION_Z_ENABLE__REG	            \
(IMU_DRIVER_USER_INTR_ENABLE_0_ADDR)
/**************************************************************/
/**\name	ANY_MOTION INTERRUPT STATUS LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Status_0 Description - Reg Addr --> 0x1C, Bit --> 2 */
#define IMU_DRIVER_USER_INTR_STAT_0_ANY_MOTION__POS           (2)
#define IMU_DRIVER_USER_INTR_STAT_0_ANY_MOTION__LEN           (1)
#define IMU_DRIVER_USER_INTR_STAT_0_ANY_MOTION__MSK          (0x04)
#define IMU_DRIVER_USER_INTR_STAT_0_ANY_MOTION__REG          \
        (IMU_DRIVER_USER_INTR_STAT_0_ADDR)
/**************************************************************/
/**\name	NO MOTION XYZ ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_En_2 Description - Reg Addr --> (0x52), Bit -->0 */
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE__POS               (0)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE__MSK               (0x01)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_X_ENABLE__REG	  \
(IMU_DRIVER_USER_INTR_ENABLE_2_ADDR)

/* Int_En_2 Description - Reg Addr --> (0x52), Bit -->1 */
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE__POS               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE__MSK               (0x02)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Y_ENABLE__REG	  \
(IMU_DRIVER_USER_INTR_ENABLE_2_ADDR)

/* Int_En_2 Description - Reg Addr --> (0x52), Bit -->2 */
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE__POS               (2)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE__MSK               (0x04)
#define IMU_DRIVER_USER_INTR_ENABLE_2_NOMOTION_Z_ENABLE__REG	  \
(IMU_DRIVER_USER_INTR_ENABLE_2_ADDR)
/**************************************************************/
/**\name	NO MOTION INTERRUPT STATUS LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Status_1 Description - Reg Addr --> 0x1D, Bit --> 7 */
#define IMU_DRIVER_USER_INTR_STAT_1_NOMOTION_INTR__POS               (7)
#define IMU_DRIVER_USER_INTR_STAT_1_NOMOTION_INTR__LEN               (1)
#define IMU_DRIVER_USER_INTR_STAT_1_NOMOTION_INTR__MSK               (0x80)
#define IMU_DRIVER_USER_INTR_STAT_1_NOMOTION_INTR__REG               \
        (IMU_DRIVER_USER_INTR_STAT_1_ADDR)
/**************************************************************/
/**\name	INTERRUPT MAPPING OF ANY MOTION_G LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Map_0 Description - Reg Addr --> 0x55, Bit -->2 */
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__POS               (2)
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__LEN               (1)
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__MSK               (0x04)
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_ANY_MOTION__REG            \
(IMU_DRIVER_USER_INTR_MAP_0_ADDR)
/**************************************************************/
/**\name	INTERRUPT1 MAPPING OF NO MOTION LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Map_0 Description - Reg Addr --> 0x55, Bit -->3 */
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__POS               (3)
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__LEN               (1)
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__MSK               (0x08)
#define IMU_DRIVER_USER_INTR_MAP_0_INTR1_NOMOTION__REG (IMU_DRIVER_USER_INTR_MAP_0_ADDR)
/**************************************************************/
/**\name	INTERRUPT2 MAPPING OF ANY MOTION LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Map_2 Description - Reg Addr --> 0x57, Bit -->2 */
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__POS      (2)
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__LEN      (1)
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__MSK     (0x04)
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_ANY_MOTION__REG     \
(IMU_DRIVER_USER_INTR_MAP_2_ADDR)
/**************************************************************/
/**\name	INTERRUPT2 MAPPING OF NO MOTION LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Map_2 Description - Reg Addr --> 0x57, Bit -->3 */
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__POS               (3)
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__LEN               (1)
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__MSK               (0x08)
#define IMU_DRIVER_USER_INTR_MAP_2_INTR2_NOMOTION__REG (IMU_DRIVER_USER_INTR_MAP_2_ADDR)
/**************************************************************/
/**\name	ANY MOTION DURATION LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Motion_0 Description - Reg Addr --> 0x5f, Bit --> 0...1 */
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__POS               (0)
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__LEN               (2)
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__MSK               (0x03)
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_ANY_MOTION_DURN__REG               \
        (IMU_DRIVER_USER_INTR_MOTION_0_ADDR)
/**************************************************************/
/**\name	SLOW/NO MOTION DURATION LENGTH, POSITION AND MASK*/
/**************************************************************/
    /* Int_Motion_0 Description - Reg Addr --> 0x5f, Bit --> 2...7 */
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__POS      (2)
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__LEN      (6)
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__MSK      (0xFC)
#define IMU_DRIVER_USER_INTR_MOTION_0_INTR_SLOW_NO_MOTION_DURN__REG       \
        (IMU_DRIVER_USER_INTR_MOTION_0_ADDR)
/**************************************************************/
/**\name	ANY MOTION THRESHOLD LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Motion_1 Description - Reg Addr --> (0x60), Bit --> 0...7 */
#define IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__POS      (0)
#define IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__LEN      (8)
#define IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__MSK      (0xFF)
#define IMU_DRIVER_USER_INTR_MOTION_1_INTR_ANY_MOTION_THRES__REG               \
        (IMU_DRIVER_USER_INTR_MOTION_1_ADDR)
/**************************************************************/
/**\name	SLOW/NO MOTION THRESHOLD LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Motion_2 Description - Reg Addr --> 0x61, Bit --> 0...7 */
#define IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES__POS       (0)
#define IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES__LEN       (8)
#define IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES__MSK       (0xFF)
#define IMU_DRIVER_USER_INTR_MOTION_2_INTR_SLOW_NO_MOTION_THRES__REG       \
        (IMU_DRIVER_USER_INTR_MOTION_2_ADDR)
/**************************************************************/
/**\name	SLOW/NO MOTION SELECT LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Motion_3 Description - Reg Addr --> (0x62), Bit --> 0 */
#define IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__POS	(0)
#define IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__LEN	(1)
#define IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__MSK	(0x01)
#define IMU_DRIVER_USER_INTR_MOTION_3_INTR_SLOW_NO_MOTION_SELECT__REG   \
(IMU_DRIVER_USER_INTR_MOTION_3_ADDR)
/**************************************************************/
/**\name	EDGE CONTROL ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->0 */
#define IMU_DRIVER_USER_INTR1_EDGE_CTRL__POS               (0)
#define IMU_DRIVER_USER_INTR1_EDGE_CTRL__LEN               (1)
#define IMU_DRIVER_USER_INTR1_EDGE_CTRL__MSK               (0x01)
#define IMU_DRIVER_USER_INTR1_EDGE_CTRL__REG		\
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
/**************************************************************/
/**\name	LEVEL CONTROL ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->1 */
#define IMU_DRIVER_USER_INTR1_LEVEL__POS               (1)
#define IMU_DRIVER_USER_INTR1_LEVEL__LEN               (1)
#define IMU_DRIVER_USER_INTR1_LEVEL__MSK               (0x02)
#define IMU_DRIVER_USER_INTR1_LEVEL__REG               \
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
/**************************************************************/
/**\name	OUTPUT TYPE ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->2 */
#define IMU_DRIVER_USER_INTR1_OUTPUT_TYPE__POS               (2)
#define IMU_DRIVER_USER_INTR1_OUTPUT_TYPE__LEN               (1)
#define IMU_DRIVER_USER_INTR1_OUTPUT_TYPE__MSK               (0x04)
#define IMU_DRIVER_USER_INTR1_OUTPUT_TYPE__REG               \
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
/**************************************************************/
/**\name	OUTPUT TYPE ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->3 */
#define IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE__POS               (3)
#define IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE__LEN               (1)
#define IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE__MSK               (0x08)
#define IMU_DRIVER_USER_INTR1_OUTPUT_ENABLE__REG		\
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
/**************************************************************/
/**\name	EDGE CONTROL ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->4 */
#define IMU_DRIVER_USER_INTR2_EDGE_CTRL__POS               (4)
#define IMU_DRIVER_USER_INTR2_EDGE_CTRL__LEN               (1)
#define IMU_DRIVER_USER_INTR2_EDGE_CTRL__MSK               (0x10)
#define IMU_DRIVER_USER_INTR2_EDGE_CTRL__REG		\
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
/**************************************************************/
/**\name	LEVEL CONTROL ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->5 */
#define IMU_DRIVER_USER_INTR2_LEVEL__POS               (5)
#define IMU_DRIVER_USER_INTR2_LEVEL__LEN               (1)
#define IMU_DRIVER_USER_INTR2_LEVEL__MSK               (0x20)
#define IMU_DRIVER_USER_INTR2_LEVEL__REG               \
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
/**************************************************************/
/**\name	OUTPUT TYPE ENABLE LENGTH, POSITION AND MASK*/
/**************************************************************/
/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->6 */
#define IMU_DRIVER_USER_INTR2_OUTPUT_TYPE__POS               (6)
#define IMU_DRIVER_USER_INTR2_OUTPUT_TYPE__LEN               (1)
#define IMU_DRIVER_USER_INTR2_OUTPUT_TYPE__MSK               (0x40)
#define IMU_DRIVER_USER_INTR2_OUTPUT_TYPE__REG               \
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)

/* Int_Out_Ctrl Description - Reg Addr --> 0x53, Bit -->7 */
#define IMU_DRIVER_USER_INTR2_OUTPUT_EN__POS               (7)
#define IMU_DRIVER_USER_INTR2_OUTPUT_EN__LEN               (1)
#define IMU_DRIVER_USER_INTR2_OUTPUT_EN__MSK               (0x80)
#define IMU_DRIVER_USER_INTR2_OUTPUT_EN__REG		\
(IMU_DRIVER_USER_INTR_OUT_CTRL_ADDR)
 /***************************************************************/
 /***\name	CHIP ID LENGTH, POSITION AND MASK    */
 /***************************************************************/
 /** Chip ID Description - Reg Addr --> (0x00), Bit --> 0...7 */
#define IMU_DRIVER_USER_CHIP_ID__POS                             (0)
#define IMU_DRIVER_USER_CHIP_ID__MSK                             (0xFF)
#define IMU_DRIVER_USER_CHIP_ID__LEN                             (8)
#define IMU_DRIVER_USER_CHIP_ID__REG                             (IMU_DRIVER_USER_CHIP_ID_ADDR)
 /***************************************************************/
 /***\name	GYRO POWER MODE LENGTH, POSITION AND MASK    */
 /***************************************************************/
 /** PMU_Status Description of GYRO - Reg Addr --> (0x03), Bit --> 3...2 */
#define IMU_DRIVER_USER_GYRO_POWER_MODE_STAT__POS                (2)
#define IMU_DRIVER_USER_GYRO_POWER_MODE_STAT__LEN                (2)
#define IMU_DRIVER_USER_GYRO_POWER_MODE_STAT__MSK                (0x0C)
#define IMU_DRIVER_USER_GYRO_POWER_MODE_STAT__REG                (IMU_DRIVER_USER_PMU_STAT_ADDR)
 /***************************************************************/
 /***\name	ACCEL POWER MODE LENGTH, POSITION AND MASK    */
 /***************************************************************/
 /** PMU_Status Description of ACCEL - Reg Addr --> (0x03), Bit --> 5...4 */
#define IMU_DRIVER_USER_ACCEL_POWER_MODE_STAT__POS               (4)
#define IMU_DRIVER_USER_ACCEL_POWER_MODE_STAT__LEN               (2)
#define IMU_DRIVER_USER_ACCEL_POWER_MODE_STAT__MSK               (0x30)
#define IMU_DRIVER_USER_ACCEL_POWER_MODE_STAT__REG               (IMU_DRIVER_USER_PMU_STAT_ADDR)
 /***************************************************************/
 /***\name	GYRO DATA XYZ LENGTH, POSITION AND MASK    */
 /***************************************************************/
 /** GYR_X (LSB) Description - Reg Addr --> (0x0C), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_8_GYRO_X_LSB__POS                   (0)
#define IMU_DRIVER_USER_DATA_8_GYRO_X_LSB__LEN                   (8)
#define IMU_DRIVER_USER_DATA_8_GYRO_X_LSB__MSK                   (0xFF)
#define IMU_DRIVER_USER_DATA_8_GYRO_X_LSB__REG                   (IMU_DRIVER_USER_DATA_8_ADDR)

 /** GYR_X (MSB) Description - Reg Addr --> (0x0D), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_9_GYRO_X_MSB__POS                   (0)
#define IMU_DRIVER_USER_DATA_9_GYRO_X_MSB__LEN                   (8)
#define IMU_DRIVER_USER_DATA_9_GYRO_X_MSB__MSK                   (0xFF)
#define IMU_DRIVER_USER_DATA_9_GYRO_X_MSB__REG                   (IMU_DRIVER_USER_DATA_9_ADDR)

 /** GYR_Y (LSB) Description - Reg Addr --> 0x0E, Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_10_GYRO_Y_LSB__POS                  (0)
#define IMU_DRIVER_USER_DATA_10_GYRO_Y_LSB__LEN                  (8)
#define IMU_DRIVER_USER_DATA_10_GYRO_Y_LSB__MSK                  (0xFF)
#define IMU_DRIVER_USER_DATA_10_GYRO_Y_LSB__REG                  (IMU_DRIVER_USER_DATA_10_ADDR)

 /** GYR_Y (MSB) Description - Reg Addr --> (0x0F), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_11_GYRO_Y_MSB__POS                  (0)
#define IMU_DRIVER_USER_DATA_11_GYRO_Y_MSB__LEN                  (8)
#define IMU_DRIVER_USER_DATA_11_GYRO_Y_MSB__MSK                  (0xFF)
#define IMU_DRIVER_USER_DATA_11_GYRO_Y_MSB__REG                  (IMU_DRIVER_USER_DATA_11_ADDR)

 /** GYR_Z (LSB) Description - Reg Addr --> (0x10), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_12_GYRO_Z_LSB__POS                  (0)
#define IMU_DRIVER_USER_DATA_12_GYRO_Z_LSB__LEN                  (8)
#define IMU_DRIVER_USER_DATA_12_GYRO_Z_LSB__MSK                  (0xFF)
#define IMU_DRIVER_USER_DATA_12_GYRO_Z_LSB__REG                  (IMU_DRIVER_USER_DATA_12_ADDR)

 /** GYR_Z (MSB) Description - Reg Addr --> (0x11), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_13_GYRO_Z_MSB__POS                  (0)
#define IMU_DRIVER_USER_DATA_13_GYRO_Z_MSB__LEN                  (8)
#define IMU_DRIVER_USER_DATA_13_GYRO_Z_MSB__MSK                  (0xFF)
#define IMU_DRIVER_USER_DATA_13_GYRO_Z_MSB__REG                  (IMU_DRIVER_USER_DATA_13_ADDR)
 /***************************************************************/
 /***\name	ACCEL DATA XYZ LENGTH, POSITION AND MASK    */
 /***************************************************************/
 /** ACC_X (LSB) Description - Reg Addr --> (0x12), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_14_ACCEL_X_LSB__POS                 (0)
#define IMU_DRIVER_USER_DATA_14_ACCEL_X_LSB__LEN                 (8)
#define IMU_DRIVER_USER_DATA_14_ACCEL_X_LSB__MSK                 (0xFF)
#define IMU_DRIVER_USER_DATA_14_ACCEL_X_LSB__REG                 (IMU_DRIVER_USER_DATA_14_ADDR)

 /** ACC_X (MSB) Description - Reg Addr --> 0x13, Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_15_ACCEL_X_MSB__POS                 (0)
#define IMU_DRIVER_USER_DATA_15_ACCEL_X_MSB__LEN                 (8)
#define IMU_DRIVER_USER_DATA_15_ACCEL_X_MSB__MSK                 (0xFF)
#define IMU_DRIVER_USER_DATA_15_ACCEL_X_MSB__REG                 (IMU_DRIVER_USER_DATA_15_ADDR)

 /** ACC_Y (LSB) Description - Reg Addr --> (0x14), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_16_ACCEL_Y_LSB__POS                 (0)
#define IMU_DRIVER_USER_DATA_16_ACCEL_Y_LSB__LEN                 (8)
#define IMU_DRIVER_USER_DATA_16_ACCEL_Y_LSB__MSK                 (0xFF)
#define IMU_DRIVER_USER_DATA_16_ACCEL_Y_LSB__REG                 (IMU_DRIVER_USER_DATA_16_ADDR)

 /** ACC_Y (MSB) Description - Reg Addr --> (0x15), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_17_ACCEL_Y_MSB__POS                 (0)
#define IMU_DRIVER_USER_DATA_17_ACCEL_Y_MSB__LEN                 (8)
#define IMU_DRIVER_USER_DATA_17_ACCEL_Y_MSB__MSK                 (0xFF)
#define IMU_DRIVER_USER_DATA_17_ACCEL_Y_MSB__REG                 (IMU_DRIVER_USER_DATA_17_ADDR)

 /** ACC_Z (LSB) Description - Reg Addr --> 0x16, Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_18_ACCEL_Z_LSB__POS                 (0)
#define IMU_DRIVER_USER_DATA_18_ACCEL_Z_LSB__LEN                 (8)
#define IMU_DRIVER_USER_DATA_18_ACCEL_Z_LSB__MSK                 (0xFF)
#define IMU_DRIVER_USER_DATA_18_ACCEL_Z_LSB__REG                 (IMU_DRIVER_USER_DATA_18_ADDR)

 /** ACC_Z (MSB) Description - Reg Addr --> (0x17), Bit --> 0...7 */
#define IMU_DRIVER_USER_DATA_19_ACCEL_Z_MSB__POS                 (0)
#define IMU_DRIVER_USER_DATA_19_ACCEL_Z_MSB__LEN                 (8)
#define IMU_DRIVER_USER_DATA_19_ACCEL_Z_MSB__MSK                 (0xFF)
#define IMU_DRIVER_USER_DATA_19_ACCEL_Z_MSB__REG                 (IMU_DRIVER_USER_DATA_19_ADDR)

#ifdef FIFO_ENABLE
 /***************************************************************/
 /***\name	FIFO BYTE COUNTER LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Fifo_Length0 Description - Reg Addr --> 0x22, Bit --> 0...7 */
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_LSB__POS               (0)
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_LSB__LEN               (8)
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_LSB__MSK               (0xFF)
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_LSB__REG               (IMU_DRIVER_USER_FIFO_LENGTH_0_ADDR)

 /**Fifo_Length1 Description - Reg Addr --> 0x23, Bit --> 0...2 */
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_MSB__POS               (0)
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_MSB__LEN               (3)
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_MSB__MSK               (0x07)
#define IMU_DRIVER_USER_FIFO_BYTE_COUNTER_MSB__REG               (IMU_DRIVER_USER_FIFO_LENGTH_1_ADDR)

 /***************************************************************/
 /***\name	FIFO DATA LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Fifo_Data Description - Reg Addr --> 0x24, Bit --> 0...7 */
#define IMU_DRIVER_USER_FIFO_DATA__POS                           (0)
#define IMU_DRIVER_USER_FIFO_DATA__LEN                           (8)
#define IMU_DRIVER_USER_FIFO_DATA__MSK                           (0xFF)
#define IMU_DRIVER_USER_FIFO_DATA__REG                           (IMU_DRIVER_USER_FIFO_DATA_ADDR)
#endif
 /***************************************************************/
 /***\name	ACCEL CONFIGURATION LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Acc_Conf Description - Reg Addr --> (0x40), Bit --> 0...3 */
#define IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__POS       (0)
#define IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__LEN       (4)
#define IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__MSK       (0x0F)
#define IMU_DRIVER_USER_ACCEL_CONFIG_OUTPUT_DATA_RATE__REG		       \
(IMU_DRIVER_USER_ACCEL_CONFIG_ADDR)

 /** Acc_Conf Description - Reg Addr --> (0x40), Bit --> 4...6 */
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__POS               (4)
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__LEN               (3)
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__MSK               (0x70)
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_BW__REG               (IMU_DRIVER_USER_ACCEL_CONFIG_ADDR)

 /** Acc_Conf Description - Reg Addr --> (0x40), Bit --> 7 */
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__POS   (7)
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__LEN   (1)
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__MSK   (0x80)
#define IMU_DRIVER_USER_ACCEL_CONFIG_ACCEL_UNDER_SAMPLING__REG	\
(IMU_DRIVER_USER_ACCEL_CONFIG_ADDR)

 /** Acc_Range Description - Reg Addr --> 0x41, Bit --> 0...3 */
#define IMU_DRIVER_USER_ACCEL_RANGE__POS                         (0)
#define IMU_DRIVER_USER_ACCEL_RANGE__LEN                         (4)
#define IMU_DRIVER_USER_ACCEL_RANGE__MSK                         (0x0F)
#define IMU_DRIVER_USER_ACCEL_RANGE__REG              \
(IMU_DRIVER_USER_ACCEL_RANGE_ADDR)
 /***************************************************************/
 /***\name	GYRO CONFIGURATION LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Gyro_Conf Description - Reg Addr --> (0x42), Bit --> 0...3 */
#define IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__POS        (0)
#define IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__LEN        (4)
#define IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__MSK        (0x0F)
#define IMU_DRIVER_USER_GYRO_CONFIG_OUTPUT_DATA_RATE__REG               \
(IMU_DRIVER_USER_GYRO_CONFIG_ADDR)

 /** Gyro_Conf Description - Reg Addr --> (0x42), Bit --> 4...5 */
#define IMU_DRIVER_USER_GYRO_CONFIG_BW__POS                      (4)
#define IMU_DRIVER_USER_GYRO_CONFIG_BW__LEN                      (2)
#define IMU_DRIVER_USER_GYRO_CONFIG_BW__MSK                      (0x30)
#define IMU_DRIVER_USER_GYRO_CONFIG_BW__REG               \
(IMU_DRIVER_USER_GYRO_CONFIG_ADDR)

 /** Gyr_Range Description - Reg Addr --> 0x43, Bit --> 0...2 */
#define IMU_DRIVER_USER_GYRO_RANGE__POS                          (0)
#define IMU_DRIVER_USER_GYRO_RANGE__LEN                          (3)
#define IMU_DRIVER_USER_GYRO_RANGE__MSK                          (0x07)
#define IMU_DRIVER_USER_GYRO_RANGE__REG                          (IMU_DRIVER_USER_GYRO_RANGE_ADDR)

#ifdef FIFO_ENABLE
 /***************************************************************/
 /***\name	FIFO HEADER LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Fifo_Config_1 Description - Reg Addr --> 0x47, Bit --> 4 */
#define IMU_DRIVER_USER_FIFO_HEADER_ENABLE__POS                  (4)
#define IMU_DRIVER_USER_FIFO_HEADER_ENABLE__LEN                  (1)
#define IMU_DRIVER_USER_FIFO_HEADER_ENABLE__MSK                  (0x10)
#define IMU_DRIVER_USER_FIFO_HEADER_ENABLE__REG                  (IMU_DRIVER_USER_FIFO_CONFIG_1_ADDR)
 /***************************************************************/
 /***\name	FIFO ACCEL ENABLE LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Fifo_Config_1 Description - Reg Addr --> 0x47, Bit --> 6 */
#define IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__POS                   (6)
#define IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__LEN                   (1)
#define IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__MSK                   (0x40)
#define IMU_DRIVER_USER_FIFO_ACCEL_ENABLE__REG                   (IMU_DRIVER_USER_FIFO_CONFIG_1_ADDR)
 /***************************************************************/
 /***\name	FIFO GYRO ENABLE LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Fifo_Config_1 Description - Reg Addr --> 0x47, Bit --> 7 */
#define IMU_DRIVER_USER_FIFO_GYRO_ENABLE__POS                    (7)
#define IMU_DRIVER_USER_FIFO_GYRO_ENABLE__LEN                    (1)
#define IMU_DRIVER_USER_FIFO_GYRO_ENABLE__MSK                    (0x80)
#define IMU_DRIVER_USER_FIFO_GYRO_ENABLE__REG                    (IMU_DRIVER_USER_FIFO_CONFIG_1_ADDR)
#endif
 /***************************************************************************/
 /** CMD REGISTERS DEFINITION START */
 /***************************************************************/
 /***\name	COMMAND REGISTER LENGTH, POSITION AND MASK*/
 /***************************************************************/
 /** Command description address - Reg Addr --> 0x7E, Bit -->  0....7 */
#define IMU_DRIVER_CMD_COMMANDS__POS                             (0)
#define IMU_DRIVER_CMD_COMMANDS__LEN                             (8)
#define IMU_DRIVER_CMD_COMMANDS__MSK                             (0xFF)
#define IMU_DRIVER_CMD_COMMANDS__REG                             (IMU_DRIVER_CMD_COMMANDS_ADDR)


 /***************************************************************************/
 /** CMD REGISTERS DEFINITION END */
#ifdef FIFO_ENABLE
 /***************************************************/
 /***\name	FIFO FRAME COUNT DEFINITION           */
 /**************************************************/
#define IMU_DRIVER_FIFO_DEPTH                            (1024)
#define FIFO_CONFIG_CHECK1                          (0x00)
#define FIFO_CONFIG_CHECK2                          (0x80)
#endif
 /***************************************************/
 /***\name	ACCEL RANGE          */
 /**************************************************/
#define IMU_DRIVER_ACCEL_RANGE_2G                        (0X03)
#define IMU_DRIVER_ACCEL_RANGE_4G                        (0X05)
#define IMU_DRIVER_ACCEL_RANGE_8G                        (0X08)
#define IMU_DRIVER_ACCEL_RANGE_16G                       (0X0C)
 /***************************************************/
 /***\name	ACCEL ODR          */
 /**************************************************/
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_RESERVED       (0x00)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_0_78HZ         (0x01)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_1_56HZ         (0x02)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_3_12HZ         (0x03)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_6_25HZ         (0x04)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_12_5HZ         (0x05)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_25HZ           (0x06)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_50HZ           (0x07)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_100HZ          (0x08)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_200HZ          (0x09)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_400HZ          (0x0A)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_800HZ          (0x0B)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_1600HZ         (0x0C)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_RESERVED0      (0x0D)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_RESERVED1      (0x0E)
#define IMU_DRIVER_ACCEL_OUTPUT_DATA_RATE_RESERVED2      (0x0F)
#define IMU_DRIVER_ACCEL_MAX_LOW_POWER_ODR               (10)
 /***************************************************/
 /***\name	ACCEL BANDWIDTH PARAMETER         */
 /**************************************************/
#define IMU_DRIVER_ACCEL_OSR4_AVG1                       (0)
#define IMU_DRIVER_ACCEL_OSR2_AVG2                       (1)
#define IMU_DRIVER_ACCEL_NORMAL_AVG4                     (2)
#define IMU_DRIVER_ACCEL_CIC_AVG8                        (3)
#define IMU_DRIVER_ACCEL_RES_NO_AVG                      (4)
#define IMU_DRIVER_ACCEL_RES_AVG2                        (5)
#define IMU_DRIVER_ACCEL_RES_AVG4                        (6)
#define IMU_DRIVER_ACCEL_RES_AVG8                        (7)
#define IMU_DRIVER_ACCEL_RES_AVG16                       (8)
#define IMU_DRIVER_ACCEL_RES_AVG32                       (9)
#define IMU_DRIVER_ACCEL_RES_AVG64                       (10)
#define IMU_DRIVER_ACCEL_RES_AVG128                      (11)

#define IMU_DRIVER_ACCEL_RES_OFFSET                      (4)

#define IMU_DRIVER_US_DISABLE                            (0)
#define IMU_DRIVER_US_ENABLE                             (1)
 /***************************************************/
 /***\name	GYRO ODR         */
 /**************************************************/
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_RESERVED		(0x00)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_25HZ			(0x06)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_50HZ			(0x07)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_100HZ			(0x08)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_200HZ			(0x09)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_400HZ			(0x0A)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_800HZ			(0x0B)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_1600HZ			(0x0C)
#define IMU_DRIVER_GYRO_OUTPUT_DATA_RATE_3200HZ			(0x0D)
 /***************************************************/
 /***\name	GYRO BANDWIDTH PARAMETER         */
 /**************************************************/
#define IMU_DRIVER_GYRO_OSR4_MODE                        (0x00)
#define IMU_DRIVER_GYRO_OSR2_MODE                        (0x01)
#define IMU_DRIVER_GYRO_NORMAL_MODE                      (0x02)
#define IMU_DRIVER_GYRO_CIC_MODE                         (0x03)
 /***************************************************/
 /***\name	GYROSCOPE RANGE PARAMETER         */
 /**************************************************/
#define IMU_DRIVER_GYRO_RANGE_2000_DEG_SEC               (0x00)
#define IMU_DRIVER_GYRO_RANGE_1000_DEG_SEC               (0x01)
#define IMU_DRIVER_GYRO_RANGE_500_DEG_SEC                (0x02)
#define IMU_DRIVER_GYRO_RANGE_250_DEG_SEC                (0x03)
#define IMU_DRIVER_GYRO_RANGE_125_DEG_SEC                (0x04)

#ifdef FIFO_ENABLE
 /***************************************************/
 /***\name	FIFO CONFIGURATIONS    */
 /**************************************************/
#define FIFO_HEADER_ENABLE                          (0x01)
#define FIFO_ACCEL_ENABLE                           (0x01)
#define FIFO_GYRO_ENABLE                            (0x01)
#define FIFO_STOPONFULL_ENABLE                      (0x01)
#define	IMU_DRIVER_FIFO_INDEX_LENGTH                     (1)
#define	IMU_DRIVER_FIFO_TAG_INTR_MASK                    (0xFC)

 /** FIFO header definitions*/
#define FIFO_HEAD_A                                 (0x84)
#define FIFO_HEAD_G                                 (0x88)
#define FIFO_HEAD_G_A                               (0x8C)

#define FIFO_HEAD_INPUT_CONFIG                      (0x48)
#define FIFO_HEAD_SKIP_FRAME                        (0x40)
#define FIFO_HEAD_OVER_READ_LSB                     (0x80)
#define FIFO_HEAD_OVER_READ_MSB                     (0x00)

 /** Clear all data in FIFO */
#define FIFO_FLUSH                                  (0xB0)

#define FIFO_INPUT_CONFIG_FLUSH                     ((int8)-7)
#define	FIFO_INPUT_CONFIG_OVER_LEN                  ((int8)-6)
#define	FIFO_OVER_READ_RETURN                       ((int8)-5)
#define	FIFO_SKIP_OVER_LEN                          ((int8)-4)
#define	FIFO_G_A_OVER_LEN                           ((int8)-3)
#define	FIFO_G_OVER_LEN                             ((int8)-2)
#define	FIFO_A_OVER_LEN                             ((int8)-1)
#endif
 /***************************************************/
 /***\name	ACCEL POWER MODE    */
 /**************************************************/
#define ACCEL_MODE_NORMAL                           (0x11)
#define	ACCEL_LOWPOWER                              (0X12)
#define	ACCEL_SUSPEND                               (0X10)
 /** BMI160 Accel power modes*/
#define IMU_DRIVER_ACCEL_SUSPEND                         0
#define IMU_DRIVER_ACCEL_NORMAL_MODE                     1
#define IMU_DRIVER_ACCEL_LOW_POWER                       2
 /***************************************************/
 /***\name	GYRO POWER MODE    */
 /**************************************************/
#define GYRO_MODE_SUSPEND                           (0x14)
#define GYRO_MODE_NORMAL                            (0x15)
#define GYRO_MODE_FASTSTARTUP                       (0x17)
 /***************************************************/
 /***\name	ENABLE/DISABLE BIT VALUES    */
 /**************************************************/
#define IMU_DRIVER_ENABLE                                (0x01)
#define IMU_DRIVER_DISABLE                               (0x00)
 /***************************************************/
 /***\name	BIT SLICE GET AND SET FUNCTIONS  */
 /**************************************************/
#define IMU_DRIVER_GET_BITSLICE(regvar, bitname)\
		((regvar & bitname##__MSK) >> bitname##__POS)


#define IMU_DRIVER_SET_BITSLICE(regvar, bitname, val)\
		((regvar & ~bitname##__MSK) | \
		((val<<bitname##__POS)&bitname##__MSK))

 /***************************************************************/
 /***\name	STRUCTURE DEFINITIONS                         */
 /***************************************************************/
 /**
 * @brief  Struct used to store the data from each
 *         sensor in IMU or PPG
 */
typedef struct{
    uint16 frame_count;        /** Number of samples from Accel */
    int16 *p_imu_data;
}threeD_imu_sensor_data_t;
 /**
 * @brief  Struct used to store all the FIFO data from
 *         IMU sensors
 */
typedef struct{
    threeD_imu_sensor_data_t accel;
    threeD_imu_sensor_data_t gyro;
    uint16 skip_frame;             /** The value of skip frame information */
    uint8 fifo_input_config_info;  /** FIFO input config info */
}imu_driver_sensor_data_t;

#endif /* SPORT_HEALTH_DRIVER_IMU_BMI160_H */

