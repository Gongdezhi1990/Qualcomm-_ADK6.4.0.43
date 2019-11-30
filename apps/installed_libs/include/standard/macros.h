/* Copyright (c) 2016 Qualcomm Technologies International, Ltd. */
/*    */
/*
  FILE
  macros.h  -  commonly used macros

  DESCRIPTION
  This file should be #included by most C or C++ source files, whether
  it is being included in the firmware or host-side code.
*/

#ifndef MACROS_H
#define MACROS_H

#ifndef HYDRA_MACROS_H
#error "macros.h should only be include from hydra_macros.h for isolation."
#endif /* HYDRA_MACROS_H */

#include <stddef.h> /* For NULL */

/* Boolean values (never test for equality with TRUE) */
#ifndef FALSE
#define FALSE           (0)
#endif

#ifndef TRUE
#define TRUE            (1)
#endif

#include <string.h> /* For memcpy etc */

/* Inlined minimum, maximum and absolute (may evaluate expressions twice) */
#ifndef MAX
#define MAX(a,b)        (((a) < (b)) ? (b) : (a))
#endif

#ifndef MIN
#define MIN(a,b)        (((a) < (b)) ? (a) : (b))
#endif

#ifndef ABS
#define ABS(a)          (((a) < 0) ? -(a) : (a))
#endif

/* Loop forever */
#ifndef forever
#define forever         for(;;)
#endif

/* Shut lint up */
#ifndef UNUSED
#define UNUSED(x)       ((void)(/*lint -e{530}*/(x)))
#endif

#ifdef SUBSYSTEM_APPS
/* bzero is needed for bc_buf_adapter, define to
 * memset until implemented in crt. */
#define bzero(dst, n) (void)(memset((dst), 0, (n)))
#endif

#ifdef __GNUC__
/* In GCC with -ansi option the "inline" keyword is not recognised. We have to
 * use __inline__ instead - see the gcc man page */
#ifndef inline
#define inline  __inline__
#endif
#elif !defined(__cplusplus) && !defined(__KCC__)
/* The "inline" keyword is only valid in C++ or kcc */
#ifndef inline
#define inline
#endif

#elif defined(__KCC__)
/* __inline__ is used in the original vm code */
#define __inline__ inline
#endif


#if !defined(USHRT_MAX) && !defined(DESKTOP_TEST_BUILD)
#define USHRT_MAX       65535
#endif

/* Function attributes are only supported by GCC */
#ifdef __GNUC__
#define GCC_ATTRIBUTE(x) __attribute__ ((x))
#else
#define GCC_ATTRIBUTE(x)
#endif
#define NORETURN GCC_ATTRIBUTE(__noreturn__)

/* Memory barrier instructions are only valid for gcc */
#ifdef __GNUC__
#define MEMORY_BARRIER() __asm__ __volatile__("" ::: "memory")
#else
#define MEMORY_BARRIER()
#endif

/**************************
 * Bit manipulation macros
 **************************/
/*
 * This macro sets the value of the bits defined by the specified fname
 * by zeroing out other bits of the 16bit word.
 */
#define MAKE_BIT_FIELD(fname,value) ((fname##_MASK) & ((value) << fname##_LSB))
/*
 * This macro gets the value of the bits defined by the specified fname.
 */
#define GET_BIT_FIELD(wd,fname) (((wd) & fname##_MASK) >> fname##_LSB)
/*
 * This macro checks whether the bits defined by the specified fname is zero or not.
 */

#define CHECK_BIT_FIELD(wd,fname) ((wd) & fname##_MASK)
/*
 * This macro sets the bits defined by the specified fname by retaining
 * the other bits. Bit definitions for a particular
 * field name can be done by CREATE_FIELD_MASK(fname,lsb,msb)
 */
