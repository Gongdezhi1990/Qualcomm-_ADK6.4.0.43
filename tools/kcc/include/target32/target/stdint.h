/* Copyright (c) 2010 - 2018 Qualcomm Technologies International, Limited */

/*
 * stdint.h for 32-bit Kalimba targets
 */

#ifndef __stdint32_included
#define __stdint32_included

#include <limits.h>

/* Fixed width, least width, fastest width types */
typedef signed char        int8_t;
typedef unsigned char      uint8_t;
typedef signed char        int_least8_t;
typedef unsigned char      uint_least8_t;
typedef signed int         int_fast8_t;
typedef unsigned int       uint_fast8_t;

typedef signed short       int16_t;
typedef unsigned short     uint16_t;
typedef signed short       int_least16_t;
typedef unsigned short     uint_least16_t;
typedef signed int         int_fast16_t;
typedef unsigned int       uint_fast16_t;

typedef signed int         int32_t;
typedef unsigned int       uint32_t;
typedef signed int         int_least32_t;
typedef unsigned int       uint_least32_t;
typedef signed int         int_fast32_t;
typedef unsigned int       uint_fast32_t;

typedef signed long long   int64_t;
typedef unsigned long long uint64_t;
typedef signed long long   int_least64_t;
typedef unsigned long long uint_least64_t;
typedef signed long long   int_fast64_t;
typedef unsigned long long uint_fast64_t;


/* Maximum width integer types */
typedef signed long long   intmax_t;
typedef unsigned long long uintmax_t;


/* Pointer types */
typedef int                intptr_t;
typedef unsigned int       uintptr_t;

#define INTPTR_MAX         INT_MAX
#define INTPTR_MIN         INT_MIN 
#define UINTPTR_MAX        UINT_MAX


/* Fixed width integer type min/max values */
#define INT8_MIN           SCHAR_MIN
#define INT8_MAX           SCHAR_MAX
#define INT_LEAST8_MIN     SCHAR_MIN
#define INT_LEAST8_MAX     SCHAR_MAX
#define INT_FAST8_MIN      INT_MIN
#define INT_FAST8_MAX      INT_MAX

#define UINT8_MAX          UCHAR_MAX
#define UINT_LEAST8_MAX	   UCHAR_MAX
#define UINT_FAST8_MAX     UINT_MAX

#define INT16_MIN          SHRT_MIN
#define INT16_MAX          SHRT_MAX
#define INT_LEAST16_MIN    SHRT_MIN
#define INT_LEAST16_MAX    SHRT_MAX
#define INT_FAST16_MIN     INT_MIN
#define INT_FAST16_MAX     INT_MAX

#define UINT16_MAX         USHRT_MAX
#define UINT_LEAST16_MAX   USHRT_MAX
#define UINT_FAST16_MAX    UINT_MAX

#define INT32_MIN          INT_MIN
#define INT32_MAX          INT_MAX
#define INT_LEAST32_MIN    INT_MIN
#define INT_LEAST32_MAX    INT_MAX
#define INT_FAST32_MIN     INT_MIN
#define INT_FAST32_MAX     INT_MAX

#define UINT32_MAX         UINT_MAX
#define UINT_LEAST32_MAX   UINT_MAX
#define UINT_FAST32_MAX    UINT_MAX

#define INT64_MIN          LLONG_MIN
#define INT64_MAX          LLONG_MAX
#define INT_LEAST64_MIN    LLONG_MIN
#define INT_LEAST64_MAX    LLONG_MAX
#define INT_FAST64_MIN     LLONG_MIN
#define INT_FAST64_MAX     LLONG_MAX

#define UINT64_MAX         ULLONG_MAX
#define UINT_LEAST64_MAX   ULLONG_MAX
#define UINT_FAST64_MAX    ULLONG_MAX

/* Maximum values */
#define INTMAX_MIN         LLONG_MIN
#define INTMAX_MAX         LLONG_MAX
#define UINTMAX_MAX        ULLONG_MAX


/* Constant expansion macros appending appropriate constant suffix */
/* Minimum width */
#define INT8_C(x)          x
#define UINT8_C(x)         x##U

#define INT16_C(x)         x
#define UINT16_C(x)        x##U

#define INT32_C(x)         x
#define UINT32_C(x)        x##U

#define INT64_C(x)         x##LL
#define UINT64_C(x)        x##ULL

/* Maximum width */
#define INTMAX_C(x)        x##LL
#define UINTMAX_C(x)       x##ULL


/* Values for types defined in stddef.h, wchar.h */
#define PTRDIFF_MIN        INT_MIN
#define PTRDIFF_MAX        INT_MAX
#define SIZE_MAX           UINT_MAX
#define WCHAR_MIN          INT_MIN
#define WCHAR_MAX          INT_MAX
#define SIG_ATOMIC_MIN     INT_MIN
#define SIG_ATOMIC_MAX     INT_MAX

/* Missing types */
/* 
 * #define WINT_MIN        INT32_MIN
 * #define WINT_MAX        INT32_MAX 
 */

#endif /* __stdint32_included */
