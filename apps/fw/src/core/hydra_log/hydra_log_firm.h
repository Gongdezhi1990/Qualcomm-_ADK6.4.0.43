/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra log (interface specialisation for firmware build)
 *
 * Embedded logs are written to a RAM buffer by the firmware and monitored
 * from host over SPI.
 *
 * The current verbosity level (debugLogLevel) can also be adjusted over SPI.
 */
#ifndef HYDRA_LOG_FIRM_H
#define HYDRA_LOG_FIRM_H

/*****************************************************************************
 * Interface dependencies
 ****************************************************************************/

#include "hydra/hydra_types.h"

#if defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS)
#include "hydra_log/hydra_log_firm_modules.h"
#endif /* defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */

/*****************************************************************************
 * Prerequisites
 ****************************************************************************/

#if defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS)
#ifndef _MODULE_NAME_
#error "_MODULE_NAME_" must be defined if INSTALL_HYDRA_LOG_PER_MODULE_LEVELS.
#endif
#endif /* defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */

/*****************************************************************************
 * Private Macros
 ****************************************************************************/

/**
 * Special storage section "attribute" for log string declarations.
 *
 * Builds that do not support DBG_STRING storage section should
 * not define INSTALL_HYDRA_LOG! (it may be possible to
 * resurrect an older log implementation variant if required)
 *
 * This of course requires GNU. I did have an explicit \#test and \#error
 * for this but lint is not setup with the \#defines needed to get past.
 */
#if defined (__KALIMBA__)
/* Alternative version that works in KCC */
#define HYDRA_LOG_STRING_ATTR _Pragma("datasection DBG_STRING")
#else
#define HYDRA_LOG_STRING_ATTR __attribute__((section("DBG_STRING")))
#endif

/**
 * Attribute for the hydra_log_firm1..4() prototypes so that the compiler
 * checks the printf format string. Unfortunately it cannot type check
 * the arguments against the format string because our log functions are
 * not variadic ie. ",...)".
 *
 * This requires GNU C. As stated above for HYDRA_LOG_STRING_ATTR lint
 * is not setup with the \#defines needed to get past.
 */
#define HYDRA_LOG_FIRM_PRINTF_ATTR GCC_ATTRIBUTE(format(printf, 1, 0))

/*****************************************************************************
 * Private Types
 ****************************************************************************/

/**
 * Embedded event log "element" type.
 *
 * The format string address and all arguments are caste to this type.
 *
 * It should be an integer type big enough to accept a native char *.
 *
 * This type is only published here as all log args must be cast by the
 * macros before calling the implementing function.
 */
typedef unsigned int hydra_log_elem;

/**
 * Hydra Log State (type)
 */
typedef struct HYDRA_LOG
{
#if defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS)
    /**
     * Per-module log threshold.
     */
    struct
    {
        /* We need the _threshold in order to avoid problems with
         * modules like int, which would otherwise give a syntax error, ie
         * hydra_log_level int;
         */
        #define HYDRA_LOG_DECL(module) hydra_log_level module ## _threshold;
        /*lint -e(123) : HYDRA_LOG_DECL defined with arguments  */
        HYDRA_LOG_FIRM_MODULES_X(HYDRA_LOG_DECL)
        #undef HYDRA_LOG_DECL
    } levels;
#else /* !defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */
    /**
     * Global log threshold.
     */
    hydra_log_level level;
#endif /* !defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */
} HYDRA_LOG;

/*****************************************************************************
 * Private Data Declarations
 ****************************************************************************/

/**
 * Hydra Log State.
 */
extern HYDRA_LOG hydra_log;

/*****************************************************************************
 * Private Data
 ****************************************************************************/

/**
 * Current debug log "verbosity" level.
 *
 * The value may be changed via SPI but we _don't_ declare it volatile
 * as this forces the compiler to load it when unnecessary (for level 0
 * messages). And seems rather unlikely that change in level during a function
 * call is going to be noticed. (code-space optimisation CUR-1129)
 *
 * Published here only to implement HYDRA_LOG# macros.
 *
 * \note Do not change its name unless you are prepared to fix debug scripts
 * that look for it in elf file.
 */
#define HYDRA_LOG_BUFFER_ATTR __attribute__((section("UNINIT_VAR")))

#if defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS)

/**
 * Paste module-specific log level symbol given a module name.
 *
 * Examples:-
 * - HYDRA_LOG_LEVEL_MODULE_SYM(lpc) => hydra_log.levels.lpc
 * - HYDRA_LOG_LEVEL_MODULE_SYM(utils) => hydra_log.levels.utils
 * Note the usual double indirection trickery to get the incoming parameter
 * substituted before we try to paste on _threshold
 */
#define HYDRA_LOG_LEVEL_MODULE_SYM_HELPER(x) hydra_log.levels.x ## _threshold
#define HYDRA_LOG_LEVEL_MODULE_SYM(MODULE_NAME) HYDRA_LOG_LEVEL_MODULE_SYM_HELPER(MODULE_NAME)

#endif /* defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */

/**
 * Log Level Alias for the module currently being compiled.
 */
#if defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS)

