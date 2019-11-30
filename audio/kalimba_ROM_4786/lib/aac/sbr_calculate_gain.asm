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
//    $aacdec.sbr_calculate_gain
//
// DESCRIPTION:
//    Calculate the gains required by hf_assembly
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r4, r6-r8, r10, rMAC, I0-I7, M0-M3, Div
//    - $aacdec.tmp
//    - L4, L5 set to zero
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_calculate_gain;
   .CODESEGMENT AACDEC_SBR_CALCULATE_GAIN_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_calculate_gain:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_CALCULATE_GAIN_ASM.SBR_CALCULATE_GAIN.PATCH_ID_0, r1)
#endif
   
   Words2Addr(r5);
   // current_t_noise_band = 0
   M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_t_noise_band] = Null;

   r2 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r2 + $aacdec.SBR_bs_limiter_bands];
   r1 = M[r2 + $aacdec.SBR_Nlow];
   r1 = r1 + 5;
   r0 = r0 * r1 (int);
   Words2Addr(r0);
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = M[r1 + $aacdec.SBR_F_table_lim_base_ptr];
   r0 = r0 + r1;
   // r1 = SBR_N_L[SBR_bs_limiter_bands]
   r1 = M[r2 + $aacdec.SBR_bs_limiter_bands];
   Words2Addr(r1);
   r3 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r3 = r3 + r1;
   r1 = M[r3 + $aacdec.SBR_N_L];
   // r0 = SBR_F_table_lim[SBR_bs_limiter_bands][SBR_N_L[SBR_bs_limiter_bands]]
   Words2Addr(r1);
   r3 = M[r0 + r1];

   M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope] = r3;

   r1 = r9 + r5;
   r1 = M[r1 + $aac.mem.SBR_bs_num_env];
   r0 = r3 * r1 (int);
   push r3;

   // allocate space in temporary memory for SBR_G_lim_boost_mantissa[][]
   call $aacdec.frame_mem_pool_allocate;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_G_lim_boost_mantissa_ptr] = r1;

   // allocate space in temporary memory for SBR_Q_M_lim_boost_mantissa[][]
   call $aacdec.frame_mem_pool_allocate;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr] = r1;

   // allocate space in temporary memory for SBR_S_M_boost_mantissa[][]
   call $aacdec.frame_mem_pool_allocate;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_S_M_boost_mantissa_ptr] = r1;

   pop r0;

   // allocate space in temporary memory for SBR_G_lim_boost_exponent[]
   call $aacdec.frame_mem_pool_allocate;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_G_lim_boost_exponent_ptr] = r1;

   // allocate space in temporary memory for SBR_Q_M_lim_boost_exponent[]
   call $aacdec.frame_mem_pool_allocate;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_Q_M_lim_boost_exponent_ptr] = r1;

   // allocate space in temporary memory for SBR_S_M_boost_exponent[]
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.corruption;
   r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r2 + $aacdec.SBR_S_M_boost_exponent_ptr] = r1;


   r2 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r2 + $aacdec.SBR_bs_limiter_bands];
   // r1 = MAX SIZE of SBR_F_table_lim = SBR_Nlow + 5
   r1 = M[r2 + $aacdec.SBR_Nlow];
   r1 = r1 + 5;
   r0 = r0 * r1 (int);
   Words2Addr(r0);
   r8 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r8 = M[r8 + $aacdec.SBR_F_table_lim_base_ptr];
   r0 = r0 + r8;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_ftable_lim_last_band_ptr] = r0;


   // for l=0:SBR_bs_num_env[ch]-1,
   r8 = 0;

   outer_envelope_loop:

      // make the initial value of each block_exponent[l] = -(2^15)
      r0 = -(1<<15);
      Words2Addr(r8);
      r3 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r3 + r8;
      M[r2 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD] = r0;
      M[r2 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD] = r0;
      M[r2 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD] = r0;

      M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_f_noise_band] = Null;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_res_band] = Null;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_res_band2] = Null;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_hi_res_band] = Null;

      // if((l==SBR_l_A[ch])||((l==0) && SBR_prevEnvIsShort[ch]))
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      r0 = M[r0 + $aacdec.SBR_l_A];
      Words2Addr(r0);
      Null = r8 - r0;
      if Z jump clear_delta;
         Null = r8;
         if NZ jump set_delta;
         r2 = r3 + r5;
         Null = M[r2 + $aacdec.SBR_prevEnvIsShort];
         if NZ jump clear_delta;
            set_delta:
            // delta = 1
            r0 = 1;
            jump save_delta;
      // else
      clear_delta:
      // delta = 0
      r0 = 0;
      save_delta:
      M[r9 + $aac.mem.TMP + $aacdec.SBR_delta] = r0;

      r1 = r5 * 6 (int);
      r1 = r1 + r8;
      Addr2Words(r8);
      r1 = r1 + r9;
      r1 = M[$aac.mem.SBR_t_E + (1*ADDR_PER_WORD) + r1];
      r2 = r5 * 3 (int);
      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_t_noise_band];
      Words2Addr(r0);
      r2 = r2 + r0;
      r2 = r2 + r9;
      r2 = M[$aac.mem.SBR_t_Q + (1*ADDR_PER_WORD) + r2];
      // if(SBR_t_E[ch][l+1] > SBR_t_Q[ch][current_t_noise_band+1])
      Null = r1 - r2;
      if LE jump dont_increment_curr_t_noise_band;
         r0 = r0 + 1;   // current_t_noise_band += 1
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_t_noise_band] = r0;
      dont_increment_curr_t_noise_band:

      Addr2Words(r5);
      M2 = 0;  // current_res_band2 = 0
      call $aacdec.sbr_get_s_mapped;
      Words2Addr(r5);

      // for k=0:SBR_N_L[SBR_bs_limiter_bands]-1,
      r2 = 0;

      limiter_band_outer_loop:

         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_limiter_band_outer_loop_bound] = r2;

         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_mantissa] = Null;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_exponent] = Null;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc1_man] = Null;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc1_exp] = Null;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc2_man] = Null;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc2_exp] = Null;

         // r2 = k
         Words2Addr(r2);
         r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_ftable_lim_last_band_ptr];
         r0 = r0 + r2;
         Addr2Words(r2);
         // limiter band start : ml1 = SBR_F_table_lim[SBR_bs_limiter_bands][k]
         r6 = M[r0];
         // limiter band end : ml2 = SBR_F_table_lim[SBR_bs_limiter_bands][k+1]
         r7 = M[r0 + (1*ADDR_PER_WORD)];
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_acc_loop_bound] = r7;

         // calculate accumulated SBR_E_orig[ch][l][ml1;ml2-1] and SBR_E_curr[ch][l][ml1:ml2-1] over the limiter band

         // M1 = m
         M1 = r6;
         r10 = r7 - r6;
         // M2 = current_res_band
         r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_res_band];
         Words2Addr(r0);
         M2 = r0;
         Addr2Words(r0);

         // I1 = ptr to SBR_E_orig[ch][l][current_res_band]
         r2 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = M[r2 + ($aacdec.SBR_num_env_bands + (1*ADDR_PER_WORD))];
         r1 = r1 * r8 (int);
         r0 = r0 + r1;
         r1 = r5 + r9;
         r1 = M[$aac.mem.SBR_E_orig_mantissa_base_ptr + r1];
         Words2Addr(r0);
         I1 = r0 + r1;

         // I3 = ptr to SBR_E_curr[ch][m][l]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = r0 + r5;
         r0 = M[r0 + $aacdec.SBR_E_current_base_ptr];
         r1 = M[r2 + $aacdec.SBR_M];
         r1 = r1 * r8 (int);
         Words2Addr(r1);
         r0 = r0 + r1;

         r2 = M1;
         Words2Addr(r2);
         I3 = r0 + r2;

         // acc1 = 0
         r2 = 0;  // mantissa
         r3 = 0;  // exponent

         M3 = r6;
         // acc2 = 0
         r6 = 0;  // mantissa
         r7 = 0;  // exponent

         // if(SBR_bs_freq_res[ch][l] == 0)
         //    rMAC -> SBR_F_table_low[current_res_band + 1]
         // else
         //    rMAC -> SBR_F_table_high[current_res_band + 1]
         r0 = r5 * 6 (int);
         Words2Addr(r8);
         r0 = r0 + r8;
         Addr2Words(r8);
         r4 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = r4 + M2;
         rMAC = r1 + $aacdec.SBR_F_table_high + (1*ADDR_PER_WORD);
         r1 = r1 + $aacdec.SBR_F_table_low + (1*ADDR_PER_WORD);
         r4 = r4 + r0;
         Null = M[r4 + $aacdec.SBR_bs_freq_res];
         if Z rMAC = r1;


         PROFILER_START(&$aacdec.profile_sbr_calculate_gain_loop1)

         // for m=ml1:ml2-1,

         do calc_accum_energy_e_orig_lim_band_loop;

            // if((m+SBR_Kx)=={SBR_F_table_high¦¦SBR_F_table_low}[current_resr_band + 2])
            r1 = M[r9 + $aac.mem.SBR_info_ptr];
            r1 = M[r1 + $aacdec.SBR_kx];
            r1 = r1 + M1;
            r0 = M[rMAC] - r1;
            if NZ jump dont_increment_curr_res_band;
               // current_res_band += 1
               M2 = M2 + MK1;
               I1 = I1 + MK1;
               rMAC = rMAC + MK1;
            dont_increment_curr_res_band:

            // acc1 += SBR_E_orig[ch][l][current_res_band]

            r4 = M[I1, 0];
            r0 = r4 AND $aacdec.FLPT_MANTISSA_MASK;
            r1 = r4 ASHIFT -$aacdec.FLPT_EXPONENT_SHIFT;
            Null = r2;
            if Z jump save_acc1;
               // equalise exponents
               r4 = r1 - r3;
               if LT jump exp_of_addition_lt_exp_of_acc1;
                  r4 = -r4;
                  r2 = r2 LSHIFT r4;
                  jump save_acc1;
               exp_of_addition_lt_exp_of_acc1:
                  r1 = r3;
                  r0 = r0 LSHIFT r4;
            save_acc1:
            r2 = r2 + r0;  // mantissa of sum
            r3 = r1;       // exponent of sum

            // acc2 += SBR_E_curr[ch][l][m]

            r4 = M[I3, MK1];
            r0 = r4 AND $aacdec.FLPT_MANTISSA_MASK;
            r1 = r4 ASHIFT -$aacdec.FLPT_EXPONENT_SHIFT;
            Null = r6;
            if Z jump save_acc2;
               // equalise exponents
               r4 = r1 - r7;
               if LT jump exp_of_addition_lt_exp_of_acc2;
                  r4 = -r4;
                  r6 = r6 LSHIFT r4;
                  jump save_acc2;
               exp_of_addition_lt_exp_of_acc2:
                  r1 = r7;
                  r0 = r0 LSHIFT r4;
            save_acc2:
            r6 = r6 + r0;  // mantissa of sum
            r7 = r1;       // exponent of sum

            M1 = M1 + 1;

         calc_accum_energy_e_orig_lim_band_loop:

         PROFILER_STOP(&$aacdec.profile_sbr_calculate_gain_loop1)

         r0 = M2;
         Addr2Words(r0);
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_res_band] = r0;

         // if(acc1 == 0) acc1 = 1e-12
         Null = r2;
         if NZ jump acc1_not_zero;
            r2 = 1;
            r3 = (-40 + 6);
         acc1_not_zero:

         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc1_man] = r2;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc1_exp] = r3;

         // if(acc2 == 0) acc2 = 1e-12
         Null = r6;
         if NZ jump acc2_not_zero;
            r6 = 1;
            r7 = (-40 + ($aacdec.SBR_ANALYSIS_SHIFT_AMOUNT * 2 + 1));  // use constant (for man-exp & for the +9)
         acc2_not_zero:

         // calculate maximum gain (SBR_Gmax) = ratio of energy of original signal
         // to that of HF-generated signal
         // SBR_Gmax = ((acc1 + 1e-12) / (acc2 + 1e-12)) * SBR_limGain[SBR_bs_limiter_gains]

         // store acc2_mantissa in 24-bits
         r4 = SIGNDET r6;

         r0 = r6 LSHIFT r4;
         // acc2_exponent - acc2_shift_bits
         r1 = r7 - r4;

         // contain acc2 mantissa in 46 bits i.e length(dec2bin((2^45 - 1) / (2^23 -1))) = 24-bits

         rMAC = r2;
         r10 = SIGNDET rMAC;
         r10 = r10 - 2;
         rMAC = rMAC LSHIFT r10;
         r10 = r10 + 24;
         // acc1_exponent - acc1_shift_bits
         r3 = r3 - r10;


         Div = rMAC / r0;
         // r3 = exponent of divide result
         r3 = r3 - r1;
         // M2 = current_res_band2
         r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_res_band2];
         M2 = r1;
         // M1 = current_hi_res_band
         r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_hi_res_band];
         M1 = r1;
         // I0 = ptr to SBR_E_orig[ch][l][current_res_band2]
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = M[r0 + $aacdec.SBR_num_env_bands + (1*ADDR_PER_WORD)];
         r1 = r1 * r8 (int);
         r1 = r1 + M2;
         Words2Addr(r1);
         r2 = r5 + r9;
         r2 = M[$aac.mem.SBR_E_orig_mantissa_base_ptr + r2];
         I0 = r1 + r2;
         // I7 = ptr to SBR_E_curr[ch][ml1][l]
         r1 = M[r0 + $aacdec.SBR_M];
         r1 = r1 * r8 (int);
         r1 = r1 + M3;
         Words2Addr(r1);
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = r0 + r5;
         r2 = M[r0 + $aacdec.SBR_E_current_base_ptr];
         I7 = r1 + r2;
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = M[r0 + $aacdec.SBR_bs_limiter_gains];
         r2 = DivResult;

         r1 = $aacdec.SBR_calc_gain_g_max_limit_mantissa;
         Null = r0 - 3;
         if NZ jump bs_limiter_gains_ne_three;
            // exponent
            r3 = r3 + $aacdec.SBR_calc_gain_current_1e10_exp;
            // mantissa
            r2 = r2 * r1 (frac);  // $aacdec.SBR_calc_gain_current_1e10_man == $aacdec.SBR_calc_gain_g_max_limit_mantissa
            jump end_if_bs_limiter_gains;
         bs_limiter_gains_ne_three:
            r0 = r0 - 1;
            r3 = r3 + r0;
         end_if_bs_limiter_gains:


         // SBR_Gmax = min(SBR_Gmax, 1e10) : r2 = SBR_Gmax_mantissa, r3 = SBR_Gmax_exponent

         // store G_max with its mantissa occupying 24-bits
         r0 = SIGNDET r2;
         r2 = r2 LSHIFT r0;
         r3 = r3 - r0;

         // SBR_Gmax = min(SBR_Gmax, 1e10)
         Null = r3 - $aacdec.SBR_calc_gain_g_max_limit_exponent;
         if NZ jump dont_compare_mantissas_g_max_limit;
            Null = r2 - r1;
            if GT r2 = r1;
               jump g_max_limited;
         dont_compare_mantissas_g_max_limit:
            if LT jump g_max_limited;
               r2 = r1;
               r3 = $aacdec.SBR_calc_gain_g_max_limit_exponent;
         g_max_limited:

         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         M[r0 + $aacdec.SBR_G_max_mantissa] = r2;
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         M[r0 + $aacdec.SBR_G_max_exponent] = r3;

         r0 = M3;
         Words2Addr(r0);
         M3 = r0;
         // I2 = ptr to SBR_G_lim_mantissa[l][ml1]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_G_lim_boost_mantissa_ptr];
         I2 = r0 + M3;
         // I5 = ptr to SBR_G_lim_exponent[l][ml1]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_G_lim_boost_exponent_ptr];
         I5 = r0 + M3;

         // I1 = ptr to SBR_Q_M_lim_mantissa[l][ml1]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr];
         I1 = r0 + M3;
         // I6 = ptr to SBR_Q_M_lim_exponent[l][ml1]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_Q_M_lim_boost_exponent_ptr];
         I6 = r0 + M3;

         // I3 = ptr to SBR_S_M_mantissa[l][ml1]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_S_M_boost_mantissa_ptr];
         I3 = r0 + M3;
         // I4 = ptr to SBR_S_M_exponent[l][ml1]
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_S_M_boost_exponent_ptr];
         I4 = r0 + M3;

         r0 = M3;
         Addr2Words(r0);
         M3 = r0;

         Addr2Words(r5);
         call $aacdec.sbr_calculate_limiter_band_boost_coefficients;
         Words2Addr(r5);

         // SBR_G_boost[l][k] = acc1 / (den + 1e-12)

         // shift acc1 left to remove all redundant sign bits
         // for maximum precision of SBR_G_boost[l][k]
         rMAC = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc1_man];
         r2 = SIGNDET rMAC;
         // contain acc1 mantissa in 46-bits to avoid overflow
         r2 = r2 - 2;
         rMAC = rMAC LSHIFT r2;
         r2 = r2 + 24;  // r2 = acc1_shift_bits

         // den = max(den, 1e-12)
         r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_exponent];
         r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_mantissa];
         if NZ jump den_not_zero;
            r1 = 1;
            r4 = (-40);
         den_not_zero:

         // contain mantissa of den in 24-bits to acheive best precision of divide result
         r3 = SIGNDET r1;  // r3 = den_shift_bits
         r1 = r1 LSHIFT r3;
         Div = rMAC / r1;
         // evaluate exponent of divide result
         r1 = r3 - r4;
         // r0 = acc1_exponent - (den_exponent - den_shift_bits)
         r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_acc1_exp];
         r0 = r0 + r1;
         r1 = DivResult;
         // SBR_G_boost_exponent[l][k] = (acc1_exponent - acc1_shift_bits) - (den_exponent - den_shift_bits)
         r0 = r0 - r2;
         // adjust SBR_G_boost[l][k] so its mantissa is held in 24-bits
         r2 = SIGNDET r1;
         r1 = r1 LSHIFT r2;
         r0 = r0 - r2;
         // adjust to correct scale and convert to fractional form
         r0 = r0 + (23 - 6);

         // SBR_G_boost[l][k] = min(SBR_G_boost[l][k], 2.51188643004795)
         r2 = $aacdec.SBR_G_boost_max_mantissa;
         Null = r0 - $aacdec.SBR_G_boost_max_exponent;
         if NZ jump dont_compare_mantissas_g_boost_limit;
            Null = r1 - r2;
            if GT r1 = r2;
               jump g_boost_limited;
         dont_compare_mantissas_g_boost_limit:
            if LT jump g_boost_limited;
               r1 = r2;
               r0 = $aacdec.SBR_G_boost_max_exponent;
         g_boost_limited:

         // r2 = k
         r2 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_limiter_band_outer_loop_bound];
         Words2Addr(r2);

         r3 = r2 + r9;
         M[r3 + $aac.mem.SBR_limiter_band_g_boost_mantissa] = r1;
         M[r3 + $aac.mem.SBR_limiter_band_g_boost_exponent] = r0;
         Addr2Words(r2);

         r0 = M2;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_res_band2] = r0;
         r0 = M1;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_hi_res_band] = r0;

         r2 = r2 + 1;
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r3 = M[r0 + $aacdec.SBR_bs_limiter_bands];
         Words2Addr(r3);
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = r0 + r3;
         r3 = M[r0 + $aacdec.SBR_N_L];

         // SBR_N_L[SBR_bs_limiter_bands] - (k+1)
         Null = r3 - r2;
      if GT jump limiter_band_outer_loop;


      // set up pointers
      // I1 <- SBR_G_lim_boost_mantissa[l][0]
      r4 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r4 + $aacdec.SBR_G_lim_boost_mantissa_ptr];
      I1 = r0;
      // I3 <- SBR_Q_M_lim_boost_mantissa[l][0]
      r0 = M[r4 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr];
      I3 = r0;
      // I7 <- SBR_S_M_boost_mantissa[l][0]
      r0 = M[r4 + $aacdec.SBR_S_M_boost_mantissa_ptr];
      I7 = r0;

      // set up pointers
      // I2 <- SBR_G_lim_boost_exponent[0]
      r0 = M[r4 + $aacdec.SBR_G_lim_boost_exponent_ptr];
      I2 = r0;
      // I4 <- SBR_Q_M_lim_boost_exponent[0]
      r0 = M[r4 + $aacdec.SBR_Q_M_lim_boost_exponent_ptr];
      I4 = r0;
      // I6 <- SBR_S_M_boost_exponent[0]
      r0 = M[r4 + $aacdec.SBR_S_M_boost_exponent_ptr];
      I6 = r0;

      r4 = 1;

      Words2Addr(r8);
      M2 = 0;
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r8;
      r0 = M[r2 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      Null = r0 AND 1;
      if NZ M2 = r4;    // set flag is SBR_G_lim_block_exponent[l] + (23 + 3 - 4) is odd

      M3 = 0;
      r0 = M[r2 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      Null = r0 AND 1;
      if Z M3 = r4;     // set flag is SBR_Q_M_lim_block_exponent[l] + (23 - 4) is odd

      L5 = 0;
      r0 = M[r2 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      Null = r0 AND 1;
      if Z L5 = r4;     // set flag is SBR_S_M_block_exponent[l] + (23 - 4) is odd
      Addr2Words(r8);


      PROFILER_START(&$aacdec.profile_sbr_calculate_gain_loop3)

      M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_save_ch] = r5;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_save_l] = r8;

      r2 = 0;

      lim_boost_outer_loop:

         // r7 = SBR_G_boost_exponent[l][k]
         M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_lim_boost_outer_loop_index] = r2;
         Words2Addr(r2);
         r3 = r2 + r9;
         r7 = M[r3 + $aac.mem.SBR_limiter_band_g_boost_exponent];

         r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_save_l];
         Words2Addr(r0);
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r8 = r1 + r0;
         r5 = M[r8 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];     // r5 = SBR_G_lim_block_exponent[l]
         r3 = M[r8 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];       // r3 = SBR_S_M_block_exponent[l]
         r8 = M[r8 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];   // r8 = SBR_Q_M_lim_block_exponent[l]

         r5 = r7 - r5;  // r5 = SBR_G_boost_exponent[l][k] - SBR_G_lim_block_exponent[l]
         r8 = r7 - r8;  // r8 = SBR_G_boost_exponent[l][k] - SBR_Q_M_lim_block_exponent[l]
         M1 = r7 - r3;  // M1 = SBR_G_boost_exponent[l][k] - SBR_S_M_block_exponent[l]

         r0 = M[r1 + $aacdec.SBR_bs_limiter_bands];
         r1 = M[r1 + $aacdec.SBR_Nlow];
         r1 = r1 + 5;
         r0 = r0 * r1 (int);
         Words2Addr(r0);

         r0 = r0 + r2;
         r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r1 = M[r1 + $aacdec.SBR_F_table_lim_base_ptr];
         // ml1 = SBR_F_table_lim[SBR_bs_limiter_bands][k]
         r10 = M[r0 + r1];

         // ml2 = SBR_F_table_lim[SBR_bs_limiter_bands][k+1]
         r0 = r0 + (1*ADDR_PER_WORD);
         r0 = M[r0 + r1];

         // r10 = ml2 - ml1
         r10 = r0 - r10;

         // r6 = SBR_G_boost[l][k]

         r3 = r2 + r9;
         r6 = M[r3 + $aac.mem.SBR_limiter_band_g_boost_mantissa];

         // for m=ml1:ml2-1,

         do lim_boost_loop;

            // SBR_G_lim_boost[l][m] = sqrt(SBR_G_lim[l][m] * SBR_G_boost[l][k])

            r0 = M[I1, 0];    // r0 = SBR_G_lim_mantissa[l][m]
            r0 = r0 * r6 (frac), // r0 = SBR_G_lim_mantissa[l][m] * SBR_G_boost_mantissa[l][k]
             r1 = M[I2, MK1];   // r1 = SBR_G_lim_exponent[l][m]

            // r1 = SBR_G_lim_exponent[l][m] + SBR_G_boost_exponent[l][k] - SBR_G_lim_block_exponent[l]
            r1 = r1 + r5;

            // shift result of multiplication up
            // to occupy 24-bits for maximum precision of sqrt result
            r2 = SIGNDET r0;
            r0 = r0 LSHIFT r2;

            // r3 = no. of bits to shift the multiplication result up by to make
            // it's exponent equal to SBR_G_lim_block_exponent[l]
            r3 = r1 - r2;

            // the sqrt result is shifted by half this no. of bits
            // so make sure its an even number
            r7 = -1;
            Null = r3 AND 1;
            if Z r7 = 0;
            r3 = r3 - r7;
            r0 = r0 LSHIFT r7;
            // r7 = this no. of bits halved
            r7 = r3 ASHIFT -1;

            // SBR_G_lim_block_exponent needs to be halved because of the square root
            // so if this is an odd number shift the mantissa down by one and the block
            // exponent is incremented by one before it is halved
            r1 = -M2;
            r0 = r0 LSHIFT r1;

            call $math.sqrt;

            // avoid the multiplication by SBR_G_boost making the result overflow
            // the amount subtracted here is added to the block exponent after it is saved
            r0 = r7 - ($aacdec.SBR_G_boost_max_exponent / 2);
            r1 = r1 LSHIFT r0,
             r0 = M[I7, 0];    // r0 = SBR_S_M[l][m]
            M[I1, MK1] = r1;    // SBR_G_lim_boost_mantissa[l][m] = r1

            r1 = 0;
            // in any particular envelope 'l' there is either a Noise or Sinusoidal component added to a particular frequency
            // bin 'm' (never both components) so only do the calculation for one of the gains and set the other to zero

            // if(SBR_S_M[l][m] == 0)
            Null = r0;
            if NZ jump s_m_ne_zero;
               M[I7, MK1] = r1; // SBR_S_M_boost_mantissa[l][m] = 0
               I5 = I3;       // I5 <- SBR_Q_M_lim_boost[l][m]
               r0 = M[I3, MK1],    // r0 = SBR_Q_M_lim_mantissa[l][m]
                r4 = M[I4, MK1];   // r4 = SBR_Q_M_lim_exponent[l][m]
               // r3 = SBR_Q_M_lim_exponent[l][m] + SBR_G_boost_exponent[l][k] - SBR_Q_M_lim_block_exponent[l]
               r3 = r8 + r4;
               I6 = I6 + MK1;
               L4 = M3;
               jump end_if_s_m_eq_zero;
            s_m_ne_zero:
               M[I3, MK1] = r1,  // SBR_Q_M_lim_boost_mantissa[l][m] = 0
                r4 = M[I6, MK1]; // r4 = SBR_S_M_exponent[l][m]
               I5 = I7;       // I5 <- SBR_S_M_boost[l][m]
               // r3 = SBR_S_M_exponent[l][m] + SBR_G_boost_exponent[l][k] - SBR_S_M_block_exponent[l]
               r3 = r4 + M1;
               r0 = M[I7, MK1]; // r0 = SBR_S_M_boost_mantissa[l][m]
               I4 = I4 + MK1;
               L4 = L5;
            end_if_s_m_eq_zero:

            // {SBR_ Q_M_lim_boost¦¦SBR_S_M_boost}[l][m] = sqrt({SBR_Q_M_lim¦¦SBR_S_M_lim}[l][m] * SBR_G_boost[l][k])
            r0 = r0 * r6 (frac);

            // shift result of multiplication up
            // to occupy 24-bits for maximum precision of sqrt result
            r2 = SIGNDET r0;
            r0 = r0 LSHIFT r2;

            // r3 = no. of bits to shift the multiplication result up by to make
            // it's exponent equal to {SBR_G_lim_block_exponent¦¦SBR_Q_M_lim_block_exponent}[l]
            r3 = r3 - r2;

            // the sqrt result is shifted by half this no. of bits
            // so make sure its an even number
            r7 = -1;
            Null = r3 AND 1;
            if Z r7 = 0;
            r3 = r3 - r7;
            r0 = r0 LSHIFT r7;

            // r7 = this no. of bits halved
            r7 = r3 ASHIFT -1;

            // {SBR_G_lim_block_exponent¦¦SBR_Q_M_lim_block_exponent} needs to be halved
            // because of the square root so if this is an odd number shift the mantissa
            // down by one and the block exponent is incremented by one before it is halved
            r1 = -L4;
            r0 = r0 LSHIFT r1;

            call $math.sqrt;

            // avoid the multiplication by SBR_G_boost[l][k] making the result overflow
            // the amount subtracted here is added to the block exponent after it is saved
            r2 = r7 - ($aacdec.SBR_G_boost_max_exponent / 2);
            r1 = r1 LSHIFT r2;
            M[I5, 0] = r1;    // {SBR_Q_M_lim_boost_mantissa¦¦SBR_S_M_boost_mantissa}[l][m] = r1

            L4 = 0;

         lim_boost_loop:

      r2 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_lim_boost_outer_loop_index];
      r2 = r2 + 1;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r1 = M[r0 + $aacdec.SBR_bs_limiter_bands];
      Words2Addr(r1);
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r1;
      r0 = M[r0 + $aacdec.SBR_N_L];
      Null = r2 - r0;
      if LT jump lim_boost_outer_loop;

      r5 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_save_ch];
      r8 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_save_l];
      Words2Addr(r8);

      PROFILER_STOP(&$aacdec.profile_sbr_calculate_gain_loop3)


      r4 = 1;

      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r8;
      r0 = M[r2 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      // add 23 to as the mantissa is now a fractional after the sqrt; add 3 and sub 4 to adjust back to correct scale
      r0 = r0 + (23 + 3 - 4);
      // before halving make sure SBR_G_lim_boost_block_exponent[l] is even
      // if one is added here the corresponding shift down by one bit was done on
      // all SBR_G_lim_boost_mantissa[l][:] values in lim_boost_outer_loop above
      Null = M2;
      if NZ r0 = r0 + r4;
      r0 = r0 ASHIFT -1;
      // avoid the multiplication by SBR_G_boost[l][k] making the result overflow; corresponding
      // shift of SBR_G_lim_boost_mantissa[l][:] values done in lim_boost_outer_loop above
      r0 = r0 + ($aacdec.SBR_G_boost_max_exponent / 2);  // r0 = SBR_G_lim_boost_block_exponent[l]
      M[r2 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD] = r0;

      r0 = M[r2 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      r0 = r0 + (23 - 4);  // add 23 to as the mantissa is now a fractional after the sqrt; sub 4 to adjust back to correct scale
      // before halving make sure SBR_Q_M_lim_boost_block_exponent[l] is even
      // if one is added here the corresponding shift down by one bit was done on
      // all SBR_Q_M_lim_boost_mantissa[l][:] values in lim_boost_outer_loop above
      Null = M3;
      if NZ r0 = r0 + r4;
      r0 = r0 ASHIFT -1;
      // avoid the multiplication by SBR_G_boost[l][k] making the result overflow; corresponding
      // shift of SBR_Q_M_lim_boost_mantissa[l][:] non-zero values done in lim_boost_outer_loop above
      r0 = r0 + ($aacdec.SBR_G_boost_max_exponent / 2);  // r0 = SBR_Q_M_lim_boost_block_exponent[l]
      M[r2 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD] = r0;

      r0 = M[r2 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      r0 = r0 + (23 - 4);  // add 23 to as the mantissa is now a fractional after the sqrt; sub 4 to adjust back to correct scale
      // before halving make sure SBR_S_M_boost_block_exponent[l] is even
      // if one is added here the corresponding shift down by one bit was done on
      // all SBR_S_M_boost_mantissa[l][:] values in lim_boost_outer_loop above
      Null = L5;
      if NZ r0 = r0 + r4;
      r0 = r0 ASHIFT -1;
      // avoid the multiplication by SBR_G_boost[l][k] making the result overflow; corresponding
      // shift of SBR_S_M_boost_mantissa[l][:] non-zero values done in lim_boost_outer_loop above
      r0 = r0 + ($aacdec.SBR_G_boost_max_exponent / 2);  // r0 = SBR_S_M_boost_block_exponent[l]
      M[r2 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD] = r0;

      L5 = 0;

      r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope];
      Words2Addr(r1);

      // overwrite pointer to point to start of SBR_G_lim_mantissa[l][:] which is
      // subsequently overwritten in lim_boost_outer_loop with SBR_G_lim_boost_mantissa[l][:]
      r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r2 + $aacdec.SBR_G_lim_boost_mantissa_ptr];
      r0 = r0 + r1;
      M[r2 + $aacdec.SBR_G_lim_boost_mantissa_ptr] = r0;

      // overwrite pointer to point to start of SBR_Q_M_lim_mantissa[l][:] which is
      // subsequently overwritten in lim_boost_outer_loop with SBR_Q_M_lim_boost_mantissa[l][:]
      r0 = M[r2 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr];
      r0 = r0 + r1;
      M[r2 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr] = r0;

      // overwrite pointer to point to start of SBR_S_M_mantissa[l][:] which is
      // subsequently overwritten in lim_boost_outer_loop with SBR_S_M_boost_mantissa[l][:]
      r0 = M[r2 + $aacdec.SBR_S_M_boost_mantissa_ptr];
      r0 = r0 + r1;
      M[r2 + $aacdec.SBR_S_M_boost_mantissa_ptr] = r0;

      Addr2Words(r8);

      r8 = r8 + 1;
      r0 = r5 + r9;
      r0 = M[$aac.mem.SBR_bs_num_env + r0];
      // SBR_bs_num_env[ch] - l
      Null = r0 - r8;
   if GT jump outer_envelope_loop;


   // free temporary memory allocated for the 3 exponent arrays
   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope];
   r1 = r5 + r9;
   r1 = M[$aac.mem.SBR_bs_num_env + r1];
   r2 = r0 * r1 (int);

   r0 = r0 * 3 (int);
   call $aacdec.frame_mem_pool_free;

   // set the 3 boost_gain_mantissa pointers back to the start of the
   // array (i.e. first freq bin of first envelope)
   Words2Addr(r2);

   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = M[r1 + $aacdec.SBR_G_lim_boost_mantissa_ptr];
   r0 = r0 - r2;
   M[r1 + $aacdec.SBR_G_lim_boost_mantissa_ptr] = r0;

   r0 = M[r1 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr];
   r0 = r0 - r2;
   M[r1 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr] = r0;

   r0 = M[r1 + $aacdec.SBR_S_M_boost_mantissa_ptr];
   r0 = r0 - r2;
   M[r1 + $aacdec.SBR_S_M_boost_mantissa_ptr] = r0;


   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;






// *********************************************************************************
// MODULE:
//    $aacdec.update_den
//
// DESCRIPTION:
//    make an addition to the 'den' accumulation (stored in mantissa-exponent form)
//
//    shift the mantissa of either 'den' or the 'addition' down by the difference in
//    exponents in order to have a common (the bigger of the two) exponent
//    then add the mantissas together
//
// INPUTS:
//    - r0 = mantissa of addition
//    - r1 = exponent of addition
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r4, r7, r10
//
// **********************************************************************************
.MODULE $M.aacdec.update_den;
   .CODESEGMENT AACDEC_UPDATE_DEN_PM;
   .DATASEGMENT DM;

   $aacdec.update_den:

   r10 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_mantissa];
   if Z jump save_den;
      r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_exponent];
      // equalise exponents
      r7 = r1 - r4;
      if LT jump exp_of_addition_lt_exp_of_den;
         r7 = -r7;
         r10 = r10 LSHIFT r7;
      jump save_den;
      exp_of_addition_lt_exp_of_den:
         r1 = r4;
         r0 = r0 LSHIFT r7;
   save_den:

   r0 = r0 + r10;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_mantissa] = r0;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_den_exponent] = r1;

   rts;

.ENDMODULE;

#endif



