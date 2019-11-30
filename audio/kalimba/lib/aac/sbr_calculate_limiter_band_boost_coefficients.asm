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
//    $aacdec.sbr_calculate_limiter_band_boost_coefficients
//
// DESCRIPTION:
//    Calculate the boost coefficients in each limiter band (frequency)
//    in each envelope (time) used to caluclate the final gain values for
//    each of the three components (signal_gain, noise_component, sinusoidal_component)
//
// INPUTS:
//    -
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_calculate_limiter_band_boost_coefficients;
   .CODESEGMENT AACDEC_SBR_CALCULATE_LIMITER_BAND_BOOST_COEFFICIENTS_PM;
   .DATASEGMENT DM;


   $aacdec.sbr_calculate_limiter_band_boost_coefficients:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_CALCULATE_LIMITER_BAND_BOOST_COEFFICIENTS_ASM.SBR_CALCULATE_LIMITER_BAND_BOOST_COEFFICIENTS.PATCH_ID_0, r4)
#endif

   PROFILER_START(&$aacdec.profile_sbr_calculate_gain_loop2)

    // for m=ml1:ml2-1,
    calc_accum_energy_e_current_lim_band_loop:

       // check if 'm' is on a noise band border
       // if((m+SBR_kx) == SBR_F_table_noise[current_f_noise_band+1])
       r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_f_noise_band];
       r2 = M[r9 + $aac.mem.SBR_info_ptr];
       r0 = M[r2 + $aacdec.SBR_kx];
       r3 = r0 + M3;
       Words2Addr(r4);
       r2 = r2 + r4;
       r2 = M[r2 + $aacdec.SBR_F_table_noise + (1*ADDR_PER_WORD)];
       Addr2Words(r4);
       Null = r3 - r2;
       if NZ jump dont_increment_current_f_noise_band;
          r4 = r4 + 1;
          M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_f_noise_band] = r4;
       dont_increment_current_f_noise_band:

       // check if 'm' is on a frequency resolution band border
       // if(SBR_bs_freq_res[ch][l] == 0) select low_frequency_resolution
       //    else select high_frequency_resolution
       r0 = r5 * 6 (int);
       r0 = r0 + r8;
       Words2Addr(r0);

       r1 = M2;
       Words2Addr(r1);

       r6 = M[r9 + $aac.mem.SBR_info_ptr];
       r2 = r6 + r1;

       r1 = r2 + $aacdec.SBR_F_table_low  + (1*ADDR_PER_WORD);
       r2 = r2 + $aacdec.SBR_F_table_high + (1*ADDR_PER_WORD);
       r6 = r6 + r0;
       Null = M[r6 + $aacdec.SBR_bs_freq_res];
       if NZ r1 = r2;
       r6 = M1 + 1;

       // if((m + SBR_kx) == {SBR_F_table_low¦¦SBR_F_table_high}[current_res_band2+1])
       Null = r3 - M[r1];
       if NZ jump dont_increment_current_res_band2_e_curr_loop;
          // current_res_band2 += 1 (stepping to next resolution band)
          I0 = I0 + MK1;
          M2 = M2 + 1;
          r1 = M2;
          call $aacdec.sbr_get_s_mapped;
       dont_increment_current_res_band2_e_curr_loop:

       // if((m + SBR_kx) == SBR_F_table_high[current_high_res_band + 1])
          // step to next Hi-resolution band
          // current_high_res_band += 1
       Words2Addr(r6);
       r0 = M[r9 + $aac.mem.SBR_info_ptr];
       r0 = r0 + r6;
       r0 = M[r0 + $aacdec.SBR_F_table_high];
       r6 = 1;
       Null = r3 - r0;
       if Z M1 = M1 + r6;

       // S_index_mapped = 0;
       M0 = 0;

       // if((l >= SBR_l_A[ch]) || (SBR_bs_add_harmonic_prev[ch][current_hi_res_band] && ...
                   // ... SBR_bs_add_harmonic_flag_prev[ch]))
       Words2Addr(r5);
       r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
       r0 = r0 + r5;
       r0 = M[r0 + $aacdec.SBR_l_A];
       Addr2Words(r5);
       r2 = M[r9 + $aac.mem.SBR_info_ptr];
       Null = r8 - r0;
       if GE jump find_middle_subband_of_high_res_band;
          r0 = r5 * 49 (int);
          r0 = r0 + M1;
          Words2Addr(r0);
          r1 = r2 + r0;
          Null = M[r1 + $aacdec.SBR_bs_add_harmonic_prev];
          if Z jump s_index_mapped_assigned;


          Words2Addr(r5);
          r1 = r2 + r5;
          r0 = M[r1 + $aacdec.SBR_bs_add_harmonic_flag_prev];
          Addr2Words(r5);
          Null = r0;
          if Z jump s_index_mapped_assigned;
             find_middle_subband_of_high_res_band:
             // find middle subband of the Hi-resolution frequency band
             // if((m + SBR_kx) == floor((SBR_F_table_high[current_hi_res_band+1] + ...
             // ... SBR_F_table_high[current_hi_res_band]) >> 1))
             r0 = M1;
             Words2Addr(r0);
             r0 = r2 + r0;
             r1 = M[r0 + $aacdec.SBR_F_table_high];
             r0 = M[r0 + $aacdec.SBR_F_table_high + (1*ADDR_PER_WORD)];
             r0 = r0 + r1;
             r0 = r0 LSHIFT -1;
             Null = r3 - r0;
             if NZ jump s_index_mapped_assigned;
         // S_index_mapped = SBR_bs_add_harmonic[ch][current_hi_res_band]
         r0 = r5 * 64 (int);
         r0 = r0 + M1;
         Words2Addr(r0);
         r0 = r0 + r2;
         r0 = M[r0 + $aacdec.SBR_bs_add_harmonic];
         M0 = r0;
       s_index_mapped_assigned:

       r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_current_t_noise_band];
       r0 = r0 * $aacdec.SBR_Nq_max (int);
       r0 = r0 + r4;
       r1 = r5 * 10 (int);
       r1 = r0 + r1;

       // SBR_E_orig[ch][current_res_band2][l]
       r4 = M[I0, 0];
       r0 = r4 AND $aacdec.FLPT_MANTISSA_MASK;  // extract mantissa

       Words2Addr(r1);
       // r2 = Q_div = SBR_Q_orig[ch][current_f_noise_band][current_t_noise_band]
       r1 = r1 + r9;
       r2 = M[($aac.mem.SBR_Q_orig) + r1];
       // r3 = Q_div2 = SBR_Q_orig_2[ch][current_f_noise_band][current_t_noise_band]
       r3 = M[($aac.mem.SBR_Q_orig2) + r1];


       // G = SBR_E_orig[ch][l][current_res_band2 + 1] / (SBR_E_curr[ch][l][m] + 1.0)

       // SBR_E_orig[ch][current_res_band2][l]
       r6 = r4;
       // contain SBR_E_orig[ch][current_res_band2][l] mantissa in 46-bits i.e length(dec2bin((2^45 - 1) / (2^23 -1))) = 24-bits
       r1 = SIGNDET r0;

       M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_e_orig_curr_res_band2_shift_bits] = r1;

       r1 = r1 - 2;
       r0 = r0 LSHIFT r1;
       rMAC = r0;

       // SBR_E_curr[ch][l][m]
       r1 = M[I7, 0];
       r0 = r1 AND $aacdec.FLPT_MANTISSA_MASK;   // extract mantissa
       r10 = r1 ASHIFT -$aacdec.FLPT_EXPONENT_SHIFT; // extract exponent

       r1 = $aacdec.SBR_calc_gain_e_curr_plus_one_value_mantissa;
       r4 = $aacdec.SBR_calc_gain_e_curr_plus_one_value_exponent;
       Null = r0;
       if Z jump save_e_curr_value;
         // equalise exponents

         r7 = r4 - r10;
         if LT r1 = r1 LSHIFT r7;
         r7 = r10 - r4;
         if LT r0 = r0 LSHIFT r7;
         r7 = r4 - r10;
         if LT r4 = r10;
       save_e_curr_value:

       r0 = r0 + r1;

       // contain SBR_E_curr[ch][m][l] mantissa in 23-bits i.e length(dec2bin((2^45 - 1) / (2^22))) = 23-bits
       r1 = SIGNDET r0;
       r0 = r0 LSHIFT r1;
       r1 = r4 - r1;
       M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_e_curr_plus_one_exp] = r1;

       Div = rMAC / r0;

         // SBR_E_orig[ch][l][current_res_band2] * {Q_div2¦¦Q_div}
         r1 = r6 AND $aacdec.FLPT_MANTISSA_MASK;  // extract mantissa
         r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_e_orig_curr_res_band2_shift_bits];
         r1 = r1 LSHIFT r4;

         r10 = r3;
         Null = M0;
         if NZ r10 = r2;

         r1 = r1 * r10 (frac);
         // contain SBR_Q_M[m] mantissa to 17-bits i.e. (2^23 - 1) / (2^17 - 1) = 64
         r0 = SIGNDET r1;
         r10 = r0 - 6;

         r0 = r1 LSHIFT r10;
         r4 = r4 + r10;

         r10 = r6 ASHIFT -$aacdec.FLPT_EXPONENT_SHIFT;  // extract exponent
         r4 = r10 - r4;

         // in any particular envelope 'l' there is either a Noise or Sinusoidal component added to a particular frequency
         // bin 'm' (never both components) so only do the calculation for one of the components and set the other to zero

         // SBR_S_index_mapped can only be non-zero once per HI_RES band
         // if(SBR_S_index_mappped == 0)
         Null = M0;
         if NZ jump s_index_mapped_not_zero;
            // SBR_Q_M = SBR_E_orig[ch][l][current_res_band2] * Q_div2
            r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
            M[r1 + $aacdec.SBR_Q_M_mantissa] = r0;
            M[r1 + $aacdec.SBR_Q_M_exponent] = r4;
            // SBR_S_M[m] = 0
            r0 = 0;
            M[I3, MK1] = r0, // mantissa
             M[I4, 0] = r0; // exponent
            jump common_exponent_found_and_den_updated;
            // else
         s_index_mapped_not_zero:
            // SBR_S_M[m] = SBR_E_orig[ch][l][[current_res_band2] * Q_div
            r1 = r4 - 6;     // adjust back to correct scale
            M[I3, MK1] = r0,   // mantissa
             M[I4, 0] = r1;    // exponent

            // SBR_Q_M = 0
            r4 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
            M[r4 + $aacdec.SBR_Q_M_mantissa] = Null;
            M[r4 + $aacdec.SBR_Q_M_exponent] = Null;

            // accumulate sinusoidal part of total energy
            // den += SBR_S_M[m]
            Null = r0;
            if NZ call $aacdec.update_den;
         common_exponent_found_and_den_updated:


      // adjust G so that its mantissa ocupies 24-bits for bet precision
      // in the fractional multiplication result
      r4 = DivResult;
      r0 = SIGNDET r4;
      r4 = r4 LSHIFT r0;

      // if((SBR_S_mapped == 0) && (delta == 1))
      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      Null = M[r1 + $aacdec.SBR_S_mapped];
      if GT jump s_mapped_eq_one;
      Null = M[r9 + $aac.mem.TMP + $aacdec.SBR_delta];
         // G = G * Q_div
         if NZ r4 = r4 * r2 (frac);
         jump g_assigned;
      // elsif(SBR_S_mapped == 1)
      s_mapped_eq_one:
         // G = G * Q_div2
         r4 = r4 * r3 (frac);
      g_assigned:


      // apply the limiter and limit the additional
      // noise energy level

      // adjust G so that its mantissa ocupies 24-bits
      r1 = SIGNDET r4;
      r4 = r4 LSHIFT r1;
      r0 = r0 + r1;

      // G_exponent = SBR_E_orig_mantissa[ch][current_res_band2][l] - E_curr_mantissa[ch][l][m]
      r3 = r6 ASHIFT -$aacdec.FLPT_EXPONENT_SHIFT;  // extract exponent
      r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_e_orig_curr_res_band2_shift_bits];
      r1 = r1 + (-2 + 24);
      r0 = r0 + r1;
      r3 = r3 - r0;
      r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_e_curr_plus_one_exp];
      r3 = r3 - r1;

      r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r1 = M[r2 + $aacdec.SBR_G_max_mantissa];
      r2 = M[r2 + $aacdec.SBR_G_max_exponent];

      Null = r2 - r3;
      if NZ jump dont_compare_g_and_gmax_mantissas;
      Null = r1 - r4;
      if GT jump g_max_gt_g;
         jump g_max_le_g;
      dont_compare_g_and_gmax_mantissas:
         if LE jump g_max_le_g;

      // if(SBR_Gmax > G)
      g_max_gt_g:

      M[I2, 0] = r4,    // SBR_G_lim_mantissa[l][m] = G_mantissa
       M[I5, 0] = r3;   // SBR_G_lim_exponent[l][m] = G_exponent

      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r1 + $aacdec.SBR_Q_M_mantissa];
      r1 = M[r1 + $aacdec.SBR_Q_M_exponent];
      // convert SBR_Q_M back to correct scale
      r1 = r1 - 6;
      M[I1, 0] = r0,    // SBR_Q_M_lim_mantissa[l][m] = SBR_Q_M_mantissa
       M[I6, 0] = r1;   // SBR_Q_M_lim_exponent[l][m] = SBR_Q_M_exponent
      jump qm_lim_and_g_lim_assigned;

         // else
         g_max_le_g:

         // SBR_Q_M_lim[l][m] = SBR_Q_M * SBR_Gmax / G
         // start the (SBR_Gmax / G) divide here
         // contain SBR_Gmax_mantissa in 46-bits for best precision of divide result
         // without overflow occuring with 24-bit denominator (SBR_G_mantissa)

         rMAC = r1 LSHIFT -2;
         Div = rMAC / r4;
         M[I2, 0] = r1,    // SBR_G_lim_mantissa[l][m] = SBR_Gmax_exponent
          M[I5, 0] = r2;   // SBR_G_lim_exponent[l][m] = SBR_Gmax_exponent
         r2 = r2 - (24 - 2);  // r2 = (SBR_G_max_exponent - Gmax_shift_bits)

         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_Q_M_exponent];
         r0 = r0 + r2;
         rMAC = r0 - r3;   // rMAC = (SBR_Q_M_exponent) + (SBR_G_max_exponent - Gmax_shift_bits) - (SBR_G_exponent)

         // contain SBR_Q_M_mantissa[l][m] in 9-bits for the multiplication
         // i.e length(dec2bin((2^14 - 1) * (2^9 - 1))) = 24-bits
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r0 + $aacdec.SBR_Q_M_mantissa];
         r3 = SIGNDET r0;
         r3 = r3 - (DAWTH-10);
         r0 = r0 LSHIFT r3;
         // rMAC = (SBR_Q_M_exponent - Q_M_shift_bits) + (SBR_G_max_exponent - Gmax_shift_bits) - (SBR_G_exponent)
         r3 = r3 + (DAWTH-24);
         rMAC = rMAC - r3;
         r1 = DivResult;
         // contain DivResult in 14-bits
         r3 = SIGNDET r1;
         r3 = r3 - 9;
         r1 = r1 ASHIFT r3;
         rMAC = rMAC - r3;
         r0 = r0 * r1 (int);

         // contain SBR_Q_M_lim_mantissa[l][m] in 17-bits inorder to acheive best precision
         // in den and avoid overflow in den_mantissa; i.e. (2^23 - 1) / (2^17 - 1) = 64
         r4 = SIGNDET r0;
         r3 = r4 - 6;
         r0 = r0 LSHIFT r3;
         // shift down by 6-bits more to adjust back to correct scale
         rMAC = rMAC - r4;
         M[I1, 0] = r0,
          M[I6, 0] = rMAC;
      qm_lim_and_g_lim_assigned:


      // estimate total energy in the envelope estimate over this limiter band
      // den += SBR_E_curr[ch][l][m] * SBR_G_lim[l][m]

      r4 = M[I7, MK1];
      r0 = r4 AND $aacdec.FLPT_MANTISSA_MASK;   // r0 = SBR_E_curr_mantissa[ch][l][m]

      rMAC = M[I2, MK1],  // rMAC = SBR_G_lim_mantissa[l][m]
       r2 = M[I5, 0];
      rMAC = rMAC * r0;

      // contain the product's mantissa in 17-bits inorder to acheive best precision in den
      // and avoid overflow in den_mantissa; i.e. (2^23 - 1) / (2^17 - 1) = 64
      r1 = SIGNDET rMAC;
      r10 = r1 - 6;

      r6 = r4 ASHIFT -$aacdec.FLPT_EXPONENT_SHIFT;  // r6 = SBR_E_curr_exponent[ch][l][m]
      r3 = r6 - r10;
      r3 = r3 + r2;

      // adjust exponent of result to correct scale and convert so mantissa is a fractional
      r1 = r3 + (23-($aacdec.SBR_ANALYSIS_SHIFT_AMOUNT*2 + 1)+3);
      r0 = rMAC LSHIFT r10;
      if NZ call $aacdec.update_den;


      // if((S_index_mapped == 0) && (delta == 1))
      //    den += SBR_Q_M_lim[l][m]
      Null = M0;
      if NZ jump dont_add_qm_lim_to_den;
         Null = M[r9 + $aac.mem.TMP + $aacdec.SBR_delta];
         if Z jump dont_add_qm_lim_to_den;
         r0 = M[I1, 0],    // r0 = SBR_Q_M_lim_mantissa[l][m]
          r1 = M[I6, 0];   // r1 = SBR_Q_M_lim_exponent[l][m]
         Null = r0;
         if NZ call $aacdec.update_den;
      dont_add_qm_lim_to_den:

      r4 = M[r9 + $aac.mem.SBR_info_ptr];

      // SBR_G_lim_block_exponent[l] = max(SBR_G_lim_exponent[l][m], SBR_G_lim_block_exponent[l])
      r0 = M[I5, MK1];
      Words2Addr(r8);
      r1 = r8 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD;
      r2 = M[r4 + r1];
      Null = r0 - r2;
      if GT M[r4 + r1] = r0;

      // SBR_Q_M_lim_block_exponent[l] = max(SBR_Q_M_lim_exponent[l][m], SBR_Q_M_lim_block_exponent[l])
      I1 = I1 + MK1;
      r0 = M[I6, MK1];
      r1 = r8 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD;
      r2 = M[r4 + r1];
      Null = r0 - r2;
      if GT M[r4 + r1] = r0;

      // SBR_S_M_block_exponent[l] = max(SBR_S_M_exponent[l][m], SBR_S_M_block_exponent[l])
      r0 = M[I4, MK1];
      r1 = r8 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD;
      r2 = M[r4 + r1];
      Null = r0 - r2;
      if GT M[r4 + r1] = r0;

      Addr2Words(r8);

      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_acc_loop_bound];
      M3 = M3 + 1;
      // ml2 - m
      Null = r0 - M3;
   if GT jump calc_accum_energy_e_current_lim_band_loop;


   PROFILER_STOP(&$aacdec.profile_sbr_calculate_gain_loop2)

   // pop rLink from stack
   jump $pop_rLink_and_rts;


.ENDMODULE;

#endif


