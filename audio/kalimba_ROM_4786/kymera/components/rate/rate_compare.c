/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_compare.c
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"

/* Define this to turn on logging of inputs and outputs of rate_compare().
 * Do not define for unit tests to avoid target dependencies of audio_log. */
#if !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
/* #define RATE_COMPARE_DEBUG */
#endif /* unit tests */

#ifdef RATE_COMPARE_DEBUG
#include "audio_log/audio_log.h"
static int rcco_serial;
#define RCCO_DBG_LOG(PLACE,A,B,C,D) L2_DBG_MSG5("rate_compare,%d," PLACE ",%d,%d,%d,%d",++rcco_serial,(A),(B),(C),(D))
#else
#define RCCO_DBG_LOG(P,A,B,C,D) do{}while(0)
#endif

/****************************************************************************
 * Private Types And Constants
 */

/* Some constants used to calculate the maximum input values
 * for which various expressions are valid i.e. don't overrun.
 * See also comments in rate_compare and rate_deviation.
 * These are all defined using #define because of the use
 * of floating point expressions.
 */

/* Sample rates are represented in multiples of 25Hz. */
#define SAMPLE_RATE_SCALE               (25)

#define MICROSECONDS_IN_1_25TH_SECOND   (RATE_MICROSECONDS_PER_SECOND / SAMPLE_RATE_SCALE)

/* Longest measurement period in seconds, i.e. 1/4 second */
#define MAX_MEASUREMENT_PERIOD          (0.25)

/* Lowest supported sample rate in Hz */
#define MIN_SAMPLE_RATE                 (8000)

/* Highest supported sample rate in Hz */
#define MAX_SAMPLE_RATE                 (192000)

/* Lowest supported sample rate in multiples of 25Hz */
#define MIN_SAMPLE_RATE_SCALED          (MIN_SAMPLE_RATE/SAMPLE_RATE_SCALE)

/* Highest supported sample rate in multiples of 25Hz */
#define MAX_SAMPLE_RATE_SCALED          (MAX_SAMPLE_RATE/SAMPLE_RATE_SCALE)

/* Highest number of samples */
#define MAX_NUM_SAMPLES                 (MAX_SAMPLE_RATE * MAX_MEASUREMENT_PERIOD)

/* Longest measurement interval in microseconds */
#define MAX_DELTA_USEC                  (RATE_MICROSECONDS_PER_SECOND * MAX_MEASUREMENT_PERIOD)

/* Sample rate for SCO measurements */
#define SCO_PSEUDO_SAMPLE_RATE          (8000)

/* Sample period for SCO measurements, in usec */
#define SCO_PSEUDO_SAMPLE_PERIOD        (RATE_MICROSECONDS_PER_SECOND / SCO_PSEUDO_SAMPLE_RATE)

/*
 * This is the margin of the numerator or denominator in rate_deviation,
 * from values corresponding to 1/4 second at 192000Hz sample rate
 * and the limit of 2^31-1. This quotient is the same for numerator
 * and denominator. I.e.:
 *   ( (2^31-1) / ( (192000 / 25) * 250000 ) ~ 1.1185
 *   ( (2^31-1) / (
 */
#define DEVIATION_RANGE_MARGIN          (1.1185)

#define DEVIATION_DELTA_USEC_MAX    ((TIME)(MAX_DELTA_USEC * DEVIATION_RANGE_MARGIN))
#define DEVIATION_NUM_SAMPLES_MAX   ((unsigned)(MAX_NUM_SAMPLES * DEVIATION_RANGE_MARGIN))

/* Cap the compare count somewhere (that's beyond any expected startup) */
#define MAX_COMPARE_COUNT               (1000)

#if RATE_COMPARE_ALG==3

/** Internal limits of rate_compare_calc_offset (RCCO) */
#define RCCO_MAX_FB_DELTA_USEC  ((1<<16)-1)
#define RCCO_MAX_FB_NUM_SAMPLES ((1<<14)-1)
#define RCCO_MAX_ABS_T_DIFF     ((1<<16)-1)

