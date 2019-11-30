/*
 * @(#)stdlib.h	1800.11 08/09/24
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
 *   (c) Copyright 1998,2003-2007 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998,2003-2007 ACE Associated Computer Experts bv
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

#ifndef __stdlib_included
#define __stdlib_included

#ifndef __size_t_defined
#define __size_t_defined
#	include <target/size.h>
#endif

#ifndef __wchar_t_defined
#define __wchar_t_defined
#	include <target/wchar.h>
#endif

typedef struct {
	int		quot;
	int    		 rem;
} div_t;
typedef struct {
	long		quot;
	long		rem;
} ldiv_t;

#ifdef __LONGLONG__
typedef struct {
	long long	quot;
	long long	rem;
} lldiv_t;
#endif /* __LONGLONG__ */

#ifndef NULL
# define NULL	( (void*) 0 )
#endif

#define EXIT_FAILURE    1
#define EXIT_SUCCESS    0

#define RAND_MAX        32767

#define MB_CUR_MAX      1

#ifdef COMPILING_LIBC
#pragma ckf atof      f  MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject
#pragma ckf atoi      f  MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject
#pragma ckf atol      f  MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject
#pragma ckf strtod    f  MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject \
			 MayStorePointer
#pragma ckf strtol    f  MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject \
			 MayStorePointer
#pragma ckf strtoul   f  MayReadPointedAtObject MayWritePointedAtObject \
			 MayWriteGlobalObject
#pragma ckf rand      f  MayReadGlobalObject MayWriteGlobalObject
#pragma ckf srand     f  MayWriteGlobalObject
#pragma ckf bsearch   i
#pragma ckf qsort     i
#pragma ckf abs       f
#pragma ckf div       f
#pragma ckf labs      f
#pragma ckf ldiv      f
#pragma ckf mblen     f  MayReadPointedAtObject
#pragma ckf mbtowc    f  MayReadPointedAtObject
#pragma ckf wctomb    f  MayWritePointedAtObject
#pragma ckf mbstowcs  f  MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf wcstombs  f  MayReadPointedAtObject MayWritePointedAtObject

#ifdef __LONGLONG__
#pragma ckf atoll	f  MayReadGlobalObject MayWriteGlobalObject \
			   MayReadPointedAtObject
#pragma ckf llabs	f
#pragma ckf lldiv	f
#pragma ckf strtoll	f  MayReadGlobalObject MayWriteGlobalObject \
			   MayReadPointedAtObject MayWritePointedAtObject \
			   MayStorePointer
#pragma ckf strtoull	f  MayReadGlobalObject MayWriteGlobalObject \
			   MayReadPointedAtObject MayWritePointedAtObject \
			   MayStorePointer
#endif
#else

#pragma ckf atof      atof      f  MayReadGlobalObject MayWriteGlobalObject \
				   MayReadPointedAtObject
#pragma ckf atoi      atoi      f  MayReadGlobalObject MayWriteGlobalObject \
				   MayReadPointedAtObject
#pragma ckf atol      atol      f  MayReadGlobalObject MayWriteGlobalObject \
				   MayReadPointedAtObject
#pragma ckf strtod    strtod    f  MayReadGlobalObject MayWriteGlobalObject \
			 	   MayReadPointedAtObject \
				   MayWritePointedAtObject
#pragma ckf strtol    strtol    f  MayReadGlobalObject MayWriteGlobalObject \
			 	   MayReadPointedAtObject \
				   MayWritePointedAtObject
#pragma ckf strtoul   strtoul   f  MayReadGlobalObject MayWriteGlobalObject \
			 	   MayReadPointedAtObject \
				   MayWritePointedAtObject
#pragma ckf rand      rand      f  MayHaveOtherEffect
#pragma ckf srand     srand     f  MayHaveOtherEffect
#pragma ckf bsearch   bsearch   f  MayHaveOtherEffect MayAccessVolatile \
				   MayRaiseException MayNotReturn \
				   MayReadGlobalObject MayWriteGlobalObject \
				   MayStorePointer
#pragma ckf qsort     qsort     f  MayHaveOtherEffect MayAccessVolatile \
				   MayRaiseException MayNotReturn \
				   MayReadGlobalObject MayWriteGlobalObject \
				   MayStorePointer
#pragma ckf abs       abs       f  
#pragma ckf div       div       f  MayRaiseException
#pragma ckf labs      labs      f  
#pragma ckf ldiv      ldiv      f  MayRaiseException
#pragma ckf mblen     mblen     f  MayReadPointedAtObject
#pragma ckf mbtowc    mbtowc    f  MayReadPointedAtObject \
				   MayWritePointedAtObject
#pragma ckf wctomb    wctomb    f  MayWritePointedAtObject
#pragma ckf mbstowcs  mbstowcs  f  MayReadPointedAtObject \
				   MayWritePointedAtObject
#pragma ckf wcstombs  wcstombs  f  MayReadPointedAtObject \
				   MayWritePointedAtObject
#ifdef __LONGLONG__
#pragma ckf llabs		f
#pragma ckf lldiv		f  MayRaiseException
#pragma ckf atoll		f  MayReadGlobalObject MayWriteGlobalObject \
				   MayReadPointedAtObject
#pragma ckf strtoll		f  MayReadGlobalObject MayWriteGlobalObject \
				   MayReadPointedAtObject
#pragma ckf strtoull		f  MayReadGlobalObject MayWriteGlobalObject \
				   MayReadPointedAtObject
#endif /* __LONGLONG__ */
#endif

double          atof (const char *);
int             atoi (const char *);
long            atol (const char *);
double          strtod (const char *, char **);
long int        strtol (const char *, char **, int);
unsigned long   strtoul (const char *, char **, int);
long double     strtold (const char *, char **); 

int     rand (void);
void    srand (unsigned int);

void    *bsearch (const void *, const void *, size_t,
		  size_t, int (*) (const void *, const void *));
void    qsort (void *, size_t, size_t,
	       int (*) (const void *, const void *));

int     abs (int);
div_t   div (int, int);
long    labs (long);
ldiv_t  ldiv (long, long);


int     mblen (const char *, size_t);
int     mbtowc (wchar_t *, const char *, size_t);
int     wctomb (char *, wchar_t);
size_t  mbstowcs (wchar_t *, const char *, size_t);
size_t  wcstombs (char *, const wchar_t *, size_t);

#ifdef __LONGLONG__
long long	atoll (const char *);
long long	llabs (long long);
lldiv_t		lldiv (long long, long long);
long long	strtoll (const char *, char **, int);
unsigned long long	strtoull (const char *, char **, int);
#endif /* __LONGLONG__ */

/* No implementations provided */
void    *calloc (size_t, size_t);
void    free (void *);
void    *malloc (size_t);
void    *realloc (void *, size_t);

#endif  /* __stdlib_included */
