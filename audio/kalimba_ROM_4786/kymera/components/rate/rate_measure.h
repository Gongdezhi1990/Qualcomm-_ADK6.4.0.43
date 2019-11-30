/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_measure.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_MEASURE_H
#define RATE_RATE_MEASURE_H

#include "rate_types.h"

/****************************************************************************
 * Public Type Definitions
 */

/**
 * Parameters for validity of a measurement
 */
typedef struct rate_measurement_validity
{
    /** Minimum interval (timestamp difference between base and last point) */
    TIME                        min_interval;

    /** Maximum age (time since last measurement) of a measurement */
    TIME                        max_age;
}
RATE_MEASUREMENT_VALIDITY;

/**
 * State structure for rate measurements using a base point and latest point.
 */
typedef struct rate_measure
{
    /** Span between previous and current is valid.
     * \note 8-bit packing results in relatively efficient byte accesses */
    bool                        tracking_valid  : 8;

    /** Started since last measurement was taken.
     * \note 8-bit packing results in relatively efficient byte accesses */
    bool                        restarted       : 8;

    /** An update contained unreliable data.
     * \note 8-bit packing results in relatively efficient byte accesses */
    bool                        unreliable      : 8;

    /** Measurement is too old. Setting this flag ensures that a measurement
     * doesn't accidentally appear to be valid again 2^32usec later.
     * \note 8-bit packing results in relatively efficient byte accesses
     */
    bool                        expired         : 8;

    /** Samples from base point until start of current buffer. */
    unsigned                    base_num_samples;

    /** Timestamp of base point */
    TIME                        base_timestamp;

    /** Samples from last point until start of current buffer. */
    unsigned                    last_num_samples;

    /** Timestamp of last point */
    TIME                        last_timestamp;

    /** Sample rate / 25 */
    unsigned                    sample_rate_div25;

    /** Sample period in the module's format */
    RATE_SAMPLE_PERIOD          sample_period;

    /** Time (not timestamp) when last valid measurement was taken */
    TIME                        last_measurement_time;

    /** Cached last measurement */
    RATE_MEASUREMENT            last_measurement;
}
RATE_MEASURE;

/****************************************************************************
 * Public Data Declarations
 */
extern RATE_MEASUREMENT_VALIDITY rate_measurement_validity_default;

/****************************************************************************
 * Public Function Declarations And Inline Definitions
 */

/** Sets the current measurements point as invalid.
 * \param rm Rate measurement context
 */
static void inline rate_measure_stop(RATE_MEASURE* rm)
{
    rm->tracking_valid = FALSE;
}

/** Retrieve the flag for the validity of the current measurement.
 * \param rm Rate measurement context
 * \return True if the values represent a valid ongoing measurement.
 */
static bool inline rate_measure_valid(const RATE_MEASURE* rm)
{
    return rm->tracking_valid;
}

/** Set the unreliable flag
 * \param rm Rate measurement context
 */
static void inline rate_measure_set_unreliable(RATE_MEASURE* rm)
{
    rm->unreliable = TRUE;
}

/** Sets the nominal sample rate.
 * \param rm Rate measurement context
 * \param rate Sample rate in Hz
 */
extern void rate_measure_set_nominal_rate(
        RATE_MEASURE* rm, unsigned rate);

/** Check whether the ratio between the given sample count and time interval
 * represents a rate within reasonable margin (+/- 5%) of the nominal rate.
 * \param nom_sp Nominal sample period [in]
 * \param num_samples Number of samples between points
 * \param elapsed_usec Timespan in microseconds between points
 * \return True if the implied rate is within range
 */
extern bool rate_measure_check_rate(
        RATE_SAMPLE_PERIOD nom_sp, unsigned num_samples, TIME elapsed_usec);

/**
 * Record a timestamp as a starting point. Set the latest point to the
 * same values.
 * \param rm Rate measurement context [in/out]
 * \param num_samples Number of samples to initially record as covered by measurement.
 * \param time Timestamp
 *
 * \note When using metadata tags as data source, the num_samples parameter is set
 * to the number of samples between the tag and end of current buffer.
 * The corresponding argument to rate_measure_continue is also set to the
 * number of samples between that tag and end of buffer then. This allows calculating
 * the number of samples between the two tags in rate_measure_take_measurement.
 */
extern void rate_measure_start(
        RATE_MEASURE* rm, unsigned num_samples, TIME time);

/**
 * Record a timestamp as a latest point. Do not move the base point.
 * \param rm Rate measurement context [in/out]
 * \param add_samples Amount by which to increase the sample counter
 * \param last_time New timestamp
 * \param last_num_samples New sample count to record. This is the length
 *                         of a range of samples ahead of the new timestamp,
 *                         just as num_samples passed to rate_measure_start.
 *                         This range overlaps the range implied by the
 *                         add_samples parameter.
 *
 * \note See note for rate_measure_start
 */
extern void rate_measure_continue(
        RATE_MEASURE* rm, unsigned add_samples, TIME last_time, unsigned last_num_samples);

/**
 * Start measurement if necessary and record a timestamp.
 * If tracking is true, calls rate_measure_continue, else rate_measure_start.
 *
 * \param rm Rate measurement context [in/out]
 * \param add_samples Amount by which to increase the sample counter
 * \param last_time New timestamp
 * \param last_num_samples New sample count to record. This is the length
 *                         of a range of samples ahead of the new timestamp,
 *                         just as num_samples passed to rate_measure_start.
 *                         This range overlaps the range implied by the
 *                         add_samples parameter.
 * \note see rate_measure_start, rate_measure_continue
 */
extern void rate_measure_update(
        RATE_MEASURE* rm, unsigned add_samples, TIME last_time, unsigned last_num_samples);

/** Check whether a measurement is available according to interval and age
 *
 * \param rm Rate measurement context [in/out] (the expired field may be set)
 * \param validity Pointer to limits for interval and age [in]
 * \param now Current system time
 * \return True if there is a valid measurement available
 */
extern bool rate_measure_available(
        RATE_MEASURE* rm, const RATE_MEASUREMENT_VALIDITY* validity, TIME now);

/**
 * Retrieve sample count and time difference from the measurement context,
 * and reset the base point to the latest point.
 *
 * \param rm Rate measurement context [in/out]
 * \param q Rate represented as a quotient [out]
 * \param validity Pointer to limits for interval and age [in]
 * \param now Current system time; this should be the same value as
 *            passed to rate_measure_available() recently
 * \return True if there was a valid rate measurement
 */
extern bool rate_measure_take_measurement(
        RATE_MEASURE* rm, RATE_MEASUREMENT* q,
        const RATE_MEASUREMENT_VALIDITY* validity, TIME now);


#endif /* RATE_RATE_MEASURE_H */
