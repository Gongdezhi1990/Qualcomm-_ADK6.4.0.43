
/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2018

    This defines public utility functions processing sensor data
    for the sport and health algorithms
*/

#include "sport_health_algorithms.h"
#include "sport_health_hub.h"
#include "sport_health_algo_utils.h"
#include "macros.h"
#include "hydra_log.h"
#include "stdlib.h"

static imu_rate_change_info_t *p_imu_rate_change_info;
static imu_rate_change_status_t imu_rate_change_status;

/**
 * @brief If we have PPG power saving switched on, decide whether the
 *     right conditions are present for power save and update the state
 *     if necessary
 *
 * @param hs_algo_info_t *seq_context    sequencer context
 *        const uint32 timestamp         in ms, from current buffer of IMU data
 */
void update_ppg_duty_cycle(sh_vm_algo_info_t *seq_context, const uint32 timestamp)
{

/* If we are in power save mode, evaluate and possibly change our
 * state: active->inactive or inactive->active */

    /* We have to satisfy the following conditions to be in p/s mode
     * 1. step status must be not stepping
     * 2. ppg signal quality must not be unknown
     * 3. recent history of hr estimate must be stable */
    if(SportHealthAlgorithmsGetStepStatus(seq_context->step_info) != SH_STEP_STATUS_STEPPING)
    {
        /* Apply timestamp conditions, if we have been inactive long enough, it's time to wake */
        uint32 dur_since_change = timestamp - seq_context->ppg_power_mode_info->state_entry_time;
        if(seq_context->ppg_power_mode_info->ppg_mode == SH_PPG_INACTIVE)
        {
            if(dur_since_change > HR_POWER_SAVE_DUTY_PERIOD - HR_POWER_SAVE_ACTIVE_PERIOD)
            {
                seq_context->ppg_power_mode_info->ppg_mode = SH_PPG_ACTIVE;
                seq_context->ppg_power_mode_info->state_entry_time = timestamp;
            }
            return;
        }

        sh_hr_feedback_t ppg_feedback = SportHealthAlgorithmsGetHeartRateSignalQuality(seq_context->heart_rate_info);
        if(ppg_feedback == SH_HR_SIGNAL_QUAL_UNKNOWN)
        {
            // Remain (or switch to) ACTIVE as conditions not satisfied
            seq_context->ppg_power_mode_info->state_entry_time = timestamp;
            return;
        }
        // So, signal quality is good, okay or poor
        uint16 history_size = SportHealthAlgorithmsGetHeartRateHistorySize(seq_context->heart_rate_info);
        sh_vm_heart_rate_info_t heart_rate;
        uint16 bpm_max, bpm_min;

        if(SportHealthAlgorithmsGetHeartRateAtIdx(&heart_rate, 0, seq_context->heart_rate_info) == SH_FAILURE)
        {
            return;
        }
        bpm_max = heart_rate.rate_bpm;
        bpm_min = heart_rate.rate_bpm;
        uint8 i;
        for (i = 1; i < history_size; i++)
        {
            if(SportHealthAlgorithmsGetHeartRateAtIdx(&heart_rate, i, seq_context->heart_rate_info) == SH_FAILURE)
            {
                return;
            }
            bpm_max = MAX(heart_rate.rate_bpm, bpm_max);
            bpm_min = MIN(heart_rate.rate_bpm, bpm_min);
        }
        if((bpm_max - bpm_min > BPM_VARIATION_FOR_POWER_SAVE) || (history_size < MIN_HR_HISTORY))
        {
            // Remain ACTIVE as HR stability condition is not satisfied
            seq_context->ppg_power_mode_info->state_entry_time = timestamp;
            return;
        }
        if(dur_since_change > HR_POWER_SAVE_ACTIVE_PERIOD)
        {
            seq_context->ppg_power_mode_info->ppg_mode = SH_PPG_INACTIVE;
            seq_context->ppg_power_mode_info->state_entry_time = timestamp;
        }
    }
    else
    {
        // Remain (or switch to) ACTIVE as user is walking or running
        seq_context->ppg_power_mode_info->ppg_mode = SH_PPG_ACTIVE;
        seq_context->ppg_power_mode_info->state_entry_time = timestamp;
    }
}
/**
 * @brief retrieve the record of whether the desired imu data rate has changed
 * @param void
 * @return bool
 */