#define HYDRA_LOG_LEVEL_CURRENT_SYM HYDRA_LOG_LEVEL_MODULE_SYM(_MODULE_NAME_)

#else /* !defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */

#define HYDRA_LOG_LEVEL_CURRENT_SYM hydra_log.level

#endif /* !defined(INSTALL_HYDRA_LOG_PER_MODULE_LEVELS) */


/*****************************************************************************
 * Private Functions
 *
 * Published here only to implement HYDRA_LOG# macros.
 *
 * One function call for each number of params to avoid passing nr args
 * from every call site (code-space optimisation CUR-1129).
 ****************************************************************************/

extern void hydra_log_firm_0
(
    const char *event_key
);

extern void hydra_log_firm_1
(
    const char *event_key,
    hydra_log_elem p1
) HYDRA_LOG_FIRM_PRINTF_ATTR;

extern void hydra_log_firm_2
(
    const char *event_key,
    hydra_log_elem p1,
    hydra_log_elem p2
) HYDRA_LOG_FIRM_PRINTF_ATTR;

extern void hydra_log_firm_3
(
    const char *event_key,
    hydra_log_elem p1,
    hydra_log_elem p2,
    hydra_log_elem p3
) HYDRA_LOG_FIRM_PRINTF_ATTR;

extern void hydra_log_firm_4
(
    const char *event_key,
    hydra_log_elem p1,
    hydra_log_elem p2,
    hydra_log_elem p3,
    hydra_log_elem p4
) HYDRA_LOG_FIRM_PRINTF_ATTR;

extern void hydra_log_firm_putchar(char c);

/*****************************************************************************
 * Public Macro specialisations
 ****************************************************************************/

#define HYDRA_LOG_SET_LEVEL_IMPL(lvl) (debugLogLevel = (lvl))

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ HYDRA_LOG0(lvl, fmt) \
    do                                                                  \
    {                                                                   \
        if (HYDRA_LOG_LEVEL_CURRENT_SYM >= lvl) \
        { \
            HYDRA_LOG_STRING(log_fmt, fmt); \
            hydra_log_firm_0(log_fmt); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ HYDRA_LOG1(lvl, fmt, a1) \
    do                                                                  \
    { \
        if (HYDRA_LOG_LEVEL_CURRENT_SYM >= lvl) \
        {                                                               \
            HYDRA_LOG_STRING(log_fmt, fmt); \
            hydra_log_firm_1(log_fmt, (hydra_log_elem)(a1)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ HYDRA_LOG2(lvl, fmt, a1, a2) \
    do                                                                  \
    {                                                                   \
        if (HYDRA_LOG_LEVEL_CURRENT_SYM >= lvl) \
        {                                                               \
            HYDRA_LOG_STRING(log_fmt, fmt); \
            hydra_log_firm_2(log_fmt, (hydra_log_elem)(a1), (hydra_log_elem)(a2)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ HYDRA_LOG3(lvl, fmt, a1, a2, a3) \
    do                                                                  \
    {                                                                   \
        if (HYDRA_LOG_LEVEL_CURRENT_SYM >= lvl) \
        {                                                               \
            HYDRA_LOG_STRING(log_fmt, fmt); \
            hydra_log_firm_3(log_fmt, (hydra_log_elem)(a1), (hydra_log_elem)(a2), (hydra_log_elem)(a3));               \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ HYDRA_LOG4(lvl, fmt, a1, a2, a3, a4) \
    do                                                                  \
    {                                                                   \
        if (HYDRA_LOG_LEVEL_CURRENT_SYM >= lvl) \
        {                                                               \
            HYDRA_LOG_STRING(log_fmt, fmt); \
            hydra_log_firm_4(log_fmt, (hydra_log_elem)(a1), (hydra_log_elem)(a2), (hydra_log_elem)(a3), (hydra_log_elem)(a4)); \
        }                                                               \
    }                                                                   \
    while (0)

#define HYDRA_LOG_PUTCHAR(c)    hydra_log_firm_putchar((char)(c & 0xff))

/*
 * Place string decl in special section.
 */
#define HYDRA_LOG_STRING_IMPL(label, text) \
    HYDRA_LOG_STRING_ATTR static const char label[] = text

/*
 * Support deprecated decl in embedded builds.
 */
#define HYDRA_LOG_STRING_DEPRECATED_IMPL(label) \
    HYDRA_LOG_STRING_ATTR static const char label[]

/*
 * Macro to set log level of specific module identified by module number.
 */
#define HYDRA_SET_LOG_LEVEL(module_index, log_level) \
    (*(((hydra_log_level *)&hydra_log.levels) + module_index) = log_level)
/**
 * Enables/disables TRB logging.
 *
 * Sets the global variable responsible for TRB logging to enable.
 */
extern void hydra_log_enable_trb_logging(bool enable);

/**
 * Log a variadic debug event to circular buffer.
 *
 * Common implementation for hydra_log_firm_N functions.
 */
extern void hydra_log_firm_variadic(const char *event_key,
                                    size_t n_args,
                                    /*lint -e{1916} Ellipsis encountered*/
                                    ...);

#endif /* HYDRA_LOG_FIRM_H */
