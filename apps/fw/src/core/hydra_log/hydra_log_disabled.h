/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra log (interface specialisation for log disabled case)
 */
#ifndef HYDRA_LOG_DISABLED_H
#define HYDRA_LOG_DISABLED_H

/*****************************************************************************
 * Interface dependencies
 ****************************************************************************/

#include "hydra_log/hydra_log.h"

/*****************************************************************************
 * Public Macro specialisations
 ****************************************************************************/

/**
 * log level set does nothing when disabled.
 */
#define HYDRA_LOG_SET_LEVEL_IMPL(lvl) ((void)0)

/**
 * Log macros are no-ops when log is disabled.
 */
#define HYDRA_LOG0(lvl, fmt) ((void)0)
#define HYDRA_LOG1(lvl, fmt, a1) ((void)0)
#define HYDRA_LOG2(lvl, fmt, a1, a2) ((void)0)
#define HYDRA_LOG3(lvl, fmt, a1, a2, a3) ((void)0)
#define HYDRA_LOG4(lvl, fmt, a1, a2, a3, a4) ((void)0)
#define HYDRA_LOG_PUTCHAR(c) ((void)(c))

/** Log is disabled */
#define HYDRA_LOG_LEVEL_CURRENT_SYM -1

/**
 * Log string decl is just empty when log is disabled.
 */
/*lint -emacro(19,HYDRA_LOG_STRING_IMPL) Useless Declaration is intentional. */
#define HYDRA_LOG_STRING_IMPL(label, text)

/**
 * Old log string decl style never could work when log is disabled.
 *
 * Just define a string to break the compilation when/if anyone tries to
 * build old code with logging disabled.
 */
#define HYDRA_LOG_STRING_DEPRECATED_IMPL(label) \
"!!! It's time to replace DECLARE_DEBUG_STRING with HYDRA_LOG_STRING !!!"

#endif /* HYDRA_LOG_DISABLED_H */
