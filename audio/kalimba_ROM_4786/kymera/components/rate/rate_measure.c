/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_measure.c
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"

/****************************************************************************
 * Public Data Definitions
 */
RATE_MEASUREMENT_VALIDITY rate_measurement_validity_default =
{
    /** Minimum measurement interval is set to the same as the max_age,
     * so that there is basically always either a cached measurement
     * or a new one can be taken. (To be exactly sure that this is always
     * the case, pass the last_timestamp field to the now parameter of
     * rate_measure_take_measurement.
     */
   .min_interval    = 10000,

   /** Maximum time for which a measurement will be reused
    * (in terms of the now parameter to rate_measure_take_measurement) */
   .max_age         = 10000
};

/* RATE_MEASUREMENT.num_samples is 16 bit wide */
#define RATE_MEASUREMENT_MAX_SAMPLES ((1<<16)-1)

/****************************************************************************
 * Public Function Declarations And Inline Definitions
 */

void rate_measure_set_nominal_rate(RATE_MEASURE* rm, unsigned sample_rate)
{
    patch_fn_shared(rate_lib);

    rm->sample_period = rate_sample_rate_to_sample_period(sample_rate);
    rm->sample_rate_div25 = rate_sample_rate_div_25(sample_rate);

    RATE_MSG4("rate_measure_set_nominal_rate %p rate %d r25 %d ts %d",
              rm, sample_rate, rm->sample_rate_div25, rm->sample_period);
}

/** Check whether the difference between the new and last timestamp
 * points to a rate outside the accepted range
 */
bool rate_measure_check_rate(RATE_SAMPLE_PERIOD nom_sp, unsigned num_samples, TIME elapsed_usec)
{
    unsigned range, min_sample_period;
    TIME min_usec;

    patch_fn_shared(rate_lib);

    range = frac_mult(nom_sp, FRACTIONAL(0.05));
    min_sample_period = nom_sp - range;

    min_usec = rate_samples_to_usec(num_samples, min_sample_period);
    if (elapsed_usec < min_usec)
    {
        return FALSE;
    }
    else
    {
        unsigned max_sample_period;
        TIME max_usec;
        max_sample_period = nom_sp + range;
        max_usec = rate_samples_to_usec(num_samples, max_sample_period);
        if (elapsed_usec > max_usec)
        {
            return FALSE;
        }
        else
        {
            return TRUE;
        }
    }
}

/*
 * \note Ensure that this function is the only one which sets tracking_valid to TRUE.
 */
void rate_measure_start(RATE_MEASURE* rm, unsigned start_num_samples, TIME start_time)
{
    patch_fn_shared(rate_lib);

    rm->base_num_samples = start_num_samples;
    rm->base_timestamp = start_time;
    rm->last_num_samples = rm->base_num_samples;
    rm->last_timestamp = rm->base_timestamp;
    rm->tracking_valid = TRUE;
    rm->restarted = TRUE;
    rm->unreliable = FALSE;
    /* Ensure that the cached measurement quotient is not used */
    rm->expired = TRUE;
}

void rate_measure_continue(
        RATE_MEASURE* rm, unsigned add_samples, TIME last_time, unsigned last_num_samples)
{
    rm->last_num_samples = last_num_samples;
    rm->last_timestamp = last_time;
    rm->base_num_samples += add_samples;
}

void rate_measure_update(
        RATE_MEASURE* rm, unsigned add_samples, TIME last_time, unsigned last_num_samples)
{
    patch_fn_shared(rate_lib);

    if (rm->tracking_valid)
    {
        rate_measure_continue(rm, add_samples, last_time, last_num_samples);
    }
    else
    {
        rate_measure_start(rm, last_num_samples, last_time);
    }
}

/** "Taking" a measurement means retrieve the time and samples since
 * the base point. It also clears the measurement i.e. copies last to base.
 * If q == NULL, only test if a call would be successful.
 */
bool rate_measure_take_measurement(RATE_MEASURE* rm, RATE_MEASUREMENT* result,
                                   const RATE_MEASUREMENT_VALIDITY* validity,
                                   TIME now)
{
    patch_fn_shared(rate_lib);

    if (!rm->tracking_valid)
    {
        RATE_MSG1("rate_measure_take_measurement %p invalid", rm);
        return FALSE;
    }

    /* Wrapping calculations -- order between timestamps is known */
    TIME age = now - rm->last_measurement_time;

    if (!rm->expired && (age <= validity->max_age))
    {
        /* Do not get a new measurement; reuse the last one */
        if (result != NULL)
        {
            *result = rm->last_measurement;
        }
        RATE_MSG4("rate_measure_take_measurement %p still valid age %d samples %d t %d",
                  rm, age, rm->last_measurement.num_samples,
                  rm->last_measurement.delta_usec);
        return TRUE;
    }
    else
    {
        /* Set this so that measurements don't accidentally appear
         * to be valid 2^32usec later
         */
        rm->expired = TRUE;

        if (rm->base_num_samples <= rm->last_num_samples)
        {
            RATE_MSG1("rate_measure_take_measurement %p invalid no samples", rm);
            return FALSE;
        }
        else
        {
            unsigned num_samples = rm->base_num_samples - rm->last_num_samples;
            if (num_samples >= RATE_MEASUREMENT_MAX_SAMPLES)
            {
                RATE_MSG2("rate_measure_take_measurement %p invalid too many samples: %d",
                          rm, num_samples);
                return FALSE;
            }
            else
            {
                /* Wrapping calculation -- timestamps always going forward in time */
                TIME interval = rm->last_timestamp - rm->base_timestamp;

                if (interval < validity->min_interval)
                {
                    RATE_MSG5("rate_measure_available %p invalid tbase %d tlast %d int %d < %d",
                              rm, rm->base_timestamp, rm->last_timestamp, interval,
                              validity->min_interval);
                    return FALSE;
                }
                else
                {
                    /* Only commit i.e. take a measurement if pointer to result
                     * variable has been passed. Otherwise this function only
                     * tests.
                     */
                    if (result != NULL)
                    {
                        RATE_MEASUREMENT qu;
                        qu.num_samples = (uint16)num_samples;
                        qu.restarted = rm->restarted;
                        qu.delta_usec = interval;
                        qu.last_timestamp = rm->last_timestamp;
                        qu.unreliable =
                                rm->unreliable
                                || ! rate_measure_check_rate(
                                        rm->sample_period, num_samples, interval);

                        *result = qu;
                        rm->last_measurement = qu;

                        RATE_MSG5("rate_measure_take_measurement %p valid samples %d t %d rst %d unr %d",
                                  rm, qu.num_samples, qu.delta_usec, qu.restarted, qu.unreliable);

                        rm->base_num_samples = rm->last_num_samples;
                        rm->base_timestamp = rm->last_timestamp;
                        rm->last_measurement_time = now;
                        rm->expired = FALSE;
                        rm->restarted = FALSE;
                        rm->unreliable = FALSE;
                    }
                    else
                    {
                        RATE_MSG4("rate_measure_available %p valid tbase %d tlast %d int %d",
                                  rm, rm->base_timestamp, rm->last_timestamp, interval);
                    }
                    return TRUE;
                }
            }
        }
    }
}

/** Check whether a measurement is available.
 * Perform the same checks as rate_measure_take_measurement,
 * though don't update the measurement state.
 */
bool rate_measure_available(RATE_MEASURE* rm,
                            const RATE_MEASUREMENT_VALIDITY* validity, TIME now)
{
    return rate_measure_take_measurement(rm, NULL, validity, now);
}

