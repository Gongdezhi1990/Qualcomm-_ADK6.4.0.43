/*
 * @(#)stddef.h	1800.54 11/07/25
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

#ifndef __stddef_included
#define __stddef_included

#ifndef __ptrdiff_t_defined     
#define __ptrdiff_t_defined     
#	include <target/pdiff.h>
#endif

#ifndef __size_t_defined
#define __size_t_defined
#	include <target/size.h>
#endif

#ifndef __wchar_t_defined
#define __wchar_t_defined
#	include <target/wchar.h>
#endif

#define NULL	( (void*) 0 )

#define offsetof(type,member) \
	((size_t)((char *)(&((type *)0)->member) - (char *)0))

#endif  /* __stddef_included */
