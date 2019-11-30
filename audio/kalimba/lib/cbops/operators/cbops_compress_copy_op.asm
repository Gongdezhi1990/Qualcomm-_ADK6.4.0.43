// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    compress_copy operator
//
// DESCRIPTION:
//    This operator does a straight forward copy of samples from the input
// buffer to the output buffer, with 1dB dynamic range compression applied.
//
// When using the operator the following data structure is used:
//    - $cbops.compress_copy_op.INPUT_START_INDEX_FIELD = the index of the input
//       buffer
//    - $cbops.compress_copy_op.OUTPUT_START_INDEX_FIELD = the index of the output
//       buffer
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.compress_copy_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.compress_copy_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.compress_copy_op.main;        // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.compress_copy_op.main
//
// DESCRIPTION:
//    Operator that copies the input sample to the output with bit shifting and
//    dynamic range compression applied.
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
//    r0-3, r10, I0, L0, I4, L4, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.compress_copy_op.main;
   .CODESEGMENT CBOPS_COMPRESS_COPY_OP_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.compress_copy_op.main:

   // start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_compress_copy_op[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_compress_copy_op;
      call $profiler.start;
   #endif

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.compress_copy_op.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.compress_copy_op.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   r2 = 1.0;

   r3 = M[r8 + $cbops.compress_copy_op.SHIFT_AMOUNT];


   do loop;
      r1 = M[I0, 1];          // r0 = x

      r1 = r1 ASHIFT r3;      // bit shifting

      r0 = r1 * 0.000030517578 (int); // x*(1/0x8000)
                              // 0.000030517578 = 1/0x8000

      r0 = r0 * r0 (frac);    // power of 2
      r0 = r0 * r0 (frac);    // power of 4

      r0 = r0 * $COMPRESS_RANGE (frac); // times compress margin

      r0 = r2 - r0;           // r0 = gain

      r1 = r1 * r0 (frac);    // apply gain

      M[I4, 1] = r1;          // write the compressed result
   loop:

   // zero the length registers and write the last sample
   L0 = 0;
   L4 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_compress_copy_op;
      call $profiler.stop;
      pop rLink;
   #endif

   rts;

.ENDMODULE;
