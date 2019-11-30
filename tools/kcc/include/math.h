/*
 * @(#)math.h	1800.54 11/07/25
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
 *   (c) Copyright 1998-2008 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998-2008 ACE Associated Computer Experts bv
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

#ifndef __math_included
#define __math_included

extern double	__huge_val(void);	/* "inf - 1" ? */
#define HUGE_VAL	((double)__INFINITY)
#define HUGE_VALF	((float)__INFINITY)
#define HUGE_VALL	((long double) __INFINITY)

#ifdef COMPILING_LIBC
#pragma ckf acos       f  MayWriteGlobalObject
#pragma ckf asin       f  MayWriteGlobalObject
#pragma ckf atan       f
#pragma ckf atan2      f  MayWriteGlobalObject
#pragma ckf cos        f  MayWriteGlobalObject
#pragma ckf sin        f  MayWriteGlobalObject
#pragma ckf tan        f  MayWriteGlobalObject
#pragma ckf cosh       f  MayWriteGlobalObject
#pragma ckf sinh       f  MayWriteGlobalObject
#pragma ckf tanh       f  MayWriteGlobalObject
#pragma ckf exp	       f  MayWriteGlobalObject
#pragma ckf frexp      f  MayWritePointedAtObject MayWriteGlobalObject \
                          MayReadGlobalObject
#pragma ckf ldexp      f  MayWriteGlobalObject
#pragma ckf log        f  MayWriteGlobalObject MayReadGlobalObject
#pragma ckf log10      f  MayWriteGlobalObject MayReadGlobalObject
#pragma ckf modf       f  MayWritePointedAtObject
#pragma ckf pow        f  MayWriteGlobalObject MayReadGlobalObject
#pragma ckf sqrt       f  MayWriteGlobalObject MayReadGlobalObject
#pragma ckf ceil       f
#pragma ckf fabs       f
#pragma ckf floor      f
#pragma ckf fmod       f  MayWriteGlobalObject MayReadGlobalObject
#else
#pragma ckf acos   acos   f  MayWriteGlobalObject
#pragma ckf asin   asin   f  MayWriteGlobalObject
#pragma ckf atan   atan   f  
#pragma ckf atan2  atan2  f  MayWriteGlobalObject
#pragma ckf cos    cos    f  
#pragma ckf sin    sin    f  
#pragma ckf tan    tan    f  
#pragma ckf cosh   cosh   f  MayWriteGlobalObject
#pragma ckf sinh   sinh   f  MayWriteGlobalObject
#pragma ckf tanh   tanh   f  
#pragma ckf exp	   exp    f  MayWriteGlobalObject
#pragma ckf frexp  frexp  f  MayWritePointedAtObject
#pragma ckf ldexp  ldexp  f  MayWriteGlobalObject
#pragma ckf log    log    f  MayWriteGlobalObject
#pragma ckf log10  log10  f  MayWriteGlobalObject
#pragma ckf modf   modf   f  MayWritePointedAtObject
#pragma ckf pow    pow    f  MayWriteGlobalObject
#pragma ckf sqrt   sqrt   f  MayWriteGlobalObject
#pragma ckf ceil   ceil   f  
#pragma ckf fabs   fabs   f  
#pragma ckf floor  floor  f  
#pragma ckf fmod   fmod   f  MayWriteGlobalObject
#endif


double  acos (double);
double  asin (double);
double  atan (double);
double  atan2 (double, double);
double  cos (double);
double  sin (double);
double  tan (double);
double  cosh (double);
double  sinh (double);
double  tanh (double);
double  exp (double);
double  frexp (double, int *);
double  ldexp (double, int);
double  log (double);
double  log10 (double);
double  modf (double, double *);
double  pow (double, double);
double  sqrt (double);
double  ceil (double);
double  fabs (double);
double  floor (double);
double  fmod (double, double);

/* no ckf numbers assigned yet... */

#pragma ckf fmax	f  
#pragma ckf fmaxf	f  
#pragma ckf fmaxl	f  
double		fmax(double, double);
float		fmaxf(float, float);
long double	fmaxl(long double, long double);
#pragma ckf fmin	f  
#pragma ckf fminf	f  
#pragma ckf fminl	f  
double		fmin(double, double);
float		fminf(float, float);
long double	fminl(long double, long double);
#pragma ckf fdim	f  
#pragma ckf fdimf	f  
#pragma ckf fdiml	f  
double		fdim(double, double);
float		fdimf(float, float);
long double	fdiml(long double, long double);

#pragma ckf exp2	f  MayWriteGlobalObject
#pragma ckf exp2f	f  MayWriteGlobalObject
double		exp2(double);
float		exp2f(float);

#pragma ckf fabsf	f
#pragma ckf fabsl	f
float		fabsf(float);
long double	fabsl(long double);
#pragma ckf sqrtf	f MayWriteGlobalObject MayReadGlobalObject
#pragma ckf sqrtl	f MayWriteGlobalObject MayReadGlobalObject
float		sqrtf(float);
long double	sqrtl(long double);
#pragma ckf ceilf	f
#pragma ckf ceill	f
float		ceilf(float);
long double	ceill(long double);
#pragma ckf floorf	f
#pragma ckf floorl	f
float		floorf(float);
long double	floorl(long double);
#pragma ckf frexpf	f MayWritePointedAtObject
#pragma ckf frexpl	f MayWritePointedAtObject
float		frexpf(float, int *);
long double	frexpl(long double, int * );
#pragma ckf ldexpf	f MayWriteGlobalObject
#pragma ckf ldexpl	f MayWriteGlobalObject 
float		ldexpf(float, int );
long double	ldexpl(long double, int);
#pragma ckf modff	f MayWritePointedAtObject
#pragma ckf modfl	f MayWritePointedAtObject
float		modff(float, float *);
long double		modfl(long double, long double *);
#pragma ckf fmodf	f MayWriteGlobalObject
float		fmodf(float, float);
#pragma ckf sinf	f
float		sinf(float);
#pragma ckf cosf	f
float		cosf(float);
#pragma ckf tanf	f MayWriteGlobalObject
float		tanf(float);
#pragma ckf coshf	f MayWriteGlobalObject
float		coshf(float);
#pragma ckf sinhf	f MayWriteGlobalObject
float		sinhf(float);
#pragma ckf tanhf	f MayWriteGlobalObject
float		tanhf(float);
#pragma ckf expf	f MayWriteGlobalObject
float		expf(float);
#pragma ckf logf	f MayWriteGlobalObject
float		logf(float);
#pragma ckf log10f	f MayWriteGlobalObject
float		log10f(float);
#pragma ckf powf	f MayWriteGlobalObject
float		powf(float, float);
#pragma ckf atanf	f 
float		atanf(float);
#pragma ckf atan2f	f MayWriteGlobalObject
float		atan2f(float, float);
#pragma ckf asinf	f MayWriteGlobalObject
float		asinf(float);
#pragma ckf acosf	f MayWriteGlobalObject
float		acosf(float);

#endif  /* __math_included */
