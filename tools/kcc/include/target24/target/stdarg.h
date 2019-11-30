/*
 * @(#)stdarg.h	1800.1 08/06/25
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
 *   (c) Copyright 1998,1999,2003 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998,1999,2003 ACE Associated Computer Experts bv
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

/* This implementation assumes variables are nicely
 * placed on the stack (e.g. structures are passed as structures, not
 * as pointers to a structure), no alignment issues exist, and
 * a next parameter is placed at a lower address (parameters passed from
 * right to left on Kalimba).
 * 
 */
#ifndef __stdarg_target_included
#define __stdarg_target_included

typedef void *va_list;

#define va_start(ap, parmN) ((void) ((ap) = (void *) (&(parmN))))
#define va_arg(ap, type)    (* --(* (type **) &(ap)))
/* ((void)0) implementation results in "constant as statement" warnings */
#define va_end(ap)          /* va_end */;
#define va_copy(aq, ap)     ((aq) = (ap))

#endif
