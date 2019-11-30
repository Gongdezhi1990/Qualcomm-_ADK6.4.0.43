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
//    $aacdec.sbr_envelope_noise_dequantisation
//
// DESCRIPTION:
//    Decode envelope and noise floor scalefactors
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r4, r6-r8, r10, I1-I6, M3
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_envelope_noise_dequantisation;
   .CODESEGMENT AACDEC_SBR_ENVELOPE_NOISE_DEQUANTISATION_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_envelope_noise_dequantisation:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5);

   r0 = &$aacdec.sbr_q_div_table_rows;
   call $mem.ext_window_access_as_ram;
   M3 = r0;

   // SBR_E_orig_mantissa over-writes SBR_E_envelope
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = r1 + r5;
   r1 = M[r1 + $aacdec.SBR_E_envelope_base_ptr];
   r6 = r5 + r9;
   M[$aac.mem.SBR_E_orig_mantissa_base_ptr + r6] = r1;
   I2 = r1;


   // amp_res is either 0 or 1, we use this to decide how much to divide
   // SBR_E_envelope[][] by.
   // if amp_res = 0.  Divide by 2,  i.e. left shift by -1 = shift_amp
   // if amp_res = 1.  Divide by 1,  i.e. left shift by  0 = shift_amp
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = r0 + r5;
   r6 = M[r0 + $aacdec.SBR_amp_res];
   r6 = r6 - 1;

   // for l=0:SBR_bs_num_env[ch]-1,
   r7 = 0;
   e_orig_outer_loop:
      // for k=0:SBR_num_env_bands[SBR_bs_freq_res[ch][l]]-1,
      r0 = r5 * 6 (int);
      r0 = r0 + r7;

      r3 = M[r9 + $aac.mem.SBR_info_ptr];
      r1 = r3 + r0;
      r0 = M[r1 + $aacdec.SBR_bs_freq_res];
      Words2Addr(r0);
      r1 = r3 + r0;
      r10 = M[r1 + $aacdec.SBR_num_env_bands];

      // SBR_E_orig_mantissa
      I1 = I2;

      do e_orig_inner_loop;

         // r3 = SBR_E_envelope[ch][k][l]
         r3 = M[I1, 0];
         // exp = SBR_E_envelope[ch][k][l] << shift_amp
         r0 = r3 LSHIFT r6;
         // if((exp < 0)||(exp >= 64))
         Null = r0;
         if NEG jump zero_e_orig_value;
            Null = r0 - 64;
            if GE jump zero_e_orig_value;
               // SBR_E_orig[ch][k][l] = 2 ^ (exp + SBR_E_deg_offset)
               r0 = r0 + ($aacdec.SBR_E_deg_offset - 5);

               // store with mantissa shifted 11 bits left
               r1 = (1<< (DAWTH-13));

               // if((amp==-2)&&(bitand(SBR_E_envelope[ch][k][l],1)==1))
               Null = r6 + 1;
               if NZ jump e_orig_value_assigned;
                  Null = r3 AND 1;
                  if Z jump e_orig_value_assigned;
                     // SBR_E_orig[ch][k][l] *= sqrt(2)
                     //   because mantissa is known to be 2^11 just set the
                     //   mantissa to sqrt(2)*2^11 = 2896
                     r1 = 2896;
#ifdef KAL_ARCH4
   r1 = r1 LSHIFT 8;
#endif
                     jump e_orig_value_assigned;
            zero_e_orig_value:
               // SBR_E_orig[ch][k][l] = 0
               // SBR_E_orig over-writes SBR_E_envelope
               r0 = 0;
               r1 = 0;
         e_orig_value_assigned:

         // store E_orig value as [12 bits Exponent][ 12 or 20-bits Mantissa]
         r0 = r0 LSHIFT $aacdec.FLPT_EXPONENT_SHIFT;
         r1 = r1 + r0;

         M[I1, 0] = r1;

         I1 = I1 + MK1;

      e_orig_inner_loop:

      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = M[r0 + $aacdec.SBR_num_env_bands + (1*ADDR_PER_WORD)];
      Words2Addr(r0);
      I2 = I2 + r0;

      r7 = r7 + (1*ADDR_PER_WORD);
      r0 = r5 + r9;
      r0 = M[$aac.mem.SBR_bs_num_env + r0];
      Words2Addr(r0);
      Null = r0 - r7;
   if GT jump e_orig_outer_loop;


   // SBR_Q_orig over-writes SBR_Q_envelope
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = r1 + r5;
   r1 = M[r1 + $aacdec.SBR_Q_envelope_base_ptr];
   I5 = r1;

   r0 = r5 * 10 (int);
   r0 = r0 + r9;
   I2 = $aac.mem.SBR_Q_orig + r0;

   I3 = $aac.mem.SBR_Q_orig2 + r0;


   // for l=0:SBR_bs_num_noise[ch]-1,
   r7 = 0;
   r8 = 9;
   r4 = 6;
   q_orig_outer_loop:

      // SBR_Q_orig
      I1 = I2;
      // SBR_Q_orig2
      I4 = I3;
      // SBR_Q_envelope
      I6 = I5;

      // for k=0:SBR_Nq-1,
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r10 = M[r0 + $aacdec.SBR_Nq];
      r3 = M[I6, MK1];

      do q_orig_inner_loop;

         // if(SBR_Q_envelope[ch][k][l] < 0)||(SBR_Q_envelope[ch][k][l] > 30))
         Null = r3;
         if NEG jump zero_q_orig_value;
            Null = r3 - 30;
            if GT jump zero_q_orig_value;
               call $aacdec.sbr_read_qdiv_tables;
               M[I1, MK1] = r1,
                M[I4, MK1] = r0;
               jump q_orig_value_assigned;
         zero_q_orig_value:
            // SBR_Q_orig[ch][k][l] = 0
            r0 = 0;
            M[I1, MK1] = r0,
             M[I4, MK1] = r0;
         q_orig_value_assigned:
         r3 = M[I6, MK1];

      q_orig_inner_loop:

      I2 = I2 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);
      I3 = I3 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);
      I5 = I5 + ($aacdec.SBR_Nq_max*ADDR_PER_WORD);


      r7 = r7 + 1;
      r0 = r5 + $aacdec.SBR_bs_num_noise;
      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r1 + r0];
      Null = r0 - r7;
   if GT jump q_orig_outer_loop;

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
