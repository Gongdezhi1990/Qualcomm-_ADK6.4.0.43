#ifndef PANIC_H
#define PANIC_H

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

#include "panicids.h"

/* Note: must include hydra_types.h before hydra_macros.h to avoid an illegal
 * redefinition of NULL */
#include "hydra/hydra_types.h"
#include "hydra/hydra_macros.h"

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
#if defined (_WIN32) && !defined(KCC_DEPENDENCY_GEN)
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
#if defined (_WIN32) && !defined(KCC_DEPENDENCY_GEN)
/* Tell windows compiler these don't return */
__declspec(noreturn)
#endif
extern void panic_diatribe(panicid deathbed_confession,
                           DIATRIBE_TYPE diatribe) NORETURN;

#ifdef __KALIMBA__
/**
 * For kalcc we need some different magic
 */
#if !defined(__GNUC__)
/*lint -e975 Don't worry about the unrecognised pragma, lint */
#pragma ckf panic f DoesNotReturn
#pragma ckf panic_diatribe f DoesNotReturn
#endif
#endif

/**
 * Entry point of the P1 panic report to P0. This is a P0 function.
 * \param id Panic ID.
 * \param t Timestamp.
 * \param arg Panic argument.
 */
void panic_from_p1(uint16 id, uint32 t, uint32 arg);

/**
 * Entry point of the P0 panic report to P1. This is a P1 function.
 * \param id Panic ID.
 * \param t Timestamp.
 * \param arg Panic argument.
 */
void panic_from_p0(uint16 id, uint32 t, uint32 arg);

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
