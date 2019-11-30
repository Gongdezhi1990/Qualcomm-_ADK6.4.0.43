// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Volume operator
//
// DESCRIPTION:
//    This operator ramps the volume from one level to another over a number of
// samples.
//
//    We do this by changing the volume with a number of small logarithmic
// steps, which are mimicked by the approximation below. The step size is small
// so that it cannot be heard as a step.  The ramp rate is set by adjusting the
// number of samples to process before the volume is moved onto the next step.
//
// @verbatim
//      step_size = (current_vol >> step_shift_amount)
//                   + step_const_amount
// @endverbatim
//
//  where "step_shift_amount" and "step_const_amount" are defined in the
//  parameter area for the operator.
//
//  After SAMPLES_PER_STEP samples the volume is changed:
// @verbatim
//      new_volume = current_vol +/- step_size
// @endverbatim
//  until the requested final volume value is reached.
//
// When using the operator the following data structure is used:
//    - $cbops.volume.INPUT_START_INDEX_FIELD = index of the input buffer
//    - $cbops.volume.OUTPUT_START_INDEX_FIELD = index of the output buffer
//    - $cbops.volume.FINAL_VALUE_FIELD = requested volume, a fractional
//       value between 0.0 and 1.0
//    - $cbops.volume.CURRENT_VALUE_FIELD = the current volume level, should
//       be initialised to zero
//    - $cbops.volume.SAMPLES_PER_STEP_FIELD = the rate at which to ramp to
//       the desired volume
//    - $cbops.volume.STEP_SHIFT_FIELD = the step shift amount to be applied
//       when calculating the gain, should be a negative integer
//    - $cbops.volume.DELTA_FIELD = the delta amount to be applied to each
//       sample, a fractional value between 0.0 and 1.0
//    - $cbops.volume.CURRENT_STEP_FIELD = the current count of the number
//       of samples at the current volume, should be initialised to zero
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.volume;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.volume[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.volume.reset,                 // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.volume.main;                  // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.volume.reset
//
// DESCRIPTION:
//    Reset routine for the volume operator, see $cbops.volume.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0
//
// *****************************************************************************
.MODULE $M.cbops.volume.reset;
   .CODESEGMENT CBOPS_VOLUME_RESET_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.volume.reset:
   // reset the state information
   r0 = M[r8 + $cbops.volume.SAMPLES_PER_STEP_FIELD];
   M[r8 + $cbops.volume.CURRENT_STEP_FIELD] = r0;
   r0 = M[r8 + $cbops.volume.FINAL_VALUE_FIELD];
   M[r8 + $cbops.volume.CURRENT_VALUE_FIELD] = r0;
   rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.volume
//
// DESCRIPTION:
//    Operator that applies a volume to a stream and allows ramping between
//    volumes.
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
//    r0-6, r10, I0, L0, I4, L4, DoLoop
//
// *****************************************************************************

.MODULE $M.cbops.volume.main;
   .CODESEGMENT CBOPS_VOLUME_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.volume.main:
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_volume[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      push rLink;
      r0 = &$cbops.profile_volume;
      call $profiler.start;
   #endif


   // initialise M1 to 1
   M1 = 1;
   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.volume.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.volume.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // get the final volume value
   r2 = M[r8 + $cbops.volume.FINAL_VALUE_FIELD];

   // get the current volume value
   r1 = M[r8 + $cbops.volume.CURRENT_VALUE_FIELD];

   // see if the volume is changing (the current volume should be the same as the final volume)
   Null = r1 - r2;
   if Z jump volume_isnt_changing;

   // are we using volume ramping
   r3 = M[r8 + $cbops.volume.SAMPLES_PER_STEP_FIELD];
   // if no volume ramping then can change volume in 1 go
   if Z jump volume_no_ramping;
      // load the step shift value
      r5 = M[r8 + $cbops.volume.STEP_SHIFT_FIELD];
      // load the delta value
      rMAC = M[r8 + $cbops.volume.DELTA_FIELD];

      // load the current step value
      r4 = M[r8 + $cbops.volume.CURRENT_STEP_FIELD];

      do loop_vol_changing;
         // decrement the samples per step counter and get the current sample
         r4 = r4 - M1, r0 = M[I0,1];
         if POS jump no_volume_change;
            // calculate the new step amount
            r6 = r1 ASHIFT r5;
            r6 = r6 + rMAC;
            // reset "current_step" to "samples_per_step"
            r4 = r3;
            // are we increasing or decreasing
            Null = r1 - r2;
            if NEG jump increase_volume;
            decrease_volume:
               r1 = r1 - r6;
               Null = r1 - r2;
               // if gone past final value then set it to the final value
               if NEG r1 = r2;
               jump set_volume;
            increase_volume:
               r1 = r1 + r6;
               Null = r1 - r2;
               // if gone past final value then set it to the final value
               if POS r1 = r2;
            set_volume:
         no_volume_change:
         // scale the input sample
         r0 = r0 * r1 (frac);
         M[I4,1] = r0;
      loop_vol_changing:

      // write back the current step count value
      M[r8 + $cbops.volume.CURRENT_STEP_FIELD] = r4;
      jump done;

   volume_no_ramping:
      // change the volume to be the final volume
      r1 = r2;

   volume_isnt_changing:
   // the volume isn't changing so do a simple copy and scale
      r10 = r10 - M1, r0 = M[I0,1];
      r0 = r0 * r1 (frac);
      do loop_no_vol_change;
         r0 = M[I0,1], M[I4,1] = r0;
         r0 = r0 * r1 (frac);
      loop_no_vol_change:
      M[I4,1] = r0;

   done:

   // write back the current volume
   M[r8 + $cbops.volume.CURRENT_VALUE_FIELD] = r1;

   // zero the length regiserts we have used
   L0 = 0;
   L4 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_volume;
      call $profiler.stop;
      jump $pop_rLink_and_rts;
   #else
      rts;
   #endif

.ENDMODULE;

