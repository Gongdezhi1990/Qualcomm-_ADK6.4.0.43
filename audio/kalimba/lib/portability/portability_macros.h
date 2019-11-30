/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#ifndef PORTABILITY_MACROS_HEADER_INCLUDED
#define PORTABILITY_MACROS_HEADER_INCLUDED

/**
 * The macros defined in this file facilitate code portability between ARCH3,
 * Arch4 and Arch5 architectures.
 *
 *
*/

// Architecture and bitwidth defines
#if defined(KAL_ARCH4)

#ifndef KYMERA
    #define DAWTH                   32
    #define ADDR_PER_WORD           4
    #define LOG2_ADDR_PER_WORD      2
    #define PC_PER_INSTRUCTION      4
    #define LOG2_PC_PER_INSTRUCTION 2
    #define DAWTH_MASK              0xFFFFFFFF
    #define WORD_ADDRESS_MASK       0xFFFFFFFC
    #define MAXINT                  0x7FFFFFFF
    #define MININT                  0x80000000
#endif

    // in some places we have to multiply number of words x4 to compute byte address
    // Words2Addr and Addr2Words parameter 'reg' is handled as a signed integer
    #define Words2Addr(reg)    reg = reg ASHIFT LOG2_ADDR_PER_WORD
    #define Addr2Words(reg)    reg = reg ASHIFT -LOG2_ADDR_PER_WORD
    // Same as above but the source and destination registers don't coincide
    #define Words2AddrSD(src_reg, dest_reg)    dest_reg = src_reg ASHIFT LOG2_ADDR_PER_WORD
    #define Addr2WordsSD(src_reg, dest_reg)    dest_reg = src_reg ASHIFT -LOG2_ADDR_PER_WORD

    // Macro to convert Q9.23 data in a register to Q1.31
    #define LEFT_JUSTIFY_24BITS(reg) reg = reg ASHIFT (DAWTH - 24)

    // Integer bit mask Q8.24
    #define INT_MASK_Q8_N      0xFF000000

#elif defined(KAL_ARCH3)

#ifndef KYMERA
    #define DAWTH                   24
    #define ADDR_PER_WORD           1
    #define LOG2_ADDR_PER_WORD      0
    #define PC_PER_INSTRUCTION      1
    #define LOG2_PC_PER_INSTRUCTION 0
    #define DAWTH_MASK              0xFFFFFF
    #define WORD_ADDRESS_MASK       0xFFFFFF
    #define MAXINT                  0x7FFFFF
    #define MININT                  0x800000
#endif

    // no conversion necessary when word addressing is used (words and addresses are equivalent)
    #define Words2Addr(reg)
    #define Addr2Words(reg)
    #define Words2AddrSD(src_reg, dest_reg)
    #define Addr2WordsSD(src_reg, dest_reg)

    // This macro is normally used to convert Q9.23 data to Q1.31 which does not apply to this architecture
    #define LEFT_JUSTIFY_24BITS(reg)

    // Integer bit mask Q8.16
    #define INT_MASK_Q8_N      0xFF0000

#elif defined(KAL_ARCH5)

#ifndef KYMERA
    #define DAWTH                   24
    #define ADDR_PER_WORD           1
    #define LOG2_ADDR_PER_WORD      0
    #define PC_PER_INSTRUCTION      4
    #define LOG2_PC_PER_INSTRUCTION 2
    #define DAWTH_MASK              0xFFFFFF
    #define WORD_ADDRESS_MASK       0xFFFFFF
    #define MAXINT                  0x7FFFFF
    #define MININT                  0x800000
#endif

    // no conversion necessary when word addressing is used (words and addresses are equivalent)
    #define Words2Addr(reg)
    #define Addr2Words(reg)
    #define Words2AddrSD(src_reg, dest_reg)
    #define Addr2WordsSD(src_reg, dest_reg)

    // This macro is normally used to convert Q9.23 data to Q1.31 which does not apply to this architecture
    #define LEFT_JUSTIFY_24BITS(reg)
    // Integer bit mask Q8.16
    #define INT_MASK_Q8_N      0xFF0000

#elif defined(KAL_ARCH5B)
// Todo: check this values
#ifndef KYMERA
    #define DAWTH                   24
    #define ADDR_PER_WORD           1
    #define LOG2_ADDR_PER_WORD      0
    #define PC_PER_INSTRUCTION      4
    #define LOG2_PC_PER_INSTRUCTION 2
    #define DAWTH_MASK              0xFFFFFF
    #define WORD_ADDRESS_MASK       0xFFFFFF
    #define MAXINT                  0x7FFFFF
    #define MININT                  0x800000
#endif

    // no conversion necessary when word addressing is used (words and addresses are equivalent)
    #define Words2Addr(reg)
    #define Addr2Words(reg)
    #define Words2AddrSD(src_reg, dest_reg)
    #define Addr2WordsSD(src_reg, dest_reg)

    // Integer bit mask Q8.16
    #define INT_MASK_Q8_N      0xFF0000

#else
    #error Unsupported architecture
#endif

// Post modifier for indexed addressing
#define MK1 ADDR_PER_WORD
#define MK2 2*MK1

// macro for converting floating-point style literals between different Q formats
// The 'Qfmt_' macro can be used to automatically shift fractional constant 'X'
// for an arbitrary Q format. The result is X in Q[R].[F] where F = DAWTH - R.
// For example,
// .CONST PI_IN_Q4_F Qfmt_(3.141592653589793, 4); // arch3: Q4.20, arch4: Q4.28
#define Qfmt_(X, R) ( (X) * (2.0 ** ( 1 - (R) )) )


#endif // PORTABILITY_MACROS_HEADER_INCLUDED
