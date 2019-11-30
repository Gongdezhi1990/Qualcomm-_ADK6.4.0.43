// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Scaling operator
//
// DESCRIPTION:
//  This operator scales the data by the supplied amount, using integer and
//  fractional multiplies. All integer multiplies use the (sat) flag to force
//  saturation if it occurs.
//
//  Note as Kalimba typically reads data from periphals as 16 bit words scaling
//  these values up to 24 bit representation can generate unexpected results.
//
//  If a positive number is scaled and the result becomes negative, the result
//  will be saturated. If external data is read by Kalimba as 16 bit data and
//  zero padded to 24 bit, the values will all be interpreted as positive
//  numbers, even if they were negative 16 bit numbers.
//
//  Therefore if data read from peripherals is to be scaled to 24 bit data it
//  is recommended you use the $cbuffer.FORCE_SIGN_EXTEND flag to force Kalimba
//  to sign extend the data as it reads it.
//
//  Furthermore this operator has two forms, $cbops.scale and
//  $cbops.scale_16_sat. The former scales the numbers treating them as 24
//  numbers throughout. The latter saturates the values to 16 bits so this
//  operator may be used to generate data aimed directly at peripherals.
//
//  This operator can be used in three 'modes', integer only, fractional only
//  and full. If a single integer multiply is required, set the fractional
//  amount to 1.0 (0x7FFFFF) and the post integer multiply amount to 1(0x1), and
//  a smaller loop is used which only includes a single multiply. If a
//  fractional multiply only is required, set both multiply amounts to 1 (0x1).
//  Again a smaller loop is used which only includes a single multiply. Finally
//  if any other combination is required, a third loop is used which includes
//  all three multiplies.
//
// When using this operator the following data structure is used:
//    - $cbops.scale.INPUT_START_INDEX_FIELD = the index of the input buffer
//    - $cbops.scale.OUTPUT_START_INDEX_FIELD = the index of the output
//       buffer
//    - $cbops.scale.PRE_INT_AMOUNT_FIELD = amount to multiply using an integer
//      mutliply
//    - $cbops.scale.FRAC_AMOUNT_FIELD = amount ot multiply using a fractional
//      multiply
//    - $cbops.scale.POST_INT_AMOUNT_FIELD = amount to multiply using an integer
//      multiply
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.scale;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.scale[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.scale.main;                   // main function

.ENDMODULE;

.MODULE $M.cbops.scale_16_sat;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.scale_16_sat[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.scale_16_sat.main;            // main function

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.scale.main
//
// DESCRIPTION:
//    Operator that scales the input word
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r10, I0, I4, L0, L4, DoLoop
//
// NOTES:
//    This operator treats values as 24 bit numbers troughout and saturates any
// results accordingly.
//
// *****************************************************************************
.MODULE $M.cbops.scale.main;
   .CODESEGMENT CBOPS_SCALE_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.scale.main:
   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_scale[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_scale;
      call $profiler.start;
   #endif

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.scale.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.scale.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // for speed pipeline the: read -> scale -> write
   r10 = r10 - 1;
   // first input
   r0 = M[I0,1];

   // read the multiply amounts to use from the parameter area
   r2 = M[r8 + $cbops.scale.PRE_INT_AMOUNT_FIELD];
   r3 = M[r8 + $cbops.scale.FRAC_AMOUNT_FIELD];
   r4 = M[r8 + $cbops.scale.POST_INT_AMOUNT_FIELD];

   // check which multiplies to do
   Null = r4 - 1;
   if NZ jump full_multiply;

   Null = r2 - 1;
   if Z jump frac_only_multiply;

   // first multiply - its either int or full both do this multiply
   r0 = r0 * r2 (int) (sat);

   Null = r3 - 0x7FFFFF;
   if NZ jump full_multiply;

   // only the integer multiply
   pre_int_only_multiply:

      // multiply already done

      do int_only_loop;
         // write the last sample and get the next
         r0 = M[I0,1], M[I4,1] = r0;

         // generate the next output sample
         r0 = r0 * r2 (int) (sat);
      int_only_loop:

   jump done;

   // only the fractional multiply
   frac_only_multiply:
      // first multiply - generate the first output sample
      r0 = r0 * r3 (frac);

      do frac_only_loop;
         // write the last sample and get the next
         r0 = M[I0,1], M[I4,1] = r0;

         // generate the next output sample
         r0 = r0 * r3 (frac);
      frac_only_loop:

   jump done;

   // all the multiplies
   full_multiply:
      // do the last two multiplies to generate the first sample
      r0 = r0 * r3 (frac);
      r0 = r0 * r4 (int) (sat);

      do full_multiply_loop;
         // write the last sample and get the next
         r0 = M[I0,1], M[I4,1] = r0;

         // generate the next output sample
         r0 = r0 * r2 (int) (sat);
         r0 = r0 * r3 (frac);
         r0 = r0 * r4 (int) (sat);
      full_multiply_loop:

   done:
   // zero the length registers and write the last sample
   L0 = 0, M[I4,1] = r0;
   L4 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_scale;
      call $profiler.stop;
      pop rLink;
   #endif
   rts;

.ENDMODULE;




// *****************************************************************************
// MODULE:
//    $cbops.scale_16_sat.main
//
// DESCRIPTION:
//    Operator that scales the input word
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-3, r10, I0, I4, L0, L4, DoLoop
//
// NOTES:
//    This operator handles inputs as 24 bit numbers and saturates outputs to 16
// bits allowing writing directly to periphals on chip.
//
//    On BC5 chips and above this operator is not required, it is more efficient to use
// $cbops.scale and enusre the $cbuffer.FORCE_STURATE flag is used when defining
// the port identifier:
//@verbatim
//    .CONST  $AUDIO_LEFT_OUT_PORT    ($cbuffer.WRITE_PORT_MASK + $cbuffer.FORCE_STURATE + 0);
//@endverbatim
//
// *****************************************************************************
.MODULE $M.cbops.scale_16_sat.main;
   .CODESEGMENT CBOPS_SCALE_16_SAT_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.scale_16_sat.main:
   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_scale_16_sat[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_scale_16_sat;
      call $profiler.start;
   #endif

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.scale.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.scale.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // for speed pipeline the: read -> scale -> write
   r10 = r10 - 1;
   // first input
   r0 = M[I0,1];

   // read the multiply amounts to use from the parameter area
   r2 = M[r8 + $cbops.scale.PRE_INT_AMOUNT_FIELD];
   r3 = M[r8 + $cbops.scale.FRAC_AMOUNT_FIELD];
   r4 = M[r8 + $cbops.scale.POST_INT_AMOUNT_FIELD];

   // check which multiplies to do
   Null = r4 - 1;
   if NZ jump full_multiply;

   Null = r2 - 1;
   if Z jump frac_only_multiply;

   // first multiply - its either int or full both do this multiply
   r0 = r0 * r2 (int) (sat);

   Null = r3 - 0x7FFFFF;
   if NZ jump full_multiply;

   // only the integer multiply
   pre_int_only_multiply:

      // multiply already done
      r0 = r0 ASHIFT 8;
      r0 = r0 ASHIFT -8;

      do int_only_loop;
         // write the last sample and get the next
         r0 = M[I0,1], M[I4,1] = r0;

         // generate the next output sample
         r0 = r0 * r2 (int) (sat);
         r0 = r0 ASHIFT 8;
         r0 = r0 ASHIFT -8;
      int_only_loop:

   jump done;

   // only the fractional multiply
   frac_only_multiply:
      // first multiply - generate the first output sample
      r0 = r0 * r3 (frac);
      r0 = r0 ASHIFT 8;
      r0 = r0 ASHIFT -8;

      do frac_only_loop;
         // write the last sample and get the next
         r0 = M[I0,1], M[I4,1] = r0;

         // generate the next output sample
         r0 = r0 * r3 (frac);
         r0 = r0 ASHIFT 8;
         r0 = r0 ASHIFT -8;
      frac_only_loop:

   jump done;

   // all the multiplies
   full_multiply:
      // do the last two multiplies to generate the first sample
      r0 = r0 * r3 (frac);
      r0 = r0 * r4 (int) (sat);
      r0 = r0 ASHIFT 8;
      r0 = r0 ASHIFT -8;

      do full_multiply_loop;
         // write the last sample and get the next
         r0 = M[I0,1], M[I4,1] = r0;

         // generate the next output sample
         r0 = r0 * r2 (int) (sat);
         r0 = r0 * r3 (frac);
         r0 = r0 * r4 (int) (sat);
         r0 = r0 ASHIFT 8;
         r0 = r0 ASHIFT -8;
      full_multiply_loop:

   done:
   // zero the length registers and write the last sample
   L0 = 0, M[I4,1] = r0;
   L4 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_scale_16_sat;
      call $profiler.stop;
      pop rLink;
   #endif
   rts;

.ENDMODULE;