#define SET_BIT_FIELD(wd,fname,val) ((wd) =                             \
                                     ((wd) & (~fname##_MASK)) | ((fname##_MASK) & ((val) << fname##_LSB)))
/*
 * This macro sets the bits defined by the specified fname by retaining
 * the other bits of the 16 bit word. Bit definitions for a particular
 * field name can be done by CREATE_FIELD_MASK(fname,lsb,msb)
 */
#define SET_BIT_FIELD_U16(wd,fname,val) ((wd) =                             \
                                     (uint16)(((wd) & (~(uint16)fname##_MASK)) | ((fname##_MASK) & ((val) << fname##_LSB))))
/*
 * Example usage is CREATE_FIELD_MASK(FIELD_NAME,4,7);
 * where the field consists of bits4-7
 */
#define CREATE_FIELD_MASK(fname,lsb,msb)        \
    enum{                                       \
        fname##_MASK = CREATE_MASK(lsb,msb),    \
        fname##_LSB = lsb                       \
    }

#define CREATE_FIELD_MASK_U16(fname,lsb,msb)    \
    enum{                                       \
        fname##_MASK = CREATE_MASK_U16(lsb,msb),\
        fname##_LSB = lsb                       \
    }

/*
 * Internal macro
 */
#define CREATE_MASK(lsb,msb) ((((uint32)1U << (((msb) + 1)& 0x1F)) - 1) & \
                                       (~((1U << (lsb)) - 1)))

#define CREATE_MASK_U16(lsb,msb) ((uint16)((((uint32)1U << (((msb) + 1)& 0x1F)) - 1) & \
                                       (~((1U << (lsb)) - 1))))

#define SET_NTH_BIT_U16(wd,n) ((wd) |= (1 << (n)))
#define CLEAR_NTH_BIT_U16(wd,n) ((wd) &= ~(1 << (n)))
#define TEST_NTH_BIT_U16(wd,n) ((wd) & (1 << (n)))

#define IS_VALID_PTR(ptr) (NULL!=(ptr))
#define IS_IN_RANGE(minv,x,maxv) ((unsigned)(x-minv) <=(maxv-minv))

/****************************************************************************
NAME
    CONVERT_TO_UINTx  -  Converts multiple bit values into a single valie
*/
#define CONVERT_TO_UINT32(x) (((uint32)((x)[0]) << 24) | \
                              ((uint32)((x)[1]) << 16) | \
                              ((uint32)((x)[2]) << 8)  | ((x)[3]))

#define CONVERT_FROM_UINT32(x,y) ((x)[0] = (uint8)((y) >> 24), \
                                  (x)[1] = (uint8)((y) >> 16), \
                                  (x)[2] = (uint8)((y) >> 8),  \
                                  (x)[3] = (uint8)((y) & 0xFF))

#define CONVERT_TO_UINT16(x) (((uint16)((x)[0]) << 8) | ((uint16)((x)[1]) ))
/* Equivalent to CONVERT_TO_UINT16 for MMU buffer. This handles the case when
   the second byte is wrapped. */
#define MMU_CONVERT_TO_UINT16(x) (((uint16)(*(x)) << 8) | ((uint16)(*MMU_INC_SRC_PTR((x),1)) ))

#define CONVERT_FROM_UINT16(x,y) ((x)[0] = (uint8)((y) >> 8)); \
    ((x)[1] = (uint8)((y) & 0xFF))

/****************************************************************************
NAME
    CONVERT_TO_LE_UINTx  -  Little Endian variants of conversion macros
*/
#define CONVERT_TO_LE_UINT32(x) (((uint32)((x)[3]) << 24) | \
                                 ((uint32)((x)[2]) << 16) | \
                                 ((uint32)((x)[1]) << 8)  | ((x)[0]))
#define CONVERT_FROM_LE_UINT32(x,y) ((x)[3] = (uint8)((y) >> 24), \
                                     (x)[2] = (uint8)((y) >> 16), \
                                     (x)[1] = (uint8)((y) >> 8),  \
                                     (x)[0] = (uint8)((y) & 0xFF))
#define CONVERT_TO_LE_UINT16(x)     (uint16)(((x)[1] << 8) | (x)[0])
#define CONVERT_FROM_LE_UINT16(x,y) ((x)[1] = (uint8)((y) >> 8)); \
    ((x)[0] = (uint8)((y) & 0xFF))

/* NOTE - Make sure to include assert.h in C file */
/* Precondition required before running this function */
#define assert_precond(condition)  assert(condition)
/* Postcondition to verify at the end of this function */
#define assert_postcond(condition) assert(condition)

/**
 * Compile time assertion.
 *
 * Asserts condition at compile time.
 *
 * This can be used when a preprocessor condition can not (e.g. because
 * sizeof information is needed) and is preferable to a run-time assertion).
 */
#define COMPILE_TIME_ASSERT(cond, msg) \
    /*lint --e{754} --e{753} --e{514}*/struct compile_time_assert_ ## msg {        \
    int compile_time_assert_ ## msg [1 - (!(cond))*2]; \
}

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
 * \example
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

