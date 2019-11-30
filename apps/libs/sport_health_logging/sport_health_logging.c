/**  Copyright (c) 2018 Qualcomm Technologies International, Ltd. */
/**     */
/** *
 * \file
 * Sport Health functions for logging over T-Bridge and GATT
 */
#include <types.h>
#include "sport_health_logging.h"
#include "sport_health_hub.h"
#include "gatt_logging_server.h"
#include "system_clock.h"
#include "panic.h"
#include <stdarg.h>
#ifdef SPORT_HEALTH_LOG_COMPRESS
#include "sport_health_log_compress.h"
#endif

/* We keep track of microsecond clock wraps so that we can keep a stable millisecond time */
static uint32 sh_log_prev_system_time_us = 0;
static uint32 sh_log_prev_timestamp_ms = 0;
static uint32 sh_log_timestamp_ms_wrap_counter = 0;
/* This is the callback into the application to send data over any GATT connections */
NotifyCallback sh_log_notifyCallback = 0;
/* Logging masks for pydbg and GATT logging */
static uint32 sh_log_pydbg_mask = 0;
static uint32 sh_log_gatt_mask = 0;

/* We sometimes want log data to be byte addressable, sometimes word, and sometimes dword
 * But the first two bytes are always byte addressable
 * We can't define it as a strucrue as packing issues will trip us up
 * And we can't use the type directly as we'll get alignment exceptions, so we have to do it the hard way
 */
typedef uint8 LogData [GATT_LOGGING_TOTAL_PACKET_SIZE];

/* Internal functions */
/* Write uint16 to buffer */
uint8 * sh_log_write_uint16 (uint8 * pBuf,uint16 val);
/* Write uint32 to buffer */
uint8 * sh_log_write_uint32 (uint8 * pBuf,uint32 val);

/* Common function for logging meta-data */
void sh_log_meta_data(uint16 type, const sensor_data_t * pInput, const sensor_data_t * pInput2);
/* Meta data can include an optional second set of sensor data for the PPG if it is doing both red + IR */
#define NO_SECOND_INPUT (const sensor_data_t *)NULL

/* Common function for logging sensor data */
void sh_log_sensor_data(uint16 type,  uint16 numSamples, const uint16* pData);

/* Set what is to be logged over pydbg */
void SportHealthLoggingSetDebugLogging(uint32 logMask)
{
    sh_log_pydbg_mask = logMask;
}

/* Set what is to be logged over BLE */
void SportHealthLoggingSetBleLogging(uint32 logMask)
{
    sh_log_gatt_mask = logMask;
}

/* Get what is to be logged over pydbg */
uint32 SportHealthLoggingGetDebugLogging(void)
{
    return sh_log_pydbg_mask;
}

/* Get what is to be logged over BLE */
uint32 SportHealthLoggingGetBleLogging(void)
{
    return sh_log_gatt_mask;
}

/* This is how the app registers its notification method
 *  If the callback is zero, logging will do nothing
 */
void SportHealthLoggingRegisterNotifyCallback(NotifyCallback callback)
{
    sh_log_notifyCallback = callback;
}

/* XYZ in each sample */
#define NUM_VAL_PER_SAMPLE 3

/* Get system time in milliseconds, dealing with wrap-around in the microsecond clock
 * Millisecond times are required for log timestamps
 */
uint32 SportHealthLoggingGetSystemTimeMs(void)
{
    uint32 timestamp_ms;
    SportHealthLoggingGetSystemTimeUs(&timestamp_ms);
    return timestamp_ms;
}

/* Get system time in microseconds & milliseconds, dealing with wrap-around in the microsecond clock */
uint32 SportHealthLoggingGetSystemTimeUs(uint32 * timestamp_ms)
{
    uint32 system_time_us = SystemClockGetTimerTime();

    uint32 t_ms = sh_log_prev_timestamp_ms + (system_time_us - sh_log_prev_system_time_us + 500ul) / 1000ul;
    sh_log_prev_system_time_us = system_time_us;
    if (t_ms < sh_log_prev_timestamp_ms)
        ++sh_log_timestamp_ms_wrap_counter;
    sh_log_prev_timestamp_ms = t_ms;

    if (timestamp_ms)
        *timestamp_ms = t_ms + sh_log_timestamp_ms_wrap_counter * SYSTEM_TIME_MS_WRAP_LIMIT;
    /* We should be OK if the function is called once in 49.71 days! */

    return system_time_us;
}

