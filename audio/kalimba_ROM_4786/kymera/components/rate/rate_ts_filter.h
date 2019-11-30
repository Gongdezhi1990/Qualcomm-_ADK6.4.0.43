/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_ts_filter.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_TS_FILTER_H
#define RATE_RATE_TS_FILTER_H

#include "rate_types.h"

/****************************************************************************
 * Public Type Definitions
 */

/** Parameters to tune the algorithm */

typedef struct {
    /** Startup countdown length */
    unsigned    startup_iterations;

    /** Update gain in running state */
    int         update_gain;

    /** Maximum residual time offset */
    unsigned    max_error_us;

    /** Constant used to calculate error factor */
    int         ef_const;

    /** Gain for updates to error factor in running state */
    int         ef_update_gain;

    /** Maximum time between updates for use in error factor */
    unsigned    max_update_int_us;

    /** Maximum error factor */
    int         ef_max;

    /** Minimum error factor */
    int         ef_min;

} RATE_TS_FILTER_PARAM;

/** Filter state */

typedef struct {
    /** Sample time in microseconds, Qu32.m */
    RATE_TIME                   sample_time;

    /** Nominal period, in microseconds, Qu16.m.
     * This does not need to be of high accuracy as it is
     * mainly a starting value */
    RATE_SAMPLE_PERIOD          nominal_sample_period;

    /** Deviation of actual sample period from nominal sample period, Qs0.31.
     * I.e. actual sample period = (1 + sp_adjust) * nominal_sample_period
     */
    int                         sp_adjust;

    /** Startup countdown, number of updates remaining */
    unsigned                    startup_remaining;

    /** Current update gain */
    int                         update_gain;

    /** While starting, time of first update; later, time of previous update */
    TIME                        last_update;

    /** Number of updates since starting */
    unsigned                    starting_updates;

    /** Calculated error factor */
    int                         error_factor;

    /** Parameters */
    const RATE_TS_FILTER_PARAM* param;
} RATE_TS_FILTER;

/****************************************************************************
 * Public Function Declarations
 */

/** Initialize a filter context with parameters.
 * \param rtsf Filter context
 * \param param Filter parameter
 */
extern void rate_ts_filter_init(RATE_TS_FILTER* rtsf, const RATE_TS_FILTER_PARAM* param);

/** Reset and place in starting state.
 * \param rtsf Filter context
 */
extern void rate_ts_filter_start(RATE_TS_FILTER* rtsf);

/** Configure with sample rate. Also restarts.
 * \param rtsf Filter context
 * \param rate Sample rate in Hz
 */
extern void rate_ts_filter_set_rate(RATE_TS_FILTER* rtsf, unsigned rate);

/** Update with a new measurement point.
 * \param rtsf Filter context
 * \param num_samples Number of samples since the last update
 * \param time Timestamp for the new measurement, in microseconds
 */
extern void rate_ts_filter_update(RATE_TS_FILTER* rtsf, unsigned num_samples, TIME time);

/** Retrieve output value of the filter, corresponding to the
 * last update.
 * \param rtsf Filter context
 * \return Output of the filter rounded to microseconds
 */
extern TIME rate_ts_filter_get_rounded(const RATE_TS_FILTER* rtsf);


/****************************************************************************
 * Public Data Declarations
 */

/** A standard set of parameters */
extern const RATE_TS_FILTER_PARAM rate_ts_filter_audio_device_param;


#endif /* RATE_RATE_TS_FILTER_H */