#endif /* RATE_COMPARE_ALG */

/****************************************************************************
 * Private Function Declarations
 */

static void rate_compare_update_rates(RATE_COMPARE* rcmp);

#if RATE_COMPARE_ALG==3
/* This function is internal but exposed for testing */
#if !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
static
#endif /* Not a test build */
bool rate_compare_calc_offset(TIME t_ref, TIME t_fb,
                              const RATE_MEASUREMENT* fb,
                              int* c_int_p, int* c_frac_p);
#endif /* RATE_CONTROL_ALG==3 */

/****************************************************************************
 * Private Function Implementations
 */
static void rate_compare_update_rates(RATE_COMPARE* rcmp)
{
    if (rate_reduce_sample_rates(rcmp->fb_sample_rate_div25, rcmp->ref_sample_rate_div25,
                                 &rcmp->fb_reduced_rate, &rcmp->ref_reduced_rate))
    {
        rcmp->max_ref_sample_count = MAXINT / rcmp->ref_reduced_rate;
    }
    else
    {
        rcmp->fb_reduced_rate = 1;
        rcmp->ref_reduced_rate = 1;
        rcmp->max_ref_sample_count = MAXINT;
    }
}

/****************************************************************************
 * Public Function Implementations
 */
void rate_compare_start(RATE_COMPARE* rcmp)
{
    patch_fn_shared(rate_lib);

    rcmp->fb_sample_count_error = 0;
    rcmp->fb_sample_count_residual = 0;
    rcmp->count = 0;
    rate_compare_update_rates(rcmp);
}

void rate_compare_set_ref_sample_rate(RATE_COMPARE* rcmp, unsigned sample_rate)
{
    patch_fn_shared(rate_lib);

    if (rcmp == NULL)
    {
        return;
    }

    unsigned reduced_rate = sample_rate;
    if (reduced_rate >= MIN_SAMPLE_RATE)
    {
        reduced_rate = rate_sample_rate_div_25(reduced_rate);
    }
    if (rcmp->ref_sample_rate_div25 != reduced_rate)
    {
        rcmp->ref_sample_rate_div25 = reduced_rate;
        rate_compare_update_rates(rcmp);
    }
}

void rate_compare_set_fb_sample_rate(RATE_COMPARE* rcmp, unsigned sample_rate)
{
    patch_fn_shared(rate_lib);

    if (rcmp == NULL)
    {
        return;
    }

    unsigned reduced_rate = sample_rate;
    if (reduced_rate >= MIN_SAMPLE_RATE)
    {
        reduced_rate = rate_sample_rate_div_25(reduced_rate);
    }
    if (rcmp->fb_sample_rate_div25 != reduced_rate)
    {
        rcmp->fb_sample_rate_div25 = reduced_rate;
        rate_compare_update_rates(rcmp);
    }
}

#if RATE_COMPARE_ALG==3
/**
 * \brief Convert the time difference t_ref-t_fb into
 * a sample count, using the sample rate implied by the
 * measurement fb. Return the result c as a signed integer part
 * floor(c) and a fractional part between 0 and 1 (c-floor(c)).
 * The widths of internal calculations are chosen to support
 * measurements up to every 50ms and sample rates up to 192k.
 *
 * \param t_ref Timestamp
 * \param t_fb Timestamp
 * \param fb Implied sample rate measurement
 * \param c_int_p (out) pointer to signed integer part of result
 * \param c_frac_p (out) pointer to unsigned fractional part of result

 * \return False if arguments were invalid or out of supported range
 */
