/*!
Copyright (C) Qualcomm Technologies International, Ltd. 2016

\file
    This defines functions for accessing and sequencing the sport and health algorithms
*/

#include "sport_health_algorithms.h"
#include "sport_health_sequencer.h"
#include "sport_health_logging.h"
#include "sport_health_algo_utils.h"
#include "macros.h"
#include "message.h"
#include <panic.h>
#include "hydra_log.h"
#include "stdlib.h"

/*#ifndef ENABLE_ATTITUDE
#define ENABLE_ATTITUDE
#endif*/

/** Define the sensor specific look up tables for supported output data rates and range */
static uint16 PPG_SAMPLE_RATE_TABLE[PPG_NUM_SAMPLE_RATE] = PPG_SPO2_SAMPLE_RATE;
/** Define the algorithm dependency look up tables */
static uint16 ALGORITHM_DEPENDENCY_TABLE[NUM_AVAILABLE_ALGORITHMS][NUM_DEPENDENT_PARAMS] = ALGORITHM_DEPENDENCY_LOOK_UP;
/** Define static sructures for storing algorithm context and results */
static sh_vm_algo_info_t seq_context;
/** Define static variables to mark whether we are in PPG power save mode, current state and time of entry */

/**
 * @brief This function initialises the sensor config data structure
 * @param p_sensor_cfg Pointer to the sensor config data structure
 */
static void sh_seq_sensor_cfg_init(sh_sensor_cfg_t *p_sensor_cfg)
{
    p_sensor_cfg->algo_max_ms = ALGORITHM_DEPENDENCY_TABLE[ALGO_DISABLED][ALGO_MAX_PERIOD];
    p_sensor_cfg->algo_min_ms = ALGORITHM_DEPENDENCY_TABLE[ALGO_DISABLED][ALGO_MIN_PERIOD];;
    p_sensor_cfg->imu_odr = 0;
    p_sensor_cfg->ppg_odr = 0;
    p_sensor_cfg->prox_odr = 0;
    p_sensor_cfg->imu_period = 0;
    p_sensor_cfg->ppg_period = 0;
    p_sensor_cfg->prox_period = 0;
    p_sensor_cfg->imu_mode = IMU_MODE_NONE;
    p_sensor_cfg->ppg_mode = PPG_MODE_NONE;
    p_sensor_cfg->prox_mode = PROXIMITY_MODE_NONE;
	
	sh_initialise_rate_change_data();
}

/**
 * @brief This function derives all the dependent algorithms to be enabled based on
 *        algorithm enable bitfield sent by customer app.
 *
 * @param p_message Contains the algorithm enable bitfield set by customer app
 * @return Returns the bitfield containing complete set algorithms and the dependency to
 *         be enabled
 */
static uint32 sh_seq_enable_dependent_algorithms(uint32 algo_enables)
{
    uint8 algo_en_idx = 0;
    uint32 algo_en = 0;
    uint32 enables = algo_enables;

    /** Derive the bitfield for dependent algorithms from algorithm enable
     *  bitfield configured by he customer app */
    for(algo_en_idx = 0; algo_en_idx < NUM_AVAILABLE_ALGORITHMS; algo_en_idx++)
    {
        algo_en = ((algo_enables >> algo_en_idx) & ALGO_EN_BASE)? algo_en_idx: ALGO_DISABLED;
        enables |= ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_DEPENDENT_EN];
    }
    return enables;
}

/**
 * @brief Derives all the enabled algorithms specific parameters related to sensor mode,
 *        algorithm min and max time window and sample rate for each sensor.
 * @param p_message (out) Pointer to the sensor config data structure
 */
