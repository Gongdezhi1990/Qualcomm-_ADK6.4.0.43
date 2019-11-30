// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

// ASM function for fade out

#include "audio_fadeout_asm.h"

.const $TRUE                                             1;
.const $FALSE                                            0;


// *****************************************************************************
// MODULE:
//    $audio.fade_out
//    $audio.fade_out_stereo
//
// DESCRIPTION:
//    Generates a fade out effect on a cbuffer with data, leaves r10, I0, I4 and
//    L4 unchanged. Stereo version does the same process on two buffers
//
// INPUTS:
//    - r0 = Fade out status
//    - r5 = Number of zeros left to flush before finished
//    - r7 = address of fadeout parameter structure
//    - r10 = Number of samples to process
//    - I0 = buffer start read address
//    - L0,B0 = buffer size, start addr
//    - I1 = right buffer start read address     (STEREO ONLY)
//    - L1,B1 = right buffer size, start addr    (STEREO ONLY)
//
// OUTPUTS:
//    - r2 = New fade out status
//    - r4 = is_finished ($previn.TRUE or $previn.FALSE)
//    - r5 = updated number of zeros left to flush before finished
//
// TRASHED REGISTERS:
//    r0-r4
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.audio.fade_out;
   .CODESEGMENT AUDIO_FADEOUT_PM;

   $audio.fade_out_stereo:

      // sampling period in usecs, preset for 16kHz:
      .const SAMPLING_PERIOD_IN_USECS  63;

// This factor gives a fade-out of ~520 samples, which is ~32ms at 16kHz.
// Increase it for a faster ramp-down.
#define SCALE_FRACT (0x0.000200)
      push rLink;
      push I1;
      push B1;

      // stereo flag
      r3 = 1;
      jump continue;

   $audio.fade_out:
      push rLink;
      // mono flag
      r3 = 0;
      // fall through

   continue:

   // Save registers
   push I0;
   push B0;
   push r10;
   push r6;

   r4 = $FALSE;
   r2 = r0;

   Null = r0 - $audio.fade_out.FLUSHING_STATE;
   if Z jump flushing;
   Null = r0 - $audio.fade_out.END_STATE;
   if Z jump zero_samples;

      // we are in running state:

      // index of samples since beginning
      r0 = M[r7 + $audio.fade_out.FADEOUT_COUNTER_FIELD];

      push r2;

      // sample count from last round, update it
      r6 = r0;

      // take first left (or mono) sample
      r0 = M[I0,0];

      // right channel? if exists, take first sample
      Null = r3;
      if Z jump dothefade;
       r2 = M[I1, 0];

      dothefade:

      do fade_out_loop;
         // Calculate scaling factor applied to current sample:
         //     SAMPLE_SCALING_FACTOR = 1.0 - (sample_counter * sampling_period_in_usecs / ramp_duration_in_usecs)
         //     (below, SCALE_FRACT = 1.0 / ramp_duration_in_usecs)

         // NOTE:
         //      - "usecs" are approximate here, 2^(-20) was/is used for quick maths, which is roughly 1E-6
         //      - for 16K sampling rate considered in SAMPLING_PERIOD_IN_USECS:
         //        ramp duration is ~11ms for actual 48K sampling rate, ~32ms for 16K sampling rate
         //        This is perfectly OK subjectively (even 5ms soft mutes are very widely used)
         //      - for SAMPLING_PERIOD_IN_USECS taken for 48K sampling rate,
         //        the ramp duration is ~32ms, for actual 16kHz sampling rate, the ramp will be ~96ms

         // r6 is sample count since the beginning of ramping
         // This coefficient r1 starts at 1.0; a larger SCALE_FRACT causes it to cross zero
         // more quickly => shorter ramp
         r1 = r6 * (SCALE_FRACT * SAMPLING_PERIOD_IN_USECS) (int);
         r1 = 1.0 - r1;
         // if we reached end of ramp, rest is just zeroing from now on
         // re-use existing flushing/zeroing and its counter & status updates etc.
         if NEG jump transition;

         r0 = r0 * r1 (frac);
         M[I0,ADDR_PER_WORD] = r0;

         // right channel? if exists, do scaling with same coeff
         Null = r3,  r0 = M[I0,0];
         if Z jump updatecount;

         r2 = r2 * r1 (frac);
         M[I1,ADDR_PER_WORD] = r2;
         r2 = M[I1,0];

      updatecount:
         r6 = r6 + 1; // update sample count

      fade_out_loop:

         pop r2;   // make sure we pop back r2, it was saved in running state
         jump updates_before_exit;

      transition:
         pop r2;  // discard the saved r2, override previous state with flushing instead of passing it through
         r2 = $audio.fade_out.FLUSHING_STATE;

      flushing:
         r5 = r5 - r10;
         if POS jump zero_samples;

         r4 = $TRUE;
         r2 = $audio.fade_out.END_STATE;
         r5 = 0;
         // fall through

      zero_samples:
         // We may still have some samples to process, just zero them
         r0 = 0;
         r1 = r10;  // keep what was r10 at this point - since we now can jump here from within a L channel loop,
         // and r10 is actual remaining count - this no longer operates on full blocks of samples with constant scaling
         do fade_out_loop_flush;
            M[I0,ADDR_PER_WORD] = r0;
         fade_out_loop_flush:

         // check for right channel
         Null = r3;
         if Z jump updates_before_exit;

         // used to be: r10 = M[SP-2*ADDR_PER_WORD]; but now we don't operate on entire given block of samples
         r10 = r1;
         do fade_out_loop_flush_right;
            M[I1,ADDR_PER_WORD] = r0;
         fade_out_loop_flush_right:


      updates_before_exit:
         // now update sample count with how much was processed here
         r10 = M[SP-2*ADDR_PER_WORD];
         r6 = M[r7 + $audio.fade_out.FADEOUT_COUNTER_FIELD];
         r6 = r6 + r10;
         M[r7 + $audio.fade_out.FADEOUT_COUNTER_FIELD] = r6;


   exit:
      // Restore registers
      pop r6;
      pop r10;

      pop B0;
      pop I0;
      Null = r3;
      if Z jump pop_return;

      // restore remaining registers for stereo
      pop B1;
      pop I1;
  pop_return:
      pop rLink;
      rts;

