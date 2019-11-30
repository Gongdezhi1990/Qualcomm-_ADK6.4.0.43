/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_ts_filter.c
 * \ingroup rate_lib
 *
 * \note This is an implementation of an algorithm derived from the TTP generator,
 * however the error factor is automatically scaled according to an average
 * time between updates, to correct for that period's influence as a loop
 * gain external to the calculations.
 */

#include "rate_private.h"

/****************************************************************************
 * Private Macro Definitions
 */

/* Use the first definition normally, the second one when debugging */
#define VOLATILE
/* #define VOLATILE volatile */

#define RATE_UPDATE_GAIN_START  FRACTIONAL(0.5)

#define RATE_TS_FILTER_STARTING (~0u)

/****************************************************************************
 * Public Data Definitions
 */

const RATE_TS_FILTER_PARAM rate_ts_filter_audio_device_param =
{
    .startup_iterations = 50,
    .update_gain        = FRACTIONAL(0.003),
    .max_error_us       = 200,
    .ef_const           = FRACTIONAL(0.2),
    .ef_update_gain     = FRACTIONAL(0.01),
    .max_update_int_us  = 10000,
    .ef_max             = FRACTIONAL(0.0002),
    .ef_min             = FRACTIONAL(0.00002)
};

/****************************************************************************
 * Public Function Implementations
 */

void rate_ts_filter_init(RATE_TS_FILTER* rtsf, const RATE_TS_FILTER_PARAM* param)
{
    if (rtsf == NULL || param == NULL)
    {
        return;
    }

    rtsf->param = param;
    rtsf->nominal_sample_period = RATE_TO_SAMPLE_PERIOD(RATE_DEFAULT_SAMPLE_RATE);
    rate_ts_filter_start(rtsf);
}

void rate_ts_filter_start(RATE_TS_FILTER* rtsf)
{
    if (rtsf == NULL)
    {
        return;
    }

    rtsf->startup_remaining = RATE_TS_FILTER_STARTING;
    rtsf->update_gain = RATE_UPDATE_GAIN_START;
    rtsf->sp_adjust = 0;
}

void rate_ts_filter_set_rate(RATE_TS_FILTER* rtsf, unsigned sample_rate)
{
    if (rtsf == NULL)
    {
        return;
    }

    if (sample_rate != 0)
    {
        rtsf->nominal_sample_period =
                rate_sample_rate_to_sample_period(sample_rate);
    }
    rate_ts_filter_start(rtsf);
}

TIME rate_ts_filter_get_rounded(const RATE_TS_FILTER* rtsf)
{
    return (rtsf->sample_time + (1u << (RATE_TIME_EXTRA_RESOLUTION-1)))
            >> RATE_TIME_EXTRA_RESOLUTION;
}

