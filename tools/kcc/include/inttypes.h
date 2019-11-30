/*
 * @(#)inttypes.h	1801.1 15/08/27
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
 *   (c) Copyright 2010-2011,2013 ACE Associated Computer Experts bv
 *   (c) Copyright 2010-2011,2013 ACE Associated Compiler Experts bv
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

#ifndef __inttypes_included
#define __inttypes_included

/* as per decrete:
 */
#include <stdint.h>
#include <stddef.h>

/* printf and scanf assist:
 */
#include <target/prtmacros.h>

/* combined result return value:
 */
typedef struct
{	intmax_t	quot;
	intmax_t	rem;
} imaxdiv_t ;


#if defined( __C99_LIBC__ )
 /* restrict implied */
#elif !defined( __C99_RESTRICT__ )
# undef restrict
# define restrict
#endif

intmax_t imaxabs( intmax_t );
imaxdiv_t imaxdiv( intmax_t , intmax_t );
intmax_t strtoimax( const char * restrict , char ** restrict , int );
uintmax_t strtoumax( const char * restrict , char ** restrict , int );
intmax_t wcstoimax( const wchar_t * restrict , wchar_t ** restrict , int );
uintmax_t wcstoumax( const wchar_t * restrict , wchar_t ** restrict , int );

#endif /* __inttypes_included */
