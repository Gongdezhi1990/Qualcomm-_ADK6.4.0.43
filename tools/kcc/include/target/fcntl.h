/*
 * @(#)fcntl.h	1800.11 08/09/24
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
 *   (c) Copyright 2003 ACE Associated Compiler Experts bv
 *   (c) Copyright 2003 ACE Associated Computer Experts bv
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
 *
 * Contents of this file are used by the 'fopen' function, and should match the
 * libOS choosen implementation values, they represent the encoding of the
 * open modes of a file.
 */

#define O_RDONLY    0x0001              /* read only */
#define O_WRONLY    0x0002              /* write only */
#define O_RDWR      0x0004              /* read/write enable */
#define O_CREAT     0x0100              /* create and open file */
#define O_TRUNC     0x0200              /* open with truncation */
#define O_APPEND    0x0800              /* to end of file */
#define O_BINARY    0x8000              /* no translation */
