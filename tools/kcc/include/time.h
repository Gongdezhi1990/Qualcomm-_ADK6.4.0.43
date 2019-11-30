/*
 * @(#)time.h	1800.54 11/07/25
 * 
 * COPYRIGHT NOTICE (NOT TO BE REMOVED):
 * 
 * This file, or parts of it, or modified versions of it, may not be copied,
 * reproduced or transmitted in any form, including reprinting, translation,
 * photocopying or microfilming, or by any means, electronic, mechanical or
 * otherwise, or stored in a retrieval system, or used for any purpose, without
 * the prior written permission of all Owners unless it is explicitly marked as
 * having Classification `Public'.
 * 
 * Owners of this file give notice:
 *   (c) Copyright 1998-2004 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998-2004 ACE Associated Computer Experts bv
 * All rights, including copyrights, reserved.
 * 
 * This file contains or may contain restricted information and is UNPUBLISHED
 * PROPRIETARY SOURCE CODE OF THE Owners.  The Copyright Notice(s) above do not
 * evidence any actual or intended publication of such source code.  This file
 * is additionally subject to the conditions listed in the RESTRICTIONS file
 * and is with NO WARRANTY.
 * 
 * END OF COPYRIGHT NOTICE
 */

#ifndef __time_included
#define __time_included

#ifndef __size_t_defined
#define __size_t_defined
#	include <target/size.h>
#endif

#include <target/clocks.h>

#ifndef NULL
# define  NULL	( (void*) 0 )
#endif

#ifndef __clock_t_defined
    typedef unsigned long    clock_t;
#   define  __clock_t_defined
#endif

#ifndef __time_t_defined
    typedef unsigned long    time_t;
#   define  __time_t_defined
#endif

struct tm {
    int     tm_sec;
    int     tm_min;
    int     tm_hour;
    int     tm_mday;
    int     tm_mon;
    int     tm_year;
    int     tm_wday;
    int     tm_yday;
    int     tm_isdst;
};

#ifdef COMPILING_LIBC
#pragma ckf clock      i
#pragma ckf difftime   f
#pragma ckf mktime     f  MayReadGlobalObject MayWriteGlobalObject \
			  MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf time       f  MayReadGlobalObject MayWriteGlobalObject \
			  MayWritePointedAtObject
#pragma ckf asctime    f  MayReadGlobalObject MayWriteGlobalObject \
			  MayReadPointedAtObject
#pragma ckf ctime      f  MayReadGlobalObject MayWriteGlobalObject \
			  MayReadPointedAtObject
#pragma ckf gmtime     f  MayReadGlobalObject MayWriteGlobalObject \
			  MayReadPointedAtObject
#pragma ckf localtime  f  MayReadGlobalObject MayWriteGlobalObject \
			  MayReadPointedAtObject
#pragma ckf strftime   f  MayReadGlobalObject MayWriteGlobalObject \
			  MayReadPointedAtObject MayWritePointedAtObject
#else
#pragma ckf clock      clock      f  MayHaveOtherEffect
#pragma ckf difftime   difftime   f  
#pragma ckf mktime     mktime     f  MayHaveOtherEffect MayReadPointedAtObject \
				     MayWritePointedAtObject
#pragma ckf time       time       f  MayHaveOtherEffect MayWritePointedAtObject
#pragma ckf asctime    asctime    f  MayReadGlobalObject MayWriteGlobalObject \
				     MayHaveOtherEffect MayReadPointedAtObject
#pragma ckf ctime      ctime      f  MayReadGlobalObject MayWriteGlobalObject \
				     MayHaveOtherEffect MayReadPointedAtObject
#pragma ckf gmtime     gmtime     f  MayReadGlobalObject MayWriteGlobalObject \
				     MayHaveOtherEffect MayReadPointedAtObject
#pragma ckf localtime  localtime  f  MayReadGlobalObject MayWriteGlobalObject \
				     MayHaveOtherEffect MayReadPointedAtObject
#pragma ckf strftime   strftime   f  MayHaveOtherEffect \
				     MayReadPointedAtObject \
				     MayWritePointedAtObject
#endif

clock_t     clock (void);
double      difftime (time_t, time_t);
time_t      mktime (struct tm *);
time_t      time (time_t *);

char        *asctime (const struct tm *);
char        *ctime (const time_t *);
struct tm   *gmtime (const time_t *);
struct tm   *localtime (const time_t *);
size_t      strftime (char *, size_t, const char *, const struct tm *);

#endif  /* __time_included */
