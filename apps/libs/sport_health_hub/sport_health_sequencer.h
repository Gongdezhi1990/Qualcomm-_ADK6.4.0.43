/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016

\file
    This defines functions for accessing and sequencing the sport and health algorithms
*/

#ifndef SPORT_HEALTH_SEQUENCER_H
#define SPORT_HEALTH_SEQUENCER_H

#include <sport_health_hub.h>

/* encode LED operation */
#define LED_NONE            (0)
#define RED_LED_ON          (1)
#define IR_LED_ON           (2)

/**
* @brief Initialise the sequencer
*/
extern void sport_health_sequencer_init(void);

/**
 * @brief Process the IMU data received from sensor by passing to
 *        different algorithms
 * @param pMessage Pointer to received IMU sensor data
 */
void sh_seq_process_imu_data(imu_sensor_data_t *pMessage);
/**
 * @brief Process the PPG data received from sensor by passing to
 *        different algorithms
 * @param pMessage Pointer to received PPG sensor data
 */
void sh_seq_process_ppg_data(ppg_sensor_data_t *pMessage);
/**
 * @brief Based on the algorithm enable bitfield received from customer
 *        app this function will enabled all the dependent algorithms and
 *        allocates context memory to each enabled algorithm.
 * @param p_message Contains the algorithm enable bitfield
 * @param p_sensor_cfg (out) Pointer to store the sensor configurations
 * @return Returns different sensor configuration like max and min rates for each
 *         sensor based on input from each algorithm
 */
bool sh_seq_algo_enable(uint32 enables, sh_sensor_cfg_t *p_sensor_cfg);
/**
 * @brief Stores the pointer to algorithm information memory which can then
 *        be used by customer app
 * @param p_algo_info (out) Pointer to algorithm information memory owned by sequencer.
 */
void sh_seq_get_algo_info(sh_vm_algo_info_t *p_algo_info);

/**
 * @brief To get the heart rate signal quality to send as feedback
 *        to PPG sensor
 * @return signal quality (Good/Ok/Bad/Very Bad)
 */
ppg_hr_feedback_t sh_seq_get_ppg_feedback(void);


#endif /* SPORT_HEALTH_SEQUENCER_H */
