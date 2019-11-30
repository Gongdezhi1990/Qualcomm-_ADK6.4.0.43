/*
 * @(#)assert.h	1800.54 11/07/25
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
 *   (c) Copyright 1998,2003,2008 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998,2003,2008 ACE Associated Computer Experts bv
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

#undef  assert

#ifdef  NDEBUG
#   define  assert(arg) ((void) 0)
#else
#   ifndef ASSERT_PRAGMA_CKF
#      define ASSERT_PRAGMA_CKF
#      pragma ckf __assert f	DoesNotReturn \
  			MayNotReturn \
			MayHaveOtherEffect \
			MayReadGlobalObject \
			MayWriteGlobalObject \
			MayReadPointedAtObject
#   endif
    void    __assert (const char *, const char *, int);
#   define  assert(arg) (!(arg) ? __assert (#arg, __FILE__, __LINE__) : (void) 0)
#endif
