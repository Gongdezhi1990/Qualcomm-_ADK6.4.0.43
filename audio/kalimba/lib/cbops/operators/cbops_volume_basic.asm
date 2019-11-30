// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Basic volume operator
//
// DESCRIPTION:
// This operator applies small decreasing changes to the gain on each sample
// until the desired volume level is reached.  This provides a basic method for
// smoothly changing the level of the samples.
//
// When using the operator the following data structure is used:
//    - $cbops.volume_basic.INPUT_START_INDEX_FIELD = input buffer index
//    - $cbops.volume_basic.OUTPUT_START_INDEX_FIELD = output buffer index
//    - $cbops.volume_basic.FINAL_VALUE_FIELD = the final desired volume
//       level
//    - $cbops.volume_basic.CURRENT_VALUE_FIELD = the current volume level,
//       should be initialised to 0 for a gentle fade at startup
//    - $cbops.volume_basic.SMOOTHING_VALUE_FIELD = the smoothing value to
//       be applied to the gain change, a fractional value between 0.0 and
//       1.0.  The smaller the value the longer the gain change takes
//    - $cbops.volume_basic.DELTA_THRESHOLD_VALUE_FIELD = threshold value
//       used to compare the difference between the current gain and the
//       desired gain.  Below this threshold no gain changes are applied
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.volume_basic;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.volume_basic[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.volume_basic.reset,            // reset function
      $cbops.function_vector.NO_FUNCTION,    // amount to use function
      &$cbops.volume_basic.main;             // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.volume_basic.reset
//
// DESCRIPTION:
//    Reset routine for the basic volume operator, see $cbops.volume_basic.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0
//
// *****************************************************************************
.MODULE $M.cbops.volume_basic.reset;
   .CODESEGMENT CBOPS_VOLUME_BASIC_RESET_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.volume_basic.reset:
   // set the current volume to the final
   r0 = M[r8 + $cbops.volume_basic.FINAL_VALUE_FIELD];
   M[r8 + $cbops.volume_basic.CURRENT_VALUE_FIELD] = r0;

   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.volume_basic.main
//
// DESCRIPTION:
//    Operator that applies a volume to a stream and allows basic ramping
//    between volumes.
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
//    r0, r1, r2, r3, r4, rMAC, r10, I0, I4, L0, L4, L1, DoLoop
//
// *****************************************************************************
.MODULE $M.cbops.volume_basic.main;
   .CODESEGMENT CBOPS_VOLUME_BASIC_MAIN_PM;
   .DATASEGMENT DM;

   // ** main routine **
   $cbops.volume_basic.main:
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_volume_basic[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_volume_basic;
      call $profiler.start;
   #endif

   // Get the offset to the read buffer to use
   r0 = M[r8 + $cbops.volume_basic.INPUT_START_INDEX_FIELD];

   // Get the input buffer read address
   r1 = M[r6 + r0];
   // Store the value in I0
   I0 = r1;
   // Get the input buffer length
   r1 = M[r7 + r0];
   // Store the value in L0
   L0 = r1;

   // Get the offset to the write buffer to use
   r0 = M[r8 + $cbops.volume_basic.OUTPUT_START_INDEX_FIELD];

   // Get the output buffer write address
   r1 = M[r6 + r0];
   // Store the value in I4 and I1
   I4 = r1;
   I1 = r1;
   // Get the output buffer length
   r1 = M[r7 + r0];
   // Store the value in L4
   L4 = r1;
   L1 = r1;

   // Load the desired final volume level
   r2 = M[r8 + $cbops.volume_basic.FINAL_VALUE_FIELD];
   // Load the current volume value
   rMAC = M[r8 + $cbops.volume_basic.CURRENT_VALUE_FIELD];
   // Load the smoothing coefficient
   r3 = M[r8 + $cbops.volume_basic.SMOOTHING_VALUE_FIELD];
   // Load the delta threshold value
   r1 = M[r8 + $cbops.volume_basic.DELTA_THRESHOLD_VALUE_FIELD];

   // If the difference between the current volume and the desired volume is
   // below the delta threshold the set the current volume to be the desired
   // volume and stop ramping
   r4 = r2 - rMAC;
   NULL = r4 - r1;
   if NEG jump no_ramping;
   // initialize modifier register
   M1 = 1;
   do loop_vol_change;
      // Apply smoothing to the volume difference and add to the current volume
      //                      Read the current input sample
      rMAC = rMAC + r4 * r3,  r0 = M[I0,1];
      // Apply the gain (current volume) to the input sample
      r0 = r0 * rMAC (frac);
      // Calculate the difference between the current volume and desired final
      // volume
      //                      Write the new sample value to the output buffer
      r4 = r2 - rMAC,         M[I1,M1] = r0;

   loop_vol_change:

   // The ramping volume has been applied, jump to the exit part of the routine
   jump done;

   no_ramping:
   // Set the current gain to the desired gain
   rMAC = r2;
   // Decrement the loop counter and do one sample outside the loop
   //                Read the first input sample
   r10 = r10 - M1,   r0 = M[I0,1];
   // Apply the gain to the first sample
   r0 = r0 * rMAC (frac);
   do loop_no_vol_change;
      // Read the next sample and Write the current sample
      r0 = M[I0,1] , M[I4,1] = r0;
      // Apply the gain to the current sample
      r0 = r0 * rMAC (frac);
   loop_no_vol_change:
   // Write the last output sample
   M[I4,1] = r0;


   done:
   // Clear the length register
   L0 = 0;
   L4 = 0;
   L1 = 0;

   // Write back the new volume
   M[r8 + $cbops.volume_basic.CURRENT_VALUE_FIELD] = rMAC;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_volume_basic;
      call $profiler.stop;
      jump $pop_rLink_and_rts;
   #endif
   rts;

.ENDMODULE;

