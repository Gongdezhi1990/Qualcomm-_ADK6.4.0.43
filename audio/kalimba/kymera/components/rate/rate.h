/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_H
#define RATE_RATE_H

#include "rate_types.h"

/* Public sub-module declarations */
#include "rate_measure.h"
#ifdef INSTALL_METADATA
#include "rate_measure_metadata.h"
#endif /* INSTALL_METADATA */
#include "rate_compare.h"
#include "rate_ts_filter.h"
#include "rate_pid.h"
#include "rate_match.h"

/* Types referenced in the declarations in this file */
#ifdef INSTALL_METADATA
#include "buffer/cbuffer_c.h"
#endif /* INSTALL_METADATA */

/* Definitions used in inline functions defined here */
#include "platform/pl_fractional.h"

/****************************************************************************
 * Public Type Definitions
 */

/****************************************************************************
 * Public Function Declarations And Inline Definitions
 */

/**********************************************
 * Conversion functions
 */

/**
 * \params t Non-negative time in seconds represented as Q1.N
 * \params sr Sample rate in Hz
 * \return Number of samples in t at sr
 */
static inline unsigned rate_time_to_samples(unsigned t, unsigned sr)
{
    return (unsigned)frac_mult(t, sr);
}

/**
 * \brief Calculate whole sample periods in time t, i.e.
 *        version of rate_time_to_samples which rounds towards zero
 * \params t Non-negative time in seconds represented as Q1.N
 * \params sr Sample rate in Hz
 * \return Number of samples in t at sr
 */
static inline unsigned rate_time_to_samples_trunc(unsigned t, unsigned sr)
{
    return (unsigned)(((uint48)t * sr) >> (DAWTH-1));
}

/* rate_conv.c */

/** Convert from a sample rate in Hz to sample period in the RATE_SAMPLE_PERIOD
 * format.
 * \param sample_rate Sample rate in Hz
 * \return Sample period in microseconds, Qu16.m
 */
extern RATE_SAMPLE_PERIOD rate_sample_rate_to_sample_period(unsigned sample_rate);

/** Calculate length of a number of sample periods in microseconds.
 * \param num_samples Number of samples
 * \param sample_period in microseconds, Qu16.m
 * \return Rounded duration of num_samples samples, in microseconds
 */
extern TIME rate_samples_to_usec(unsigned num_samples, RATE_SAMPLE_PERIOD sample_period);

/** Calculate length of a number of sample periods in microseconds, when the
 * sample count is specified with some fractional bits.
 * \param num_samples_frac Number of samples, Qn.x where x is given by the
 *                         sample_shift argument
 * \param sample_shift Number of fractional bits in num_samples_frac.
 *                     The useful and tested range is 0..8.
 * \param sample_period in microseconds, Qu16.m
 * \return Rounded duration of num_samples samples, in microseconds
 */
extern TIME rate_samples_frac_to_usec(unsigned num_samples_frac,
                                      unsigned sample_shift,
                                      RATE_SAMPLE_PERIOD sample_period);

/**********************************************
 * Utility functions
 */

/** Reduce two sample rates. The purpose is to minimize the number of bits
 * needed for calculations involving ratios of sample rates; the result
 * is not an irreducible pair. With rates up to 192000Hz, the output
 * values are <= 2560.
 * Either both fs1 and fs2 are in Hz, or both are in Hz divided by 25,
 * but not mixed.
 * \param fs1 First sample rate in Hz or in Hz divided by 25.
 * \param fs2 Second sample rate in Hz or in Hz divided by 25.
 * \param reduced1 Pointer to variable for first reduced sample rate [out]
 * \param reduced2 Pointer to variable for second reduced sample rate [out]
 * \return False if any invalid inputs were detected. True does not guarantee validity.
 */
extern bool rate_reduce_sample_rates(unsigned fs1, unsigned fs2, unsigned* reduced1, unsigned* reduced2);

/** Given a supported sample rate, produce the rate divided by 25.
 * This function is not guaranteed to be correct for other arguments.
 * \param fs Full sample rate
 * \return fs/25 when fs is a supported sample rate, undefined otherwise.
 */
static inline unsigned rate_sample_rate_div_25(unsigned fs)
{
#ifdef __GNUC__
    return fs/25;
#else
    return frac_mult(fs, FRACTIONAL(0.04));
#endif
}

/* rate_metadata.c */
#ifdef INSTALL_METADATA

/** Determine the timestamp type, if any, contained in a metadata tag.
 * \param tag Metadata tag [in]
 * \return Type of timestamp in the tag
 */
extern RATE_TIMESTAMP_TYPE rate_metadata_get_timestamp_type(const metadata_tag* tag);

#endif /* INSTALL_METADATA */


#endif /* RATE_RATE_H */
