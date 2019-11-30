#ifndef BIGINT_H
#define BIGINT_H

/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * big integer support
 *
 */

/**
 * \defgroup bigint bigint
 * \ingroup core
 * \section bigint_description DESCRIPTION
 *   This library provides data types and functions for creating and
 *   performing arithmetic operations on 64 bit integers.
*/

#include "hydra/hydra_types.h"


/* Define (supposedly) opaque 64 bit signed and unsigned integer types */
#ifdef BIGINT_64BIT_INTRINSIC
#ifndef HOSTTOOLS /* HostTools use definitions of (u)int<n> = (u)int<n>_t, which may clash with below */
typedef long long int64;
typedef unsigned long long uint64;
#endif
#else
typedef struct int64 { int32 a; int32 b; } int64;
typedef struct uint64 { uint32 a; uint32 b; } uint64;
#endif


/**
 * construct 64 bit signed integer from a 32 bit integer
 *
 * \returns Tne 64 bit integer formed by extending the 32 bit integer "a", performing
 *  sign extend if appropriate.
 */
extern int64 int64_from32(int32 a);
/**
 * construct 64 bit unsigned integer from a 32 bit integer
 *
 * \returns Tne 64 bit integer formed by extending the 32 bit integer "a", performing
 *  sign extend if appropriate.
 */
extern uint64 uint64_from32(uint32 a);


/**
 * construct 64 bit signed integer from 32 bit integers
 *
 * \return Tne 64 bit integer formed by combining two 32 bit integers; "hi" specifies
 * the most significant half, and "lo" specifies the least significant half:
 *       return (hi << 32) + lo;
 */
extern int64 int64_from32x2(int32 hi, int32 lo);

/**
 * construct 64 bit unsigned integer from 32 bit integers
 *
 * \return Tne 64 bit integer formed by combining two 32 bit integers; "hi" specifies
 * the most significant half, and "lo" specifies the least significant half:
 *       return (hi << 32) + lo;
 */
extern uint64 uint64_from32x2(uint32 hi, uint32 lo);


/**
 * construct a 64 bit signed integer from an array
 *
 * \returns  The 64 bit integer formed by combining an array of either four 16 bit
 *   integers or eight 8 bit integers. The array "v" is treated as being little
 *   endian, i.e. with the least significant part of the result in v[0].
 *   entry.
 */
extern int64 int64_from16x4(const int16 v[4]);

/**
 * construct a 64 bit signed integer from an array
 *
 * \returns  The 64 bit integer formed by combining an array of either four 16 bit
 *   integers or eight 8 bit integers. The array "v" is treated as being little
 *   endian, i.e. with the least significant part of the result in v[0].
 *   entry.
 */
extern int64 int64_from8x8(const int8 v[8]);

/**
 * construct a 64 bit unsigned integer from an array
 *
 * \returns  The 64 bit integer formed by combining an array of either four 16 bit
 *   integers or eight 8 bit integers. The array "v" is treated as being little
 *   endian, i.e. with the least significant part of the result in v[0].
 *   entry.
 */
extern uint64 uint64_from16x4(const uint16 v[4]);

/**
 * construct a 64 bit unsigned integer from an array
 *
 * \returns  The 64 bit integer formed by combining an array of either four 16 bit
 *   integers or eight 8 bit integers. The array "v" is treated as being little
 *   endian, i.e. with the least significant part of the result in v[0].
 *   entry.
 */
extern uint64 uint64_from8x8(const uint8 v[8]);


/**
 * extract the least significant 32 bits of a 64 bit integer
 *
 * \returns Tne least significant 32 bits of "a":
 *        return a & 0xFFFF;
 */
extern int32 int64_lo32(int64 a);
/**
 * extract the least significant 32 bits of a 64 bit integer
 *
 * \returns Tne least significant 32 bits of "a":
 *        return a & 0xFFFF;
 */
extern uint32 uint64_lo32(uint64 a);


/**
 * extract the most significant 32 bits of a 64 bit integer
 *
 * \returns     Tne most significant 32 bits of "a":
 *        return a >> 16;
 */
extern int32 int64_hi32(int64 a);

/**
 * extract the most significant 32 bits of a 64 bit integer
 *
 * \returns     Tne most significant 32 bits of "a":
 *        return a >> 16;
 */
extern uint32 uint64_hi32(uint64 a);


/**
 * deconstruct a 64 bit signed integer to an array
 * 
 * Splits a 64 bit integer \p a into an array of two 32 bit
 * integers. The array \p v is treated as being little endian,
 * i.e. with the least significant part of the result in \c v[0].
 */
extern void int64_to32x2(int64 a, int32 v[2]);

/**
 * deconstruct a 64 bit signed integer to an array
 *
 * Splits a 64 bit integer \p a into an array of four 16 bit
 * integers. The array \p v is treated as being little endian,
 * i.e. with the least significant part of the result in \c v[0].
 */
extern void int64_to16x4(int64 a, int16 v[4]);

