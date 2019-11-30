/*
 * @(#)stdfix.h	1800.54 11/07/25
 * 
 * COPYRIGHT NOTICE (NOT TO BE REMOVED):
 * 
 * This file, or parts of it, or modified versions of it, may not be copied,
 * reproduced or transmitted in any form, including reprinting, translation,
 * photocopying or microfilming, or by any means, electronic, mechanical or
 * otherwise, or stored in a retrieval system, or used for any purpose, without
 * the prior written permission of all Owners unless it is explicitly marked as
 * having Classification `Public'.
 *   Distribution: Run-Time Module, Object.
 * 
 * Owners of this file give notice:
 *   (c) Copyright 2004,2005 ACE Associated Compiler Experts bv
 *   (c) Copyright 2004,2005 ACE Associated Computer Experts bv
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

#ifndef __stdfix_included
#define __stdfix_included 1

/* keywords:
 *	_Fract _Accum _Sat	[fract accum sat, via this include]
 *
 * (constant) suffixes:
 *	[u]h[r|k]	[unsigned] short [fract|accum]
 *	[u][r|k]	[unsigned] [fract|accum]
 *	[u]l[r|k]	[unsigned] long [fract|accum]
 */

/* raise conflicts where possible, to enforce "simple" keywords: */

#define fract	_Fract
#define accum	_Accum
#define sat	_Sat

/* Supported _Modwrap extension */
#ifdef __EMBEDDEDC_MODWRAP__
#define	modwrap	_Modwrap
#endif

/* Supported _Circ extension */
#ifdef __EMBEDDEDC_CIRC__
#define	circ	_Circ
#endif

#ifndef __C99_RESTRICT__
#define restrict /* _Restrict */
#endif

#include "target/fixlimits.h"	/* by genlimits generated */

#ifdef COMPILING_LIBC
#pragma ckf mulir      f
#pragma ckf mulilr     f
#pragma ckf mulik      f
#pragma ckf mulilk     f
#pragma ckf divir      f
#pragma ckf divilr     f
#pragma ckf divik      f
#pragma ckf divilk     f
#pragma ckf rdivi      f
#pragma ckf lrdivi     f
#pragma ckf kdivi      f
#pragma ckf lkdivi     f
#pragma ckf idivr      f
#pragma ckf idivlr     f
#pragma ckf idivk      f
#pragma ckf idivlk     f
#pragma ckf muliur     f
#pragma ckf muliulr    f
#pragma ckf muliuk     f
#pragma ckf muliulk    f
#pragma ckf diviur     f
#pragma ckf diviulr    f
#pragma ckf diviuk     f
#pragma ckf diviulk    f
#pragma ckf urdivi     f
#pragma ckf ulrdivi    f
#pragma ckf ukdivi     f
#pragma ckf ulkdivi    f
#pragma ckf idivur     f
#pragma ckf idivulr    f
#pragma ckf idivuk     f
#pragma ckf idivulk    f
#pragma ckf abshr      f
#pragma ckf absr       f
#pragma ckf abslr      f
#pragma ckf abshk      f
#pragma ckf absk       f
#pragma ckf abslk      f
#pragma ckf roundhr    f
#pragma ckf roundr     f
#pragma ckf roundlr    f
#pragma ckf roundhk    f
#pragma ckf roundk     f
#pragma ckf roundlk    f
#pragma ckf rounduhr   f
#pragma ckf roundur    f
#pragma ckf roundulr   f
#pragma ckf rounduhk   f
#pragma ckf rounduk    f
#pragma ckf roundulk   f
#pragma ckf countlshr  f
#pragma ckf countlsr   f
#pragma ckf countlslr  f
#pragma ckf countlshk  f
#pragma ckf countlsk   f
#pragma ckf countlslk  f
#pragma ckf countlsuhr f
#pragma ckf countlsur  f
#pragma ckf countlsulr f
#pragma ckf countlsuhk f
#pragma ckf countlsuk  f
#pragma ckf countlsulk f
#pragma ckf bitshr     f
#pragma ckf bitsr      f
#pragma ckf bitslr     f
#pragma ckf bitshk     f
#pragma ckf bitsk      f
#pragma ckf bitslk     f
#pragma ckf bitsuhr    f
#pragma ckf bitsur     f
#pragma ckf bitsulr    f
#pragma ckf bitsuhk    f
#pragma ckf bitsuk     f
#pragma ckf bitsulk    f
#pragma ckf hrbits     f
#pragma ckf rbits      f
#pragma ckf lrbits     f
#pragma ckf hkbits     f
#pragma ckf kbits      f
#pragma ckf lkbits     f
#pragma ckf uhrbits    f
#pragma ckf urbits     f
#pragma ckf ulrbits    f
#pragma ckf uhkbits    f
#pragma ckf ukbits     f
#pragma ckf ulkbits    f
#pragma ckf strtofxhr  f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxr   f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxlr  f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxhk  f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxk   f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxlk  f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxuhr f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxur  f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxulr f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxuhk f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxuk  f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#pragma ckf strtofxulk f MayReadGlobalObject MayWriteGlobalObject \
			 MayReadPointedAtObject MayWritePointedAtObject
