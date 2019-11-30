/****************************************************************************
 * Copyright (c) 2010 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \defgroup error_handling Error handling
 *
 *
 * \file panic.h
 * \ingroup panic
 *
 * \brief
 * This is the public header file for the panic module. It should
 * be included in all modules that want to panic the Curator.
 *
 * \defgroup panic Panic
 * \ingroup error_handling
 *
 * \brief
 *  Record or report a deathbed confession, and then attempt to recover in a
 *  platform and configuration specific manner.
 */

#ifndef PANIC_H
#define PANIC_H

#include "panicids.h"

#include "hydra/hydra_macros.h"
#include "hydra/hydra_types.h"

/** Macro to allow a pointer to be passed as the diatribe parameter */
#define POINTER_CAST_FOR_DIATRIBE(x)    (DIATRIBE_TYPE)((unsigned)(x) & UINT_MAX)

/**
 * Protest and die
 *
 * Terminate the system. On a non-embedded target this is likely to call
 * exit() with the supplied parameter. On an embedded target it enters an
 * infinite loop, resulting in a reset if the watchdog is enabled.
 *
 * The "deathbed_confession" gives an indication of why the system was
 * terminated. This can usually be obtained either from the environment or
 * via a debugger.
 */
#ifdef _WIN32
/* Tell windows compiler these don't return */
__declspec(noreturn)
#endif
extern void panic(panicid deathbed_confession) NORETURN;

/**
 * Protest volubly and die
 *
 * Like panic(), but stores an extra argument in preserved memory --
 * this can be used to pinpoint the reason for a panic when debugging.
 */
#ifdef _WIN32
/* Tell windows compiler these don't return */
__declspec(noreturn)
#endif
extern void panic_diatribe(panicid deathbed_confession,
                           DIATRIBE_TYPE diatribe) NORETURN;

#if defined(__KALIMBA__) && !defined(_lint)
/**
 * For kalcc we need some different magic
 */
#if !defined(__GNUC__) && !defined(USE_DUMMY_PANIC)
#pragma ckf panic f DoesNotReturn
#pragma ckf panic_diatribe f DoesNotReturn
#endif
#endif

/**
 * Report the last panic ID to the host
 *
 * Report the last panic ID to the host. This function should only
 * be called if the last reset was caused by a call to panic().
 */
extern void panic_report(void);

/*
 * The specialised panic variants below are here to minimise the in-line
 * footprint of the assert() & assert_expr() macros.
 *
 * They are strictly part of assert implementation - but there is no library for
 * that.
 */

/**
 * Panic on invalid assertion without logging.
 */
/*lint -function(exit, panic_on_assert) i.e. NORETURN */
extern void panic_on_assert(
    uint16 line_num
) NORETURN;

/**
 * Panic on invalid assertion logging the caller's PC & src line num.
 *
 * The line num is useful as the compiler tends to consolidate multiple
 * calls within a function.
 */
/*lint -function(exit, panic_on_assert_very_brief) i.e. NORETURN */
extern void panic_on_assert_very_brief(uint16 line_num) NORETURN;

/**
 * Panic on invalid assertion logging the filepath & line num.
 */
/*lint -function(exit, panic_on_assert_brief) i.e. NORETURN  */
extern void panic_on_assert_brief(
    const char *file_path,
    uint16 line_num
) NORETURN;

/**
 * Panic on invalid assertion logging the filepath, line num & assertion text.
 */
/*lint -function(exit, panic_on_assert_verbose) i.e. NORETURN */
extern void panic_on_assert_verbose(
    const char *file_path,
    uint16 line_num,
    const char *assertion_text
) NORETURN;

#endif /* PANIC_H */