bool sh_get_desired_data_rate_has_changed(void)
{
    return imu_rate_change_status.desired_data_rate_has_changed;
}
/**
 * @brief Fill in gap in IMU data due to IMU settle time
 * @param p_imu_rate_change_info pointer to structure containing info needed to interpolate data after a rate change
 * @param p_imu_input_data pointer to buffer containing latest IMU data
 * @return void
 */
void sh_fill_data_gap(sh_vm_algo_info_t *seq_context, const imu_rate_change_info_t *p_imu_rate_change_info, imu_input_data_t *p_imu_input_data){
    /* Function to fill in 'data gap' due to a change in the data rate (there is a period in which no data is
     * output when a data rate transition is being executed).  This is done by linear interpolation between
     * the last sensor data at the old data rate (we always retain these values) and the first data at the new
     rate. */

    uint32 current_data_duration_ms = (p_imu_input_data->frame_count-1)*p_imu_input_data->sampling_interval;
    uint8  new_sample_counter;

    /* Check that the data gap and sample rate are positive.  Otherwise something has gone wrong */
    if (current_data_duration_ms + p_imu_rate_change_info->last_sample_time < p_imu_input_data->last_sample_time &&
            p_imu_input_data->sampling_interval > 0)
    {
        uint32 time_gap_ms = p_imu_input_data->last_sample_time - current_data_duration_ms -
                p_imu_rate_change_info->last_sample_time;

        /* Calculate how many sample periods need to be covered by interpolation - at the new data rate */
        uint8 n_intervals = (uint8)(time_gap_ms/p_imu_input_data->sampling_interval);

        if (n_intervals < MIN_NUM_DATA_INTERVALS)
            return; /* Shouldn't get here: we expect to have to insert at least one sample! */

        /* Ensure we don't try to insert more samples than we have buffer-space for */
        uint8 n_samples_to_insert = MIN(n_intervals-1, (imu_rate_change_status.imu_data_buff_length)/IMU_DRIVER_FIFO_A_LENGTH -(p_imu_input_data->frame_count));
        /*BMI160_NUM_VAL_PER_SAMPLE is for 3 axes (x,y,z), 2 bytes per axis*/

        /*  Remember how many samples were added, so we can suppress false taps at the end points of the interpolated data  */
        SportHealthAlgorithmsSetTapParam(seq_context->tap_info, SH_TAP_PARAM_N_SAMPLES_INSERTED, &n_samples_to_insert);

        /* If there are n samples to add, the number of intervals between samples is one greater - needed for delta_x etc. */
        n_intervals = n_samples_to_insert + 1;

        /* Calculate the number of bytes in the buffer currently occupied prior to inserting the new interpolated values -
         this is the #bytes we will need to shift along the buffer to make space for the new data */
        uint32 n_bytes = IMU_NUM_VAL_PER_SAMPLE*sizeof(int16)*(p_imu_input_data->frame_count);

        /* We will shift from the start of the buffer (p_source), far enough along (to p_dest) that we can fit the new data in
        from the start of the buffer */
        int16 *p_source = p_imu_input_data->p_imu_data;
        int16 *p_dest   = p_source + IMU_NUM_VAL_PER_SAMPLE*n_samples_to_insert; /* assuming pointer increments by 2 bytes */

        /* Calculate the amounts by which each successive interpolated value will change from its predecessor.
           This calculation is for LINEAR interpolation so we divide the gap we must fill by the number of
           periods covered */
        int16 delta_x = (p_source[0] - p_imu_rate_change_info->last_x_sample)/(n_intervals);
        int16 delta_y = (p_source[1] - p_imu_rate_change_info->last_y_sample)/(n_intervals);
        int16 delta_z = (p_source[2] - p_imu_rate_change_info->last_z_sample)/(n_intervals);

        /* move the contents of the buffer to 'make room' for the interpolated values, and update the frame count */
        memmove(p_dest, p_source, n_bytes);
        p_imu_input_data->frame_count = p_imu_input_data->frame_count + n_samples_to_insert;
        /* Now fill the gap*/
        for(new_sample_counter = 1; new_sample_counter <= n_samples_to_insert; new_sample_counter++)
        {
          *(p_source) = p_imu_rate_change_info->last_x_sample + new_sample_counter*delta_x;
          ++p_source;
          *(p_source) = p_imu_rate_change_info->last_y_sample + new_sample_counter*delta_y;
          ++p_source;
          *(p_source) = p_imu_rate_change_info->last_z_sample + new_sample_counter*delta_z;
          ++p_source;
        }
    }

}
/**
 * @brief initialise the fields of the structure needed for imu data rate changes
 * @param void
 * @return void
 */
