/****************************************************************************
 * COMMERCIAL IN CONFIDENCE
* Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
 *
 * Cambridge Silicon Radio Ltd
 * http://www.csr.com
 *
 ************************************************************************//**
 * \file pl_intrinsics.c
 * \ingroup platform
 *
 * C implementation of intrinsic operations on Kalimba
 *
 ****************************************************************************/

/* This is only needed for host-based unit tests */
#ifdef DESKTOP_TEST_BUILD
 
/****************************************************************************
Include Files
*/
#include "platform/pl_intrinsics.h"
#include "types.h"
#include "io_defs.h"

/****************************************************************************
Private Macro Declarations
*/

/****************************************************************************
Global Variable Definitions
*/


/*
 * Register definitions for host based tests.
 */

volatile unsigned  STACK_POINTER = 0;
#if defined(CHIP_BASE_CRESCENDO) || defined(CHIP_NAPIER)
volatile clock_divide_rate_enum CLOCK_DIVIDE_RATE = 0;
#else /* CHIP_BASE_CRESCENDO */
volatile uint24 CLOCK_DIVIDE_RATE = 0;
#endif /* CHIP_BASE_CRESCENDO */
volatile CHIP_VERSION_T SUB_SYS_CHIP_VERSION = 0;

#ifdef K32
#define MAX_UNSIGNED_LONG_LONG_BITS 62
#else
#define MAX_UNSIGNED_LONG_LONG_BITS 46
#endif

/****************************************************************************
Private Type Declarations
*/

/****************************************************************************
Private Constant Declarations
*/

/****************************************************************************
Private Variable Definitions
*/

/****************************************************************************
Private Function Prototypes
*/

/****************************************************************************
Private Function Definitions
*/

/****************************************************************************
Public Function Definitions
*/

/**
 * NAME
 *   pl_sign_detect_asm
 *
 * \brief C version of the SIGNDET asm instruction on Kalimba
 *
 * FUNCTION
 *   From the "Kalimba DSP User Guide" (CS-101693-UG):
 *
 *   The SIGNDET kalimba instruction gives the number of redundant sign bits
 *   in the source operarand. For example
 *    0000 1101 0101 0101 1100 1111 - has 3 redundant sign bits
 *    1001 0101 0101 0100 0111 1111 - has 0 redundant sign bits
 *    0000 0000 0000 0000 0000 0001 - has 22 redundant sign bits
 *    1111 1111 1111 1111 1111 1111 - has 23 redundant sign bits
 *    0000 0000 0000 0000 0000 0000 - has 23 redundant sign bits (special case)
 *
 * \param[in] input  Operand on which to find the number of redudant sign bits
 *
 * \return Number of redundant sign bits - 0 to 23.
 *
 * \note
 *   Unlike the Kalimba version, can only operate on 24 bit operands
 *
 */
int pl_sign_detect(int x)
{
    unsigned i;
    int y;

#if defined(__GNUC__) && !defined(K32)
    /* emulating CPU's native word is wider than emulated */
    x <<= 8;
#endif
    for (i=0, y=x<<1; i <= MAX_SIGNDET_BITS && (y ^ x) >= 0; ++i, y<<=1) {}
    return i;
}

int pl_sign_detect_long(int48 x)
{
    unsigned i;
    int48 y;

#if defined(__GNUC__) && !defined(K32)
    /* emulating CPU's native word is wider than emulated */
    x <<= 16;
#endif
    for (i=0, y=x<<1; i <= MAX_UNSIGNED_LONG_LONG_BITS && (y ^ x) >= 0; ++i, y<<=1) {}
    return i;
}

unsigned pl_fractional_divide(unsigned num,unsigned den)
{
   /* Blank function just return */
   return 0;
}

/**
 * \brief Return the number of 1 bits in the input
 * \param[in] input argument
 * \return    Number of bits in the input which are 1
 */
unsigned pl_one_bit_count(unsigned input)
{
    unsigned c = 0;
    while (input != 0)
    {
        c += (input & 1);
        input >>= 1;
    }
    return c;
}

/**
 * \brief Emulate saturating ABS instruction
 */
unsigned int pl_abs(int x)
{
#if defined(__GNUC__) && !defined(K32)
    /* Emulating CPU's native word is wider than emulated; shift so that the
     * negation of 0x80,0000 overflows on the emulating CPU. */
    x <<= 8;
#endif
    if (x < 0)
    {
        x = - x;
        if (x < 0)
        {
            /* saturate; this decrement will wrap */
            x -= 1;
        }
    }
#if defined(__GNUC__) && !defined(K32)
    x >>= 8;
#endif
    return (unsigned) x;
}

/**
 * \brief Saturating abs for double words
 */
uint48 pl_abs_long(int48 x)
{
#if defined(__GNUC__) && !defined(K32)
    /* Emulating CPU's native word is wider than emulated; shift so that the
     * negation of 0x8000,0000,0000 overflows on the emulating CPU. */
    x <<= 16;
#endif
    if (x < 0)
    {
        x = - x;
        if (x < 0)
        {
            /* saturate; this decrement will wrap */
            x -= 1;
        }
    }
#if defined(__GNUC__) && !defined(K32)
    x >>= 16;
#endif
    return (uint48) x;
}


/**
 * \brief Stub for obtaining current function's return address
 */
unsigned int pl_get_return_addr()
{
    return 0;
}

#endif /* DESKTOP_TEST_BUILD */
