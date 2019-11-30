/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_log_soft.h
 * \ingroup AUDIO_LOG
 *
 * Audio log (interface specialisation for host software build)
 *
 * Software (c.f. firmware) implementation stores string in normal memory and
 * maps log to stdout.
 */
#ifndef AUDIO_LOG_SOFT_H
#define AUDIO_LOG_SOFT_H

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
extern audio_log_level audio_log_current_level;

/*****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * Log Event.
 *
 * Implements the AUDIO_LOG_# macros for non-embedded target.
 */
void audio_log_soft(const char *file_name, int line_num, const char *fmt, ...);

/*****************************************************************************
 * Public Macro specialisations
 ****************************************************************************/

/*
 * Just set the level inline (as it is published anyway).
 */
#define AUDIO_LOG_SET_LEVEL_IMPL(lvl) (audio_log_current_level = (lvl))

#define AUDIO_LOG_INIT_IMPL() ((void)0)

/*
 * Logging macro implementations.
 *
 * The "audio_log_level tmp_lvl = lvl;" the code below stops the compiler warning
 * about comparison always true when level = 0.
 */

#define AUDIO_LOG0(lvl, fmt) \
    do { \
    audio_log_level tmp_lvl = lvl; \
    if (audio_log_current_level >= tmp_lvl) \
      audio_log_soft(__FILE__,__LINE__, (fmt)); \
    } \
    while (0)

#define AUDIO_LOG1(lvl, fmt, a1)\
    do { \
    audio_log_level tmp_lvl = lvl; \
    if (audio_log_current_level >= tmp_lvl) \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1)); \
    } \
    while (0)

#define AUDIO_LOG2(lvl, fmt, a1, a2)\
    do { \
    audio_log_level tmp_lvl = lvl; \
    if (audio_log_current_level >= tmp_lvl) \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2)); \
    } \
    while (0)

#define AUDIO_LOG3(lvl, fmt, a1, a2, a3) \
    do { \
    audio_log_level tmp_lvl = lvl; \
    if (audio_log_current_level >= tmp_lvl) \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3)); \
    } \
    while (0)

#define AUDIO_LOG4(lvl, fmt, a1, a2, a3, a4) \
    do { \
    audio_log_level tmp_lvl = lvl; \
    if (audio_log_current_level >= tmp_lvl) \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3), (a4)); \
    } \
    while (0)

#define AUDIO_LOG5(lvl, fmt, a1, a2, a3, a4, a5) \
    do { \
    audio_log_level tmp_lvl = lvl; \
    if (audio_log_current_level >= tmp_lvl) \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3), (a4), (a5)); \
    } \
    while (0)

#define AUDIO_ALWAYS_LOG0(fmt) \
    do { \
      audio_log_soft(__FILE__,__LINE__, (fmt)); \
    } \
    while (0)

#define AUDIO_ALWAYS_LOG1(fmt, a1)\
    do { \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1)); \
    } \
    while (0)

#define AUDIO_ALWAYS_LOG2(fmt, a1, a2)\
    do { \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2)); \
    } \
    while (0)

#define AUDIO_ALWAYS_LOG3(fmt, a1, a2, a3) \
    do { \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3)); \
    } \
    while (0)

#define AUDIO_ALWAYS_LOG4(fmt, a1, a2, a3, a4) \
    do { \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3), (a4)); \
    } \
    while (0)

#define AUDIO_ALWAYS_LOG5(fmt, a1, a2, a3, a4, a5) \
    do { \
      audio_log_soft(__FILE__,__LINE__, (fmt), (a1), (a2), (a3), (a4), (a5)); \
    } \
    while (0)

/*
 * Just declare a normal string for host builds...
 */
#define AUDIO_LOG_STRING_IMPL(label, text) \
    static const char label[] = text

/*
 * Continue to support the deprecated string decl style in host builds...
 */
#define AUDIO_LOG_STRING_DEPRECATED_IMPL(label) \
    static const char label[]

#endif /* AUDIO_LOG_SOFT_H */
