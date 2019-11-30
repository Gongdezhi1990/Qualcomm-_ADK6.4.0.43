/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_log_firm.h
 * \ingroup AUDIO_LOG
 *
 * Audio log (interface specialisation for firmware build)
 *
 * Embedded logs are written to a RAM buffer by the firmware and monitored
 * from host over SPI.
 *
 * The current verbosity level (debugLogLevel) can also be adjusted over SPI.
 */
#ifndef AUDIO_LOG_FIRM_H
#define AUDIO_LOG_FIRM_H

/*****************************************************************************
 * Interface dependencies
 ****************************************************************************/

#include "hydra_types.h"

/*****************************************************************************
 * Private Macros
 ****************************************************************************/

/**
 * Special storage section "attribute" for log string declarations.
 *
 * Builds that do not support DBG_STRING storage section should
 * not define INSTALL_AUDIO_LOG! (it may be possible to
 * resurrect an older log implementation variant if required)
 *
 * This of course requires GNU. I did have an explicit \#test and \#error
 * for this but lint is not setup with the \#defines needed to get past.
 */
#if !defined (__GNUC__)
/* Alternative version that works in KCC */
#define AUDIO_LOG_STRING_ATTR _Pragma("datasection DEBUG_TRACE_STRINGS")
#else
#define AUDIO_LOG_STRING_ATTR __attribute__((section("DBG_STRING")))
#endif

/** Format checking in _gcc_warnings builds */
#if defined(__GNUC__) && defined(AUDIO_LOG_CHECK_FORMATS)
#define AUDIO_LOG_CHECK_FMT0(F)                 audio_log_firm_check((F));
#define AUDIO_LOG_CHECK_FMT1(F,P1)              audio_log_firm_check((F),(P1));
#define AUDIO_LOG_CHECK_FMT2(F,P1,P2)           audio_log_firm_check((F),(P1),(P2));
#define AUDIO_LOG_CHECK_FMT3(F,P1,P2,P3)        audio_log_firm_check((F),(P1),(P2),(P3));
#define AUDIO_LOG_CHECK_FMT4(F,P1,P2,P3,P4)     audio_log_firm_check((F),(P1),(P2),(P3),(P4));
#define AUDIO_LOG_CHECK_FMT5(F,P1,P2,P3,P4,P5)  audio_log_firm_check((F),(P1),(P2),(P3),(P4),(P5));
#else
#define AUDIO_LOG_CHECK_FMT0(F)
#define AUDIO_LOG_CHECK_FMT1(F,P1)
#define AUDIO_LOG_CHECK_FMT2(F,P1,P2)
#define AUDIO_LOG_CHECK_FMT3(F,P1,P2,P3)
#define AUDIO_LOG_CHECK_FMT4(F,P1,P2,P3,P4)
#define AUDIO_LOG_CHECK_FMT5(F,P1,P2,P3,P4,P5)
#endif

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
typedef uintptr_t audio_log_elem;
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
 * Published here only to implement AUDIO_LOG# macros.
 *
 * \note Do not change its name unless you are prepared to fix debug scripts
 * that look for it in elf file.
 */
extern /*volatile*/ audio_log_level debugLogLevel;

/*****************************************************************************
 * Private Functions
 *
 * Published here only to implement AUDIO_LOG# macros.
 *
 * One function call for each number of params to avoid passing nr args
 * from every call site (code-space optimisation CUR-1129).
 ****************************************************************************/

extern void audio_log_firm_0(
    audio_log_elem event_key
);
extern void audio_log_firm_1(
    audio_log_elem event_key,
    audio_log_elem p1
);
extern void audio_log_firm_2(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2
);
extern void audio_log_firm_3(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2,
    audio_log_elem p3
);
extern void audio_log_firm_4(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2,
    audio_log_elem p3,
    audio_log_elem p4
);
extern void audio_log_firm_5(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2,
    audio_log_elem p3,
    audio_log_elem p4,
    audio_log_elem p5
);

/* Downloadable builds use different logging. See more at the function definition. */
extern void audio_log_firm_download_0(
    audio_log_elem event_key
);
extern void audio_log_firm_download_1(
    audio_log_elem event_key,
    audio_log_elem p1
);
extern void audio_log_firm_download_2(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2
);
extern void audio_log_firm_download_3(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2,
    audio_log_elem p3
);
extern void audio_log_firm_download_4(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2,
    audio_log_elem p3,
    audio_log_elem p4
);
extern void audio_log_firm_download_5(
    audio_log_elem event_key,
    audio_log_elem p1,
    audio_log_elem p2,
    audio_log_elem p3,
    audio_log_elem p4,
    audio_log_elem p5
);


#ifdef __GNUC__
static inline void audio_log_firm_check(const char* fmt, ...)
    __attribute__((format(printf,1,2)));
static inline void audio_log_firm_check(const char* fmt, ...)
{
    NOT_USED(fmt);
}
#endif

extern void audio_log_firm_init(void);

/*****************************************************************************
 * Public Macro specialisations
 ****************************************************************************/

#define AUDIO_LOG_SET_LEVEL_IMPL(lvl) (debugLogLevel = (lvl))

#define AUDIO_LOG_INIT_IMPL() audio_log_firm_init()

