/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_compare.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_COMPARE_H
#define RATE_RATE_COMPARE_H

#include "rate_types.h"

/****************************************************************************
 * Public Type Definitions
 */

/** RATE_COMPARE_RESULT is a bitmap */
typedef enum
{
    /** Invalid parameters */
    RATE_COMPARE_FAILED = 0,

    /** Valid comparison */
    RATE_COMPARE_VALID          = (1<<0),

    /** (Re-)Started measurements */
    RATE_COMPARE_START          = (1<<1),

    /** One or both measurements indicated that they
     * were unreliable, so the result is too */
    RATE_COMPARE_UNRELIABLE     = (1<<2)

} RATE_COMPARE_RESULT;

#define RATE_COMPARE_ALG (3)

/** Context for comparisons.
 * Its purpose is twofold: to reduce length of parameter lists,
 * and as a placeholder for future state fields such as error
 * residuals. I.e. the API is in the shape needed to add such
 * extensions without changing client code.
 */
typedef struct
{
    /** TeX formulas in parentheses refer to CS-00401459-DD section 8.1 */

    /** Sample rate of the feedback side, in multiples of 25Hz (f_s^f) */
    unsigned    fb_sample_rate_div25;

    /** Sample rate of the reference side, in multiples of 25Hz (f_s^r) */
    unsigned    ref_sample_rate_div25;

    /**
     * Add state fields (such as error residuals) when necessary
     */
    /** Reduced rate quotient, feedback rate (g_s^f) */
    unsigned    fb_reduced_rate;

    /** Reduced rate quotient, reference rate (g_s^r) */
    unsigned    ref_reduced_rate;

    /** Difference of expected to reported feedback sample count,
     * integer part (signed)
     */
    int         fb_sample_count_error;

    /** Difference of expected to reported feedback sample count,
     * fractional part (unsigned) in Qu1.n
     */
    unsigned    fb_sample_count_residual;

    /** Limit on ref_sample_count to avoid having to reduce precision */
    unsigned    max_ref_sample_count;

    /** Number of comparisons since start (p) */
    unsigned    count;

#if RATE_COMPARE_ALG==3
    /** Integer feedback sample count error accumulator (A_p) */
    int         fb_acc;

    /** Numerator of rational expected feedback sample count (b_p) */
    int         fb_exp_samples_remainder;

    /** Fractional part of starting sample count offset (c_0 - \lfloor c_0 \rfloor)
     * (non-negative) */
    int         offset_samples_frac;
#endif /* RATE_COMPARE_ALG==3 */
} RATE_COMPARE;

/****************************************************************************
 * Public Function Declarations
 */

/** (Re-)start a comparison i.e. reset any state.
 * \param rcmp Rate compare context
 */
extern void rate_compare_start(RATE_COMPARE* rcmp);

/** Get number of successful comparisons since start
 * \param rcmp Rate compare context
 * \return Count of comparisons
 */
static inline unsigned rate_compare_get_count(const RATE_COMPARE* rcmp)
{
    return rcmp->count;
}

/** Configure the rate comparison state with a reference sample rate
 * \param rcmp Rate compare context
 * \param sample_rate Reference side sample rate in Hz, or in Hz divided by 25
 */
extern void rate_compare_set_ref_sample_rate(RATE_COMPARE* rcmp, unsigned sample_rate);

/** Configure the rate comparison state with a feedback sample rate
 * \param rcmp Rate compare context
 * \param sample_rate Feedback side sample rate in Hz, or in Hz divided by 25
 */
extern void rate_compare_set_fb_sample_rate(RATE_COMPARE* rcmp, unsigned sample_rate);

/** Return a fractional value for (rate_fb/rate_ref - 1)
 * from two rates represented by quotients fb and ref.
 *
 * \param rcmp Rate compare context (writeable to allow for updating state)
 * \param ref Time/samples quotient representing the reference rate
 * \param fb Time/samples quotient representing the feedback rate
 * \param deviation [out] Result, approx.
 *                  (actual fb sample period / expected fb sample period)
 * \return Information about how to treat the output value
 */
extern RATE_COMPARE_RESULT rate_compare(RATE_COMPARE* rcmp, const RATE_MEASUREMENT* ref,
                         const RATE_MEASUREMENT* fb, int* deviation);

/** Return a fractional value for (rate_fb - 1)
 * from a rate represented by the quotient fb.
 *
 * \param sample_rate_div25 Scaled sample rate
 * \param fb Time/samples quotient representing the feedback rate
 * \param deviation [out] Result
 * \return True if input values were within range to produce a valid result
 */
extern bool rate_deviation(unsigned sample_rate_div25, const RATE_MEASUREMENT* fb,
                           int* deviation);

#endif /* RATE_RATE_COMPARE_H */