#else
#pragma ckf mulir      mulir      f
#pragma ckf mulilr     mulilr     f
#pragma ckf mulik      mulik      f
#pragma ckf mulilk     mulilk     f
#pragma ckf divir      divir      f
#pragma ckf divilr     divilr     f
#pragma ckf divik      divik      f
#pragma ckf divilk     divilk     f
#pragma ckf rdivi      rdivi      f
#pragma ckf lrdivi     lrdivi     f
#pragma ckf kdivi      kdivi      f
#pragma ckf lkdivi     lkdivi     f
#pragma ckf idivr      idivr      f
#pragma ckf idivlr     idivlr     f
#pragma ckf idivk      idivk      f
#pragma ckf idivlk     idivlk     f
#pragma ckf muliur     muliur     f
#pragma ckf muliulr    muliulr    f
#pragma ckf muliuk     muliuk     f
#pragma ckf muliulk    muliulk    f
#pragma ckf diviur     diviur     f
#pragma ckf diviulr    diviulr    f
#pragma ckf diviuk     diviuk     f
#pragma ckf diviulk    diviulk    f
#pragma ckf urdivi     urdivi     f
#pragma ckf ulrdivi    ulrdivi    f
#pragma ckf ukdivi     ukdivi     f
#pragma ckf ulkdivi    ulkdivi    f
#pragma ckf idivur     idivur     f
#pragma ckf idivulr    idivulr    f
#pragma ckf idivuk     idivuk     f
#pragma ckf idivulk    idivulk    f
#pragma ckf abshr      abshr      f
#pragma ckf absr       absr       f
#pragma ckf abslr      abslr      f
#pragma ckf abshk      abshk      f
#pragma ckf absk       absk       f
#pragma ckf abslk      abslk      f
#pragma ckf roundhr    roundhr    f
#pragma ckf roundr     roundr     f
#pragma ckf roundlr    roundlr    f
#pragma ckf roundhk    roundhk    f
#pragma ckf roundk     roundk     f
#pragma ckf roundlk    roundlk    f
#pragma ckf rounduhr   rounduhr   f
#pragma ckf roundur    roundur    f
#pragma ckf roundulr   roundulr   f
#pragma ckf rounduhk   rounduhk   f
#pragma ckf rounduk    rounduk    f
#pragma ckf roundulk   roundulk   f
#pragma ckf countlshr  countlshr  f
#pragma ckf countlsr   countlsr   f
#pragma ckf countlslr  countlslr  f
#pragma ckf countlshk  countlshk  f
#pragma ckf countlsk   countlsk   f
#pragma ckf countlslk  countlslk  f
#pragma ckf countlsuhr countlsuhr f
#pragma ckf countlsur  countlsur  f
#pragma ckf countlsulr countlsulr f
#pragma ckf countlsuhk countlsuhk f
#pragma ckf countlsuk  countlsuk  f
#pragma ckf countlsulk countlsulk f
#pragma ckf bitshr     bitshr     f
#pragma ckf bitsr      bitsr      f
#pragma ckf bitslr     bitslr     f
#pragma ckf bitshk     bitshk     f
#pragma ckf bitsk      bitsk      f
#pragma ckf bitslk     bitslk     f
#pragma ckf bitsuhr    bitsuhr    f
#pragma ckf bitsur     bitsur     f
#pragma ckf bitsulr    bitsulr    f
#pragma ckf bitsuhk    bitsuhk    f
#pragma ckf bitsuk     bitsuk     f
#pragma ckf bitsulk    bitsulk    f
#pragma ckf hrbits     hrbits     f
#pragma ckf rbits      rbits      f
#pragma ckf lrbits     lrbits     f
#pragma ckf hkbits     hkbits     f
#pragma ckf kbits      kbits      f
#pragma ckf lkbits     lkbits     f
#pragma ckf uhrbits    uhrbits    f
#pragma ckf urbits     urbits     f
#pragma ckf ulrbits    ulrbits    f
#pragma ckf uhkbits    uhkbits    f
#pragma ckf ukbits     ukbits     f
#pragma ckf ulkbits    ulkbits    f
#pragma ckf strtofxhr  strtofxhr  f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxr   strtofxr   f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxlr  strtofxlr  f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxhk  strtofxhk  f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxk   strtofxk   f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxlk  strtofxlk  f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxuhr strtofxuhr f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxur  strtofxur  f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxulr strtofxulr f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxuhk strtofxuhk f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxuk  strtofxuk  f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#pragma ckf strtofxulk strtofxulk f MayReadGlobalObject MayWriteGlobalObject \
				    MayReadPointedAtObject \
				    MayWritePointedAtObject
