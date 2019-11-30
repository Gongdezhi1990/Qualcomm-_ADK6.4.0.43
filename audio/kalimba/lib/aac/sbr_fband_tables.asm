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
//    $aacdec.sbr_fband_tables
//
// DESCRIPTION:
//    Calculate 'fTable's that contain frequency borders used by various parts
//    of the decoder
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r5, r6, r10, I1, I2
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_fband_tables;
   .CODESEGMENT AACDEC_SBR_FBAND_TABLES_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_fband_tables:

   // push rLink onto stack
   push rLink;

   r3 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r3 + $aacdec.SBR_Nmaster];
   r1 = M[r3 + $aacdec.SBR_bs_xover_band];

   // SBR_Nhigh = SBR_Nmaster - SBR_bs_xover_band
   r5 = r0 - r1;
   M[r3 + $aacdec.SBR_Nhigh] = r5;

   // SBR_Nlow = (SBR_Nhigh>>1) + (SBR_Nhigh - ((sbr_Nhigh>>1)<<1))
   r2 = r5 ASHIFT -1;
   r3 = r2 ASHIFT  1;

   r3 = r5 - r3;
   r2 = r2 + r3;
   r3 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r3 + $aacdec.SBR_Nlow] = r2;
   M[r3 + $aacdec.SBR_num_env_bands]   = r2;
   M[r3 + $aacdec.SBR_num_env_bands + (1*ADDR_PER_WORD)] = r5;


   // base pointer to SBR_Fmaster[SBR_bs_xover_band]
   I1 = r3 + $aacdec.SBR_Fmaster;
   Words2Addr(r1);
   I1 = I1 + r1;
   // base pointer to SBR_F_table_high
   I2 = r3 + $aacdec.SBR_F_table_high;

   // SBR_Nhigh + 1
   r10 = r5 + 1;

   do f_table_high_loop;
      r0 = M[I1, MK1];
      M[I2, MK1] = r0;
   f_table_high_loop:

   // SBR_M = SBR_F_table_high[SBR_Nhigh] - SBR_F_table_high[0]
   I2 = I2 - MK1;
   r0 = M[I2, 0];

   Words2Addr(r5);
   I2 = I2 - r5;
   Addr2Words(r5);
   r1 = M[I2, 0];

   r0 = r0 - r1;

   M[r3 + $aacdec.SBR_M] = r0;
   // SBR_kx = SBR_F_table_high[0]
   M[r3 + $aacdec.SBR_kx] = r1;


   I1 = r3 + $aacdec.SBR_F_table_low;

   // SBR_F_table_low[0] = SBR_F_table_high[0]
   r0 = M[I2, MK1];
   M[I1, MK1] = r0;

   // r5 = lsb of SBR_Nhigh
   r5 = r5 AND 1;
   if NZ jump high_table_pointer_assigned;  // check that LSHIFT affects the status register
      I2 = I2 + MK1;
   high_table_pointer_assigned:

   // SBR_Nlow
   r10 = r2;

   do f_table_low_loop;
      r0 = M[I2, MK2];
      M[I1, MK1] = r0;
   f_table_low_loop:


   // Nq = max(1, round(SBR_bs_noise_bands*(log2(SBR_k2/SBR_kx)) + 0.5))
   Words2Addr(r1);
   r10 = r2;

   r0 = r1 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
   call $aacdec.sbr_read_one_word_from_flash;
   r5 = r0;

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_k2];
   Words2Addr(r0);
   r0 = r0 + (&$aacdec.sbr_log_base2_table - (1*ADDR_PER_WORD));
   call $aacdec.sbr_read_one_word_from_flash;

   r0 = r0 - r5;
   r0 = r0 LSHIFT (DAWTH-16);

   r2 = r10;

   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r3 = M[r1 + $aacdec.SBR_bs_noise_bands];
   r3 = r3 ASHIFT (DAWTH-3);

   r3 = r3 * r0 (frac);
   // scale back down and round
   r3 = r3 * (1.0/(1<<(DAWTH-6))) (frac);

   // set SBR_Nq = max(SBR_Nq, 1)
   r0 = r3 - 1;
   if LT r3 = r3 - r0;

   // set SBR_Nq = min(5, SBR_Nq)
   r0 = r3 - 5;
   if GT r3 = r3 - r0;
   M[r1 + $aacdec.SBR_Nq] = r3;

   I1 = r1 + $aacdec.SBR_F_table_noise;
   r1 = r1 + $aacdec.SBR_F_table_low;
   // SBR_F_table_noise[0] = SBR_F_table_low[0]
   r0 = M[r1];
   M[I1, MK1] = r0;

   // SBR_Nq
   r10 = r3;

   // j = 0
   r0 = 0;

   do f_table_noise_loop;
      // j += round((SBR_Nlow - j)/(SBR_Nq-index))
      r5 = r2 - r0;
      r5 = r5 ASHIFT 16;
      Words2Addr(r10);
      r6 = M[r9 + $aac.mem.SBR_one_over_x_ptr];
      r6 = r6 - (1*ADDR_PER_WORD);
      r6 = M[r6 + r10];
      Addr2Words(r10);
      r5 = r5 * r6 (frac);
      r5 = r5 ASHIFT -16;

      r0 = r0 + r5;
      Words2Addr(r0);

      // SBR_F_table_noise[index] = SBR_F_table_low[j]
      r5 = M[r1 + r0];
      Addr2Words(r0);
      M[I1, MK1] = r5;
   f_table_noise_loop:

   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r5 = r1 + $aacdec.SBR_table_map_k_to_g;
   // k
   r2 = 0;

   map_k_to_g_outer_loop:

      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      I1 = r1 + $aacdec.SBR_F_table_noise;
      r1 = M[I1, MK1];

      // SBR_Nq
      r10 = r3;

      // for g=0:SBR_Nq-1,
      do map_k_to_g_inner_loop;

         Null = r1 - r2;
         if GT jump dont_map;
            r1 = M[I1, 0];
            Null = r1 - r2;
            if LE jump dont_map;
               // SBR_table_map_k_to_g[k] = g
               r1 = r3 - r10;
               Words2Addr(r2);
               M[r5 + r2] = r1;
               Addr2Words(r2);
               jump map_k_to_g_inner_loop_done;
         dont_map:

         r1 = M[I1, MK1];
      map_k_to_g_inner_loop:

      map_k_to_g_inner_loop_done:

      // k += 1
      r2 = r2 + 1;
   Null = r2 - 64;
   if LT jump map_k_to_g_outer_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
