/*******************************************************************************
Copyright (c) 2017 Qualcomm Technologies International, Ltd.


FILE NAME
    sport_health_algorithms.h

DESCRIPTION
    Includes all the algorithm interface files and algorithm information data
    structures
*/

#ifndef SPORT_HEALTH_ALGORITHMS_H
#define SPORT_HEALTH_ALGORITHMS_H

#include <types.h>

/**
 * @brief error code enum
 */
typedef enum
{
    SH_SUCCESS = 0,
    SH_FAILURE = 1
} sh_err_code_t;
/**
 * @brief Struct used to store the data from each
 *        sensor in IMU
 */
typedef struct{
    uint32 last_sample_time;  /**< Last sample time in ms */
    uint16 frame_count;       /**< Number of samples from Accel */
    int16 *p_imu_data;        /**< Pointer to data from IMU sensor */
    uint16  range;             /**< Range of Accel/Gyro */
    uint8  sampling_interval; /**< Sampling interval of Accel */
} imu_input_data_t;
/**
 * @brief Struct used to store the data from each
 *        LED in PPG
 */
typedef struct{
    uint32 last_sample_time;  /**< Last sample time */
    uint16 frame_count;       /**< Number of samples from PPG */
    uint16 *p_ppg_data;       /**< Pointer to data from PPG sensor */
    uint16  range;            /**< Range of PPG data */
    uint8  sampling_interval; /**< Sampling interval of PPG */
} ppg_input_data_t;
/**
 * @brief Struct used to store the data from IMU
 *        for all the active sensors
 */
typedef struct{
    imu_input_data_t accel;
    imu_input_data_t gyro;
} imu_bio_data_t;

/**
 * @brief Struct used to store the data from PPG
 *        for all the active LEDs
 */
typedef struct{
    ppg_input_data_t red;
    ppg_input_data_t ir;
} ppg_bio_data_t;
/**
* @brief Step context type
*/
typedef struct step_context sh_vm_step_info_public_t;
/**
* @brief: type used for setting the parameter update type
          SH_STEP_PARAM_FOOT_MOUNTED, boolean
      SH_STEP_PARAM_FREQ_FILTER, one pole iir filter weighting : consecutive steps are down weighted by this (Q8)
      SH_STEP_PARAM_SLOW_WALKING_BELOW, below this limit it is slow walking (Hz, Q8)
      SH_STEP_PARAM_MIN_PEAK_RATIO, peaks must exceed this value to be considered contenders (Q8)
*/
typedef enum {
    SH_STEP_PARAM_FOOT_MOUNTED,
    SH_STEP_PARAM_FREQ_FILTER,
    SH_STEP_PARAM_SLOW_WALKING_BELOW,
    SH_STEP_PARAM_MIN_PEAK_RATIO
} sh_vm_step_param_t;
/**
  * @brief type used for describing the stepping description
  */
typedef enum {
    SH_STEP_TYPE_UNKNOWN = 0,
    SH_STEP_TYPE_WALKING = 1,
    SH_STEP_TYPE_RUNNING = 2
} sh_step_type_t;
/**
  * @brief type used for describing the stepping state
  */
typedef enum {
    SH_STEP_STATUS_UNKNOWN = 0,
    SH_STEP_STATUS_STATIONARY,
    SH_STEP_STATUS_NOT_STEPPING,
    SH_STEP_STATUS_STEPPING
} sh_step_status_t;
/**
  * @brief type used for describing a single step
  * @param timestamp
  * @param frequency in Hz scaled by 2^8
  * @param type
  */
