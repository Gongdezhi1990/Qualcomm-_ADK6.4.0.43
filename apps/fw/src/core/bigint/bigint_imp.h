/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/**
 * \file 
 * macros to implement big integer support
 *
 * Most of the operations on 64 bit integers can be implemented using either
 * inline intrinsic operations or explicit function calls. This file performs
 * the appropriate expansion for the target platform and selected options.
 */

#ifndef BIGINT_IMP_H
#define BIGINT_IMP_H


/* Rename some functions to use them as helpers for the compiler's intrinsic
   support (but must not do this if the intrinsic support is not being used as
   the calling convention for struct's may be different) */
#if defined(XAPGCC) && defined(BIGINT_64BIT_INTRINSIC)
/* Functions used for both signed and unsigned integers */
#define uint64_add_   __addsi2
#define uint64_sub_   __subsi2
#define uint64_asl_   __ashlsi3
#define uint64_mul_   __mulsi3
/* Functions used for signed integers only */
#define int64_from32_ __extendhisi2
#define int64_negate_ __negsi2
#define int64_mod_    __modsi3
#define int64_div_    __divsi3
#define int64_asr_    __ashrsi3
/* Functions used for unsigned integers only */
#define uint64_mod_   __umodsi3
#define uint64_div_   __udivsi3
#define uint64_asr_   __lshrsi3
#endif


/* Expand the appropriate implementation for this platform (either intrinsic
   operations supported by the compiler or explicit function calls) */
#ifdef BIGINT_64BIT_INTRINSIC
#define BIGINT_IMP(intrinsic, function) (intrinsic)
#else
#define BIGINT_IMP(intrinsic, function) (function)
#endif


/* Some operations are always implemented as functions */

extern int64 int64_from16x4_(const int16 v[4]);
#define int64_from16x4(v)     int64_from16x4_(v)

extern int64 int64_from8x8_(const int8 v[8]);
#define int64_from8x8(v)      int64_from8x8_(v)

extern uint64 uint64_from16x4_(const uint16 v[4]);
#define uint64_from16x4(v)    uint64_from16x4_(v)

extern uint64 uint64_from8x8_(const uint8 v[8]);
#define uint64_from8x8(v)     uint64_from8x8_(v)

extern void int64_to32x2_(int64 a, int32 v[2]);
#define int64_to32x2(a, v)    int64_to32x2_((a), (v))

extern void int64_to16x4_(int64 a, int16 v[4]);
#define int64_to16x4(a, v)    int64_to16x4_((a), (v))

extern void int64_to8x8_(int64 a, int8 v[8]);
#define int64_to8x8(a, v)     int64_to8x8_((a), (v))

extern void uint64_to32x2_(uint64 a, uint32 v[2]);
#define uint64_to32x2(a, v)   uint64_to32x2_((a), (v))

extern void uint64_to16x4_(uint64 a, uint16 v[4]);
#define uint64_to16x4(a, v)   uint64_to16x4_((a), (v))

extern void uint64_to8x8_(uint64 a, uint8 v[8]);
#define uint64_to8x8(a, v)   uint64_to8x8_((a), (v))

extern int64 int64_mul16_(int64 a, int16 b);
#define int64_mul16(a, b)     int64_mul16_((a), (b))

extern uint64 uint64_mul16_(uint64 a, uint16 b);
#define uint64_mul16(a, b)    uint64_mul16_((a), (b))

extern int64 int64_divmod_(int64 n, int64 d, int64 *q);
#define int64_divmod(n, d, q) int64_divmod_((n), (d), (q))

extern uint64 uint64_divmod_(uint64 n, uint64 d, uint64 *q);
#define uint64_divmod(n, d, q) uint64_divmod_((n), (d), (q))

extern int64 int64_div16_(int64 n, int16 d);
#define int64_div16(n, d)     int64_div16_((n), (d))