static void sh_seq_get_sensor_cfg(sh_sensor_cfg_t *p_sensor_cfg)
{
    uint8 algo_en_idx = 0;
    uint16 sensor_mode = 0;
    uint32 algo_en = 0;

    /** Dependent parameter calculation for IMU sensors */
    for(algo_en_idx = 0; algo_en_idx < NUM_IMU_ALGORITHMS; algo_en_idx++)
    {
        algo_en = ((seq_context.enables >> algo_en_idx) & ALGO_EN_BASE)? algo_en_idx: ALGO_DISABLED;
        p_sensor_cfg->algo_min_ms = MAX(p_sensor_cfg->algo_min_ms, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_MIN_PERIOD]);
        p_sensor_cfg->algo_max_ms = MIN(p_sensor_cfg->algo_max_ms, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_MAX_PERIOD]);
        p_sensor_cfg->imu_odr     = MAX(p_sensor_cfg->imu_odr, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_SAMPLE_RATE]);
        sensor_mode              |= ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_SENSOR_MASK];
    }
    /** Another for loop to avoid 'if' condition within for. Continue the dependent parameter
     * calculation for PPG algorithms */
    for(algo_en_idx = algo_en_idx; algo_en_idx < NUM_IMU_PPG_ALGORITHMS; algo_en_idx++)
    {
        algo_en = ((seq_context.enables >> algo_en_idx) & ALGO_EN_BASE)? algo_en_idx: ALGO_DISABLED;
        p_sensor_cfg->algo_min_ms = MAX(p_sensor_cfg->algo_min_ms, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_MIN_PERIOD]);
        p_sensor_cfg->algo_max_ms = MIN(p_sensor_cfg->algo_max_ms, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_MAX_PERIOD]);
        p_sensor_cfg->ppg_odr     = MAX(p_sensor_cfg->ppg_odr, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_SAMPLE_RATE]);
        sensor_mode              |= ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_SENSOR_MASK];
    }
    /** Another for loop to avoid 'if' condition within for. Continue the dependent parameter
     * calculation for PPG algorithms */
    for(algo_en_idx = algo_en_idx; algo_en_idx < NUM_AVAILABLE_ALGORITHMS; algo_en_idx++)
    {
        algo_en = ((seq_context.enables >> algo_en_idx) & ALGO_EN_BASE)? algo_en_idx: ALGO_DISABLED;
        p_sensor_cfg->algo_min_ms = MAX(p_sensor_cfg->algo_min_ms, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_MIN_PERIOD]);
        p_sensor_cfg->algo_max_ms = MIN(p_sensor_cfg->algo_max_ms, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_MAX_PERIOD]);
        p_sensor_cfg->prox_odr    = MAX(p_sensor_cfg->prox_odr, ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_SAMPLE_RATE]);
        sensor_mode              |= ALGORITHM_DEPENDENCY_TABLE[algo_en][ALGO_SENSOR_MASK];
    }

    p_sensor_cfg->imu_period =  IMU_LP_SAMPLE_INTERVAL_MS_TABLE[p_sensor_cfg->imu_odr];
    p_sensor_cfg->ppg_period =  PPG_SAMPLE_RATE_TABLE[p_sensor_cfg->ppg_odr];
    p_sensor_cfg->prox_period = PPG_SAMPLE_RATE_TABLE[p_sensor_cfg->prox_odr];
    p_sensor_cfg->imu_mode = (sensor_mode & SEQ_IMU_MODE_MASK)?
                            (sensor_mode & SEQ_IMU_MODE_MASK) :
                            IMU_MODE_NONE;
    p_sensor_cfg->ppg_mode = (sensor_mode & SEQ_PPG_MODE_MASK)?
                            ((sensor_mode & SEQ_PPG_MODE_MASK) >> SEQ_PPG_MODE_POS) :
                            PPG_MODE_NONE;
    p_sensor_cfg->prox_mode = (sensor_mode & SEQ_PROX_MODE_MASK)?
                              ((sensor_mode & SEQ_PROX_MODE_MASK) >> SEQ_PROX_MODE_POS) :
                              PROXIMITY_MODE_NONE;

    SportHealthLoggingSensorRequired(p_sensor_cfg);
}
/**
 * @brief Allocate and free context memory for each enabled or disabled algorithm.
 *        Also, calls the initialisation function corresponding to each enabled algorithm
 *
 * @param toggle_enables Bitfield containing information about the algorithms whose
 *                       enable/disable states needs to be toggled. The algorithms
 *                       corresponding to bits set to 1 will have their enable/disable
 *                       state toggled.
 *
 * @return TRUE/FALSE
 */
