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
//    $aacdec.sbr_envelope_noise_dequantisation_coupling_mode
//
// DESCRIPTION:
//    Decode envelope and noise floor scalefactors in coupling mode
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - SBR_E_orig and SBR_Q_orig
//
// TRASHED REGISTERS:
//    - all plus (except r5) first 2 elements of $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_envelope_noise_dequantisation_coupling_mode;
   .CODESEGMENT AACDEC_SBR_ENVELOPE_NOISE_DEQUANTISATION_COUPLING_MODE_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_envelope_noise_dequantisation_coupling_mode:

   // push rLink onto stack
   push rLink;

   r0 = &$aacdec.sbr_q_div_table_rows;
   call $mem.ext_window_access_as_ram;
   M3 = r0;

   r0 = I0;
   M[r9 + $aac.mem.GETBITS_SAVED_I0] = r0;
   r0 = L0;
   M[r9 + $aac.mem.GETBITS_SAVED_L0] = r0;
   L0 = 0;

   // left
   // SBR_E_orig_mantissa over-writes SBR_E_envelope
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = M[r1 + $aacdec.SBR_E_envelope_base_ptr];
   M[r9 + $aac.mem.SBR_E_orig_mantissa_base_ptr] = r1;
   I2 = r1;

   // amp0 = -1
   r7 = -1;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_amp_res];
   if NZ r7 = 0;  // amp0 = 0
   M[r9 + $aac.mem.TMP] = r7;


   // right
   // SBR_E_orig_mantissa over-writes SBR_E_envelope
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = M[r1 + $aacdec.SBR_E_envelope_base_ptr + (1*ADDR_PER_WORD)];
   M[r9 + $aac.mem.SBR_E_orig_mantissa_base_ptr + (1*ADDR_PER_WORD)] = r1;
   I5 = r1;

   // amp1 = -1
   r6 = -1;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r0 + $aacdec.SBR_amp_res + (1*ADDR_PER_WORD)];
   if NZ r6 = 0;  // amp1 = 0
   M[r9 + $aac.mem.TMP + (1*ADDR_PER_WORD)] = r6;


   // for l=0:SBR_bs_num_env[1]-1,
   // r7 = 0;

   M0 = 0;
   e_orig_outer_loop:
      // for k=0:SBR_num_env_bands[SBR_bs_freq_res[1][l]]-1,
      r3 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = r3 + M0;
      r0 = M[r0 + $aacdec.SBR_bs_freq_res];
      Words2Addr(r0);
      r10 = r3 + r0;
      r10 = M[r10 + $aacdec.SBR_num_env_bands];

      // SBR_E_orig_mantissa(1,:,:)
      I1 = I2;
      // SBR_E_orig_mantissa(2,:,:)
      I7 = I5;

      do e_orig_inner_loop;


         r0 = M[I7, 0], // r3 = SBR_E_envelope[2][k][l]
          r3 = M[I1, 0]; // r3 = SBR_E_envelope[1][k][l]
         // exp1 = SBR_E_envelope[2][k][l] << amp1
         r8 = r0 LSHIFT r6;

         // exp0 = (SBR_E_envelope[1][k][l] << amp0) +1
         r0 = r3 LSHIFT r7;
         r0 = r0 + 1;
         // if((exp0 < 0)||(exp0 >= 64)||(exp1 < 0)||(exp1 > 24))
         if NEG jump zero_e_orig_value;
            Null = r0 - 64;
            if GE jump zero_e_orig_value;
               Null = r8;
               if NEG jump zero_e_orig_value;
                  Null = r8 - 24;
                  if GT jump zero_e_orig_value;
                     // SBR_E_orig[ch][k][l] = 2 ^ (exp0 + SBR_E_deg_offset - 16)
                     r0 = r0 + (($aacdec.SBR_E_deg_offset - 16) + 8);
#ifdef KAL_ARCH4
                     r1 = 0x400000;
#else
                     r1 = 0x4000;
#endif

                     // if((amp0==-1)&&(bitand(SBR_E_envelope[1][k][l],1)==1))
                     Null = r7 + 1;
                     if NZ jump e_orig_value_nextstep;
                        Null = r3 AND 1;
                        if Z jump e_orig_value_nextstep;
#ifdef KAL_ARCH4
                        r1 = 0x5a8200;
#else
                        r1 = 0x5a82;