#ifdef OPTIM_UINT64_DIVMOD31
extern uint64 uint64_divmod31_opt(uint64 n, uint32 d, uint32 *r);
#define uint64_div16(n, d)    uint64_divmod31_opt((n), (d), NULL)
#else
extern uint64 uint64_div16_(uint64 n, uint16 d);
#define uint64_div16(n, d)    uint64_div16_((n), (d))
#endif

extern int16 int64_divmod16_(int64 n, int16 d, int64 *q);
#define int64_divmod16(n, d, q) int64_divmod16_((n), (d), (q))

extern uint16 uint64_divmod16_(uint64 n, uint16 d, uint64 *q);
#define uint64_divmod16(n, d, q) uint64_divmod16_((n), (d), (q))

extern int64 int64_asr_(int64 in, unsigned int bits);
/* (the intrinsic >> operator is never used for a signed arithmetic shift
   because the result is undefined when used on signed types with the most
   significant bit set, and sign extend is required) */
#define int64_asr(in, bits)   int64_asr_((in), (bits))

extern int int64_cmp_(int64 a, int64 b);
#define int64_cmp(a, b)       int64_cmp_((a), (b))

extern int uint64_cmp_(uint64 a, uint64 b);
#define uint64_cmp(a, b)      uint64_cmp_((a), (b))


/* Some operations use intrinsic compiler operations if possible */
/*lint -esym(666, int64_*, uint64_*) repeated macro parameters are safe */

extern int64 int64_from32_(int32 a);
#define int64_from32(a)       BIGINT_IMP((int64) (int32) (a), \
                                         int64_from32_(a))

extern uint64 uint64_from32_(uint32 a);
#define uint64_from32(a)      BIGINT_IMP((uint64) (uint32) (a), \
                                         uint64_from32_(a))

extern int64 int64_from32x2_(int32 hi, int32 lo);
#define int64_from32x2(hi, lo) BIGINT_IMP((((int64) (hi)) << 32) \
                                          | ((int64) (uint32) (lo)), \
                                          int64_from32x2_((hi), (lo)))

extern uint64 uint64_from32x2_(uint32 hi, uint32 lo);
#define uint64_from32x2(hi, lo) BIGINT_IMP((((uint64) (hi)) << 32) \
                                           | ((uint64) (lo)), \
                                           uint64_from32x2_((hi), (lo)))

extern int32 int64_lo32_(int64 a);
#define int64_lo32(a)         BIGINT_IMP((int32) (((uint32) (a)) \
                                                  & 0xFFFFFFFFUL), \
                                         int64_lo32_(a))

extern uint32 uint64_lo32_(uint64 a);
#define uint64_lo32(a)        BIGINT_IMP((uint32) (((uint32) (a)) \
                                                   & 0xFFFFFFFFUL), \
                                         uint64_lo32_(a))

extern int32 int64_hi32_(int64 a);
/* (right shift is performed on an unsigned type to ensure defined behaviour
   even if the most significant bit is set) */
#define int64_hi32(a)         BIGINT_IMP((int32) (((uint64) (a)) >> 32), \
                                         int64_hi32_(a))

extern uint32 uint64_hi32_(uint64 a);
#define uint64_hi32(a)        BIGINT_IMP((uint32) (((uint64) (a)) >> 32), \
                                         uint64_hi32_(a))

extern int64 int64_cast_(uint64 a);
#define int64_cast(a)         BIGINT_IMP((int64) (a), \
                                         int64_cast_(a))

extern uint64 uint64_cast_(int64 a);
#define uint64_cast(a)        BIGINT_IMP((uint64) (a), \
                                         uint64_cast_(a))

extern bool int64_is_negative_(int64 a);
#define int64_is_negative(a)  BIGINT_IMP(((int64) (a)) < 0, \
                                         int64_is_negative_(a))

extern bool int64_is_zero_(int64 a);
#define int64_is_zero(a)      BIGINT_IMP(((int64) (a)) == 0, \
                                         int64_is_zero_(a))

