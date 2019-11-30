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
//    $aacdec.sbr_channel_pair_element
//
// DESCRIPTION:
//    Get sbr channel pair element information
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - all plus $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_channel_pair_element;
   .CODESEGMENT AACDEC_SBR_CHANNEL_PAIR_ELEMENT_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_channel_pair_element:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.SBR_CHANNEL_PAIR_ELEMENT_ASM.SBR_CHANNEL_PAIR_ELEMENT.SBR_CHANNEL_PAIR_ELEMENT.PATCH_ID_0, r3)
#endif   
   

   r0 = 1;
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r1 + $aacdec.SBR_HF_reconstruction] = r0;

   // SBR_bs_data_extra = getbits(1)
   call $aacdec.get1bit;

   // if(SBR_bs_data_extra)
   if NZ call $aacdec.get1byte;

   // SBR_bs_coupling
   call $aacdec.get1bit;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r0 + $aacdec.SBR_bs_coupling] = r1;

   // if(SBR_bs_coupling)
   if Z jump no_coupling;

      // sbr_grid(0)
      r5 = 0;
      PROFILER_START(&$aacdec.profile_sbr_grid)

      r1 = $aacdec.sbr_grid;
#ifdef AACDEC_ELD_ADDITIONS
      r2 = $aacdec.eld_sbr_grid;
      r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r0 - $aacdec.ER_AAC_ELD;
      if Z r1 = r2;
