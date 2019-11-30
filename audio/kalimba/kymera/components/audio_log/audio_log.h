/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_log.h
 * \ingroup AUDIO_LOG
 *
 * Audio log (interface)
 *
 */
#ifndef AUDIO_LOG_H
#define AUDIO_LOG_H

/**
 * \section intro Introduction
 * \defgroup AUDIO_LOG Audio log (interface)
 *
 * This module implements a set of "printf" like macros that can be used to
 * log interesting events for debugging purposes. The interface is intended
 * to be efficient in embedded "firmware" targets. For convenience an
 * implementation is also provided for host/unit test "software" builds.
 *
 * The main macros of interest are:-
 *
 * - L#_DBG_MSG(fmt)
 * - L#_DBG_MSG1(fmt, a1)
 * - L#_DBG_MSG2(fmt, a1, a2)
 * - L#_DBG_MSG3(fmt, a1, a2, a3)
 * - L#_DBG_MSG4(fmt, a1, a2, a3, a4)
 *
 * Where the # represents verbosity level (0 - 5), and the suffix must match
 * the nr or args.
 *
 * \section apology Apology
 *
 * There has to be one macro for each number of arguments cos variadic
 * macros are not supported in ansi-c.
 *
 * \section config Configuration
 *
 * INSTALL_AUDIO_LOG must be defined else all macros will have an empty
 * definition.
 *
 * \section debug_intro Example use
 *
 \verbatim

    *
    * declare debug string (not linked)
    *
    AUDIO_LOG_STRING(event_name, "interesting");
    uint16 count = 123;

    *
    * log away.
    *
    L0_DBG_MSG("something bad happened");
    L2_DBG_MSG2("something %s has happened %d times", event_name, count);
    L4_DBG_MSG1("something very trivial has happened %d times", count);

 \endverbatim
 *
 * \internal
 *
 * \section audio_log_impl Implementation
 *
 * On embedded targets the event logging functions store the event format
 * string address and any parameters into a circular buffer in conventional
 * memory and update a pointer variable. The log buffer can then be polled
 * decoded, and displayed by host script via SPI.
 *
 * To avoid actually using any memory space on the target system for the
 * event format strings and debug strings, they are declared in a
 * dedicated data segment (see note). This segment is _not linked_
 * but _is available_ to a host side script to lookup the format and any string
 * arguments.
 *
 * For example the host side script xap2emu.py/log() uses the .elf file
 * to interpret the circular buffer content and produce a terminal window
 * style output of the strings.
 *
 * Note: This trick requires a toolchain (e.g. binutils) that can be directed
 * to place strings in a special segment.
 *
 * On non-embeded targets the log is output directly to stdout.
 */

/*****************************************************************************
 * Interface
 ****************************************************************************/

/**
 * Audio Log Verbosity Levels.
 */
typedef enum
{
    /**
     * errors
     *
     * Likely only used by panic, assert etc.
     *
     * Do not abuse this log level. Logs at this level can not be switched
     * off dynamically (indeed the level comparison is optimised out).
     */
    AUDIO_LOG_ERROR = 0,
    /**
     * warnings
     *
     * The unusual, the unexpected but not fatal.
     */
    AUDIO_LOG_WARN = 1,
    /**
     * major, but normal system level events
     */
    AUDIO_LOG_NORMAL = 2,
    /**
     * minor system level events, major module level events
     */
    AUDIO_LOG_VERBOSE = 3,
    /**
     * minor module level events
     */
    AUDIO_LOG_V_VERBOSE = 4,
    /**
     * minutiae
     */
    AUDIO_LOG_V_V_VERBOSE = 5,
    /**
     * default/startup log level.
     *
     * The level that a system boots into.
     */
    AUDIO_LOG_DEFAULT = AUDIO_LOG_NORMAL
} audio_log_level;

/**
 * Set the current log verbosity threshold.
 *
 * Only events at, or lower than this level will be logged.
 */
#define audio_log_level_set(level) AUDIO_LOG_SET_LEVEL_IMPL(level)

/**
 * Initialise the audio log
 */

#define audio_log_init() AUDIO_LOG_INIT_IMPL()

/*
 * Logging macros.
 *
 * These macro names have not been changed to the AUDIO_LOG namespace
 * because:-
 * - They are weird enough that they are unlikely to collide with anyone else.
 * - There is a separate bug to change them all "manana"!
 */

/*
 * Log an Error.
 *
 * Do not abuse this error level. They can not be disabled at run time.
 */
