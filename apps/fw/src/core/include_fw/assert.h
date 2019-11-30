/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
    \file
    Replacement assert macro when not using a library.
*/
#include "hydra/hydra_types.h"
#include "panic/panic.h"
#include "hydra_log/hydra_log.h"

/* avoid confusing test with library assert.h */
#ifndef XAP_ASSERT_H
#define XAP_ASSERT_H

#ifdef NDEBUG

#define assert(x) \
    ((void)0)

#define assert_expr(x) \
    ((void)0)

#else /* NDEBUG */

/*
 * Permit use of assert with constant conditions.
 */
/*lint -emacro(774,assert) */
/*lint -emacro(774,assert_expr) */
/*lint -emacro({774},assert) */
/*lint -emacro({774},assert_expr) */
#if defined(_lint) || defined(__KALIMBA__) /* life is too short to work out how to tell lint about __BASE_FILE__ */
#define __BASE_FILE__ __FILE__
#endif
#if defined(INSTALL_VERBOSE_ASSERT_LOGGING) && defined(INSTALL_HYDRA_LOG)
/*
 * verbose assert variant
 */

#ifndef __KALIMBA__/**
 * Declare a single copy of the source file name.
 * This is stored in debug_string section for assert logging.
 *
 * \port __BASE_FILE__ is defined automatically by GCC but not all others.
 * ( But then all debug logging depends on GCC at present. )
 */
HYDRA_LOG_STRING(assert_file, __BASE_FILE__);
#else
/* The Kalimba compiler objects if we declare but don't use assert_file */
#define assert_file __BASE_FILE__
#endif

#define assert(assertion) \
    do { \
        if (!(assertion)) { \
            HYDRA_LOG_STRING(assert_text, #assertion); \
            panic_on_assert_verbose(assert_file, __LINE__, assert_text); \
        } \
    } while (0)

#define assert_expr(assertion) \
    ((assertion)? (void)0 : panic_on_assert_brief(assert_file, __LINE__))

#elif defined(INSTALL_BRIEF_ASSERT_LOGGING) && defined(INSTALL_HYDRA_LOG)
/*
 * brief assert variant
 */

#ifndef __KALIMBA__
/**
 * Declare a single copy of the source file name.
 * This is stored in debug_string section for assert logging.
 *
 * \port __BASE_FILE__ is defined automatically by GCC but not all others.
 * ( But then all debug logging depends on GCC at present. )
 */
HYDRA_LOG_STRING(assert_file, __BASE_FILE__);
#else
/* The Kalimba compiler objects if we declare but don't use assert_file */
#define assert_file __BASE_FILE__
#endif

#define assert(assertion) \
    ((assertion)? (void)0 : panic_on_assert_brief(assert_file, __LINE__))

#define assert_expr(assertion) \
    assert(assertion)

#elif defined(INSTALL_VERY_BRIEF_ASSERT_LOGGING) && defined(INSTALL_HYDRA_LOG)
/*
 * very brief assert variant
 */

#define assert(assertion) \
    ((assertion)? (void)0 : panic_on_assert_very_brief(__LINE__))

#define assert_expr(assertion) \
    assert(assertion)

#else /* !defined(INSTALL_?_ASSERT_LOGGING) */
/*
 * silent (default) assert variant
 */
#define assert(assertion) \
    ((assertion)? ((void)0) : panic_on_assert(__LINE__))

#define assert_expr(assertion) \
    assert(assertion)

#endif /* !defined(INSTALL_?_ASSERT_LOGGING) */
#endif /* NDEBUG */

#endif /* XAP_ASSERT_H */
