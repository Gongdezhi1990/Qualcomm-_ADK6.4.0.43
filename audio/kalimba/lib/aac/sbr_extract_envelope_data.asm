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
//    $aacdec.sbr_extract_envelope_data
//
// DESCRIPTION:
//    Calculate envelope information
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r4, r6-r8, r10, I1-I5, M2
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_extract_envelope_data;
   .CODESEGMENT AACDEC_SBR_EXTRACT_ENVELOPE_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_extract_envelope_data:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5); // In all code below r5 is a channel address offset

   // delta = 1;
   r6 = 1;
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = r5;
   if Z jump delta_assigned;
      Null = M[r1 + $aacdec.SBR_bs_coupling];
      if NZ r6 = r6 + r6;
   delta_assigned:

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   r0 = M[r0 + $aacdec.SBR_E_envelope_base_ptr];
   I2 = r0;

   r4 = M[r1 + $aacdec.SBR_Nlow];
   r8 = M[r1 + $aacdec.SBR_Nhigh];
   r3 = r5 * 5 (int);
   r7 = r5 * 6 (int);

   M2 = 0;
   // for l=0:SBR_bs_num_env[ch]-1,
   outer_loop:

      // if(SBR_bs_df_env[ch][l]==0)
      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r1 = r1 + r3;
      Null = M[r1 + $aacdec.SBR_bs_df_env];
      if NZ jump not_delta_freq_coding_in_this_env;
         // r1 = SBR_bs_freq_res[ch][l]
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = r1 + r7;
         r1 = M[r1 + $aacdec.SBR_bs_freq_res];
         Words2Addr(r1);

         Null = r6 - 1;
         if Z jump dont_shift_e_envelope;
            I1 = I2;
            I4 = I2;

            r0 = M[r9 + $aac.mem.SBR_info_ptr];
            r0 = r0 + r1;
            r10 = M[r0 + $aacdec.SBR_num_env_bands];
            r10 = r10 - 1;

            r0 = M[I1, MK1];

            do shift_e_envelope_loop;
               r0 = r0 LSHIFT 1;
               M[I4, MK1] = r0,
                r0 = M[I1, MK1];
            shift_e_envelope_loop:

            r0 = r0 LSHIFT 1;
            M[I4, MK1] = r0;
         dont_shift_e_envelope:

         I1 = I2 + MK1;
         I4 = I2;

         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = r0 + r1;
         r10 = M[r0 + $aacdec.SBR_num_env_bands];
         r10 = r10 - 1;

         do e_envelope_add_previous_loop;
            // SBR_E_envelope[ch][k][l] += SBR_E_envelope[ch][k-1][l]
            r0 = M[I4, MK1],
             r1 = M[I1, 0];
            // if(SBR_E_envelope[ch][k][l]<0) SBR_E_envelope[ch][k][l] = 0
            r1 = r1 + r0;
            if NEG r1 = 0;

            M[I1, MK1] = r1;
         e_envelope_add_previous_loop:

         jump eval_loop_index;
      not_delta_freq_coding_in_this_env:

      // r1 = SBR_bs_freq_res[ch][l]
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = r0 + r7;
      r1 = M[r0 + $aacdec.SBR_bs_freq_res];
      // g = SBR_bs_freq_res[ch][l-1]
      r0 = M[r0 + $aacdec.SBR_bs_freq_res - (1*ADDR_PER_WORD)];

      Null = M2;
      if NZ jump this_is_not_first_envelope;
         // g = SBR_freq_res_prev[ch]
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = r0 + r5;
         r0 = M[r0 + $aacdec.SBR_freq_res_prev];
      this_is_not_first_envelope:

      // if(SBR_bs_freq_res[ch][l]==g)
      Null = r1 - r0;
      if NZ jump g_ne_bs_freq_res;

         Words2Addr(r1);
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = r0 + r1;
         r10 = M[r0 + $aacdec.SBR_num_env_bands];
         Null = M2;
         if Z jump this_is_first_envelope_g_eq_bs_freq_res;
            Words2Addr(r8);
            I4 = I2 - r8;
            Addr2Words(r8);
            jump envelope_pointer_assigend_g_eq_bs_freq_res;
         this_is_first_envelope_g_eq_bs_freq_res:
            r0 = r5 * 49 (int);
            r1 = M[r9 + $aac.mem.SBR_info_ptr];
            r1 = r1 + r0;
            I4 = r1 + $aacdec.SBR_E_prev;
         envelope_pointer_assigend_g_eq_bs_freq_res:

         I1 = I2;

         do bs_freq_res_eq_g_e_env_loop;
            r0 = M[I1, 0];
            r0 = r0 * r6 (int),
             r1 = M[I4, MK1];
            r0 = r0 + r1;
            M[I1, MK1] = r0;
         bs_freq_res_eq_g_e_env_loop:

         jump eval_loop_index;
      // elsif((g == 1) && (SBR_bs_freq_res[ch][l] == 0))
      g_ne_bs_freq_res:
      Null = r0 - 1;
      if NZ jump g_ne_1_or_freq_res_ne_0;
         Null = r1;
         if NZ jump g_ne_1_or_freq_res_ne_0;
            // r2 = outer loop bound
            r2 = r4;
            I1 = I2;

            // for k=0:{SBR_Nhigh|SBR_Nlow}
            r0 = M[r9 + $aac.mem.SBR_info_ptr];
            I5 = r0 + $aacdec.SBR_F_table_low;
            outer_loop_g_eq_one_low_freq_res:
               Null = M2;
               if Z jump this_is_first_envelope_g_eq_one_low_freq_res;
               Words2Addr(r8);
               I4 = I2 - r8;
               Addr2Words(r8);
                  jump envelope_pointer_assigned_g_eq_one_low_freq_res;
               this_is_first_envelope_g_eq_one_low_freq_res:
                  r0 = r5 * 49 (int);
                  r1 = M[r9 + $aac.mem.SBR_info_ptr];
                  r1 = r1 + r0;
                  I4 = r1 + $aacdec.SBR_E_prev;
               envelope_pointer_assigned_g_eq_one_low_freq_res:

               // for i=0:SBR_Nhigh-1,
               r10 = r8;
               r0 = M[r9 + $aac.mem.SBR_info_ptr];
               I3 = r0 + $aacdec.SBR_F_table_high;
               do g_eq_one_low_freq_res_e_env_loop;
                  // if(SBR_F_TableHigh[i]==SBR_F_TableLow[k])
                  r0 = M[I3, MK1],
                   r1 = M[I5, 0];
                  Null = r0 - r1;
                  if NZ jump not_same_frequency_border;
                     // SBR_E_envelope[ch][k][l] = G_E + (delta * SBR_E_envelope[ch][k][l])
                     r0 = M[I1, 0];
                     r0 = r0 * r6 (int),
                      r1 = M[I4, 0]; // r1 = G_E
                     r0 = r0 + r1;
                     M[I1, 0] = r0;
                  not_same_frequency_border:
                  I4 = I4 + MK1;
               g_eq_one_low_freq_res_e_env_loop:

               I1 = I1 + MK1;
               I5 = I5 + MK1;

               r2 = r2 - 1;
            if GT jump outer_loop_g_eq_one_low_freq_res;

            jump eval_loop_index;
      // elsif((g == 0) && (SBR_bs_freq_res[ch][l] == 1))
      g_ne_1_or_freq_res_ne_0:
      Null = r0;
      if NZ jump eval_loop_index;
         Null = r1 - 1;
         if NZ jump eval_loop_index;
            // r2 = outer loop bound
            r2 = r8;
            I1 = I2;

            // for k=0:{SBR_Nhigh|SBR_Nlow}
            r0 = M[r9 + $aac.mem.SBR_info_ptr];
            I5 = r0 + $aacdec.SBR_F_table_high;
            outer_loop_g_eq_zero_high_freq_res:

               Null = M2;
               if Z jump this_is_first_envelope_g_eq_zero_high_freq_res;
                  Words2Addr(r8);
                  I4 = I2 - r8;
                  Addr2Words(r8);
                  jump envelope_pointer_assigned_g_eq_zero_high_freq_res;
               this_is_first_envelope_g_eq_zero_high_freq_res:
                  r0 = r5 * 49 (int);
                  r1 = M[r9 + $aac.mem.SBR_info_ptr];
                  r1 = r1 + r0;
                  I4 = r1 + $aacdec.SBR_E_prev;
               envelope_pointer_assigned_g_eq_zero_high_freq_res:

               // for i=0:SBR_Nhigh-1,
               r10 = r4;
               r0 = M[r9 + $aac.mem.SBR_info_ptr];
               I3 = r0 + $aacdec.SBR_F_table_low;
               do g_eq_zero_high_freq_res_e_env_loop;
                  // if((SBR_F_TableLow[i] <= SBR_F_TableHigh[k])...
                     // && (SBR_F_TableHigh[k] < SBR_F_TableLow[i+1]))
                  r0 = M[I3, MK1],
                   r1 = M[I5, 0];
                  Null = r0 - r1;
                  if GT jump h_res_border_n_between_adj_l_res_borders;
                     r0 = M[I3, 0];
                     Null = r1 - r0;
                     if GE jump h_res_border_n_between_adj_l_res_borders;
                        // SBR_E_envelope[ch][k][l] = G_E + (delta * SBR_E_envelope[ch][k][l])
                        r0 = M[I1, 0];
                        r0 = r0 * r6 (int),
                         r1 = M[I4, 0]; // r1 = G_E
                        r0 = r0 + r1;
                        M[I1, 0] = r0;
                  h_res_border_n_between_adj_l_res_borders:

                  I4 = I4 + MK1;
               g_eq_zero_high_freq_res_e_env_loop:

               I1 = I1 + MK1;
               I5 = I5 + MK1;

               r2 = r2 - 1;
            if GT jump outer_loop_g_eq_zero_high_freq_res;

      eval_loop_index:
      Words2Addr(r8);
      I2 = I2 + r8;
      Addr2Words(r8);

      r3 = r3 + MK1;
      r7 = r7 + MK1;
      M2 = M2 + 1;
      r0 = r5 + r9;
      r0 = M[$aac.mem.SBR_bs_num_env + r0];
      Null = r0 - M2;
   if GT jump outer_loop;



   // SBR_E_prev[ch][0:SBR_Nhigh-1] = SBR_E_envelope[ch][0:SBR_Nhigh-1][SBR_bs_num_env[ch]-1]
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   r0 = M[r0 + $aacdec.SBR_E_envelope_base_ptr];
   r1 = M2 - 1;
   r1 = r8 * r1 (int);
   Words2Addr(r1);
   I4 = r0 + r1;

   r1 = r5 * 49 (int);
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = r0 + r1;
   I1 = r0 + $aacdec.SBR_E_prev;

   r10 = r8 - 1;
   r2 = M[I4, MK1];

   do e_prev_loop;
      M[I1, MK1] = r2,
       r2 = M[I4, MK1];
   e_prev_loop:

   M[I1, MK1] = r2;

   Addr2Words(r5); // In all code above used r5 is a channel address offset

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