extern bool uint64_is_zero_(uint64 a);
#define uint64_is_zero(a)     BIGINT_IMP(((uint64) (a)) == 0, \
                                         uint64_is_zero_(a))

extern int64 int64_negate_(int64 a);
#define int64_negate(a)       BIGINT_IMP(-((int64) (a)), \
                                         int64_negate_(a))

/* At the time of writing GCC for Xap synthesizes incorrect code for basic
   arithmetic ("+" and "-") operations, so always use explicit function
   calls for those */
#ifdef XAPGCC
#define BIGINT_IMP_XAPGCC(intrinsic, function) (function)
#else
#define BIGINT_IMP_XAPGCC(intrinsic, function) \
    BIGINT_IMP((intrinsic), (function))
#endif

extern int64 int64_add_(int64 a, int64 b);
#define int64_add(a, b)       BIGINT_IMP_XAPGCC(((int64) (a)) \
                                                + ((int64) (b)), \
                                                int64_add_((a), (b)))

extern uint64 uint64_add_(uint64 a, uint64 b);
#define uint64_add(a, b)      BIGINT_IMP_XAPGCC(((uint64) (a)) \
                                                + ((uint64) (b)), \
                                                uint64_add_((a), (b)))

extern int64 int64_sub_(int64 a, int64 b);
#define int64_sub(a, b)       BIGINT_IMP_XAPGCC(((int64) (a)) \
                                                - ((int64) (b)), \
                                                int64_sub_((a), (b)))

extern uint64 uint64_sub_(uint64 a, uint64 b);
#define uint64_sub(a, b)      BIGINT_IMP_XAPGCC(((uint64) (a)) \
                                                - ((uint64) (b)), \
                                                uint64_sub_((a), (b)))

extern int64 int64_mul_(int64 a, int64 b);
#define int64_mul(a, b)       BIGINT_IMP(((int64) (a)) * ((int64) (b)), \
                                         int64_mul_((a), (b)))

extern uint64 uint64_mul_(uint64 a, uint64 b);
#define uint64_mul(a, b)      BIGINT_IMP(((uint64) (a)) * ((uint64) (b)), \
                                         uint64_mul_((a), (b)))

extern int64 int64_div_(int64 n, int64 d);
#define int64_div(n, d)       BIGINT_IMP(((int64) (n)) / ((int64) (d)), \
                                         int64_div_((n), (d)))

extern uint64 uint64_div_(uint64 n, uint64 d);
#define uint64_div(n, d)      BIGINT_IMP(((uint64) (n)) / ((uint64) (d)), \
                                         uint64_div_((n), (d)))

extern int64 int64_mod_(int64 n, int64 d);
#define int64_mod(n, d)       BIGINT_IMP(((int64) (n)) % ((int64) (d)), \
                                         int64_mod_((n), (d)))

extern uint64 uint64_mod_(uint64 n, uint64 d);
#define uint64_mod(n, d)      BIGINT_IMP(((uint64) (n)) % ((uint64) (d)), \
                                         uint64_mod_((n), (d)))

extern int64 int64_asl_(int64 in, unsigned int bits);
#define int64_asl(in, bits)   BIGINT_IMP(((int64) (in)) << (bits), \
                                         int64_asl_((in), (bits)))

extern uint64 uint64_asl_(uint64 in, unsigned int bits);
#define uint64_asl(in, bits)  BIGINT_IMP(((uint64) (in)) << (bits), \
                                         uint64_asl_((in), (bits)))

extern uint64 uint64_asr_(uint64 in, unsigned int bits);
#define uint64_asr(in, bits)  BIGINT_IMP(((uint64) (in)) >> (bits), \
                                         uint64_asr_((in), (bits)))


/* Some (simple) operations are always implemented as macros */

#define int64_mod16(n, d)     int64_divmod16((n), (d), NULL)
#define uint64_mod16(n, d)    uint64_divmod16((n), (d), NULL)
#define int64_abs(a)          (int64_is_negative(a) ? int64_negate(a) : (a))


#endif
