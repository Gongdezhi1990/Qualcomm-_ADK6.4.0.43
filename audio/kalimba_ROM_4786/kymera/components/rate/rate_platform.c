/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_platform.h
 * \ingroup rate_lib
 *
 */

#include "rate_private.h"

#ifdef RATE_PLATFORM_GENERIC_ADDS_SUBS

/* Global variable used to check that RATE_ADDS and RATE_SUBS are
 * called between RATE_ENABLE_SATURATING and RATE_RESTORE_SATURATING.
 * \note This is not saved/restored on interrupts, the gcc environment
 *       does not emulate interrupts anyway.
 */
volatile bool rate_saturating_mode_generic = FALSE;

/* In the kymera build system, MININT is set on the compiler commandline
 * or via #define in a pre-include file. The value is 0x800000 or 0x80000000.
 *
 * This is not a good format for arithmetic use; -8388608 or -2147483648
 * are preferable.
 */

#if defined(__KCC__) && (DAWTH==24)
/* Compiler's int is 24 bits, avoid checking whether MININT is 0x80000000 */

STATIC_ASSERT(MININT==0x800000, MININT_IS_0x800000);
#define RATE_MININT (-8388608)

#else /* > 24-bit compiler */
/* Compiler's int is 32 bit or larger */

STATIC_ASSERT(MININT==0x800000 || MININT==0x80000000, MININT_IS_0x800000_OR_0x80000000);
#define RATE_MININT ((MININT==0x800000) ? -8388608 : -2147483648)

#endif /* > 24-bit compiler */

/**
 * \brief Saturating addition
 * \note This implementation does not rely on the host
 * processor's behaviour when addition overflows.
 * It depends on that MAXINT and MININT are set
 * for the emulated CPU, not the host.
 * Performance is not a concern.
 */
int rate_adds_generic(int x, int y)
{
    int z;

    PL_ASSERT(rate_saturating_mode_generic);

    if (x >= 0)
    {
        if (y > (MAXINT - x))
        {
            z = MAXINT;
        }
        else
        {
            z = x + y;
        }
    }
    else
    {
        if (y < (RATE_MININT - x))
        {
            z = RATE_MININT;
        }
        else
        {
            z = x + y;
        }
    }
    return z;
}

/**
 * \brief Saturating subtraction
 * \note This implementation does not rely on the host
 * processor's behaviour when addition overflows.
 * It depends on that MAXINT and MININT are set
 * for the emulated CPU, not the host.
 * Performance is not a concern.
 */
int rate_subs_generic(int x, int y)
{
    int z;

    PL_ASSERT(rate_saturating_mode_generic);

    if (x >= 0)
    {
        if (y < -(MAXINT - x))
        {
            z = MAXINT;
        }
        else
        {
            z = x - y;
        }
    }
    else
    {
        if (y > -(RATE_MININT - x))
        {
            z = RATE_MININT;
        }
        else
        {
            z = x - y;
        }
    }
    return z;
}


#endif /* RATE_PLATFORM_GENERIC_ADDS_SUBS */

#ifdef TODO_B_247162
int rate_fractional_divide(int num, int den)
{
    PL_ASSERT((num >= 0) && (den > 0));
    if (num >= den)
    {
        return MAXINT;
    }
    else
    {
        return (int)((((int48)num << (DAWTH-1)) + (den >> 1)) / den);
    }
}
#endif /* TODO_B_247162 */

