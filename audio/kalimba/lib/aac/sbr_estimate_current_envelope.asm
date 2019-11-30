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
//    $aacdec.sbr_estimate_current_envelope
//
// DESCRIPTION:
//    Estimate the envelope for the current SBR frame using the energy of the subbands
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I0-I7, M0-M3
//    - an element of $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_estimate_current_envelope;
   .CODESEGMENT AACDEC_SBR_ESTIMATE_CURRENT_ENVELOPE_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_estimate_current_envelope:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5);

   PROFILER_START(&$aacdec.profile_sbr_estimate_current_envelope_internal)

   // allocate temporary memory for SBR_E_current
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_M];
   r1 = r5 + r9;
   r1 = M[$aac.mem.SBR_bs_num_env + r1];
   r0 = r0 * r1 (int);
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.corruption;
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   M[r0 + $aacdec.SBR_E_current_base_ptr] = r1;


   // if(SBR_bs_interpol_freq == 1)
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = M[r0 + $aacdec.SBR_bs_interpol_freq];
   if Z jump interpolation_off;

      // base ptr to SBR_t_E[ch]
      r0 = r5 * 6 (int);
      r0 = r0 + r9;
      I1 = r0 + $aac.mem.SBR_t_E;

      // base pointer to SBR_E_curr[ch][0]
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      r0 = M[r0 + $aacdec.SBR_E_current_base_ptr];
      I0 = r0;

      M0 = (64*ADDR_PER_WORD);
      // for l=0:SBR_bs_num_env[ch]-1,
      r0 = r5 + r9;
      r0 = M[$aac.mem.SBR_bs_num_env + r0];
      M1 = r0;

      // SBR_t_E[ch][0]
      r3 = M[I1, MK1];

      e_curr_envelope_outer_loop_interp_on:

         // lower_i = SBR_t_E[ch][l]
         r2 = r3;
         // upper_i = SBR_t_E[ch][l+1]
         r3 = M[I1, MK1];

         // div = 1 / (upper_i - lower_i)
         r8 = r3 - r2;
         M3 = r8;
         r1 = 0xFFE;
         r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
         Null = r0 - $aacdec.ER_AAC_ELD;
         // Note we're now using the ELD one_over_div array for ELD and non-ELD for simplicity
         if NZ r8 = r8 AND r1;
         Words2Addr(r8);
         r0 = M[r9 + $aac.mem.SBR_est_curr_env_one_over_div_ptr];
         r0 = r0 + r8;
         r4 = M[r0 - (1*ADDR_PER_WORD)];

         // I2 <- real(X_sbr[ch][SBR_kx][lower_i+SBR_tHFAdj]); I4 <- imag(X_sbr[ch][SBR_kx][lower_i+SBR_tHFAdj])
         r0 = r2 + $aacdec.SBR_tHFAdj;
         r0 = r0 * $aacdec.X_SBR_WIDTH (int);
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r1 = M[r1 + $aacdec.SBR_kx];
         r0 = r0 + r1;
         Words2Addr(r0);
         r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
         I2 = r1 + r0;
         r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
         I4 = r1 + r0;

         // for m=0:SBR_M-1,
         M2 = 0;

         e_curr_band_inner_loop_interp_on:

            // for i=(lower_i + SBR_tHFAdj):(upper_i + SBR_tHFAdj)-1,
            r10 = M3;
            // nrg = 0
            rMAC = 0;

            I3 = I2 + M2;
            I5 = I4 + M2,
             r0 = M[I3, M0];

            do e_curr_envelope_inner_loop_interp_on;
               // nrg += magnitude(X_sbr[ch][j][i]) ^ 2
               rMAC = rMAC + r0 * r0,
                r1 = M[I5, M0];

               rMAC = rMAC + r1 * r1,
                r0 = M[I3, M0];
            e_curr_envelope_inner_loop_interp_on:

            // SBR_E_curr[ch][m][l] = nrg / div
            r1 = SIGNDET rMAC;
            rMAC = rMAC LSHIFT r1;
            r0 = 24 - r1;

            call $aacdec.sbr_fp_mult_frac;

         M2 = M2 + MK1;
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = M[r0 + $aacdec.SBR_M];
         Words2Addr(r0);
         r0 = r0 - M2,
          M[I0, MK1] = r2;
         if GT jump e_curr_band_inner_loop_interp_on;

      M1 = M1 - 1;
      if NZ jump e_curr_envelope_outer_loop_interp_on;

      jump exit;
   interpolation_off:


      r0 = r5 * 6 (int);
      r0 = r0 + r9;
      I6 = $aac.mem.SBR_t_E + r0;

      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      r0 = M[r0 + $aacdec.SBR_E_current_base_ptr];
      I2 = r0;

      // M3 = l_i
      r0 = M[I6, MK1];
      M3 = r0;

      // for l=0:SBR_bs_num_env[ch]-1,
      M0 = 0;

      r8 = r5;

      e_curr_envelope_outer_loop_interp_off:

         r4 = M[r9 + $aac.mem.SBR_info_ptr];
         I7 = r4 + $aacdec.SBR_F_table_high;
         r5 = r4 + $aacdec.SBR_F_table_low;
         // r0 = SBR_bs_freq_res[ch][l]
         r0 = r8 * 6 (int);
         r0 = r0 + M0;
         r1 = r4 + r0;
         r0 = M[r1 + $aacdec.SBR_bs_freq_res];
         if Z I7 = r5;

         Words2Addr(r0);
         // r0 = SBR_num_env_bands[SBR_bs_freq_res[ch][l]]
         r1 = r4 + r0;
         r0 = M[r1 + $aacdec.SBR_num_env_bands];
         M[r9 + $aac.mem.TMP + $aacdec.SBR_est_curr_env_freq_band_loop_bound] = r0;

         // r5 = k_l
         r5 = M[I7, MK1];

         // r6 = l_i
         r6 = M3;
         // M3 = u_i
         r0 = M[I6, MK1];
         M3 = r0;

         r1 = M[r4 + $aacdec.SBR_kx];
         r0 = r5 - r1;
         I2 = I2 + r0;

         r0 = r6 + $aacdec.SBR_tHFAdj;
         r0 = r0 * ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD) (int);

         r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
         I1 = r1 + r0;
         r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
         I5 = r1 + r0;

         // for p=0:SBR_num_env_bands[SBR_bs_freq_res[ch][l]]-1,
         M1 = 0;
         e_curr_band_outer_loop_interp_off:

            // r7 = k_l[p]
            r7 = r5;
            // r5 = k_h[p+1]
            r5 = M[I7, MK1];

            r0 = r5 - r7;
            r1 = M3 - r6;
            r4 = r0 * r1 (int);

            r1 = 0xFFE;
            r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
            Null = r0 - $aacdec.ER_AAC_ELD;
            // Note we're now using the ELD one_over_div array for ELD and non-ELD for simplicity
            if NZ r4 = r4 AND r1;
            Words2Addr(r4);
            r0 = M[r9 + $aac.mem.SBR_est_curr_env_one_over_div_ptr];
            r0 = r0 + r4;
            r4 = M[r0 - (1*ADDR_PER_WORD)];

            // nrg = 0
            rMAC = 0;

            Words2Addr(r7);
            I0 = I1 + r7;
            I4 = I5 + r7;
            Addr2Words(r7);

            // for i=(l_i+SBR_tHFAdj):(u_i+SBR_tHFAdj)-1,
            r3 = r6;
            e_curr_envelope_inner_loop_interp_off:

               // for j=k_l:k_h-1,
               r10 = r5 - r7;
               r0 = M[I0, MK1];

               do e_curr_band_inner_loop_interp_off;

                  // nrg += magnitude(X_sbr[ch][j][i]) ^ 2
                  rMAC = rMAC + r0 * r0,
                   r1 = M[I4, MK1];

                  rMAC = rMAC + r1 * r1,
                   r0 = M[I0, MK1];

               e_curr_band_inner_loop_interp_off:

               r0 = 64 - r5;
               r0 = r0 + r7;
               Words2Addr(r0);
               I4 = I4 + r0;
               r0 = r0 - ADDR_PER_WORD;
               I0 = I0 + r0;

               r3 = r3 + 1;
               Null = M3 - r3;
            if GT jump e_curr_envelope_inner_loop_interp_off;


            // SBR_E_curr[ch][m][l] = nrg / div
            r1 = SIGNDET rMAC;
            rMAC = rMAC LSHIFT r1;
            r0 = 24 - r1;

            call $aacdec.sbr_fp_mult_frac;

            // for k=k_l:k_h-1,
            r10 = r5 - r7;

            do write_e_curr_band_values;
               M[I2, MK1] = r2;
            write_e_curr_band_values:

            r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_est_curr_env_freq_band_loop_bound];
            M1 = M1 + 1;
            Null = r0 - M1;
         if GT jump e_curr_band_outer_loop_interp_off;

         // update SBR_E_curr pointers for next envelope
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = M[r1 + $aacdec.SBR_kx];
         r0 = r5 - r0;
         r1 = M[r1 + $aacdec.SBR_M];
         r1 = r1 - r0;
         Words2Addr(r1);
         I2 = I2 + r1;

         M0 = M0 + MK1;
         r0 = r8 + r9;
         r0 = M[$aac.mem.SBR_bs_num_env + r0];
         Words2Addr(r0);
         Null = r0 - M0;
      if GT jump e_curr_envelope_outer_loop_interp_off;

      r5 = r8;

   exit:


   PROFILER_STOP(&$aacdec.profile_sbr_estimate_current_envelope_internal)

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