typedef struct {
    uint32         timestamp;
    uint16         frequency_Hz_Q8;
    sh_step_type_t type;
} sh_vm_step_t;
/* Quaternian definition */
#define QUAT_SIZE 4
typedef int32 quat32_t[QUAT_SIZE];
/**
* @brief Stores the context of attitude algorithm
*/
typedef struct attitude_context sh_vm_attitude_info_public_t;
/**
* @brief Attitude parameters
*
*   SH_ATT_PARAM_UPDATE_GYRO_COV_TIME_STEP,    Time in ms after which the covariance update takes place.
*                                              The larger value, the faster and less accurate the algorithm. Default: 30
*    SH_ATT_PARAM_MAX_GYRO_BIAS,               Gyro bias in deg/s
*    SH_ATT_PARAM_DUR_STABLE_ACC_FOR_UPDATE,   Time in ms after which the accelerometer update takes place. Default: 500
*    SH_ATT_PARAM_COMPUTE_CONFIDENCE,          Is the confidence needed; true/false? Default: false.
*    SH_ATT_PARAM_REHOME                       Align the yaw origin with the device. Input param irrelevant.
*/
typedef enum {
    SH_ATT_PARAM_UPDATE_GYRO_COV_TIME_STEP,
    SH_ATT_PARAM_MAX_GYRO_BIAS,
    SH_ATT_PARAM_DUR_STABLE_ACC_FOR_UPDATE,
    SH_ATT_PARAM_COMPUTE_CONFIDENCE,
    SH_ATT_PARAM_REHOME
} sh_vm_attitude_param_t;
/**
*	@brief  Distance context type
*/
typedef struct distance_context sh_vm_distance_info_public_t;
/**
  * @brief: type used for the user information
  */
typedef struct
{
    uint16     id;
    uint16     height_cm;
} sh_vm_user_info_t;
/**
  * @brief: type used for describing the GNSS position confidence
  */
typedef struct
{
    uint32 major_cm;
    uint32 minor_cm;
    uint32 ang_degs;
    uint32 alt_cm;
} sh_vm_gnss_position_conf_t;
/**
  * @brief: type used for the GNSS position information
  */
typedef struct
{
    uint32 timestamp_ms;
    int32  lat;
    int32  lon;
    int32  alt_cm;
    sh_vm_gnss_position_conf_t conf;
} sh_vm_gnss_position_t;
/**
  * @brief: type used for describing the odometer information
  */
typedef struct
{
    uint32   timestamp_ms;
    uint32   d_cm;
} sh_vm_odometer_t;
/**
  * @brief: type used for describing the calibration infomation
  */
typedef struct
{
    uint32         d_cm;
    uint32         dt_ms;
    uint16         id;
    uint16         step_count;
    sh_step_type_t stepType;
} sh_vm_calibration_info_t;
/**
  * @brief: type used for describing the fit quality
  */
typedef enum {
    SH_DISTANCE_FIT_UNKNOWN = 0,
    SH_DISTANCE_FIT_GOOD = 1,
    SH_DISTANCE_FIT_OK = 2,
    SH_DISTANCE_FIT_POOR = 3
} sh_vm_fit_t;
/**
  * @brief: type used for describing the learning benefit quality
  */
typedef enum {
    SH_DISTANCE_LEARNING_BENEFIT_UNKNOWN = 0,
    SH_DISTANCE_LEARNING_BENEFIT_GOOD = 1,
    SH_DISTANCE_LEARNING_BENEFIT_OK = 2,
    SH_DISTANCE_LEARNING_BENEFIT_POOR = 3
} sh_vm_distance_learning_t;
/**
  * @brief: type used for describing the model
  */
typedef enum {
    SH_DISTANCE_MODEL_UNKNOWN = 0,
    SH_DISTANCE_MODEL_DEFAULT = 1,
    SH_DISTANCE_MODEL_USER_INFO = 2,
    SH_DISTANCE_MODEL_LEARNT = 3,
    SH_DISTANCE_MODEL_CALIBRATED = 4
} sh_vm_distance_model_t;
/**
  * @brief: type used for setting the parameter update type
  */
