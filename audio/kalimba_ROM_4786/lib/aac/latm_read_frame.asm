// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.latm_read_frame
//
// DESCRIPTION:
//    Read an latm frame (1 raw_data_block's worth per call)
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
.MODULE $M.aacdec.latm_read_frame;
   .CODESEGMENT AACDEC_LATM_READ_FRAME_PM;
   .DATASEGMENT DM;

   $aacdec.latm_read_frame:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.LATM_READ_FRAME_ASM.LATM_READ_FRAME.LATM_READ_FRAME.PATCH_ID_0, r1)
#endif


   // default is no faults detected
   M[r9 + $aac.mem.FRAME_UNDERFLOW] = Null;
   M[r9 + $aac.mem.FRAME_CORRUPT] = Null;
   M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION] = Null;

   // save some info to restore in case buffer under flow happened
   r0 = I0;
   M[r9 + $aac.mem.saved_I0] = r0;
   r0 = M[r9 + $aac.mem.GET_BITPOS];
   M[r9 + $aac.mem.saved_bitpos] = r0;
   r0 = M[r9 + $aac.mem.latm.CURRENT_SUBFRAME];
   M[r9 + $aac.mem.saved_current_subframe] = r0;

   // See if an LATM_SYNC_WORD follows
   r0 = 11;
   call $aacdec.showbits;
   Null = r1 - $aacdec.LATM_SYNC_WORD;
   if NZ jump no_latm_sync_word;
      // AudioSyncStream() flavour of LOAS present in the ISO reference encoded streams consists of:
      // - syncword                 11bits
      // - length information       13bits
      //
      // LOAS - Low Overhead Audio Stream
      r0 = 11;
      call $aacdec.getbits;
      Null = r1 - $aacdec.LATM_SYNC_WORD;
      if NE jump corrupt_file_error;
         r0 = 13;
         // skip audio_mux_element length
         call $aacdec.getbits;
   no_latm_sync_word:

   // call audio_mux_element() with muxConfigPresent = 1
   r0 = 1;
   call $aacdec.audio_mux_element;

#ifdef AACDEC_ENABLE_LATM_GARBAGE_DETECTION
   // garbage test
   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   Null = r0 -  ($aacdec.MAX_AAC_FRAME_SIZE_IN_BYTES*8*2);
   if POS jump garbage_detected;
#endif

   // check whether under flow has happened
   r0 = M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL];

   r1 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r0 = r0 - r1;
   if NEG jump buffer_underflow_occured;

   // buffer underflow already checked, any possible_frame_corruption
   // will mean input stream error.
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump corrupt_file_error;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

   buffer_underflow_occured:

      // restore saved context
      r0 = M[r9 + $aac.mem.saved_I0];
      I0 = r0;
      r0 = M[r9 + $aac.mem.saved_bitpos];
      M[r9 + $aac.mem.GET_BITPOS] = r0;
      r0 = M[r9 + $aac.mem.saved_current_subframe];
      M[r9 + $aac.mem.latm.CURRENT_SUBFRAME] = r0;

      r0 = 1;
      M[r9 + $aac.mem.FRAME_UNDERFLOW] = r0;
      // pop rLink from stack
      jump $pop_rLink_and_rts;

#ifdef AACDEC_ENABLE_LATM_GARBAGE_DETECTION
   garbage_detected:
   // we have detected garbage, discard everything in the
   // buffer, hopefully next chunck of data will be valid
   // This in practice should not happen
   #ifdef DEBUG_AACDEC
         r0 = M[r9 + $aac.mem.FRAME_GARBAGE_ERRORS];
         r0 = r0 + 1;
         M[r9 + $aac.mem.FRAME_GARBAGE_ERRORS] = r0;
   #endif
   r0 = M[r9 + $aac.mem.FRAME_NUM_BITS_AVAIL];
   r1 = M[r9 + $aac.mem.saved_bitpos];
   r0 = r0 - r1;
   r0 = r0 + 16;
   r0 = r0 LSHIFT -4;
   M0 = r0;
   r0 = M[r9 + $aac.mem.saved_I0];
   I0 = r0;
   r0 = M[I0, M0];
#endif

   corrupt_file_error:
      M[r9 + $aac.mem.latm.CURRENT_SUBFRAME] = Null;
      r0 = 1;
      M[r9 + $aac.mem.FRAME_CORRUPT] = r0;
      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;

