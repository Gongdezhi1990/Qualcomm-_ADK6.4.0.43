/*
 * @(#)string.h	1800.54 11/07/25
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

#ifndef __string_included
#define __string_included

#ifndef __size_t_defined
#define __size_t_defined
#	include <target/size.h>
#endif

#ifndef NULL
# define NULL	( (void*) 0 )
#endif

#ifdef COMPILING_LIBC
#pragma ckf memcpy    f  MayStorePointer MayReadPointedAtObject \
			 MayWritePointedAtObject
#pragma ckf memmove   f  MayStorePointer MayReadPointedAtObject \
			 MayWritePointedAtObject
#pragma ckf strcpy    f  MayStorePointer MayReadPointedAtObject \
			 MayWritePointedAtObject
#pragma ckf strncpy   f  MayStorePointer MayReadPointedAtObject \
			 MayWritePointedAtObject
#pragma ckf strcat    f  MayStorePointer MayReadPointedAtObject \
			 MayWritePointedAtObject
#pragma ckf strncat   f  MayStorePointer MayReadPointedAtObject \
			 MayWritePointedAtObject
#pragma ckf memcmp    f  MayReadPointedAtObject
#pragma ckf strcmp    f  MayReadPointedAtObject
#pragma ckf strcoll   f  MayReadPointedAtObject
#pragma ckf strncmp   f  MayReadPointedAtObject
#pragma ckf strxfrm   f  MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf memchr    f  MayStorePointer MayReadPointedAtObject
#pragma ckf strchr    f  MayStorePointer MayReadPointedAtObject
#pragma ckf strcspn   f  MayReadPointedAtObject
#pragma ckf strpbrk   f  MayStorePointer MayReadPointedAtObject
#pragma ckf strrchr   f  MayStorePointer MayReadPointedAtObject
#pragma ckf strspn    f  MayReadPointedAtObject
#pragma ckf strstr    f  MayStorePointer MayReadPointedAtObject
#pragma ckf strtok    f  MayHaveOtherEffect MayStorePointer \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf memset    f  MayStorePointer MayWritePointedAtObject
#pragma ckf strerror  f  MayReadGlobalObject MayWriteGlobalObject
#pragma ckf strlen    f  MayReadPointedAtObject
#else
#pragma ckf memcpy    memcpy    f  MayStorePointer MayReadPointedAtObject \
                                   MayWritePointedAtObject
#pragma ckf memmove   memmove   f  MayStorePointer MayReadPointedAtObject \
                                   MayWritePointedAtObject
#pragma ckf strcpy    strcpy    f  MayStorePointer MayReadPointedAtObject \
                                   MayWritePointedAtObject
#pragma ckf strncpy   strncpy   f  MayStorePointer MayReadPointedAtObject \
                                   MayWritePointedAtObject
#pragma ckf strcat    strcat    f  MayStorePointer MayReadPointedAtObject \
                                   MayWritePointedAtObject
#pragma ckf strncat   strncat   f  MayStorePointer MayReadPointedAtObject \
                                   MayWritePointedAtObject
#pragma ckf memcmp    memcmp    f  MayReadPointedAtObject
#pragma ckf strcmp    strcmp    f  MayReadPointedAtObject
#pragma ckf strcoll   strcoll   f  MayReadPointedAtObject
#pragma ckf strncmp   strncmp   f  MayReadPointedAtObject
#pragma ckf strxfrm   strxfrm   f  MayWritePointedAtObject MayReadPointedAtObject
#pragma ckf memchr    memchr    f  MayStorePointer MayReadPointedAtObject
#pragma ckf strchr    strchr    f  MayStorePointer MayReadPointedAtObject
#pragma ckf strcspn   strcspn   f  MayReadPointedAtObject
#pragma ckf strpbrk   strpbrk   f  MayStorePointer MayReadPointedAtObject
#pragma ckf strrchr   strrchr   f  MayStorePointer MayReadPointedAtObject
#pragma ckf strspn    strspn    f  MayReadPointedAtObject
#pragma ckf strstr    strstr    f  MayStorePointer MayReadPointedAtObject
#pragma ckf strtok    strtok    f  MayHaveOtherEffect MayStorePointer \
				   MayReadPointedAtObject \
				   MayWritePointedAtObject
#pragma ckf memset    memset    f  MayStorePointer MayWritePointedAtObject
#pragma ckf strerror  strerror  f  MayHaveOtherEffect
#pragma ckf strlen    strlen    f  MayReadPointedAtObject
#endif

void    *memcpy (void *, const void *, size_t);
void    *memmove (void *, const void *, size_t);
char    *strcpy (char *, const char *);
char    *strncpy (char *, const char *, size_t);

char    *strcat (char *, const char *);
char    *strncat (char *, const char *, size_t);

int     memcmp (const void *, const void *, size_t);
int     strcmp (const char *, const char *);
int     strcoll (const char *, const char *);
int     strncmp (const char *, const char *, size_t);
size_t  strxfrm (char *, const char *, size_t);

void    *memchr (const void *, int, size_t);
char    *strchr (const char *, int);
size_t  strcspn (const char *, const char *);
char    *strpbrk (const char *, const char *);
char    *strrchr (const char *, int);
size_t  strspn (const char *, const char *);
char    *strstr (const char *, const char *);
char    *strtok (char *, const char *);

void    *memset (void *, int, size_t);
char    *strerror (int);
size_t  strlen (const char *);

#endif  /* __string_included */
