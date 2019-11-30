#include "sport_health_hub_private.h"
#include "sport_health_hub.h"
#include "sport_health_sequencer.h"
#include "sport_health_logging.h"
#include "sport_health_algo_utils.h"
#include <time.h>
#include <stdio.h>
#include <stdlib.h>
#include <vmtypes.h>
#include <os.h>
#include <panic.h>
#include <operator.h>
#include <bitserial_api.h>
#include <hydra_log.h>

#define SH_CASUAL_WAKE_IND 0x555

static uint16 *p_sensor_results = NULL;
static imu_sensor_data_t imu_sensor_data = 0;
static ppg_sensor_data_t ppg_sensor_data = 0;

uint16 IMU_LP_SAMPLE_INTERVAL_MS_TABLE[IMU_NUM_LOW_POWER_ODR] = IMU_SUPPORTED_LOW_POWER_ODR;

sh_hub_data_t *p_sh_hub_data = NULL;
sh_sensor_cfg_t *p_sensor_config = NULL;

/* Static allocation for sensor result buffer */
uint32 sensor_rslt_buff[MAX_SENSOR_RSLT_BUFFER_SIZE_WORDS] = {0};

/******************************************************************************
 *              PRIVATE FUNCTIONS DEFINITIONS
 ******************************************************************************/
/**
 * @brief Allocate memory to the hub data structure and intialise
 *        each parameter.
 */
static bool sh_hub_data_initialise(void)
{
    /* Should only ever be initialised once */
    SH_HUB_ASSERT(p_sh_hub_data == NULL);
    p_sh_hub_data = (sh_hub_data_t *)PanicUnlessMalloc(sizeof(sh_hub_data_t));
	
	/* Should only ever be initialised once */
	SH_HUB_ASSERT(p_sensor_config == NULL);
	p_sensor_config = (sh_sensor_cfg_t *)PanicUnlessMalloc(sizeof(sh_sensor_cfg_t));

    if(p_sh_hub_data != NULL)
    {
        SportHealthLoggingVersionInfo();
        p_sh_hub_data->bt_interval_ms = 0;
        p_sh_hub_data->curr_wake = 0;
        p_sh_hub_data->next_wake = 0;
        p_sh_hub_data->wake_count = 0;
        p_sh_hub_data->imu_mode = IMU_MODE_NONE;
        p_sh_hub_data->ppg_mode = PPG_MODE_NONE;
        p_sh_hub_data->prox_mode = PROXIMITY_MODE_NONE;
        p_sh_hub_data->ppg_mode_shadow = PPG_MODE_NONE;
        p_sh_hub_data->in_out_status = SH_IN_OUT_TYPE_OUT;
        p_sh_hub_data->v_motion_detect = FALSE;
        p_sh_hub_data->motion_detect_info = IMU_NO_MOTION;
    }
    return (p_sh_hub_data != NULL);
}
/**
 * @brief Calculates the optimal target window (min and max) for the wake up period
 *        by factoring in min and max from algorithm, FIFO depth and BT wake
 *        up intervals. Based on the calculated wake interval, a buffer is allocated
 *        to store the sensor results.
 *
 * @param sensor_sample_time Sampling interval in millisecond for the configured sensor
 * @return success TRUE/FALSE
 */
static bool sh_hub_cfg_target_time(uint8 sensor_sample_time)
{
    bool success = FALSE;

    uint16 target_early_ms = (uint16)p_sh_hub_data->min_interval_ms;
    uint16 target_late_ms = (uint16)p_sh_hub_data->max_interval_ms;
    uint16 target_ms = 0;
    uint16 buff_len_bytes = 0;

    /**  for each active allowed timer window calculate overall minimum and maximum */
    switch(p_sh_hub_data->bt_mode)
    {
        case SH_VM_BT_MODE_OFF:
            /**  choose the longest allowed period to minimise wakes / save power */
            /**  DEFAULT VALUES ABOVE OK - ASSUMING DS ENABLED -
             * This case allows changing defaults if needed */
            success = TRUE;
            break;
        case SH_VM_BT_MODE_SNIFF:
            /**  Align with the best aligning to fraction or multiple BT sniff periods */
            /**  Alignement shoud occur automatically - this code verifies it can */
            if(p_sh_hub_data->bt_interval_ms < p_sh_hub_data->min_interval_ms)
            {
                /**  e.g. (((1000 - 1)/500) + 1) * 500 = 1000
                 *      (((1000 - 1)/400) + 1) * 400 = 1200
                 **/
                target_ms = (((p_sh_hub_data->min_interval_ms - 1) / p_sh_hub_data->bt_interval_ms)+1) * p_sh_hub_data->bt_interval_ms;
            }
            else if(p_sh_hub_data->bt_interval_ms < p_sh_hub_data->max_interval_ms)
            {
                target_ms = p_sh_hub_data->bt_interval_ms;
            }
            else /**  BT interval > max_interval */
            {
                target_ms = (((p_sh_hub_data->bt_interval_ms - 1) / p_sh_hub_data->max_interval_ms)+1) * p_sh_hub_data->bt_interval_ms;
                target_early_ms = MAX(target_ms - TARGET_WINDOW_SIZE_DIV2_MS, target_early_ms);
                target_late_ms = MIN(target_ms + TARGET_WINDOW_SIZE_DIV2_MS, target_late_ms);
            }

            if ((target_ms < target_early_ms) || (target_ms > target_late_ms))
            {
                /**  WARN - SNIFF CAN NOT ALIGN TO TARGET EARLY-LATE WINDOW */
            }
            success = TRUE;
            break;
        case SH_VM_BT_MODE_ACTIVE:
            /**  choose the shortest allowed period to minimise peak load / peak memory ** may change plan later*/
            target_early_ms = MAX(TARGET_PERIOD_BT_ACTIVE - TARGET_WINDOW_SIZE_DIV2_MS, target_early_ms);
            target_late_ms = MIN(target_early_ms + 2*TARGET_WINDOW_SIZE_DIV2_MS, target_late_ms);
            target_ms = target_late_ms;
            success = TRUE;
        break;
    }

    p_sh_hub_data->target_ms = target_ms;
    p_sh_hub_data->target_early_ms = target_early_ms;
    p_sh_hub_data->target_late_ms = target_late_ms;

    if(sensor_sample_time)
    {
        if(p_sensor_results == NULL)
        {
            /** Since IMU sensor has larger data requirements we keep IMU memory requiremnt
             *  as reference for even PPG */
            buff_len_bytes = (target_late_ms * IMU_DRIVER_FIFO_A_LENGTH * sizeof(uint16))/sensor_sample_time;
            p_sensor_results = (uint16 *)sensor_rslt_buff;
            sh_set_imu_data_buff_length(buff_len_bytes);
        }
    }


    return success;
}
/**
 * @brief Start the casual timer by sending the early and late to vm_timer
 */