/* Log current version of Sports & Health library */
void SportHealthLoggingVersionInfo(void)
{
    SHG_DBG_MSG2(SH_LOG_DEBUG,"SH_VERSION_INFO: %u, %u", SportHealthLoggingGetSystemTimeMs(), LOGGING_VERSION);
}

/* Log Sports & Health algorithm status */
void SportHealthLoggingAlgoStatus(sh_algo_type algo_type, sh_algo_status algo_status, uint32 algo_enable_bitfield)
{
    if(algo_status == ALGO_CONFIG)
    {
        SHG_DBG_MSG4(SH_LOG_DEBUG,"SH_ALGO_STATUS: %u, %u, %u, %u", SportHealthLoggingGetSystemTimeMs(), algo_status, algo_enable_bitfield, 0);
    }
    else
    {
        SHG_DBG_MSG3(SH_LOG_DEBUG,"SH_ALGO_STATUS: %u, %u, %u", SportHealthLoggingGetSystemTimeMs(), algo_status, algo_type);
    }
}

/* Log Sports & Health sensor requirements for a given algorithm list */
void SportHealthLoggingSensorRequired(void *p_info)
{
    sh_sensor_cfg_t *p_sensor_cfg = (sh_sensor_cfg_t *)p_info;
    SHG_DBG_MSG3(SH_LOG_DEBUG,"SH_SENSOR_REQUIRED: %u, %u, %u", SportHealthLoggingGetSystemTimeMs(), p_sensor_cfg->algo_max_ms,
                p_sensor_cfg->algo_min_ms);
    SHG_DBG_MSG4(SH_LOG_DEBUG,"%u, %u, %u, %u", p_sensor_cfg->imu_period, p_sensor_cfg->ppg_period, p_sensor_cfg->imu_mode, p_sensor_cfg->ppg_mode);
}

/* Log Sports & Health algorithm feedback */
void SportHealthLoggingAlgoFeedback(sh_sensor_type sensor_type, algo_feedback_t *algo_feedback)
{
    if(sensor_type == PPG_SENSOR)
        SHG_DBG_MSG2(SH_LOG_DEBUG,"SH_ALGO_FEEDBACK: %u, %u", SportHealthLoggingGetSystemTimeMs(), algo_feedback->hr_feedback);
}

