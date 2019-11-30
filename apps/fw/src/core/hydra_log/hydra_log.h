/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra log (interface)
 */
#ifndef HYDRA_LOG_H
#define HYDRA_LOG_H

#include "hydra/hydra_macros.h" /* for GCC_ATTRIBUTE */

/*****************************************************************************
 * Interface
 ****************************************************************************/

/**
 * Hydra Log Verbosity Levels.
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
    HYDRA_LOG_ERROR = 0,
    /**
     * warnings
     *
     * The unusual, the unexpected but not fatal.
     */
    HYDRA_LOG_WARN = 1,
    /**
     * major, but normal system level events
     */
    HYDRA_LOG_NORMAL = 2,
    /**
     * minor system level events, major module level events
     */
    HYDRA_LOG_VERBOSE = 3,
    /**
     * minor module level events
     */
    HYDRA_LOG_V_VERBOSE = 4,
    /**
     * minutiae
     */
    HYDRA_LOG_V_V_VERBOSE = 5,
    /**
     * default/startup log level.
     *
     * The level that a system boots into.
     */
    HYDRA_LOG_DEFAULT = HYDRA_LOG_NORMAL
} hydra_log_level;

/*
 * Logging macros.
 *
 * These macro names have not been changed to the HYDRA_LOG namespace
 * because:-
 * - They are weird enough that they are unlikely to collide with anyone else.
 * - There is a separate bug to change them all "manana"!
 */

/*
 * Log an Error.
 *
 * Do not abuse this error level. They can not be disabled at run time.
 */
#define L0_DBG_MSG(x)                 HYDRA_LOG0(0, x)
#define L0_DBG_MSG1(x, a)             HYDRA_LOG1(0, x, a)
#define L0_DBG_MSG2(x, a, b)          HYDRA_LOG2(0, x, a, b)
#define L0_DBG_MSG3(x, a, b, c)       HYDRA_LOG3(0, x, a, b, c)
#define L0_DBG_MSG4(x, a, b, c, d)    HYDRA_LOG4(0, x, a, b, c, d)

/*
 * Log a warning.
 */
#define L1_DBG_MSG(x)                 HYDRA_LOG0(1, x)
#define L1_DBG_MSG1(x, a)             HYDRA_LOG1(1, x, a)
#define L1_DBG_MSG2(x, a, b)          HYDRA_LOG2(1, x, a, b)
#define L1_DBG_MSG3(x, a, b, c)       HYDRA_LOG3(1, x, a, b, c)
#define L1_DBG_MSG4(x, a, b, c, d)    HYDRA_LOG4(1, x, a, b, c, d)

/*
 * Log a major event.
 */
#define L2_DBG_MSG(x)                 HYDRA_LOG0(2, x)
#define L2_DBG_MSG1(x, a)             HYDRA_LOG1(2, x, a)
#define L2_DBG_MSG2(x, a, b)          HYDRA_LOG2(2, x, a, b)
#define L2_DBG_MSG3(x, a, b, c)       HYDRA_LOG3(2, x, a, b, c)
#define L2_DBG_MSG4(x, a, b, c, d)    HYDRA_LOG4(2, x, a, b, c, d)

/*
 * Log a minor event.
 */
#define L3_DBG_MSG(x)                 HYDRA_LOG0(3, x)
#define L3_DBG_MSG1(x, a)             HYDRA_LOG1(3, x, a)
#define L3_DBG_MSG2(x, a, b)          HYDRA_LOG2(3, x, a, b)
#define L3_DBG_MSG3(x, a, b, c)       HYDRA_LOG3(3, x, a, b, c)
#define L3_DBG_MSG4(x, a, b, c, d)    HYDRA_LOG4(3, x, a, b, c, d)

/**
 * Log minor module level event.
 */
#define L4_DBG_MSG(x)                 HYDRA_LOG0(4, x)
#define L4_DBG_MSG1(x, a)             HYDRA_LOG1(4, x, a)
#define L4_DBG_MSG2(x, a, b)          HYDRA_LOG2(4, x, a, b)
#define L4_DBG_MSG3(x, a, b, c)       HYDRA_LOG3(4, x, a, b, c)
#define L4_DBG_MSG4(x, a, b, c, d)    HYDRA_LOG4(4, x, a, b, c, d)

/**
 * Log minutiae
 */
