/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file
 *
 * \defgroup hydra_types hydra_types
 * \ingroup hydra_includes
 *
 * Header included by all HydraCore common firmware to define types.
 * This file exists as a buffer zone between the HydraCore code
 * and local subsystem firmware.  Subsystems should update it to
 * include their own type definitions and provide any fixes necessary
 * for using the HydraCore code.
 *
 * Note: Integer type semantics
 *
 * Note that the HydraCore types have specific semantics
 *
 *  - "uintN" means "the *smallest* native type with at least N bits". Note that
 *  this *does not guarantee* that there are really N bits in the type, so for
 *  example variables of such types cannot be relied upon to wrap at their
 *  logical minimum or maximum.
 *  - "uintNf" means "the *fastest* native type with at least N bits".  The
 *  same caveat applies as above, of course.
 *
 * True known-width integer types are not available in HydraCore common code.
 *
 */

#ifndef HYDRA_TYPES_H
#define HYDRA_TYPES_H

/**
 * This the types definition file used by the HydraCore common firmware
 * itself.
 */
#include "types.h"

#ifdef __KALIMBA__
#ifdef __KALIMBA32__
typedef uint32          uint16f;
/* Audio subsystem code already defines uint24. Anyways, all the basic
 * types should be defined in types.h and it is wrong for hydra_types to
 * assume what is defined on the given codebase/platform */
/* typedef uint32          uint24; */
#else /* KALIMBA32 */
typedef uint24          uint16f;
#endif
#else /* XAP */
typedef uint16          uint16f;
typedef uint32          uint24;
#endif

/* For convenience */
#define UINT_BIT ((unsigned)CHAR_BIT * sizeof(unsigned int)/sizeof(char))
#define ULONG_BIT ((unsigned)CHAR_BIT * sizeof(unsigned long)/sizeof(char))
typedef TIME_INTERVAL INTERVAL;


/**
 * \name Provide some types not usually present in subsystem firmware.
 * These are needed by CCP messages.
 *
 * We assume there is a \c uint8 type that is at least 8 bits wide,
 * but may be wider, and use that for types representing smaller
 * numbers.  These will never hold more than the numbers of bits
 * they claim.
 */
/*@{*/

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
#define DIATRIBE_TYPE           uint16f

#endif /* HYDRA_TYPES_H */
