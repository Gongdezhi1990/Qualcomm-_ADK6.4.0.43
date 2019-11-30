/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*
FILE
    types.h  -  commonly used typedefs

DESCRIPTION
    This file should be #included by every C or C++ source file, whether
    it is being included in the firmware or host-side code.
*/

#ifndef TYPES_H
#define TYPES_H

#ifndef HYDRA_TYPES_H
#error "types.h should only be include from hydra_types.h for isolation."
#endif /* HYDRA_TYPES_H */

#include <stddef.h> /* For ptrdiff_t and size_t */
#include <limits.h> /* For _MAX,_MIN macros */

/*!
 * \class uint8
 * \brief Portable type 8 bit unsigned integer
 * \note On earlier platforms a uint8 was the same size as uint16.
 */
typedef unsigned char   uint8;
/*!
 * \class uint16
 * \brief Portable type 16 bit unsigned integer
 */
typedef unsigned short  uint16;
/*!
 * \class uint32
 * \brief Portable type 32 bit unsigned integer
 */
typedef unsigned long   uint32;

/*!
 * \class int8
 * \brief Portable type 8 bit signed integer
 * \note On earlier platforms an int8 was the same size as int16.
 */
typedef signed char     int8;
/*!
 * \class int16
 * \brief Portable type 16 bit signed integer
 */
typedef signed short    int16;
/*!
 * \class int32
 * \brief Portable type 32 bit signed integer
 */
typedef signed long     int32;

#ifndef __cplusplus
typedef unsigned        bool;
#endif

typedef unsigned int uintptr; /* Used in some legacy bluecore code*/

#endif
