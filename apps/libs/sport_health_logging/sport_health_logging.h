/**  Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health functions for logging over T-Bridge and GATT
 */
#ifndef SPORT_HEALTH_LOGGING_H
#define SPORT_HEALTH_LOGGING_H

#include "sport_health_driver.h"
#include "sport_health_algorithms.h"
#include <hydra_log.h>
#include <types.h>

#define LOGGING_VERSION    (111)
#define SYSTEM_TIME_MS_WRAP_LIMIT (4294967UL)
#define SYSTEM_CLOCKS_PER_MILLISECOND      ((uint32)1000)

/* Log IDs for context and debug logging types */
#define GATT_LOGGING_DEBUG_DATA (uint8)4
#define GATT_LOGGING_DEBUG      (uint8)9

/* Log IDs per sensor types */
#define GATT_LOGGING_SENSOR_BASE    (uint8)10
#define GATT_LOGGING_IMU            (GATT_LOGGING_SENSOR_BASE)
#define GATT_LOGGING_PPG            (GATT_LOGGING_SENSOR_BASE+1)
#define GATT_LOGGING_SENSOR_END     (GATT_LOGGING_PPG)

/* Log IDs for algorithm context types */
#define GATT_LOGGING_ALGO_BASE   (uint8)16
#define GATT_LOGGING_FRAME_BASE  (uint8)26

#define INSTALL_HYDRA_LOG 1

/* Logging mask values. There is one of these for pydbg logging, and one for BLE logging */
typedef enum {
    SH_LOG_DEBUG           = 0x0001, /* Printf-style debug */
    SH_LOG_IMU_SENSOR_DATA = 0x0002, /* IMU meta + sensor data (currently ACC x,y,z only) */
    SH_LOG_PPG_SENSOR_DATA = 0x0004, /* PPG meta + sensor data (IR and/or Red) */
    SH_LOG_STEP_DIGEST     = 0x0008, /* Step algo input */
    SH_LOG_STEP_RESULT     = 0x0010, /* Step algo output */
    SH_LOG_STEP_PARAMS     = 0x0020, /* Step algo param change */
    SH_LOG_DIST_DIGEST     = 0x0040, /* Distance algo input */
    SH_LOG_DIST_RESULT     = 0x0080, /* Distance algo output */
    SH_LOG_DIST_PARAMS     = 0x0100, /* Distance algo param change */
    SH_LOG_TAP_DIGEST      = 0x0200, /* Tap algo input */
    SH_LOG_TAP_RESULT      = 0x0400, /* Tap algo output */
    SH_LOG_TAP_PARAMS      = 0x0800, /* Tap algo param change */
    SH_LOG_HR_DIGEST       = 0x1000, /* Heart rate algo input */
    SH_LOG_HR_RESULT     = 0x2000, /* Heart rate algo output */
    SH_LOG_HR_PARAMS     = 0x4000, /* Heart rate algo param change */
/* Logging of context data is not currently supported, but these are here for future use */
    SH_LOG_STEP_CONTEXT    = 0x08000, /* Step algo context */
    SH_LOG_DIST_CONTEXT    = 0x10000, /* Distance algo context */
    SH_LOG_TAP_CONTEXT     = 0x20000, /* Tap algo context */
    SH_LOG_HR_CONTEXT      = 0x40000, /* Heat Rate algo context */
/* Attitude is not currently included in the library, but these are here for future use */
    SH_LOG_ATTITUDE_DIGEST = 0x080000,
    SH_LOG_ATTITUDE_RESULT = 0x100000,
    SH_LOG_ATTITUDE_PARAM  = 0x200000,
    SH_LOG_ATTITUDE_CONTEXT= 0x400000
}sh_log_mask;

/* Masks to allow all logs of a certain type to be checked or turned on and off */
#define SH_LOG_DIGEST_MASK (SH_LOG_STEP_DIGEST | SH_LOG_DIST_DIGEST | SH_LOG_TAP_DIGEST | SH_LOG_HR_DIGEST)
#define SH_LOG_RESULT_MASK (SH_LOG_STEP_RESULT | SH_LOG_DIST_RESULT | SH_LOG_TAP_RESULT | SH_LOG_HR_RESULT)
#define SH_LOG_PARAMS_MASK (SH_LOG_STEP_PARAMS | SH_LOG_DIST_PARAMS | SH_LOG_TAP_PARAMS | SH_LOG_HR_PARAMS)
#define SH_LOG_CONTEXT_MASK (SH_LOG_STEP_CONTEXT | SH_LOG_DIST_CONTEXT | SH_LOG_TAP_CONTEXT | SH_LOG_HR_CONTEXT)

