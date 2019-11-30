/*
 * @(#)ctype.h	1800.54 11/07/25
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
 *   (c) Copyright 1998,2003,2004 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998,2003,2004 ACE Associated Computer Experts bv
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

#ifndef __ctype_included
#define __ctype_included

#include <limits.h>

#ifdef COMPILING_LIBC
#pragma ckf isalnum   p
#pragma ckf isalpha   p
#pragma ckf iscntrl   p
#pragma ckf isdigit   p
#pragma ckf isgraph   p
#pragma ckf islower   p
#pragma ckf isprint   p
#pragma ckf ispunct   p
#pragma ckf isspace   p
#pragma ckf isupper   p
#pragma ckf isxdigit  p
#pragma ckf tolower   p
#pragma ckf toupper   p
#else
#pragma ckf isalnum   isalnum   p
#pragma ckf isalpha   isalpha   p
#pragma ckf iscntrl   iscntrl   p
#pragma ckf isdigit   isdigit   p
#pragma ckf isgraph   isgraph   p
#pragma ckf islower   islower   p
#pragma ckf isprint   isprint   p
#pragma ckf ispunct   ispunct   p
#pragma ckf isspace   isspace   p
#pragma ckf isupper   isupper   p
#pragma ckf isxdigit  isxdigit  p
#pragma ckf tolower   tolower   p
#pragma ckf toupper   toupper   p
#endif

int     isalnum (int);
int     isalpha (int);
int     iscntrl (int);
int     isdigit (int);
int     isgraph (int);
int     islower (int);
int     isprint (int);
int     ispunct (int);
int     isspace (int);
int     isupper (int);
int     isxdigit (int);
int     tolower (int);
int     toupper (int);

#if UCHAR_MAX <= 255
/* For implementations with a larger unsigned character range,
 * we choose to only use functions instead of macros indexing an array
 * (since the array becomes very large).
 */
extern const unsigned char   __Ctype[];
#define __ctype     (&__Ctype[1])

#define isalnum(c)  (__ctype[c] & 0x07)
#define isalpha(c)  (__ctype[c] & 0x03)
#define iscntrl(c)  (__ctype[c] & 0x20)
#define isdigit(c)  (__ctype[c] & 0x04)
#define isgraph(c)  (__ctype[c] & 0x47)
#define islower(c)  (__ctype[c] & 0x01)
#define isprint(c)  (__ctype[c] & 0x87)
#define ispunct(c)  (__ctype[c] & 0x40)
#define isspace(c)  (__ctype[c] & 0x10)
#define isupper(c)  (__ctype[c] & 0x02)
#define isxdigit(c) (__ctype[c] & 0x08)
#endif

#endif  /* __ctype_included */