#define L0_DBG_MSG(x)                 AUDIO_ALWAYS_LOG0(x)
#define L0_DBG_MSG1(x, a)             AUDIO_ALWAYS_LOG1(x, a)
#define L0_DBG_MSG2(x, a, b)          AUDIO_ALWAYS_LOG2(x, a, b)
#define L0_DBG_MSG3(x, a, b, c)       AUDIO_ALWAYS_LOG3(x, a, b, c)
#define L0_DBG_MSG4(x, a, b, c, d)    AUDIO_ALWAYS_LOG4(x, a, b, c, d)
#define L0_DBG_MSG5(x, a, b, c, d, e) AUDIO_ALWAYS_LOG5(x, a, b, c, d, e)

/*
 * Log a warning.
 */
#ifdef INSTALL_UNCONDITIONAL_LOG_LEVEL_1

#define L1_DBG_MSG(x)                 AUDIO_ALWAYS_LOG0(x)
#define L1_DBG_MSG1(x, a)             AUDIO_ALWAYS_LOG1(x, a)
#define L1_DBG_MSG2(x, a, b)          AUDIO_ALWAYS_LOG2(x, a, b)
#define L1_DBG_MSG3(x, a, b, c)       AUDIO_ALWAYS_LOG3(x, a, b, c)
#define L1_DBG_MSG4(x, a, b, c, d)    AUDIO_ALWAYS_LOG4(x, a, b, c, d)
#define L1_DBG_MSG5(x, a, b, c, d, e) AUDIO_ALWAYS_LOG5(x, a, b, c, d, e)

#else /* INSTALL_UNCONDITIONAL_LOG_LEVEL_1 */

#define L1_DBG_MSG(x)                 AUDIO_LOG0(1, x)
#define L1_DBG_MSG1(x, a)             AUDIO_LOG1(1, x, a)
#define L1_DBG_MSG2(x, a, b)          AUDIO_LOG2(1, x, a, b)
#define L1_DBG_MSG3(x, a, b, c)       AUDIO_LOG3(1, x, a, b, c)
#define L1_DBG_MSG4(x, a, b, c, d)    AUDIO_LOG4(1, x, a, b, c, d)
#define L1_DBG_MSG5(x, a, b, c, d, e) AUDIO_LOG5(1, x, a, b, c, d, e)

#endif /* INSTALL_UNCONDITIONAL_LOG_LEVEL_1 */

/*
 * Log a major event.
 */
#ifdef INSTALL_UNCONDITIONAL_LOG_LEVEL_2

#define L2_DBG_MSG(x)                 AUDIO_ALWAYS_LOG0(x)
#define L2_DBG_MSG1(x, a)             AUDIO_ALWAYS_LOG1(x, a)
#define L2_DBG_MSG2(x, a, b)          AUDIO_ALWAYS_LOG2(x, a, b)
#define L2_DBG_MSG3(x, a, b, c)       AUDIO_ALWAYS_LOG3(x, a, b, c)
#define L2_DBG_MSG4(x, a, b, c, d)    AUDIO_ALWAYS_LOG4(x, a, b, c, d)
#define L2_DBG_MSG5(x, a, b, c, d, e) AUDIO_ALWAYS_LOG5(x, a, b, c, d, e)

#else /* INSTALL_UNCONDITIONAL_LOG_LEVEL_2 */

#define L2_DBG_MSG(x)                 AUDIO_LOG0(2, x)
#define L2_DBG_MSG1(x, a)             AUDIO_LOG1(2, x, a)
#define L2_DBG_MSG2(x, a, b)          AUDIO_LOG2(2, x, a, b)
#define L2_DBG_MSG3(x, a, b, c)       AUDIO_LOG3(2, x, a, b, c)
#define L2_DBG_MSG4(x, a, b, c, d)    AUDIO_LOG4(2, x, a, b, c, d)
#define L2_DBG_MSG5(x, a, b, c, d, e) AUDIO_LOG5(2, x, a, b, c, d, e)

#endif /* INSTALL_UNCONDITIONAL_LOG_LEVEL_2 */

/*
 * Log a minor event.
 */
#define L3_DBG_MSG(x)                 AUDIO_LOG0(3, x)
#define L3_DBG_MSG1(x, a)             AUDIO_LOG1(3, x, a)
#define L3_DBG_MSG2(x, a, b)          AUDIO_LOG2(3, x, a, b)
#define L3_DBG_MSG3(x, a, b, c)       AUDIO_LOG3(3, x, a, b, c)
#define L3_DBG_MSG4(x, a, b, c, d)    AUDIO_LOG4(3, x, a, b, c, d)
#define L3_DBG_MSG5(x, a, b, c, d, e) AUDIO_LOG5(3, x, a, b, c, d, e)

/**
 * Log minor module level event.
 */