/**
 * deconstruct a 64 bit signed integer to an array
 *
 * Splits a 64 bit integer \p a into an array of eight 8 bit
 * integers. The array \p v is treated as being little endian,
 * i.e. with the least significant part of the result in \c v[0].
 */
extern void int64_to8x8(int64 a, int8 v[8]);

/**
 * deconstruct a 64 bit unsigned integer to an array
 * 
 * Splits a 64 bit integer \p a into an array of two 32 bit
 * integers. The array \p v is treated as being little endian,
 * i.e. with the least significant part of the result in \c v[0].
 */
extern void uint64_to32x2(uint64 a, uint32 v[2]);

/**
 * deconstruct a 64 bit unsigned integer to an array
 *
 * Splits a 64 bit integer \p a into an array of four 16 bit
 * integers. The array \p v is treated as being little endian,
 * i.e. with the least significant part of the result in \c v[0].
 */
extern void uint64_to16x4(uint64 a, uint16 v[4]);

/**
 * deconstruct a 64 bit unsigned integer to an array
 *
 * Splits a 64 bit integer \p a into an array of eight 8 bit
 * integers. The array \p v is treated as being little endian,
 * i.e. with the least significant part of the result in \c v[0].
 */
extern void uint64_to8x8(uint64 a, uint8 v[8]);


/**
 * convert an unsigned 64 bit integer to signed
 *
 * This function is only suitable for converting between the signed and
 * unsigned forms of 64 bit integers. They must not be used to promote
 * smaller integer types.
 */
extern int64 int64_cast(uint64 a);

/**
 * convert a signed 64 bit integer to unsigned
 *
 * This function is only suitable for converting between the signed and
 * unsigned forms of 64 bit integers. They must not be used to promote
 * smaller integer types.
 */
extern uint64 uint64_cast(int64 a);


/**
 * determine the sign of a 64 bit signed integer
 *
 * \returns TRUE if \p a is less than zero, else FALSE.
 */
extern bool int64_is_negative(int64 a);


/**
 * is a 64 bit signed integer zero
 *
 * \returns TRUE if "a" is zero, else FALSE.
 */
extern bool int64_is_zero(int64 a);

/**
 * is a 64 bit unsigned integer zero
 *
 * \returns TRUE if "a" is zero, else FALSE.
 */
extern bool uint64_is_zero(uint64 a);




/**
 * negate a 64 bit signed integer
 * 
 * \return  The negation of \p a:
 *       return -a;
 */
extern int64 int64_negate(int64 a);



/**
 * calculate the magnitude of a 64 bit signed integer
 *
 * \returns  The magnitude of \p a:
 *       return a < 0 ? -a : a;
 */
extern int64 int64_abs(int64 a);




/**
 * add two 64 bit signed integers
 * 
 * \returns The sum of \p a and \p b
 */
extern int64 int64_add(int64 a, int64 b);

/**
 * add two 64 bit unsigned integers
 * 
 * \returns The sum of \p a and \p b
 */
extern uint64 uint64_add(uint64 a, uint64 b);


/**
 * subtract two 64 bit signed integers
 *
 * \returns a-b;
 */
extern int64 int64_sub(int64 a, int64 b);
/**
 * subtract two 64 bit unsigned integers
 *
 * \returns a-b;
 */
extern uint64 uint64_sub(uint64 a, uint64 b);


/**
 * multiply two 64 bit signed integers
 *
 * \returns \p a * \p b
 */
extern int64 int64_mul(int64 a, int64 b);
/**
 * multiply two 64 bit unsigned integers
 *
 * \returns \p a * \p b
 */
extern uint64 uint64_mul(uint64 a, uint64 b);


/**
 * signed 64 bit by 16 bit multiplication
 *
 * \returns \p a* \p b
 */
extern int64 int64_mul16(int64 a, int16 b);
/**
 * unsigned 64 bit by 16 bit multiplication
 *
 * \returns \p a* \p b
 */
extern uint64 uint64_mul16(uint64 a, uint16 b);


/**
 * divide two 64 bit signed integers
 *
 * \returns n / d (rounded towards zero) 
 */
extern int64 int64_div(int64 n, int64 d);
/**
 * divide two 64 bit unsigned integers
 *
 * \returns
 */
extern uint64 uint64_div(uint64 n, uint64 d);


/**
 * divide two 64 bit signed integers, returning remainder
 *
 * \returns     The remainder when "n" is divided by "d" (with the same sign as "n"):
 *        return n % d;
 */
extern int64 int64_mod(int64 n, int64 d);
/**
 * divide two 64 bit unsigned integers, returning remainder
 *
 * \returns     The remainder when "n" is divided by "d" (with the same sign as "n"):
 *       return n % d;
 */
extern uint64 uint64_mod(uint64 n, uint64 d);


/**
 * divide two 64 bit signed integers
 *
 *     Divide "n" by "d", storing the quotient (rounded towards zero) in "q":
 *        if (q) *q = n / d;
 *
 * \returns The remainder when "n" is divided by "d" (with the same sign as "n"):
 *   return n % d;
 *
 */
