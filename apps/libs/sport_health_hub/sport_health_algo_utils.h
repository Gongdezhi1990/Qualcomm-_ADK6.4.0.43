/*******************************************************************************
Copyright (c) 2018 Qualcomm Technologies International, Ltd.

FILE NAME
    sport_health_algo_utils.h

DESCRIPTION
    This defines public utility functions processing sensor data
    for the sport and health algorithms
*/

#ifndef SPORT_HEALTH_ALGO_UTILS_H
#define SPORT_HEALTH_ALGO_UTILS_H

#include <types.h>
#include "sport_health_algorithms.h"

#define BPM_VARIATION_FOR_POWER_SAVE 4
#define MIN_HR_HISTORY 4
#define HR_POWER_SAVE_DUTY_PERIOD 120000  // ms for entire cycle
#define HR_POWER_SAVE_ACTIVE_PERIOD 10000 // ms for "on" part of cycle

#define NEW_STEP_STATUS_DWELL_TIME_MS 3000 /*3 seconds */
#define MIN_NUM_DATA_INTERVALS        2    /* Minimum number of data points to insert is one less than this */

/** Structure for holding IMU rate change decision stuff */
typedef struct
{
    uint32 time_of_step_status_change;
    uint32 time_since_step_status_change;
    sh_step_status_t  prev_status;
} imu_rate_decision_info_t;

/** Structure for holding IMU data rate change info */
typedef struct
{
    uint32                   last_sample_time;
    int16                    last_x_sample;
    int16                    last_y_sample;
    int16                    last_z_sample;
    imu_output_data_rate_t   imu_data_rate;
    uint8                    imu_period;
    bool                     check_for_data_gap;
    imu_rate_decision_info_t imu_rate_decision_info;
} imu_rate_change_info_t;

/** Structure for holding data needed for monitoring the need to change IMU data rate */
typedef struct
{
    uint32            imu_data_buff_length;
    sh_step_status_t  curr_status;
    bool              desired_data_rate_has_changed;
} imu_rate_change_status_t;

/**
 * @brief To update the ppg duty cycle status based on current
 * hr stability and use context
 * @return void
 */
void update_ppg_duty_cycle(sh_vm_algo_info_t *seq_context, const uint32 timestamp);
/**
 * @brief Fill in gap in IMU data due to IMU settle time
 * @param p_imu_rate_change_info pointer to structure containing info needed to interpolate data after a rate change
 * @param p_imu_input_data pointer to buffer containing latest IMU data
 * @return void
 */
void sh_fill_data_gap(sh_vm_algo_info_t *seq_context, const imu_rate_change_info_t *p_imu_rate_change_info, imu_input_data_t *p_imu_input_data);
/**
 * @brief set the appropriate data rate for the IMU
 * @param new_data_rate the desired data rate
 * @return void
 */
void sh_set_data_rate(imu_output_data_rate_t new_data_rate);
/**
 * @brief retrieve the bool stating whether the desired imu data rate has changed
 * @param void
 * @return the bool stating whether the desired imu data rate has changed
 */
bool sh_get_imu_rate_change_status(void);
/**
 * @brief record that the desired imu data rate has changed
 * @param has_changed a bool indicating whether the desired imu data rate has changed
 * @return void
 */
void sh_set_imu_rate_has_changed(bool has_changed);
/**
 * @brief record the length of the buffer allocated for imu data
 * @param buff_size_bytes the size o0f the buffer allocated for imu data
 * @return void
 */
void sh_set_imu_data_buff_length(const uint32 buff_size_bytes);
/**
 * @brief get the desired imu data rate
 * @param void
 * @return the desired imu data rate
 */
imu_output_data_rate_t sh_get_imu_rate(void);
/**
 * @brief retrieve stored desired imu rate and update sensor config with this info
 * @param p_sensor_config a pointer to the sensor config structure to be updated
 * @return void
 */
void sh_get_updated_sensor_config(sh_sensor_cfg_t *p_sensor_config);
/**
 * @brief retrieve stored desired imu rate and update sensor config with this info
 * @param p_sensor_config a pointer to the sensor config structure to be updated
 * @return void
 */
void sh_set_check_data_gap(bool check_data_gap);
/**
 * @brief initialise the fields of the structure needed for imu data rate changes
 * @param void
 * @return void
 */
void sh_initialise_rate_change_data(void);

/**
 * @brief retrieve the record of whether the desired imu data rate has changed
 * @param void
 * @return bool
 */
bool sh_get_desired_data_rate_has_changed(void);

/**
* @brief Maintain the decision information for issuing rate change commands.  If step says we have
* made a transition between being stationary and some other status, and the  new state persists,
* then we decide we should change the IMU data rate.
* @param p_step_info pointer to the structure containing status (stationary, not stepping etc.)
* @param last_sample_time the time of the last data sample in the buffer
* @return void
*/
void sh_update_imu_rate_change_decision_info(sh_vm_step_info_public_t *p_step_info, uint32 last_sample_time);

/**
 * @brief Linearly interpolate for any data gap due to IMU settling time at rate transitions
 *        and store the most recent sample values.
 * @param p_imu_input_data Pointer to structure for input data
 */
void sh_deal_with_potential_data_rate_changes(sh_vm_algo_info_t *seq_context, imu_input_data_t *p_imu_input_data);

#endif // SPORT_HEALTH_ALGO_UTILS_H


//#endif // SPORT_HEALTH_ALGO_UTILS_H