.ENDMODULE;



// *****************************************************************************
// MODULE:
//    $_mono_cbuffer_fadeout
//
// DESCRIPTION:
//    C wrapper for the common audio.fade_out function, to be used by operators.
//
// INPUTS:
//    - r0 = CBuffer pointer to input data that will be in-place processed
//    - r1 = number of samples to process (does not check this against CBuffer amount of data)
//    - r2 = pointer to FADEOUT_PARAMS structure
//
// OUTPUTS:
//    - r0 = return code (0 for success)
//    - other outputs/updates are placed inside the operator data structure
//
// TRASHED REGISTERS:
//    r0-r3
//
// NOTES: The following can be generalised (e.g. if it receives pointer to fadeout param
//        struct separately, it can be used by other operators, too - for now it is mono
//        wrapper working with the basic mono operator's structures.
//
// *****************************************************************************
.MODULE $M.audio.fadeout;
   .CODESEGMENT AUDIO_FADEOUT_PM;

$_mono_cbuffer_fadeout:

   push rLink;
   pushm <I0, L0>;
   push B0;
   pushm <r4, r5, r7, r10>;

   // let's set up pointer to the fade-out parameter structure
   // If this is generalised then the wrapper could be re-used by others.
   r7 = r2;

   // r10 = Number of samples to process
   r10 = r1;

   // r5 = Number of zeros left to flush before finished
   r5 = M[r7 + $audio.fade_out.FADEOUT_FLUSH_COUNT_FIELD];

   call $cbuffer.get_read_address_and_size_and_start_address;

   // get the input buffer info from sink
   // I0 = buffer start read address, L0,B0 = buffer size, start addr
   push r2;
   pop B0;

   I0 = r0;
   L0 = r1;

   // r0 = Fade out status
   r0 = M[r7 + $audio.fade_out.FADEOUT_STATE_FIELD];

   //  r7 = fadeout parameter structure

   // *** do mono fade-out ***
   call $audio.fade_out;

   // r2 = New fade out status
   M[r7 + $audio.fade_out.FADEOUT_STATE_FIELD] = r2;

   // r5 = updated number of zeros left to flush before finished
   M[r7 + $audio.fade_out.FADEOUT_FLUSH_COUNT_FIELD] = r5;

   // r4 = is_finished (TRUE or FALSE)
   r0 = r4;

   popm <r4, r5, r7, r10>;
   pop B0;
   popm <I0, L0>;
   pop rLink;

   rts;

.ENDMODULE;