#endif

/* 7.18a.6: saturating mul/div function prototypes */

/* Undefined Behavior: of the divs, when 2nd operand is 0, 0 is returned */

int mulir( int, fract );
long int mulilr( long int, long fract );
int mulik( int, accum );
long int mulilk( long int, long accum );

int divir( int, fract );
long int divilr( long int, long fract );
int divik( int, accum );
long int divilk( long int, long accum );

fract rdivi( int, int );
long fract lrdivi( long int, long int );
accum kdivi( int, int );
long accum lkdivi( long int, long int );

int idivr( fract, fract );
long int idivlr( long fract, long fract );
int idivk( accum, accum );
long int idivlk( long accum, long accum );

unsigned int muliur( unsigned int, unsigned fract );
unsigned long int muliulr( unsigned long int, unsigned long fract );
unsigned int muliuk( unsigned int, unsigned accum );
unsigned long int muliulk( unsigned long int, unsigned long accum );

unsigned int diviur( unsigned int, unsigned fract );
unsigned long int diviulr( unsigned long int, unsigned long fract );
unsigned int diviuk( unsigned int, unsigned accum );
unsigned long int diviulk( unsigned long int, unsigned long accum );

unsigned fract urdivi( unsigned int, unsigned int );
unsigned long fract ulrdivi( unsigned long int, unsigned long int );
unsigned accum ukdivi( unsigned int, unsigned int );
unsigned long accum ulkdivi( unsigned long int, unsigned long int );

unsigned int idivur( unsigned fract, unsigned fract );
unsigned long int idivulr( unsigned long fract, unsigned long fract );
unsigned int idivuk( unsigned accum, unsigned accum );
unsigned long int idivulk( unsigned long accum, unsigned long accum );


/* ISO/IEC TR 18037:2004(E), 7.18a.6.2 */

#pragma typegeneric __absfx
short fract abshr( short fract );
#pragma typegeneric __absfx
fract absr( fract );
#pragma typegeneric __absfx
long fract abslr( long fract );
#pragma typegeneric __absfx
short accum abshk( short accum );
#pragma typegeneric __absfx
accum absk( accum );
#pragma typegeneric __absfx
long accum abslk( long accum );

#define absfx(x)	__absfx(x)


/* ISO/IEC TR 18037:2004(E), 7.18a.6.3 */

#pragma typegeneric __roundfx
short fract roundhr( short fract , int );
#pragma typegeneric __roundfx
fract roundr( fract , int );
#pragma typegeneric __roundfx
long fract roundlr( long fract , int );
#pragma typegeneric __roundfx
short accum roundhk( short accum , int );
#pragma typegeneric __roundfx
accum roundk( accum , int );
#pragma typegeneric __roundfx
long accum roundlk( long accum , int );