void sh_initialise_rate_change_data(void)
{
    p_imu_rate_change_info = NULL;

    imu_rate_change_status.curr_status = SH_STEP_STATUS_UNKNOWN;
    imu_rate_change_status.desired_data_rate_has_changed = FALSE;
    imu_rate_change_status.imu_data_buff_length = 0;
}
/**
 * @brief Maintain the decision information for issuing rate change commands.  If step says we have
 * made a transition between being stationary and some other status, and the  new state persists,
 * then we decide we should change the IMU data rate.
 * @param p_step_info pointer to the structure containing status (stationary, not stepping etc.)
 * @param last_sample_time the time of the last data sample in the buffer
 * @return void
 */
void sh_update_imu_rate_change_decision_info(sh_vm_step_info_public_t *p_step_info, uint32 last_sample_time)
{
    sh_step_status_t step_status = SportHealthAlgorithmsGetStepStatus(p_step_info);

    /* If the latest step_status is different from last time, record the change and the time at which it occurred and
     reset the time for which the change has persisted.  Remember what transition occurred.  Otherwise, if there was
     a transition recently, calculate how long ago it happened and if the new status has persisted for long enough, record that
     we need an IMU data rate change and prepare for any future status change by reseting our recollection of  this status change */
    if (step_status != imu_rate_change_status.curr_status)
    {
        if (p_imu_rate_change_info == NULL)
        {
            p_imu_rate_change_info = (imu_rate_change_info_t *)malloc(sizeof(imu_rate_change_info_t));
        }

        if (p_imu_rate_change_info != NULL)
        {
            p_imu_rate_change_info->imu_rate_decision_info.prev_status = imu_rate_change_status.curr_status;
            p_imu_rate_change_info->imu_rate_decision_info.time_of_step_status_change = last_sample_time;
            p_imu_rate_change_info->imu_rate_decision_info.time_since_step_status_change = 0;
            p_imu_rate_change_info->check_for_data_gap = FALSE;

            imu_rate_change_status.curr_status = step_status;
        }

    }
    else if ((p_imu_rate_change_info != NULL) && (p_imu_rate_change_info->imu_rate_decision_info.prev_status != imu_rate_change_status.curr_status))
    {
        /* Here, a status change has persisted - Calculate how long for. */
        p_imu_rate_change_info->imu_rate_decision_info.time_since_step_status_change =
                last_sample_time - p_imu_rate_change_info->imu_rate_decision_info.time_of_step_status_change;

        /* If the status change has persisted for long enough, check if it was a change to or from a stationary condition,
        and if it was, record the new data rate that the IMU should be set to */
        if (p_imu_rate_change_info->imu_rate_decision_info.time_since_step_status_change > NEW_STEP_STATUS_DWELL_TIME_MS)
        {
            switch (imu_rate_change_status.curr_status){
            case SH_STEP_STATUS_STATIONARY:
                sh_set_data_rate(IMU_OUTPUT_DATA_RATE_50HZ);
                break;
            default:
                sh_set_data_rate(IMU_OUTPUT_DATA_RATE_100HZ);
                break;
            }
            /* Any needed IMU data rate change has been recorded - so prepare for the next one. */
            p_imu_rate_change_info->imu_rate_decision_info.prev_status = imu_rate_change_status.curr_status;
            p_imu_rate_change_info->imu_rate_decision_info.time_since_step_status_change = 0;
        }

    }

}
/**
 * @brief Linearly interpolate for any data gap due to IMU settling time at rate transitions
 *        and store the most recent sample values.
 * @param p_imu_input_data Pointer to structure for input data
 */
