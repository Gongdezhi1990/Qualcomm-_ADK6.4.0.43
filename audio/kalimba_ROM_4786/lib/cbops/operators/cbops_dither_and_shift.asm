// *****************************************************************************
// Copyright (c) 2007 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************


// *****************************************************************************
// NAME:
//  dither_and_shift operator
// DESCRIPTION:
//  This operator is applied to the audio when a truncation (right shift) is
//  performed on audio samples. Dithering is used to avoid harmonic effects on
//  very low level audio caused by quantization error. dither_and_shift operator
//  adds a small amount of dither (based on the shift amount) to the audio and
//  then shift the data.
//    Noise shaped dithering can increase the dynamic range (2-3 bits)
//  for low frequencies (<10khz). Care must be taken when using the noise
//  shaped dithering type. First, it must only used when dithered audio is
//  directly written to the DAC, otherwise any further operation on noise-shaped
//  dithered data could potentially add distortion. Secondly, it must only be used
//  for 44100Hz and 48000Hz music data.
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.dither_and_shift;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.dither_and_shift[$cbops.function_vector.STRUC_SIZE] =
      $cbops.function_vector.NO_FUNCTION,   // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.dither_and_shift.main;        // main function

.ENDMODULE;

// Expose the function table to C
.set $_cbops_dither_table,      $cbops.dither_and_shift

