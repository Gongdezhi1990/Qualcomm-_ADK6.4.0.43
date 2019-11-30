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
//    $aacdec.sbr_fmaster_table_calc_fscale_gt_zero
//
// DESCRIPTION:
//    Calculate the fMaster table when 'bs_freq_scale' > 0
//
// INPUTS:
//    - r2 SBR_k2
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5-r7, r10, rMAC, I1-I4, Div
//    - $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_fmaster_table_calc_fscale_gt_zero;
   .CODESEGMENT AACDEC_SBR_FMASTER_TABLE_CALC_FSCALE_GT_ZERO_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_fmaster_table_calc_fscale_gt_zero:

   // push rLink onto stack
   push rLink;

   // if( (k2/k0) > 2.2449 )
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_k0];
   r1 = r0 ASHIFT (DAWTH-8);
   r1 = r1 * 0.561225 (frac);
   r6 = r2 ASHIFT (DAWTH-10);
   Null = r6 - r1;
   if GT jump two_regions;
      // twoRegions = 0
      M[r9 + $aac.mem.TMP + $aacdec.SBR_two_regions] = Null;
      // SBR_k1 = SBR_k2
      r1 = r2;
      jump save_k1;
   two_regions:
      // twoRegions = 1
      r1 = 1;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_two_regions] = r1;
      // SBR_k1 = SBR_k0 * 2
      r1 = r0 * 2 (int);

   save_k1:
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r0 + $aacdec.SBR_k1] = r1;
   Words2Addr(r1);

   r0 = r1 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
   call $aacdec.sbr_read_one_word_from_flash;
   r6 = r0 LSHIFT (DAWTH-16);

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_k0];
   Words2Addr(r0);
   r0 = r0 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
   call $aacdec.sbr_read_one_word_from_flash;
   r3 = r0 LSHIFT (DAWTH-16);

   // r5 = SBR_bands = temp_1(SBR_bs_freq_scale-1)
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_bs_freq_scale];
   r2 = r6 - r3;
   Words2Addr(r0);
   r6 = M[r9 + $aac.mem.SBR_fscale_gt_zero_temp_1_ptr];
   r6 = r6 - (1*ADDR_PER_WORD);
   r5 = M[r6 + r0];
   r0 = r2;
   r6 = M[r9 + $aac.mem.SBR_info_ptr];
   r6 = M[r6 + $aacdec.SBR_k0];
   r3 = r9 + $aac.mem.TMP + $aacdec.SBR_vDk0_length;

   call $aacdec.form_vDkx;
   Null = M[r9 + $aac.mem.FRAME_CORRUPT];
   if NZ jump $aacdec.corruption;

   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk0_length];
   // SBR_numBands0 + 1
   r5 = r0 + 1;

   call $aacdec.sbr_bubble_sort;

   // base pointer to SBR_vDk0
   I2 = r1;
   I3 = r1 - (1*ADDR_PER_WORD);

   r0 = r5;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.corruption;

   // base pointer to SBR_vk0
   I1 = r1;
   I4 = r1;

   // SBR_vk0(0) = SBR_k0
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_k0];
   M[I1, MK1] = r0;

   r10 = r5 - 1;

   do sbr_vk0_loop;
      r2 = M[I2, MK1];

      r0 = r0 + r2;
      M[I1, MK1] = r0;
   sbr_vk0_loop:



   // if(twoRegions == 1)
   // r0 = M[$aacdec.sbr_info + $aacdec.SBR_two_regions];
   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_two_regions];
   Null = r0;
   if Z jump not_two_regions;


      // r5 = SBR_bands = temp_1(SBR_bs_freq_scale-1)
      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = M[r1 + $aacdec.SBR_bs_freq_scale];
      Words2Addr(r0);
      r6 = M[r9 + $aac.mem.SBR_fscale_gt_zero_temp_1_ptr];
      r6 = r6 - (1*ADDR_PER_WORD);
      r5 = M[r6 + r0];

      // r6 = 1/SBR_warp = temp_2(SBR_bs_alter_scale)
      r0 = M[r1 + $aacdec.SBR_bs_alter_scale];
      Words2Addr(r0);
      r1 = M[r9 + $aac.mem.SBR_fscale_gt_zero_temp_2_ptr];
      r6 = M[r1 + r0];

      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r1 = M[r1 + $aacdec.SBR_k2];
      Words2Addr(r1);

      r0 = r1 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
      call $aacdec.sbr_read_one_word_from_flash;
      r10 = r0;

      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r0 + $aacdec.SBR_k1];
      Words2Addr(r0);
      r0 = r0 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
      call $aacdec.sbr_read_one_word_from_flash;

      r2 = r10 LSHIFT (DAWTH-16);
      r3 = r0 LSHIFT (DAWTH-16);

      r2 = r2 - r3;
      r0 = r2 * r6 (frac);
      r6 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r6 = M[r6 + $aacdec.SBR_k1];
      r3 = r9 + $aac.mem.TMP + $aacdec.SBR_vDk1_length;

      call $aacdec.form_vDkx;
      Null = M[r9 + $aac.mem.FRAME_CORRUPT];
      if NZ jump $aacdec.corruption;



      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk1_length];
      // r5 = SBR_numBands1 - 1
      r5 = r0 - 1;

      call $aacdec.sbr_bubble_sort;

      // r0 = max(SBR_vDk0[])
      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk0_length];
      Words2Addr(r0);
      I3 = I3 + r0;
      r0 = M[I3, 0];

      // r2 = min(SBR_vDk1[])
      r2 = M[r1];

      // r0 = change = max(SBR_vDk0[]) - min(SBR_vDk1[])
      r0 = r0 - r2;
      if LE jump no_vDk1_change;
         // r3 = (SBR_vDk1[SBR_numBands1-1] - SBR_vDk1[0]) / 2
         Words2Addr(r5);
         r3 = M[r1 + r5];
         r6 = r3 - r2;
         r6 = r6 ASHIFT -1;
         // change = min(change, r3)
         r6 = r0 - r6;
         if GT r0 = r0 - r6;
         // SBR_vDk1[0] += change
         M[r1] = r2 + r0;
         // SBR_vDk1[SBR_numBands1-1] -= change
         r0 = r3 - r0;
         M[r1 + r5] = r0;
         Addr2Words(r5);
      no_vDk1_change:

      // r0 = SBR_numBands1
      r0 = r5 + 1;
      call $aacdec.sbr_bubble_sort;


      // base pointer to SBR_vDk1
      I2 = r1;

      // SBR_numBands1 + 1
      r0 = r5 + 2;
      call $aacdec.tmp_mem_pool_allocate;
      if NEG jump $aacdec.corruption;

      // base pointer to SBR_vk1
      I1 = r1;

      // SBR_vk1[0] = SBR_k1
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r0 + $aacdec.SBR_k1];
      M[I1, MK1] = r0;

      // SBR_numBands1
      r10 = r5 + 1;

      do sbr_vk1_loop;
         r2 = M[I2, MK1];
         r0 = r0 + r2;
         M[I1, MK1] = r0;
      sbr_vk1_loop:

      // SBR_numBands0 + 1
      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk0_length];
      r0 = r0 + 1;

      // Nmaster = SBR_numBands0 + SBR_numBands1
      r5 = r5 + r0;
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r2 + $aacdec.SBR_Nmaster] = r5;

      // base pointer to SBR_Fmaster
      I1 = r2 + $aacdec.SBR_Fmaster;

      // SBR_Fmaster[0:SBR_numBands0] = SBR_vk0[0:SBR_numBands0]
      r10 = r0;

      do two_regions_fmaster_loop_1;
         r2 = M[I4, MK1];
         M[I1, MK1] = r2;
      two_regions_fmaster_loop_1:

      // pointer to SBR_vk1[1]
      I2 = r1 + (1*ADDR_PER_WORD);

      // SBR_Fmaster[SBR_numBands0:SBR_Nmaster-1] = SBR_vk1[1:SBR_numBands1-2]
      r10 = r5 - r0;
      r10 = r10 + 1;

      do two_regions_fmaster_loop_2;
         r2 = M[I2, MK1];
         M[I1, MK1] = r2;
      two_regions_fmaster_loop_2:


      // free temporary memory allocated for SBR_vk0[], SBR_vDk0[], SBR_vk1[], SBR_vDk1[]
      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk0_length];
      r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk1_length];
      r0 = r0 + r1;
      r0 = r0 * 2 (int);
      r0 = r0 + 2;
      call $aacdec.tmp_mem_pool_free;

      // pop rLink from stack
      jump $pop_rLink_and_rts;



   not_two_regions:


      // SBR_Nmaster = SBR_numBands0
      r0 = r5 - 1;
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r2 + $aacdec.SBR_Nmaster] = r0;

      // base pointer to SBR_vk0
      I1 = r1;
      // base pointer to SBR_Fmaster
      I2 = r2 + $aacdec.SBR_Fmaster;


      // SBR_numBands0 + 1
      r10 = r5;

      do not_two_regions_fmaster_loop;
         r0 = M[I1, MK1];
         M[I2, MK1] = r0;
      not_two_regions_fmaster_loop:


      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_vDk0_length];
      r0 = r0 * 2 (int);
      r0 = r0 + 1;
      call $aacdec.tmp_mem_pool_free;


      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $aacdec.form_vDkx