#define L4_DBG_MSG(x)                 AUDIO_LOG0(4, x)
#define L4_DBG_MSG1(x, a)             AUDIO_LOG1(4, x, a)
#define L4_DBG_MSG2(x, a, b)          AUDIO_LOG2(4, x, a, b)
#define L4_DBG_MSG3(x, a, b, c)       AUDIO_LOG3(4, x, a, b, c)
#define L4_DBG_MSG4(x, a, b, c, d)    AUDIO_LOG4(4, x, a, b, c, d)
#define L4_DBG_MSG5(x, a, b, c, d, e) AUDIO_LOG5(4, x, a, b, c, d, e)

/**
 * Log minutiae
 */
#define L5_DBG_MSG(x)                 AUDIO_LOG0(5, x)
#define L5_DBG_MSG1(x, a)             AUDIO_LOG1(5, x, a)
#define L5_DBG_MSG2(x, a, b)          AUDIO_LOG2(5, x, a, b)
#define L5_DBG_MSG3(x, a, b, c)       AUDIO_LOG3(5, x, a, b, c)
#define L5_DBG_MSG4(x, a, b, c, d)    AUDIO_LOG4(5, x, a, b, c, d)
#define L5_DBG_MSG5(x, a, b, c, d, e) AUDIO_LOG5(5, x, a, b, c, d, e)

/*
 * Shorthand for normal level message.
 */
#define DBG_MSG(x)                    AUDIO_LOG0(2, x)
#define DBG_MSG1(x, a)                AUDIO_LOG1(2, x, a)
#define DBG_MSG2(x, a, b)             AUDIO_LOG2(2, x, a, b)
#define DBG_MSG3(x, a, b, c)          AUDIO_LOG3(2, x, a, b, c)
#define DBG_MSG4(x, a, b, c, d)       AUDIO_LOG4(2, x, a, b, c, d)
#define DBG_MSG5(x, a, b, c, d, e)    AUDIO_LOG5(2, x, a, b, c, d, e)

/**
 * Declare a string for use in Audio logs.
 *
 * Example use:-
\verbatim

    * declare log string *
    AUDIO_LOG_STRING(event_name, "interesting");

    * log away *
    DBG_MSG2(AUDIO_LOG_NORMAL, "something %s happened", event_name);

\endverbatim
 *
 * \internal
 *
 * Declaration is empty if !INSTALL_AUDIO_LOG,
 * a normal static string for host/unit test builds,
 * and toolchain magic ensures the storage is not actually
 * linked in embedded builds.
 */
#define AUDIO_LOG_STRING(label, text) AUDIO_LOG_STRING_IMPL(label, text)

/*****************************************************************************
 * Deprecated macros.
 *
 * Support for old code that we don't want to fix "just now".
 ****************************************************************************/

/**
 * Is Audio Logging enabled?
 *
 * \deprecated  Use INSTALL_AUDIO_LOG instead.
 *
 * Defined as rubbish to deliberately break and discover any old references
 * that would otherwise just be silently undefined.
 */
#define DEBUG_PRINTF_ENABLED xxx use INSTALL_AUDIO_LOG instead xxx

/**
 * Declare a string for use in a Audio Debug Log.
 *
 * \deprecated  Use AUDIO_LOG_STRING instead. This macro could never have
 * worked for code built with logging disabled.
 */
#define DECLARE_DEBUG_STRING(label) AUDIO_LOG_STRING_DEPRECATED_IMPL(label)

/**
 * Log string cast type.
 *
 * \deprecated  No longer required.  Was for casting strings when used as
 * argument to a Audio Debug Log. The macros now do the cast as necessary.
 */
#define DBG_STR const char *

/*****************************************************************************
 * Select Firmware, Software or Disabled Implementation.
 ****************************************************************************/

#if defined(INSTALL_AUDIO_LOG)
#if defined(DESKTOP_TEST_BUILD) || defined (FORCE_AUDIO_LOG_SOFT)
#define AUDIO_LOG_SOFT
#else /* !defined(DESKTOP_TEST_BUILD) */
#define AUDIO_LOG_FIRM
#endif /* !defined(DESKTOP_TEST_BUILD) */
#else /* !defined(INSTALL_AUDIO_LOG) */
#define AUDIO_LOG_DISABLED
#endif /* !defined(INSTALL_AUDIO_LOG) */

/*****************************************************************************
 * Include Implementation Specific Macros
 *
 * The implementing macros vary not just the source code!
 ****************************************************************************/

#if defined(AUDIO_LOG_FIRM)
#include "audio_log_firm.h"
#elif defined(AUDIO_LOG_SOFT)
#include "audio_log_soft.h"
#elif defined(AUDIO_LOG_DISABLED)
#include "audio_log_disabled.h"
#else
#error "Unknown AUDIO_LOG implementation selected."
#endif

#endif /* AUDIO_LOG_H */
