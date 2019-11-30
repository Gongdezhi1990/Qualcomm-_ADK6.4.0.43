// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_ff_rew
//
// DESCRIPTION:
//    - r1 = fast_forward_remaining_samples_ls
//    - r2 = fast_forward_remaining_samples_ms
//
// OUTPUTS:
//    - r4 - Status of fast fwd/rwd (0 --> FAIL/PENDING, 1 --> SUCCESS)
//    - r5 - LS value of output samples produced
//    - r6 - MS value of output samples produced
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.aac_ff_rew;
   .CODESEGMENT AACDEC_AAC_FF_REW_PM;
   .DATASEGMENT DM;

   $aacdec.aac_ff_rew:

   push rLink;

   r4 = 0;

   Null = M[r9 + $aac.mem.SKIP_FUNCTION];
   if Z jump $pop_rLink_and_rts;

   //find out which FF/REW function to call (ADTS or MP4)

   #ifdef AACDEC_MP4_FILE_TYPE_SUPPORTED
      r3 = M[r9 + $aac.mem.READ_FRAME_FUNCTION];
      Null = r3 - M[$aacdec.read_frame_func_table + 0];
      if NZ jump not_mp4_file;
         // check whether mp4 FF/REW is done
         call $aacdec.mp4_ff_rew_get_status;
         jump $pop_rLink_and_rts;

   #endif

   not_mp4_file:
      call $aacdec.adts_ff_rew;

   jump $pop_rLink_and_rts;

.ENDMODULE;

#ifdef AACDEC_MP4_FILE_TYPE_SUPPORTED

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_ff_rew_get_status
//
// DESCRIPTION:
//    - r1 = fast_forward_remaining_samples_ls
//    - r2 = fast_forward_remaining_samples_ms
//
// OUTPUTS:
//    - r4 - Status of fast fwd/rwd (0 --> FAIL/PENDING, 1 --> SUCCESS)
//    - r5 - LS value of output samples produced
//    - r6 - MS value of output samples produced
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.mp4_ff_rew_get_status;
   .CODESEGMENT AACDEC_AAC_FF_REW_PM;
   .DATASEGMENT DM;

   $aacdec.mp4_ff_rew_get_status:

   push rLink;

   r4 = 0;

   // Check mp4 FF/REW status
   r0 = M[r9 + $aac.mem.MP4_FF_REW_STATUS];

   Null = r0 - $aacdec.MP4_FF_REW_SEEK_NOT_POSSIBLE;
   // Unable to jump. Just return and allow decoder to gobble
   // This might happen if STZ2 is present instead of STSZ. To be fixed
   if Z jump $pop_rLink_and_rts;

   Null = r0 - $aacdec.MP4_FF_REW_NULL;
   if NZ jump mp4_ff_rew_status_not_null;
      // initial state. Change to IN_PROGRESS
      M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS] = r1;
      M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS] = r2;
      r1 = $aacdec.MP4_FF_REW_IN_PROGRESS;
      M[r9 + $aac.mem.MP4_FF_REW_STATUS] = r1;
      jump $pop_rLink_and_rts;

   mp4_ff_rew_status_not_null:
   Null = r0 - $aacdec.MP4_FF_REW_IN_PROGRESS;
   if Z jump $pop_rLink_and_rts; // Ongoing. Just return

   r4 = 1;
   r5 = M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS];
   r6 = M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS];
   r1 = $aacdec.MP4_FF_REW_NULL;
   M[r9 + $aac.mem.MP4_FF_REW_STATUS] = r1;
   M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS] = Null;
   M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS] = Null;

   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

// *****************************************************************************
// MODULE:
//    $aacdec.adts_ff_rew
//
// DESCRIPTION:
//    - r1 = fast_forward_remaining_samples_ls
//    - r2 = fast_forward_remaining_samples_ms
//
// OUTPUTS:
//    - r4 - Status of fast fwd/rwd (0 --> FAIL, 1 --> SUCCESS)
//    - r5 - LS value of output samples produced
//    - r6 - MS value of output samples produced
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.adts_ff_rew;
   .CODESEGMENT AACDEC_AAC_FF_REW_PM;
   .DATASEGMENT DM;

   $aacdec.adts_ff_rew:

   push rLink;

   M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS] = r1;
   M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS] = r2;

   // check if it is rewind
   Null = r2;
   if POS jump not_rewind;
      r1 = Null - r1;
      r2 = Null - r2 - Borrow;

   not_rewind:
      // rMAC = {r2,r1)
      rMAC = r1 LSHIFT 0 (LO);
      r3 = 0x800000;
      rMAC = rMAC + r2 * r3 (UU);

      r0 = 2;
      r3 = $aacdec.MAX_AUDIO_FRAME_SIZE_IN_WORDS;
      Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
      if NZ r3 = r0 * r3 (int);

      Div = rMAC/r3;
      rMAC = M[r9 + $aac.mem.FRAME_LENGTH];
      r6 = DivResult;

      Null = M[r9 + $aac.mem.AVG_BIT_RATE];
      if Z jump avg_bit_rate_unknown;

      r2 = M[r9 + $aac.mem.AVG_BIT_RATE];
      r2 = r2 * 1000 (int); // convert to bits per second
      rMAC = r2 LSHIFT -3; // Divide by 8 (bits to bytes)
      rMAC = r3 * rMAC;

      r1 = M[r9 + $aac.mem.SF_INDEX_FIELD];;

      r0 = &$aacdec.sampling_freq_lookup;
      r5 = r1;
      call $mem.ext_window_access_as_ram;
      r1 = r5;
      r0 = M[r0 + r5];
      r1 = M[r9 + $aac.mem.SAMPLE_RATE_TAGS_FIELD];
      r1 = r1  + $aacdec.OFFSET_TO_SAMPLE_RATE_TAG;
      r1 = r1 + r0;
      r0 = M[r1]; // read sampling frequency
      r0 = r0 * 2 (int); // rMAC has double the actual value
      Div = rMAC/r0;
      rMAC= DivResult;

   avg_bit_rate_unknown:

      rMAC = rMAC * r6;

      r3 = rMAC LSHIFT 23;
      r4 = rMAC LSHIFT -1;

      // Seek must be negative in case of rewind
      Null = M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS];
      if POS jump pos_seek;
         r3 = Null - r3;
         r4 = Null - r4 - Borrow;

   pos_seek:
      r6 = M[r9 + $aac.mem.SKIP_FUNCTION];
      // set I0 to point to cbuffer for mp3 input stream
      r5 = M[r9 + $aac.mem.codec_struc];
      r0 = M[r5 + $codec.DECODER_IN_BUFFER_FIELD];
#ifdef BASE_REGISTER_MODE
      call $cbuffer.get_read_address_and_size_and_start_address;
      push r2;
      pop  B0;
#else
      call $cbuffer.get_read_address_and_size;
#endif
      I0 = r0;

      call $aacdec.skip_through_file;

      r4 = 1;
      r5 = M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS];
      r6 = M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS];
      M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS] = Null;
      M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS] = Null;
      L0 = 0;
      // pop rLink from stack
      jump $pop_rLink_and_rts;


.ENDMODULE;