static void sh_start_casual_timer(const uint32 settle_offset_ms)
{
    uint32 curr_time =  (uint32) VmGetTimerTime();
    uint32 early = (1000 * (p_sh_hub_data->target_early_ms)) + curr_time;
    uint32 late = (1000 * (p_sh_hub_data->target_late_ms)) + curr_time;

    UNUSED(early); //reserved for future use
    UNUSED(late); //reserved for future use
    uint32 target_ms = MAX(p_sh_hub_data->target_late_ms, settle_offset_ms);
    MessageSendLater(&(p_sh_hub_data->hub_task), SH_CASUAL_WAKE_IND, 0 , target_ms);
}
/**
 * @brief This function based on configured sensor mode sends message
 *        to driver task for corresponding sensor configuration
 * @param p_sensor_cfg Pointer to sensor configuration data structure
 * @return
 */
static bool sh_hub_sensor_config(sh_sensor_cfg_t *p_sensor_cfg)
{
    /* Assume failure until proven otherwise */
    bool success = FALSE;
    uint16 imu_fifo_ms = 0;
    uint16 ppg_fifo_ms = 0;
    uint16 prox_fifo_ms = 0;
    uint16 imu_sensor_rslt_ms = 0;
    uint16 ppg_sensor_rslt_ms = 0;
    uint16 prox_sensor_rslt_ms = 0;
    uint16 max_sensor_rslt_ms = 0;

    p_sh_hub_data->imu_mode = p_sensor_cfg->imu_mode;
    p_sh_hub_data->ppg_mode = p_sensor_cfg->ppg_mode;
    p_sh_hub_data->prox_mode = p_sensor_cfg->prox_mode;
    p_sh_hub_data->algo_min_ms = p_sensor_cfg->algo_min_ms;
    p_sh_hub_data->algo_max_ms = p_sensor_cfg->algo_max_ms;

    switch(p_sensor_cfg->imu_mode)
    {
        case ACCEL_ONLY:
            imu_fifo_ms = IMU_FIFO_DEPTH_ACC * p_sensor_cfg->imu_period;
            imu_sensor_rslt_ms = MAX_SENSOR_RSLT_DEPTH_ACC * p_sensor_cfg->imu_period;
            break;
        case GYRO_ONLY:
            imu_fifo_ms = IMU_FIFO_DEPTH_GYRO * p_sensor_cfg->imu_period;
            imu_sensor_rslt_ms = MAX_SENSOR_RSLT_DEPTH_GYRO * p_sensor_cfg->imu_period;
            break;
        case ACCEL_GYRO_BOTH:
            imu_fifo_ms = IMU_FIFO_DEPTH_ACC_GYRO * p_sensor_cfg->imu_period;
            imu_sensor_rslt_ms = MAX_SENSOR_RSLT_DEPTH_ACC_GYRO * p_sensor_cfg->imu_period;
            break;
        case IMU_MODE_NONE:
            imu_fifo_ms = 0;
            imu_sensor_rslt_ms = 0;
            break;
    }

    switch(p_sensor_cfg->ppg_mode)
    {
        case HR_ONLY:
            ppg_fifo_ms = PPG_FIFO_DEPTH_PPG * p_sensor_cfg->ppg_period;
            ppg_sensor_rslt_ms = MAX_SENSOR_RSLT_DEPTH_PPG * p_sensor_cfg->ppg_period;
            break;
        case SPO2_ONLY:
            ppg_fifo_ms = PPG_FIFO_DEPTH_SPO2 * p_sensor_cfg->ppg_period;
            ppg_sensor_rslt_ms = MAX_SENSOR_RSLT_DEPTH_SPO2 * p_sensor_cfg->ppg_period;
            break;
        case PPG_MODE_NONE:
            ppg_fifo_ms = 0;
            ppg_sensor_rslt_ms = 0;
            break;
    }

    switch(p_sensor_cfg->prox_mode)
    {
        case PROXIMITY_ONLY:
            prox_fifo_ms = PROX_FIFO_DEPTH * p_sensor_cfg->prox_period;
            prox_sensor_rslt_ms = MAX_SENSOR_RSLT_DEPTH_PROX * p_sensor_cfg->prox_period;
            break;
        case PROXIMITY_MODE_NONE:
            prox_fifo_ms = 0;
            prox_sensor_rslt_ms = 0;
            break;
    }

    max_sensor_rslt_ms = MIN(prox_sensor_rslt_ms, MIN(imu_sensor_rslt_ms, ppg_sensor_rslt_ms));

    p_sh_hub_data->fifo_min_ms = FIFO_DEPTH_MIN_MS;
    /** Due to memory contraints max buffer size for storing sensor data is limited to 512 bytes. Therefore,
      * this buffer limit needs to be factored in for max time limit calculation. */
    p_sh_hub_data->fifo_max_ms = MIN(MIN(prox_fifo_ms , MIN(imu_fifo_ms, ppg_fifo_ms)), max_sensor_rslt_ms);
    if(p_sh_hub_data->fifo_max_ms == 0)
        p_sh_hub_data->fifo_max_ms = MAX(prox_fifo_ms, MAX(imu_fifo_ms, ppg_fifo_ms));

    if((p_sh_hub_data->fifo_max_ms < p_sh_hub_data->algo_min_ms) || (p_sh_hub_data->fifo_min_ms > p_sh_hub_data->algo_max_ms))
    {
        /**  BAD CONFIG INSERT WARNING HERE */
    }
    p_sh_hub_data->min_interval_ms = MAX(p_sh_hub_data->fifo_min_ms, p_sh_hub_data->algo_min_ms);
    p_sh_hub_data->max_interval_ms = MIN(p_sh_hub_data->fifo_max_ms, p_sh_hub_data->algo_max_ms);

    /**  Calculate preferred wake interval */
    sh_hub_cfg_target_time(p_sensor_cfg->imu_period);

    /** Configure the IMU sensor */
    if (!sh_get_imu_rate_change_status())
    {
        IMU_CONFIG_REQ_T *imu_cfg_msg;
        imu_cfg_msg = (IMU_CONFIG_REQ_T *)PanicUnlessMalloc(sizeof(IMU_CONFIG_REQ_T));
        if(imu_cfg_msg != NULL)
        {
            imu_cfg_msg->imu_mode = p_sensor_cfg->imu_mode;
            imu_cfg_msg->v_output_data_rate = p_sensor_cfg->imu_odr;
            imu_cfg_msg->v_motion_detect = p_sh_hub_data->v_motion_detect;
            MessageSend(p_sh_hub_data->driver_task, IMU_CONFIG_REQ, imu_cfg_msg);
            success = TRUE;
        }
    }
    else
    {
        IMU_CONFIG_UPDATE_REQ_T *imu_cfg_msg;
        imu_cfg_msg = (IMU_CONFIG_UPDATE_REQ_T *)PanicUnlessMalloc(sizeof(IMU_CONFIG_UPDATE_REQ_T));
        if(imu_cfg_msg != NULL)
        {
            imu_cfg_msg->imu_mode = p_sensor_cfg->imu_mode;
            imu_cfg_msg->v_range = 0;
            imu_cfg_msg->v_output_data_rate = p_sensor_cfg->imu_odr;
            MessageSend(p_sh_hub_data->driver_task, IMU_CONFIG_UPDATE_REQ, imu_cfg_msg);
            success = TRUE;
            return success;
        }
    }
    /** Configure the PPG sensor */
    PPG_CONFIG_REQ_T *ppg_cfg_msg;
    ppg_cfg_msg = (PPG_CONFIG_REQ_T *)PanicUnlessMalloc(sizeof(PPG_CONFIG_REQ_T));
    if(ppg_cfg_msg != NULL)
    {
        ppg_cfg_msg->ppg_mode = p_sensor_cfg->ppg_mode;
        ppg_cfg_msg->v_sample_rate = p_sensor_cfg->ppg_odr;
        MessageSend(p_sh_hub_data->driver_task, PPG_CONFIG_REQ, ppg_cfg_msg);
        success = TRUE;
    }
    /** Configure the PROXIMITY sensor */
    PROX_CONFIG_REQ_T *prox_cfg_msg;
    prox_cfg_msg = (PROX_CONFIG_REQ_T *)PanicUnlessMalloc(sizeof(PROX_CONFIG_REQ_T));
    if(prox_cfg_msg != NULL)
    {
        prox_cfg_msg->prox_mode = p_sensor_cfg->prox_mode;
        prox_cfg_msg->v_sample_rate = p_sensor_cfg->prox_odr;
        MessageSend(p_sh_hub_data->driver_task, PROX_CONFIG_REQ, prox_cfg_msg);
        success = TRUE;
    }
    return success;
}
/**
 * @brief This function on being woken up by the vm_timer
 *        sends FIFO read request to imu sensor. It also
 *        books the next wake up period.
 * @return success TRUE/FALSE
 */
