// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Two to one channel copy operator
//
// DESCRIPTION:
//    Operator to copy data from one source into two separate sinks, for example
// to generate sidetones. The data read from the ADC is copied into two buffers,
// one buffer for further processing before transmitting, and one buffer is
// routed out to the DAC for the user.
//
//  The input samples are copied to both of the output buffers
// @verbatim
//      Input1 data                       Output data buffer
//      -----------------------+-------------------->
//                             |
//                             |
//      ------------------------
//      Input2 data
//
// @endverbatim
//
// When using the operator the following data structure is used:
//    - $cbops.one_to_two_chan_copy.INPUT_START_INDEX_FIELD = index of the input
//       buffer
//    - $cbops.one_to_two_chan_copy.OUTPUT_A_START_INDEX_FIELD = index of first
//       output buffer
//    - $cbops.one_to_two_chan_copy.OUTPUT_B_START_INDEX_FIELD = index of second
//       output buffer
//
// *****************************************************************************


#include "stack.h"
#include "cbops.h"

.MODULE $M_cbops.two_to_one_chan_copy;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.two_to_one_chan_copy[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset vector
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.two_to_one_chan_copy.main;    // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.two_to_one_chan_copy.main
//
// DESCRIPTION:
//    Operator that copies the input sample and writes it to two output buffers
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
//    r0, r1, r2, r10, I1, L1, M1
//
// *****************************************************************************
.MODULE $M_cbops.two_to_one_chan_copy.main;
   .CODESEGMENT CBOPS_TWO_TO_ONE_CHAN_COPY_MAIN_PM;
   .DATASEGMENT DM;

   // ** main routine **
$cbops.two_to_one_chan_copy.main:

   // push rLink onto stack
   push rLink;

#ifdef ENABLE_PROFILER_MACROS
   .VAR/DM1 $cbops.profile_two_to_one_chan_copy[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
   r0 = &$cbops.profile_two_to_one_chan_copy;
   call $profiler.start;
#endif

   // get the offset to the left read buffer to use
   r0 = M[r8 + $cbops.two_to_one_chan_copy.INPUT_A_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the right read buffer to use
   r0 = M[r8 + $cbops.two_to_one_chan_copy.INPUT_B_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // get the offset to the rightt write buffer to use
   r0 = M[r8 + $cbops.two_to_one_chan_copy.OUTPUT_B_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I1
   I1 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L1
   L1 = r1;

   M1 = 1;

   r2 = M[r8 + $cbops.two_to_one_chan_copy.INPUT_A_GAIN_FIELD];
   r3 = M[r8 + $cbops.two_to_one_chan_copy.INPUT_B_GAIN_FIELD];

   do read_loop;
      // Get the current left sample
      r0 = M[I0,M1];
      // Get the current right sample
      r1 = M[I4,M1];
      // scale the samples for mixing
      r0 = r0 * r2 (frac);
      r1 = r1 * r3 (frac);
      // Mix together for the output sample
      r0 = r0 + r1;
      // Write the mixed samples to the output
      M[I1,M1] = r0;

read_loop:

   // zero the length registers we have used
   L0 = 0;
   L4 = 0;
   L1 = 0;

#ifdef ENABLE_PROFILER_MACROS
   r0 = &$cbops.profile_two_to_one_chan_copy;
   call $profiler.stop;
#endif

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