static bool sh_seq_initialise_algorithms(uint32 toggle_enables)
{
    /* Assume failure until proven otherwise */
    bool success = TRUE;

    if(toggle_enables & ALGO_EN_STEP)
    {
        seq_context.enables ^= ALGO_EN_STEP;
        if (seq_context.step_info == NULL)
        {
            seq_context.step_info = PanicUnlessMalloc(SportHealthAlgorithmsGetStepContextSize());
            /* Allocate memory for heap block used as working buffer along with step. Step is a pre-
               requisite for every other algorithm to run */
            seq_context.algo_heap_block_info = PanicUnlessMalloc(SportHealthAlgorithmsGetHeapBlockSize());
            if((seq_context.step_info != NULL) && (seq_context.algo_heap_block_info != NULL))
            {
                SportHealthAlgorithmsInitHeapBlock(seq_context.algo_heap_block_info);
                SportHealthAlgorithmsInitStep(seq_context.step_info);
                SportHealthLoggingAlgoStatus(ALGO_STEP, ALGO_INIT, 0);
            }
            else
            {
                success = FALSE;
            }
        }
        else
        {
            free(seq_context.step_info);
            seq_context.step_info = NULL;
            free(seq_context.algo_heap_block_info);
            seq_context.algo_heap_block_info = NULL;
            SportHealthLoggingAlgoStatus(ALGO_STEP, ALGO_DELETE, 0);
        }
    }
    if (toggle_enables & (ALGO_EN_HR | ALGO_EN_SPO2))
    {
        uint16 mask = toggle_enables & (ALGO_EN_HR | ALGO_EN_SPO2);
        seq_context.enables ^= mask;
        if(seq_context.heart_rate_info == NULL)
        {
            seq_context.heart_rate_info = PanicUnlessMalloc(SportHealthAlgorithmsGetHeartRateContextSize());
            if(seq_context.heart_rate_info != NULL)
            {
                SportHealthAlgorithmsInitHeartRate(seq_context.heart_rate_info);
                SportHealthLoggingAlgoStatus(ALGO_HR, ALGO_INIT, 0);
            }
            else
            {
                success = FALSE;
            }
        }
        else
        {
            free(seq_context.heart_rate_info);
            seq_context.heart_rate_info = NULL;
            SportHealthLoggingAlgoStatus(ALGO_HR, ALGO_DELETE, 0);
        }
    }
    if (toggle_enables & ALGO_EN_TAP)
    {
        seq_context.enables ^= ALGO_EN_TAP;
        if(seq_context.tap_info == NULL)
        {
            seq_context.tap_info = PanicUnlessMalloc(SportHealthAlgorithmsGetTapContextSize());
            if(seq_context.tap_info != NULL)
            {
                SportHealthAlgorithmsInitTap(seq_context.tap_info);
                SportHealthLoggingAlgoStatus(ALGO_TAP, ALGO_INIT, 0);
            }
            else
            {
                success = FALSE;
            }
        }
        else
        {
            free(seq_context.tap_info);
            seq_context.tap_info = NULL;
            SportHealthLoggingAlgoStatus(ALGO_TAP, ALGO_DELETE, 0);
        }
    }
    if (toggle_enables & ALGO_EN_DISTANCE)
    {
        seq_context.enables ^= ALGO_EN_DISTANCE;
        if(seq_context.distance_info == NULL)
        {
            /* Distance context has an additional block, This framentation is done to ensure
             * optimal use of memory pool blocks
            */
            uint32 additional_distance_context_size = 0;
            uint32 distance_context_size = SportHealthAlgorithmsGetDistanceContextSize(&additional_distance_context_size);
            seq_context.distance_info = PanicUnlessMalloc(distance_context_size);
            seq_context.additional_context_buffer = PanicUnlessMalloc(additional_distance_context_size);
            if((seq_context.distance_info != NULL) && (seq_context.additional_context_buffer != NULL))
            {
                SportHealthAlgorithmsInitDistance(seq_context.distance_info, seq_context.additional_context_buffer);
                SportHealthLoggingAlgoStatus(ALGO_DISTANCE, ALGO_INIT, 0);
            }
            else
            {
                success = FALSE;
            }
        }
        else
        {
            free(seq_context.distance_info);
            seq_context.distance_info = NULL;
            free(seq_context.additional_context_buffer);
            seq_context.additional_context_buffer = NULL;
            SportHealthLoggingAlgoStatus(ALGO_DISTANCE, ALGO_DELETE, 0);
        }
    }
    if (toggle_enables & ALGO_EN_PPG_LOW_POWER)
    {
        seq_context.enables ^= ALGO_EN_PPG_LOW_POWER;
        if(seq_context.ppg_power_mode_info == NULL)
        {
            L0_DBG_MSG1("5: %u", sizeof(sh_vm_ppg_power_save_info_t));
            seq_context.ppg_power_mode_info = PanicUnlessMalloc(sizeof(sh_vm_ppg_power_save_info_t));
            seq_context.ppg_power_mode_info->ppg_mode = SH_PPG_ACTIVE;
            seq_context.ppg_power_mode_info->state_entry_time = 0;
        }
        else
        {
            free(seq_context.ppg_power_mode_info);
            seq_context.ppg_power_mode_info = NULL;
        }
    }
#ifdef ENABLE_ATTITUDE
    if (toggle_enables & ALGO_EN_ATTITUDE)
    {
        seq_context.enables ^= ALGO_EN_ATTITUDE;
        if(seq_context.attitude_info == NULL)
        {
            seq_context.attitude_info = PanicUnlessMalloc(SportHealthAlgorithmsGetAttitudeContextSize());
            if(seq_context.attitude_info != NULL)
            {
                SportHealthAlgorithmsInitAttitude(seq_context.attitude_info);
                SportHealthLoggingAlgoStatus(ALGO_ATTITUDE, ALGO_INIT, 0);
            }
            else
            {
                success = FALSE;
            }
        }
        else
        {
            free(seq_context.attitude_info);
            seq_context.attitude_info = NULL;
            SportHealthLoggingAlgoStatus(ALGO_ATTITUDE, ALGO_DELETE, 0);
        }
    }
#endif
    if (toggle_enables & ALGO_EN_PROXIMITY)
    {
        seq_context.enables ^= ALGO_EN_PROXIMITY;

    }
    if (toggle_enables & ALGO_EN_TAP_RATES)
    {
        seq_context.enables ^= ALGO_EN_TAP_RATES;

    }
    return success;
}
/**
 * @brief Initialise the context and results for
 *        each algorithm
 */
