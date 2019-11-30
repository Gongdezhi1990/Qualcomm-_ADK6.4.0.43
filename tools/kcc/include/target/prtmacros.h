/*
 * @(#)prtmacros.h	1801.1 15/08/27
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
 *   (c) Copyright 2013 ACE Associated Computer Experts bv
 *   (c) Copyright 2013 ACE Associated Compiler Experts bv
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

/* printf and scanf assist:
 */

/* intptr_t:
 */
#define PRIdPTR		"d"
#define PRIiPTR		"i"
#define PRIoPTR		"o"
#define PRIuPTR		"u"
#define PRIxPTR		"x"
#define PRIXPTR		"X"

#define SCNdPTR		"d"
#define SCNiPTR		"i"
#define SCNoPTR		"o"
#define SCNuPTR		"u"
#define SCNxPTR		"x"

/* intmax_t: */
#define PRIdMAX		"lld"
#define PRIiMAX		"lli"
#define PRIoMAX		"llo"
#define PRIuMAX		"llu"
#define PRIxMAX		"llx"
#define PRIXMAX		"llX"

#define SCNdMAX		"lld"
#define SCNiMAX		"lli"
#define SCNoMAX		"llo"
#define SCNuMAX		"llu"
#define SCNxMAX		"llx"

/* 8-bit thingees:
 */
#define PRId8		"d"
#define PRIdLEAST8	"d"
#define PRIdFAST8	"d"
#define PRIi8		"i"
#define PRIiLEAST8	"i"
#define PRIiFAST8	"i"
#define PRIo8		"o"
#define PRIoLEAST8	"o"
#define PRIoFAST8	"o"
#define PRIu8		"u"
#define PRIuLEAST8	"u"
#define PRIuFAST8	"u"
#define PRIx8		"x"
#define PRIxLEAST8	"x"
#define PRIxFAST8	"x"
#define PRIX8		"X"
#define PRIXLEAST8	"X"
#define PRIXFAST8	"X"

#define SCNd8		"d"
#define SCNdLEAST8	"d"
#define SCNdFAST8	"d"
#define SCNi8		"i"
#define SCNiLEAST8	"i"
#define SCNiFAST8	"i"
#define SCNo8		"o"
#define SCNoLEAST8	"o"
#define SCNoFAST8	"o"
#define SCNu8		"u"
#define SCNuLEAST8	"u"
#define SCNuFAST8	"u"
#define SCNx8		"x"
#define SCNxLEAST8	"x"
#define SCNxFAST8	"x"

/* 16-bit thingees:
 */
#define PRId16		"d"
#define PRIdLEAST16	"d"
#define PRIdFAST16	"d"
#define PRIi16		"i"
#define PRIiLEAST16	"i"
#define PRIiFAST16	"i"
#define PRIo16		"o"
#define PRIoLEAST16	"o"
#define PRIoFAST16	"o"
#define PRIu16		"u"
#define PRIuLEAST16	"u"
#define PRIuFAST16	"u"
#define PRIx16		"x"
#define PRIxLEAST16	"x"
#define PRIxFAST16	"x"
#define PRIX16		"X"
#define PRIXLEAST16	"X"
#define PRIXFAST16	"X"

#define SCNd16		"d"
#define SCNdLEAST16	"d"
#define SCNdFAST16	"d"
#define SCNi16		"i"
#define SCNiLEAST16	"i"
#define SCNiFAST16	"i"
#define SCNo16		"o"
#define SCNoLEAST16	"o"
#define SCNoFAST16	"o"
#define SCNu16		"u"
#define SCNuLEAST16	"u"
#define SCNuFAST16	"u"
#define SCNx16		"x"
#define SCNxLEAST16	"x"
#define SCNxFAST16	"x"

/* 32-bit thingees:
 */
#define PRId32		"ld"
#define PRIdLEAST32	"ld"
#define PRIdFAST32	"ld"
#define PRIi32		"li"
#define PRIiLEAST32	"li"
#define PRIiFAST32	"li"
#define PRIo32		"lo"
#define PRIoLEAST32	"lo"
#define PRIoFAST32	"lo"
#define PRIu32		"lu"
#define PRIuLEAST32	"lu"
#define PRIuFAST32	"lu"
#define PRIx32		"lx"
#define PRIxLEAST32	"lx"
#define PRIxFAST32	"lx"
#define PRIX32		"lX"
#define PRIXLEAST32	"lX"
#define PRIXFAST32	"lX"

#define SCNd32		"ld"
#define SCNdLEAST32	"ld"
#define SCNdFAST32	"ld"
#define SCNi32		"li"
#define SCNiLEAST32	"li"
#define SCNiFAST32	"li"
#define SCNo32		"lo"
#define SCNoLEAST32	"lo"
#define SCNoFAST32	"lo"
#define SCNu32		"lu"
#define SCNuLEAST32	"lu"
#define SCNuFAST32	"lu"
#define SCNx32		"lx"
#define SCNxLEAST32	"lx"
#define SCNxFAST32	"lx"

/* 64-bit thingees:
 */
#define PRId64		"lld"
#define PRIdLEAST64	"lld"
#define PRIdFAST64	"lld"
#define PRIi64		"lli"
#define PRIiLEAST64	"lli"
#define PRIiFAST64	"lli"
#define PRIo64		"llo"
#define PRIoLEAST64	"llo"
#define PRIoFAST64	"llo"
#define PRIu64		"llu"
#define PRIuLEAST64	"llu"
#define PRIuFAST64	"llu"
#define PRIx64		"llx"
#define PRIxLEAST64	"llx"
#define PRIxFAST64	"llx"
#define PRIX64		"llX"
#define PRIXLEAST64	"llX"
#define PRIXFAST64	"llX"

#define SCNd64		"lld"
#define SCNdLEAST64	"lld"
#define SCNdFAST64	"lld"
#define SCNi64		"lli"
#define SCNiLEAST64	"lli"
#define SCNiFAST64	"lli"
#define SCNo64		"llo"
#define SCNoLEAST64	"llo"
#define SCNoFAST64	"llo"
#define SCNu64		"llu"
#define SCNuLEAST64	"llu"
#define SCNuFAST64	"llu"
#define SCNx64		"llx"
#define SCNxLEAST64	"llx"
#define SCNxFAST64	"llx"