/* The SHG_DBG_xxx macros obey the logging mask, and may send data to GATT as well as normal debug logging. Pass the mask for what you want to log */
#define SHG_DBG_MSG0(MASK,x)             {if (SportHealthLoggingDebugData(x,MASK,0)) DBG_MSG(x);}
#define SHG_DBG_MSG1(MASK,x,a1)          {if (SportHealthLoggingDebugData(x,MASK,1,a1)) DBG_MSG1(x,a1);}
#define SHG_DBG_MSG2(MASK,x,a1,a2)       {if (SportHealthLoggingDebugData(x,MASK,2,a1,a2)) DBG_MSG2(x,a1,a2);}
#define SHG_DBG_MSG3(MASK,x,a1,a2,a3)    {if (SportHealthLoggingDebugData(x,MASK,3,a1,a2,a3)) DBG_MSG3(x,a1,a2,a3);}
#define SHG_DBG_MSG4(MASK,x,a1,a2,a3,a4) {if (SportHealthLoggingDebugData(x,MASK,4,a1,a2,a3,a4)) DBG_MSG4(x,a1,a2,a3,a4);}

/* Num BLE connections supported */
/* (This value should match the GATT configuration) */
#define MAX_LOGGING_BLE_CONNECTIONS (2)
#define GATT_LOGGING_PAYLOAD_SIZE_BYTES (18)
#define GATT_LOGGING_PAYLOAD_SIZE_WORDS (9)
#define GATT_LOGGING_PAYLOAD_SIZE_DWORDS (4)
#define GATT_LOGGING_HEADER_SIZE (2)
#define GATT_LOGGING_TOTAL_PACKET_SIZE (GATT_LOGGING_HEADER_SIZE + GATT_LOGGING_PAYLOAD_SIZE_BYTES)
#define NUM_META_PARAMS (12)

typedef enum
{
    ALGO_STEP,
    ALGO_TAP,
    ALGO_ATTITUDE,
    ALGO_DISTANCE,
    ALGO_HR,
    ALGO_NONE
}sh_algo_type;

/* None also marks the maximum algo number */
#define MAX_NUM_ALGO (ALGO_NONE)

typedef enum
{
    ALGO_INIT,
    ALGO_CONFIG,
    ALGO_DELETE
}sh_algo_status;

typedef enum
{
    IMU_SENSOR,
    PPG_SENSOR,
    MAX_NUM_SENSOR
}sh_sensor_type;

typedef enum
{
    PPG_RED,
    PPG_IR,
    PPG_RED_IR,
    MAX_NUM_PPG_SENSOR
}sh_ppg_sensor_type;

typedef enum
{
    IMU_ACCEL,
    IMU_GYRO,
    IMU_ACCEL_GYRO,
    MAX_NUM_IMU_SENSOR
}sh_imu_sensor_type;

typedef struct
{
    sh_hr_feedback_t hr_feedback;
}algo_feedback_t;


/* GATT Notification callback to be registered by application */
typedef void (*NotifyCallback) (const uint8 *,uint16);

/* Set what is to be logged over pydbg */
void SportHealthLoggingSetDebugLogging(uint32 logMask);
/* Set what is to be logged over BLE */
void SportHealthLoggingSetBleLogging(uint32 logMask);
/* Get what is to be logged over pydbg */
uint32 SportHealthLoggingGetDebugLogging(void);
/* Get what is to be logged over BLE */
uint32 SportHealthLoggingGetBleLogging(void);

/* Register a callback for use by the logging library when it wants to send data over GATT */
void SportHealthLoggingRegisterNotifyCallback(NotifyCallback callback);
/* Log current version of Sports & Health library */
void SportHealthLoggingVersionInfo(void);
/* Log Sports & Health algorithm feedback */
void SportHealthLoggingAlgoFeedback(sh_sensor_type sensor_type, algo_feedback_t *algo_feedback);
/* Log Sports & Health algorithm status */
void SportHealthLoggingAlgoStatus(sh_algo_type algo_type, sh_algo_status algo_status, uint32 algo_enable_bitfield);
/* Log Sports & Health sensor requirements for a given algorithm list */
void SportHealthLoggingSensorRequired(void *p_info);
/* Log Sports & Health PPG sensor data */
void SportHealthLoggingPpgSensorData(const ppg_sensor_data_t *p_message);
/* Log Sports & Health IMU sensor data */
void SportHealthLoggingImuSensorData(const imu_sensor_data_t *p_message);
/* Log Sports & Health debug data. Returns TRUE if debug logging over pydbg is on */
bool SportHealthLoggingDebugData(const char * fmt, uint32 mask, uint8 nArgs, ...);
/* Log Sports & Health algorithm context (Currently not used) */
void SportHealthLoggingAlgoContext(sh_algo_type algo_type, const uint8 * p_algo_context_octets, uint16 algo_data_size);
/* Get system time in milliseconds, dealing with wrap-around in the microsecond clock
   Millisecond times are required for log timestamps */
uint32 SportHealthLoggingGetSystemTimeMs(void);
/* Get system time in microseconds & milliseconds, dealing with wrap-around in the microsecond clock */
uint32 SportHealthLoggingGetSystemTimeUs(uint32 * timestamp_ms);
#endif /* SPORT_HEALTH_LOGGING_H */
