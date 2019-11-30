/*****************************************************************************
*
* Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
*
*****************************************************************************/
/**
 * \file macros.h
 *  Commonly used macros.
 */

#ifndef MACROS_H
#define MACROS_H

#include <string.h>
#include "util.h" /* Contains MIN,MAX, ABS macro definitions that are common between platforms */

#define UNUSED(x) ((void)(x))

#define bzero(dst, n) (void)(memset((dst), 0, (n)))

/* Macro from hydra includes needed for subreport_panic function */
/* Function attributes are only supported by GCC */
#ifdef __GNUC__
#define GCC_ATTRIBUTE(x) __attribute__ ((x))
#else
#define GCC_ATTRIBUTE(x)
#endif

#ifndef USE_DUMMY_PANIC
#define NORETURN GCC_ATTRIBUTE(__noreturn__)
#else
#define NORETURN
#endif

/**
 * Array size (of first dimension if multi-dimension)
 */
#define ARRAY_DIM(a) (sizeof(a) / sizeof((a)[0]))

/**
 * Address of first element within this array.
 *
 * N.B. This (a) form works with multiple dimensions. (c.f. &a[0])
 */
#define ARRAY_BEGIN(a) (a)

/**
 * Address of first element beyond this array.
 *
 * N.B. This (a + x) form works with multiple dimensions. (c.f. &a[x])
 */
#define ARRAY_END(a) ((a) + ARRAY_DIM(a))

/**
 * Iterate over fixed size array.
 *
 * int ax[] = {1,2,3,4,5};
 * int *ix;
 * int sum = 0;
 * ARRAY_FOREACH(ix, ax)
 * {
 *      sum += *ix;
 * }
 */
#define ARRAY_FOREACH(i,a) for ((i) = ARRAY_BEGIN(a); (i) != ARRAY_END(a); ++(i))

/**
 * Get address of structure given address of member.
 */
#define STRUCT_FROM_MEMBER(sname, mname, maddr) \
    ((sname *)((char *)(maddr) - offsetof(sname, mname)))

/**
 * Get address of const structure given address of member.
 */
#define STRUCT_FROM_CONST_MEMBER(sname, mname, maddr) \
    ((const sname *)((const char *)(maddr) - offsetof(sname, mname)))


/*****************************************************************************
 * uint32 utilities
 *****************************************************************************/

/**
 * MS 16 bits of uint32.
 *
 * \public \memberof uint32
 */
#define UINT32_MSW(x) \
    ((uint16)((x) >> 16))

/**
 * LS 16 bits of uint32.
 *
 * \public \memberof uint32
 */
#define UINT32_LSW(x) \
    ((uint16)((x) & 0xFFFF))

/**
 * Build uint32 from two uint16s
 *
 * \public \memberof uint32
 */
#define UINT32_BUILD(msw, lsw) \
    (((uint32)(msw) << 16) | (uint16)(lsw))

/**
 * Swap (16 bit) ends of a uint32
 *
 * \public \memberof uint32
 */
#define UINT32_SWAP_ENDS(x) \
    UINT32_BUILD(UINT32_LSW(x), UINT32_MSW(x))


/*****************************************************************************
 * uint16 utilities
 * On Kalimba (u)int8, (u)in16 are all 24 bit. So the real MSO is bits 16-23. 
 * These macros however refer to bits0-15 of a (u)int16 value. So LSO is b0-7 
 * and MSO is b8-15.
 *****************************************************************************/

/**
 * MS octet of x
 *
 * \public \memberof uint16
 */
#define UINT16_MSO(x) \
    ((uint8)(((x) >> 8) & 0xFF))

/**
 * LS octet of x
 *
 * \public \memberof uint16
 */
#define UINT16_LSO(x) \
    ((uint8)((x) & 0xFF))

/**
 * Build uint16 from two octets
 *
 * \public \memberof uint16
 */
#define UINT16_BUILD(mso,lso) \
    ((uint16)(((uint8)(mso) << 8) | (uint8)(lso)))

/**
 * Swap octets of a uint16
 *
 * \public \memberof uint16
 */
#define UINT16_SWAP_ENDS(x) \
    UINT16_BUILD(UINT16_LSO(x), UINT16_MSO(x))

/**
 * Read a single uint16 value from a raw buffer location pointed to by
 * p_data (auto incremented) and store it at the specified result location.
 */
#define UNMARSHALL_1X_UINT16_INTO_UINT24( p_data, result ) \
        ((result) = *(p_data)++)

/**
 * Read two uint16 values from a raw buffer location pointed to by p_data
 * (auto incremented) and store the combined result at the specified result
 * location. The first of the two uint16 values is treated as the LS 16 bits.
 */
#define UNMARSHALL_2X_UINT16_INTO_UINT24( p_data, result ) \
        ((result) = *(p_data)++, (result) += ((*(p_data)++) << 16))

/**
 *  Almost identical to the above, but intended for a 32-bit destination
 */
#define UNMARSHALL_2X_UINT16_INTO_UINT32( p_data, result ) \
        ((result) = *(p_data)++, (result) += (((uint32)(*(p_data)++)) << 16))

#endif /* MACROS_H */
