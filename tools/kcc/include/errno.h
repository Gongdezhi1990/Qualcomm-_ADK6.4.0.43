/*
 * @(#)errno.h	1800.54 11/07/25
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
 *   (c) Copyright 1998 ACE Associated Compiler Experts bv
 *   (c) Copyright 1991-1994,1998 ACE Associated Computer Experts bv
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

#ifndef __errno_included
#define __errno_included

#define EPERM	1	/* Not super-user			*/
#define ENOENT	2	/* No such file or directory		*/
#define ESRCH	3	/* No such process			*/
#define EINTR	4	/* interrupted system call		*/
#define EIO	5	/* I/O error				*/
#define ENXIO	6	/* No such device or address		*/
#define E2BIG	7	/* Arg list too long			*/
#define ENOEXEC	8	/* Exec format error			*/
#define EBADF	9	/* Bad file number			*/
#define ECHILD	10	/* No children				*/
#define EAGAIN	11	/* No more processes			*/
#define ENOMEM	12	/* Not enough core			*/
#define EACCES	13	/* Permission denied			*/
#define EFAULT	14	/* Bad address				*/
#define ENOTBLK	15	/* Block device required		*/
#define EBUSY	16	/* Mount device busy			*/
#define EEXIST	17	/* File exists				*/
#define EXDEV	18	/* Cross-device link			*/
#define ENODEV	19	/* No such device			*/
#define ENOTDIR	20	/* Not a directory			*/
#define EISDIR	21	/* Is a directory			*/
#define EINVAL	22	/* Invalid argument			*/
#define ENFILE	23	/* File table overflow			*/
#define EMFILE	24	/* Too many open files			*/
#define ENOTTY	25	/* Not a typewriter			*/
#define ETXTBSY	26	/* Text file busy			*/
#define EFBIG	27	/* File too large			*/
#define ENOSPC	28	/* No space left on device		*/
#define ESPIPE	29	/* Illegal seek				*/
#define EROFS	30	/* Read only file system		*/
#define EMLINK	31	/* Too many links			*/
#define EPIPE	32	/* Broken pipe				*/
#define EDOM	33	/* Math arg out of domain of func	*/
#define ERANGE	34	/* Math result not representable	*/

#ifdef MULTI_THREADED	/* define this for multi-threaded implementation */
extern int *__errno(void);
#define errno   (* __errno ())
#endif

#ifndef errno
extern int	errno;	/* otherwise, errno is just a global variable */
#endif

#endif  /* __errno_included */
