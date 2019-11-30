/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra log (interface specialisation for host software build)
 *
 * Software (c.f. firmware) implementation stores string in normal memory and
 * maps log to stdout.
 */
#ifndef HYDRA_LOG_SOFT_H
#define HYDRA_LOG_SOFT_H

/*****************************************************************************
 * Interface dependencies
 ****************************************************************************/

/*****************************************************************************
 * Private Data
 ****************************************************************************/

/**
 * Current log level.
 *
 * Published for use in macro implementation.
 */
extern hydra_log_level hydra_log_current_level;

/*****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * Log Event.
 *
 * Implements the HYDRA_LOG_# macros for non-embedded target.
 *
 * On GCC type check the arguments against the printf format string.
 */
void hydra_log_soft(const char *file_name, int line_num, const char *fmt, ...)
#ifdef SUBSYSTEM_APPS
    ; /* we don't want this on Apps at the moment */
#else
    GCC_ATTRIBUTE(format(printf, 3, 4));
#endif

/*****************************************************************************
 * Public Macro specialisations
 ****************************************************************************/

/*
 * Just set the level inline (as it is published anyway).
 */
#define HYDRA_LOG_SET_LEVEL_IMPL(lvl) (hydra_log_current_level = (lvl))

/*
 * Logging macro implementations.
 *
 * The "hydra_log_level tmp_lvl = lvl;" hack below stops the compiler warning
 * about comparison always true when level = 0.
 */

#define HYDRA_LOG0(lvl, fmt) \
    do { \
    hydra_log_level tmp_lvl = lvl; \
    if (hydra_log_current_level >= tmp_lvl) \
      hydra_log_soft(__FILE__,__LINE__, (fmt)); \
    } \
    while (0)

#define HYDRA_LOG1(lvl, fmt, a1)\
    do { \
    hydra_log_level tmp_lvl = lvl; \
    if (hydra_log_current_level >= tmp_lvl) \
      hydra_log_soft(__FILE__,__LINE__, (fmt), (a1)); \
    } \
    while (0)

#define HYDRA_LOG2(lvl, fmt, a1, a2)\
    do { \
    hydra_log_level tmp_lvl = lvl; \
    if (hydra_log_current_level >= tmp_lvl) \
      hydra_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2)); \
    } \
    while (0)

#define HYDRA_LOG3(lvl, fmt, a1, a2, a3) \
    do { \
    hydra_log_level tmp_lvl = lvl; \
    if (hydra_log_current_level >= tmp_lvl) \
      hydra_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3)); \
    } \
    while (0)

#define HYDRA_LOG4(lvl, fmt, a1, a2, a3, a4) \
    do { \
    hydra_log_level tmp_lvl = lvl; \
    if (hydra_log_current_level >= tmp_lvl) \
      hydra_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3), (a4)); \
    } \
    while (0)

/** putchar goes through the normal log print for desktop builds */
#define HYDRA_LOG_PUTCHAR(c)    HYDRA_LOG1(2, "%c", c)

/* Expose current log level */
#define HYDRA_LOG_LEVEL_CURRENT_SYM hydra_log_current_level

/*
 * Just declare a normal string for host builds...
 */
#define HYDRA_LOG_STRING_IMPL(label, text) \
    static const char label[] = text

/*
 * Continue to support the deprecated string decl style in host builds...
 */
#define HYDRA_LOG_STRING_DEPRECATED_IMPL(label) \
    static const char label[]

#endif /* HYDRA_LOG_SOFT_H */