void sport_health_sequencer_init(void)
{
    seq_context.enables = 0;
    /** Initialise the context for each algorithm */
    seq_context.step_info = NULL;
    seq_context.tap_info  = NULL;
    seq_context.distance_info = NULL;
    seq_context.heart_rate_info = NULL;
#ifdef ENABLE_ATTITUDE
    seq_context.attitude_info = NULL;
#endif
    seq_context.ppg_power_mode_info = NULL;
}
/**
 * @brief Process the IMU data received from sensor by passing to
 *        different algorithms. Generates results for all the enabled
 *        algorithm which can then be used by customer application for
 *        further processing
 *
 * @param p_message Pointer to received IMU sensor data
 */
void sh_seq_process_imu_data(imu_sensor_data_t *p_message)
{
    imu_bio_data_t imu_bio_data;

    /* Copy the IMU accel data*/
    imu_bio_data.accel.frame_count      = p_message->accel.frame_count;
    imu_bio_data.accel.last_sample_time = p_message->accel.last_sample_time;
    imu_bio_data.accel.range            = p_message->accel.range;
    imu_bio_data.accel.sampling_interval= p_message->accel.sampling_interval;
    imu_bio_data.accel.p_imu_data       = *(p_message->accel.data.p_imu_data);

    /* Copy the IMU accel data*/
    imu_bio_data.gyro.frame_count      = p_message->gyro.frame_count;
    imu_bio_data.gyro.last_sample_time = p_message->gyro.last_sample_time;
    imu_bio_data.gyro.range            = p_message->gyro.range;
    imu_bio_data.gyro.sampling_interval= p_message->gyro.sampling_interval;
    imu_bio_data.gyro.p_imu_data       = *(p_message->gyro.data.p_imu_data);
	
    /* Option for rate change and interpolation is needed only when TAP is enabled */
    if((seq_context.tap_info != NULL) && (seq_context.enables & ALGO_EN_TAP_RATES))
    {
        /* There might be a need to 'interpolate' between different sensor rates*/
        sh_deal_with_potential_data_rate_changes(&seq_context, &(imu_bio_data.accel));
    }

#ifndef SPORTS_HEALTH_LOGGING_DISABLE
    /* Log input to IMU algorithms */
    p_message->accel.frame_count = imu_bio_data.accel.frame_count;
    SportHealthLoggingImuSensorData(p_message);
#endif
    /*Call the IMU algorithms*/
    if(seq_context.step_info != NULL)
        SportHealthAlgorithmsRunStep(seq_context.step_info, &imu_bio_data.accel);

    if(seq_context.tap_info != NULL)
    {
        SportHealthAlgorithmsRunTap(seq_context.tap_info, seq_context.step_info, &imu_bio_data.accel);
		/*If we are running Tap, update the info that decides whether an IMU rate change is needed*/
        if (seq_context.enables & ALGO_EN_TAP_RATES)
        {
            if (!sh_get_desired_data_rate_has_changed())
            {
                sh_update_imu_rate_change_decision_info(seq_context.step_info, imu_bio_data.accel.last_sample_time);
            }
        }
	}
		
    if(seq_context.distance_info != NULL)
        SportHealthAlgorithmsRunDistance(seq_context.distance_info, seq_context.step_info);

#ifdef ENABLE_ATTITUDE
    if(seq_context.attitude_info != NULL)
        SportHealthAlgorithmsRunAttitude(seq_context.attitude_info, seq_context.step_info, &imu_bio_data.accel, &imu_bio_data.gyro);
#endif

}