#endif


                     e_orig_value_nextstep:
                        rMAC = r1;
                        Words2Addr(r8);
                        r4 = M[r9 + $aac.mem.SBR_E_pan_tab_ptr];
                        r4 = M[r4 + r8];
                        Addr2Words(r8);
                        call $aacdec.sbr_fp_mult_frac;
                        r8 = 24 - r8;
                        rMAC = r1;
                        Words2Addr(r8);
                        r4 = M[r9 + $aac.mem.SBR_E_pan_tab_ptr];
                        r4 = M[r4 + r8];
                        Addr2Words(r8);
                        M[I1, MK1] = r2;
                        call $aacdec.sbr_fp_mult_frac;

                        jump e_orig_value_assigned;

         zero_e_orig_value:
               // SBR_E_orig[ch][k][l] = 0
               // SBR_E_orig over-writes SBR_E_envelope
               r2 = 0;
               M[I1, MK1] = r2;

         e_orig_value_assigned:

         M[I7, MK1] = r2;
      e_orig_inner_loop:

      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = M[r0 + $aacdec.SBR_num_env_bands + (1*ADDR_PER_WORD)];
      Words2Addr(r0);
      I2 = I2 + r0;
      I5 = I5 + r0;

      M0 = M0 + (1*ADDR_PER_WORD);
      r0 = M[r9 + $aac.mem.SBR_bs_num_env];
      Words2Addr(r0);
      Null = r0 - M0;
   if GT jump e_orig_outer_loop;




   // left
   // SBR_Q_orig over-writes SBR_Q_envelope
   r6 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = M[r6 + $aacdec.SBR_Q_envelope_base_ptr];
   I2 = r1;
   M0 = r9 + $aac.mem.SBR_Q_orig;
   I3 = r9 + $aac.mem.SBR_Q_orig2;

   // right
   // SBR_Q_orig over-writes SBR_Q_envelope
   r1 = M[r6 + $aacdec.SBR_Q_envelope_base_ptr + (1*ADDR_PER_WORD)];
   I5 = r1;

   M1 = r9 + $aac.mem.SBR_Q_orig  + (10*ADDR_PER_WORD);
   I6 = r9 + $aac.mem.SBR_Q_orig2 + (10*ADDR_PER_WORD);

   // for l=0:SBR_bs_num_noise[1]-1,
   r6 = M[r6 + $aacdec.SBR_bs_num_noise];
   r7 = 0;
   r8 = 9;
   q_orig_outer_loop:

      // for k=0:SBR_Nq-1,
      r10 = M[r9 + $aac.mem.SBR_info_ptr];
      r10 = M[r10 + $aacdec.SBR_Nq];

      // SBR_Q_orig(1,:,:)
      I1 = I2;
      // SBR_Q_orig2(1,:,:)
      I4 = I3;
      // SBR_Q_orig(2,:,:)
      I7 = I5;
      // SBR_Q_orig2(2,:,:)
      I0 = I6;

      M2 = 0;

      do q_orig_inner_loop;

         // if((SBR_Q_envelope[1][k][l] < 0)||(SBR_Q_envelope[1][k][l] > 30)||(SBR_Q_envelope[2][k][l] < 0)||(SBR_Q_envelope[2][k][l] > 24))
         r3 = M[I1, MK1],
          r4 = M[I7, MK1];
         Null = r3;
         if NEG jump zero_q_orig_value;
            Null = r3 - 30;
            if GT jump zero_q_orig_value;
               Null = r4;
               if NEG jump zero_q_orig_value;
                  Null = r4 - 24;
                  if GT jump zero_q_orig_value;
                     r4 = r4 ASHIFT -1;

                     call $aacdec.sbr_read_qdiv_tables;

                     M[I4, MK1] = r0;
                     r0 = M0 + M2;
                     M[r0] = r1;

                     r4 = 12 - r4;

                     call $aacdec.sbr_read_qdiv_tables;

                     M[I0, MK1] = r0;
                     r0 = M1 + M2;
                     M[r0] = r1;

                     jump q_orig_value_assigned;
         zero_q_orig_value:
            // SBR_Q_orig[ch][k][l] = 0
            r0 = 0;

            r1 = M0 + M2;
            M[r1] = Null;

            M[I4, MK1] = r0;

            r1 = M1 + M2;
            M[r1] = Null;
            M[I0, MK1] = r0;
         q_orig_value_assigned:

         M2 = M2 + (1*ADDR_PER_WORD);

      q_orig_inner_loop:

      r4 = M[r9 + $aac.mem.SBR_info_ptr];

      I2 = I2 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);
      I3 = I3 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);
      I5 = I5 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);
      I6 = I6 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);

      M0 = M0 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);
      M1 = M1 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);

      r7 = r7 + 1;
      Null = r6 - r7;
   if GT jump q_orig_outer_loop;


   r0 = M[r9 + $aac.mem.GETBITS_SAVED_I0];
   I0 = r0;
   r0 = M[r9 + $aac.mem.GETBITS_SAVED_L0];
   L0 = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
