// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.decode_cpe
//
// DESCRIPTION:
//    Get channel pair element information
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything
//
// *****************************************************************************
.MODULE $M.aacdec.decode_cpe;
   .CODESEGMENT AACDEC_DECODE_CPE_PM;
   .DATASEGMENT DM;

   $aacdec.decode_cpe:

   // push rLink onto stack
   push rLink;

   // make sure that sample_rate is known
   // (ie. corrupt frames might get us here with out it being set)
   Null = M[r9 + $aac.mem.SF_INDEX_FIELD];
   if NEG jump $aacdec.possible_corruption;

   // make sure we haven't had too many CPEs
   r0 = M[r9 + $aac.mem.NUM_CPEs];
   r0 = r0 + 1;
   M[r9 + $aac.mem.NUM_CPEs] = r0;
   Null = r0 - $aacdec.MAX_NUM_CPES;
   if GT jump $aacdec.possible_corruption;

   // dummy = getbits(4);  //element_instance_tag
   call $aacdec.get4bits;

   // set initial ics and spec pointers to the left channel
   r4 = r9 + $aac.mem.ICS_LEFT;
   M[r9 + $aac.mem.CURRENT_ICS_PTR] = r4;
   r0 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
   M[r9 + $aac.mem.CURRENT_CHANNEL] = Null;

   // set ms_mask_present = 0;
   M[r4 + $aacdec.ics.MS_MASK_PRESENT_FIELD] = Null;

   // common_window = getbits(1);
   call $aacdec.get1bit;
   M[r9 + $aac.mem.COMMON_WINDOW] = r1;

   // if (common_window == 1)
   if Z jump not_common_window;

      // ics_info();
      call $aacdec.ics_info;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump $aacdec.possible_corruption;


      // copy across ltp data from right chan to left (if common window)
      r1 = M[r9 + $aac.mem.ICS_LEFT + $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD];
      M[r9 + $aac.mem.ICS_RIGHT + $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD] = r1;

      r1 = M[r9 + $aac.mem.ICS_LEFT  + $aacdec.ics.LTP_INFO_PTR_FIELD];
      M[r9 + $aac.mem.ICS_RIGHT + $aacdec.ics.LTP_INFO_PTR_FIELD] = r1;

      r1 = M[r9 + $aac.mem.ICS_LEFT  + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD];
      M[r9 + $aac.mem.ICS_RIGHT + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD] = r1;


      // ms_mask_present = getbits(2);
      r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
      call $aacdec.get2bits;
      M[r4 + $aacdec.ics.MS_MASK_PRESENT_FIELD] = r1;

      // if (ms_mask_preset == 1)
      Null = r1 - 1;
      if NZ jump ms_mask_not_one;

         // allocate max_sfb words for the ms_used data
         // use frame memory
         r0 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
         call $aacdec.frame_mem_pool_allocate;
         if NEG jump $aacdec.possible_corruption;
         M[r4 + $aacdec.ics.MS_USED_PTR_FIELD] = r1;

         // set the initial bitmask = 1
         r7 = 1;

         // for g = 0:num_window_groups-1,
         r5 = M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD];
         num_win_groups_loop:

            // set I1 = start of ms_used array
            r1 = M[r4 + $aacdec.ics.MS_USED_PTR_FIELD];
            I1 = r1;

            // for sfb = 0:max_sfb-1,
            r6 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
            if Z jump max_sfb_loop_end;
            max_sfb_loop:

               // ms_used(g,sfb) = getbits(1);
               call $aacdec.get1bit;

               // read current ms_used word
               r2 = M[I1,0];
               // clear the current bit
               r3 = r2 AND r7;
               r2 = r2 - r3;
               // if getbits was 1 then add the bit to the current word
               Null = r1;
               if NZ r2 = r2 + r7;

               // move on to the next sfb
               r6 = r6 - r0,
                M[I1, MK1] = r2;          // and store back ms_used word
            if NZ jump max_sfb_loop;
            max_sfb_loop_end:

            // left shift the bit mask
            r7 = r7 LSHIFT 1;

            // move on to the next window group
            r5 = r5 - 1;
         if NZ jump num_win_groups_loop;

      ms_mask_not_one:

      // copy across data from the left ics structure to the right
      // need to copy all data up to ics.NUM_SEC_FIELD
      r10 = $aacdec.ics.NUM_SEC_FIELD/ADDR_PER_WORD;
      I1 = r9 + $aac.mem.ICS_LEFT;
      I2 = r9 + $aac.mem.ICS_RIGHT;


      do ics_copy;
         r0 = M[I1, MK1];
         M[I2, MK1] = r0;
      ics_copy:

   not_common_window:

   // -- reset the list of unpacked huffman tables --
   call $aacdec.huffman_reset_unpacked_list;

   // individual_channel_stream();
   call $aacdec.individual_channel_stream;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump $aacdec.possible_corruption;

   // set ics and spec pointers to the right channel
   r0 = r9 + $aac.mem.ICS_RIGHT;
   M[r9 + $aac.mem.CURRENT_ICS_PTR] = r0;
   r0 = M[r9 + $aac.mem.BUF_RIGHT_PTR];
   M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
   r0 = 1;
   M[r9 + $aac.mem.CURRENT_CHANNEL] = r0;

   // aacdec_individual_channel_stream();
   call $aacdec.individual_channel_stream;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