/* This function is internal but exposed for testing */
#if !defined(UNIT_TEST_BUILD) && !defined(DESKTOP_TEST_BUILD)
static
#endif /* Not a test build */
bool rate_compare_calc_offset(TIME t_ref, TIME t_fb,
                              const RATE_MEASUREMENT* fb,
                              int* c_int_p, int* c_frac_p)
{
    if ((c_int_p == NULL) || (c_frac_p == NULL) || (fb == NULL))
    {
        return FALSE;
    }

    /*
     *       t_ref - t_fb
     *  ----------------------------------- =: c_int + c_frac
     *  (fb->delta_usec /  fb->num_samples)
     *
     *  where c_int is integer and 0 <= c_frac < 1.
     *
     *  the numerator could be as large as 50ms and pos or neg,
     *  i.e. takes 16bit + sign
     *  num_samples <= 192000 * 50ms ~ 9600 < 2^14
     *  fb->delta_usec <= 50ms < 2^16
     *  so in:
     *
     *  (t_ref - t_fb) * fb->num_samples
     *  --------------------------------
     *            fb->delta_usec
     *
     *  the numerator fits in 31+sign bits i.e. in an int32.
     */
    int32 t_diff = (int32)t_ref - (int32)t_fb;

    if ((fb->delta_usec > RCCO_MAX_FB_DELTA_USEC)
        || (fb->delta_usec == 0)
        || (fb->num_samples > RCCO_MAX_FB_NUM_SAMPLES)
        || (fb->num_samples == 0)
        || (pl_abs_i32(t_diff) > RCCO_MAX_ABS_T_DIFF))
    {
        return FALSE;
    }

    int32 num = t_diff * (int)fb->num_samples;

    int c_int, c_frac;
    int32 res;

    if (t_diff >= 0)
    {
        /* TODO this is a div/mod pair, could use an intrinsic */
        c_int = (int)(num / (int32)fb->delta_usec);
        res = num - c_int * (int)fb->delta_usec;
    }
    else
    {
        num = - num;
        /* This is not a regular div/mod pair */
        c_int = (int)((num  + (int)fb->delta_usec - 1) / (int32)fb->delta_usec);
        res = c_int * (int)fb->delta_usec - num;
        c_int = - c_int;
    }

    PL_ASSERT((res >= 0) && (res < (int32)fb->delta_usec));
    c_frac = rate_fractional_divide((int)res, (int)fb->delta_usec);

    *c_int_p = c_int;
    *c_frac_p = c_frac;

    return TRUE;
}
#endif /* RATE_COMPARE_ALG==3 */