/* Log a batch of PPG samples - both the meta data (framing info) and the samples */
void SportHealthLoggingPpgSensorData(const ppg_sensor_data_t *p_message)
{
    bool logPydbg = (sh_log_pydbg_mask & SH_LOG_PPG_SENSOR_DATA);
    bool logGatt = ((sh_log_gatt_mask & SH_LOG_PPG_SENSOR_DATA) && sh_log_notifyCallback);
    uint32 i;
    if ((logPydbg == 0) && (logGatt == 0)) return;
    /** Check if both red and IR data exists */
    if ((p_message->red.frame_count > 0) && (p_message->IR.frame_count > 0))
    {
        if (logPydbg) {
            uint16 *p_ppg_data = *(p_message->IR.data.p_ppg_data);
            DBG_MSG2("SH_PPG_RED_IR: %u, %u", SportHealthLoggingGetSystemTimeMs(), p_message->red.last_sample_time);
            DBG_MSG3("%u, %u, %u", p_message->red.sampling_interval,
            p_message->red.range, p_message->red.frame_count);
            for(i = 0; i < p_message->red.frame_count; i++)
            {
                DBG_MSG1("%u",p_ppg_data[(2*i)]);
            }
            DBG_MSG2("SH_PPG_IR: %u, %u", SportHealthLoggingGetSystemTimeMs(), p_message->IR.last_sample_time);
            DBG_MSG3("%u, %u, %u", p_message->IR.sampling_interval,
                p_message->IR.range, p_message->IR.frame_count);
            for(i = 0; i < p_message->IR.frame_count; i++)
            {
                DBG_MSG1("%u",p_ppg_data[(2*i) + 1]);
            }
        }
        if (logGatt) {
            /* Log the Meta data first */
            sh_log_meta_data (PPG_RED_IR,&p_message->red,&p_message->IR);
            sh_log_sensor_data(GATT_LOGGING_PPG,p_message->red.frame_count,*(p_message->red.data.p_ppg_data));
            sh_log_sensor_data(GATT_LOGGING_PPG,p_message->IR.frame_count,*(p_message->IR.data.p_ppg_data));
        }
    }
    else if (p_message->red.frame_count > 0)
    {

        if (logPydbg) {
            uint16 *p_ppg_data = *(p_message->red.data.p_ppg_data);
            /** Log the data input to HR algorithm from PPG sensor */
            DBG_MSG2("SH_PPG_RED: %u, %u", SportHealthLoggingGetSystemTimeMs(), p_message->red.last_sample_time);
            DBG_MSG3("%u, %u, %u", p_message->red.sampling_interval,
                    p_message->red.range, p_message->red.frame_count);
            for(i = 0; i < p_message->red.frame_count; i++)
            {
                DBG_MSG1("%u",p_ppg_data[i]);
            }
        }
        if (logGatt) {
            /* Log the Meta data first */
            sh_log_meta_data (PPG_RED,&p_message->red,NO_SECOND_INPUT);
            sh_log_sensor_data(GATT_LOGGING_PPG,p_message->red.frame_count,*(p_message->red.data.p_ppg_data));
        }
    }
    else if (p_message->IR.frame_count > 0)
    {
        if (logPydbg) {
            uint16 *p_ppg_data = *(p_message->IR.data.p_ppg_data);
            /** Log the data input to HR algorithm from PPG sensor */
            DBG_MSG2("SH_PPG_IR: %u, %u", SportHealthLoggingGetSystemTimeMs(), p_message->IR.last_sample_time);
            DBG_MSG3("%u, %u, %u", p_message->IR.sampling_interval,
                p_message->IR.range, p_message->IR.frame_count);
            for(i = 0; i < p_message->IR.frame_count; i++)
            {
                DBG_MSG1("%u",p_ppg_data[i]);
            }
        }
        if (logGatt) {
            /* Log the Meta data first */
            sh_log_meta_data (PPG_IR,&p_message->IR,NO_SECOND_INPUT);
            sh_log_sensor_data(GATT_LOGGING_PPG,p_message->IR.frame_count,*(p_message->IR.data.p_ppg_data));
        }
    }
}

/* Log a batch of IMU samples - both the meta data (framing info) and the samples */
void SportHealthLoggingImuSensorData(const imu_sensor_data_t *p_message)
{
    bool logPydbg = (sh_log_pydbg_mask & SH_LOG_IMU_SENSOR_DATA);
    bool logGatt = ((sh_log_gatt_mask & SH_LOG_IMU_SENSOR_DATA) && sh_log_notifyCallback);
    uint32 i;
    int16 *p_imu_data = *(p_message->accel.data.p_imu_data);
    uint32 imu_data_size = p_message->accel.frame_count *  NUM_VAL_PER_SAMPLE;
    /** Log the data input to IMU algorithms from IMU sensor */
    if (logPydbg) {
        DBG_MSG2("SH_IMU_ACC: %u, %u", SportHealthLoggingGetSystemTimeMs(), p_message->accel.last_sample_time);
        DBG_MSG3("%u, %u, %u", p_message->accel.sampling_interval,
            p_message->accel.range, p_message->accel.frame_count);
        for(i = 0; i < imu_data_size; )
        {
            DBG_MSG3("%d, %d, %d",p_imu_data[i],
                    p_imu_data[i+1], p_imu_data[i+2]);
            /* Note that this call requires the samples to be in sequence  */
            i = i + NUM_VAL_PER_SAMPLE;
        }
    }
    if (logGatt) {
#ifdef SPORT_HEALTH_LOG_COMPRESS
        LogData logData;
        sh_log_compress_imu_info compressInfo;
        uint8 size = SportHealthLoggingCompressImuHeader(p_message, &compressInfo, logData);
        while (size != 0) {
            sh_log_notifyCallback(logData,size); /* Send the last payload */
            /* The compression function will return 0 when it is done (or if there is an error) */
            size = SportHealthLoggingCompressImuData(p_message, &compressInfo, logData);
        }
#else
        /* Log the Meta data first */
        sh_log_meta_data (IMU_ACCEL,(&p_message->accel),NO_SECOND_INPUT);
        sh_log_sensor_data(GATT_LOGGING_IMU,imu_data_size,(const uint16 *)p_imu_data);
#endif
    }
}

