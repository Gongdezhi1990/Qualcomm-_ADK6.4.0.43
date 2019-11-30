// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_SBR_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.sbr_extension_data
//
// DESCRIPTION:
//    read in data and calculate tables used in SBR decoding process
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - all including $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_extension_data;
   .CODESEGMENT AACDEC_SBR_EXTENSION_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_extension_data:

   // push rLink onto stack
   push rLink;

   // store the number of bits read so far so that later we can calculate how
   // much sbr data has been read
   r0 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_bit_count] = r0;


   // if(SBR_bs_extension_type == EXT_SBR_DATA_CRC)
   Null = r1 - $aacdec.EXT_SBR_DATA_CRC;
   if NZ jump not_crc_data_present;
      // read SBR_num_crc_bits
      r0 = 10;
      call $aacdec.getbits;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_num_crc_bits] = r1;
   not_crc_data_present:

   call $aacdec.get1bit;

   // if(SBR_bs_header_flag)
   if NZ call $aacdec.sbr_header;

   call $aacdec.sbr_reset;

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = M[r0 + $aacdec.SBR_header_count];
   if Z jump end_if_header_count;
      // calculate tables used in SBR decoding process
      Null = M[r0 + $aacdec.SBR_reset];
      if Z jump dont_calc_new_tables;
         PROFILER_START(&$aacdec.profile_sbr_calc_tables)
         call $aacdec.sbr_calc_tables;
         PROFILER_STOP(&$aacdec.profile_sbr_calc_tables)
         Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
         if NZ jump $aacdec.possible_corruption;
      dont_calc_new_tables:

      // if(single_channel_element)
      r0 = M[r9 + $aac.mem.NUM_SCEs];
      if Z jump not_single_channel_element;
         call $aacdec.sbr_single_channel_element;
         jump end_if_header_count;
      not_single_channel_element:

      // elseif(channel_pair_element)
      r0 = M[r9 + $aac.mem.NUM_CPEs];
      if Z jump end_if_header_count;
         PROFILER_START(&$aacdec.profile_sbr_channel_pair_element)
         call $aacdec.sbr_channel_pair_element;
         PROFILER_STOP(&$aacdec.profile_sbr_channel_pair_element)

   end_if_header_count:


   // r0 = number of sbr bits that should be read
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = M[r1 + $aacdec.SBR_cnt];
   r0 = r0 * 8 (int);

   // r2 = number of sbr bits actually read so far
   r2 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r1 = M[r1 + $aacdec.SBR_bit_count];
   r2 = r2 - r1;
#ifdef AACDEC_PS_ADDITIONS
   r2 = r2 + 4;
#endif

   Null = r0 - r2;
   if LT jump $aacdec.possible_corruption;

   // r1 = SBR_num_align_bits = (SBR_cnt*8) - 4 - SBR_num_bits
#ifdef AACDEC_PS_ADDITIONS
   r1 = r2;
#else
   r1 = r2 + 4;
#endif
   r1 = r0 - r1;

   // r10 = no. bytes to discard
   r10 = r1 ASHIFT -3;

   // discard bits before the remaining bytes
   r0 = r10 ASHIFT 3;
   r0 = r1 - r0;
   call $aacdec.getbits;

   // discard bytes

   do discard_align_bytes_loop;
      call $aacdec.get1byte;
   discard_align_bytes_loop:

   exit:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