#ifndef CAPABILITY_DOWNLOAD_BUILD
#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG0(lvl, fmt) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT0(fmt)                                       \
        if (debugLogLevel >= lvl) \
        { \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_0((audio_log_elem)(log_fmt)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG1(lvl, fmt, a1) \
    do                                                                  \
    { \
        AUDIO_LOG_CHECK_FMT1(fmt,a1)                                    \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_1((audio_log_elem)(log_fmt), (audio_log_elem)(a1)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG2(lvl, fmt, a1, a2) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT2(fmt,a1,a2)                                 \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_2((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG3(lvl, fmt, a1, a2, a3) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT3(fmt,a1,a2,a3)                              \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_3((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3));               \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG4(lvl, fmt, a1, a2, a3, a4) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT4(fmt,a1,a2,a3,a4)                           \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_4((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG5(lvl, fmt, a1, a2, a3, a4, a5) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT5(fmt,a1,a2,a3,a4,a5)                        \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_5((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4), (audio_log_elem)(a5)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG0(fmt) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_0((audio_log_elem)(log_fmt)); \
        AUDIO_LOG_CHECK_FMT0(fmt)                                       \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG1(fmt, a1) \
    do                                                                  \
    { \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_1((audio_log_elem)(log_fmt), (audio_log_elem)(a1)); \
        AUDIO_LOG_CHECK_FMT1(fmt,a1) \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG2(fmt, a1, a2) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_2((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2)); \
        AUDIO_LOG_CHECK_FMT2(fmt,a1,a2) \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG3(fmt, a1, a2, a3) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_3((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3));               \
        AUDIO_LOG_CHECK_FMT3(fmt,a1,a2,a3) \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG4(fmt, a1, a2, a3, a4) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_4((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4)); \
        AUDIO_LOG_CHECK_FMT4(fmt,a1,a2,a3,a4)                           \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG5(fmt, a1, a2, a3, a4, a5) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_5((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4), (audio_log_elem)(a5)); \
        AUDIO_LOG_CHECK_FMT5(fmt,a1,a2,a3,a4,a5)                        \
    }                                                                   \
    while (0)
#else /* CAPABILITY_DOWNLOAD_BUILD */
#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG0(lvl, fmt) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT0(fmt)                                       \
        if (debugLogLevel >= lvl) \
        { \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_download_0((audio_log_elem)(log_fmt)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG1(lvl, fmt, a1) \
    do                                                                  \
    { \
        AUDIO_LOG_CHECK_FMT1(fmt,a1)                                    \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_download_1((audio_log_elem)(log_fmt), (audio_log_elem)(a1)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG2(lvl, fmt, a1, a2) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT2(fmt,a1,a2)                                 \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_download_2((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG3(lvl, fmt, a1, a2, a3) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT3(fmt,a1,a2,a3)                              \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_download_3((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3));               \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG4(lvl, fmt, a1, a2, a3, a4) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT4(fmt,a1,a2,a3,a4)                           \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_download_4((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_LOG5(lvl, fmt, a1, a2, a3, a4, a5) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_CHECK_FMT5(fmt,a1,a2,a3,a4,a5)                        \
        if (debugLogLevel >= lvl) \
        {                                                               \
            AUDIO_LOG_STRING(log_fmt, fmt); \
            audio_log_firm_download_5((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4), (audio_log_elem)(a5)); \
        }                                                               \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG0(fmt) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_download_0((audio_log_elem)(log_fmt)); \
        AUDIO_LOG_CHECK_FMT0(fmt)                                       \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG1(fmt, a1) \
    do                                                                  \
    { \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_download_1((audio_log_elem)(log_fmt), (audio_log_elem)(a1)); \
        AUDIO_LOG_CHECK_FMT1(fmt,a1) \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG2(fmt, a1, a2) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_download_2((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2)); \
        AUDIO_LOG_CHECK_FMT2(fmt,a1,a2) \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG3(fmt, a1, a2, a3) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_download_3((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3));               \
        AUDIO_LOG_CHECK_FMT3(fmt,a1,a2,a3) \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG4(fmt, a1, a2, a3, a4) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_download_4((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4)); \
        AUDIO_LOG_CHECK_FMT4(fmt,a1,a2,a3,a4)                           \
    }                                                                   \
    while (0)

#define /*lint --e{607} --e{960} --e{685} --e{568} --e{42} --e{40} */ AUDIO_ALWAYS_LOG5(fmt, a1, a2, a3, a4, a5) \
    do                                                                  \
    {                                                                   \
        AUDIO_LOG_STRING(log_fmt, fmt); \
        audio_log_firm_download_5((audio_log_elem)(log_fmt), (audio_log_elem)(a1), (audio_log_elem)(a2), (audio_log_elem)(a3), (audio_log_elem)(a4), (audio_log_elem)(a5)); \
        AUDIO_LOG_CHECK_FMT5(fmt,a1,a2,a3,a4,a5)                        \
    }                                                                   \
    while (0)

#endif

/*
 * Place string decl in special section.
 */
#define AUDIO_LOG_STRING_IMPL(label, text) \
    AUDIO_LOG_STRING_ATTR static const char label[] = text

/*
 * Support deprecated decl in embedded builds.
 */
#define AUDIO_LOG_STRING_DEPRECATED_IMPL(label) \
    AUDIO_LOG_STRING_ATTR static const char label[]

#endif /* AUDIO_LOG_FIRM_H */
