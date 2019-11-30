// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.        http://www.csr.com
// 
//
// $Change$  $DateTime$
// *****************************************************************************

#ifndef STACK_HEADER_INCLUDED
#define STACK_HEADER_INCLUDED

// *****************************************************************************
// MODULE:
//    $stack.PUSH_ALL_C and $stack.POP_ALL_C
//
// DESCRIPTION:
//    Macros to push/pop all registers which the C calling-convention expects
//    will not be trashed over a function call.
//
// TRASHED REGISTERS:
//    none
//
// BUGS:
//    CS-124812-UG (draft ZG) sec 6.1 states that "For future
//    compatibility, hand-written assembly functions callable from C
//    should save and restore the value of all modify, length, and
//    base registers (including M3) that might be modified by the
//    called assembly."
//    Naughtily, we don't do this (FIXME). We're getting away with it
//    because as of kcc-26 the compiler only relies on M0/1/2. A future
//    compiler upgrade could in theory change this, and then we'd be
//    sorry. (However, the chance of the compiler finding a way to
//    intuit the use of circular buffer hardware -- base/length regs --
//    from C source seems minimal.)
//
// *****************************************************************************

// Uses the efficient multi-register stack op codes for KAL_ARCH3, KAL_ARCH4
// etc architectures
// The amount the SP is incremented by when PUSH_ALL_C macro is used
#define PUSH_ALL_C_SP_INC (21 * ADDR_PER_WORD)

#ifndef PUSH_ALL_C
   #define PUSH_ALL_C \
      pushm <r4, r5, r6, r7, r8, r9, rLink, rMACB>; \
      pushm <I0, I1, I2, I4, I5, I6>;        \
      pushm <M0, M1, M2, L0, L1, L4, L5>;
#endif

#ifndef POP_ALL_C
   #define POP_ALL_C \
      popm <M0, M1, M2, L0, L1, L4, L5>;  \
      popm <I0, I1, I2, I4, I5, I6>;      \
      popm <r4, r5, r6, r7, r8, r9, rLink, rMACB>;
#endif


// *****************************************************************************
// MODULE:
//    $stack.PUSH_ALL_ASM and $stack.POP_ALL_ASM
//
// DESCRIPTION:
//    Macros to push/pop all registers for ASM calling C-code
//
// TRASHED REGISTERS:
//    none
//
// *****************************************************************************
// The amount the SP is incremented by when PUSH_ALL_ASM macro is used
#define PUSH_ALL_ASM_SP_INC (16 * ADDR_PER_WORD)

#ifndef PUSH_ALL_ASM
   #define PUSH_ALL_ASM \
      pushm <rMAC, r0, r1, r2, r3, r10>; \
      pushm <I3, I7>;                    \
      pushm <M0, M1, M2, L0, L1, L4, L5>;\
      M0 = 0;M1 = 1;M2 = -1;\
      L0 = 0;L1 = 0;L4 = 0;L5 = 0;\
      r0 = M[$ARITHMETIC_MODE];          \
      push r0;                           \
      M[$ARITHMETIC_MODE] = 0;
#endif

#ifndef POP_ALL_ASM
   #define POP_ALL_ASM \
      pop r0;                           \
      M[$ARITHMETIC_MODE] = r0;         \
      popm <M0, M1, M2, L0, L1, L4, L5>;\
      popm <I3, I7>;                    \
      popm <rMAC, r0, r1, r2, r3, r10>; 
#endif
   
#endif // STACK_HEADER_INCLUDED
