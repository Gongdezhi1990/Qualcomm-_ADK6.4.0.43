/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_types.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_TYPES_H
#define RATE_RATE_TYPES_H

#include "types.h"


/**********************************************************************************
 * Public macros
 */

/**********************************************************************************
 * Public type definitions
 */

/**
 * Unsigned type for time with extended precision (Qu32.m).
 * The integer part is 32 bits wide, the extra precision depends
 * on the size of the machine's next larger integer type.
 *
 * The signed type is used when calculating the difference between times
 * and the order of the two arguments isn't known, as well as to
 * control the signedness of arguments to arithmetic operators.
 */
#if DAWTH==24
typedef uint48 RATE_TIME;
typedef int48 RATE_STIME;
#else
typedef uint64 RATE_TIME;
typedef int64 RATE_STIME;
#endif

/**
 * Nominal sample period, microseconds, Qu16.m
 * (the exponent is RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION)
 */
typedef unsigned RATE_SAMPLE_PERIOD;

/**
 * A short signed time interval, with the same resolution
 * as RATE_SAMPLE_PERIOD, i.e. Qs15.m
 */
typedef int RATE_SHORT_INTERVAL;

/* Constants describing precision/width */
enum
{
    /** Unsigned integer bits in RATE_SAMPLE_PERIOD */
    RATE_SAMPLE_PERIOD_INT_BITS             = 16,

    /** Fractional bits in RATE_SAMPLE_PERIOD */
    RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION     = (DAWTH - RATE_SAMPLE_PERIOD_INT_BITS),

    /** Integer bits without sign bit in RATE_SHORT_INTERVAL */
    RATE_SHORT_INTERVAL_INT_BITS             = (RATE_SAMPLE_PERIOD_INT_BITS - 1),

    /** Fractional bits in RATE_SHORT_INTERVAL */
    RATE_SHORT_INTERVAL_EXTRA_RESOLUTION     = RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION,

    /** Unsigned integer bits in RATE_TIME. There is one less
     * integer bit in RATE_STIME i.e. the fraction is the same
     * length in RATE_TIME and RATE_STIME.
     */
    RATE_TIME_INT_BITS                      = 32,

    /** Fractional bits in RATE_TIME and RATE_STIME */
    RATE_TIME_EXTRA_RESOLUTION              = (2*DAWTH - RATE_TIME_INT_BITS),

    /** Left shift to convert from RATE_SAMPLE_PERIOD to RATE_TIME */
    RATE_SAMPLE_PERIOD_TO_TIME_SHIFT        = (RATE_TIME_EXTRA_RESOLUTION
                                              - RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION)
};

/** Constants for time units */
#define RATE_MICROSECONDS_PER_SECOND        (1000000)
#define RATE_MICROSECONDS_PER_MILLISECOND   (1000)

#define RATE_TIME_SECOND \
    ((RATE_TIME)RATE_MICROSECONDS_PER_SECOND << RATE_TIME_EXTRA_RESOLUTION)

#define RATE_TIME_MILLISECOND \
    ((RATE_TIME)RATE_MICROSECONDS_PER_MILLISECOND << RATE_TIME_EXTRA_RESOLUTION)

#define RATE_STIME_SECOND \
    ((RATE_STIME)RATE_MICROSECONDS_PER_SECOND << RATE_TIME_EXTRA_RESOLUTION)

#define RATE_STIME_MILLISECOND \
    ((RATE_STIME)RATE_MICROSECONDS_PER_MILLISECOND << RATE_TIME_EXTRA_RESOLUTION)

#define RATE_SAMPLE_PERIOD_MILLISECOND \
    ((RATE_SAMPLE_PERIOD)RATE_MICROSECONDS_PER_SECOND << RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION)

#ifdef INSTALL_METADATA

/** A shorthand for the type of timestamp contained in a metadata tag */
typedef enum rate_timestamp_type_enum
{
    RATE_TIMESTAMP_TYPE_NONE        = 0,
    RATE_TIMESTAMP_TYPE_TTP,
    RATE_TIMESTAMP_TYPE_TOA
}
RATE_TIMESTAMP_TYPE;

#endif /* INSTALL_METADATA */

/** This structure represents a rate as a quotient
 * of a sample count and time difference
 */
typedef struct
{
    /** Number of samples (numerator of rate) */
    uint16      num_samples;

    /** True if this measurement is the first retrieved,
     * or not contiguous with a previously retrieved measurement.
     * \note 8-bit packing results in relatively efficient byte access */
    bool        restarted   : 8;

    /** True if this measurement might be untrustworthy, e.g. outside
     * a reasonable range around the nominal rate.
     * \note 8-bit packing results in relatively efficient byte access */
    bool        unreliable  : 8;

    /** Elapsed time (denominator of rate) */
    TIME        delta_usec;

    /** Timestamp of end of last measurement,
     * for calculating offsets between measurements */
    TIME        last_timestamp;
}
RATE_MEASUREMENT;

/** This type represents an optional relative rate
 * consisting of nominal sample rate, sample count
 * and time difference
 */
typedef struct
{
    /** True if the quotient is valid
     * \note 8-bit packing results in relatively efficient byte access */
    bool                valid   : 8;

    /** Nominal rate in Hz divided by 25, to fit in 16 bits */
    uint16              nominal_rate_div25;

    /** Rate represented as quotient of sample count
     * and time difference
     */
    RATE_MEASUREMENT    q;
}
RATE_RELATIVE_RATE;

#endif /* RATE_RATE_TYPES_H */
