// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// *****************************************************************************
// NAME:
//    Sidetone mix operator
//
// DESCRIPTION:
//    In a headset type application it is often desirable to feed some of the
// received microphone signal back out on the speaker. This gives the user some
// indication the headset is on. This operator mixes data from a supplied buffer
// in with the "main" data stream.
//
// @verbatim
//      Received SCO data       +         Data to DAC
//      ---------------------->O---------------------->
//                             ^
//                             |
//                             O x g
//                             |
//                             |
//                    Received ADC data
// @endverbatim
//
//
//    The number of samples in the sidetone buffer is monitored at this point.
// If there are too few, extra samples are created by repeating the last sample
// from the buffer. If too many samples are present, one sample will be
// discarded during each call.
//
// When using the operator the following data structure is used:
//    - $cbops.sidetone_mix.INPUT_START_INDEX_FIELD = the input buffer
//       index
//    - $cbops.sidetone_mix.OUTPUT_START_INDEX_FIELD = the output buffer
//       index
//    - $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD = the address of the
//       sidetone cbuffer struc
//    - $cbops.sidetone_mix.SIDETONE_MAX_SAMPLES_FIELD = the number of
//       samples allowed to build up in the sidetone buffer
//    - $cbops.sidetone_mix.GAIN_FIELD = the address of the gain to apply
//       to the sidetone samples
//
// *****************************************************************************

#include "stack.h"
#include "cbops.h"

.MODULE $M.cbops.sidetone_mix;
   .DATASEGMENT DM;

   // ** function vector **
   .VAR $cbops.sidetone_mix[$cbops.function_vector.STRUC_SIZE] =
      &$cbops.sidetone_mix.reset,           // reset function
      $cbops.function_vector.NO_FUNCTION,   // amount to use function
      &$cbops.sidetone_mix.main;            // main function

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.sidetone_mix.reset
//
// DESCRIPTION:
//    Reset routine for the side tone mix operator, see $cbops.sidetone_mix.main
//
// INPUTS:
//    - r8 = pointer to operator structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, I0, L0
//
// *****************************************************************************
.MODULE $M_cbops.sidetone_mix.reset;
   .CODESEGMENT CBOPS_SIDETONE_MIX_RESET_PM;
   .DATASEGMENT DM;

   $cbops.sidetone_mix.reset:
   // push rLink onto stack
   push rLink;
   // empty the sidetone buffer
   r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
   call $cbuffer.get_read_address_and_size;
   I0 = r0;
   L0 = r1;

   // write a zero in the previous location
   r0 = M[I0,-1];
   r0 = 0;
   M[I0,1] = r0;

   r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
   r1 = I0;
   call $cbuffer.set_write_address;
   L0 = 0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $cbops.sidetone_mix.main
//
// DESCRIPTION:
//    Operator that copies the output/input word and reads/writes the samples
//    to/from the sidetone buffer
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
//    r0, r1, r2, r3, r5, r6, r10, rMAC, M1, I0, I1, I4, L0, L1, L4, DoLoop
//
// *****************************************************************************
.MODULE $M_cbops.sidetone_mix.main;
   .CODESEGMENT CBOPS_SIDETONE_MIX_MAIN_PM;
   .DATASEGMENT DM;

   $cbops.sidetone_mix.main:

   // push rLink onto stack
   push rLink;

#ifdef PATCH_LIBS
    LIBS_SLOW_SW_ROM_PATCH_POINT($cbops.sidetone_mix.main.PATCH_ID_0, r1)
#endif

   #ifdef ENABLE_PROFILER_MACROS
      .VAR/DM1 $cbops.profile_sidetone_mix[$profiler.STRUC_SIZE] = $profiler.UNINITIALISED, 0 ...;
      r0 = &$cbops.profile_sidetone_mix;
      call $profiler.start;
   #endif

   // get the offset to the read buffer to use
   r0 = M[r8 + $cbops.sidetone_mix.INPUT_START_INDEX_FIELD];

   // setup modify registers
   M1 = 1;

   // get the input buffer read address
   r1 = M[r6 + r0];
   // store the value in I0
   I0 = r1;
   // get the input buffer length
   r1 = M[r7 + r0];
   // store the value in L0
   L0 = r1;

   // get the offset to the write buffer to use
   r0 = M[r8 + $cbops.sidetone_mix.OUTPUT_START_INDEX_FIELD];

   // get the output buffer write address
   r1 = M[r6 + r0];
   // store the value in I4
   I4 = r1;
   // get the output buffer length
   r1 = M[r7 + r0];
   // store the value in L4
   L4 = r1;

   // calculate the number of samples in the sidetone buffer
   r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
   call $cbuffer.calc_amount_data;

   // save r10 - in case it is trashed below
   r6 = r10;

   // are there enough sidetone samples to copy to the dac
   r3 = r0 - r10;
   if GE jump no_underflow;

      // there weren't enough samples - we have to insert some

      // get the tone buffer write address and size
      r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
      call $cbuffer.get_write_address_and_size;
      I1 = r0;
      L1 = r1;

      // dummy read to move back to the last good sample
      r0 = M[I1,-1];

      // get the number of samples to insert in r10
      // and get the last good sample
      r10 = -r3, r0 = M[I1,M1];

      do make_missing_samples_loop;
         M[I1,1] = r0;
      make_missing_samples_loop:

      // update the current write address:
      r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
      r1 = I1;
      call $cbuffer.set_write_address;
      // clear the length register
      L1 = 0;

   no_underflow:


   // ** write to DAC **
   // get the sidetone buffer read address and size
   r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
   call $cbuffer.get_read_address_and_size;
   I1 = r0;
   // set the length & get the first DAC sample
   L1 = r1, rMAC = M[I0,M1];

   // load r10 with (loop count-1) and get the first sidetone sample
   r10 = r6 - M1, r0 = M[I1,M1];

   // if we have a lot of spare samples in the side tone buffer get rid of some
   r2 = M[r8 + $cbops.sidetone_mix.SIDETONE_MAX_SAMPLES_FIELD];
   // r3 contains the number of excess samples in the side tone buffer
   NULL = r3 - r2;
   if NEG jump no_sidetone_adjust;
      // consume another sample
      r0 = M[I1,1];
   no_sidetone_adjust:

   // read the address of the gain to use
   r1 = M[r8 + $cbops.sidetone_mix.GAIN_FIELD];
   // dereference it
   r1 = M[r1];

   do write_loop;
      // calculate the current output sample and read a side tone sample
      rMAC = rMAC + r0 * r1, r0 = M[I1,1];

      // get the next DAC value and write the result
      rMAC = M[I0,1],  M[I4,1] = rMAC;
   write_loop:
   // calculate the last output sample
   rMAC = rMAC + r0 * r1;
   // zero L1 and write the last output sample
   L1 = 0, M[I4,1] = rMAC;
   // set the read pointer in the side tone buffer
   r0 = M[r8 + $cbops.sidetone_mix.SIDETONE_BUFFER_FIELD];
   r1 = I1;
   call $cbuffer.set_read_address;

   // zero the remaining length registers we have used
   L0 = 0;
   L4 = 0;

   // stop profiling if enabled
   #ifdef ENABLE_PROFILER_MACROS
      r0 = &$cbops.profile_sidetone_mix;
      call $profiler.stop;
   #endif

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
