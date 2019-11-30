/**
 * Copyright (c) 2016 - 2017 Qualcomm Technologies International, Ltd.
 *
 * \file  timed_playback.h
 *
 * \ingroup ttp
 *
 * Public header file for the timed playback module.
 */

#ifndef TIMED_PLAYBACK_H
#define TIMED_PLAYBACK_H

/*****************************************************************************
Include Files
*/
#include "types.h"
#include "cbops_mgr/cbops_mgr.h"
#include "buffer/cbuffer_c.h"

/****************************************************************************
Public Constant declarations
*/
/** Timestamps will be reframed to the same length. Each covering the
 * amount of samples given by this constant. */
#define TIMED_PLAYBACK_REFRAME_PERIOD      (512)

/****************************************************************************
Public Type Declarations
*/
typedef struct TIMED_PLAYBACK_STRUCT TIMED_PLAYBACK;

/* unachievable latency callback function / data for unsolicited message */
typedef void (*unachievable_latency_callback)(unsigned con_id, unsigned ep_id, unsigned data);

typedef struct
{
    unachievable_latency_callback callback;      /**< callback function */
    unsigned con_id;                             /**< connection ID */
    unsigned ep_id;                              /**< endpoint ID */
} ttp_unachv_lat_callback_struct;

/**
 * Structure for holding the internal settings of the PID controller.
 */
typedef struct pid_controller_settings_stuct
{
    /** P factor for the pid controller */
    int p_factor;

    /** I factor for the pid controller */
    int i_factor;

    /** Max speed the error can decline. */
    int error_decay;

    /** Max speed the error can grow */
    int error_grow;

    /** Scaling between controller output and fractional warp value */
    int warp_scale;
} pid_controller_settings;

/**
 * Structure for holding the PID controller state.
 */
typedef struct pid_controller_state_struct
{
    /**
     * Fractional value in the interval of (-0.005, 0.005) representing the rate
     * adjustment/warp value calculated by the PID controller to keep the endpoint
     * timed playback error in the minimum.
     */
    int warp;

    /**
     * The proportional part of the warp.
     */
    int warp_p_term;

    /**
     * The integral part of the warp.
     */
    int warp_i_term;

    /**
     * The integral part of the error used to calculate warp_i_term.
     */
    int error_i_sum;

    /* Min, max and average error is used to create a lowpass filter like calculation
     * to smoothen out the sudden variation of the error.  */
    int min_error;
    int max_error;
    int avg_error;
}pid_controller_state ;



/****************************************************************************
Public Function Declarations
*/

/**
 * \brief Runs the PID controller for the given error.
 *
 * \param pid_state Pointer to the pid_controller_state instance
 * \param pid_params Pointer to the pid_controller_settings instance
 * \param error Time difference between the timestamp time and the expected playback time.
 */
extern void timed_run_pid_controller(pid_controller_state *pid_state, const pid_controller_settings *pid_params, TIME_INTERVAL error);

/**
 * \brief Resets the PID controller internal state.
 * \param pid_params Pointer to the pid_controller_settings instance
 */
extern void timed_reset_pid_controller(pid_controller_state *pid_state);

/**
 * \brief set pid contoller default settings
 * \param pid_params Pointer to the pid_controller_settings instance
 */
extern void timed_set_pid_controller_default_settings(pid_controller_settings *pid_params);

/**
 * \brief Creates a timed playback module.
 *
 * \return pointer to the timed playback instance, Null if insufficient resources.
 */
extern TIMED_PLAYBACK* timed_playback_create(void );

/**
 * \brief set pid controller proportional gain
 * \param pid_params Pointer to the pid_controller_settings instance
 * \p_factor proportional gain value
 */
extern void timed_set_pid_controller_pfactor(pid_controller_settings *pid_params, int p_factor);

/**
 * \brief set pid controller integral gain
 * \param pid_params Pointer to the pid_controller_settings instance
 * \i_factor integral gain value
 */
extern void timed_set_pid_controller_ifactor(pid_controller_settings *pid_params, int i_factor);

/**
 * \brief Initialise the timed playback instance for time aligning playback of audio.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param cbops_op_graph - Cbops graph used for discarding, silence insertion and rate
 *              adjust.
 * \param sample_rate - The sample rate in which the timed plalyback instance should work.
 * \param period - The period in which the timed_playback_run is called.
 * \param cback callback function
 * \param con_id connection ID for the host
 * \param ep_id endpoint ID instantiating
 *
 * \return True if the initialisation was successful, false otherwise.
 */
extern bool timed_playback_init(TIMED_PLAYBACK *timed_pb,
    cbops_mgr *cbops_manager, unsigned sample_rate, TIME_INTERVAL period, unachievable_latency_callback cback, unsigned con_id, unsigned ep_id, unsigned delay);

/**
 * \brief Enable hardware warping for this timed playback instance
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param ep - Endpoint for rate adjustment
 * \param rate_adjust - Function to call with adjustment value
 */
extern void timed_playback_enable_hw_warp(TIMED_PLAYBACK *timed_pb, void *data, void (*rate_adjust)(void *data, int32 adjust_val));

/**
 * \brief Configure extra output delay for this timed playback instance
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 * \param delay - Delay value (microseconds)
 */
extern void timed_playback_set_delay(TIMED_PLAYBACK *timed_pb, unsigned delay);

/**
 * \brief Plays the next block of audio from in_buff to out_buff aiming for the
 *    target time specified in the metadata.
 *
 * \param timed_pb - Pointer to the timed_pb playback instance
 */
extern void timed_playback_run(TIMED_PLAYBACK *timed_pb);

/**
 * \brief Destroys the timed playback instance.
 *
 * \param timed_pb Pointer to the timed_pb playback instance
 */
extern void timed_playback_destroy(TIMED_PLAYBACK* timed_playback);

/**
 * \brief Return the time needed in us to use the samples with the given sample rate.
 *
 *      time [us] = (samples * 1 000 000)/ sample rate [samples/sec]
 *
 * \param samples Samples available to consume.
 * \param sample_rate The rate in which the samples are consumed in samples/sec.
 *
 * \return Time needed to use the samples
 */
extern TIME_INTERVAL convert_samples_to_time(unsigned samples, unsigned sample_rate);



/**
 * \brief Calculates the samples consumed within a given time.
 *
 *      samples = (time * sample_rate)/ 1 000 000
 *
 * \param time Time interval in which samples are consumed.
 * \param sample_rate - The data consumption sample rate at the buffer.
 *
 * \return samples
 */
extern unsigned convert_time_to_samples(TIME_INTERVAL time, unsigned sample_rate);



#endif /* TIMED_PLAYBACK_H */