RATE_COMPARE_RESULT rate_compare(RATE_COMPARE* rcmp, const RATE_MEASUREMENT* ref,
                                 const RATE_MEASUREMENT* fb, int* deviation)
{
    RATE_COMPARE_RESULT result = RATE_COMPARE_VALID;

    patch_fn_shared(rate_lib);

    if (rcmp == NULL || ref == NULL || fb == NULL || deviation == NULL)
    {
        return RATE_COMPARE_FAILED;
    }

    if (ref->num_samples >= rcmp->max_ref_sample_count)
    {
        /* TBC: need version of algorithm for larger values? */
        return RATE_COMPARE_FAILED;
    }

    if (ref->restarted || fb->restarted)
    {
        rcmp->count = 0;
    }
    if (ref->unreliable || fb->unreliable)
    {
        result |= RATE_COMPARE_UNRELIABLE;
    }

    /* See CS-00401459-DD 0.4 sec 8.1 */

    if (rcmp->count == 0)
    {
        /* Starting */
        rcmp->fb_exp_samples_remainder = 0;

        int c0_int, c0_frac;
        /* Both these subtractions can wrap. That's OK because
         * the direction in time is known */
        TIME ref_start_timestamp = ref->last_timestamp - ref->delta_usec;
        TIME fb_start_timestamp = fb->last_timestamp - fb->delta_usec;

        if (! rate_compare_calc_offset(ref_start_timestamp, fb_start_timestamp, fb, &c0_int, &c0_frac))
        {
            /* unable to start, wait for next TBC */
            RCCO_DBG_LOG("-1", ref_start_timestamp, fb_start_timestamp, fb->num_samples, fb->delta_usec);
            return FALSE;
        }
        rcmp->offset_samples_frac = c0_frac;
        rcmp->fb_acc = c0_int;
        result |= RATE_COMPARE_START;

        RCCO_DBG_LOG("0", ref_start_timestamp, fb_start_timestamp, rcmp->fb_acc, rcmp->offset_samples_frac);
    }

    RCCO_DBG_LOG("1", ref->restarted, ref->num_samples, ref->delta_usec, ref->last_timestamp);
    RCCO_DBG_LOG("2", fb->restarted, fb->num_samples, fb->delta_usec, fb->last_timestamp);

    /* Update expected feedback sample count by converting reference samples
     * to feedback rate, with remainder saved in fb_exp_samples_numerator
     */
    int next_numerator = rcmp->fb_exp_samples_remainder
                         + (int)ref->num_samples * (int)rcmp->fb_reduced_rate;
    /* TODO This is a div/mod pair, could use an intrinsic */
    int add_exp_samples = next_numerator / (int)rcmp->ref_reduced_rate;
    int new_remainder = next_numerator
                        - add_exp_samples * (int)rcmp->ref_reduced_rate;

    /** b_p */
    rcmp->fb_exp_samples_remainder = new_remainder;
    /** Update integer accumulator */
    rcmp->fb_acc += add_exp_samples - (int)fb->num_samples;

    /** c_p */
    int cp_int, cp_frac;
    if (! rate_compare_calc_offset(ref->last_timestamp, fb->last_timestamp, fb, &cp_int, &cp_frac))
    {
        /* unable to use, restart? TBC */
        RCCO_DBG_LOG("-2", ref->last_timestamp, fb->last_timestamp, fb->num_samples, fb->delta_usec);
        return RATE_COMPARE_FAILED;
    }

    /* Calculate E_p in a double word, Q(DAWTH-1).(DAWTH).
     * Use int48 which in K32 is actually an int64. */
    int48 ep;

    /* Accumulating integer part */
    ep = (int48)rcmp->fb_acc << DAWTH;

    /* add (c_0 - floor(c_0)) */
    ep += (int48)rcmp->offset_samples_frac << 1;

    /* subtract c_p */
    ep -= (int48)cp_int << DAWTH;
    ep -= (int48)cp_frac << 1;

    /* add b_p / g_s^r */
    int bp_frac = rate_fractional_divide(new_remainder, rcmp->ref_reduced_rate);
    ep += (int48)bp_frac << 1;

    /* Split E_p into integer (floor) and fractional (non-negative) */
    rcmp->fb_sample_count_error = (int)(ep >> DAWTH);
    /* Extract lower word, then shift right 1 bit without sign extension */
    rcmp->fb_sample_count_residual = (int)((unsigned)ep >> 1);


    /* Calculate a relative deviation as if the error
     * had occurred in a single measurement period,
     * i.e. relative to fb->num_samples.
     * The caller should usually scale this down.
     * Ignoring this accumulation, the deviation is approx.
     * (reference rate / feedback rate), or
     * (feedback sample period / reference sample period).
     */
    int dev;
    /* clamp adjustments */
    int err_limit = frac_mult(fb->num_samples, FRACTIONAL(0.05));
    if (rcmp->fb_sample_count_error >= err_limit)
    {
        dev = FRACTIONAL(0.05);
        result |= RATE_COMPARE_UNRELIABLE;
    }
    else if (rcmp->fb_sample_count_error <= - err_limit)
    {
        dev = - FRACTIONAL(0.05);
        result |= RATE_COMPARE_UNRELIABLE;
    }
    else
    {
        /* Signed fractional divide for the error integer part */
        unsigned mag_err;
        bool neg_err = (rcmp->fb_sample_count_error < 0);
        mag_err = neg_err ? (unsigned)(-rcmp->fb_sample_count_error)
                          : (unsigned)(rcmp->fb_sample_count_error);

        dev = rate_fractional_divide(mag_err, fb->num_samples);
        if (neg_err)
        {
            dev = - dev;
        }

        /* Add the contribution from the error fractional part */
        dev += (rcmp->fb_sample_count_residual + (fb->num_samples/2))
               / fb->num_samples;
    }
    *deviation = dev;

    if (rcmp->count < MAX_COMPARE_COUNT)
    {
        rcmp->count += 1;
    }

    RCCO_DBG_LOG("3", rcmp->fb_exp_samples_remainder, rcmp->fb_acc, rcmp->fb_sample_count_error, rcmp->fb_sample_count_residual);
    RCCO_DBG_LOG("4", dev, rcmp->count, result, 0);

    return result;
}