/**
 * Stringify an expression.
 *
 * Example:-
 *\verbatim

     #define X 1024
     #define Y X

     assert(CSR_STRINGIFY(X) == "X");
     assert(CSR_STRINGIFY(Y) == "Y");
     assert(CSR_STRINGIFY(a < b) == "a < b");

\endverbatim
 *
 * Known uses:-
 * - CSR_EXPAND_AND_STRINGIFY()
*/
#define CSR_STRINGIFY(x) #x

/**
 * Macro-expand and then stringify an expression.
 *
 * Example:-
 *\verbatim

     #define X 1024
     #define Y X

     assert(CSR_STRINGIFY(X) == "X");
     assert(CSR_STRINGIFY(Y) == "Y");

     assert(CSR_EXPAND_AND_STRINGIFY(X) == "1024");
     assert(CSR_EXPAND_AND_STRINGIFY(Y) == "1024");

\endverbatim
 *
 * The trick here is that the arg to EXPAND_AND_STRINGIFY gets fully expanded
 * before EXPAND_AND_STRINGIFY itself is expanded. So the STRINGIFY(x) macro
 * it references "sees" the already macro-expanded value of x (which it
 * stringifies as usual).
 *
 * Q: So if macro args are expanded before the macro bodies why aren't
 * arguments to the underlying STRINGIFY expanded anyway?
 *
 * A: Because arguments that are stringified # or pasted ## in that macro
 * are not expanded. # and ## wouldn't be much use if this wasn't the case.
 *
 * \ref http://gcc.gnu.org/onlinedocs/cpp/Argument-Prescan.html#Argument-Prescan
 */
#define CSR_EXPAND_AND_STRINGIFY(x) CSR_STRINGIFY(x)

/*****************************************************************************
 * unsized integer utilities
 *****************************************************************************/

/**
 * Integer Divide, rounded up.
 */
#define INT_DIV_ROUNDED_UP(n, d) \
    (((n) + ((d) - 1)) / (d))

/*****************************************************************************
 * pseudo bool utilities
 *****************************************************************************/
/**
 * Compare two boolean values
 * Lint complains with warning 679 about comparing pseudo bool values
 * because TRUE can be represented by multiple integer values. This
 * macro copes with that problem.
 */
 #define BOOL_EQUAL(a, b)      (!(a) == !(b))
 
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
 *****************************************************************************/

/**
 * MS octet of x
 *
 * \public \memberof uint16
 */
#define UINT16_MSO(x) \
    ((uint8)((x) >> 8))

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
    (((uint8)(mso) << 8) | (uint8)(lso))

/**
 * Swap octets of a uint16
 *
 * \public \memberof uint16
 */
#define UINT16_SWAP_ENDS(x) \
    UINT16_BUILD(UINT16_LSO(x), UINT16_MSO(x))

/*****************************************************************************
 * inline versions of memcpy and memset
 *****************************************************************************/

/**
 * Inline memcpy for:-
 *
 * a) running from RAM.
 * b) small copies that can be unrolled by the compiler. (e.g. writes to a
 * multi-word register).
 */
#define INLINE_MEMCPY(dst, src, n) \
    do { \
        size_t inline_memcpy_i; \
        for (inline_memcpy_i = 0; inline_memcpy_i < (n); ++inline_memcpy_i) { \
            (dst)[inline_memcpy_i] = (src)[inline_memcpy_i]; \
        } \
    } while (0)

/**
 * Inline memset for:-
 *
 * a) running from RAM.
 * b) small regions that can be unrolled by the compiler. (e.g. writes to a
 * multi-word register).
 */
#define INLINE_MEMSET(dst, val, n) \
    do { \
        size_t inline_memset_i; \
        for (inline_memset_i = 0; inline_memset_i < (n); ++inline_memset_i) { \
            (dst)[inline_memset_i] = val; \
        } \
    } while (0)

#ifdef DESKTOP_TEST_BUILD
/*
 * HERE: COMPLETE HEADER BROKEN MADNESS WARNING
 *
 * Somebody kludged some MMU types into the test version of macros.h,
 * presumably because they couldn't be bothered to find a sensible header.
 * We are now trying to avoid header duplication, because you never know
 * which header anything is actually including, so they are now here.
 *
 * They shouldn't be anywhere near macros.h in the first place, since
 * they are for buffer handling.  However, it seems they are required
 * by NFC.  They really need to have a proper header of their own,
 * but I don't have the affected builds to fix up.
 */