#pragma typegeneric __roundfx
unsigned short fract rounduhr( unsigned short fract , int );
#pragma typegeneric __roundfx
unsigned fract roundur( unsigned fract , int );
#pragma typegeneric __roundfx
unsigned long fract roundulr( unsigned long fract , int );
#pragma typegeneric __roundfx
unsigned short accum rounduhk( unsigned short accum , int );
#pragma typegeneric __roundfx
unsigned accum rounduk( unsigned accum , int );
#pragma typegeneric __roundfx
unsigned long accum roundulk( unsigned long accum , int );

#define roundfx(x,y)	__roundfx(x,y)


/* ISO/IEC TR 18037:2004(E), 7.18a6.4 */

#pragma typegeneric __countlsfx
int countlshr( short fract );
#pragma typegeneric __countlsfx
int countlsr( fract );
#pragma typegeneric __countlsfx
int countlslr( long fract );
#pragma typegeneric __countlsfx
int countlshk( short accum );
#pragma typegeneric __countlsfx
int countlsk( accum );
#pragma typegeneric __countlsfx
int countlslk( long accum );

#pragma typegeneric __countlsfx
int countlsuhr( unsigned short fract );
#pragma typegeneric __countlsfx
int countlsur( unsigned fract );
#pragma typegeneric __countlsfx
int countlsulr( unsigned long fract );
#pragma typegeneric __countlsfx
int countlsuhk( unsigned short accum );
#pragma typegeneric __countlsfx
int countlsuk( unsigned accum );
#pragma typegeneric __countlsfx
int countlsulk( unsigned long accum );

#define countlsfx(x)	__countlsfx(x)


/* ISO/IEC TR 18037:2004(E), 7.18.6.5 */

int_hr_t bitshr( short fract );
int_r_t bitsr( fract );
int_lr_t bitslr( long fract );
int_hk_t bitshk( short accum );
int_k_t bitsk( accum );
int_lk_t bitslk( long accum );

uint_uhr_t bitsuhr( unsigned short fract );
uint_ur_t bitsur( unsigned fract );
uint_ulr_t bitsulr( unsigned long fract );
uint_uhk_t bitsuhk( unsigned short accum );
uint_uk_t bitsuk( unsigned accum );
uint_ulk_t bitsulk( unsigned long accum );


/* ISO/IEC TR 18037:2004(E), 7.18.6.6 */

short fract hrbits( int_hr_t );
fract rbits( int_r_t );
long fract lrbits( int_lr_t );
short accum hkbits( int_hk_t );
accum kbits( int_k_t );
long accum lkbits( int_lk_t );

unsigned short fract uhrbits( uint_uhr_t );
unsigned fract urbits( uint_ur_t );
unsigned long fract ulrbits( uint_ulr_t );
unsigned short accum uhkbits( uint_uhk_t );
unsigned accum ukbits( uint_uk_t );
unsigned long accum ulkbits( uint_ulk_t );

/* ISO/IEC TR 18037:2004(E), 7.18.6.7 */

/* see above clauses for typegeneric functions */


/* ISO/IEC TR 18037:2004(E), 7.18.6.8, with strto -> strtofx */

short fract strtofxhr( const char * restrict , char ** restrict );
fract strtofxr( const char * restrict , char ** restrict );
long fract strtofxlr( const char * restrict , char ** restrict );

short accum strtofxhk( const char * restrict , char ** restrict );
accum strtofxk( const char * restrict , char ** restrict );
long accum strtofxlk( const char * restrict , char ** restrict );

unsigned short fract strtofxuhr( const char * restrict , char ** restrict );
unsigned fract strtofxur( const char * restrict , char ** restrict );
unsigned long fract strtofxulr( const char * restrict , char ** restrict );

unsigned short accum strtofxuhk( const char * restrict , char ** restrict );
unsigned accum strtofxuk( const char * restrict , char ** restrict );
unsigned long accum strtofxulk( const char * restrict , char ** restrict );

#endif /* never before __stdfix_included */
