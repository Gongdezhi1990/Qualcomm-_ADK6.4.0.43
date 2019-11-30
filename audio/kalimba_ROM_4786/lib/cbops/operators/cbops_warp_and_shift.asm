// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Warp and shift operator
//
// DESCRIPTION:
//    Operator that provides warping (adjusting of the input to the output
//    sample rate).  It also applies a shift operation to each output sample.
//
// @verbatim
// -------------------------------------------------------------------------
// WARP - Explanation of operation (shown with R=4)
// -------------------------------------------------------------------------
//
//     |       x         x        x        |       x         x        x        |
//  original                            original                            original
//  sample n                            sample n+1                          sample n+2
//              inserted upsamples                  inserted upsamples
//
//
//
// Interpolation explained (shown with Fs_out > Fs_in):
//
//     |       x         x        x        |       x         x        x        |        x
//           X                                X                                 X
//     output sample n                  output sample n+1              output sample n+1
//
//
//  upsampled n              upsampled n+1
//     |                       x
//     A                       B
//          X output sample
//     <---> <---------------->
//       c         1-c
//
//  x_out = (1-c) * A   + c * B
//
//   warp amount = 0 means no warp. ie. Fs_out = Fs_in
//              .  1.0 means Fs_out is 1/R*100% faster
//                -1.0 means Fs_out is 1/R*100% slower
// @endverbatim
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.warp_and_shift;
   .DATASEGMENT DM;


   // ** function vector **
   .VAR $cbops.warp_and_shift[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.warp_and_shift.reset,         // reset vector
      &$cbops.warp_and_shift.amount_to_use, // amount to use function
      &$cbops.warp_and_shift.main;          // main function

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $cbops.warp_and_shift.reset
//
// DESCRIPTION:
//    Reset function for the warp and shift operator.  The filter history is
//    reset and the internal processing states set to zero.
//
// INPUTS:
//    - r8 = pointer to operator structure:
//       - $cbops.warp_and_shift.INPUT_START_INDEX_FIELD = the index of the
//          input buffer
//       - $cbops.warp_and_shift.OUTPUT_START_INDEX_FIELD = the index of the
//          output buffer
//       - $cbops.warp_and_shift.SHIFT_AMOUNT_FIELD = amount to shift input
//          value by (eg. 8, -8)
//       - $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD = address of filter
//          coefficients to use
//       - $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD = address of some
//          memory to use for sorting the filter taps
//       - $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD = address of a memory
//          location that stores the target warp
//       - $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD = A value of 400 equates
//          to approx 1% sample rate adjustment over 1.5secs (using a warp
//          filter with R=4)
//       - $cbops.warp_and_shift.CURRENT_WARP_FIELD = leave initialised to
//          zero
//       - $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD = leave initialised
//          to zero
//       - $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD = leave
//          initialised to zero
//       - $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD = leave initialised to
//          zero
//       - $cbops.warp_and_shift.PREVIOUS_STATE_FIELD = leave initialised to
//          zero
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, I0
//
// *****************************************************************************
.MODULE $M.cbops.warp_and_shift.reset;
   .CODESEGMENT CBOPS_WARP_AND_SHIFT_RESET_PM;
   .DATASEGMENT DM;

   // ** reset function **
   $cbops.warp_and_shift.reset:

   // zero the filter delay line
   r0 = M[r8 + $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD];
   I0 = r0;
#ifdef BASE_REGISTER_MODE
   // get the start address
   r0 = M[r8 + $cbops.warp_and_shift.DATA_TAPS_START_FIELD];
   push r0;
   pop B0;
#endif
   r0 = M[r8 + $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD];
   r10 = M[r0 + $cbops.warp_and_shift.filt_coefs.L_FIELD];
   r10 = r10 + r10;
   L0 = r10;
   r0 = 0;
   do zero_data_taps_loop;
      M[I0, 1] = r0;
   zero_data_taps_loop:

   L0 = 0;
   // zero state information
   M[r8 + $cbops.warp_and_shift.CURRENT_WARP_FIELD] = Null;
   M[r8 + $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD] = Null;
   M[r8 + $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD] = Null;
   M[r8 + $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD] = Null;
   M[r8 + $cbops.warp_and_shift.PREVIOUS_STATE_FIELD] = Null;
   rts;

.ENDMODULE;




// *****************************************************************************
// MODULE:
//    $cbops.warp_and_shift.amount_to_use
//
// DESCRIPTION:
//    Amount to use function for the warp and shift operator.  The number of
//    input samples to process is calculated based on the amount of space in
//    the output buffer and the current warp factor that is being applied.
//
// INPUTS:
//    - r5 = the minimum of the number of input samples available and the
//      amount of output space available
//    - r6 = the number of input samples available
//    - r7 = the amount of output space available
//    - r8 = pointer to operator structure:
//       - $cbops.warp_and_shift.INPUT_START_INDEX_FIELD = the index of the
//          input buffer
//       - $cbops.warp_and_shift.OUTPUT_START_INDEX_FIELD = the index of the
//          output buffer
//       - $cbops.warp_and_shift.SHIFT_AMOUNT_FIELD = amount to shift input
//          value by (eg. 8, -8)
//       - $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD = address of filter
//          coefficients to use
//       - $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD = address of some
//          memory to use for sorting the filter taps
//       - $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD = address of a memory
//          location that stores the target warp
//       - $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD = A value of 400 equates
//          to approx 1% sample rate adjustment over 1.5secs (using a warp
//          filter with R=4)
//       - $cbops.warp_and_shift.CURRENT_WARP_FIELD = leave initialised to
//          zero
//       - $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD = leave initialised
//          to zero
//       - $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD = leave
//          initialised to zero
//       - $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD = leave initialised to
//          zero
//       - $cbops.warp_and_shift.PREVIOUS_STATE_FIELD = leave initialised to
//          zero
//
// OUTPUTS:
//    - r5 = number of input samples to use
//
// TRASHED REGISTERS:
//    r0, r4, r5
//
// *****************************************************************************
.MODULE $M.cbops.warp_and_shift.amount_to_use;
   .CODESEGMENT CBOPS_WARP_AND_SHIFT_AMOUNT_TO_USE_PM;
   .DATASEGMENT DM;

   // ** amount to use function **
   $cbops.warp_and_shift.amount_to_use:
   //   r6 = amount_input_data
   //   r7 = amount_output_space
   // calculate the required amount of input data we need given the amount of output space we have
   r0 = M[r8 + $cbops.warp_and_shift.CURRENT_WARP_FIELD];
   r4 = r0 * r7 (frac);
   // multiply by 1/R
   r0 = M[r8 + $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD];
   r0 = M[r0 + $cbops.warp_and_shift.filt_coefs.INV_R_FIELD];
   r4 = r4 * r0 (frac);
   r4 = r7 - r4;
   // because of rounding errors to be safe we knock off 2
   r4 = r4 - 2;
   if NEG r4 = 0;
   // if the amount of data avaiable > calculate amount -> set amount available to calculate amount
   Null = r4 - r5;
   if NEG r5 = r4;
   rts;

.ENDMODULE;






// *****************************************************************************
// MODULE:
//    $cbops.warp_and_shift.main
//
// DESCRIPTION:
//    Main processing function for the warp and shift operator.  The input
//    samples are resampled at a new rate determined by the current warp factor.
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure:
//       - $cbops.warp_and_shift.INPUT_START_INDEX_FIELD = the index of the
//          input buffer
//       - $cbops.warp_and_shift.OUTPUT_START_INDEX_FIELD = the index of the
//          output buffer
//       - $cbops.warp_and_shift.SHIFT_AMOUNT_FIELD = amount to shift input
//          value by (eg. 8, -8)
//       - $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD = address of filter
//          coefficients to use
//       - $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD = address of some
//          memory to use for sorting the filter taps
//       - $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD = address of a memory
//          location that stores the target warp (1.0 equates to 100% warp)
//          100% being twice the original sample rate!
//       - $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD = Sets the maximum rate
//          at which the warp is changed.  A value of 1.0 equates to 100% warp
//          every timer period - that a lot!.  So a value of 100/2^23 being
//          used, where the audio copy timer period is 1.5ms, equates to a 0.8%
//          change in warp every second.
//       - $cbops.warp_and_shift.CURRENT_WARP_FIELD = leave initialised to
//          zero
//       - $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD = leave initialised
//          to zero
//       - $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD = leave
//          initialised to zero
//       - $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD = leave initialised to
//          zero
//       - $cbops.warp_and_shift.PREVIOUS_STATE_FIELD = leave initialised to
//          zero
//    - r10 = the number of samples to process
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0 - r7, I0 - I7, L0, L1, L4, L5, M0 - M3
//
// *****************************************************************************
.MODULE $M.cbops.warp_and_shift.main;
   .CODESEGMENT CBOPS_WARP_AND_SHIFT_MAIN_PM;
   .DATASEGMENT DM;

   // ** main routine **
   $cbops.warp_and_shift.main:
   // push rLink onto stack
   push rLink;
   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_warp_and_shift[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_warp_and_shift;
      call $profiler.start;
   #endif


   // get the offset to the read buffer to use
   r2 = M[r8 + $cbops.warp_and_shift.INPUT_START_INDEX_FIELD];

   // get the input buffer read address
   r3 = M[r6 + r2];
   // store the value in I0
   I0 = r3;
   // get the input buffer length
   r4 = M[r7 + r2];
   // Store the value in L0
   L0 = r4;
#ifdef BASE_REGISTER_MODE
   // get the start address
   r4 = M[r5 + r2];
   push r4;
   pop B0;
#endif

   // get the offset to the write buffer to use
   r2 = M[r8 + $cbops.warp_and_shift.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r3 = M[r6 + r2];
   // store the value in I4
   I4 = r3;
   // get the output buffer length
   r4 = M[r7 + r2];
   // store the value in L4
   L4 = r4;
#ifdef BASE_REGISTER_MODE
   // get the start address
   r4 = M[r5 + r2];
   push r4;
   pop B4;
#endif

   // r7 = shift amount
   r7 = M[r8 + $cbops.warp_and_shift.SHIFT_AMOUNT_FIELD];

   // r6 = warp amount
   r6 = M[r8 + $cbops.warp_and_shift.CURRENT_WARP_FIELD];

   // r5 = current warp coef
   r5 = M[r8 + $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD];

   // I1 = filters data taps buffer
   r0 = M[r8 + $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD];
   I1 = r0;
#ifdef BASE_REGISTER_MODE
   // get the start address
   r0 = M[r8 + $cbops.warp_and_shift.DATA_TAPS_START_FIELD];
   push r0;
   pop B1;
#endif

   // I5 = pointer to current filter coef
   // I6 = pointer to start of filter coefs
   r0 = M[r8 + $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD];
   I6 = r0 + $cbops.warp_and_shift.filt_coefs.COEFS_FIELD;
   r2 = M[r8 + $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD];
   I5 = I6 + r2;
   // read R
   r1 = M[r0 + $cbops.warp_and_shift.filt_coefs.R_FIELD];
   // read L
   r0 = M[r0 + $cbops.warp_and_shift.filt_coefs.L_FIELD];

   // set up some registers that are needed as follows:
   // L1 = 2*L
   // M0 = (1-2*L)*R - 1
   // M1 = 1
   // M2 = R
   // M3 = 1 - 2*L
   // r3 = R - 2
   // r4 = 2*L - 3
   L1 = r0 + r0;
   r2 = r0 * r1 (int);
   r2 = r2 * 2 (int);
   r2 = r1 - r2;
   M1 = 1;
   M0 = r2 - M1;
   M2 = r1;
   M3 = M1 - L1;
   r3 = r1 - 2;
   r4 = L1 - 3;


   // I2 counts down the number of input samples used
   I2 = r10;
   // I3 counts the number of output samples produced
   I3 = 0;

   r10 = 0;

   // restore last value of Sample A (r2)
   r2 = M[r8 + $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD];

   // return to the position in the loop that we finished at during the last call
   r0 = M[r8 + $cbops.warp_and_shift.PREVIOUS_STATE_FIELD];
   if NZ jump r0;

   sample_loop:

      // Generate upsampled sample A
      r10 = r10 + r4, r0 = M[I1,M1], r1 = M[I5,M2];
      rMAC = r0 * r1, r0 = M[I1,M1], r1 = M[I5,M2];
      do loop1;
         rMAC = rMAC + r0 * r1, r0 = M[I1,M1], r1 = M[I5,M2];
      loop1:
      // Move I1, in the filter history, back in preparation for the next sample
      // Move I5, in the coefficient buffer, back in preparation for the next sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I5,M0];
      rMAC = rMAC + r0 * r1;
      r2 = rMAC;

      // update pointers if we need a new input sample for B
      // test if coef pointer (I5) has wrapped (note I7 is a dummy 16bit destination)
      I7 = I5 - I6;
      if POS jump no_offset_update1;
         // read new input sample
         // and dummy read to shift coef index reg
         r0 = M[I0,M1], r1 = M[I5,M2];
         // decrement I2
         // and write new input sample to delay line
         I2 = I2 - M1, M[I1,M1] = r0;
         if Z call done;
      no_offset_update1:


      // Generate upsampled sample B
      r10 = r10 + r4, r0 = M[I1,M1], r1 = M[I5,M2];
      rMAC = r0 * r1, r0 = M[I1,M1], r1 = M[I5,M2];
      do loop2;
         rMAC = rMAC + r0 * r1, r0 = M[I1,M1], r1 = M[I5,M2];
      loop2:
      rMAC = rMAC + r0 * r1, r0 = M[I1,M3], r1 = M[I5,M0];
      rMAC = rMAC + r0 * r1;

      // Apply: x_out = (1-c) * A   + c * B
      rMAC = rMAC - r5 * rMAC;
      // dummy read to increment I3 (num output samples count)
      rMAC = rMAC + r2 * r5, r0 = M[I3,1];

      // apply shift with round and saturate beforehand
      r0 = rMAC;
      r0 = r0 ASHIFT r7;

      // adjust pointers if the interpolation coef 'r5' wraps
      // +ve increment means Fs_out > Fs_in
      // and store result
      r5 = r5 + r6, M[I4,1] = r0;
      if POS jump no_wrap;
         if V jump wrap_pos;
            // neg wrap I5 = I5 - 1;
            I5 = I5 - 2;
         wrap_pos:
         // pos wrap I5 = I5 + 1;
         I5 = I5 + 1;
         // remove sign bit
         r5 = r5 LSHIFT 1;
         r5 = r5 LSHIFT -1;
      no_wrap:


      // update pointers for the next output sample
      I5 = I5 - r3;
      // test if coef pointer (I5) has wrapped (note I7 is a dummy 16bit destination)
      I7 = I5 - I6;
      if POS jump sample_loop;
         // read new input sample
         // and do dummy read to shift coef index reg
         r0 = M[I0,M1], r1 = M[I5,M2];
         // decrement I2
         // and write new input sample to delay line
         I2 = I2 - M1, M[I1,M1] = r0;
         if Z call done;
      no_offset_update2:

   jump sample_loop;

   done:
   // work out where we finished and save the state for next time
   M[r8 + $cbops.warp_and_shift.PREVIOUS_STATE_FIELD] = rLink;

   // store updated "current warp coef" for next time
   M[r8 + $cbops.warp_and_shift.CURRENT_WARP_COEF_FIELD] = r5;

   // store updated "current filter coef addr" for next time
   r0 = I5 - I6;
   M[r8 + $cbops.warp_and_shift.CURRENT_FILT_COEF_ADDR_FIELD] = r0;

   // store updated value of sample A (r2) for next time
   M[r8 + $cbops.warp_and_shift.CURRENT_SAMPLE_A_FIELD] = r2;

   // store updated "data taps addr" pointer
   r0 = I1;
   M[r8 + $cbops.warp_and_shift.DATA_TAPS_ADDR_FIELD] = r0;

   // set number of output samples produced
   r0 = I3;
#ifndef TODO_CBOPS_GLOBALS
    // do we really need this dependency here? (if yes, easy to sort out if we had the head pointer)
   M[$cbops.amount_written] = r0;
#endif

   // adjust warp amount for next time
   r0 = M[r8 + $cbops.warp_and_shift.WARP_TARGET_ADDR_FIELD];
   r2 = M[r8 + $cbops.warp_and_shift.WARP_MAX_RAMP_FIELD];
   r0 = M[r0];

   // scale WARP_TARGET and WARP_MAX_RAMP by the R (upsample rate) of
   // the filter used. This allows the stored values to be independent of R.
   // read R for filter used
   r1 = M[r8 + $cbops.warp_and_shift.FILT_COEFS_ADDR_FIELD];
   r1 = M[r1 + $cbops.warp_and_shift.filt_coefs.R_FIELD];
   // adjust WARP_TARGET and WARP_MAX_RAMP
   r0 = r0 * r1 (int);
   r2 = r2 * r1 (int);

   // we slowly tend towards the target warp to reduce the chance of hearing
   // a sudden change in sampling rate
   r0 = r0 - r6;
   r0 = r0 ASHIFT -7;

   // limit the maximum step change of the warp
   if NEG jump neg_step;
      r1 = r0 - r2;
      if POS r0 = r0 - r1;
      jump limit_done;
   neg_step:
      r1 = r0 + r2;
      if NEG r0 = r0 - r1;
   limit_done:
   r6 = r6 + r0;
   M[r8 + $cbops.warp_and_shift.CURRENT_WARP_FIELD] = r6;

   // zero the length registers we've set
   L0 = 0;
   L1 = 0;
   L4 = 0;
   L5 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_warp_and_shift;
      call $profiler.stop;
   #endif
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// ##################### VERY HIGH QUALITY FILTER #########################

   // replaced the old VHQ filter (now L=20, R=6)
   .CONST   $cbops.warp_and_shift.very_high_quality_filter_L          20;
   .CONST   $cbops.warp_and_shift.very_high_quality_filter_R          6;
   .CONST   $cbops.warp_and_shift.very_high_quality_filter_data_size  (2 * $cbops.warp_and_shift.very_high_quality_filter_L);
   // this is for a filter with L=20, i.e 2*L = 40
   // the buffer should be located in DM1 and be circular

.MODULE $M.cbops.warp_and_shift.very_high_quality_filter;
   .BLOCK/DM2 very_high_quality_filter_table;
     /*
         Improved filter - doesn't show amplitude wobbling at 20K. It has smaller footprint than some other possible candidates.
         It kept upsampling ratio reasonable (6), but steeper longer filter (L=20) that plays key role on top of resampling ratio.
         Another decent candidate in measurements was e.g. resampler with L=18, R=10.
         It can be generated via following (first zero is always removed):
         [y,coef]=resample(zeros(1,L*2+1),R,1,L,7); fprintf('%11.8f,\n',coef(2:end))
     */
        .VAR L = $cbops.warp_and_shift.very_high_quality_filter_L;
        .VAR R = $cbops.warp_and_shift.very_high_quality_filter_R;
        .VAR INV_R = (1.0/$cbops.warp_and_shift.very_high_quality_filter_R);
        .VAR coefs[2*$cbops.warp_and_shift.very_high_quality_filter_L*$cbops.warp_and_shift.very_high_quality_filter_R] =
        -0.00005778,      -0.00012037,      -0.00016486,       -0.00016740,
        -0.00011224,       0.00000000,       0.00014786,        0.00029107,
         0.00037990,       0.00037006,       0.00023925,       -0.00000000,
        -0.00029653,      -0.00056884,      -0.00072521,       -0.00069145,
        -0.00043834,      -0.00000000,       0.00052465,        0.00099085,
         0.00124499,       0.00117096,       0.00073289,       -0.00000000,
        -0.00085689,      -0.00160101,      -0.00199124,       -0.00185481,
        -0.00115026,       0.00000000,       0.00132204,        0.00245043,
         0.00302450,       0.00279673,       0.00172226,       -0.00000000,
        -0.00195350,      -0.00359841,      -0.00441494,       -0.00405902,
        -0.00248577,      -0.00000000,       0.00279009,        0.00511398,
         0.00624443,       0.00571454,       0.00348405,       -0.00000000,
        -0.00387764,      -0.00707892,      -0.00861033,       -0.00785033,
        -0.00476900,       0.00000000,       0.00527171,        0.00959298,
         0.01163221,       0.01057397,       0.00640526,       -0.00000000,
        -0.00704261,      -0.01278344,      -0.01546389,       -0.01402515,
        -0.00847755,       0.00000000,       0.00928427,        0.01682209,
         0.02031525,       0.01839653,       0.01110398,       -0.00000000,
        -0.01213069,      -0.02195692,      -0.02649282,       -0.02397288,
        -0.01446130,       0.00000000,       0.01578751,        0.02857331,
         0.03447927,       0.03120863,       0.01883541,       -0.00000000,
        -0.02059666,      -0.03732145,      -0.04510095,       -0.04089362,
        -0.02473103,       0.00000000,       0.02718196,        0.04940830,
         0.05991990,       0.05454938,       0.03313993,       -0.00000000,
        -0.03682304,      -0.06736811,      -0.08229838,       -0.07553904,
        -0.04631742,       0.00000000,       0.05262076,        0.09756610,
         0.12101776,       0.11303041,       0.07070866,       -0.00000000,
        -0.08447623,      -0.16171367,      -0.20837000,       -0.20379008,
        -0.13492190,       0.00000000,       0.18991444,        0.41201067,
         0.63533199,       0.82624948,       0.95471486,        1.00000000,
         0.95471486,       0.82624948,       0.63533199,        0.41201067,
         0.18991444,       0.00000000,      -0.13492190,       -0.20379008,
        -0.20837000,      -0.16171367,      -0.08447623,       -0.00000000,
         0.07070866,       0.11303041,       0.12101776,        0.09756610,
         0.05262076,       0.00000000,      -0.04631742,       -0.07553904,
        -0.08229838,      -0.06736811,      -0.03682304,       -0.00000000,
         0.03313993,       0.05454938,       0.05991990,        0.04940830,
         0.02718196,       0.00000000,      -0.02473103,       -0.04089362,
        -0.04510095,      -0.03732145,      -0.02059666,       -0.00000000,
         0.01883541,       0.03120863,       0.03447927,        0.02857331,
         0.01578751,       0.00000000,      -0.01446130,       -0.02397288,
        -0.02649282,      -0.02195692,      -0.01213069,       -0.00000000,
         0.01110398,       0.01839653,       0.02031525,        0.01682209,
         0.00928427,       0.00000000,      -0.00847755,       -0.01402515,
        -0.01546389,      -0.01278344,      -0.00704261,       -0.00000000,
         0.00640526,       0.01057397,       0.01163221,        0.00959298,
         0.00527171,       0.00000000,      -0.00476900,       -0.00785033,
        -0.00861033,      -0.00707892,      -0.00387764,       -0.00000000,
         0.00348405,       0.00571454,       0.00624443,        0.00511398,
         0.00279009,      -0.00000000,      -0.00248577,       -0.00405902,
        -0.00441494,      -0.00359841,      -0.00195350,       -0.00000000,
         0.00172226,       0.00279673,       0.00302450,        0.00245043,
         0.00132204,       0.00000000,      -0.00115026,       -0.00185481,
        -0.00199124,      -0.00160101,      -0.00085689,       -0.00000000,
         0.00073289,       0.00117096,       0.00124499,        0.00099085,
         0.00052465,      -0.00000000,      -0.00043834,       -0.00069145,
        -0.00072521,      -0.00056884,      -0.00029653,       -0.00000000,
         0.00023925,       0.00037006,       0.00037990,        0.00029107,
         0.00014786,       0.00000000,      -0.00011224,       -0.00016740,
        -0.00016486,      -0.00012037,      -0.00005778,       -0.00000000;
   .ENDBLOCK;
.ENDMODULE;

// ##################### HIGH QUALITY FILTER #############################
   .CONST   $cbops.warp_and_shift.high_quality_filter_L               4;
   .CONST   $cbops.warp_and_shift.high_quality_filter_R               4;
   .CONST   $cbops.warp_and_shift.high_quality_filter_data_size       (2 * $cbops.warp_and_shift.high_quality_filter_L);
   // this is for a filter with L=4, i.e 2*L = 8
   // the buffer should be located in DM1 and be circular

.MODULE $M.cbops.warp_and_shift.high_quality_filter;
   .BLOCK/DM2 high_quality_filter_table;
    /* IMPROVED COEFS2 - previous replacement for VHQ filter, pushes amplitude instability issue up by ~2.5kHz, so it's manifesting above 19K */
        .VAR L = $cbops.warp_and_shift.high_quality_filter_L;
        .VAR R = $cbops.warp_and_shift.high_quality_filter_R;
        .VAR INV_R = (1.0/$cbops.warp_and_shift.high_quality_filter_R);
        .VAR coefs[2*$cbops.warp_and_shift.high_quality_filter_L*$cbops.warp_and_shift.high_quality_filter_R]=
        -0.04220875,      -0.06272111,      -0.04708433,       -0.00000000,
         0.06926949,       0.10629172,       0.08281764,       -0.00000000,
        -0.12184116,      -0.19922765,      -0.16845210,        0.00000000,
         0.29946572,       0.63223242,       0.89474065,        1.00000000,
         0.89474065,       0.63223242,       0.29946572,        0.00000000,
        -0.16845210,      -0.19922765,      -0.12184116,       -0.00000000,
         0.08281764,       0.10629172,       0.06926949,       -0.00000000,
        -0.04708433,      -0.06272111,      -0.04220875,        0.00000000;
   .ENDBLOCK;
.ENDMODULE;


// ##################### MEDIUM QUALITY FILTER ###########################
   .CONST   $cbops.warp_and_shift.medium_quality_filter_L             2;
   .CONST   $cbops.warp_and_shift.medium_quality_filter_R             3;
   .CONST   $cbops.warp_and_shift.medium_quality_filter_data_size     (2 * $cbops.warp_and_shift.medium_quality_filter_L);
   // this is for a filter with L=2, i.e 2*L = 4
   // the buffer should be located in DM1 and be circular

.MODULE $M.cbops.warp_and_shift.medium_quality_filter;
   .BLOCK/DM2 medium_quality_filter_table;
       .VAR L = $cbops.warp_and_shift.medium_quality_filter_L;
       .VAR R = $cbops.warp_and_shift.medium_quality_filter_R;
       .VAR INV_R = (1.0/$cbops.warp_and_shift.medium_quality_filter_R);
       // Filter coefs (first zero is always removed)
       // Generated with the matlab line:
       // R=3; L=2; [y,coef]=interp(zeros(1,L*2+1),R,L); fprintf('%11.8f,\n',coef(2:end))
      .VAR coefs[2*$cbops.warp_and_shift.medium_quality_filter_L*$cbops.warp_and_shift.medium_quality_filter_R]=
      -0.07367647,      -0.09045671,       0.00000000,       0.38557849,
       0.76704929,       1.00000000,       0.76704929,       0.38557849,
       0.00000000,      -0.09045671,      -0.07367647,       0.00000000;
   .ENDBLOCK;
.ENDMODULE;