static bool sh_hub_wake_up(void)
{
    /** Assume failure until proven otherwise */
    bool success = FALSE;
    /** When proximity status is SH_IN_OUT_TYPE_OUT, and motion detection is enabled, move the
     * activity monitoring system to sleep in case NO_MOTION is detected.
    */
    if((p_sh_hub_data->in_out_status == SH_IN_OUT_TYPE_OUT) &&
      (p_sh_hub_data->v_motion_detect) && (p_sh_hub_data->motion_detect_info == IMU_NO_MOTION))
    {
        /** Cancel any earlier timer! */
        MessageCancelAll(&(p_sh_hub_data->hub_task), SH_CASUAL_WAKE_IND);
        return TRUE;
    }
    /**  Book next wake period */
    sh_start_casual_timer(0);
    if ((p_sh_hub_data->prox_mode != PROXIMITY_MODE_NONE) && (p_sh_hub_data->imu_mode == IMU_MODE_NONE))
    {
        PROX_READ_FIFO_REQ_T *read_fifo_msg;
        read_fifo_msg = (PROX_READ_FIFO_REQ_T *)PanicUnlessMalloc(sizeof(PROX_READ_FIFO_REQ_T));
        read_fifo_msg->prox_mode = p_sh_hub_data->prox_mode;
        MessageSend(p_sh_hub_data->driver_task, PROX_READ_FIFO_REQ, read_fifo_msg);
    }
#ifdef PPG_ONLY
    /* In the case where only PPG sensor is available */
    if (p_sh_hub_data->ppg_mode != PPG_MODE_NONE)
    {
        PPG_READ_FIFO_REQ_T *read_fifo_msg;
        read_fifo_msg = (PPG_READ_FIFO_REQ_T *)PanicUnlessMalloc(sizeof(PPG_READ_FIFO_REQ_T));
        read_fifo_msg->p_ppg_sensor_data = &ppg_sensor_data;
        if(ppg_sensor_data.red.data.p_ppg_data == NULL)
        {
            read_fifo_msg->p_ppg_sensor_data->red.data.p_ppg_data = (uint16 **)PanicUnlessMalloc(sizeof(uint16*));
            read_fifo_msg->p_ppg_sensor_data->IR.data.p_ppg_data = (uint16 **)PanicUnlessMalloc(sizeof(uint16*));
        }
        *(read_fifo_msg->p_ppg_sensor_data->red.data.p_ppg_data) = p_sensor_results;
        *(read_fifo_msg->p_ppg_sensor_data->IR.data.p_ppg_data)  = p_sensor_results;
        /**  This needs to be fixed with proper feedback from Algos */
        read_fifo_msg->ppg_hr_feedback = HR_DATA_OK; /*sh_seq_get_ppg_feedback();*/
        read_fifo_msg->ppg_mode = p_sh_hub_data->ppg_mode;
        /*L0_DBG_MSG1("PPG Feedback: %d", read_fifo_msg->ppg_hr_feedback);*/
        MessageSend(p_sh_hub_data->driver_task, PPG_READ_FIFO_REQ, read_fifo_msg);
        success = TRUE;
    }
#else
	if (sh_get_imu_rate_change_status())
    {
        /** Cancel the earlier timer! */
        MessageCancelAll(&(p_sh_hub_data->hub_task), SH_CASUAL_WAKE_IND);
    }
    /** After wake up, read the IMU sensor FIFO data */
    if (p_sh_hub_data->imu_mode != IMU_MODE_NONE)
    {
        IMU_READ_FIFO_REQ_T *read_fifo_msg;
        #ifdef DEBUG_MSG_FLOW
        L0_DBG_MSG("HUB Task: Sending IMU_READ_FIFO_REQ");
        #endif
        read_fifo_msg = (IMU_READ_FIFO_REQ_T *)PanicUnlessMalloc(sizeof(IMU_READ_FIFO_REQ_T));
        read_fifo_msg->p_imu_sensor_data = &imu_sensor_data;
        if(imu_sensor_data.accel.data.p_imu_data == NULL)
        {
            read_fifo_msg->p_imu_sensor_data->accel.data.p_imu_data = (int16 **)PanicUnlessMalloc(sizeof(int16*));
        }
        *(read_fifo_msg->p_imu_sensor_data->accel.data.p_imu_data) = (int16 *)p_sensor_results;
        read_fifo_msg->imu_mode = p_sh_hub_data->imu_mode;
        MessageSend(p_sh_hub_data->driver_task, IMU_READ_FIFO_REQ, read_fifo_msg);
        success = TRUE;
    }
	if (sh_get_imu_rate_change_status())
    {
        sh_get_updated_sensor_config(p_sensor_config);
        sh_hub_sensor_config(p_sensor_config);
    }
	
#endif
    return success;
}
/**
 * @brief This function updates the bluetooth interval and mode. On each
 *        change in interval a new target time for wake up is calculated.
 * @param p_message Contains the bluetooth interval and mode.
 * @return
 */
