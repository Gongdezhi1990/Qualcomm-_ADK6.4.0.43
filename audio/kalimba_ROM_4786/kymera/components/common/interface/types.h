/**************************************************************************** 
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 *
 ************************************************************************//** 
 * \file types.h
 * Type definitions for Kalimba
 *
 * MODULE : Types
 *
 ****************************************************************************/

#ifndef TYPES_H
#define TYPES_H

/****************************************************************************
Include Files
*/

#include <stddef.h>

#ifdef __GNUC__
#include <stdint.h>
#else
typedef unsigned int uintptr_t;
#endif

/****************************************************************************
Public Macro Declarations
*/

#define NOT_USED(x) (void)(x)

/* Define NULL if not already defined */
#ifndef NULL
#define NULL ((void *) 0)
#endif

/****************************************************************************
Public Type Declarations
*/

/**
 * \name global_types
 * Natural word size is 24 bits on Kalimba. C compiler allows use of only 24, 48 
 * or 72 bit types. So define only these types.
 * 
 * NOTE - in general use int for types that must be at least 16 bits, and uint24
 * types that must be at least 24 bits. Then our code should port from Kalimba to
 * a 16 bit machine (eg Xap) or a 32 bit machine Crescendo Kalimba or PC
 * reasonably easily
 */
/**@{*/
typedef unsigned int    uint24;

typedef int      int24;

/* Typedef uint8, uint16 and uint32 to aid clarity when dealing with hardware
 * registers and hardware related function parameters.
 * Generally we use the next-largest if exact size not available.
 */

#if defined(__GNUC__) || defined(K32)

/* Fixed-size types for GCC & K32 builds. */
#ifdef __GNUC__
typedef uint64_t               uint48;
typedef int64_t                int48;
typedef int64_t                int64;
typedef uint64_t               uint64;
#else /* K32 */
typedef long long            int64;
typedef unsigned long long   uint64;
typedef int64                int48;
typedef uint64               uint48;
#endif

typedef unsigned char          uint8;

typedef unsigned short         uint16;
typedef unsigned int           uint32;

typedef signed char            int8;
typedef short                  int16;
typedef int                    int32;

typedef uint32          CHIP_VERSION_T;

#else /* defined(__GNUC__) || defined(K32) */

/* Fixed-size types for KCC builds. */
typedef unsigned long   uint48;
typedef long            int48;

typedef uint24          uint8;
typedef uint24          uint16;
typedef uint48          uint32;

typedef int24           int8;
typedef int24           int16;
typedef int48           int32;

typedef long long                int64;
typedef unsigned long long       uint64;

typedef uint24          CHIP_VERSION_T;

#endif /* defined(__GNUC__) || defined(K32) */

#ifndef __cplusplus
typedef unsigned int             bool;
#endif
/**@}*/

#ifndef TRUE
#define TRUE 1
#endif

#ifndef FALSE
#define FALSE 0
#endif

typedef uint32 TIME;
typedef int32 TIME_INTERVAL;

/****************************************************************************
Global Variable Definitions
*/

/****************************************************************************
Public Function Prototypes
*/
#endif
