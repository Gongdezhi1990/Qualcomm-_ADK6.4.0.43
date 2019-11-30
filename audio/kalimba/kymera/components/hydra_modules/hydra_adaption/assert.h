/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/*
FILE
        assert.h - replacement assert macro when not using a library

DESCRIPTION
        This is here to provide a replacement assert() macro for use
        in on-chip debugging.  The major difference is that we call
        panic().  This has various helpful effects:
        - Interrupts are blocked so foreground code no longer runs.
        - The MAC accelerator is turned off, so buffer state is
          preserved.
        - The processor other than the one which hit the condition is
          also stopped in its tracks.
        - (If on a build with debugging enabled) the watchdog is kicked,
          so the chip won't reset.

       It is not intended that assert() should be a replacement for
       panic(); direct calls to panic() with a meaningful error code
       should be used for tests in production code.
 */

#include "panic/panic.h"

/* avoid confusing test with library assert.h */
#ifndef XAP_ASSERT_H
#define XAP_ASSERT_H

/*
 * Conditional run-time assertion.
 *
 * Calls panic(PANIC_HYDRA_ASSERTION_FAILED) on XAP if assertion fails.
 *
 * #define NDEBUG before inclusion to disable.
 */
#ifdef NDEBUG
#define assert(x) ((void)0)
#else
#define assert(x) ( (x) ? ((void)0) : panic_diatribe(PANIC_HYDRA_ASSERTION_FAILED, __LINE__) )
#endif

#define assert_expr(x) assert(x)

#endif /* XAP_ASSERT_H */
