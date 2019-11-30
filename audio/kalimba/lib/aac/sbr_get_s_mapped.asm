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
//    $aacdec.sbr_get_s_mapped
//
// DESCRIPTION:
//    Calculate the S_mapped variable
//
// INPUTS:
//    - M2 current_band
//    - r5 current channel (0/1)
//    - r8 envelope 'l'
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r2
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_get_s_mapped;
   .CODESEGMENT AACDEC_SBR_GET_S_MAPPED_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_get_s_mapped:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_GET_S_MAPPED_ASM.SBR_GET_S_MAPPED.PATCH_ID_0, r1)
#endif

   // if(SBR_bs_freq_res[ch][l] == 1)
   r0 = r5 * 6 (int);
   r0 = r0 + r8;
   Words2Addr(r0);
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r1 = r1 + r0;
   r0 = M[r1 + $aacdec.SBR_bs_freq_res];
   Words2Addr(r5);
   Null = r0 - 1;
   if NZ jump low_frequency_resolution;
      //((l >= SBR_l_A[ch]) || ...
         // ... (SBR_bs_add_harmonic_prev[ch][current_band]&&SBR_bs_add_harmonic_flag_prev[ch]))
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      r0 = M[r0 + $aacdec.SBR_l_A];
      Null = r8 - r0;
      if GE jump set_if_add_harmonic_in_curr_band_hi_f_res;
         Addr2Words(r5);
         r0 = r5 * 49 (int);
         r0 = r0 + M2;
         Words2Addr(r0);
         Words2Addr(r5);
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = r1 + r0;
         Null = M[r1 + $aacdec.SBR_bs_add_harmonic_prev];
         if Z jump clear_s_mapped;
            r1 = M[r9 + $aac.mem.SBR_info_ptr];
            r1 = r1 + r5;
            Null = M[r1 + $aacdec.SBR_bs_add_harmonic_flag_prev];
            if Z jump clear_s_mapped;
               set_if_add_harmonic_in_curr_band_hi_f_res:
               // SBR_S_mapped = SBR_bs_add_harmonic[ch][current_band]
               Addr2Words(r5);
               r0 = r5 * 64 (int);
               r0 = r0 + M2;
               Words2Addr(r0);
               Words2Addr(r5);
               r1 = M[r9 + $aac.mem.SBR_info_ptr];
               r1 = r1 + r0;
               r0 = M[r1 + $aacdec.SBR_bs_add_harmonic];
               r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
               M[r1 + $aacdec.SBR_S_mapped] = r0;
               jump exit;

   low_frequency_resolution:

   // lb = first Hi-resolution band in current Low-resolution band
   // lb = (2*current_band) - bitand(SBR_Nhigh, 1)
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_Nhigh];
   r0 = r0 AND 1;
   r2 = M2 + M2;
   r2 = r2 - r0;

   // ub = first Hi-resolution band in next Low-resolution band
   // ub = (2*(current_band+1)) - bitand(SBR_Nhigh, 1)
   r1 = M2 + 1;
   r1 = r1 * 2 (int);
   r1 = r1 - r0;

   // for b=lb:ub-1,
   check_bands_loop:

      // if((l >= SBR_l_A[ch]) || ...
         // (SBR_bs_add_harmonic_prev[ch][b]&&SBR_bs_add_harmonic_flag_prev[ch]))
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      r0 = M[r0 + $aacdec.SBR_l_A];
      Null = r8 - r0;
      if GE jump set_if_add_harmonic_in_curr_band_low_f_res;
         Addr2Words(r5);
         r0 = r5 * 49 (int);
         r0 = r0 + r2;
         Words2Addr(r0);
         Words2Addr(r5);
         push r5;
         r5 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = r0 + r5;
         pop r5;
         Null = M[r0 + $aacdec.SBR_bs_add_harmonic_prev];
         if Z jump eval_loop_index;
            r0 = M[r9 + $aac.mem.SBR_info_ptr];
            r0 = r0 + r5;
            Null = M[r0 + $aacdec.SBR_bs_add_harmonic_flag_prev];
            if Z jump eval_loop_index;
               set_if_add_harmonic_in_curr_band_low_f_res:
               // if(SBR_bs_add_harmonic[ch][b] == 1)
               Addr2Words(r5);
               r0 = r5 * 64 (int);
               r0 = r0 + r2;
               Words2Addr(r0);
               Words2Addr(r5);
               push r5;
               r5 = M[r9 + $aac.mem.SBR_info_ptr];
               r0 = r0 + r5;
               pop r5;
               r0 = M[r0 + $aacdec.SBR_bs_add_harmonic];
               if Z jump eval_loop_index;
                  r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
                  M[r1 + $aacdec.SBR_S_mapped] = r0;
                  jump exit;

   eval_loop_index:
   r2 = r2 + 1;
   Null = r1 - r2;
   if GT jump check_bands_loop;

   clear_s_mapped:
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r1 + $aacdec.SBR_S_mapped] = Null;

   exit:

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
