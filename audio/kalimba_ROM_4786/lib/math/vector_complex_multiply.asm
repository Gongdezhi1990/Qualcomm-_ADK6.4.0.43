// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef MATH_VECTOR_COMPLEX_MULTIPLY_INCLUDED
#define MATH_VECTOR_COMPLEX_MULTIPLY_INCLUDED

#include "math_library.h"

// *****************************************************************************
// MODULE:
//    $math.vector_complex_multiply
//
// DESCRIPTION:
//    Library subroutine to multiply 2 vectors of complex numbers together.
//  Real and imaginary components are in separate vectors, and for minimum CPU
//  cycle usage the real and imaginary vectors should be in different data
//  memories.
//
// INPUTS:
//    - I0 = input vector 1 real components
//    - I4 = input vector 1 imag components
//    - I1 = input vector 2 real components
//    - I5 = input vector 2 imag components
//    - I2 = output vector  real components
//    - I6 = output vector  imag components
//    - r10 = size of vector (n)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-r5, r10, DoLoop
//
// CPU USAGE:
//    for n odd:   7 + 4n Cycles
//        n even: 10 + 4n Cycles
//
// *****************************************************************************
.MODULE $M.math.vector_complex_multiply;
   .CODESEGMENT MATH_VECTOR_COMPLEX_MULTIPLY_PM;
   .DATASEGMENT DM;

$math.vector_complex_multiply:

   // handle odd and even vector sizes differently
   Null = r10 AND 1;
   if NZ jump n_odd;
      // 2 multiplies are handled outside the loop so decrement n by 2
      // and carry out 1 of the complex multiplies here
      r10 = r10 - 2;
      // for iteration 0:

      // read real1
      // and read imag2
      r0 = M[I0,MK1], r3 = M[I5,MK1];
      // imag2 * real1
      // and read imag1
      // and read real2
      rMAC = r3 * r0, r2 = M[I4,MK1], r1 = M[I1,MK1];
      // imag1 * real2
      rMAC = rMAC + r2 * r1;
      // real1 * real2
      // and write imag_result
      rMAC = r0 * r1, M[I6,MK1] = rMAC;
      // imag1 * imag2
      rMAC = rMAC - r2 * r3;
      // write real_result
      M[I2,MK1] = rMAC;
n_odd:

   // divide count by 2 - since we process 2 complex multiplies per loop
   r10 = r10 LSHIFT -1;

   // carry out half of an intial multiply outside of the loop
   // read real1        (for iteration 0)
   // and read imag2    (for iteration 0)
   r0 = M[I0,MK1], r3 = M[I5,MK1];
   // imag2 * real1     (for iteration 0)
   // and read imag1    (for iteration 0)
   // and read real2    (for iteration 0)
   rMAC = r3 * r0, r2 = M[I4,MK1], r1 = M[I1,MK1];
   // imag1 * real2     (for iteration 0)
   // and read real1    (for iteration 2i+1)
   // and read imag2    (for iteration 2i+1)
   rMAC = rMAC + r2 * r1, r4 = M[I0,MK1], r5 = M[I5,MK1];


   // carry out 2 multiples per loop, these are pipelined to acheive the
   // minimum cycle count
   do loop;

      // real1 * real2           (for iteration 2i)
      // and write imag_result   (for iteration 2i)
      // and read real2          (for iteration 2i+1)
      rMAC = r0 * r1, M[I6,MK1] = rMAC, r1 = M[I1,MK1];

      // imag1 * imag2           (for iteration 2i)
      // and read real1          (for iteration 2i+2)
      rMAC = rMAC - r2 * r3, r0 = M[I0,MK1];

      // imag2 * real1           (for iteration 2i+1)
      // and write real_result   (for iteration 2i)
      // and read image1         (for iteration 2i+1)
      rMAC = r5 * r4, M[I2,MK1]= rMAC, r2 = M[I4,MK1];

      // imag1 * real2           (for iteration 2i+1)
      // and read image2         (for iteration 2i+2)
      rMAC = rMAC + r2 * r1, r3 = M[I5,MK1];

      // real1 * real2           (for iteration 2i+1)
      // and write imag_result   (for iteration 2i+1)
      // and read real1          (for iteration 2i+3)
      rMAC = r4 * r1, M[I6,MK1] = rMAC, r4 = M[I0,MK1];

      // imag1 * imag2           (for iteration 2i+1)
      // and read real2          (for iteration 2i+2)
      rMAC = rMAC - r5 * r2, r1  = M[I1,MK1];

      // imag2 * real1           (for iteration 2i+2)
      // and write real_result   (for iteration 2i+1)
      // and read imag1          (for iteration 2i+2)
      rMAC = r3 * r0, M[I2,MK1]= rMAC, r2  = M[I4,MK1];

      // imag1 * real2           (for iteration 2i+2)
      // and read imag2          (for iteration 2i+3)
      rMAC = rMAC + r2 * r1, r5 = M[I5,MK1];

loop:

   // carry out the remaining half of the final complex multiply
   // real1 * real2       (for iteration 2i+4)
   rMAC = r0 * r1, M[I6,MK1] = rMAC;
   // imag1 * imag2       (for iteration 2i+4)
   rMAC = rMAC - r2 * r3;
   //  write real_result  (for iteration 2i+4)
   M[I2,MK1]= rMAC;
   rts;

.ENDMODULE;

#endif // MATH_VECTOR_COMPLEX_MULTIPLY_INCLUDED