static bool sh_hub_update_timer(SH_VM_SET_TIMING_INFO_REQ_T *p_message)
{
    /** Assume failure until proven otherwise */
    bool success = FALSE;

    p_sh_hub_data->bt_interval_ms = p_message->bt_interval_ms;
    p_sh_hub_data->bt_mode = p_message->bt_mode;
    if(sh_hub_cfg_target_time(0))
            success = TRUE;
     return success;
}
/**
 * @brief This function process the received data from PPG sensor.
 * @param sensor_status TRUE/FALSE
 */
static void sh_hub_process_ppg_data(bool sensor_status)
{
    if(sensor_status == ppg_success)
        sh_seq_process_ppg_data(&ppg_sensor_data);
}
/**
 * @brief This function process the received data from IMU sensor.
 *        Also, if PPG is enabled, sends a FIFO read request to PPG
 *        sensor.
 * @param sensor_status TRUE/FALSE
 */
static void sh_hub_process_imu_data(bool sensor_status)
{
    if(sensor_status == imu_success)
        sh_seq_process_imu_data(&imu_sensor_data);

    /*PROX_READ_FIFO_REQ must be used to detect the proximity in 2 cases:
     * a) When PROXIMITY is enabled, PPG is disabled i.e., ppg_mode is set to PPG_MODE_NONE
     * b) When PROXIMITY is enabled, the current IN/OUT status is set to SH_IN_OUT_TYPE_OUT
    */
    if ((p_sh_hub_data->prox_mode != PROXIMITY_MODE_NONE) &&
       ((p_sh_hub_data->in_out_status == SH_IN_OUT_TYPE_OUT) ||(p_sh_hub_data->ppg_mode == PPG_MODE_NONE)))
    {
        PROX_READ_FIFO_REQ_T *read_fifo_msg;
        read_fifo_msg = (PROX_READ_FIFO_REQ_T *)PanicUnlessMalloc(sizeof(PROX_READ_FIFO_REQ_T));
        read_fifo_msg->prox_mode = p_sh_hub_data->prox_mode;
        MessageSend(p_sh_hub_data->driver_task, PROX_READ_FIFO_REQ, read_fifo_msg);
    }
    /* Have we "flipped" PPG active/inactive mode? Based on this PPG duty cycle changes for
     * low power mode gets activated or deactivated.
     */
    if(sh_seq_ppg_duty_cycle_update(&imu_sensor_data) && (p_sh_hub_data->in_out_status == SH_IN_OUT_TYPE_IN))
    {
       ppg_mode_t temp_ppg_mode = p_sh_hub_data->ppg_mode_shadow;
       p_sh_hub_data->ppg_mode_shadow = p_sh_hub_data->ppg_mode;
       p_sh_hub_data->ppg_mode = temp_ppg_mode;
       /* Below mode goes active in the absence of proximity sensor */
       if (p_sh_hub_data->prox_mode == PROXIMITY_MODE_NONE)
       {
           PPG_SUSPEND_REQ_T * suspend_msg;
           suspend_msg = (PPG_SUSPEND_REQ_T *)PanicUnlessMalloc(sizeof(PPG_SUSPEND_REQ_T));
           suspend_msg->ppg_mode = p_sh_hub_data->ppg_mode;
           // This message toggles the suspend/active state by sending mode=1/0
           MessageSend(p_sh_hub_data->driver_task, PPG_SUSPEND_REQ, suspend_msg);
       }
    }
    if ((p_sh_hub_data->ppg_mode != PPG_MODE_NONE) && (p_sh_hub_data->in_out_status == SH_IN_OUT_TYPE_IN))
    {
        PPG_READ_FIFO_REQ_T *read_fifo_msg;
        read_fifo_msg = (PPG_READ_FIFO_REQ_T *)PanicUnlessMalloc(sizeof(PPG_READ_FIFO_REQ_T));
        read_fifo_msg->p_ppg_sensor_data = &ppg_sensor_data;
        if(ppg_sensor_data.red.data.p_ppg_data == NULL)
        {
            read_fifo_msg->p_ppg_sensor_data->red.data.p_ppg_data = (uint16 **)PanicUnlessMalloc(sizeof(uint16*));
            read_fifo_msg->p_ppg_sensor_data->IR.data.p_ppg_data = (uint16 **)PanicUnlessMalloc(sizeof(uint16*));
        }
        *(read_fifo_msg->p_ppg_sensor_data->red.data.p_ppg_data) = p_sensor_results;
        *(read_fifo_msg->p_ppg_sensor_data->IR.data.p_ppg_data)  = p_sensor_results;
        /* Currently this is fixed to HR_DATA_OK. But if required we can use the
         * feedback "sh_seq_get_ppg_feedback()" from algorithms to adjust the current
         * levels in PPG sensor.
         */
        read_fifo_msg->ppg_hr_feedback = HR_DATA_OK;
        read_fifo_msg->ppg_mode = p_sh_hub_data->ppg_mode;
        MessageSend(p_sh_hub_data->driver_task, PPG_READ_FIFO_REQ, read_fifo_msg);
    }
}
/**
 * @brief Message handler for VM_ENABLE_REQ sent to hub by
 *        customer app task
 * @param pMessage Contains the algorithm enable bitfield
 */