extern int64 int64_divmod(int64 n, int64 d, int64 *q);
/**
 * divide two 64 bit unsigned integers
 *
 *     Divide "n" by "d", storing the quotient (rounded towards zero) in "q":
 *        if (q) *q = n / d;
 *
 * \returns The remainder when "n" is divided by "d" (with the same sign as "n"):
 *   return n % d;
 *
 */
extern uint64 uint64_divmod(uint64 n, uint64 d, uint64 *q);


/**
 * signed 64 bit by 16 bit division
 *
 * \returns n / d
 */
extern int64 int64_div16(int64 n, int16 d);
/**
 * unsigned 64 bit by 16 bit division
 *
 * \returns n / d
 */
extern uint64 uint64_div16(uint64 n, uint16 d);


/**
 * remainder of signed 64 bit by 16 bit division
 *
 * \returns The remainder when "n" is divided by "d" (with the same sign as "n")
 */
extern int16 int64_mod16(int64 n, int16 d);
/**
 * remainder of signed 64 bit by 16 bit division
 *
 * \returns The remainder when "n" is divided by "d"
 */
extern uint16 uint64_mod16(uint64 n, uint16 d);


/**
 * signed 64 bit by 16 bit division
 *
 * Divide \p n by \p d, storing the quotient (rounded towards zero) in \p q:
 *   if (q) *q = n / d;
 *
 * \returns n % d
 */
extern int16 int64_divmod16(int64 n, int16 d, int64 *q);
/**
 * unsigned 64 bit by 16 bit division
 *
 * Divide \p n by \p d, storing the quotient (rounded towards zero) in \p q:
 *   if (q) *q = n / d;
 *
 * \returns n % d
 */
extern uint16 uint64_divmod16(uint64 n, uint16 d, uint64 *q);


/**
 * signed arithmetic left shift
 *
 * Arithmetically shifts \p in left by \p bits bit positions, storing the
 * result in \p out:
 *      *out = *in << bits;
 *
 *  Like the standard C "<<" operator, \p bits must be between 0 and 63
 *  inclusive, otherwise the result is undefined.
 */
extern int64 int64_asl(int64 in, unsigned int bits);
/**
 * unsigned arithmetic left shift
 *
 * Arithmetically shifts \p in left by \p bits bit positions, storing the
 * result in \p out:
 *      *out = *in << bits;
 *
 *  Like the standard C "<<" operator, \p bits must be between 0 and 63
 *  inclusive, otherwise the result is undefined.
 */
extern uint64 uint64_asl(uint64 in, unsigned int bits);


/**
 * signed arithmetic right shift (with sign extend)
 *
 *  Arithmetically shifts \p in right by \p bits bit positions, storing the
 *  result in \p out:
 *      *out = *in >> bits;
 *
 *  Performs sign extension.
 *
 *  Like the standard C "<<" operator, \p bits must be between 0 and 63
 *  inclusive, otherwise the result is undefined.
 * 
 */
extern int64 int64_asr(int64 in, unsigned int bits);
/**
 * unsigned arithmetic right shift (with sign extend)
 *
 *  Arithmetically shifts \p in right by \p bits bit positions, storing the
 *  result in \p out:
 *      *out = *in >> bits;
 *
 *  Zero fills.
 *
 *  Like the standard C "<<" operator, \p bits must be between 0 and 63
 *  inclusive, otherwise the result is undefined.
 * 
 */
extern uint64 uint64_asr(uint64 in, unsigned int bits);


/**
 * compare two 64 bit signed integers
 *
 * \returns The result of comparing "a" to "b":
 *           if (*a < *b)  return -1;
 *      else if (*a == *b) return 0;
 *      else if (*b < *a)  return +1;
 */
extern int int64_cmp(int64 a, int64 b);
/**
 * compare two 64 bit unsigned integers
 *
 * \returns The result of comparing "a" to "b":
 *           if (*a < *b)  return -1;
 *      else if (*a == *b) return 0;
 *      else if (*b < *a)  return +1;
 */
extern int uint64_cmp(uint64 a, uint64 b);


/**
 * add a uint16 to a little-endian 64-bit number
 *
 * Adds \p value16 to the 64-bit number in the four locations at \p ptr64,
 * where \c *ptr64 holds the least significant word.
 */
extern void uint64_le_add_uint16(uint16 *ptr64, uint16 value16);

/**
 * add a uint32 to a little-endian 64-bit number
 *
 * Adds "value32" to the 64-bit number in the four locations at "src64",
 * (where "*src64" holds the least significant word) and places the result
 * in the four locations at "dest64".
 */
extern void uint64_le_add_uint32(const uint16 *src64, 
                                 uint16 *dest64, 
                                 uint32 value32);


/* Include operations that are implemented as macros. */
#include "bigint/bigint_imp.h"


#endif /* BIGINT_H */
