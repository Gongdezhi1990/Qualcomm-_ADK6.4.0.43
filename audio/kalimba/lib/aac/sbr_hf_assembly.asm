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
//    $aacdec.sbr_hf_assembly
//
// DESCRIPTION:
//    Applies variable gain to X_sbr and adds in variable scale noise and
//    sinusoids as appropriate
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I0-I5, I7, L0, L5, M0-M3
//    - $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_hf_assembly;
   .CODESEGMENT AACDEC_SBR_HF_ASSEMBLY_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_hf_assembly:

   // push rLink onto stack
   push rLink;

   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_HF_ASSEMBLY_ASM.SBR_HF_ASSEMBLY.SBR_HF_ASSEMBLY.PATCH_ID_0, r6)
#endif

   Words2Addr(r5); // In all code below r5 is a channel address offset

   // allocate temporary memory for SBR_G_filt[]
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r1 + (2048*ADDR_PER_WORD);
   M[r1 + $aacdec.SBR_G_filt_ptr] = r0;

   // allocate temporary memory for SBR_Q_filt[]
   r0 = r1 + (2048*ADDR_PER_WORD) + (49*ADDR_PER_WORD);
   M[r1 + $aacdec.SBR_Q_filt_ptr] = r0;

   // if(SBR_reset == 1)   SBR_f_index_noise = 0;
   //    else  SBR_f_index_noise = SBR_index_noise_prev[ch]
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r6 = r0 + r5;
   r6 = M[r6 + $aacdec.SBR_index_noise_prev];
   Null = M[r0 + $aacdec.SBR_reset];
   if NZ r6 = 0;

   M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_noise] = r6;
   Words2Addr(r6);

   // map SBR_V_noise table into cache
   r0 = &$aacdec.sbr_v_noise;
   call $mem.ext_window_access_as_ram;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_v_noise_start] = r0;
   I0 = r0 + r6;
   I0 = I0 + MK1;

   // SBR_f_index_sine = SBR_psi_is_prev[ch]
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = r0 + r5;
   r0 = M[r0 + $aacdec.SBR_psi_is_prev];
   M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_sine] = r0;


   // for l=0:SBR_bs_num_env[ch]-1,
   r8 = 0;
   outer_envelope_loop:

      Addr2Words(r5);
      call $aacdec.sbr_hf_assembly_initialise_outer_loop_iteration;
      Words2Addr(r5);


      // for i=SBR_t_E[ch][l]:SBR_t_E[ch][l+1]-1,

      envelope_time_border_loop:

         // r6 = SBR_GQ_index[ch]
         r10 = M[r9 + $aac.mem.SBR_info_ptr];
         r6 = r10 + r5;
         r6 = M[r6 + $aacdec.SBR_GQ_index];

         // I1 <- SBR_G_filt[0]
         r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r0 = M[r1 + $aacdec.SBR_G_filt_ptr];
         I1 = r0;

         // I4 <- SBR_Q_filt[0]
         r0 = M[r1 + $aacdec.SBR_Q_filt_ptr];
         I4 = r0;

         r10 = M[r10 + $aacdec.SBR_M];

         // if(SBR_H_SL == 4)
         //    SBR_G_filt[0:SBR_M-1] = 0; SBR_Q_filt[0:SBR_M-1] = 0;
         Null = M[r9 + $aac.mem.TMP + $aacdec.SBR_H_SL];
         if Z jump dont_clear_g_and_q_filt;

            r0 = 0;
            do clear_g_and_q_filt_loop;
               M[I1, MK1] = r0,
                M[I4, MK1] = r0;
            clear_g_and_q_filt_loop:
            r10 = M[r9 + $aac.mem.SBR_info_ptr];
            r10 = M[r10 + $aacdec.SBR_M];
         dont_clear_g_and_q_filt:

         // SBR_G_temp[ch][SBR_GQ_index[ch]][0:SBR_M-1] = SBR_G_lim_boost[l][0:SBR_M-1]
         // SBR_Q_temp[ch][SBR_GQ_index[ch]][0:SBR_M-1] = SBR_Q_M_lim_boost[l][0:SBR_M-1]

         r1 = r5 * 5 (int);
         Words2Addr(r6);
         r1 = r1 + r6;
         Addr2Words(r6);

         Words2Addr(r8);

         r2 = M[r9 + $aac.mem.SBR_info_ptr];
         r3 = r2 + r8;
         r1 = r2 + r1;
         r0 = M[r3 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
         M[r1 + $aacdec.SBR_G_TEMP_BLOCK_EXPONENT_ARRAY_FIELD] = r0;

         r0 = M[r3 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
         M[r1 + $aacdec.SBR_Q_TEMP_BLOCK_EXPONENT_ARRAY_FIELD] = r0;

         r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope];
         r0 = r0 * r8 (int);

         r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r2 = M[r2 + $aacdec.SBR_G_lim_boost_mantissa_ptr];
         r3 = r0 + r2;
         M[r1 + $aacdec.SBR_G_TEMP_LIM_ENV_ADDR_ARRAY_FIELD] = r3;

         r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         r2 = M[r2 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr];
         r2 = r0 + r2;
         M[r1 + $aacdec.SBR_Q_TEMP_LIM_ENV_ADDR_ARRAY_FIELD] = r2;

         Addr2Words(r8);


         PROFILER_START(&$aacdec.profile_sbr_hf_assembly_loop_2)
         // if(SBR_H_SL == 0)
         Null = M[r9 + $aac.mem.TMP + $aacdec.SBR_H_SL];
         if NZ jump smooth_the_gain_values;
            // I3 <- SBR_G_temp[ch][SBR_GQ_index[ch]][0]
            I3 = r3;
            // I2 <- SBR_Q_temp[ch][SBR_GQ_index[ch]][0]
            I2 = r2;

            r0 = r5 * 5 (int);
            Words2Addr(r6);
            r1 = r0 + r6;
            Addr2Words(r6);

            // SBR_G_filt[0:SBR_M-1] = SBR_G_temp[ch][SBR_GQ_index[ch]][0:SBR_M-1]
            // SBR_Q_filt[0:SBR_M-1] = SBR_Q_temp[ch][SBR_GQ_index[ch]][0:SBR_M-1]
            r2 = M[r9 + $aac.mem.SBR_info_ptr];
            r3 = r2 + r1;
            r0 = M[r3 + $aacdec.SBR_G_TEMP_BLOCK_EXPONENT_ARRAY_FIELD];
            M[r2 + $aacdec.SBR_G_FILT_BLOCK_EXPONENT_FIELD] = r0;

            r0 = M[r3 + $aacdec.SBR_Q_TEMP_BLOCK_EXPONENT_ARRAY_FIELD];
            M[r2 + $aacdec.SBR_Q_FILT_BLOCK_EXPONENT_FIELD] = r0;

            // for 0:SBR_M-1,

            do g_and_q_filt_no_smoothing_loop;
               r0 = M[I3, MK1];
               r1 = M[I2, MK1];

               M[I1, MK1] = r0,
                M[I4, MK1] = r1;
            g_and_q_filt_no_smoothing_loop:

            jump g_and_q_filt_assigned;
         smooth_the_gain_values:


            // make the initial value of each block_exponent = -(2^15)
            r6 = M[r9 + $aac.mem.SBR_info_ptr];
            r0 = -(1<<15);
            M[r6 + $aacdec.SBR_G_FILT_BLOCK_EXPONENT_FIELD] = r0;
            M[r6 + $aacdec.SBR_Q_FILT_BLOCK_EXPONENT_FIELD] = r0;

            r6 = r6 + r5;
            r6 = M[r6 + $aacdec.SBR_GQ_index];

            Addr2Words(r5);
            call $aacdec.sbr_hf_assembly_calc_gain_filters_smoothing_mode;
            Words2Addr(r5);
         g_and_q_filt_assigned:

         PROFILER_STOP(&$aacdec.profile_sbr_hf_assembly_loop_2)

         Addr2Words(r5);
         call $aacdec.sbr_hf_assembly_initialise_signal_gain_and_component_loop;
         // Note above function sets r5 is be something different (not chan number)
         M0 = 0;
         M3 = MK1;


         PROFILER_START(&$aacdec.profile_sbr_hf_assembly_loop_1)
         // for m=0:SBR_M-1,

         do x_sbr_band_loop;

            // Applying Gain

            rMAC = M[I1, 0],    // real(X_sbr[ch][m + SBR_kx][i + SBR_tHFAdj])
             r1 = M[I7, MK1];    // SBR_G_filt[m]

            // r0 = real(W1[m]) = SBR_G_filt[m] * real(X_sbr[ch][m + SBR_kx][i + SBR_tHFAdj]);
            rMAC = rMAC * r1,
             r3 = M[I3, MK1],    // SBR_S_M_boost[l][m]
             r2 = M[I4, 0];    // imag(X_sbr[ch][m + SBR_kx][i + SBR_tHFAdj])
            r0 = rMAC ASHIFT r4;

            // r1 = imag(W1[m]) = SBR_G_filt[m] * imag(X_sbr[ch][m + SBR_kx][i + SBR_tHFAdj]);
            rMAC = r1 * r2;

            r1 = rMAC ASHIFT r4,
             rMAC = M[I2, M0];    // SBR_Q_filt[m]

            r3 = r3 + M2;
            if NZ jump s_m_boost_calculations;
            // Adding Noise Component

            r5 = -r5,   // rev = -rev
             r2 = M[I0, M3];    // real(V_noise[SBR_f_index_noise])

            // r2 = SBR_Q_filt[m] * real(V_noise[SBR_f_index_noise])
            rMAC = rMAC * r2,
             r3 = M[I5, MK1];    // imag(V_noise[SBR_f_index_noise])
            r2 = rMAC ASHIFT r8,
             rMAC = M[I2, M3];    // SBR_Q_filt[m]

            // r3 = SBR_Q_filt[m] * imag(V_noise[SBR_f_index_noise])
            rMAC = rMAC * r3;
            r3 = rMAC ASHIFT r8;

            jump calculate_x_sbr_new_values;

            s_m_boost_calculations:
            // Adding Sinusoidal Component
            r5 = -r5,   // rev = -rev
             r2 = M[I0,M3];   // dummy read to implement SBR_f_index_noise = bitand(SBR_f_index_noise+1,511)
            I5 = I0 + L0;

            r3 = r3 - M2,
             rMAC = M[I2, M3];    // SBR_Q_filt[m] dummy read to increment I2

            if Z jump write_x_sbr_new_values;

            // real(psi) = SBR_S_M_boost[l][m] * phi_re_sin[SBR_f_index_sine]
            r2 = r6 * r3 (int);
            r2 = r2 ASHIFT r7;

            // imag(psi) = SBR_S_M_boost[l][m] * rev * phi_im_sin[SBR_f_index_sine]
            r3 = r3 * r5 (int);
            r3 = r3 ASHIFT r7;

            calculate_x_sbr_new_values:

            // real(X_sbr[ch][m+SBR_kx][i+SBR_tHFAdj]) = (SBR_Q_filt[m]*real(V_noise[SBR_f_index_noise])) + real(W1[m])
            r0 = r0 + r2;

            // imag(X_sbr[ch][m+SBR_kx][i+SBR_tHFAdj]) = (SBR_Q_filt[m]*imag(V_noise[SBR_f_index_noise])) + imag(W1[m])
            r1 = r1 + r3;

            write_x_sbr_new_values:

            M[I1, MK1] = r0,    // real(X_sbr[ch][m + SBR_kx][i + SBR_tHFAdj])
             M[I4, MK1] = r1;    //  imag(X_sbr[ch][m + SBR_kx][i + SBR_tHFAdj])

         x_sbr_band_loop:


         PROFILER_STOP(&$aacdec.profile_sbr_hf_assembly_loop_1)


         r5 = M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_save_ch];
         r8 = M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_save_l];
         Words2Addr(r5);

         L0 = 0;
         L5 = 0;

         push Null; pop B0;
         push Null; pop B5;



         // SBR_GQ_index[ch] += 1
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r0 = r0 + r5;
         r2 = M[r0 + $aacdec.SBR_GQ_index];
         r2 = r2 + 1;
         // if(SBR_GQ_index[ch] >= 5)
         Null = r2 - 5;
         if GE r2 = 0;
         M[r0 + $aacdec.SBR_GQ_index] = r2;


         M1 = M1 + 1;
         r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_time_envelope_border];
         Null = r0 - M1;
      if GT jump envelope_time_border_loop;


      r8 = r8 + 1;
      r0 = r5 + r9;
      r0 = M[$aac.mem.SBR_bs_num_env + r0];
      Null = r0 - r8;
   if GT jump outer_envelope_loop;


   Addr2Words(r5);
   call $aacdec.sbr_hf_assembly_save_persistent_gain_signal_envelopes;
   Words2Addr(r5);

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_M];
   r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope];
   r1 = r1 * 3 (int);
   r0 = r0 + r1;
   r1 = r5 + r9;
   r1 = M[$aac.mem.SBR_bs_num_env + r1];
   r0 = r0 * r1 (int);
   call $aacdec.frame_mem_pool_free;

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
