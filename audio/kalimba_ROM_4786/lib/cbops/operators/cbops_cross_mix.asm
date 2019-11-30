// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//  cross_mix operator
// DESCRIPTION:
//  operator that mixes two channels to two or more channels
//
// *****************************************************************************
#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.cross_mix;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.cross_mix[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.cross_mix.main;        // main function

.ENDMODULE;
// *************************************************************************************************
// MODULE:
//    $cbops.cross_mix.main
//
// DESCRIPTION:
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
// When using the operator the following data structure is used:
//      $cbops.cross_mix.INPUT1_START_INDEX_FIELD    ->  index of the input1  buffer
//      $cbops.cross_mix.INPUT2_START_INDEX_FIELD    ->  index of the input2  buffer
//      $cbops.cross_mix.OUTPUT1_START_INDEX_FIELD   ->  index of the output1 buffer
//                                                       y1 = a11*x1 + a12*x2
//      $cbops.cross_mix.OUTPUT2_START_INDEX_FIELD   ->  index of the output2 buffer
//                                                       if Negative no second output, else
//                                                       y2 = a21*x1 + a22*x2
//      $cbops.cross_mix.COEFF11_FIELD               ->  a11
//      $cbops.cross_mix.COEFF12_FIELD               ->  a12
//      $cbops.cross_mix.COEFF21_FIELD               ->  a21
//      $cbops.cross_mix.COEFF22_FIELD               ->  a22
//
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
// assume everything
// NOTES:
//   output samples are satuated to +-1
// *****************************************************************************************************
.MODULE $M.cbops.cross_mix.main;
   .CODESEGMENT CBOPS_CROSS_MIX_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.cross_mix.main:
   push rLink;
  #ifdef ENABLE_PROFILER_MACROS
      // start profiling if enabled
      .VAR/DM1 $cbops.profile_cross_mix[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_cross_mix;
      call $profiler.start;
  #endif

   // get the offset to the first read buffer to use
   r0 = M[r8 + $cbops.cross_mix.INPUT1_START_INDEX_FIELD];
   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the second read buffer to use
   r0 = M[r8 + $cbops.cross_mix.INPUT2_START_INDEX_FIELD];
   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I1
   I1 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L1
   L1 = r1;

   // get the offset to the first write buffer to use
   r0 = M[r8 + $cbops.cross_mix.OUTPUT1_START_INDEX_FIELD];
   // get the input buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // get a11 coefficient
   r4 = M[r8 + $cbops.cross_mix.COEFF11_FIELD];
   // get a12 coefficient
   r5 = M[r8 + $cbops.cross_mix.COEFF12_FIELD];

   // get the offset to the second write buffer to use
   r0 = M[r8 + $cbops.cross_mix.OUTPUT2_START_INDEX_FIELD];
   // negative means no second output buffer
   if NEG jump single_output;
   // get the input buffer write address
   r1 = M[r6 + r0];
   // store the value in I5
   I5 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L5
   L5 = r1;

   // get a21 coefficient
   r6 = M[r8 + $cbops.cross_mix.COEFF21_FIELD];
   // get a22 coefficient
   r7 = M[r8 + $cbops.cross_mix.COEFF22_FIELD];

   // cross mix loop:
   // y1 = a11*x1 + a12*x2
   // y2 = a21*x1 + a22*x2
   // for speed optimization read first samples ouside loop
   r0 = M[I0, 1];
   r10 = r10 - 1;
   rMAC = r0 * r4, r1 = M[I1, 1];
   do cross_mix_loop;
      rMAC = rMAC + r1*r5;
      rMAC = r0*r6, M[I4, 1] = rMAC, r0 = M[I0, 1];
      rMAC = rMAC + r1*r7, r1 = M[I1, 1];
      rMAC = r0*r4, M[I5, 1] = rMAC;
   cross_mix_loop:
   rMAC = rMAC + r1*r5;
   rMAC = r0*r6, M[I4, 1] = rMAC;
   rMAC = rMAC + r1*r7;
   M[I5, 1] = rMAC;
   jump end;


   single_output:
   // cross mix loop, only one output channel
   // y1 = a11*x1 + a12*x2
   // for speed optimization read first samples ouside loop
   r0 = M[I0, 1];
   r10 = r10 - 1;
   rMAC = r0 * r4, r1 = M[I1, 1];
   do cross_mix_loop_2;
      rMAC = rMAC + r1*r5,  r1 = M[I1, 1];
      rMAC = r0*r4, M[I4, 1] = rMAC, r0 = M[I0, 1];
   cross_mix_loop_2:
   rMAC = rMAC + r1*r5;
   M[I4, 1] = rMAC;

   end:
   L4 = 0;
   L5 = 0;
   L0 = 0;
   L1 = 0;
  #ifdef ENABLE_PROFILER_MACROS
      // stop profiling if enabled
      r0 = &$cbops.profile_cross_mix;
      call $profiler.stop;
  #endif

    // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