typedef enum {
    SH_DISTANCE_UPDATE_USER_INFO = 1,
    SH_DISTANCE_UPDATE_POSITION = 2,
    SH_DISTANCE_UPDATE_ODO = 3,
    SH_DISTANCE_UPDATE_CALIBRATION = 4,
    SH_DISTANCE_PARAM_GOOD_GNSS_CONF_MS,
    SH_DISTANCE_PARAM_MIN_LEARNING_STEPS,
    SH_DISTANCE_PARAM_MAX_INTERVAL_MS
} sh_vm_distance_update_t;
/**
*	@brief  Tap context type
*/
typedef struct tap_context sh_vm_tap_info_public_t;
/**
*	@brief  enum for types of tap detected
*/
typedef enum
{
    SH_TAP_TYPE_UNKNOWN = 0,
    SH_TAP_TYPE_SINGLE_TAP = 1,
    SH_TAP_TYPE_DOUBLE_TAP = 2
} sh_tap_type_t;
/**
 @brief: enum used by SportHealthAlgorithmsSetTapParam function to indicate
     which parameter is being passed in.  Note
     all parameters have defaults

    SH_TAP_PARAM_MIN_DOUBLE_TAP_DURATION_MS,  the min spacing for 2 taps to be considered a double tap (default 150ms)
    SH_TAP_PARAM_MAX_DOUBLE_TAP_DURATION_MS,  the max spacing for 2 taps to be considered a double tap (default 500ms)
    SH_TAP_PARAM_MIN_TAP_SEPARATION_MS,  the min allowed between 2 taps (or either type) (default 1000ms)

    Threshold modifiers for different step types (Q8): default for each is 256 which is 1 Q8
    The order for each is: unknown/stationary/notstepping/stepping/running.
    Each of these four flags requires an array of 5 modifiers.
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_LO_RATE_Q8,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_HI_RATE_Q8,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_LO_RATE_SECOND_DERIV_Q8,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_HI_RATE_SECOND_DERIV_Q8,

    SH_TAP_PARAM_DOWN_VEC_Q8, modify the tap axis weights (it is a 3d vector of magnitude 256)
*/
typedef enum {
    SH_TAP_PARAM_MIN_DOUBLE_TAP_DURATION_MS,
    SH_TAP_PARAM_MAX_DOUBLE_TAP_DURATION_MS,
    SH_TAP_PARAM_MIN_TAP_SEPARATION_MS,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_LO_RATE_Q8,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_HI_RATE_Q8,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_LO_RATE_SECOND_DERIV_Q8,
    SH_TAP_PARAM_STEP_TYPE_THRESHOLD_MODIFIER_HI_RATE_SECOND_DERIV_Q8,
    SH_TAP_PARAM_DOWN_VEC_Q8,
	SH_TAP_PARAM_N_SAMPLES_INSERTED
} sh_vm_tap_param_t;
/**
*	@brief  This opaque wrapper structure stores heart_rate_context
*           i.e. a pointer to the information for heart rate (PPG) functionality.
*           It is passed by value into most of the functions.
*/
typedef struct hr_context sh_vm_heart_rate_info_public_t;
/**
* @brief enum used for feeding back the signal quality to the ppg driver
*/
typedef enum {
    SH_HR_SIGNAL_QUAL_UNKNOWN = 0,
    SH_HR_SIGNAL_QUAL_GOOD = 1,
    SH_HR_SIGNAL_QUAL_OK = 2,
    SH_HR_SIGNAL_QUAL_POOR = 3
} sh_hr_feedback_t;
/**
  * @brief enum used for describing the heart rate quality
  */
typedef enum {
    SH_HR_QUALITY_UNKNOWN = 0,
    SH_HR_QUALITY_GOOD = 1,
    SH_HR_QUALITY_OK = 2,
    SH_HR_QUALITY_POOR = 3
} sh_heart_rate_quality_t;
/**
 @brief: enum used by SportHealthAlgorithmsSetHeartRateParam function to indicate
  * @brief type used for describing a heart rate estimate
  * @param t
  * @param rate_bpm beats per minute
  * @param quality indicators, see below
  */
typedef struct {
    uint32 t;
    uint16 rate_bpm;
    sh_heart_rate_quality_t quality;
    sh_hr_feedback_t signal_quality;
} sh_vm_heart_rate_info_t;
/**
 @brief: enum used by set_heart_rate_param function to indicate
     which parameter is being passed in.  Note
     all parameters have defaults

    SH_HR_PARAM_DETREND_INTERVAL_MS,  de-trending interval in ms (default is 2000)
    SH_HR_PARAM_DETREND_STEP_MS,  de-trending step in ms (default is 40)
*/
typedef enum {
    SH_HR_PARAM_DETREND_INTERVAL_MS,
    SH_HR_PARAM_DETREND_STEP_MS
} sh_vm_heart_rate_param_t;
/**
 * @brief PPG power mode enum
 */
