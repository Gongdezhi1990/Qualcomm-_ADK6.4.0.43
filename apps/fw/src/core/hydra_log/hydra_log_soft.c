/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file
 * Hydra Log (unit test/host software variant implementation)
 */

/*****************************************************************************
 * Implementation dependencies.
 ****************************************************************************/

#include "hydra_log/hydra_log.h"

#if defined(HYDRA_LOG_SOFT)

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <errno.h>

#include "os_def.h"
#include "coal/trace_time.h"
#include "hydra/hydra_macros.h"

/*****************************************************************************
 * Private definitions.
 ****************************************************************************/

#define HYDRA_LOG_MAX_FILE_LEN 512

/*****************************************************************************
 * Private Functions
 ****************************************************************************/

/**
 * Extract filename field from end of full path.
 */
static const char *extract_module_name(const char *file)
{
   const char *module_name = file;
   if (NULL != file && HYDRA_LOG_MAX_FILE_LEN > strlen(file))
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
    char timestr[TRACE_TIME_BUF_SIZE]="";
    trace_time(timestr, TRACE_TIME_BUF_SIZE);
    printf("%s, ", timestr);
}

/*****************************************************************************
 * Public Data.
 ****************************************************************************/

hydra_log_level hydra_log_current_level = HYDRA_LOG_V_V_VERBOSE;

/*****************************************************************************
 * Public Functions
 ****************************************************************************/

void hydra_log_soft(const char *name, int line, const char *fmt, ...)
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

#endif /* defined(INSTALL_HYDRA_SOFT) */