static void sh_vm_enable(SH_VM_ENABLE_REQ_T *p_message)
{
    hub_state_event_t event;

    if(!(p_message->enables))
    {
        return;
    }
    p_sh_hub_data->v_motion_detect = (p_message->enables & (ALGO_EN_MOTION_DETECT))? TRUE : FALSE;
    event.id = HUB_STATE_EVENT_ALGO_ENABLE;
    event.args = (void *)p_message;

    if(sh_hub_sm_handle_event(event))
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = FALSE;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
    }
}
/**
 * @brief Message handler for SH_VM_SET_TIMING_INFO_REQ sent to hub by
 *        customer app task.
 * @param p_message
 */
static void sh_vm_set_timing_info(SH_VM_SET_TIMING_INFO_REQ_T *p_message)
{
    hub_state_event_t event;
    event.id = HUB_STATE_EVENT_UPDATE_TIMER;
    event.args = (void *)p_message;

    if(sh_hub_sm_handle_event(event))
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_SET_TIMING_INFO_CFM);
        message->status = FALSE;
        MessageSend(p_sh_hub_data->app_task, SH_VM_SET_TIMING_INFO_CFM, message);
    }
}
/**
 * @brief Message handler for SH_CASUAL_WAKE_IND sent to hub by vm_timer task
 * @param p_message
 */
static void sh_vm_casual_wake(Message p_message)
{
    UNUSED(p_message);
    uint32 wake_cnt = 0;
    hub_state_event_t event;
    event.id = HUB_STATE_EVENT_ACTIVATE;
    event.args = (void *)(&wake_cnt);

    sh_hub_sm_handle_event(event);
}
/**
 * @brief Message handler for PROX_CONFIG_RSP sent to hub by driver task
 * @param p_message Status of PROX config (SUCCESS/FAILURE)
 */
static void sh_dr_prox_config_rsp(PROX_CONFIG_RSP_T *p_message)
{
    hub_state_event_t event;
    event.id = HUB_STATE_EVENT_PROX_ACTIVATE;
    event.args = (void *)p_message;

    if(p_sh_hub_data->prox_mode == PROXIMITY_MODE_NONE)
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = TRUE;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
        return;
    }

    if(sh_hub_sm_handle_event(event))
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = TRUE;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
    }
}

