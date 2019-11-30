/*
 * @(#)setjmp.h	1800.11 08/09/24
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
 *   (c) Copyright 1998,2001-2003 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998,2001-2003 ACE Associated Computer Experts bv
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

/* This file is target dependent, given contents are only an example
 * and should be made compliant to the actual target
 */

typedef struct {
    void    *FramePointer;
    void    *ProgramCounter;
    void    *StackPointer;

    /* this structure needs to be filled with everything that */
    /* needs to be preserved, such as program counter, stack */
    /* pointer, callee-saved registers, etc. */
    int    extra[30];
} jmp_buf[1];
