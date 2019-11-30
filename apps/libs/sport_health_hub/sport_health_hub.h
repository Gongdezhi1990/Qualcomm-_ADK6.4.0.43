/**  Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health defines for interfacing to the customer application.
 * This file contains the message structure and ids
 */

#ifndef SPORT_HEALTH_H
#define SPORT_HEALTH_H

#include <time.h>
#include <sport_health_algorithms.h>
#include <sport_health_driver.h>

/* Specific 'pmalloc' pool configurations. */
#define PMALLOC_SPORT_HEALTH_EXTRAS {2048, 1 },

#define SPORT_HEALTH_MESSAGE_BASE 2200
#define DEFAULT_TIMESTAMP 0xFFFF

/**  Algorithm enable bitfield */
#define ALGO_EN_NONE            (0)
#define ALGO_EN_BASE            (1)
#define ALGO_EN_STEP            (1 << 0)
#define ALGO_EN_TAP             (1 << 1)
#define ALGO_EN_DISTANCE        (1 << 2)
#define ALGO_EN_ATTITUDE        (1 << 3)
#define ALGO_EN_HR              (1 << 4)
#define ALGO_EN_SPO2            (1 << 5)
#define ALGO_EN_PROXIMITY       (1 << 6)
#define ALGO_EN_PPG_LOW_POWER   (1 << 7)
#define ALGO_EN_TAP_RATES       (1 << 8)
#define ALGO_EN_MOTION_DETECT   (1 << 9)

/**
 * @brief Sensor mode calculation masks
*/
#define SEQ_NONE_MASK           (0)
#define SEQ_ACC_MASK            (1 << 0)
#define SEQ_GYRO_MASK           (1 << 1)
#define SEQ_IMU_MODE_MASK       (SEQ_ACC_MASK|SEQ_GYRO_MASK) /**< Bits 0 and 1 are IMU mode positions */
#define SEQ_PPG_HR_MASK         (1 << 2)
#define SEQ_PPG_SPO2_MASK       (1 << 3)
#define SEQ_PPG_MODE_MASK       (0x0C) /**< Bits 2 and 3 are PPG mode positions */
#define SEQ_PPG_MODE_POS        (2)
#define SEQ_PROXIMITY_MASK      (1 << 4)
#define SEQ_PROX_MODE_MASK      (0x10) /**< Bits 4 is PROXIMITY mode positions */
#define SEQ_PROX_MODE_POS       (4)


/**
 * @brief This look up table is a control interface between the algorithm and sequencer.
 *        The fields defined in this table can be changed as per the algorithm requirements.
 *        The fields are used to derive the wake period in hub, sample rate and corresponding
 *        sensor mode for each enabled algorithm.
*/
#define ALGORITHM_DEPENDENCY_LOOK_UP \
{/** MIN_PERIOD, MAX_PERIOD, DEPENDENT_ALGO_MSK,     SAMPLE_RATE,               SENSOR_MASK */                          \
{        250,        500,     ALGO_EN_NONE,      IMU_OUTPUT_DATA_RATE_50HZ,     SEQ_ACC_MASK        },  /** step */       \
{        250,        500,     ALGO_EN_STEP,      IMU_OUTPUT_DATA_RATE_50HZ,     SEQ_ACC_MASK        },  /** tap */        \
{        250,        500,     ALGO_EN_STEP,      IMU_OUTPUT_DATA_RATE_50HZ,     SEQ_NONE_MASK       },  /** distance */   \
{        250,        500,     ALGO_EN_STEP,      IMU_OUTPUT_DATA_RATE_50HZ,     SEQ_GYRO_MASK       },  /** attitude */   \
{        250,        500,     ALGO_EN_STEP,      PPG_SAMPLE_RATE25,             SEQ_PPG_HR_MASK     },  /** hr */ \
{        250,        500,     ALGO_EN_STEP,      PPG_SAMPLE_RATE25,             SEQ_PPG_SPO2_MASK   },  /** spo2 */     \
{        250,        500,     ALGO_EN_NONE,      PPG_SAMPLE_RATE25,             SEQ_PROXIMITY_MASK  },  /** proximity */     \
{          0,       0xFFFFL,  ALGO_EN_NONE,           0,                        SEQ_NONE_MASK       }   /** INVALID */    \
}\

#define NUM_IMU_ALGORITHMS        (4)
#define NUM_PPG_ALGORITHMS        (2)
#define NUM_IMU_PPG_ALGORITHMS    (NUM_IMU_ALGORITHMS + NUM_PPG_ALGORITHMS)
#define NUM_DEPENDENT_PARAMS      (5) /**< Number of columns in above look up table */
#define NUM_AVAILABLE_ALGORITHMS  (8)
#define ALGO_DISABLED             (7) /**< Row 8 in above table is of INVALID entry to derive IMU_MODE_NONE/PPG_MODE_NONE */