#define NUM_META_DATA_PER_LINE (4)
/* Log the frame data for a batch of samples, pInput2 will be present only for PPG RED+IR
 * This works for both IMU and PPG data as the data structures are the same apart from the sample type
 */
void sh_log_meta_data(uint16 type,  const sensor_data_t * pInput, const sensor_data_t * pInput2)
{
    /* Do nothing if we are not initialised */
    if (sh_log_notifyCallback == 0) return;
    uint8 logType = GATT_LOGGING_FRAME_BASE;
    if (type != IMU_ACCEL) {
        logType++; /* PPG */
    }
    uint16 sizePerLine = NUM_META_DATA_PER_LINE * sizeof(uint32);
    LogData logData;
    uint8* payload = &logData[2];
    logData[0] = logType;
    logData[1] = (uint8)sizePerLine;
    /* We don't use a loop here as each item is unique */
    payload = sh_log_write_uint32(payload,SportHealthLoggingGetSystemTimeMs());
    payload = sh_log_write_uint32(payload,type);
    payload = sh_log_write_uint32(payload,pInput->last_sample_time);
    payload = sh_log_write_uint32(payload,pInput->sampling_interval);
    sh_log_notifyCallback(logData,sizePerLine+GATT_LOGGING_HEADER_SIZE);
    payload = &logData[2];
    payload = sh_log_write_uint32(payload,pInput->range);
    payload = sh_log_write_uint32(payload,pInput->frame_count);
    if (type != PPG_RED_IR) {
        logData[1] = 2 * sizeof(uint32);
    }
    else {
        if (pInput2 == NULL) {
            return;
        }
        payload = sh_log_write_uint32(payload,pInput2->last_sample_time);
        payload = sh_log_write_uint32(payload,pInput2->sampling_interval);
        sh_log_notifyCallback(logData,sizePerLine+GATT_LOGGING_HEADER_SIZE);
        payload = &logData[2];
        payload = sh_log_write_uint32(payload,pInput2->range);
        payload = sh_log_write_uint32(payload,pInput2->frame_count);
        logData[1] = 2 * sizeof(uint32);
    }
    sh_log_notifyCallback(logData,logData[1]+GATT_LOGGING_HEADER_SIZE);
}

/* Log sensor data - either PPG or IMU */
#define NUM_SAMPLES_PER_LINE 9
void sh_log_sensor_data(uint16 type,  uint16 numSamples, const uint16* pData)
{
    LogData logData;
    uint8* payload;
    uint16 sizePerLine = NUM_SAMPLES_PER_LINE * sizeof(uint16);
    uint16 index = 0;
    uint16 samples = 0;
    uint16 i;
    logData[0] = type;
    while (numSamples > 0) {
        if (numSamples >= NUM_SAMPLES_PER_LINE) {
            logData[1] = sizePerLine;
            samples = NUM_SAMPLES_PER_LINE;
        }
        else {
            logData[1] = numSamples * sizeof(uint16);
            samples = numSamples;
        }
        payload = &logData[2];
        for (i=0; i<samples; ++i) {
            payload = sh_log_write_uint16(payload,pData[index++]);
        }
        numSamples -= samples;
        sh_log_notifyCallback(logData,logData[1]+GATT_LOGGING_HEADER_SIZE);
    }
}