/**
 * @brief Message handler for IMU_CONFIG_RSP sent to hub by driver task
 * @param p_message Status of IMU config (SUCCESS/FAILURE)
 */
static void sh_imu_config_rsp(IMU_CONFIG_RSP_T * p_message)
{
    hub_state_event_t event;
    event.id = HUB_STATE_EVENT_IMU_ACTIVATE;
    event.args = (void *)p_message;

    if(p_sh_hub_data->imu_mode == IMU_MODE_NONE)
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = TRUE;
        message->sensor_type = IMU;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
        return;
    }

    if(sh_hub_sm_handle_event(event))
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = TRUE;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
    }
}
/**
 * @brief Message handler for IMU_CONFIG_UPDATE_RSP sent to hub by driver task
 * @param p_message IMU settle_time for rate change
 */
static void sh_imu_config_update_rsp(IMU_CONFIG_UPDATE_RSP_T *pMessage)
{
    sh_set_imu_rate_has_changed(FALSE);
    sh_set_check_data_gap(TRUE);
    sh_start_casual_timer(pMessage->settle_time);
}
/**
 * @brief Message handler for IMU_MOTION_DETECT_IND sent to hub by driver task
 * @param p_message Contains IMU motion detect information
 */
static void sh_imu_motion_detect_handle(IMU_MOTION_DETECT_IND_T *pMessage)
{
    p_sh_hub_data->motion_detect_info = pMessage->motion_detect_info;
    /** When a motion is detected trigger wake up intervals, i.e., move
     * activity monitoring to ACTIVE.
    */
    if (p_sh_hub_data->motion_detect_info == IMU_ANY_MOTION)
    {
        /**  Book next wake period */
        sh_start_casual_timer(0);
    }
}
/**
 * @brief Message handler for PPG_CONFIG_RSP sent to hub by driver task
 * @param p_message Status of PPG config (SUCCESS/FAILURE)
 */
static void sh_ppg_config_rsp(PPG_CONFIG_RSP_T * p_message)
{
    hub_state_event_t event;
    event.id = HUB_STATE_EVENT_PPG_ACTIVATE;
    event.args = (void *)p_message;

    if(p_sh_hub_data->ppg_mode == PPG_MODE_NONE)
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = TRUE;
        message->sensor_type = PPG;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
        return;
    }

    if(sh_hub_sm_handle_event(event))
    {
        MAKE_SH_HUB_MESSAGE(SH_VM_ENABLE_CFM);
        message->status = TRUE;
        MessageSend(p_sh_hub_data->app_task, SH_VM_ENABLE_CFM, message);
    }
}
/**
 * @brief Message handler for IMU_READ_FIFO_RSP sent to hub by driver task
 * @param p_message Status of FIFO read (SUCCESS/FAILURE)
 */
static void sh_dr_imu_read_fifo_rsp(IMU_READ_FIFO_RSP_T * p_message)
{
    if(p_message->status == imu_success)
    {
        hub_state_event_t event;
        event.id = HUB_STATE_EVENT_IMU_DATA_RCVD;
        event.args = (void *)p_message;
        if(sh_hub_sm_handle_event(event))
        {
            MessageSend(p_sh_hub_data->app_task, SH_VM_RESULTS_IND, NULL);
        }
    }
}
/**
 * @brief Message handler for PROX_READ_FIFO_RSP sent to hub by driver task
 * @param p_message Status of FIFO read (SUCCESS/FAILURE)
 */
static void sh_dr_prox_read_fifo_rsp(PROX_READ_FIFO_RSP_T * pMessage)
{
    if(pMessage->status == prox_success)
    {
        p_sh_hub_data->in_out_status = pMessage->in_out_status? SH_IN_OUT_TYPE_OUT:SH_IN_OUT_TYPE_IN;
    }
}

/**
 * @brief Message handler for PPG_READ_FIFO_RSP sent to hub by driver task
 * @param p_message Status of FIFO read (SUCCESS/FAILURE)
 */
static void sh_dr_ppg_read_fifo_rsp(PPG_READ_FIFO_RSP_T * p_message)
{
    if(p_message->status == ppg_success)
    {
        hub_state_event_t event;
        event.id =     HUB_STATE_EVENT_PPG_DATA_RCVD;
        event.args = (void *)p_message;

        if(sh_hub_sm_handle_event(event))
        {
            MessageSend(p_sh_hub_data->app_task, SH_VM_RESULTS_IND, NULL);
        }
    }
}
/**
 * @brief This is profile handler to all the messages sent to Hub task
 * @param task Task ID
 * @param id Message ID
 * @param message Message content
 */
