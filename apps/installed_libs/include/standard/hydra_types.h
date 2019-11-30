#ifndef HYDRA_TYPES_H
#define HYDRA_TYPES_H

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */

/**
 * This the types definition file used by the CDA device common firmware
 * itself.
 */
#include "types.h"

#ifdef __KALIMBA__
#ifdef HAVE_32BIT_DATA_WIDTH
typedef uint32          uint16f;
typedef uint32          uint24;
#else /* HAVE_32BIT_DATA_WIDTH */
typedef uint24          uint16f;
#endif
#else /* XAP */
typedef uint16          uint16f;
typedef uint32          uint24;
#endif

/* For convenience */
#define UINT_BIT ((unsigned)CHAR_BIT * sizeof(unsigned int)/sizeof(char))
#define ULONG_BIT ((unsigned)CHAR_BIT * sizeof(unsigned long)/sizeof(char))

/**
 * 4-bit wide field within a variable that is at least 16 bits wide.
 */
typedef uint8 uint4;
/**
 * Single bit field within a variable that is at least 16 bits wide.
 */
typedef uint8 Bit;

typedef uint8 uint2;
typedef uint8 uint7;
typedef uint16 uint11;
typedef uint16 uint12;

typedef uint16 NullTermString;
typedef uint16 NULL_TERM_STRING;
/*@}*/

/** Allow us to pass arguments to panic/fault diatribe
 * of the native type
 */
#define DIATRIBE_TYPE           uint32

#endif /* HYDRA_TYPES_H */