#include "hydra_types.h"

/* Now a couple of extra items to make the MMU work */

/* These functions wrap memcpy and memcmp to allow MMU buffer
 * wraparound to be implemented
 */
extern void *coal_memcpy(void *dest, const void *src, size_t n);
extern int coal_memcmp(const void *s1, const void *s2, size_t n);

#define memcpy coal_memcpy
#define memcmp coal_memcmp

extern void *memcpy_from_buf(void *dest, const void *src, size_t len);
extern void *memcpy_to_buf(void *dest, const void *src, size_t len);
extern void *memcpy_buf_to_buf(void *dest, const void *src, size_t len);

/* These functions allow safe mmu pointer increment (with wrap) */
#define MMU_INC_SRC_PTR(p, i) mmu_inc_src_ptr(p, i)
#define MMU_INC_DEST_PTR(p, i) mmu_inc_dest_ptr(p, i)

extern const uint8 *mmu_inc_src_ptr(const uint8 *p, uint16 i);
extern uint8 *mmu_inc_dest_ptr(uint8 *p, uint16 i);

/* Assert extensions */
#define assert_expr(x) \
    ((void)0)

#endif /* DESKTOP_TEST_BUILD */

#ifdef USE_BITFIELDS
#define NOT_A_BITFIELD(type, name, size)  unsigned int name:size
#else
#define NOT_A_BITFIELD(type, name, size)  type name
#endif

/*
 * These macros can be used to ensure that a type that is not explicitly
 * referenced is not elided from the DWARF.  They achieve this by declaring a
 * global of that type but putting it in a special section that contains only
 * these unreferenced symbols.  (kld appears to elide a section that is wholly
 * unreferenced, whereas an individual unreferenced symbol in a section
 * also containing referenced symbols is not eliminated.)
 *
 * These macros currently do nothing for tool chains other than kcc.
 */
#ifdef __KALIMBA__
#define UNREFERENCED_ATTR _Pragma("datasection UNREFERENCED")
#define PRESERVE_TYPE_FOR_DEBUGGING_(typename, tag) \
    /*lint --e{19} --e{129} Useless declarations */\
    UNREFERENCED_ATTR tag typename preserve_##tag##_##typename##_for_debugging;
#else
#define PRESERVE_TYPE_FOR_DEBUGGING_(typename, tag)
#endif
#define PRESERVE_ENUM_FOR_DEBUGGING(typename) \
        PRESERVE_TYPE_FOR_DEBUGGING_(typename, enum)

#define PRESERVE_STRUCT_FOR_DEBUGGING(typename) \
        PRESERVE_TYPE_FOR_DEBUGGING_(typename, struct)

#define PRESERVE_TYPE_FOR_DEBUGGING(typename) \
        PRESERVE_TYPE_FOR_DEBUGGING_(typename,)

#if defined(__KALIMBA__) && !defined(DESKTOP_TEST_BUILD)
/* This macro tells the linker to put functions that have it before their
 * definitions at the start of the image. This can be done for performance
 * benefits but also is a must for functions that deal with SQIF commands */
#define RUN_FROM_RAM _Pragma("codesection tcm")
#else
#define RUN_FROM_RAM
#endif

/* Same needs to be done for consts that SQIF code uses. */
#if defined(__KALIMBA__) && !defined(DESKTOP_TEST_BUILD)
#define PUT_INTO_RAM _Pragma("datasection dmtcm")
#else
#define PUT_INTO_RAM
#endif

/* Processor-specific macro to validate function pointers before they're
 * followed.
 *
 * The destination address has the bottom bit cleared, and the word at that
 * location examined to see if it is a push instruction (bits 10-12 set), as
 * 'all' functions start with a push instruction of some kind.
 */
#if defined(__KALIMBA__) && defined(DEBUG_CATCH_P1_INVALID_FN_PTRS)
#define VALIDATE_FN_PTR(p) \
    do { \
        if ( (*(uint16 *)(P1D_SQIF1_CACHED_LOWER + ((uint32)(p) & (0xfffffffeUL))) \
             & 0x1C00) != 0x1C00) \
            panic(PANIC_P1_INVALID_FUNCTION_POINTER); \
    } while (0)
#else
#define VALIDATE_FN_PTR(p)
#endif

#endif /* MACROS_H */
