/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//** 
 * \file pl_intrinsics.h
 * \ingroup platform
 *
 * Header file for Kalimba instrinsics
 *
 * NOTES : Allows Kalimba specific assembler code to be called from C code,
 * and provides equivalent code for non kalimba builds
 *
 ****************************************************************************/
#ifndef PL_INTRINSICS_H
#define PL_INTRINSICS_H

/****************************************************************************
Include Files
*/
#include "types.h"

/****************************************************************************
Public Macro Declarations
*/

/**
 * Macro to find the highest set bit.
 *
 * \note 
 * Assumes x is a MAX_SIGNDET_BITS bit unsigned int, and is greater than 0
 * Also due to how signdet works Mask cannot have its MSB set. 
 *
 * \par
 * CALLER MUST ENSURE THESE RESTRICTIONS ARE MET
 */

#define MAX_SIGNDET_BITS (DAWTH-2)

#define MAX_BIT_POS(x) (MAX_SIGNDET_BITS - PL_SIGNDET(x))

/****************************************************************************
Public Type Declarations
*/

/****************************************************************************
Global Variable Definitions
*/

#include "platform/pl_interrupt.h"

/* Register definitions needed for unit tests */
extern volatile CHIP_VERSION_T SUB_SYS_CHIP_VERSION;

/** Macros to lock and unlock IRQs on Kalimba. Map to code in interrupt.asm */
#define LOCK_INTERRUPTS interrupt_block()
#define UNLOCK_INTERRUPTS interrupt_unblock()

/* The atomic block macros can be used round sections of code that satisfy
 * the following criteria:
 *    * They do not call block_ or unblock_interrupts().
 *    * They do not adjust INT_UNBLOCK.
 *    * They do not exit the block by any means other than falling out of
 *      the bottom (no break, continue, goto or return).
 *
 * For safety it's advisable not to call any functions (otherwise one of
 * the above criteria could be violated accidentally. This applies doubly
 * to dereferencing function pointer to functions with a wide remit (such as
 * the radiosched slopseq functions and itimer functions).
 *
 * Example of use:
 *
 *     ATOMIC_BLOCK_START {
 *         atomic code;
 *     } ATOMIC_BLOCK_END;
 *
 * NOTE: This code is stolen from Bluecore - specifically, int.h.
 * Before making any improvements/optimisations, check their versions
 * to see if we can ride their coat-tails.
 */

#define ATOMIC_BLOCK_START \
    do { \
        interrupt_block();

#define ATOMIC_BLOCK_END \
        interrupt_unblock(); \
    } while(0)


/**
 * Maps to SIGNDET instruction on Kalimba - finds the number of redundant sign
 * bits.
 */
#define PL_SIGNDET(m) pl_sign_detect(m)

/****************************************************************************
Public Function Prototypes
*/

/**
 * Exposes the SIGNDET instruction
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm int pl_sign_detect(int input)
{
    @[
      .restrict input:bank1
      .restrict :bank1
    ]
    @{} = SIGNDET @{input};
}
#else /* DESKTOP_TEST_BUILD */
extern int pl_sign_detect(int input);
#endif /* DESKTOP_TEST_BUILD */

/**
 * Exposes the SIGNDET rMAC(B) instruction.
 * The result is in the range -8..63, i.e. must be declared signed.
 * The argument int48 is known to be synonymous with int64 on K32
 * i.e. does not mean the values are restricted to 48 significant bits.
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm int pl_sign_detect_long(int48 input)
{
    @[
      .restrict input:large_rmac
      .restrict :bank1
    ]
    @{} = SIGNDET @{input};
}
#else /* DESKTOP_TEST_BUILD */
extern int pl_sign_detect_long(int48 input);
#endif /* DESKTOP_TEST_BUILD */

/**
 * Exposes the ONEBITCOUNT instruction
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm unsigned pl_one_bit_count(unsigned input)
{
    @[
      .restrict input:bank1
      .restrict :bank1
    ]
    @{} = ONEBITCOUNT @{input};
}
#else /* !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__) */
extern unsigned pl_one_bit_count(unsigned input);
#endif /* !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__) */

