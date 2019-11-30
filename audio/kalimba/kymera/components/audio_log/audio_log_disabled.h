/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_log_disabled.h
 * \ingroup AUDIO_LOG
 *
 * Audio log (interface specialisation for log disabled case)
 *
 */
#ifndef AUDIO_LOG_DISABLED_H
#define AUDIO_LOG_DISABLED_H

/*****************************************************************************
 * Interface dependencies
 ****************************************************************************/

#include "audio_log.h"

/*****************************************************************************
 * Public Macro specialisations
 ****************************************************************************/

/**
 * log level set does nothing when disabled.
 */
#define AUDIO_LOG_SET_LEVEL_IMPL(lvl) ((void)0)

#define AUDIO_LOG_INIT_IMPL() ((void)0)

/**
 * Log macros are no-ops when log is disabled.
 */
#define AUDIO_LOG0(lvl, fmt) ((void)0)
#define AUDIO_LOG1(lvl, fmt, a1) ((void)(a1))
#define AUDIO_LOG2(lvl, fmt, a1, a2) ((void)(a1), (void)(a2))
#define AUDIO_LOG3(lvl, fmt, a1, a2, a3) ((void)(a1), (void)(a2), (void)(a3))
#define AUDIO_LOG4(lvl, fmt, a1, a2, a3, a4) ((void)(a1), (void)(a2), (void)(a3), (void)(a4))
#define AUDIO_LOG5(lvl, fmt, a1, a2, a3, a4, a5) ((void)(a1), (void)(a2), (void)(a3), (void)(a4), (void)(a5))

#define AUDIO_ALWAYS_LOG0(fmt) ((void)0)
#define AUDIO_ALWAYS_LOG1(fmt, a1) ((void)(a1))
#define AUDIO_ALWAYS_LOG2(fmt, a1, a2) ((void)(a1), (void)(a2))
#define AUDIO_ALWAYS_LOG3(fmt, a1, a2, a3) ((void)(a1), (void)(a2), (void)(a3))
#define AUDIO_ALWAYS_LOG4(fmt, a1, a2, a3, a4) ((void)(a1), (void)(a2), (void)(a3), (void)(a4))
#define AUDIO_ALWAYS_LOG5(fmt, a1, a2, a3, a4, a5) ((void)(a1), (void)(a2), (void)(a3), (void)(a4), (void)(a5))

/**
 * Log string decl is just empty when log is disabled.
 */
/*lint -emacro(19,AUDIO_LOG_STRING_IMPL) Useless Declaration is intentional. */
#define AUDIO_LOG_STRING_IMPL(label, text) static const char label[] = ""

/**
 * Old log string decl style never could work when log is disabled.
 *
 * Just define a string to break the compilation when/if anyone tries to
 * build old code with logging disabled.
 */
#define AUDIO_LOG_STRING_DEPRECATED_IMPL(label) \
"!!! It's time to replace DECLARE_DEBUG_STRING with AUDIO_LOG_STRING !!!"

#endif /* AUDIO_LOG_DISABLED_H */