extern uint16 IMU_LP_SAMPLE_INTERVAL_MS_TABLE[IMU_NUM_LOW_POWER_ODR];

/**
 * @brief define each column in the algorithm
 *        dependency look up table
 */
typedef enum{
        ALGO_MIN_PERIOD = 0,
        ALGO_MAX_PERIOD,
        ALGO_DEPENDENT_EN,
        ALGO_SAMPLE_RATE,
        ALGO_SENSOR_MASK
}algorithm_dependency_table_param_t;

/**  defines the rate and preferred wake intervals */
typedef struct
{
    uint16 algo_min_ms;
    uint16 algo_max_ms;
    uint8 imu_odr;
    uint8 imu_period;
    uint8 ppg_odr;
    uint8 ppg_period;
    uint8 prox_odr;
    uint8 prox_period;
    imu_mode_t imu_mode;
    ppg_mode_t ppg_mode;
    prox_mode_t prox_mode;
} sh_sensor_cfg_t;

/**
 * @brief configue the BT timing interval and mode to allow the scheduler to
 *        optimise FIFO depth and wake intervals
 */

typedef enum
{
    SH_VM_BT_MODE_OFF = 0,
    SH_VM_BT_MODE_SNIFF = 1,
    SH_VM_BT_MODE_ACTIVE = 2
} sh_vm_bt_mode_t;

/**  Hub to/from VM App messages */
typedef enum
{
    SH_VM_SET_TIMING_INFO_REQ = SPORT_HEALTH_MESSAGE_BASE,
    SH_VM_SET_TIMING_INFO_CFM,
    SH_VM_ENABLE_REQ,
    SH_VM_ENABLE_CFM,
    SH_VM_RESULTS_IND,
    SH_VM_IN_OUT_IND
}sh_vm_message_t;

/**
 * @brief   Different types of avaiable sports
 *          and health sensors
 */
typedef enum
{
    IMU,
    PPG,
    NONE
}sensor_type_t;

/**
 * @brief   This message is sent to app in response to SH_VM_ENABLE_REQ
 */
typedef struct
{
    bool    status;
    sensor_type_t sensor_type;

} SH_VM_ENABLE_CFM_T;

/**
 * @brief   This message is sent to app in response to SH_VM_SET_TIMING_INFO_REQ
 */
typedef struct
{
    bool    status;
} SH_VM_SET_TIMING_INFO_CFM_T;

/** This message is sent to set the bluetooth mode and interval */
typedef struct
{
    uint32          bt_interval_ms;
    sh_vm_bt_mode_t bt_mode;
} SH_VM_SET_TIMING_INFO_REQ_T;


/**
 * @brief set enables to start processing
 *        power_mode give the algorithms guidance on the power / quality trade off
 */
typedef enum
{
    SH_VM_PWR_LOW_POWER = 1,
    SH_VM_PWR_DEFAULT = 2,
    SH_VM_PWR_HIGH_QUALITY = 3
}sh_vm_pwr_mode_t;

/** This message is sent to set the algorithm enable bitfield and algorithm information
    data structure */
typedef struct
{
    uint32             enables;
    sh_vm_algo_info_t    *p_algo_info;
    sh_vm_pwr_mode_t   power_mode;
}SH_VM_ENABLE_REQ_T;




/**  In/Out indications */
typedef enum
{
    SH_IN_OUT_TYPE_IN = 0,
    SH_IN_OUT_TYPE_OUT = 1,
} sh_in_out_type_t;

typedef struct
{
    clock_t          timeStamp;
    sh_in_out_type_t tap_type;
}SH_VM_IN_OUT_IND_T;

/**
 * @brief Initialise the Hub and create the corresponding task
 * @param app_task Task Id for customer app
 */
Task SportHealthHubInit(Task app_task);
/**
 * @brief Register the driver (IMUPPG) task in hub
 * @param driver_task Task Id for the IMUPPG task
 */
void SportHealthHubCfg(Task driver_task);

/**
 * @brief Update the PPG duty cycle mode and return true if it changes
 *
 * @return mode was flipped by update (1=flipped, 0=same)
 */
bool sh_seq_ppg_duty_cycle_update(imu_sensor_data_t *p_message);

/**
 * @brief Notify the driver about the detected interrupt
 */
void sh_interrupt_notify(void);

#endif /**  SPORT_HEALTH_H */