static void sh_hub_message_handler ( Task pTask, MessageId pId, Message pMessage )
{
    UNUSED(pTask);
    UNUSED(pMessage);

    switch ((MessageId)pId)
    {
        /**  config messages from application */
        case SH_VM_SET_TIMING_INFO_REQ:
             sh_vm_set_timing_info((SH_VM_SET_TIMING_INFO_REQ_T *)pMessage);
             break;

        case SH_VM_ENABLE_REQ:
             sh_vm_enable((SH_VM_ENABLE_REQ_T *)pMessage);
             break;

        case IMU_CONFIG_RSP:
             sh_imu_config_rsp((IMU_CONFIG_RSP_T *)pMessage);
             break;

        case IMU_CONFIG_UPDATE_RSP:
            sh_imu_config_update_rsp((IMU_CONFIG_UPDATE_RSP_T *)pMessage);
            break;

        case IMU_READ_FIFO_RSP:
             sh_dr_imu_read_fifo_rsp((IMU_READ_FIFO_RSP_T *)pMessage);
             break;

        case IMU_MOTION_DETECT_IND:
             sh_imu_motion_detect_handle((IMU_MOTION_DETECT_IND_T *)pMessage);
             break;

        case PPG_CONFIG_RSP:
             sh_ppg_config_rsp((PPG_CONFIG_RSP_T *)pMessage);
             break;

        case PPG_READ_FIFO_RSP:
             sh_dr_ppg_read_fifo_rsp((PPG_READ_FIFO_RSP_T *)pMessage);
             break;

        case PROX_CONFIG_RSP:
             sh_dr_prox_config_rsp((PROX_CONFIG_RSP_T *)pMessage);
             break;
        case PROX_READ_FIFO_RSP:
             sh_dr_prox_read_fifo_rsp((PROX_READ_FIFO_RSP_T *)pMessage);
             break;

        case SH_CASUAL_WAKE_IND:
             if(p_sh_hub_data)
             {
                 sh_vm_casual_wake(pMessage);
             }
             break;

    }
}
/******************************************************************************
 *              PUBLIC FUNCTIONS DEFINITIONS
 ******************************************************************************/
/**
 * @brief Event handler when the hub state is uninitialised
 *
 * @param event
 *
 * @return success TRUE/FALSE
 */
bool sh_hub_state_uninitialised_handle_event(hub_state_event_t event)
{
    /* Assume failure until proven otherwise */
    bool success = FALSE;

    switch(event.id)
    {
        case HUB_STATE_EVENT_INITIALISE:
        {
            if(sh_hub_data_initialise())
            {
                /** Call the core sequencer function to enable
                 *  the configured algorithm and their dependencies */
                sport_health_sequencer_init();

                /** Initialisation successful */
                sh_hub_set_state(HUB_STATE_READY);
                success = TRUE;
            }
        }
        break;

        default:
        {
            SH_HUB_DEBUG_INFO(("Unhandled event [%d]\n", event.id));
            SH_HUB_PANIC();
        }
    }
    return success;
}
/**
 * @brief Event handler when the hub state is ready i.e.,
 *        hub is initialised and algorithms are enabled as per
 *        configured bitfield in customer app. Also, the required
 *        memory is allocated for each enabled algorithm. After
 *        receiving CONFIG RSP from corresponding sensor, the target
 *        time window is set in vm_casual_timer
 *
 * @param event
 *
 * @return success TRUE/FALSE
 */