/**
 * @brief Process the PPG data received from sensor by passing to
 *        different algorithms.Generates results for all the enabled
 *        algorithm which can then be used by customer application for
 *        further processing
 *
 * @param p_message Pointer to received PPG sensor data
 */
void sh_seq_process_ppg_data(ppg_sensor_data_t *p_message)
{
    ppg_bio_data_t ppg_bio_data;

#ifndef SPORTS_HEALTH_LOGGING_DISABLE
    /* Log input to HR algorithms */
    SportHealthLoggingPpgSensorData(p_message);
#endif

    /** Copy the red LED data*/
    ppg_bio_data.red.frame_count      = p_message->red.frame_count;
    ppg_bio_data.red.last_sample_time = p_message->red.last_sample_time;
    ppg_bio_data.red.range            = p_message->red.range;
    ppg_bio_data.red.sampling_interval= p_message->red.sampling_interval;
    ppg_bio_data.red.p_ppg_data       = *(p_message->red.data.p_ppg_data);

    /** Copy the IR LED data*/
    ppg_bio_data.ir.frame_count      = p_message->IR.frame_count;
    ppg_bio_data.ir.last_sample_time = p_message->IR.last_sample_time;
    ppg_bio_data.ir.range            = p_message->IR.range;
    ppg_bio_data.ir.sampling_interval= p_message->IR.sampling_interval;
    ppg_bio_data.ir.p_ppg_data       = *(p_message->IR.data.p_ppg_data);

    /** Call the PPG algorithms*/
    if(seq_context.heart_rate_info != NULL)
        SportHealthAlgorithmsRunHeartRate(seq_context.heart_rate_info, seq_context.step_info, &ppg_bio_data.ir);
}
/**
 * @brief Based on the algorithm enable bitfield received from customer
 *        app this function will enabled all the dependent algorithms and
 *        allocates or free context and result memory for each enabled or
 *        disabled algorithm. It also derives the dependent algorithm parameters.
 *
 * @param p_message Contains the algorithm enable bitfield
 * @param p_sensor_cfg (out) Pointer to store the sensor configurations
 * @return Returns different sensor configuration like max and min rates for each
 *         sensor based on input from each algorithm
 */
