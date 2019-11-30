/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_platform.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_PLATFORM_H
#define RATE_RATE_PLATFORM_H

#include "rate_types.h"

/****************************************************************************
 * Arithmetic Definitions
 *
 * Depending on the processor, saturating additions may be accessible
 * using special instructions, or by setting mode flags,
 * or emulated.
 *
 * For an explanation why this isn't somewhere central like
 * components/platform, see B-245019
 */

#if defined(__KCC__)
/*
 * On Kalimba, saturating addition is available by
 * setting bit 0 of the memory mapped register
 * ARITHMETIC_MODE, then using the usual + and -
 * operators. There is no special compiler support
 * for this. The ARITHMETIC_MODE register's state
 * is saved during interrupts and generally functions
 * expect ARITHMETIC_MODE to be 0 on entry.
 */

#include "io_map.h"

/** Turn on saturating behaviour for additions,
 * optionally (i.e. as CPU permits) save current
 * state in local variable S
 *
 * \note the more straightforward looking
        unsigned S = ARITHMETIC_MODE;
        ARITHMETIC_MODE |= 1;
 * reads ARITHMETIC_MODE twice because it is a volatile, e.g.:
        r6 = Null + -8172;
        r5 = M[r6 + Null];
        r7 = M[r6 + Null];
        r7 = r7 OR 0x1;
        M[r6 + Null] = r7;
 */
#define RATE_ENABLE_SATURATING(S) \
        unsigned S; \
        ARITHMETIC_MODE = (S = ARITHMETIC_MODE) | 1

/** Restore/turn off saturating behaviour */
#define RATE_RESTORE_SATURATING(S) \
        ARITHMETIC_MODE = (S)

/** Saturating addition, this has to be used
 * between RATE_ENABLE_SATURATING and RATE_RESTORE_SATURATING */
#define RATE_ADDS(X,Y) ((X) + (Y))

/** Saturating subtraction, this has to be used
 * between RATE_ENABLE_SATURATING and RATE_RESTORE_SATURATING */
#define RATE_SUBS(X,Y) ((X) - (Y))

#else /* !defined __KCC__ */

#define RATE_PLATFORM_GENERIC_ADDS_SUBS

/* There is no saturating mode.
 * However keep track of it for testing. */
#define RATE_ENABLE_SATURATING(S)   \
    bool S; \
    rate_saturating_mode_generic = (S = rate_saturating_mode_generic) | 1

#define RATE_RESTORE_SATURATING(S)  \
    rate_saturating_mode_generic = (S)

#define RATE_ADDS(X,Y) rate_adds_generic((X),(Y))
#define RATE_SUBS(X,Y) rate_subs_generic((X),(Y))

int rate_adds_generic(int x, int y);
int rate_subs_generic(int x, int y);

extern volatile bool rate_saturating_mode_generic;

#endif /* processor */

/****************************************************************************
 * Fractional division
 */

/** TODO Define and use a local fractional division until pl_fractional_divide
 * is fixed, in places where results can be greater than 0.5. (I.e. calls which
 * produce a deviation don't need this.) */
#define TODO_B_247162

#ifdef TODO_B_247162
int rate_fractional_divide(int num, int den);
#else
#define rate_fractional_divide(N,D) pl_fractional_divide(N,D)
#endif


#endif /* RATE_RATE_PLATFORM_H */