void rate_ts_filter_update(RATE_TS_FILTER* rtsf, unsigned num_samples, TIME time)
{
    if (rtsf == NULL)
    {
        return;
    }

    RATE_TIME time_scaled = (RATE_TIME)time << RATE_TIME_EXTRA_RESOLUTION;
    const RATE_TS_FILTER_PARAM* param = rtsf->param;

    if (rtsf->startup_remaining == RATE_TS_FILTER_STARTING)
    {
        rtsf->sample_time = time_scaled;
        rtsf->startup_remaining = param->startup_iterations;
        rtsf->last_update = time;
        rtsf->starting_updates = 0;
    }
    else
    {
        /** Calculate the error factor based on the average update interval.
         * The amount by which the error increases from one update to
         * the next is proportional to the time between updates.
         * I.e. in the feedback loop outside this function, from sp_adjust
         * to error, the gain is proportional to the update interval.
         * To have similar loop behaviour for a range of update intervals,
         * calculate an error factor which is inversely proportional to the
         * update interval, within bounds (ef_min..ef_max).
         * The constant ef_const relates to the steady-state error
         * for a given rate deviation:
         * If real_sample_period = (1 + sp_a) * nominal_sample_period, then
         * steady_state_error = sp_a * update_interval / ef_const.
         *
         * For instance the default parameters produce a steady-state error
         * of 100us for sp_a=1%, update_interval=2ms.
         */
        if (rtsf->startup_remaining > 0)
        {
            VOLATILE TIME time_since_start;
            VOLATILE TIME average_update_interval;

            rtsf->starting_updates += 1;
            time_since_start = time - rtsf->last_update;

            average_update_interval = time_since_start / rtsf->starting_updates;
            if (average_update_interval > param->max_update_int_us)
            {
                average_update_interval = param->max_update_int_us;
            }

            rtsf->error_factor = param->ef_const / (int)average_update_interval;
        }
        else
        {
            VOLATILE TIME elapsed = time - rtsf->last_update;
            rtsf->last_update = time;

            if (elapsed > param->max_update_int_us)
            {
                elapsed = param->max_update_int_us;
            }

            VOLATILE int update_error_factor = param->ef_const / (int)elapsed;

            rtsf->error_factor = frac_mult(param->ef_update_gain, update_error_factor)
                                 + frac_mult(
                                         (FRACTIONAL(1.0)- param->ef_update_gain),
                                         rtsf->error_factor);
        }
        if (rtsf->error_factor > param->ef_max)
        {
            rtsf->error_factor = param->ef_max;
        }
        else if (rtsf->error_factor < param->ef_min)
        {
            rtsf->error_factor = param->ef_min;
        }

        /* In the first step, adjustment is Qs16.m, where
         * m == RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION + DAWTH - 1.
         *
         * First cast nominal_sample_period from unsigned to signed without
         * size change, then cast to tell the compiler to produce a double
         * width result.
         */
        VOLATILE RATE_STIME adjustment;
        VOLATILE RATE_STIME predicted_sample_period;

        static const int adjustment_shift = RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION
                                            + DAWTH - 1 - RATE_TIME_EXTRA_RESOLUTION;

        adjustment = ((RATE_STIME)(int)(rtsf->nominal_sample_period) * rtsf->sp_adjust)
                     >> adjustment_shift;

        /* Doing the shift in unsigned avoids an unnecessary sign extension step during
         * the double word shift, thus the produced assembly is cleaner
         */
        predicted_sample_period =
                (RATE_STIME)(((RATE_TIME)(rtsf->nominal_sample_period)) << RATE_SAMPLE_PERIOD_TO_TIME_SHIFT)
                + adjustment;

        VOLATILE RATE_STIME predicted_delta = predicted_sample_period * (int)num_samples;

        rtsf->sample_time += predicted_delta;

        VOLATILE RATE_STIME error = (RATE_STIME)time_scaled - (RATE_STIME)rtsf->sample_time;
        VOLATILE TIME error_trunc_us = error >> RATE_TIME_EXTRA_RESOLUTION;
        VOLATILE int update;

        if ( (error_trunc_us > (int)param->max_error_us)
             || (error_trunc_us < -(int)param->max_error_us))
        {
            /* Should restart; clipping won't stabilize things */
            rtsf->startup_remaining = RATE_TS_FILTER_STARTING;
            update = 0;
        }
        else
        {
            /* Decompose the error factor into a right shift and mantissa,
             * to avoid long multiplication of error with error factor below.
             */
            VOLATILE unsigned error_shift = pl_sign_detect(rtsf->error_factor);
            VOLATILE unsigned error_factor_normalized = rtsf->error_factor << error_shift;

            /* The "+ 1" accounts for changing from 32 fractional bits
             * in RATE_STIME error to 31 fractional bits at the input
             * to frac_mult
             */
            update = frac_mult((int)(error >> (error_shift + 1)), error_factor_normalized);
        }

        rtsf->sp_adjust =
                (int)
                ( ( (RATE_STIME)update * rtsf->update_gain
                    + (RATE_STIME)(rtsf->sp_adjust) * (FRACTIONAL(1.0) - rtsf->update_gain) )
                  >> (DAWTH - 1) );

        if ((rtsf->startup_remaining > 0)
            && (rtsf->startup_remaining != RATE_TS_FILTER_STARTING))
        {
            rtsf->startup_remaining -= 1;
            if (rtsf->startup_remaining == 0)
            {
                rtsf->update_gain = param->update_gain;
                rtsf->last_update = time;
            }
        }
    }
}
