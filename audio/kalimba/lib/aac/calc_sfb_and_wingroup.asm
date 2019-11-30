// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"


// *****************************************************************************
// MODULE:
//    $aacdec.calc_sfb_and_wingroup
//
// DESCRIPTION:
//    Calculate the scale factor bands and window group information
//
// INPUTS:
//    - r4 = current ics pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5-r7, r10, I1-I3
//
// *****************************************************************************
.MODULE $M.aacdec.calc_sfb_and_wingroup;
   .CODESEGMENT AACDEC_CALC_SFB_AND_WINGROUP_PM;
   .DATASEGMENT DM;

   $aacdec.calc_sfb_and_wingroup:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.CALC_SFB_AND_WINGROUP_ASM.CALC_SFB_AND_WINGROUP.CALC_SFB_AND_WINGROUP.PATCH_ID_0, r5)
#endif
   

   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z jump eight_short_sequence;

   only_long_sequence:
   long_start_sequence:
   long_stop_sequence:

      // num_windows = 1
      // num_window_groups = 1
      // window_group_length[num_window_groups - 1] = 1
      r0 = 1;
      M[r4 + $aacdec.ics.NUM_WINDOWS_FIELD] = r0;
      M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD] = r0;
      M[r4 + $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD] = r0;

      // num_swb = num_swb_long_window[sampling_freq]
      // Note: sampling_freq = sampling_freq_lookup[sf_index]
      r5 = M[r9 + $aac.mem.SF_INDEX_FIELD];
      r0 = &$aacdec.sampling_freq_lookup;
      call $mem.ext_window_access_as_ram;
      Words2Addr(r5);
      r5 = M[r0 + r5];
      r0 = &$aacdec.num_swb_long_window;

#ifdef AACDEC_ELD_ADDITIONS
      r6 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r6 - $aacdec.ER_AAC_ELD;
      if NE jump num_swb_long_window_selected;
         r1 = &$aacdec.num_swb_long_window_480;
         r0 = &$aacdec.num_swb_long_window_512;
         Null = M[r9 + $aac.mem.ELD_frame_length_flag];
         if NZ r0 = r1;
      num_swb_long_window_selected:
#endif // AACDEC_ELD_ADDITIONS

      call $mem.ext_window_access_as_ram;
      Words2Addr(r5);
      r0 = M[r0 + r5];
      M[r4 + $aacdec.ics.NUM_SWB_FIELD] = r0;

      // error if max_sfb > num_swb
      r1 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
      Null = r0 - r1;
      if NEG jump $aacdec.possible_corruption;

      // allocate tmp memory for sect_sfb_offset
      // size: num_swb + 1
      //  - it's only needed up to scalefactors being applied
      r10 = r0;
      r0 = r0 + 1;
      call $aacdec.tmp_mem_pool_allocate;
      if NEG jump $aacdec.possible_corruption;
      M[r4 + $aacdec.ics.SECT_SFB_OFFSET_PTR_FIELD] = r1;
      I1 = r1;

      // for i=0:num_swb,
      // {
      //    sect_sfb_offset[0][i] = swb_offset_long_window[sampling_freq][i];
      //    swb_offset[i] = swb_offset_long_window[sampling_freq][i];
      // }

      r0 = &$aacdec.swb_offset_long_table;

#ifdef AACDEC_ELD_ADDITIONS
      // r6 loaded with AOT above
      Null = r6 - $aacdec.ER_AAC_ELD;
      if NE jump swb_offset_long_table_selected;
      r1 = &$aacdec.swb_offset_long_table_480;
      r0 = &$aacdec.swb_offset_long_table_512;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r1;
      swb_offset_long_table_selected:
#endif // AACDEC_ELD_ADDITIONS
      r1 = r9 + $aac.mem.SWB_OFFSET_FIELD;
      M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD] = r1;
      I2 = r1;
      call $mem.ext_window_access_as_ram;
      r0 = M[r0 + r5];
      call $mem.ext_window_access_as_ram;
      I3 = r0;

      do long_sect_sfb_offset_loop;
         r0 = M[I3, MK1];
         M[I1, MK1] = r0;
         M[I2, MK1] = r0;
      long_sect_sfb_offset_loop:

      // force last offset to be the end
      r0 = 1024;

#ifdef AACDEC_ELD_ADDITIONS
      // r6 loaded with AOT above
      Null = r6 - $aacdec.ER_AAC_ELD;
      if NE jump last_offset_set;
      r1 = 480;
      r0 = 512;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r0 = r1;
      last_offset_set:
