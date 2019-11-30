// *****************************************************************************
// Copyright (c) 2008 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef MATH_DIV48_INCLUDED
#define MATH_DIV48_INCLUDED

#include "math_library.h"

// ***************************************************************************
//
// FUNCTION:
//    Div48
//
// DESCRIPTION:
//    Fractional integer division of two 48 bit numbers.
//    L_num / L_denom.
//    L_num and L_denom must be positive and L_num < L_denom.
//    L_denom = denom_hi<<16 + denom_lo<<1
//    denom_hi is a normalized number.
//
// INPUTS:
//    - r0 = num_l
//    - r1 = num_h
//    - r2 = denom_l
//    - r3 = denom_h
//
// OUTPUTS:
//    - rMAC = A[m]  L_num * (1/L_denom)
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r4, r5, r6, r9, rMAC
//
// CPU USAGE:
//
// NOTES:
//    PROGRAM MEMORY:    words
//
//    Inputs :
//
//    L_num
//              48 bit long signed integer (Word32) whose value falls in the
//              range : 0x0000 0000 < L_num < L_denom
//
//    L_denom = denom_hi<<16 + denom_lo<<1      (DPF)
//
//    denom_hi
//              24 bit positive normalized integer whose value falls in the
//              range : 0x4000 < hi < 0x7fff
//    denom_lo
//              24 bit positive integer whose value falls in the
//              range : 0 < lo < 0x7fff
//
//    Return Value :
//
//    L_div
//              48 bit long signed integer (Word32) whose value falls in the
//              range : 0x0000 0000 <= L_div <= 0x7fff ffff.
//
//    Algorithm:
//
//    - find = 1/L_denom.
//       First approximation: approx = 1 / denom_hi
//       1/L_denom = approx * (2.0 - L_denom * approx )
//
//    -  result = L_num * (1/L_denom)
//
//
// ****************************************************************************
.MODULE $M.math.div48;
    .CODESEGMENT MATH_DIV48_PM;
	.DATASEGMENT DM;

$math.div48:
$div48:

   // First approximation: 1 / L_denom = 1/denom_hi
   // approx = div_s ((Word16) 0x3fff, denom_hi);

   // r0 contains num_l
   // r1 contains num_h
   // r2 contains denom_l
   // r3 contains denom_h

   rMAC = 0.249999880790710;    // 0x1fffff
   r0 = -1;                     // 0xffffff
   rMAC0 = r0;
   Div = rMAC/r3;
   r4 = DivResult;
   r5 = 1;
   // 1/L_denom = approx * (2.0 - L_denom * approx)
   // L_32 = Mpy_32_16 (denom_hi, denom_lo, approx);
   // Multiply LSW0 (denom_lo) and MSW1(approx)
   r2 = r2 * r4 (frac);
   // Shift rMAC 16 bits to the right
   rMAC = r2 * r5;
   // Multiply MSW0(denom_hi) and MSW1(approx)
   rMAC = rMAC + r3 * r4 (SS);

   r2 = rMAC LSHIFT DAWTH;
   // Setup DPF
   r2 = r2 LSHIFT -1;
   r3 = rMAC LSHIFT 0;

   // L_32 = L_sub ((Word32) 0x7fffffffL, L_32);
   r6 = 1.0;  // 0x7fffff
   // Setup DPF
   r5 = 1.0;  // 0x7fffff
   r5 = r5 - r2;
   r6 = r6 - r3 - borrow;

   end_TWOmApp:

   // L_Extract (L_32, &hi, &lo);
   // hi is in r6 and lo in r5

   // L_32 = Mpy_32_16 (hi, lo, approx);
   r2 = 1;
   // Multiply LSW0 (LO) and MSW1(approx)
   r5 = r5 * r4 (frac);
   // Shift rMAC 16 bits to the right
   rMAC = r5 * r2;
   // Multiply MSW0(HI) and MSW1(approx)
   rMAC = rMAC + r6 * r4 (SS);

   // L_num * (1/L_denom)
   // L_Extract (L_32, &hi, &lo);
   // lo
   r2 = rMAC LSHIFT DAWTH;
   // Setup DPF
   r2 = r2 LSHIFT -1;
   // hi
   r3 = rMAC LSHIFT 0;

   // L_Extract (L_num, &n_hi, &n_lo);
   // n_hi is in r1 and n_lo in r0

   // L_32 = Mpy_32 (n_hi, n_lo, hi, lo);
   // L_32 = Mpy_32 (r1,   r0,   r3, r2);
   r5 = 1;
   // n_lo * hi
   r0 = r0 * r3 (frac);
   rMAC = r0 * r5;
   // lo * n_hi
   r2 = r2 * r1 (frac);
   rMAC = rMAC + r2 * r5;
   // n_hi * hi
   rMAC = rMAC + r1 * r3 (SS);

   // L_32 = L_shl (L_32, 2);

   rMAC = rMAC ASHIFT 2;
   rts;

.ENDMODULE;

#endif // MATH_DIV48_INCLUDED