typedef enum
{
    SH_PPG_INACTIVE = 0,
    SH_PPG_ACTIVE = 1
} sh_vm_ppg_power_save_mode_t;

/**
 * @brief PPG power mode enum
 */
typedef struct
{
    uint32 state_entry_time;
    sh_vm_ppg_power_save_mode_t ppg_mode;
} sh_vm_ppg_power_save_info_t;

/**
* @brief Algorithm memory pool type
*/
typedef struct heapBlock_t sh_vm_algo_heap_block_info_t;

/** Stores all the information related to each algorithm */
typedef struct
{
    uint32                           enables;     /**< Bitfield pointing to enabled algorithms including dependency */
    sh_vm_step_info_public_t *       step_info;
    sh_vm_tap_info_public_t *        tap_info;
    sh_vm_distance_info_public_t *   distance_info;
    sh_vm_attitude_info_public_t *   attitude_info;
    sh_vm_heart_rate_info_public_t * heart_rate_info;
    sh_vm_ppg_power_save_info_t *    ppg_power_mode_info;
    sh_vm_algo_heap_block_info_t  *  algo_heap_block_info;
    void *                           additional_context_buffer;
} sh_vm_algo_info_t;
/**
 * @brief Wrapper to return the size of step_context_t
 * @return the context size in bytes
 */
uint32 SportHealthAlgorithmsGetStepContextSize(void);
/**
 * @brief Initialise the state of the algorithms
 * @param sh_vm_step_info_public_t step_info [in,out]
 */
void SportHealthAlgorithmsInitStep(sh_vm_step_info_public_t * step_info);
/**
* @brief Reset the step count to zero (for example at midnight)
*        without losing any other conext information
* @param sh_vm_step_info_public_t step_info [in,out]
*/
void SportHealthAlgorithmsResetStepCount(sh_vm_step_info_public_t * step_info);
/**
 * @brief Wrapper to call the step algorithm
 * @param sh_vm_step_info_public_t step_info [in,out]
 * @param imu_input_data_t p_imu_input_data [in]
 */
void SportHealthAlgorithmsRunStep(sh_vm_step_info_public_t * step_info, const imu_input_data_t *p_imu_input_data);
/**
 * @brief Get step last processed time
 * @param sh_vm_step_info_public_t step_info [in]
 * @return timestamp in ms (or MAX_UINT32 if invalid)
 */
uint32 SportHealthAlgorithmsGetStepTime(const sh_vm_step_info_public_t * step_info);
/**
 * @brief Get number of steps
 * @param sh_vm_step_info_public_t step_info [in]
 * @return number of steps (or MAX_UINT16 if invalid)
 */
uint16 SportHealthAlgorithmsGetStepNumber(const sh_vm_step_info_public_t * step_info);
/**
 * @brief Get step error rate per second, Q8 (scaled by 2^8)
 * @param sh_vm_step_info_public_t step_info [in]
 * @return time in ms (or MAX_UINT16 if invalid)
 */
uint16 SportHealthAlgorithmsGetStepErrorRate(const sh_vm_step_info_public_t * step_info);
/**
 * @brief Get status (enum sh_step_status_t: unknown/stationary/not stepping/stepping 0/1/2/3)
 * @param sh_vm_step_info_public_t step_info [in]
 * @return sh_step_status_t (enum)
 */
sh_step_status_t SportHealthAlgorithmsGetStepStatus(const sh_vm_step_info_public_t * step_info);
/**
 * @brief Get step history size
 * @param sh_vm_step_info_public_t step_info [in]
 * @return number of step objects (sh_vm_step_t) kept in the history buffer
 */
