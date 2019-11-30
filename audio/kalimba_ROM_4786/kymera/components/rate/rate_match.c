/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_match.c
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"
#if defined(RATE_MATCH_LOG)||defined(RATE_MATCH_DEBUG)
#include "audio_log/audio_log.h"
#endif

/****************************************************************************
 * Public Data
 */
#ifdef RATE_MATCH_ADAPT_DEFAULT_NONE
const RATE_MATCH_CONFIG rate_match_default_config =
{
    .pid_kp         =   FRACTIONAL(RATE_MATCH_PID_KP_DEFAULT),
    .pid_ki         =   FRACTIONAL(RATE_MATCH_PID_KI_DEFAULT
                                   * RATE_MATCH_PERIOD_DEFAULT),

    .adapt_fn       =   NULL,
    .enable_adapt   =   FALSE,
    .adapt_cfg_u8_1 =   0,
    .adapt_cfg_u8_2 =   0,
    .adapt_cfg_u8_3 =   0,
    .adapt_cfg_int_1 =  0,
    .adapt_cfg_int_2 =  0,
    .adapt_cfg_int_3 =  0
};
#endif /* RATE_MATCH_ADAPT_DEFAULT_NONE */

/****************************************************************************
 * Public Function Implementations
 */

/** \brief Set up a RATE_MATCH_CONTROL context and connect it
 *         to the related objects
 *  \param rmc The RATE_MATCH_CONTROL instance
 *  \param fb_measure Pointer to a RATE_MEASURE instance used for feedback
 *  \param config Configuration (i.e. coefficients). If NULL, default
 *                coefficients will be used.
 *  \param valid_criteria Measurement criteria
 *  \param sample_rate Feedback path sample rate (either Hz or Hz/25)
 *  \return False if invalid parameters (NULLs) were passed, true otherwise
 */
bool rate_match_init(RATE_MATCH_CONTROL* rmc, RATE_MEASURE* fb_measure,
                     const RATE_MATCH_CONFIG* config,
                     const RATE_MEASUREMENT_VALIDITY* valid_criteria,
                     unsigned sample_rate)
{
    patch_fn_shared(rate_lib);

    if ((rmc == NULL) || (fb_measure == NULL) || (valid_criteria == NULL))
    {
        return FALSE;
    }
    if (config == NULL)
    {
        config = & rate_match_default_config;
    }

    rmc->fb_measure = fb_measure;
    rmc->valid_criteria = valid_criteria;
    rmc->config = config;
    rmc->enable_adapt = config->enable_adapt;
    rate_pid_set_coeff(&rmc->pid, config->pid_kp, config->pid_ki);

    /** Set the fb sample rate, and default the reference sample rate
     * to the same. The reference rate will be updated later.
     */
    rate_compare_set_ref_sample_rate(&rmc->cmp, sample_rate);
    rate_compare_set_fb_sample_rate(&rmc->cmp, sample_rate);

    rate_match_reset(rmc);

#ifdef RATE_MATCH_DEBUG
    rmc->log_serial = 0;
#endif /* RATE_MATCH_DEBUG */

    return TRUE;
}

/** \brief Update and calculate a new correction
 *
 */
bool rate_match_update(RATE_MATCH_CONTROL* rmc, const RATE_RELATIVE_RATE* ref,
                       int* correction, TIME now)
{
    patch_fn_shared(rate_lib);

    if (rmc == NULL || ref == NULL || correction == NULL)
    {
        return FALSE;
    }

    if (!ref->valid)
    {
        rate_match_reset(rmc);
        return FALSE;
    }

    RATE_MEASUREMENT fb;
    if (rate_measure_take_measurement(rmc->fb_measure, &fb, rmc->valid_criteria, now))
    {
        int deviation = 0;
        rate_compare_set_ref_sample_rate(&rmc->cmp, ref->nominal_rate_div25);

#ifdef RATE_MATCH_DEBUG
        if (rmc->enable_trace)
        {
            L3_DBG_MSG5("\"rate_match\",%d, 1, %d,%d,%d,%d",
                        ++(rmc->log_serial),
                        ref->q.num_samples, ref->q.delta_usec,
                        fb.num_samples, fb.delta_usec);
        }
#endif /* RATE_MATCH_DEBUG */

        RATE_COMPARE_RESULT compare_status =
                rate_compare(&rmc->cmp, &ref->q, &fb, &deviation);
        if (compare_status != RATE_COMPARE_FAILED)
        {
#ifdef RATE_MATCH_DEBUG
            if (rmc->enable_trace)
            {
                L3_DBG_MSG5("\"rate_match\",%d, 2, %d,%d,%d,%d",
                            ++(rmc->log_serial),
                            compare_status,
                            rmc->cmp.fb_sample_count_error,
                            rmc->cmp.fb_sample_count_residual,
                            deviation);
            }
#endif /* RATE_MATCH_DEBUG */

            int speed;

            if (rmc->config->adapt_fn != NULL)
            {
                speed = (rmc->config->adapt_fn)(rmc, deviation, compare_status);
            }
            else
            {
                speed = rmc->pid_speed;
            }

            /* Integrate the new deviation */
            *correction = rate_pid_update_scaled(&rmc->pid, deviation, speed);

            return TRUE;
        }
    }
    return FALSE;
}

/** \brief Reset rate match state
 *
 */
void rate_match_reset(RATE_MATCH_CONTROL* rmc)
{
    patch_fn_shared(rate_lib);

    if (rmc != NULL)
    {
        /* Reset error and startup count */
        rate_compare_start(&rmc->cmp);
        rate_pid_reset(&rmc->pid);
        rmc->pid_speed = MAXINT; /* FRACTIONAL(1.0) */
        if (rmc->config->adapt_fn != NULL)
        {
            (rmc->config->adapt_fn)(rmc, 0, RATE_MATCH_ADAPT_RESET);
        }
    }
}

#ifdef RATE_MATCH_DEBUG
/** \brief Enable/disable debug data trace output */
void rate_match_trace_enable(RATE_MATCH_CONTROL* rmc, bool enable)
{
    if (rmc != NULL)
    {
        rmc->enable_trace = enable;
    }
}

/** \brief Accessor for the trace enable flag */
bool rate_match_is_trace_enabled(const RATE_MATCH_CONTROL* rmc)
{
    return (rmc != NULL) && rmc->enable_trace;
}

/** \brief Trace with caller defined data */
void rate_match_trace_client(RATE_MATCH_CONTROL* rmc, int p1, int p2, int p3)
{
    if ((rmc != NULL) && rmc->enable_trace)
    {
        L3_DBG_MSG5("\"rate_match\",%d, 3, %d,%d,%d,%d",
                    ++(rmc->log_serial), rmc->pid.pid_int, p1, p2, p3);
    }
}

#endif /* RATE_MATCH_DEBUG */