void sh_deal_with_potential_data_rate_changes(sh_vm_algo_info_t *seq_context, imu_input_data_t *p_imu_input_data){

    if (p_imu_rate_change_info == NULL)
        return;

    if (p_imu_rate_change_info->check_for_data_gap &&
            p_imu_input_data->sampling_interval == p_imu_rate_change_info->imu_period)
    {
        /* do the interpolation */
        sh_fill_data_gap(seq_context, p_imu_rate_change_info, p_imu_input_data);
        free(p_imu_rate_change_info);
        p_imu_rate_change_info = NULL;
    }
    else
    {
        /* Store the last IMU sample */
        p_imu_rate_change_info->last_x_sample = *(p_imu_input_data->p_imu_data + IMU_NUM_VAL_PER_SAMPLE*(p_imu_input_data->frame_count-1));
        p_imu_rate_change_info->last_y_sample = *(p_imu_input_data->p_imu_data + IMU_NUM_VAL_PER_SAMPLE*(p_imu_input_data->frame_count-1) + 1);
        p_imu_rate_change_info->last_z_sample = *(p_imu_input_data->p_imu_data + IMU_NUM_VAL_PER_SAMPLE*(p_imu_input_data->frame_count-1) + 2);
        p_imu_rate_change_info->last_sample_time = p_imu_input_data->last_sample_time;
    }

}
/**
 * @brief set the appropriate data rate for the IMU
 * @param new_data_rate the desired data rate
 * @return void
 */
 void sh_set_data_rate(imu_output_data_rate_t new_data_rate)
 {
     if(p_imu_rate_change_info->imu_data_rate != new_data_rate)
     {
         p_imu_rate_change_info->imu_data_rate = new_data_rate;
         imu_rate_change_status.desired_data_rate_has_changed = TRUE;
         p_imu_rate_change_info->imu_period =  IMU_LP_SAMPLE_INTERVAL_MS_TABLE[p_imu_rate_change_info->imu_data_rate];
     }
 }
 /**
  * @brief record that the desired imu data rate has changed
  * @param has_changed a bool indicating whether the desired imu data rate has changed
  * @return void
  */
  void sh_set_imu_rate_has_changed(bool has_changed)
  {
      imu_rate_change_status.desired_data_rate_has_changed = has_changed;
  }
 /**
  * @brief record the length of the buffer allocated for imu data
  * @param buff_size_bytes the size o0f the buffer allocated for imu data
  * @return void
  */
  void sh_set_imu_data_buff_length(const uint32 buff_size_bytes)
  {
      imu_rate_change_status.imu_data_buff_length = buff_size_bytes;
  }
 /**
  * @brief get the desired imu data rate
  * @param void
  * @return the desired imu data rate
  */
  imu_output_data_rate_t sh_get_imu_rate(void)
  {
      return p_imu_rate_change_info->imu_data_rate;
  }
  /**
   * @brief retrieve stored desired imu rate and update sensor config with this info
   * @param p_sensor_config a pointer to the sensor config structure to be updated
   * @return void
   */
  void sh_get_updated_sensor_config(sh_sensor_cfg_t *p_sensor_config)
  {
      p_sensor_config->imu_odr = sh_get_imu_rate();
      p_sensor_config->imu_period =  IMU_LP_SAMPLE_INTERVAL_MS_TABLE[p_sensor_config->imu_odr];
  }
 /**
 * @brief retrieve the bool stating whether the desired imu data rate has changed
 * @param void
 * @return the bool stating whether the desired imu data rate has changed
 */
 bool sh_get_imu_rate_change_status(void)
 {
     return imu_rate_change_status.desired_data_rate_has_changed;
 }
 /**
 * @brief record whether the possible existence of a data gap due to imu
 * settling time at a rate change should be examined
 * @param check_data_gap whether ther is a potential need for interpolation
 * @return void
 */
 void sh_set_check_data_gap(bool check_data_gap)
 {
     p_imu_rate_change_info->check_for_data_gap = check_data_gap;
 }

