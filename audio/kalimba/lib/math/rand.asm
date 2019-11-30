// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef MATH_RAND_INCLUDED
#define MATH_RAND_INCLUDED

#include "math_library.h"

.MODULE $M.math.rand.variables;
   .CODESEGMENT PM;
   .DATASEGMENT DM;

   .CONST  $math.RAND_SEED       21845;
   
   .CONST  $math.RAND_MULT       -4508331;
   .CONST  $math.RAND_INC        17;
   
   .VAR    $math.rand_num = $math.RAND_SEED;

.ENDMODULE;


//******************************************************************************
// MODULE:
//    $math.rand
//
// DESCRIPTION:
//    y = rand(), generating a single random number
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - r0 = rand();
//
// TRASHED REGISTERS:
//    none
//
// CPU USAGE:
//    5 cycles
//
// NOTES:
//    The random number generated are in the range of [-1.0,1.0].
//
//******************************************************************************
.MODULE $M.math.rand;
   .CODESEGMENT MATH_RAND_PM;
   .DATASEGMENT DM;

   $math.rand:

   // load the previous seed
   r0 = M[$math.rand_num];
   // generate the new value
   r0 = r0 * $math.RAND_MULT(int);
   r0 = r0 + $math.RAND_INC;
   // save the value
   M[$math.rand_num] = r0;
   rts;

.ENDMODULE;





//******************************************************************************
// MODULE:
//    $math.vector_rand
//
// DESCRIPTION:
//    Calculate Y = rand(len(Y),1)
//
// INPUTS:
//    - I0 = &Y
//    - r10 = LENGTH(Y)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r10, DoLoop, I0
//
// CPU USAGE:
//    8 + n*2 cycles
//
// NOTES:
//    The random numbers generated are in the range of [-1.0,1.0].
//
//******************************************************************************
.MODULE $M.math.vector_rand;
   .CODESEGMENT MATH_VECTOR_RAND_PM;
   .DATASEGMENT DM;

   $math.vector_rand:

   // load the previous seed and multiplier
   r0 = M[$math.rand_num];
   r1 = $math.RAND_MULT;

   // generate one outside the loop and decrement r10
   r10 = r10 - 1;
   r0 = r0 * r1 (int);
   r0 = r0 + $math.RAND_INC;

   do loop;
      r0 = r0 * r1 (int), M[I0,MK1] = r0;
      r0 = r0 + $math.RAND_INC;
loop:

   // save the final value
   M[I0,MK1] = r0;
   // save the seed
   M[$math.rand_num] = r0;

   rts;

.ENDMODULE;

#endif  // MATH_RAND_INCLUDED