bool rate_deviation(unsigned sample_rate_div25, const RATE_MEASUREMENT* fb, int* deviation)
{
    patch_fn_shared(rate_lib);

    if (fb == NULL || deviation == NULL)
    {
        return FALSE;
    }

    *deviation = 0;

    /*
     * 1) General inputs
     *      fb_delta_usec fb_nom_rate/25
     *      ---------------------------
     *      fb_samples * (1e6/25)
     *
     *      rate/25 <= 7680 (192k)
     *      samples <= 48000 (192k, 0.25s)
     *      delta_usec <= 250000 (0.25s)
     *      -> numerator,denominator <= 2^31
     *      rate * samples <= 2^29 (192k)
     *
     *      samples >= 800 (8k, 0.1s)
     *      -> denominator >= 2^25
     *
     * 2) SCO inputs: rate is 8000, time is up to 1s
     *      this can be special-cased since the sample period
     *      is an integer number of microsesconds
     *
     * Use a fractional division to obtain the deviation.
     */

    int32 den, num;

    if (sample_rate_div25 == (SCO_PSEUDO_SAMPLE_RATE/SAMPLE_RATE_SCALE))
    {
        den = (int32)fb->num_samples * SCO_PSEUDO_SAMPLE_PERIOD;
        num = (int32)fb->delta_usec;
    }
    else
    {
        if ((fb->num_samples == 0)
            || (sample_rate_div25 < MIN_SAMPLE_RATE_SCALED)
            || (sample_rate_div25 > MAX_SAMPLE_RATE_SCALED)
            || (fb->num_samples > DEVIATION_NUM_SAMPLES_MAX)
            || (fb->delta_usec > DEVIATION_DELTA_USEC_MAX))
        {
            return FALSE;
        }
        den = (int32)fb->num_samples * MICROSECONDS_IN_1_25TH_SECOND;
        num = (int32)fb->delta_usec * (int)sample_rate_div25;
    }


#if DAWTH==24
    /* Scale inputs before using fractional divide.
     * A maximal shift of 6 bits is expected (range of sample counts)
     */
    static const int max_normalise_shift = 6;
    int scale_den = pl_sign_detect_long(den);
    int scale_num = pl_sign_detect_long(num);
    if (scale_den < scale_num)
    {
        scale_num = scale_den;
    }
    if (scale_num < 0)
    {
        scale_num = 0;
    }
    else if (scale_num > max_normalise_shift)
    {
        scale_num = max_normalise_shift;
    }

    /* Normalize then take the top : i.e. shift left by scale_num,
     * then right by DAWTH. Or shift right by DAWTH - scale_num.
     * (Note that on 24-bit Kalimba, int32 is implemented as int48.)
     */
    scale_num = DAWTH - scale_num;
    int num_s24 = (int)(num >> scale_num);
    int den_s24 = (int)(den >> scale_num);
    num_s24 = num_s24 - den_s24;

    if ((num_s24 > den_s24) || (num_s24 < -den_s24))
    {
        /* Result of division would saturate */
        return FALSE;
    }

    /* pl_fractional_divide only works for non-negative arguments */
    if (num_s24 < 0)
    {
        *deviation = - rate_fractional_divide((unsigned)(-num_s24), (unsigned)den_s24);
    }
    else
    {
        *deviation = - rate_fractional_divide((unsigned)(num_s24), (unsigned)den_s24);
    }
#else /* DAWTH */
    num = num - den;

    if ((num >= den) || (num <= -den))
    {
        /* Result of division would saturate */
        return FALSE;
    }

    /* pl_fractional_divide only works for non-negative arguments */
    if (num < 0)
    {
        *deviation = - rate_fractional_divide(-num, den);
    }
    else
    {
        *deviation = rate_fractional_divide(num, den);
    }
#endif /* DAWTH */

    return TRUE;
}
