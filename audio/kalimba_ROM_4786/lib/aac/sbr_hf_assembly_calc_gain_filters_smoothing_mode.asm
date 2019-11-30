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
//    $aacdec.sbr_hf_assembly_calc_gain_filters_smoothing_mode
//
// DESCRIPTION:
//    - calculates G_filt[0:SBR_M-1] (Gain Filter) and Q_filt[0:SBR_M-1] (Noise gain filter)
//    - when smoothing mode is active in the current envelope
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
.MODULE $M.aacdec.sbr_hf_assembly_calc_gain_filters_smoothing_mode;
   .CODESEGMENT AACDEC_SBR_HF_ASSEMBLY_CALC_GAIN_FILTERS_SMOOTHING_MODE_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_hf_assembly_calc_gain_filters_smoothing_mode:

   Words2Addr(r5); // In all code below r5 is a channel address offset

   // for j=0:4,
   r1 = 0;

   g_and_q_filt_smoothing_outer_loop:

      r4 = M[r9 + $aac.mem.SBR_info_ptr];
      r10 = M[r4 + $aacdec.SBR_M];
      Words2Addr(r10);

      // I1 <- SBR_G_filt[0]
      I1 = I1 - r10;
      // I4 <- SBR_Q_filt[0]
      I4 = I4 - r10;

      // temp_index += 1
      r6 = r6 + 1;
      // if(temp_index >= 5)
      Null = r6 - 5;
      if GE r6 = 0;

      r2 = r5 * 5(int);
      Words2Addr(r6);
      r2 = r2 + r6;
      Addr2Words(r6);

      // I3 <- SBR_G_temp[ch][temp_index][0]
      r3 = r4 + r2;
      r0 = M[r3 + $aacdec.SBR_G_TEMP_LIM_ENV_ADDR_ARRAY_FIELD];
      I3 = r0;

      // I4 <- SBR_Q_temp[ch][temp_index][0]
      r0 = M[r3 + $aacdec.SBR_Q_TEMP_LIM_ENV_ADDR_ARRAY_FIELD];
      I2 = r0;

      // if(SBR_G_filt_block_exponent < SBR_G_temp_block_exponent[ch][temp_index])
      //    SBR_G_filt_block_exponent = SBR_G_temp_block_exponent[ch][temp_index]
      r0 = M[r3 + $aacdec.SBR_G_TEMP_BLOCK_EXPONENT_ARRAY_FIELD];
      r3 = M[r4 + $aacdec.SBR_G_FILT_BLOCK_EXPONENT_FIELD];
      r4 = r0 - r3;
      if LE jump dont_update_gfilt_block_exp;
         r3 = M[r9 + $aac.mem.SBR_info_ptr];
         M[r3 + $aacdec.SBR_G_FILT_BLOCK_EXPONENT_FIELD] = r0;
      dont_update_gfilt_block_exp:

      // if(SBR_Q_filt_block_exponent < SBR_Q_temp_block_exponent[ch][temp_index])
      //    SBR_Q_filt_block_exponent = SBR_Q_temp_block_exponent[ch][temp_index]
      r7 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = r7 + r2;
      r0 = M[r0 + $aacdec.SBR_Q_TEMP_BLOCK_EXPONENT_ARRAY_FIELD];
      r3 = M[r7 + $aacdec.SBR_Q_FILT_BLOCK_EXPONENT_FIELD];
      r7 = r0 - r3;
      if LE jump dont_update_qfilt_block_exp;
         r3 = M[r9 + $aac.mem.SBR_info_ptr];
         M[r3 + $aacdec.SBR_Q_FILT_BLOCK_EXPONENT_FIELD] = r0;
      dont_update_qfilt_block_exp:

      Null = r1;
      if NZ jump not_first_smoothing_loop;
         r4 = 0;
         r7 = 0;
      not_first_smoothing_loop:

      Addr2Words(r10);

      // r2 = SBR_H_smooth[j]
      Words2Addr(r1);
      r0 = M[r9 + $aac.mem.SBR_h_smooth_ptr];
      r2 = M[r0 + r1];
      Addr2Words(r1);

      // for m=0:SBR_M-1,

      do g_and_q_filt_smoothing_inner_loop;

         // SBR_G_filt[m] += SBR_G_temp[ch][temp_index][m] * SBR_Hsmooth[j]
         r0 = M[I3, MK1];
         r0 = r0 * r2 (frac),
          rMAC = M[I1, 0];

         Null = r4 - 1;
         if NEG jump shift_g_temp_multiplication;
            r3 = -r4;
            rMAC = rMAC LSHIFT r3;
            jump update_g_filt;
         shift_g_temp_multiplication:
            r0 = r0 LSHIFT r4;
         update_g_filt:
         r0 = r0 + rMAC;
         M[I1, MK1] = r0;


         // SBR_Q_filt[m] += SBR_Q_temp[ch][temp_index][m] * SBR_Hsmooth[j]
         r0 = M[I2, MK1];
         r0 = r0 * r2 (frac),
          rMAC = M[I4, 0];

         Null = r7 - 1;
         if NEG jump shift_q_temp_multiplication;
            r3 = -r7;
            rMAC = rMAC LSHIFT r3;
            jump update_q_filt;
         shift_q_temp_multiplication:
            r0 = r0 LSHIFT r7;
         update_q_filt:
         r0 = r0 + rMAC;
         M[I4, MK1] = r0;

      g_and_q_filt_smoothing_inner_loop:

   r1 = r1 + 1;
   Null = r1 - 4;
   if LE jump g_and_q_filt_smoothing_outer_loop;

   Addr2Words(r5); // In all code above r5 was a channel address offset (revert it a back to channel number)
   rts;

.ENDMODULE;

#endif


