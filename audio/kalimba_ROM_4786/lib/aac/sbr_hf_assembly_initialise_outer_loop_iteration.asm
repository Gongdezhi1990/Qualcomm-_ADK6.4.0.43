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
//    $aacdec.sbr_hf_assembly_initialise_outer_loop_iteration
//
// DESCRIPTION:
//    initialse outer-loop iteration of hf_assembly for current envelope
//
// INPUTS:
//    - r5 current channel (0/1)
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -
//    -
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_hf_assembly_initialise_outer_loop_iteration;
   .CODESEGMENT AACDEC_SBR_HF_ASSEMBLY_INITIALISE_OUTER_LOOP_ITERATION_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_hf_assembly_initialise_outer_loop_iteration:

   Words2Addr(r5); // In all code below r5 is a channel address offset

   // if(SBR_bs_smoothing_mode == 1)
   //    SBR_H_SL = 0
   // else
   //    SBR_H_SL = 4
   // end
   r0 = 4;
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = M[r1 + $aacdec.SBR_bs_smoothing_mode];
   if NZ r0 = 0;

   r3 = 1;
   r2 = 0;
   // if((l == SBR_l_A[ch]) || ((l == 0) && SBR_prevEnvIsShort[ch]))
   //    SBR_H_SL = 0;
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = r1 + r5;
   r1 = M[r1 + $aacdec.SBR_l_A];
   Null = r8 - r1;
   if Z r0 = 0;
   if Z r2 = r3;
   Null = r8;
   if NZ jump save_sbr_h_sl;
      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r1 = r1 + r5;
      Null = M[r1 + $aacdec.SBR_prevEnvIsShort];
      if NZ r2 = r3;
      if NZ r0 = 0;
   save_sbr_h_sl:

   M[r9 + $aac.mem.TMP + $aacdec.SBR_H_SL] = r0;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_noise_component_flag] = r2;


   // if((l == 0) && (SBR_reset == 1))
   Null = r8;
   if NZ jump end_if_first_envelope_and_reset;
   r4 = M[r9 + $aac.mem.SBR_info_ptr];
   Null = M[r4 + $aacdec.SBR_reset];
   if Z jump end_if_first_envelope_and_reset;
      // SBR_G_temp[ch][0:3][0:SBR_M-1] = SBR_G_lim_boost[0][0:SBR_M-1]
      // SBR_Q_temp[ch][0:3][0:SBR_M-1] = SBR_Q_M_lim_boost[0][0:SBR_M-1]
      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = M[r1 + $aacdec.SBR_G_lim_boost_mantissa_ptr];
      r1 = M[r1 + $aacdec.SBR_Q_M_lim_boost_mantissa_ptr];
      r2 = M[r4 + $aacdec.SBR_G_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      r3 = M[r4 + $aacdec.SBR_Q_M_LIM_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
      r6 = r5 * 5 (int);
      r10 = 4;
      r6 = r6 + r4;

      do g_and_q_temp_reset_loop;
         M[$aacdec.SBR_G_TEMP_LIM_ENV_ADDR_ARRAY_FIELD + r6] = r0;
         M[$aacdec.SBR_Q_TEMP_LIM_ENV_ADDR_ARRAY_FIELD + r6] = r1;
         M[$aacdec.SBR_G_TEMP_BLOCK_EXPONENT_ARRAY_FIELD + r6] = r2;
         M[$aacdec.SBR_Q_TEMP_BLOCK_EXPONENT_ARRAY_FIELD + r6] = r3;
         r6 = r6 + (1*ADDR_PER_WORD);
      g_and_q_temp_reset_loop:

      // SBR_GQ_index[ch] = 4
      r0 = 4;
      r1 = r4 + r5;
      M[r1 + $aacdec.SBR_GQ_index] = r0;

   end_if_first_envelope_and_reset:


   r0 = r5 * 6 (int);
   Words2Addr(r8);
   r0 = r0 + r8;
   Addr2Words(r8);
   r1 = r0 + r9;
   r1 = M[$aac.mem.SBR_t_E + r1];
   M1 = r1;
   r1 = r0 + r9;
   r1 = M[$aac.mem.SBR_t_E + ADDR_PER_WORD + r1];

   M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_time_envelope_border] = r1;

   Addr2Words(r5); // In all code above r5 was a channel address offset (revert it back)

   rts;

.ENDMODULE;

#endif