// *************************************************************************************************
// MODULE:
//    $cbops.dither_and_shift.main
//
// DESCRIPTION:
//
// INPUTS:
//    - r6 = pointer to the list of input and output buffer pointers
//    - r7 = pointer to the list of buffer lengths
//    - r8 = pointer to operator structure
//    - r10 = the number of samples to process
// When using the operator the following data structure is used:
//   - $cbops.dither_and_shift.INPUT_START_INDEX_FIELD = index of the input buffer
//   - $cbops.dither_and_shift.OUTPUT_START_INDEX_FIELD = index of the output buffer
//   - $cbops.dither_and_shift.SHIFT_AMOUNT_FIELD = amount of shift after dithering (must be negative)
//   - $cbops.dither_and_shift.DITHER_TYPE_FIELD =  type of dither , can be one of the following values
//         -$cbops.dither_and_shift.NO_DITHER    -> no dither, just shift
//         -$cbops.dither_and_shift.TPDF_DITHER  -> add triangular pdf dither before shifting
//         -$cbops.dither_and_shift.NOISESHAPED_DITHER ->add dither and shape quantize noise, then shift
//          this value must only be used when output buffer is DAC port, and fs=44100Hz or 48000Hz
//   - $cbops.dither_and_shift.DITHER_FILTER_HIST_FIELD = history buffer for dithering,
//     (size = $cbops.dither_and_shift.FILTER_COEFF_SIZE), define in DM1 for faster operation
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
// assume everything
// *****************************************************************************************************
.MODULE $M.cbops.dither_and_shift.main;
   .CODESEGMENT CBOPS_DITHER_AND_SHIFT_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.dither_and_shift.main:
   push rLink;
  #ifdef ENABLE_PROFILER_MACROS
      // start profiling if enabled
      .VAR/DM1 $cbops.profile_dither_and_shift[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_dither_and_shift;
      call $profiler.start;
  #endif

   .VAR rand_num = $math.RAND_SEED; // random value used for dithering
   .VAR dither_func[] = &no_dither, &tpdf_dither, noiseshaped_dither; // functions to run for different type of dithering

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.dither_and_shift.INPUT_START_INDEX_FIELD];
   Words2Addr(r0);    // input_index (arch4: in addrs)
   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the second read buffer to use
   r0 = M[r8 + $cbops.dither_and_shift.OUTPUT_START_INDEX_FIELD];
   Words2Addr(r0);    // output_index (arch4: in addrs)
   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   #if defined(cbops_dither_and_shift_INCLUDE_COMPRESSOR)
      // see if compressor is enabled
      Null = M[r8 + $cbops.dither_and_shift.ENABLE_COMPRESSOR_FIELD];
      if Z jump compress_end;
      // save a few register for future use
      r7 = r10;
      r6 = I0;
      // compressor loop (in place)
      M0 = -MK1;
      M1 = MK1;
      r10 = r10 - 1, r0 = M[I0, M1];
      // compress first sample outside the loop
      r2 = r0 * r0 (frac);
      r2 = r2 * r2 (frac);
      r2 = r2 * (-0.1087) (frac);
      r2 = r2 * r0 (frac);
      do compress_loop;
         r3 = r2 + r0, r0 = M[I0,M0];         // compressed = sample*(1-0.1087*sample^4)
         r2 = r0 * r0 (frac);                 // sampe^2
         r2 = r2 * r2 (frac), M[I0, MK2] = r3;   // sample^4
         r2 = r2 * (-0.1087) (frac);
         r2 = r2 * r0 (frac);
      compress_loop:
      r3 = r2 + r0, r0 = M[I0,M0];
      r10 = r7, M[I0,M0] = r3;
      I0 = r6;
      compress_end:
   #endif //#ifdef $cbops.dither_and_shift.INCLUDE_COMPRESSOR
   // decide which dithering function to call
   r2 = M[rand_num];
   r3 = M[r8 + $cbops.dither_and_shift.DITHER_TYPE_FIELD];
   Words2Addr(r3);   // enum dither type (arch4: in addrs)
   r5 = 1.0;
   r0 = M[r3 + dither_func];
   r7 = M[r8 + $cbops.dither_and_shift.SHIFT_AMOUNT_FIELD];
   // calc dither amplitude
   r6 = -DAWTH - r7;
   r6 = r5 ASHIFT r6;
   M1 = $math.RAND_INC;
   r4 = $math.RAND_MULT;
   jump r0;

   // no dithering, just shift the audio data and rounding
   no_dither:
   r3 = 1.0;
   r4 = r3 ASHIFT r7;                         
   r10 = r10 - 1;
   rMAC = M[I0, MK1];
   rMAC = rMAC ASHIFT r7(56bit);
   rMAC = MIN r4;

   do just_shift_loop;
      rMAC = M[I0, MK1], M[I4, MK1] = rMAC;
      rMAC = rMAC ASHIFT r7(56bit);
      rMAC = MIN r4;
   just_shift_loop:
   M[I4, MK1] = rMAC;
   jump end;

   // triangular dithering
   tpdf_dither:
   //----------------------------------------------------
   //              tpdf dither
   //                  |
   //               +--V--+    +---------+
   // input ------->| sum |----| Q(shift)|----->  output
   //               +-----+    +---------+
   //
   // ----------------------------------------------------
   I5 = &$cbops.dither_and_shift.coeffs + ($cbops.dither_and_shift.FILTER_COEFF_SIZE + 1)*ADDR_PER_WORD;
   rMAC = M[I0, MK1];
   rMAC = rMAC + r2 * r6;
   do tpdf_dither_loop;
      r2 = r2 * r4 (int);                       // save output, generate uniform random value
      r2 = r2 + M1;                             // get input
      rMAC =rMAC + r2 * r6;                     // add TPDF dither
      rMAC = rMAC + r5 * r6;                    // dc remove
      r2 = r2 * r4 (int), M[I5,0] = rMAC;       // generate another uniform random value
      r2 = r2 + M1, r1 = M[I5,0];
      r1 = r1 ASHIFT r7, rMAC = M[I0, MK1];        // shift operation, get next input
      rMAC = rMAC + r2 * r6, M[I4, MK1] = r1;      // save output
   tpdf_dither_loop:
   rMAC = M[I0, -MK1];
   jump end;

   // noise shaped dithering
   noiseshaped_dither:
   //-----------------------------------------------------------------------
   //   Noise-Shaped dithering structure
   //
   //                           tpdf dither
   //                               |
   //           +-----+          +--V--+     +----------+
   // input --->| sub |-----+----| sum |-----| Q(shift) |--------+---->output
   //           +--+--+     |    +-----+     +----------+        |
   //              |        |                                    |
   //              |        +------------------+                 |
   //              |                           |                 |
   //              |      +--------+        +--V--+   +----+     |
   //              +------| Filter |<-------| sub |<--| DQ |<----+
   //                     +--------+        +-----+   +----+
   //
   //----------------------------------------------------------------------
   // set up registers
   L1 = $cbops.dither_and_shift.FILTER_COEFF_SIZE * ADDR_PER_WORD;
   r0 = M[r8 + $cbops.dither_and_shift.DITHER_FILTER_HIST_FIELD];
   I1 = r0;
   I5 = &$cbops.dither_and_shift.coeffs;
   M0 = -($cbops.dither_and_shift.FILTER_COEFF_SIZE + 1)*ADDR_PER_WORD;
   M[$cbops.dither_and_shift.coeffs + ($cbops.dither_and_shift.FILTER_COEFF_SIZE + 1)*ADDR_PER_WORD] = r6;
   r6 = r6 * 0.75 (frac);
   r3 = -1;
   r0 = -r7;
   r3 = r3 ASHIFT r0, r1 = M[I1,0];
   do shaped_dither_loop;
      // generate first uniform random value
      r2 = r2 * r4 (int), rMAC = M[I0, MK1], r0 = M[I5, MK1];
      r2 = r2 + M1, M[I1, -MK1] = r1;
      // run filter
      #if defined(FILTER_COEFF_SIZE_7)
      rMAC = rMAC - r1 * r0, r1 = M[I1, -MK1], r0 = M[I5, MK1];
      rMAC = rMAC - r1 * r0, r1 = M[I1, -MK1], r0 = M[I5, MK1];
      #endif
      rMAC = rMAC - r1 * r0, r1 = M[I1, -MK1], r0 = M[I5, MK1];
      rMAC = rMAC - r1 * r0, r1 = M[I1, -MK1], r0 = M[I5, MK1];
      rMAC = rMAC - r1 * r0, r1 = M[I1, -MK1], r0 = M[I5, MK1];
      rMAC = rMAC - r1 * r0, r1 = M[I1,0], r0 = M[I5, MK1];
      rMAC = rMAC - r1 * r0;
      rMAC = rMAC + r2 * r6, M[I5,0] = rMAC;
      // generate second uniform random value
      r2 = r2 * r4 (int), r0 = M[I5, MK1];
      r2 = r2 + M1;
      // dither
      rMAC = rMAC + r2 * r6, r1 = M[I5,M0];
      // dc removal
      rMAC = rMAC + r5 * r1;
      // dequantize
      r1 = rMAC AND r3;
      // quantize
      rMAC = r1 ASHIFT r7;
      // feed back
      r1 = r1 - r0, M[I4, MK1] = rMAC;
      r1 = r1 * 4 (int)(sat);
   shaped_dither_loop:
   L1 = 0, M[I1,0] = r1;
   r0 = I1;
   M[r8 + $cbops.dither_and_shift.DITHER_FILTER_HIST_FIELD] = r0;

   end:
   L0 = 0;
   L4 = 0;
   M[rand_num] = r2;

  #ifdef ENABLE_PROFILER_MACROS
      // stop profiling if enabled
      r0 = &$cbops.profile_dither_and_shift;
      call $profiler.stop;
  #endif
    // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
// *****************************************************************************
// NAME:
//  $cbops.dither_and_shift.coeffs
// DESCRIPTION:
//  coefficient for feed-back filter, two sets of taps are provided here
//  to see the filter response in MATLAB do: freqz([1 -4*coeffs])
//
// *****************************************************************************
.MODULE $M.cbops.dither_and_shift.coeffs;
   .DATASEGMENT DM;
   .VAR/DM2 $cbops.dither_and_shift.coeffs[$cbops.dither_and_shift.FILTER_COEFF_SIZE+2] =
   #if defined(FILTER_COEFF_SIZE_7)
      0.56133527898090,
     -0.77524354319836,
      0.86648630699037,
     -0.79362354082620,
      0.56907855492224,
     -0.31752327548559,
      0.10827523331119, 0, 0;
   #else
      0.54602619736894,
     -0.63777198265141,
      0.54174933914712,
     -0.34770370646217,
      0.11651094321320, 0, 0;
   #endif
.ENDMODULE;
