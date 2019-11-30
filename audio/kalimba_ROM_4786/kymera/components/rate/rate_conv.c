/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_conv.c
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"

/****************************************************************************
 * Private Constants
 */
enum {
    /* Limits of the range of supported sample rates */
    FS_MIN = 8000,
    FS_MAX = 192000,
    FS_SCALE = 25,

    /* 192000 / 11025 = 2560 / 147, and the latter is irreducible.
     * So 2560 is the smallest upper bound that can be achieved
     * for the supported sample rates.
     */
    FS_REDUCED_MAX = 2560
};

/****************************************************************************
 * Public Function Implementations
 */

/**
 * \brief Calculate the sample period in the RATE_SAMPLE_PERIOD representation
 * for each of the supported sample rates. The result is not guaranteed for
 * any other input.
 * \note This function is typically called as part of configuration, rather
 * than frequently during processing.
 * \param sample_rate Sample rate in Hz
 * \return Sample period in RATE_SAMPLE_PERIOD representation
 */
RATE_SAMPLE_PERIOD rate_sample_rate_to_sample_period(unsigned sample_rate)
{
    /*
     * Basically
     *                   1e6 * 2^(DAWTH-16)
     * result = round(------------------------)
     *                       sample rate
     * However in order to fit the numerator in a signed int,
     * so that the division can be carried out in hardware,
     * reduce numerator and denominator by 50.
     *
     * For sample_rate=11025 the rounding errors are too large so
     * just return the compile-time calculated value in that case.
     */
    if (sample_rate == 11025)
    {
        return RATE_TO_SAMPLE_PERIOD(11025);
    }
    else
    {
        static const int numerator =
                (RATE_MICROSECONDS_PER_SECOND / (2*RATE_SCALE_RATES))
                << RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION;

        /*
         * This is derived from rate_sample_rate_div25 but achieves /50
         * in one step.
         */
#ifdef __GNUC__
        int sample_rate_div50 = sample_rate / 50;
#else
        int sample_rate_div50 = frac_mult(sample_rate, FRACTIONAL(0.02));
#endif

        int result = (numerator + sample_rate_div50/2) / sample_rate_div50;
        return (RATE_SAMPLE_PERIOD) result;
    }
}

TIME rate_samples_to_usec(unsigned num_samples, RATE_SAMPLE_PERIOD sample_period)
{
    /* The calculation within the type RATE_SAMPLE_PERIOD would overrun
     * if the result is > 65535us. Since it's not easy to get access to overflow
     * status in C, check the integer microsecond range in a separate calculation
     * first.
     * Leave a lot of room for truncation of sample_period.
     */
    if (num_samples * (sample_period >> RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION) <= 50000)
    {
        RATE_SAMPLE_PERIOD t = sample_period * num_samples;

        return (t + (1 << (RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION-1)))
               >> RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION;
    }
    else
    {
        RATE_TIME t = ((RATE_TIME)sample_period) * num_samples;

        return (t + (1 << (RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION-1)))
               >> RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION;
    }
}

TIME rate_samples_frac_to_usec(unsigned num_samples_frac,
                               unsigned sample_shift,
                               RATE_SAMPLE_PERIOD sample_period)
{
    patch_fn_shared(rate_lib);

    /* Similar to rate_samples_to_usec, so see also comments there. */

    unsigned res_shift = RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION + sample_shift;

    if (num_samples_frac * (sample_period >> RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION) <= 50000)
    {
        RATE_SAMPLE_PERIOD t = sample_period * num_samples_frac;

        return (t + (1 << (res_shift-1))) >> res_shift;
    }
    else
    {
        RATE_TIME t = ((RATE_TIME)sample_period) * num_samples_frac;

        return (t + (1 << (res_shift-1))) >> res_shift;
    }
}

/* Not fully general, only works for the supported sample rates.
 * The common divisor which this function removes is of the form
 * 25^k * 3^n * 2^m, for 0 <= k, n <= 1.
 */
bool rate_reduce_sample_rates(unsigned fs1, unsigned fs2, unsigned* reduced1, unsigned* reduced2)
{
    patch_fn_shared(rate_lib);

    if (reduced1 == NULL || reduced2 == NULL)
    {
        return FALSE;
    }

    /* Frequent enough to be worth a special case */
    if (fs1 == fs2)
    {
        *reduced1 = 1;
        *reduced2 = 1;
        return TRUE;
    }

    /* Order the pair; it allows some simplifications below. */
    if (fs1 < fs2)
    {
        unsigned f, *p;
        f = fs1; fs1 = fs2; fs2 = f;
        p = reduced1; reduced1 = reduced2; reduced2 = p;
    }
    /* fs1 >= fs2 */

    /* Fail if fs1 < FS_MIN/FS_SCALE or fs2 < FS_MIN/FS_SCALE.
     * Note (fs1 < FS_MIN/FS_SCALE) => (fs2 < FS_MIN/FS_SCALE).
     */
    if (FS_MIN/FS_SCALE > fs2)
    {
        return FALSE;
    }
    /* Fail if fs1 > FS_MAX or fs2 > FS_MAX.
     * Note (fs2 > FS_MAX) => (fs1 > FS_MAX).
     */
    if (fs1 > FS_MAX)
    {
        return FALSE;
    }

    /*
     * Since supported rates range from 8000 to 192000 Hz, one can tell
     * whether the argument contain the factor 25 by their magnitude
     * (because 192000/25 > 8000).
     */
    /* If (fs1 >= FS_MIN) and (fs2 >= FS_MIN), both should be
     * divisible by 25. (If they are not, the result may be wrong;
     * this function does not guarantee to detect unsupported rates.)
     * Note (fs2 >= FS_MIN) => (fs1 >= FS_MIN)
     */
    if (fs2 >= FS_MIN)
    {
        fs1 = rate_sample_rate_div_25(fs1);
        fs2 = rate_sample_rate_div_25(fs2);
    }
    else if (fs1 >= FS_MIN)
    {
        /* One rate is >=8000, the other <8000 */
        return FALSE;
    }

    /*
     * Reduce by common power of two
     */
    /* If fs_i = 2^n * k, for an odd k, then pot_i = 2^(n+1)-1 */
    unsigned pot1 = fs1 ^ (fs1 - 1);
    unsigned pot2 = fs2 ^ (fs2 - 1);
    unsigned potgcd = pot1 & pot2;
    unsigned potm = MAX_SIGNDET_BITS - pl_sign_detect(potgcd);
    fs1 >>= potm;
    fs2 >>= potm;

    /* Try reducing by 3.
     *
     * Cast to int before divisions to get the compiler to emit hardware divisions.
     *
     * Assembly code could save time here:
     * since the power of 2 in the prime decomposition is the
     * same before/after dividing by 3, one can perform the reduction
     * by power of 2 in parallel with waiting for div/mod 3,
     * then reduce the result of div 3.
     */
    if ((((int)fs1 % 3) == 0) && (((int)fs2 % 3) == 0))
    {
        fs1 = (unsigned)((int)fs1 / 3);
        fs2 = (unsigned)((int)fs2 / 3);
    }

#if 0
    /* This is not needed to get the results
     * under 2560, so skip it.
     */
    if ((fs1 % fs2) == 0)
    {
        fs1 /= fs2;
        fs2 = 1;
    }
#endif
    /* This will not happen with the supported rates. */
    if (fs1 > FS_REDUCED_MAX)
    {
        return FALSE;
    }

    *reduced1 = fs1;
    *reduced2 = fs2;
    return TRUE;
}