#endif // AACDEC_ELD_ADDITIONS

      M[I1, MK1] = r0;
      M[I2, MK1] = r0;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

   eight_short_sequence:

      r6 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r6 - $aacdec.ER_AAC_ELD;
      if EQ jump $pop_rLink_and_rts;

      // num_windows = 8
      // window_group_length[num_window_groups - 1] = 1
      r0 = 8;
      M[r4 + $aacdec.ics.NUM_WINDOWS_FIELD] = r0;
      r0 = 1;
      M[r4 + $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD] = r0;

      // num_swb = num_swb_short_window[sampling_freq]
      // Note: sampling_freq = sampling_freq_lookup[sf_index]
      r5 = M[r9 + $aac.mem.SF_INDEX_FIELD];
      r0 = &$aacdec.sampling_freq_lookup;
      call $mem.ext_window_access_as_ram;
      Words2Addr(r5);
      r5 = M[r0 + r5];

      r0 = &$aacdec.num_swb_short_window;
      call $mem.ext_window_access_as_ram;
      Words2Addr(r5);
      r0 = M[r0 + r5];
      M[r4 + $aacdec.ics.NUM_SWB_FIELD] = r0;

      // error if max_sfb > num_swb
      r1 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
      Null = r0 - r1;
      if NEG jump $aacdec.possible_corruption;
      r10 = r0;
      // for i=0:num_swb,
      //    swb_offset[i] = swb_offset_short_window[sampling_freq][i]


      r0 = &$aacdec.swb_offset_short_table;
      r1 = r9 + $aac.mem.SWB_OFFSET_FIELD;//&$aacdec.swb_offset;
      M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD] = r1;
      I1 = r1;
      call $mem.ext_window_access_as_ram;
      r0 = M[r0 + r5];
      call $mem.ext_window_access_as_ram;

      I3 = r0;

      do short_sect_sfb_offset_loop;
         r0 = M[I3, MK1];
         M[I1, MK1] = r0;
      short_sect_sfb_offset_loop:

      r0 = 128;
      M[I1, MK1] = r0;

      // for i=0:num_windows-2,
      //    if bitset(scalefactor_grouping,6-1)==0
      //    {
      //       num_window_groups++;
      //       window_group_length[num_window_groups-1] = 1;
      //    }
      //    else
      //    {
      //       window_group_length[num_window_groups-1]++;
      //    }
      r2 = M[r4 + $aacdec.ics.SCALE_FACTOR_GROUPING_FIELD];
      r2 = r2 LSHIFT (DAWTH-8);
      I1 = r4 + $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD;
      r10 = 7;
      // num_window_groups = 1
      r7 = 1;
      do short_num_windows_loop;
         r2 = r2 LSHIFT 1;
         if NEG jump scalefactor_grouping_bit_not_set;
            r7 = r7 + 1;
            I1 = I1 + (1*ADDR_PER_WORD);
            r0 = 1;
            jump update_window_group_length;
         scalefactor_grouping_bit_not_set:
            r0 = M[I1,0];
            r0 = r0 + 1;
         update_window_group_length:
         M[I1,0] = r0;
      short_num_windows_loop:

      // store back num_window_groups
      M[r4 + $aacdec.ics.NUM_WINDOW_GROUPS_FIELD] = r7;


      // for g=0:num_window_groups-1
      // {
      //    sect_sfb = 0;
      //    offset = 0;
      //    for i=0:num_swb-1,
      //    {
      //       width = swb_offset_short_window[sampling_freq][i+1] -
      //               swb_offset_short_window[sampling_freq][i];
      //       width = width * window_group_length[g];
      //       sect_sfb_offset[g][sect_sfb++] = offset;
      //       offset +=width;
      //    }
      //    sect_sfb_offset[g][sect_sfb] = offset;
      // }

      // allocate tmp memory for sect_sfb_offset
      // size: (num_swb + 1) * num_window_groups
      //  - it's only needed up to scalefactors being applied
      r0 = M[r4 + $aacdec.ics.NUM_SWB_FIELD];
      r0 = r0 + 1;
      r0 = r0 * r7 (int);
      call $aacdec.tmp_mem_pool_allocate;
      if NEG jump $aacdec.possible_corruption;
      M[r4 + $aacdec.ics.SECT_SFB_OFFSET_PTR_FIELD] = r1;
      I3 = r1;

      // g=0
      r6 = 0;
      I1 = r4 + $aacdec.ics.WINDOW_GROUP_LENGTH_FIELD;

      short_sect_sfb_offset_outer_loop:

         // offset=0
         r3 = 0,
          r2 = M[I1, MK1];       // r2 = window_group_length[g];

         r0 = M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD];
         I2 = r0;

         // r0 = swb_offset_short_window[sampling_freq][i+1];
         // r1 = swb_offset_short_window[sampling_freq][i];
         r10 = M[r4 + $aacdec.ics.NUM_SWB_FIELD];
         r0 = M[I2, MK1];
         do short_sect_sfb_offset_inner_loop;
            r1 = r0;
            r0 = M[I2, MK1];
            r1 = r0 - r1;
            r1 = r1 * r2 (int);
            r3 = r3 + r1,
             M[I3, MK1] = r3;
         short_sect_sfb_offset_inner_loop:

         // force last offset to be the end
         r3 = r2 * 128 (int);
         M[I3, MK1] = r3;

         r6 = r6 + 1;
         Null = r6 - r7;
      if NZ jump short_sect_sfb_offset_outer_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