//
// DESCRIPTION:
//    form the vDk0 or vDk1 arrays
//
// INPUTS:
//    - r0 = r2 when x==0 or r2/SBR_warp when x==1
//    - r2 = log2(SBR_ky/SBR_kx)/8
//    - r3 = pointer to place to store vDkx length
//    - r5 = SBR_numBands
//    - r6 = SBR_kx"
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5-r7, r10, rMAC, I1-I4, Div
//    - $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.form_vDkx;
   .CODESEGMENT AACDEC_FORM_VDKX_PM;
   .DATASEGMENT DM;

   $aacdec.form_vDkx:

   push rLink;

   r5 = r5 ASHIFT (16 + $aacdec.SBR_LOG_NATURAL_SHIFT_AMOUNT - 1);
   r5 = r5 * r0 (frac);
   r5 = r5 * (1.0/65536) (frac);
   r5 = r5 * 2 (int);

   // q = (SBR_k2 / SBR_k1)^(1.0 / SBR_numBands1)
   rMAC = r2 ASHIFT (-DAWTH+24) (LO);

   Div = rMAC / r5;
   r1 = DivResult;

   r0 = r1 LSHIFT 3;  // multiply by 8
   r0 = r0 AND (DAWTH_MASK-0xFF);   // Round the input to the 2^x code


   // -- Calculates 2^(r0) where r0 is a +ve fractional number --

   // rMAC = table index
   r1 = r0 LSHIFT -(24-$aacdec.SBR_POW2_TABLE_SHIFT_AMOUNT);
   Words2Addr(r1);

   // get interpolation points
   r10 = M[r9 + $aac.mem.SBR_pow2_table_ptr];
   rMAC = M[r10 + r1];
   r10 = r10 + (1*ADDR_PER_WORD);
   r1 = M[r10 + r1];
