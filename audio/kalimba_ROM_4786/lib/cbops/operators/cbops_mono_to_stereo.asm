// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    mono_to_stereo operator
//
// DESCRIPTION:
//    This operator does a small algorithm to convert mono audio (left channel of
//    stereo input) to stereo audio. The result is a simulation of stereo sound
//    that is easier to listen to than a straight 1 to 2 copy (double_copy operator).
//
// When using the operator the following data structure and parameters is used:
//    - $cbops.mono_to_stereo.INPUT_START_INDEX_FIELD = the index of the input
//       buffer
//    - $cbops.mono_to_stereo.OUTPUT_1_START_INDEX_FIELD = the index of the output
//       1 buffer
//    - $cbops.mono_to_stereo.OUTPUT_2_START_INDEX_FIELD = the index of the output
//       2 buffer
//    - $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD = the index of the delay
// (circular) buffer, from which the delayed audio samples are read.
//    - $cbops.mono_to_stereo.RATIO = determines how much stereo effect to put in,
//      put it to 0 means no effect is added, output = delayed input,
// put it to 1 means max effect.
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.mono_to_stereo;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.mono_to_stereo[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.mono_to_stereo.reset,       // reset function
      $cbops.function_vector.NO_FUNCTION,    // amount to use function
      &$cbops.mono_to_stereo.main;              // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.mono_to_stereo.reset
// DESCRIPTION:
//    This reset function will clear the delay buffer
// INPUTS:
//    None.
// OUTPUTS:
//    None.
// *****************************************************************************

.MODULE $M.cbops.mono_to_stereo.reset;
   .CODESEGMENT CBOPS_MONO_TO_STEREO_RESET_PM;
   .DATASEGMENT DM;


$cbops.mono_to_stereo.reset:

   // push rLink onto stack
   push rLink;

   //** function starts

   // setup the AG for delay buffer writting
   // get the delay buffer struc address
   r0 = M[r8 + $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD];
   // get the delay buffer write address
   call $cbuffer.get_write_address_and_size;
   // I4 for the delay buffer write
   I4 = r0;
   // backup this write address
   r3 = r0;
   L4 = r1;

   r10 = r1 + 1;
   r0 = 0;

   // write 0 to the whole buffer
   do reset_loop;
      M[I4, 1] = r0;
reset_loop:

   // restore the current delay buffer write address
   r0 = M[r8 + $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD];
   r1 = r3;
   call $cbuffer.set_write_address;
   L4 = 0;


   //** function ends
   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.mono_to_stereo.main
//
// DESCRIPTION:
//    Operator that convert mono audio to stereo audio. The user has control of
//    the delay amount and effect ratio.
//    Due to the shortage of cbuffer AG, the operation is divided into 2 stages.
//    First one copies samples from DAC out buffer to delay buffer; second one
//    reads both DAC out buffer and delay buffer, then process the samples and
//    send it to output port.
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r9 = link register for PC
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0-5, r10, I0, L0, I1, L1, I4, L4, I5, L5, M0 DoLoop
//
// *****************************************************************************

.MODULE $M.cbops.mono_to_stereo.main;
   .CODESEGMENT CBOPS_MONO_TO_STEREO_MAIN_PM;
   .DATASEGMENT DM;

   //.VAR $LoopCountStore;


$cbops.mono_to_stereo.main:

   // push rLink onto stack
   push rLink;


#ifdef ENABLE_PROFILER_MACROS
   // start profiling if enabled
   .VAR/DM1 $cbops.profile_mono_to_stereo[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
   r0 = &$cbops.profile_mono_to_stereo;
   call $profiler.start;
#endif


   // ** adc read buffer I0, L0; delay buffer write I4, L4
   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.mono_to_stereo.INPUT_START_INDEX_FIELD];
   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // r4 stores the input buffer start address
   // for use in the second loop
   r4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;
   // store the input buffer length in r5
   // for use in second loop
   r5 = r1;


   // get the delay buffer struc address
   r0 = M[r8 + $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD];
   // get the delay buffer write address
   call $cbuffer.get_write_address_and_size;
   // I4 for the delay buffer write
   I4 = r0;
   L4 = r1;


   // preserve r10 value
   r3 = r10;

   do loop;
      // simply copy the input to delay buffer
      r2 = M[I0,1];
      M[I4,1] = r2;
loop:

   L0 = 0;

   // store back the current delay buffer write address
   r0 = M[r8 + $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD];
   r1 = I4;
   call $cbuffer.set_write_address;
   // M[r0+2] = r1;
   L4 = 0;


   // ** now goto the second loop


   // get the offset to the first write buffer to use
   r0 = M[r8 + $cbops.mono_to_stereo.OUTPUT_1_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // get the offset to the second write buffer to use
   r0 = M[r8 + $cbops.mono_to_stereo.OUTPUT_2_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I5
   I1 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L5
   L1 = r1;


   // setup the current sample pointers
   // restore the save from last loop
   I0 = r4;
   L0 = r5;


   // setup the delay sample pointers
   // get the offset to the delay buffer read entry
   r0 = M[r8 + $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD];
   // get the read entry pointer of delay buffer
   call $cbuffer.get_read_address_and_size;
   // current sample pointer in I1
   I5 = r0;
   L5 = r1;


   // restore the loop counter here to free r3
   r10 = r3 - 1;

   // * Due to the way of parallelism applied in this algorithm,
   //   the first must be taken out of the main loop.

   // ratio of the stereo effect, passed in as a parameter
   r1 = M[r8 + $cbops.mono_to_stereo.RATIO];
   r0 = 1.0;


   // read and process the first element
   r0 = r0 - r1, r2 = M[I5,1], r3 = M[I0,1];

   r2 = r2 * r0 (frac);
   r5 = r3 * r1 (frac);
   r4 = r2 - r5;
   r5 = r2 + r5, r2 = M[I5,1], r3 = M[I0,1];

   do loop2;

      // OP_left  = IP(t-delay) * Coef_1 - IP(t) * Coef_2
      // OP_right = IP(t-delay) * Coef_1 + IP(t) * Coef_2

      //** the algorithm starts here

      // write the results from last round of loop
      // first term in algorithm
      r2 = r2 * r0 (frac), M[I1,1] = r4, M[I4,1] = r5;
      // second term in algorithm
      r5 = r3 * r1 (frac);

      // left channel
      r4 = r2 - r5;
      // read the input for next round of loop
      // right channel
      r5 = r2 + r5, r2 = M[I5,1], r3 = M[I0,1];

      //** algorithm ends

loop2:

   // dummy read to move the read pointer back
   r2 = M[I5,-1], r3 = M[I0,-1];

   // write the last result
   M[I1,1] = r4, M[I4,1] = r5;


   // Zero the length register
   L4 = 0;
   L1 = 0;

   L0 = 0;
   // store the current delay buffer read pointer
   r0 = M[r8 + $cbops.mono_to_stereo.DELAY_BUF_INDEX_FIELD];
   r1 = I5;
   call $cbuffer.set_read_address;
   L5 = 0;


#ifdef ENABLE_PROFILER_MACROS
   // stop profiling if enabled
   r0 = &$cbops.profile_mono_to_stereo;
   call $profiler.stop;
#endif

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;
