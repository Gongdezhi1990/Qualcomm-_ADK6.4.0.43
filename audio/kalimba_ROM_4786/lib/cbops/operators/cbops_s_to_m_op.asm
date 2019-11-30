// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Stereo to mono sample convertor operator
//
// DESCRIPTION:
//    This operator performs a stereo to mono conversion. It achieves this by
//    performing a copy and mix of samples from the left and right input
//    buffers to the output buffer.
//
// When using the operator the following data structure is used:
//    - $cbops.s_to_m_op.INPUT_LEFT_INDEX_FIELD = the index of the left input buffer
//    - $cbops.s_to_m_op.INPUT_RIGHT_INDEX_FIELD = the index of the right input buffer
//    - $cbops.s_to_m_op.OUTPUT_MONO_INDEX_FIELD = the index of the output buffer
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.s_to_m_op;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.s_to_m_op[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.s_to_m_op.main;               // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.s_to_m_op.main
//
// DESCRIPTION:
//    Operator that performs a copy and mix of samples from the left and right
//    input buffers to the output buffer.
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
//    r0-3, r10, I0, I1, I4, L1, L0, L4, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.s_to_m_op.main;
   .CODESEGMENT CBOPS_S_TO_M_OP_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.s_to_m_op.main:

   // Start profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_s_to_m_op[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_s_to_m_op;
      call $profiler.start;
   #endif

   // Set up the I/O
   r0 = M[r8 + $cbops.s_to_m_op.INPUT_LEFT_INDEX_FIELD];    // Get the index to the left buffer addr.
   r1 = M[r6 + r0];                                         // Get the left input buffer read address
   I0 = r1;                                                 // Store for indexed access
   r1 = M[r7 + r0];                                         // Get the input buffer length
   L0 = r1;                                                 // Store for circular access

   r0 = M[r8 + $cbops.s_to_m_op.INPUT_RIGHT_INDEX_FIELD];   // Get the index to the right buffer addr.
   r1 = M[r6 + r0];                                         // Get the right input buffer read address
   I4 = r1;                                                 // Store for indexed access
   r1 = M[r7 + r0];                                         // Get the input buffer length
   L4 = r1;                                                 // Store for circular access

   r0 = M[r8 + $cbops.s_to_m_op.OUTPUT_MONO_INDEX_FIELD];   // Get the index to the (mono) output buffer
   r1 = M[r6 + r0];                                         // Get the output buffer write address
   I1 = r1;                                                 // Store for indexed access
   r1 = M[r7 + r0];                                         // Get the input buffer length
   L1 = r1;                                                 // Store for circular access

   r3 = -1;                                                 // Shift value for 1/2

   // Pipeline the: read -> shift -> add -> write
   r0 = M[I0, 1],                                           // Prime the pipeline (get left i/p sample)
    r1 = M[I4, 1];                                          // Prime the pipeline (get right i/p sample)

   r0 = r0 ASHIFT r3;                                       // Scale left i/p by 1/2
   r2 = r1 ASHIFT r3;                                       // Scale right i/p by 1/2

   // Loop over all input samples (r10 = Number of samples to mix)
   do loop;
      r2 = r2 + r0,                                         // Mix the left and right inputs
       r0 = M[I0, 1],                                       // Get next left i/p
       r1 = M[I4, 1];                                       // Get next right i/p

      r0 = r0 ASHIFT r3,                                    // Scale left i/p by 1/2
       M[I1, 1] = r2;                                       // Write the mono output sample

      r2 = r1 ASHIFT r3;                                    // Scale right i/p by 1/2
   loop:

   // Zero the used length registers
   L0 = 0;
   L1 = 0;
   L4 = 0;

   // Stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_s_to_m_op;
      call $profiler.stop;
      pop rLink;
   #endif

   rts;

.ENDMODULE;