#ifdef KAL_ARCH4
   // Keep K32 and K24 bit exact otherwise compliancy test results diverge
   r1 = r1 * (1.0/256) (frac);
   r1 = r1 LSHIFT 8;
   rMAC = rMAC * (1.0/256) (frac);
   rMAC = rMAC LSHIFT 8;
   r0 = r0 LSHIFT 8;
#endif

   r1 = r1 - rMAC;

   // do interpolation
   r0 = r0 LSHIFT ($aacdec.SBR_POW2_TABLE_SHIFT_AMOUNT);
   r0 = r0 LSHIFT -1;
   rMAC = rMAC + r0 * r1;

#ifdef KAL_ARCH4
   // Keep K32 and K24 bit exact otherwise compliancy test results diverge
   rMAC = rMAC * (1.0/256) (frac);
   rMAC = rMAC LSHIFT 8;
#endif

   // -- Allocate and form vDkx buffer --

   // SBR_numBands1
   r10 = r5;

   // r5 = r2 = q_0 = SBR_k1 (fractional representation)
   r5 = r6 LSHIFT (DAWTH-8);
   r7 = r6;  // (integer representation)

   // allocate temporary memory for SBR_vDkx
   r0 = r10;
   M[r3] = r0;
   call $aacdec.tmp_mem_pool_allocate;
   if NEG jump $aacdec.corruption;

   // index into SBR_vDkx
   I2 = r1;

   do vDkx_loop;
      // q_(k-1) = q_k  (integer representation)
      r0 = r7;

      r5 = r5 * rMAC (frac);
#ifdef KAL_ARCH4
      // Keep K32 and K24 bit exact otherwise compliancy test results diverge
      r5 = r5 * (1.0/256) (frac);
      r5 = r5 LSHIFT 8;
#endif

      // round and convert to integer representation
      r7 = r5 * (1.0/(1<<(DAWTH-9))) (frac);
      r5 = r5 * 2 (int);

      // SBR_vDk0(k) = q_k - q_(k-1) (integer representation)
      r2 = r7 - r0;
      M[I2, MK1] = r2;
   vDkx_loop:

   jump $pop_rLink_and_rts;


.ENDMODULE;

#endif
