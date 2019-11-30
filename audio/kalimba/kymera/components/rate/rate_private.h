/****************************************************************************
 * Copyright 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file rate_private.h
 * \ingroup rate_lib
 *
 */

#ifndef RATE_RATE_PRIVATE_H
#define RATE_RATE_PRIVATE_H

/* This module has its audio log format strings checked in gcc warnings builds */
#define AUDIO_LOG_CHECK_FORMATS

/* Define this to trace some calculation results */
/* #define RATE_TRACE */

#include "rate.h"
#include "rate_platform.h"
#ifdef INSTALL_METADATA
#include "buffer/buffer_metadata.h"
#endif /* INSTALL_METADATA */
#include "pl_fractional.h"
#include "pl_intrinsics.h"
#include "pl_assert.h"
#include "patch/patch.h"
#ifdef RATE_TRACE
#include "audio_log/audio_log.h"
#endif /* RATE_TRACE */
#if defined(UNIT_TEST_BUILD)  ||defined(DESKTOP_TEST_BUILD)
/* Declarations of internals exposed to unit tests; include here
 * so that any differences cause errors
 */
#include "rate_test.h"
#endif

/****************************************************************
 * Module private macros
 */
#ifdef RATE_TRACE
#define RATE_MSG1(F,A)          L3_DBG_MSG1(F,A)
#define RATE_MSG2(F,A,B)        L3_DBG_MSG2(F,A,B)
#define RATE_MSG3(F,A,B,C)      L3_DBG_MSG3(F,A,B,C)
#define RATE_MSG4(F,A,B,C,D)    L3_DBG_MSG4(F,A,B,C,D)
#define RATE_MSG5(F,A,B,C,D,E)  L3_DBG_MSG5(F,A,B,C,D,E)
#else /* RATE_TRACE */
#define RATE_MSG1(F,A)          do{}while(0)
#define RATE_MSG2(F,A,B)        do{}while(0)
#define RATE_MSG3(F,A,B,C)      do{}while(0)
#define RATE_MSG4(F,A,B,C,D)    do{}while(0)
#define RATE_MSG5(F,A,B,C,D,E)  do{}while(0)
#endif /* RATE_TRACE */

/* Rounding division of 1e6 microseconds in Qu16.m by sample rate; in order
 * to fit in the provided bits, divide numerator and denominator by 25.
 *
 * Define as a macro so that with a literal argument it can be calculated at
 * compile time.
 */
#define RATE_TO_SAMPLE_PERIOD(R) \
    (RATE_SAMPLE_PERIOD) \
    ( ( ( (RATE_TIME)(RATE_MICROSECONDS_PER_SECOND / RATE_SCALE_RATES) \
          << RATE_SAMPLE_PERIOD_EXTRA_RESOLUTION ) \
        + ( (R) / (2*RATE_SCALE_RATES) ) ) \
      / ((R) / RATE_SCALE_RATES) )


/****************************************************************
 * Module private types and enums
 */

enum
{
    RATE_DEFAULT_SAMPLE_RATE        = 48000,
    RATE_SCALE_RATES                = 25
};


#endif /* RATE_RATE_PRIVATE_H */