bool sh_hub_state_ready_handle_event(hub_state_event_t event)
{
    /* Assume failure until proven otherwise */
    bool success = FALSE;

    /** READY event must have args */
    SH_HUB_ASSERT(event.args != NULL);
    if (event.args == NULL) return FALSE;

    switch(event.id)
    {
        case HUB_STATE_EVENT_ALGO_ENABLE:
        case HUB_STATE_EVENT_ALGO_DISABLE:
        {
			
            SH_VM_ENABLE_REQ_T *p_message = (SH_VM_ENABLE_REQ_T *)event.args;

            if(sh_seq_algo_enable(p_message->enables, p_sensor_config))
            {
                sh_hub_sensor_config(p_sensor_config);
                sh_seq_get_algo_info(p_message->p_algo_info);
            }
        }
        break;

        case HUB_STATE_EVENT_IMU_ACTIVATE:
        {
            IMU_CONFIG_RSP_T *p_message = (IMU_CONFIG_RSP_T *)event.args;
            if((p_sh_hub_data->ppg_mode == PPG_MODE_NONE) && (p_message->status == imu_success)
               && (p_sh_hub_data->prox_mode == PROXIMITY_MODE_NONE))
            {
                sh_start_casual_timer(0);
                /** READY successful */
                sh_hub_set_state(HUB_STATE_ACTIVE);
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_PPG_ACTIVATE:
        {
            PPG_CONFIG_RSP_T *p_message = (PPG_CONFIG_RSP_T *)event.args;
            if((p_message->status == ppg_success)&& (p_sh_hub_data->prox_mode == PROXIMITY_MODE_NONE))
            {
                /* When Proximity mode is none, the status is set as in ear by default */
                p_sh_hub_data->in_out_status = SH_IN_OUT_TYPE_IN;
                sh_start_casual_timer(0);
                /** READY successful */
                sh_hub_set_state(HUB_STATE_ACTIVE);
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_PROX_ACTIVATE:
        {
            PROX_CONFIG_RSP_T *p_message = (PROX_CONFIG_RSP_T *)event.args;
            if(p_message->status == ppg_success)
            {
                sh_start_casual_timer(0);
                /** READY successful */
                sh_hub_set_state(HUB_STATE_ACTIVE);
                success = TRUE;
            }
        }
        break;

        /** This can happen when after algo disable in ACTIVE state,
         * hub sends the disable request to sensor and while for waiting
         * fo response is woken up by vm_timer */
        case HUB_STATE_EVENT_ACTIVATE:
        {
            success = TRUE;
        }
        break;

        case HUB_STATE_EVENT_UPDATE_TIMER:
        {
            SH_VM_SET_TIMING_INFO_REQ_T *p_message = (SH_VM_SET_TIMING_INFO_REQ_T *)event.args;

            if(sh_hub_update_timer(p_message))
            {
                success = TRUE;
            }
        }
        break;

        default:
        {
            SH_HUB_DEBUG_INFO(("Unhandled event [%d]\n", event.id));
            SH_HUB_PANIC();
        }
    }
    return success;
}
/**
 * @brief Event handler when the hub state is active i.e.,
 *        hub is woken up by the vm_timer and the sensor data
 *        is received for processing. On receiving new algorithm
 *        enable REQ, HUB state remains unchanged for partial disable
 *        and makes transition back to READY state only for complete
 *        disable. For BT time updae there is no change in state.
 *
 * @param event
 *
 * @return success TRUE/FALSE
 */
bool sh_hub_state_active_handle_event(hub_state_event_t event)
{
    /* Assume failure until proven otherwise */
    bool success = FALSE;

    /** UPDATE_TIMER event must have args */
    SH_HUB_ASSERT(event.args != NULL);
    if (event.args == NULL) return FALSE;

    switch(event.id)
    {
        case HUB_STATE_EVENT_ACTIVATE:
        {
            if(sh_hub_wake_up())
            {
                /** ACTIVE successful */
                sh_hub_set_state(HUB_STATE_ACTIVE);
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_IMU_DATA_RCVD:
        {
            IMU_READ_FIFO_RSP_T *p_message = (IMU_READ_FIFO_RSP_T *)event.args;
            sh_hub_process_imu_data(p_message->status);
            if((p_sh_hub_data->ppg_mode == PPG_MODE_NONE) && (p_message->status == imu_success))
            {
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_PPG_DATA_RCVD:
        {
            PPG_READ_FIFO_RSP_T *p_message = (PPG_READ_FIFO_RSP_T *)event.args;
            if (p_sh_hub_data->in_out_status == SH_IN_OUT_TYPE_IN)
            {
                sh_hub_process_ppg_data(p_message->status);
            }
            if(p_message->status == ppg_success)
            {
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_ALGO_ENABLE:
        {
            SH_VM_ENABLE_REQ_T *p_message = (SH_VM_ENABLE_REQ_T *)event.args;

            if(p_message->enables)
            {
                if(sh_seq_algo_enable(p_message->enables, p_sensor_config))
                {
                    sh_seq_get_algo_info(p_message->p_algo_info);
                }
            }
            else
            {
                event.id = HUB_STATE_EVENT_ALGO_DISABLE;
                /* Moves to READY state */
                sh_hub_set_state(HUB_STATE_READY);
                sh_hub_sm_handle_event(event);
            }
        }
        break;

        case HUB_STATE_EVENT_IMU_ACTIVATE:
        {
            IMU_CONFIG_RSP_T *p_message = (IMU_CONFIG_RSP_T *)event.args;
            if((p_sh_hub_data->ppg_mode == PPG_MODE_NONE) && (p_message->status == imu_success))
            {
                /** Remains in ACTIVE state */
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_PPG_ACTIVATE:
        {
            PPG_CONFIG_RSP_T *p_message = (PPG_CONFIG_RSP_T *)event.args;
            if(p_message->status == ppg_success)
            {
                /** Remains in ACTIVE state */
                success = TRUE;
            }
        }
        break;

        case HUB_STATE_EVENT_UPDATE_TIMER:
        {
            SH_VM_SET_TIMING_INFO_REQ_T *p_message = (SH_VM_SET_TIMING_INFO_REQ_T *)event.args;

            if(sh_hub_update_timer(p_message))
            {
                /** Remains in ACTIVE state */
                success = TRUE;
            }
        }
        break;

        default:
        {
            SH_HUB_DEBUG_INFO(("Unhandled event [%d]\n", event.id));
            SH_HUB_PANIC();
        }
    }
    return success;
}
/**
 * @brief Sets the state for the hub
 * @param state (in) state to be set for the hub
 */
void sh_hub_set_state(hub_state_t state)
{
    SH_HUB_ASSERT(p_sh_hub_data != NULL);
    p_sh_hub_data->state = state;
}
/**
 * @brief Gets the state for the hub
 * @return The current state of the hub
 */
hub_state_t sh_hub_get_state(void)
{
    if (p_sh_hub_data == NULL)
    {
        /* If we haven't allocated the library data yet then we must be in
           uninitialised state */
        return HUB_STATE_UNINITIALISE;
    }
    return p_sh_hub_data->state;
}
/**
 * @brief Initialise the Hub and create the corresponding task
 * @param app_task Task Id for customer app
 */
Task SportHealthHubInit(Task app_task)
{
    hub_state_event_t event;
    event.id = HUB_STATE_EVENT_INITIALISE;
    event.args = NULL;
    /* Allocate memory pool for Activity Monitoring */
    if(sh_hub_sm_handle_event(event))
    {
        p_sh_hub_data->hub_task.handler = sh_hub_message_handler;
        p_sh_hub_data->app_task = app_task;
        return(&(p_sh_hub_data->hub_task));
    }
    else
    {
        return NULL;
    }
}
/**
 * @brief Register the driver (IMUPPG) task in hub
 * @param driver_task Task Id for the IMUPPG task
 */
void SportHealthHubCfg(Task driver_task)
{
    p_sh_hub_data->driver_task = driver_task;
}
/**
 * @brief Delete the hub task as application has triggered
 *        teardown
 */
void sport_health_hub_delete(void)
{
    /** Clear any pending timer requests */
    MessageCancelAll(&(p_sh_hub_data->hub_task), SH_CASUAL_WAKE_IND);
    if(p_sh_hub_data)
    {
        /** Clear the memory allocated for Hub Task */
        free(p_sh_hub_data);
        p_sh_hub_data = NULL;
    }
}
/**
 * @brief Notify the driver about the detected interrupt
 */
void sh_interrupt_notify(void)
{
    /* Send IMU interrupt notification from Hub to Driver */
    MessageSend(p_sh_hub_data->driver_task, IMU_INTERRUPT_IND, 0);
}

