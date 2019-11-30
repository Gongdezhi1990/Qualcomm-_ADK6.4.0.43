/* Copyright (c) 2010 - 2018 Qualcomm Technologies International, Limited */

/*
 * stdint.h for 24-bit Kalimba targets
 */

#ifndef __stdint24_included
#define __stdint24_included

#include <limits.h>

/* Fixed width, least width, fastest width types */

/* Exact 8-bit, 16-bit, 32-bit and 64-bit types are not available
 * on 24-bit Kalimba targets, but we should support int_least<size>_t 
 * and int_fast<size>_t for these cases.
 */
typedef signed int         int_least8_t;
typedef unsigned int       uint_least8_t;
typedef signed int         int_fast8_t;
typedef unsigned int       uint_fast8_t;
typedef signed int         int_least16_t;
typedef unsigned int       uint_least16_t;
typedef signed int         int_fast16_t;
typedef unsigned int       uint_fast16_t;
typedef signed long        int_least32_t;
typedef unsigned long      uint_least32_t;
typedef signed long        int_fast32_t;
typedef unsigned long      uint_fast32_t;
typedef signed long long   int_least64_t;
typedef unsigned long long uint_least64_t;
typedef signed long long   int_fast64_t;
typedef unsigned long long uint_fast64_t;

/* Exact type widths supported on 24-bit Kalimba targets,
 * with least and fast variants.
 */
typedef signed int         int24_t;
typedef unsigned int       uint24_t;
typedef signed int         int_least24_t;
typedef unsigned int       uint_least24_t;
typedef signed int         int_fast24_t;
typedef unsigned int       uint_fast24_t;

typedef signed long        int48_t;
typedef unsigned long      uint48_t;
typedef signed long        int_least48_t;
typedef unsigned long      uint_least48_t;
typedef signed long        int_fast48_t;
typedef unsigned long      uint_fast48_t;

typedef signed long long   int72_t;
typedef unsigned long long uint72_t;
typedef signed long long   int_least72_t;
typedef unsigned long long uint_least72_t;
typedef signed long long   int_fast72_t;
typedef unsigned long long uint_fast72_t;

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

/* Types whose exact widths are not supported, but
 * we publish int_least<size>_t and int_fast<size>_t
 * types for them.
 */
#define INT_LEAST8_MIN     INT_MIN
#define INT_LEAST8_MAX     INT_MAX
#define UINT_LEAST8_MAX	   UINT_MAX

#define INT_FAST8_MIN      INT_MIN
#define INT_FAST8_MAX      INT_MAX
#define UINT_FAST8_MAX     UINT_MAX

#define INT_LEAST16_MIN    INT_MIN
#define INT_LEAST16_MAX    INT_MAX
#define UINT_LEAST16_MAX   UINT_MAX

#define INT_FAST16_MIN     INT_MIN
#define INT_FAST16_MAX     INT_MAX
#define UINT_FAST16_MAX    UINT_MAX

#define INT_LEAST32_MIN    LONG_MIN
#define INT_LEAST32_MAX    LONG_MAX
#define UINT_LEAST32_MAX   ULONG_MAX

#define INT_FAST32_MIN     LONG_MIN
#define INT_FAST32_MAX     LONG_MAX
#define UINT_FAST32_MAX    ULONG_MAX

#define INT_LEAST64_MIN    LLONG_MIN
#define INT_LEAST64_MAX    LLONG_MAX
#define UINT_LEAST64_MAX   ULLONG_MAX

#define INT_FAST64_MIN     LLONG_MIN
#define INT_FAST64_MAX     LLONG_MAX
#define UINT_FAST64_MAX    ULLONG_MAX

/* Defines for fully supported types */
#define INT24_MIN          INT_MIN
#define INT24_MAX          INT_MAX
#define INT_LEAST24_MIN    INT_MIN
#define INT_LEAST24_MAX    INT_MAX
#define INT_FAST24_MIN     INT_MIN
#define INT_FAST24_MAX     INT_MAX

#define UINT24_MAX         UINT_MAX
#define UINT_LEAST24_MAX   UINT_MAX
#define UINT_FAST24_MAX    UINT_MAX

#define INT48_MIN          LONG_MIN
#define INT48_MAX          LONG_MAX
#define INT_LEAST48_MIN    LONG_MIN
#define INT_LEAST48_MAX    LONG_MAX
#define INT_FAST48_MIN     LONG_MIN
#define INT_FAST48_MAX     LONG_MAX

#define UINT48_MAX         ULONG_MAX
#define UINT_LEAST48_MAX   ULONG_MAX
#define UINT_FAST48_MAX    ULONG_MAX

#define INT72_MIN          LLONG_MIN
#define INT72_MAX          LLONG_MAX
#define INT_LEAST72_MIN    LLONG_MIN
#define INT_LEAST72_MAX    LLONG_MAX
#define INT_FAST72_MIN     LLONG_MIN
#define INT_FAST72_MAX     LLONG_MAX

#define UINT72_MAX         ULLONG_MAX
#define UINT_LEAST72_MAX   ULLONG_MAX
#define UINT_FAST72_MAX    ULLONG_MAX


/* Maximums */
#define INTMAX_MIN         LLONG_MIN
#define INTMAX_MAX         LLONG_MAX
#define UINTMAX_MAX        ULLONG_MAX


/* Constant expansion macros appending appropriate constant suffix */
/* Minimum width */
#define INT8_C(x)          x
#define UINT8_C(x)         x##U

#define INT16_C(x)         x
#define UINT16_C(x)        x##U

#define INT24_C(x)         x
#define UINT24_C(x)        x##U

#define INT32_C(x)         x##L
#define UINT32_C(x)        x##UL

#define INT48_C(x)         x##L
#define UINT48_C(x)        x##UL

#define INT64_C(x)         x##LL
#define UINT64_C(x)        x##ULL

#define INT72_C(x)         x##LL
#define UINT72_C(x)        x##ULL

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

#endif /* __stdint24_included */
