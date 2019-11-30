/****************************************************************************
 * Copyright (c) 2013 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
/**
 * \file audio_log_soft.c
 * \ingroup AUDIO_LOG
 *
 * Audio Log (unit test/host software variant implementation)
 *
 */

/*****************************************************************************
 * Implementation dependencies.
 ****************************************************************************/

#include "audio_log.h"

/*
 * Only compile this implementation variant if the header tells us it's
 * a software/unit test target!
 */
#if defined(AUDIO_LOG_SOFT)
#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "hydra_macros.h"

/*****************************************************************************
 * Private definitions.
 ****************************************************************************/

#define AUDIO_LOG_MAX_FILE_LEN 512

#ifdef INSTALL_AUDIO_LOG
/* We only support Linux builds */
#define SEPARATOR_DIR_SLASH '/'
#endif

/*****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * Extract filename field from end of full path.
 */
static const char *extract_module_name(const char *file)
{
   const char *module_name = file;
   if (NULL != file && AUDIO_LOG_MAX_FILE_LEN > strlen(file))
   {
      /*
       * Extract the module name in a string
       * For instance: osabslogprintutils.c in windows file string below
       * V:\uri\osabs\nucleus\code\osabslogprintutils.c
       * strrchr returns the LAST occurence of the \ in the string
       */
      module_name = strrchr(file, SEPARATOR_DIR_SLASH);
      module_name = (NULL == module_name) ? file : module_name + 1;
   }
   return module_name;
}

/**
 * Print "trace time" to stdout.
 */
static void printf_time(void)
{
#ifndef INSTALL_AUDIO_LOG
/* Disabled for now - TODO implement this properly ? */
    char timestr[TRACE_TIME_BUF_SIZE]="";
    trace_time(timestr, TRACE_TIME_BUF_SIZE);
    printf("%s, ", timestr);
#endif
}

/*****************************************************************************
 * Public Data.
 ****************************************************************************/

audio_log_level audio_log_current_level = AUDIO_LOG_V_V_VERBOSE;

/*****************************************************************************
 * Public Functions
 ****************************************************************************/

void audio_log_soft(const char *name, int line, const char *fmt, ...)
{
    va_list va_argp;
    const char *fname = extract_module_name(name);

    /*
     * Prefix time, file & line.
     */
    printf_time();
    printf("%s(%u):", fname, line);

    /*
     * Pass fmt and varargs to vprintf.
     *
     * \todo    Parse the fmt manually here to treat all ints as uint16s
     * for compatability with firmware.
     */
    va_start(va_argp, fmt);
    vprintf(fmt, va_argp);
    va_end( va_argp );

    printf("\n");
}

#endif /* defined(INSTALL_AUDIO_SOFT) */
