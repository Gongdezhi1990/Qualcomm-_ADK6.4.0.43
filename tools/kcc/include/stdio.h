/*
 * @(#)stdio.h  1800.11 08/09/24
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
 *   (c) Copyright 1998-2004,2008 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998-2004,2008 ACE Associated Computer Experts bv
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

#ifndef __stdio_included
#define __stdio_included

/* need va_list type */
#include <stdarg.h>

#ifndef __size_t_defined
#define __size_t_defined
#   include <target/size.h>
#endif

#define EOF             (-1)

#ifdef COMPILING_LIBC
#pragma ckf printf    f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf scanf     f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayStorePointer
#pragma ckf sprintf   f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf snprintf  f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf sscanf    f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf vprintf   f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf vsprintf  f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf vsnprintf f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer
#pragma ckf vsscanf   f  MayReadGlobalObject MayWriteGlobalObject \
                         MayReadPointedAtObject MayWritePointedAtObject \
                         MayStorePointer

#else
#pragma ckf printf    printf    f  MayHaveOtherEffect MayWriteGlobalObject \
                                   MayReadPointedAtObject \
                                   MayReadGlobalObject MayWriteGlobalObject
#pragma ckf scanf     scanf     f  MayHaveOtherEffect MayReadGlobalObject \
                                   MayWritePointedAtObject \
                                   MayReadGlobalObject MayWriteGlobalObject
#pragma ckf sprintf   sprintf   f  MayReadPointedAtObject \
                                   MayWritePointedAtObject \
                                   MayReadGlobalObject MayWriteGlobalObject
#pragma ckf sscanf    sscanf    f  MayReadPointedAtObject \
                                   MayWritePointedAtObject \
                                   MayReadGlobalObject MayWriteGlobalObject
#pragma ckf vprintf   vprintf   f  MayHaveOtherEffect MayWriteGlobalObject \
                                   MayReadPointedAtObject \
                                   MayReadGlobalObject MayWriteGlobalObject
#pragma ckf vsprintf  vsprintf  f  MayReadPointedAtObject \
                                   MayWritePointedAtObject \
                                   MayReadGlobalObject MayWriteGlobalObject

/* No CCMIR CKFs defined for these functions */
#pragma ckf snprintf  f  MayReadPointedAtObject \
                         MayWritePointedAtObject \
                         MayReadGlobalObject MayWriteGlobalObject
#pragma ckf vsnprintf f  MayReadPointedAtObject \
                         MayWritePointedAtObject \
                         MayReadGlobalObject MayWriteGlobalObject
#pragma ckf vsscanf   f  MayReadPointedAtObject \
                         MayWritePointedAtObject \
                         MayReadGlobalObject MayWriteGlobalObject
#endif

int printf (const char *, ...);
int scanf (const char *, ...);
int sprintf (char *, const char *, ...);
int snprintf (char *, int, const char *, ...);
int sscanf (const char *, const char *, ...);
int vprintf (const char *, va_list);
int vsprintf (char *, const char *, va_list);
int vsnprintf (char *, int, const char *, va_list);
int vsscanf (const char *, const char *, va_list);

int putchar (int);

/*
 * Flags as used in printf/scanf formatting
 */
#define LEFTJUST        0x0001  /* left justify */
#define NEEDSIGN        0x0002  /* always begin with + or - */
#define NEEDSPACE       0x0004  /* add space if no sign */
#define ALTERNATE       0x0008  /* alternate form */
#define ZEROS           0x0010  /* pad with leading zeros */
#define ARGCHAR         0x0020  /* argument is char */
#define ARGSHORT        0x0040  /* argument is short */
#define ARGLONG         0x0080  /* argument is long */
#define ARGLONGLONG     0x0100  /* argument is long long */
#define ARGLDOUBLE      0x0200  /* argument is long double */
#define ARGUNSIGNED     0x0400  /* argument is unsigned ('U') */
#define SUPPRESS        0x0800  /* suppress assignment */
#define ANYCONV         0x1000  /* set after a conversion is done */

extern int      __sformat (const char *, va_list,
                           int (*) (int, int, void *), void *, int);

extern int      __pformat(const char *, va_list,
                          int (*dochar)(int), void *args);

extern void     __pfmtFP(char *, long double, int, int, int, int);
#ifdef  __DSPC__
extern void     __pfmtFix(char *, void *, int, int, int, int);
#endif
#ifdef __EMBEDDEDC__
extern void     __pfmtEmb(char *, void *, int, int, int, int);
#endif

#endif  /* __stdio_included */
