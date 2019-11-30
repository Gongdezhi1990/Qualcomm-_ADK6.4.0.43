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
//    $aacdec.sbr_fmaster_table_calc_fscale_eq_zero
//
// DESCRIPTION:
//    Calculate the fMaster table when 'bs_freq_scale' = 0
//
// INPUTS:
//    - r2 SBR_k2
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5, r6, r10, rMAC, I1, I2, M1
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_fmaster_table_calc_fscale_eq_zero;
   .CODESEGMENT AACDEC_SBR_FMASTER_TABLE_CALC_FSCALE_EQ_ZERO_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_fmaster_table_calc_fscale_eq_zero:

   // push rLink onto stack
   push rLink;

   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r1 + $aacdec.SBR_k0];
   r3 = r2 - r0;

   r1 = M[r1 + $aacdec.SBR_bs_alter_scale];
   if NZ jump bs_alter_scale_not_zero;
      // dk
      r1 = -1;
      jump end_if_bs_alter_scale;
   bs_alter_scale_not_zero:
      // dk
      r1 = -2;
      r3 = r3 + 2;
   end_if_bs_alter_scale:


   r3 = r3 ASHIFT r1;
   r3 = r3 ASHIFT 1;

   // SBR_numBands = min(r3, 63);
   r5 = r3 - 63;
   if GT r3 = r3 - r5;

   r5 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r5 + $aacdec.SBR_Nmaster] = r3;
   rMAC = -r1;
   // SBR_k2Achieved
   r5 = r3 * rMAC (int);
   r5 = r5 + r0;

   // SBR_k2Diff
   r5 = r2 - r5;

   r10 = r3;
   r0 = r3;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;
   // base pointer to SBR_vDk[]
   I1 = r1;

   // SBR_vDk[0:SBR_numBands-1] = SBR_dk;
   do vdk_loop;
      M[I1, MK1] = rMAC;
   vdk_loop:


   Null = r5;
   if Z jump f_master_loop_section;
      if GT jump k2diff_gt_zero;
         // incr = 1
         M1 = 1;
         // k = 0
         r6 = 0;

         jump end_if_k2diff;
      k2diff_gt_zero:
         // incr = -1
         M1 = -1;
         // k = SBR_numBands - 1;
         r6 = r3 - 1;

      end_if_k2diff:

      // pointer to SBR_vDK[k]
      Words2Addr(r6);
      I1 = r1 + r6;

      k2diff_loop:
         r0 = M[I1, 0];
         r0 = r0 - M1;
         M[I1, M1] = r0;

         // SBR_k2Diff = SBR_k2Diff + incr
         r5 = r5 + M1;
      if NZ jump k2diff_loop;
   f_master_loop_section:

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   I2 = r0 + $aacdec.SBR_Fmaster;
   // SBR_Fmaster[0] = SBR_k0
   r0 = M[r0 + $aacdec.SBR_k0];
   M[I2, MK1] = r0;

   I1 = r1;

   r10 = r3;


   do f_master_loop;
      // SBR_Fmaster[1:SBR_numBands] = SBR_Fmaster[0:SBR_numBands-1] ...
      //                               + SBR_vDk[0:SBR_numBands-1]
      r1 = M[I1, MK1];
      r0 = r0 + r1;
      M[I2, MK1] = r0;
   f_master_loop:


   // free temporary memory allocated for SBR_vDk[]
   r0 = r3;
   call $aacdec.tmp_mem_pool_free;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