uint16 SportHealthAlgorithmsGetStepHistorySize(const sh_vm_step_info_public_t * step_info);
/**
 * @brief Get a step at index
 * @param p_step, the step at index idx [out]
 * @param idx, from 0 to SportHealthAlgorithmsGetStepHistorySize [in]
 * @param sh_vm_step_info_public_t step_info [in]
 * @return sh_err_code_t  (enum:  SH_SUCCESS/SH_FAILURE 0/1)
 */
sh_err_code_t SportHealthAlgorithmsGetStep(sh_vm_step_t * p_step, const uint32 idx, const sh_vm_step_info_public_t * step_info);
/**
* @brief Pass the parameters to the step algorithm
* @param sh_vm_step_info_public_t step_info [in,out]
* @param sh_vm_step_param_t update_type [in]
* @param void * p_input_data [in]
* @return sh_err_code_t  (enum:  SH_SUCCESS/SH_FAILURE 0/1)
*/
sh_err_code_t SportHealthAlgorithmsSetStepParam(sh_vm_step_info_public_t * step_info, sh_vm_step_param_t update_type, void * p_input_data);
/**
 * @brief Wrapper to return the size of attitude_context_t
 * @return the context size in bytes
 */
uint32 SportHealthAlgorithmsGetAttitudeContextSize(void);
/**
 * @brief Initialise the state of the algorithm
 * @param sh_vm_attitude_info_public_t attitude_info [in,out]
 */
void SportHealthAlgorithmsInitAttitude(sh_vm_attitude_info_public_t *attitude_info);
/**
 * @brief Wrapper to call the attitude algorithm
 * @param sh_vm_attitude_info_public_t attitude_info [in,out]
 * @param const sh_vm_step_info_public_t step_info [in]
 * @param imu_input_data_t *p_acc_data [in]
 * @param imu_input_data_t *p_gyro_data [in]
 */
void SportHealthAlgorithmsRunAttitude(sh_vm_attitude_info_public_t *attitude_info, const sh_vm_step_info_public_t * step_info, imu_input_data_t *p_acc_data, imu_input_data_t *p_gyro_data);
/**
 * @brief get the timestamp (ms) at which the attitude is valid
 * @param const sh_vm_attitude_info_public_t attitude_info [in]
 * @return timestamp (ms) (or MAX_UINT32 if invalid)
 */
uint32 SportHealthAlgorithmsGetAttitudeTime(const sh_vm_attitude_info_public_t * attitude_info);
/**
* @brief get the pitch of the device in degrees range: -90 -> 90 degrees
     For an x, y, z right hand set reference frame, +ve pitch is
     rotation about the y axis such that the x axis would point downwards
* @param const sh_vm_attitude_info_public_t attitude_info [in]
* @return pitch (degrees) (or MAX_INT16 if invalid)
*/
int16 SportHealthAlgorithmsGetAttitudePitch(const sh_vm_attitude_info_public_t *attitude_info);
/**
* @brief get the roll of the device in degrees range -180 -> 180 degrees
     For an x, y, z right hand set reference frame +ve roll is
     rotation about the x axis such that the y axis would point up
* @param const sh_vm_attitude_info_public_t attitude_info [in]
* @return roll degrees (or MAX_INT16 if invalid)
*/
int16 SportHealthAlgorithmsGetAttitudeRoll(const sh_vm_attitude_info_public_t *attitude_info);
/**
* @brief get confidence associated with pitch and/or roll in degrees
* @param const sh_vm_attitude_info_public_t attitude_info [in]
* @return confidence (degrees) (or MAX_INT32 if invalid)
*/
uint16 SportHealthAlgorithmsGetAttitudeTiltConfidence(const sh_vm_attitude_info_public_t *attitude_info);
/**
* @brief get quaternion scaled by 2^30
*        Quaternions are a compact, unambiguous and numerically convenient
*        way of representing orientation.  Orientations are represented as a
*        rotation of fixed reference set of axis about a unit vector(u) by
*        and angle (theta)
*        The quaternion convention used is:
*        q = [sin(theta/2)*ux  sin(theta/2)*uy sin(theta/2)*uz cos(theta/2)]*2^30
*        Where [ux uy yz] is a 3D unit vector
*        theta is an angle in radians
  @param quat32_t quaternion [in/out]
* @param const sh_vm_attitude_info_public_t attitude_info [in]
* @return sh_err_code_t [SH_SUCCESS/SH_FAILURE] [0/1]
*/
sh_err_code_t SportHealthAlgorithmsGetAttitudeQuaternion(quat32_t quaternion, const  sh_vm_attitude_info_public_t *attitude_info);
/**
* @brief get confidence of the orientation vector in degrees
     RMS error between the result and the true orientation vector
* @param const sh_vm_attitude_info_public_t attitude_info [in]
* @return confidence (degrees) (or MAX_INT32 if invalid)
*/
uint16 SportHealthAlgorithmsGetAttitudeConfidence(const sh_vm_attitude_info_public_t *attitude_info);
/**
* @brief Pass the parameters to the attitude algorithm
* @param sh_vm_attitude_info_public_t attitude_info [in,out]
* @param sh_vm_attitude_param_t update_type [in]
* @param void * p_input_data [in]
* @return sh_err_code_t  (enum:  SH_SUCCESS/SH_FAILURE 0/1)
*/
sh_err_code_t SportHealthAlgorithmsSetAttitudeParam(sh_vm_attitude_info_public_t * attitude_info, const sh_vm_attitude_param_t update_type, void * p_input_data);
/**
 * @brief Wrapper to return the size of distance_context_t
 * @param additional_chunk_size [in,out]
 * @return the context size in bytes
 */