#define L5_DBG_MSG(x)                 HYDRA_LOG0(5, x)
#define L5_DBG_MSG1(x, a)             HYDRA_LOG1(5, x, a)
#define L5_DBG_MSG2(x, a, b)          HYDRA_LOG2(5, x, a, b)
#define L5_DBG_MSG3(x, a, b, c)       HYDRA_LOG3(5, x, a, b, c)
#define L5_DBG_MSG4(x, a, b, c, d)    HYDRA_LOG4(5, x, a, b, c, d)

/** Return True if the effective log level is greater than or equal
 * to the given level. This can be used to isolate the code that needs
 * to be run only on a certain log level. */
#define DBG_LEVEL_RUN(lvl) (HYDRA_LOG_LEVEL_CURRENT_SYM >= lvl)
/*
 * Shorthand for normal level message.
 */
#define DBG_MSG(x)                    HYDRA_LOG0(2, x)
#define DBG_MSG1(x, a)                HYDRA_LOG1(2, x, a)
#define DBG_MSG2(x, a, b)             HYDRA_LOG2(2, x, a, b)
#define DBG_MSG3(x, a, b, c)          HYDRA_LOG3(2, x, a, b, c)
#define DBG_MSG4(x, a, b, c, d)       HYDRA_LOG4(2, x, a, b, c, d)

#define DBG_PUTCHAR(c)                HYDRA_LOG_PUTCHAR(c)

/**
 * Declare a string for use in Hydra logs.
 *
 * Example use:-
\verbatim

    * declare log string *
    HYDRA_LOG_STRING(event_name, "interesting");

    * log away *
    DBG_MSG2(HYDRA_LOG_NORMAL, "something %s happened", event_name);

\endverbatim
 *
 * \internal
 *
 * Declaration is empty if !INSTALL_HYDRA_LOG,
 * a normal static string for host/unit test builds,
 * and toolchain magic ensures the storage is not actually
 * linked in embedded builds.
 */
#define HYDRA_LOG_STRING(label, text) HYDRA_LOG_STRING_IMPL(label, text)

/**
 * Called by processor 0 to set the registers for P1 firmware log
 * transactions so they get to the correct destination subsystem.
 */
extern void set_p1_firmware_log_trb_parameters(void);

/*****************************************************************************
 * Deprecated macros.
 *
 * Support for old code that we don't want to fix "just now".
 ****************************************************************************/

/**
 * Is Hydra Logging enabled?
 *
 * \deprecated  Use INSTALL_HYDRA_LOG instead.
 *
 * Defined as rubbish to deliberately break and discover any old references
 * that would otherwise just be silently undefined.
 */
#define DEBUG_PRINTF_ENABLED xxx use INSTALL_HYDRA_LOG instead xxx

/**
 * Declare a string for use in a Hydra Debug Log.
 *
 * \deprecated  Use HYDRA_LOG_STRING instead. This macro could never have
 * worked for code built with logging disabled.
 */
#define DECLARE_DEBUG_STRING(label) HYDRA_LOG_STRING_DEPRECATED_IMPL(label)

/**
 * Log string cast type.
 *
 * \deprecated  No longer required.  Was for casting strings when used as
 * argument to a Hydra Debug Log. The macros now do the cast as necessary.
 */
#define DBG_STR const char *

/*****************************************************************************
 * Select Firmware, Software or Disabled Implementation.
 ****************************************************************************/

#if defined(INSTALL_HYDRA_LOG)
#if defined(DESKTOP_TEST_BUILD)
#define HYDRA_LOG_SOFT
#else /* !defined(DESKTOP_TEST_BUILD) */
#define HYDRA_LOG_FIRM
#endif /* !defined(DESKTOP_TEST_BUILD) */
#else /* !defined(INSTALL_HYDRA_LOG) */
#define HYDRA_LOG_DISABLED
#endif /* !defined(INSTALL_HYDRA_LOG) */

/*****************************************************************************
 * Include Implementation Specific Macros
 *
 * The implementing macros vary not just the source code!
 ****************************************************************************/

#if defined(HYDRA_LOG_FIRM)
#include "hydra_log/hydra_log_firm.h"
#elif defined(HYDRA_LOG_SOFT)
#include "hydra_log/hydra_log_soft.h"
#elif defined(HYDRA_LOG_DISABLED)
#include "hydra_log/hydra_log_disabled.h"
#else
#error "Unknown HYDRA_LOG implementation selected."
#endif

#endif /* HYDRA_LOG_H */