/**
 * Exposes the MIN instruction. Note that the comparison is signed.
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm int pl_min(int x, int y)
{
    @[
      /* Not specifying bank1_with_rmac for y because I've seen this
       * cause the compiler to copy a bank1 reg to rMAC then use
       * rMAC for y.
       */
      .restrict y:bank1
      .restrict :bank1
      .target x
    ]
    @{} = MIN @{y};
}
#else
static inline int pl_min(int x, int y)
{
    return (x < y) ? x : y;
}
#endif

/**
 * The argument and result int48 is known to be synonymous with int64 on K32
 * i.e. does not mean the values are restricted to 48 significant bits.
 * TODO if needed, an inline assembly version could be written.
 */
static inline int48 pl_min_long(int48 x, int48 y)
{
    return (x < y) ? x : y;
}

#ifdef K32
#define pl_min_i32(x,y) pl_min(x,y)
#else
#define pl_min_i32(x,y) pl_min_long(x,y)
#endif

/**
 * Exposes the MAX instruction. Note that the comparison is signed.
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm int pl_max(int x, int y)
{
    @[
      /* Not specifying bank1_with_rmac for y because I've seen this
       * cause the compiler to copy a bank1 reg to rMAC then use
       * rMAC for y.
       */
      .restrict y:bank1
      .restrict :bank1
      .target x
    ]
    @{} = MAX @{y};
}
#else
static inline int pl_max(int x, int y)
{
    return (x > y) ? x : y;
}
#endif

/**
 * The argument and result int48 is known to be synonymous with int64 on K32
 * i.e. does not mean the values are restricted to 48 significant bits.
 * TODO if needed, an inline assembly version could be written.
 */
static inline int48 pl_max_long(int48 x, int48 y)
{
    return (x > y) ? x : y;
}

#ifdef K32
#define pl_max_i32(x,y) pl_max(x,y)
#else
#define pl_max_i32(x,y) pl_max_long(x,y)
#endif

/**
 * Exposes the ABS instruction.
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm unsigned int pl_abs(int x)
{
    @[
      .restrict x:bank1
      .restrict :bank1
    ]
    @{} = ABS @{x};
}
#else
extern unsigned int pl_abs(int x);
#endif

/**
 * The argument and result (u)int48 is known to be synonymous with (u)int64
 * on K32 i.e. does not mean the values are restricted to 48 significant bits.
 * TODO if needed, an inline assembly version could be written.
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
static inline uint48 pl_abs_long(int48 x)
{
    if (x < 0)
    {
        x = - x;
        /* Saturation */
        x -= (x < 0);
    }
    return (uint48)x;
}
#else
extern uint48 pl_abs_long(int48 x);
#endif

#ifdef K32
#define pl_abs_i32(x) pl_abs(x)
#else
#define pl_abs_i32(x) pl_abs_long(x)
#endif

extern unsigned pl_fractional_divide(unsigned num,unsigned den);

/** Obtain the return address of the current function.
 *  The result of the inline assembly version is only
 *  valid when called before the first function call within
 *  the function's body. (See B-227057 for more.)
 */
#ifdef __GNUC__
extern unsigned int pl_get_return_addr(void);
#else /* __GNUC__ */
/* KCC >= 5.0 has a builtin function for this, but
 * there doesnt' seem to be a macro like __KCC__ that
 * allows code to check which version of kcc is used
 * to build it. So always use the pre-kcc5 implementation.
 */
asm unsigned int pl_get_return_addr(void)
{
    @{} = rLINK;
}
#endif /* __GNUC__ */

/**
 * Execute a Kalimba NOP instruction, and prevent the compiler from
 * reordering code around it.
 * Expected to be of use in hardware driver code and the like.
 */
#if !defined(DESKTOP_TEST_BUILD) && !defined(__GNUC__)
asm void nop(void)
{
    @[
      .barrier
    ]
    nop;
}
#else
#define nop() ((void)0)
#endif

#ifdef DESKTOP_TEST_BUILD
/* System-time manipulation, for test builds only */
extern void test_set_time(TIME time);
extern void test_add_time(TIME_INTERVAL time);
#endif /* DESKTOP_TEST_BUILD */


#endif   /* PL_INTRINSICS_H */
