// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_read_frame
//
// DESCRIPTION:
//    Read an mp4 frame (1 raw_data_block's worth per call)
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - assume everything including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.mp4_read_frame;
   .CODESEGMENT AACDEC_MP4_READ_FRAME_PM;
   .DATASEGMENT DM;

   $aacdec.mp4_read_frame:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.MP4_READ_FRAME_ASM.MP4_READ_FRAME.MP4_READ_FRAME.PATCH_ID_0, r1)
#endif
   
   // default is no faults detected
   M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION] = Null;
   M[r9 + $aac.mem.FRAME_UNDERFLOW] = Null;

   // if mp4 header has not been fully parsed then continue parsing it
   r0 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
   Null = r0 AND 0x1;
   if Z call $aacdec.mp4_sequence;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump possible_corruption;

   Null = M[r9 + $aac.mem.FRAME_UNDERFLOW];
   if NZ jump frame_underflow;

   // once the mp4 header has been parsed we still need there to be enough data
   // in the buffer to decode a frame
   r0 = M[r9 + $aac.mem.MP4_HEADER_PARSED];
   Null = r0 AND 0x1;
   if Z jump frame_underflow;

      Null = M[r9 + $aac.mem.MP4_DECODING_STARTED];
      if NZ jump read_raw_data_block;

      // check enough data available to decode the first frame
      r0 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];
      Null = r0 - $aacdec.MAX_AAC_FRAME_SIZE_MP4;
      if NEG jump frame_underflow;

      r0 = 1;
      M[r9 + $aac.mem.MP4_DECODING_STARTED] = r0;


   read_raw_data_block:
   // check if all valid data in mdat has been processed
   Null = M[r9 + $aac.mem.MDAT_PROCESSED];
   if NZ jump frame_underflow;

   r1 = M[r9 + $aac.mem.FAST_FWD_SAMPLES_LS];
   r2 = M[r9 + $aac.mem.FAST_FWD_SAMPLES_MS];
   Null = r1 OR r2;
   if Z jump not_fast_fwd_rew;
      call $aacdec.mp4_ff_rew;
      Null = M[r9 + $aac.mem.FRAME_UNDERFLOW];
      if NZ jump frame_underflow;
      jump $pop_rLink_and_rts;

not_fast_fwd_rew:
   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   M[r9 + $aac.mem.TEMP_BIT_COUNT] = r0;
   // -- Decode the raw data block --
   PROFILER_START(&$aacdec.profile_raw_data_block)
#ifdef AACDEC_ELD_ADDITIONS
   r2 = $aacdec.raw_data_block;
   r3 = $aacdec.eld_raw_data_block;
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if EQ r2 = r3;
   // r0 and r1 are not required for $aacdec.raw_data_block but they do no harm
   r1 = $aacdec.BYTE_ALIGN_ON;
   r0 = M[r9 + $aac.mem.CHANNEL_CONFIGURATION_FIELD];
   call r2;
#else
   call $aacdec.raw_data_block;
#endif // AACDEC_ELD_ADDITIONS
   PROFILER_STOP(&$aacdec.profile_raw_data_block)

   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump possible_corruption;

   // Update mp4 frame count
   r0 = M[r9 + $aac.mem.MP4_FRAME_COUNT];
   r0 = r0 + 1;
   M[r9 + $aac.mem.MP4_FRAME_COUNT] = r0;


   call $aacdec.byte_align;

   // Calculate number of bytes read in mp4a atom
   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r3 = M[r9 + $aac.mem.TEMP_BIT_COUNT];
   r0 = r0 - r3;
   r0 = r0 ASHIFT -3;


   r3 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
   r2 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
   r2 = r0 + r2;
   r3 = r3 + Carry;


   M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)] = r3;
   M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)] = r2;


   // take off the mdat offset
   r1 = M[r9 + $aac.mem.MDAT_OFFSET + (0*ADDR_PER_WORD)];
   r0 = M[r9 + $aac.mem.MDAT_OFFSET + (1*ADDR_PER_WORD)];
   r2 = r2 - r0;
   r3 = r3 - r1 - Borrow;

   r4 = 1;
   r1 = M[r9 + $aac.mem.MDAT_SIZE + (0*ADDR_PER_WORD)];
   r0 = M[r9 + $aac.mem.MDAT_SIZE + (1*ADDR_PER_WORD)];
   r5 = M[r9 + $aac.mem.MDAT_SIZE + (2*ADDR_PER_WORD)];
   r6 = r0;
   r0 = r0 + r5; /// check if r5 can be used ravneet
   r1 = r1 + r6 + Carry;

   //Compare this with file offset
   r0 = r0 - r2;
   r1 = r1 - r3 - Borrow;
   if POS r4 = 0;
   //Note: $aacdec.mdat_size + 0 can be ignored for practical purposes. To be Removed

   M[r9 + $aac.mem.MDAT_PROCESSED] = r4;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

   frame_underflow:
      r0 = 1;
      M[r9 + $aac.mem.FRAME_UNDERFLOW] = r0;

      // pop rLink from stack
      jump $pop_rLink_and_rts;


   possible_corruption:
      // Calculate number of bytes read in mp4a atom
      r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r3= M[r9 + $aac.mem.TEMP_BIT_COUNT];
      r0 = r0 - r3;
      r0 = r0 ASHIFT -3;


      r3 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)];
      r2 = M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)];
      r2 = r0 + r2;
      r3 = r3 + Carry;


      M[r9 + $aac.mem.MP4_FILE_OFFSET + (0*ADDR_PER_WORD)] = r3;
      M[r9 + $aac.mem.MP4_FILE_OFFSET + (1*ADDR_PER_WORD)] = r2;


      r0 = M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL];
      r3 = M[r9 + $aac.mem.READ_BIT_COUNT];
      r0 = r0 - r3;
      if POS jump $aacdec.corruption;
         r0 = 1;
         M[r9 + $aac.mem.FRAME_UNDERFLOW] = r0;
         // pop rLink from stack
         jump $pop_rLink_and_rts;


.ENDMODULE;