bool sh_seq_algo_enable(uint32 enables, sh_sensor_cfg_t *p_sensor_cfg)
{
    /* Assume failure until proven otherwise */
    bool success = FALSE;
    uint32 algo_enables = 0;
    uint32 diff_algo_enables = 0;

    SportHealthLoggingAlgoStatus(ALGO_NONE, ALGO_CONFIG, enables);
    sh_seq_sensor_cfg_init(p_sensor_cfg);

    seq_context.ppg_power_mode_info->ppg_mode = ((enables & ALGO_EN_PPG_LOW_POWER) != 0);

    algo_enables = sh_seq_enable_dependent_algorithms(enables);
    /** Calculate the difference between the current and next enables */
    diff_algo_enables = seq_context.enables ^ algo_enables;
    if(sh_seq_initialise_algorithms(diff_algo_enables))
    {
        sh_seq_get_sensor_cfg(p_sensor_cfg);
        success = TRUE;
    }
    return success;
}
/**
 * @brief Stores the pointer to algorithm information memory which can then
 *        be used by customer app
 * @param p_algo_info (out) Pointer to algorithm information memory owned by sequencer.
 */
void sh_seq_get_algo_info(sh_vm_algo_info_t *p_algo_info)
{
    p_algo_info->step_info = seq_context.step_info;
    p_algo_info->tap_info   = seq_context.tap_info;
    p_algo_info->distance_info = seq_context.distance_info;
    p_algo_info->attitude_info = seq_context.attitude_info;
    p_algo_info->heart_rate_info = seq_context.heart_rate_info;
}

/**
 * @brief To get the heart rate signal quality to send as feedback
 *        to PPG sensor
 * @return signal quality (Good/Ok/Bad/Very Bad)
 */
ppg_hr_feedback_t sh_seq_get_ppg_feedback(void)
{
    algo_feedback_t algo_feedback;
    algo_feedback.hr_feedback = SportHealthAlgorithmsGetHeartRateSignalQuality(seq_context.heart_rate_info);

#ifndef SPORTS_HEALTH_LOGGING_DISABLE
    SportHealthLoggingAlgoFeedback(PPG_SENSOR, &algo_feedback);
#endif

    return algo_feedback.hr_feedback;
}

/**
 * @brief Update the PPG duty cycle mode and return true if it changes
 *
 * @return mode was flipped by update (1=flipped, 0=same)
 */
bool sh_seq_ppg_duty_cycle_update(imu_sensor_data_t *p_message)
{
    bool flipped = FALSE;
    if(seq_context.enables & ALGO_EN_PPG_LOW_POWER)
    {
        bool orig_mode = seq_context.ppg_power_mode_info->ppg_mode;
        update_ppg_duty_cycle(&seq_context, p_message->accel.last_sample_time);
        flipped = (orig_mode != seq_context.ppg_power_mode_info->ppg_mode);
    }
    return flipped;
}