/* Log Sports & Health algorithm context (Currently not used as generates too much data */
void SportHealthLoggingAlgoContext(sh_algo_type algo_type, const uint8 * p_algo_context_octets, uint16 algo_data_size)
{
    /* Not currently doing per-algorithm checking here */
    if ((sh_log_notifyCallback == 0) || ((sh_log_gatt_mask & SH_LOG_CONTEXT_MASK) == 0)) return;
    LogData logData;
    uint16 lineSize = GATT_LOGGING_PAYLOAD_SIZE_BYTES;
    uint16 i, index=0;
    logData[0] = algo_type; //TODO
    while (algo_data_size > 0) {
        if (algo_data_size < GATT_LOGGING_PAYLOAD_SIZE_BYTES) {
            lineSize = algo_data_size;
        }
        algo_data_size -= lineSize;
        logData[1] = lineSize;
        for (i=0; i<lineSize; ++i) {
            logData[i+GATT_LOGGING_HEADER_SIZE] = p_algo_context_octets[index++];
        }
        sh_log_notifyCallback(logData,lineSize+GATT_LOGGING_HEADER_SIZE);
    }
}

/* Log printf-type data. Logs the type (mask) and then the parameters
   Returns TRUE if debug logging over pydbg enabled */
bool SportHealthLoggingDebugData(const char * fmt, uint32 mask,uint8 nArgs, ...)
{
    UNUSED(fmt);
    /* If no GATT logging */
    if ((sh_log_notifyCallback == 0) || ((sh_log_gatt_mask & mask) == 0)) {
               return (sh_log_pydbg_mask & mask);
        }
    /* Send logging info to GATT server */
    /* Each report is 1 byte type, 1 byte length, 2 byte type, up to 4 x 32 bit words of data
     * If we received multiple log messages of the same type, we increment after each so that lost message can be noticed
     */
    static uint16 baseLastLogType = 0;
    static uint16 lastLogType = 0;
    uint16 logType;
    uint8 size = sizeof(uint16) + sizeof(uint32)*nArgs;
    LogData logData;
    uint8* payload = &logData[2];
    uint32 arg = 0;
    logType = (uint16)mask; /* All the log types we are interested in fit in 16 bits */
    if ((logType == baseLastLogType) && (logType >= SH_LOG_STEP_DIGEST) && ((*fmt) != 'S')) {
        ++lastLogType; /* Record which log message number this is within this set */
        logType = lastLogType;
    }
    else {
        baseLastLogType = logType;
        lastLogType = logType;
    }
    va_list va_argp;
    va_start(va_argp, nArgs);
    logData[0] = GATT_LOGGING_DEBUG;
    logData[1] = size;
    payload = sh_log_write_uint16(payload,logType);
    while (nArgs--) {
        arg = va_arg(va_argp, uint32);
        payload=sh_log_write_uint32(payload,arg);
    }
    sh_log_notifyCallback(logData,size+GATT_LOGGING_HEADER_SIZE);
        /* Now do any pydbg logging required */
        return (sh_log_pydbg_mask & mask);

}

/* Write uint16 to buffer, little-endian */
uint8 * sh_log_write_uint16 (uint8 * pBuf,uint16 val)
{
    *pBuf++ = (val & 0xff);
    *pBuf++ = ((val >> 8) & 0xff);
    return pBuf;
}

/* Write uint32 to buffer, little-endian */
uint8 * sh_log_write_uint32 (uint8 * pBuf,uint32 val)
{
    *pBuf++ = (val & 0xff);
    *pBuf++ = ((val >> 8) & 0xff);
    *pBuf++ = ((val >> 16) & 0xff);
    *pBuf++ = ((val >> 24) & 0xff);
    return pBuf;
}