uint32 SportHealthAlgorithmsGetDistanceContextSize(uint32 * additional_chunk_size);
/**
 * @brief Initialise the state of the algorithm
 * @param additional_chunk [in,out]
 * @param distance_info [in,out]
 */
void SportHealthAlgorithmsInitDistance(sh_vm_distance_info_public_t *distance_info, void * additional_chunk);
/**
* @brief Reset the total distance travelled for the algorithm
*        without resetting any user information (e.g. to start counting from midnight)
* @param distance_info [in,out]
*/
void SportHealthAlgorithmsResetDistance(sh_vm_distance_info_public_t *distance_info);
/**
 * @brief Pass the user info or GNSS, or odometer or calibration info to the distance algorithm
 * @param distance_info [in,out]
 * @param update_type [in]
 * @param p_input_data [in]
 */
void SportHealthAlgorithmsSetDistanceParam(sh_vm_distance_info_public_t *distance_info, sh_vm_distance_update_t update_type, void * p_input_data, const sh_vm_step_info_public_t * step_info);
/**
 * @brief Run the distance algorithm
 * @param distance_info [in,out]
 * @param step_info [in]
 */
void SportHealthAlgorithmsRunDistance(sh_vm_distance_info_public_t *distance_info, const sh_vm_step_info_public_t * step_info);
/**
* @brief Get user id
* @param distance_info [in]
* @return uint32 user id
*/
uint32 SportHealthAlgorithmsGetDistanceUserId(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get distance travelled in cm
* @param distance_info [in]
* @return uint32 distance in cm
*/
uint32 SportHealthAlgorithmsGetDistance(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief get estimated confidence on the distance, in cm
* @param distance_info [in]
* @return uint32 distance confidence in cm
*/
uint32 SportHealthAlgorithmsGetDistanceConfidence(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get speed in cm per sec
* @param distance_info [in]
* @param uint32 speed in cm per sec
*/
uint32 SportHealthAlgorithmsGetDistanceSpeed(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get speed confidence in cm per sec
* @param distance_info [in]
* @return uint32 speed confidence in cm per sec
*/
uint32 SportHealthAlgorithmsGetDistanceSpeedConfidence(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get estimated length of 1 step from the distance info, in cm
* @param distance_info [in]
* @return uint32 stride length in cm
*/
uint32 SportHealthAlgorithmsGetDistanceStrideLength(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get current time from distance info, in ms
* @param distance_info [in]
* @return uint32 time in ms
*/
uint32 SportHealthAlgorithmsGetDistanceTime(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get model type used for distance
* @param distance_info [in]
* @return sh_vm_distance_model_t
*/
sh_vm_distance_model_t SportHealthAlgorithmsGetDistanceModelType(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get walking model quality
* @param distance_info [in]
* @return sh_vm_fit_t
*/
sh_vm_fit_t SportHealthAlgorithmsGetDistanceWalkingModelQuality(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get running model quality
* @param distance_info [in]
* @return sh_vm_fit_t
*/
sh_vm_fit_t SportHealthAlgorithmsGetDistanceRunningModelQuality(const sh_vm_distance_info_public_t *distance_info);
/**
* @brief Get learning benefit
* @param distance_info [in]
* @return sh_vm_distance_learning_t
*/
sh_vm_distance_learning_t SportHealthAlgorithmsGetDistanceLearningBenefit(const sh_vm_distance_info_public_t *distance_info);
/**
 * @brief Wrapper to return the size of tap_context_t
 * @return the context size in bytes
 */
uint32 SportHealthAlgorithmsGetTapContextSize(void);
/**
 * @brief Initialise the state of the tap algorithm
 * @param sh_vm_tap_info_public_t tap_info [in,out]
 */
void SportHealthAlgorithmsInitTap(sh_vm_tap_info_public_t *tap_info);
/**
 * @brief Run the tap algorithm
 *        The tap conxtext will be propagated using the
 *        accelerometer data in p_imu_input_data.
 *        The step_info is used to adjust thresholds
 *        when the user is in motion to reduce false alarms
 *
 * @param sh_vm_tap_info_public_t tap_info [in,out]
 * @param sh_vm_step_info_public_t step_info [in]
 * @param imu_input_data_t * p_imu_input_data [in]
 */
void SportHealthAlgorithmsRunTap(sh_vm_tap_info_public_t *tap_info, const sh_vm_step_info_public_t * step_info, const imu_input_data_t * p_imu_input_data);
/**
* @brief Get tap type (enum sh_tap_type_t: unknown/single/double 0/1/2)
* @param sh_vm_tap_info_public_t tap_info [in]
* @return sh_tap_type_t
*/
sh_tap_type_t SportHealthAlgorithmsGetTapType(const sh_vm_tap_info_public_t *tap_info);
/**
 * @brief Get the time in ms. If no tap has occurred it will be set to UNSET_TIMESTAMP 0xFFFFFFFF
 * @param sh_vm_tap_info_public_t tap_info [in]
 * @return uint32 timestamp in ms
 */
uint32 SportHealthAlgorithmsGetTapTime(const sh_vm_tap_info_public_t *tap_info);
/**
 * @brief Get tap amplitude.   If no tap has occurred it will be set to 0xFFFFFFFF
 *        Amplitude is a relative measure of a taps strength and will depend on the form factor.
 *        It should only be used for comparing between taps.
 * @param sh_vm_tap_info_public_t tap_info [in]
 * @return uint32 amplitude
 */
uint32 SportHealthAlgorithmsGetTapAmplitude(const sh_vm_tap_info_public_t *tap_info);
/**
 * @brief set tap parameters
 * @param sh_vm_tap_info_public_t tap_info [in/out]
 * @param sh_vm_tap_param_t update_type [in]
 * @param void * p_input_data [in]
 * @return sh_err_code_t  (enum:  SH_SUCCES/SH_FAILURE 0/1)
 */
sh_err_code_t SportHealthAlgorithmsSetTapParam(sh_vm_tap_info_public_t *tap_info, sh_vm_tap_param_t update_type, const void * p_input_data);
/**
 * @brief Wrapper to return the size of heart_rate_context_t
 * @return uint32 the context size in bytes
 */
uint32 SportHealthAlgorithmsGetHeartRateContextSize(void);
/**
 * @brief Initialise the state of the algorithm
 * @param sh_vm_heart_rate_info_public_t hr_info [in,out]
 */
void SportHealthAlgorithmsInitHeartRate(sh_vm_heart_rate_info_public_t *hr_info);
/**
 * @brief Wrapper to call the heart rate algorithm with ppg data and step context information
 * @param sh_vm_heart_rate_info_public_t *hr_info [in,out]
 */
void SportHealthAlgorithmsRunHeartRate(sh_vm_heart_rate_info_public_t *hr_info, const sh_vm_step_info_public_t * step_info, const ppg_input_data_t *p_ppg_input_data);
/**
* @brief Get time (ms) at which the heart rate is applicable
* @param sh_vm_heart_rate_info_public_t hr_info [in]
* @return uint32 timestamp in ms (or MAX_UINT32 if invalid)
*/
uint32 SportHealthAlgorithmsGetHeartRateTime(const sh_vm_heart_rate_info_public_t *hr_info);
/**
* @brief get heart rate in beats per minute (bpm)
* @param sh_vm_heart_rate_info_public_t hr_info [in]
* @return uint16 heart rate in beats/minute (or MAX_UINT16 if invalid)
*/
uint16 SportHealthAlgorithmsGetHeartRate(const sh_vm_heart_rate_info_public_t *hr_info);
/**
* @brief get heart rate esimate quality (enum sh_heart_rate_quality_t: unknown/good/ok/poor 0/1/2/3)
* @param sh_vm_heart_rate_info_public_t hr_info [in]
* @return sh_heart_rate_quality_t
*/
sh_heart_rate_quality_t SportHealthAlgorithmsGetHeartRateQuality(const sh_vm_heart_rate_info_public_t *hr_info);
/**
* @brief estimate quality of the heart rate signal for feedback to the driver (enum sh_heart_rate_quality_t: good/ok/bad/vary bad 0/1/2/3)
* @param sh_vm_heart_rate_info_public_t hr_info [in]
* @return sh_hr_feedback_t (check if get_heart_rate returns a valid value)
*/
sh_hr_feedback_t SportHealthAlgorithmsGetHeartRateSignalQuality(const sh_vm_heart_rate_info_public_t *hr_info);
/**
 * @brief Get heart rate history size
 * @param sh_vm_heart_rate_info_public_t heart_rate_info [in]
 * @return number of heart rate objects (hr) kept in the history buffer
 */
uint16 SportHealthAlgorithmsGetHeartRateHistorySize(const sh_vm_heart_rate_info_public_t * heart_rate_info);
/**
 * @brief Get a heart rate at index
 * @param p_heart_rate, the heart rate at index idx [out]
 * @param idx, from 0 to get_heart_rate_history_size [in]
 * @param sh_vm_heart_rate_info_public_t step_info [in]
 * @return sh_err_code  (enum:  SH_SUCCESS/SH_FAILURE 0/1)
 */
sh_err_code_t SportHealthAlgorithmsGetHeartRateAtIdx(sh_vm_heart_rate_info_t * p_heart_rate, const uint32 idx, const sh_vm_heart_rate_info_public_t * heart_rate_info);
/**
 * @brief set heart rate parameters
 * @param sh_vm_heart_rate_info_public_t heart_rate_info [in/out]
 * @param sh_vm_heart_rate_param_t update_type [in]
 * @param void * p_input_data [in]
 * @return sh_err_code_t  (enum:  SH_SUCCES/SH_FAILURE 0/1)
 */
sh_err_code_t SportHealthAlgorithmsSetHeartRateParam(sh_vm_heart_rate_info_public_t * heart_rate_info, sh_vm_heart_rate_param_t update_type, const void * p_input_data);
/**
 * @brief Wrapper to return the size of heap block (memory pool)
 *        to be used by algorithm's working buffer
 * @return the heap block size in bytes
 */
uint32 SportHealthAlgorithmsGetHeapBlockSize(void);
/**
 * @brief Initialise the heap block of the algorithms
 * @param sh_vm_algo_heap_block_info_t sh_vm_algo_heap_block_info_t
 */
void SportHealthAlgorithmsInitHeapBlock(sh_vm_algo_heap_block_info_t * sh_vm_algo_heap_block_info_t);
#endif // SPORT_HEALTH_ALGORITHMS_H