#endif
      call r1;

      PROFILER_STOP(&$aacdec.profile_sbr_grid)

      // sbr.bs_frame_class(2) = sbr.bs_frame_class(1);
      // sbr.bs_num_env(2) = sbr.bs_num_env(1);
      // sbr.bs_num_noise(2) = sbr.bs_num_noise(1);
      // sbr.bs_pointer(2) = sbr.bs_pointer(1);
      r0 = M[r9 + $aac.mem.SBR_bs_frame_class];
      M[r9 + $aac.mem.SBR_bs_frame_class + (1*ADDR_PER_WORD)] = r0;
      r10 = M[r9 + $aac.mem.SBR_bs_num_env];
      M[r9 + $aac.mem.SBR_bs_num_env + (1*ADDR_PER_WORD)] = r10;
      r4 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r1 = M[r4 + $aacdec.SBR_bs_num_noise];
      r10 = r10 + 1;
      M[r4 + $aacdec.SBR_bs_num_noise + (1*ADDR_PER_WORD)] = r1;
      r0 = M[r9 + $aac.mem.SBR_bs_pointer];
      M[r9 + $aac.mem.SBR_bs_pointer + (1*ADDR_PER_WORD)] = r0;


      // for n=0:sbr.bs_num_env(1),
      //    sbr.t_E(2, n+1) = sbr.t_E(1, n+1);
      //    sbr.bs_freq_res(2, n+1) = sbr.bs_freq_res(1, n+1);
      // end;
      I4 = r9 + $aac.mem.SBR_t_E;
      I1 = r9 + $aac.mem.SBR_t_E + (6*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      I2 = r0 + $aacdec.SBR_bs_freq_res;
      I5 = r0 + $aacdec.SBR_bs_freq_res + (6*ADDR_PER_WORD);
      do copy_loop_1;
         r0 = M[I4, MK1],
          r4 = M[I2, MK1];
         M[I1, MK1] = r0,
          M[I5, MK1] = r4;
      copy_loop_1:


      // for n=0:sbr.bs_num_noise(1),
      //    sbr.t_Q(2, n+1) = sbr.t_Q(1, n+1);
      // end;
      r10 = r1 + 1;
      I4 = r9 + $aac.mem.SBR_t_Q;
      I1 = r9 + $aac.mem.SBR_t_Q + (3*ADDR_PER_WORD);
      do copy_loop_2;
         r0 = M[I4, MK1];
         M[I1, MK1] = r0;
      copy_loop_2:


      PROFILER_START(&$aacdec.profile_sbr_dtdf)
      // sbr_dtdf(0)
      r5 = 0; // channel
      call $aacdec.sbr_dtdf;

      // sbr_dtdf(1)
      r5 = 1; // channel
      call $aacdec.sbr_dtdf;
      PROFILER_STOP(&$aacdec.profile_sbr_dtdf)

      PROFILER_START(&$aacdec.profile_sbr_invf)
      // sbr_invf(0)
      r5 = 0; // channel
      call $aacdec.sbr_invf;
      PROFILER_STOP(&$aacdec.profile_sbr_invf)

      // for n=0:sbr.Nq-1,
      //    sbr.bs_invf_mode(2, n+1) = sbr.bs_invf_mode(1, n+1);
      // end;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r10 = M[r0 + $aacdec.SBR_Nq];
      I4 = r9 + $aac.mem.SBR_bs_invf_mode;
      I1 = r9 + $aac.mem.SBR_bs_invf_mode + (5*ADDR_PER_WORD);
      do copy_loop_3;
         r0 = M[I4, MK1];
         M[I1, MK1] = r0;
      copy_loop_3:


      PROFILER_START(&$aacdec.profile_sbr_envelope)
      // sbr_envelope(0)
      r5 = 0; // channel
      call $aacdec.sbr_envelope;
      PROFILER_STOP(&$aacdec.profile_sbr_envelope)

      // sbr_noise(0)
      PROFILER_START(&$aacdec.profile_sbr_noise)
      call $aacdec.sbr_noise;
      PROFILER_STOP(&$aacdec.profile_sbr_noise)

      PROFILER_START(&$aacdec.profile_sbr_envelope)
      // sbr_envelope(1)
      r5 = 1; //channel
      call $aacdec.sbr_envelope;
      PROFILER_STOP(&$aacdec.profile_sbr_envelope)

      // sbr_noise(1)
      PROFILER_START(&$aacdec.profile_sbr_noise)
      call $aacdec.sbr_noise;
      PROFILER_STOP(&$aacdec.profile_sbr_noise)
      jump end_if_coupling;



   no_coupling:
      PROFILER_START(&$aacdec.profile_sbr_grid)
      // sbr_grid(0)
      r5 = 0;
      r1 = $aacdec.sbr_grid;
#ifdef AACDEC_ELD_ADDITIONS
      r2 = $aacdec.eld_sbr_grid;
      r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r0 - $aacdec.ER_AAC_ELD;
      if Z r1 = r2;
#endif
      call r1;


      // sbr_grid(1)
      r5 = 1;
      r1 = $aacdec.sbr_grid;
#ifdef AACDEC_ELD_ADDITIONS
      r2 = $aacdec.eld_sbr_grid;
      r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
      Null = r0 - $aacdec.ER_AAC_ELD;
      if Z r1 = r2;
#endif
      call r1;

      PROFILER_STOP(&$aacdec.profile_sbr_grid)

      PROFILER_START(&$aacdec.profile_sbr_dtdf)
      // sbr_dtdf(0)
      r5 = 0; // channel
      call $aacdec.sbr_dtdf;

      // sbr_dtdf(1)
      r5 = 1; // channel
      call $aacdec.sbr_dtdf;
      PROFILER_STOP(&$aacdec.profile_sbr_dtdf)

      PROFILER_START(&$aacdec.profile_sbr_invf)
      // sbr_invf(0)
      r5 = 0; // channel
      call $aacdec.sbr_invf;

      // sbr_invf(1)
      r5 = 1; // channel
      call $aacdec.sbr_invf;
      PROFILER_STOP(&$aacdec.profile_sbr_invf)


      PROFILER_START(&$aacdec.profile_sbr_envelope)
      // sbr_envelope(0)
      r5 = 0; // channel
      call $aacdec.sbr_envelope;

      // sbr_envelope(1)
      r5 = 1; // channel
      call $aacdec.sbr_envelope;
      PROFILER_STOP(&$aacdec.profile_sbr_envelope)

      PROFILER_START(&$aacdec.profile_sbr_noise)
      // sbr_noise(0)
      r5 = 0; // channel
      call $aacdec.sbr_noise;

      // sbr_noise(1)
      r5 = 1; // channel
      call $aacdec.sbr_noise;
      PROFILER_STOP(&$aacdec.profile_sbr_noise)

   end_if_coupling:


   // sbr.bs_add_harmonic(1, 1:64) = 0;
   // sbr.bs_add_harmonic(2, 1:64) = 0;
   //
   // sbr.bs_add_harmonic_flag(1) = getbits(1);
   // sbr.num_sbr_bits = sbr.num_sbr_bits + 1;
   //
   // if ( sbr.bs_add_harmonic_flag(1) == 1 )
   //    sbr_sinusoidal_coding(0);
   // end;
   //
   // sbr.bs_add_harmonic_flag(2) = getbits(1);
   // sbr.num_sbr_bits = sbr.num_sbr_bits + 1;
   //
   // if ( sbr.bs_add_harmonic_flag(2) == 1 )
   //    sbr_sinusoidal_coding(1);
   // end;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   I1 = r0 + $aacdec.SBR_bs_add_harmonic;
   r6 = 0;

   add_harmonic_loop:
      call $aacdec.get1bit;
      r7 = -64;
      r10 = r9 + r6;
      M[$aac.mem.SBR_bs_add_harmonic_flag + r10] = r1;

      if Z jump no_sinusoidal_coding;

         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r10 = M[r0 + $aacdec.SBR_Nhigh];
         r7 = r10 - 64;

         do sinusoidal_coding_loop;
            call $aacdec.get1bit;
            M[I1, MK1] = r1;
         sinusoidal_coding_loop:
      no_sinusoidal_coding:

      r10 = -r7;
      r0 = 0;
      do clear_bs_add_harmonic_loop;
         M[I1, MK1] = r0;
      clear_bs_add_harmonic_loop:

      r6 = r6 + (1*ADDR_PER_WORD);
      Null = r6 - (2*ADDR_PER_WORD);
   if NZ jump add_harmonic_loop;




   // if(sbr.bs_coupling==1)
   //    sbr_envelope_noise_dequantisation_coupling_mode();
   // else
   //    sbr_envelope_noise_dequantisation(0);
   //    sbr_envelope_noise_dequantisation(1);
   // end;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = M[r0 + $aacdec.SBR_bs_coupling];
   if NZ jump coupling_mode;
      PROFILER_START(&$aacdec.profile_sbr_envelope_noise_dequantisation)
      r5 = 0;
      call $aacdec.sbr_envelope_noise_dequantisation;
      r5 = 1;
      call $aacdec.sbr_envelope_noise_dequantisation;
      PROFILER_STOP(&$aacdec.profile_sbr_envelope_noise_dequantisation)
      jump done_dequant;

   coupling_mode:
      PROFILER_START(&$aacdec.profile_sbr_envelope_noise_dequantisation_coupling_mode)
      call $aacdec.sbr_envelope_noise_dequantisation_coupling_mode;
      PROFILER_STOP(&$aacdec.profile_sbr_envelope_noise_dequantisation_coupling_mode)

   done_dequant:
   call $aacdec.get1bit;

   // if(SBR_bs_extended_data)
   Null = r1;
   if Z jump no_extended_data;
      // SBR_bs_extension_size
      call $aacdec.get4bits;

      Null = r1 - 15;
      if NZ jump count_not_15;
         // SBR_bs_esc_count
         call $aacdec.get1byte;
         r1 = r1 + 15;
      count_not_15:

#ifdef AACDEC_PS_ADDITIONS

      r5 = M[r9 + $aac.mem.PS_info_ptr];
      M[r5 +  $aacdec.PS_EXT_DATA_READ] = Null;
      // r8 = num_bits_left = cnt * 8
      r8 = r1 * 8 (int);
      if Z jump no_extended_data;
      // while(num_bits_left > 7)
      ps_extension_data_loop:
         // SBR_bs_extension_id = getbits(2)
         call $aacdec.get2bits;
         // num_bits_left -= 2
         r8 = r8 - 2;
         // if(SBR_bs_extension_id == EXT_ID_PS)
         Null = r1 - $aacdec.PS_EXT_ID_PARAMETRIC_STEREO;
         if NZ jump end_if_ext_id_ps;
            r0 = 1;
            M[r9 + $aac.mem.PS_present] = r0;

            // if(PS_EXT_DATA_READ == 0)
            Null = M[r5+ $aacdec.PS_EXT_DATA_READ];
            if NZ jump ps_ext_data_already_read;
               M[r5 + $aacdec.PS_EXT_DATA_READ] = r0;
               call $aacdec.ps_data;
               jump end_if_ext_id_ps;
            // else
            ps_ext_data_already_read:
               call $aacdec.get6bits;
               // num_bits_left -= 6
               r8 = r8 - 6;

         end_if_ext_id_ps:
      Null = r8 - 7;
      if GT jump ps_extension_data_loop;

      // if(num_bits_left > 0) getbits(num_bits_left)
      r0 = r8;
      if NZ call $aacdec.getbits;

#else

      r10 = r1;
      Null = r10 AND 1;
      if Z jump cnt_not_odd;
         call $aacdec.get1byte;
         r10 = r10 - 1;
      cnt_not_odd:

      r10 = r10 LSHIFT -1;
      do discard_cnt_bytes;
         call $aacdec.get2bytes;
      discard_cnt_bytes:

#endif

   no_extended_data:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
