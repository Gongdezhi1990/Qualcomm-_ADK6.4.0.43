// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_ELD_ADDITIONS

// *****************************************************************************
// MODULE:
// $aacdec.eld_sbr_data
//
// DESCRIPTION:
// (i)  If the "bs_header_flag" is set then call "sbr_header"
// (ii) Call the sbr_single_channel_element/sbr_channel_pair_element function based on
// the value of id_aac
//
// INPUTS:
// r4 - id_aac
//
// OUTPUTS:
//    -  None
// TRASHED REGISTERS:
//    - All
//
// *****************************************************************************
.MODULE $aacdec.eld_sbr_data;
   .CODESEGMENT AACDEC_LD_SBR_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.eld_sbr_data:

   push rLink;
   push r4;
   // *****************************************************************************
   // extract bs_sbr_crc_bits (10)
   // *****************************************************************************
   r0 = 10;
   r2 = M[r9 +  $aac.mem.ELD_sbr_crc_flag];
   if NZ call $aacdec.getbits;          // output is in r1
   r5 = r1;                             // bs_sbr_crc_bits
   // *****************************************************************************
   // extract bs_header_flag (1)
   // *****************************************************************************
   call $aacdec.get1bit;               // output is in r1
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


   // *****************************************************************************
   // call the appropriate function based on id_aac
   // *****************************************************************************
   end_if_header_count:
   pop r4;
   Null = r4 - $aacdec.ID_SCE;
   if NZ jump id_cpe_check;
   call $aacdec.sbr_single_channel_element;
   jump low_delay_sbr_data_done;

   id_cpe_check:
      Null = r4 - $aacdec.ID_CPE;
      if Z call $aacdec.sbr_channel_pair_element;

   low_delay_sbr_data_done:
      jump $pop_rLink_and_rts;

.ENDMODULE;

#endif

