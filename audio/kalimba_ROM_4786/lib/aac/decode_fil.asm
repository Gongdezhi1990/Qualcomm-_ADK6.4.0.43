// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.decode_fil
//
// DESCRIPTION:
//    Decode the fill element
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
.MODULE $M.aacdec.decode_fil;
   .CODESEGMENT AACDEC_DECODE_FIL_PM;
   .DATASEGMENT DM;

   $aacdec.decode_fil:

   // push rLink onto stack
   push rLink;

   // make sure that sample_rate is known
   // (ie. corrupt frames might get us here with out it being set)
   Null = M[r9 + $aac.mem.SF_INDEX_FIELD];
   if NEG jump $aacdec.possible_corruption;

   // r10 = cnt = amount of fill data
   call $aacdec.get4bits;
   r10 = r1;

   // if (cnt == 15)
   // {
   //    esc_count = getbits(8);
   //    cnt += esc_count - 1;
   // }
   Null = r10 - 15;
   if NZ jump cnt_not_escaped;
      call $aacdec.get1byte;
      r10 = r1 + 14;
   cnt_not_escaped:



#ifdef AACDEC_SBR_ADDITIONS

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r0 + $aacdec.SBR_cnt] = r10;

   // if not fill data then just exit
   if Z jump $pop_rLink_and_rts;

   // sbr.bs_extension_type = getbits(4);
   call $aacdec.get4bits;
   // if(bs_extension_type == EXT_SBR_DATA || EXT_SBR_DATA_CRC)
   Null = r1 - $aacdec.EXT_SBR_DATA;
   if Z jump implicit_sbr_signalling;
      Null = r1 - $aacdec.EXT_SBR_DATA_CRC;
      if NZ jump no_implicit_sbr_signalling;

      implicit_sbr_signalling:

         // set sbr present flag
         r0 = 1;
         M[r9 + $aac.mem.SBR_PRESENT_FIELD] = r0;

         // read in sbr data
         PROFILER_START(&$aacdec.profile_sbr_extension_data)
         call $aacdec.sbr_extension_data;
         Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
         if NZ jump $aacdec.possible_corruption;
         PROFILER_STOP(&$aacdec.profile_sbr_extension_data)

         // pop rLink from stack
         jump $pop_rLink_and_rts;

   no_implicit_sbr_signalling:

      r0 = 4;
      r10 = r10 - 1;
      // skip any fill bytes
      Null = r10 AND 1;
      if Z jump discard_extra_bits;
         r10 = r10 - 1;
         r0 = r0 + 8;
      discard_extra_bits:
      call $aacdec.getbits;
      r10 = r10 LSHIFT -1;
      do fillbyte_pair_loop;
         call $aacdec.get2bytes;
      fillbyte_pair_loop:

      // pop rLink from stack
      jump $pop_rLink_and_rts;

#else
   // No SBR additions enabled so just skip the SBR data

   do fillbyte_loop;
      call $aacdec.get1byte;
   fillbyte_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

#endif


.ENDMODULE;
