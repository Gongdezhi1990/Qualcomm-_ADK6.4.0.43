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
//    $aacdec.sbr_hf_assembly_save_persistent_gain_signal_envelopes
//
// DESCRIPTION:
//    saves each envelope of the Gain arrays (signal (G) and noise (Q))
//    which may be required in the subsequent frame if smoothing mode is used
//    to apply the gains
//
// INPUTS:
//    -
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    -
//    -
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_hf_assembly_save_persistent_gain_signal_envelopes;
   .CODESEGMENT AACDEC_SBR_HF_ASSEMBLY_SAVE_PERSISTENT_GAIN_SIGNAL_ENVELOPES_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_hf_assembly_save_persistent_gain_signal_envelopes:

   Words2Addr(r5);
   // SBR_index_noise_prev[ch] = SBR_f_index_noise
   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_noise];
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r1 = r1 + r5;
   M[r1 + $aacdec.SBR_index_noise_prev] = r0;

   // SBR_psi_is_prev[ch] = SBR_f_index_sine
   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_sine];
   M[r1 + $aacdec.SBR_psi_is_prev] = r0;

   // write persistent envelopes into SBR_G_temp and SBR_Q_temp arrays
   r6 = M[r1 + $aacdec.SBR_GQ_index];
   Words2Addr(r6);
   r7 = r5 * 5 (int);

   r8 = M[r9 + $aac.mem.SBR_info_ptr];

   M0 = 4;
   r0 = r5 * ($aacdec.SBR_M_MAX * 2) (int);
   Addr2Words(r5);
   M1 = r0 - $aacdec.SBR_M_MAX*ADDR_PER_WORD;

   save_persistent_g_and_temp_envelopes_loop:
      r6 = r6 + (1*ADDR_PER_WORD);
      Null = r6 - (5*ADDR_PER_WORD);
      if GE r6 = 0;

      r0 = r7 + r6;

      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r0;
      r2 = M[r2 + $aacdec.SBR_G_TEMP_LIM_ENV_ADDR_ARRAY_FIELD];
      Null = r8 - r2;
      if Z jump g_and_q_temp_envelope_saved;
         r8 = r2;

         r10 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope];

         M1 = M1 + ($aacdec.SBR_M_MAX*ADDR_PER_WORD);

         I0 = r2;
         r1 = M[r9 + $aac.mem.SBR_info_ptr];
         r2 = r1 + r0;
         r2 = M[r2 + $aacdec.SBR_Q_TEMP_LIM_ENV_ADDR_ARRAY_FIELD];
         I4 = r2;
         r1 = r1 + M1;
         I1 = r1 + $aacdec.SBR_G_TEMP_PERSISTENT_ENVELOPES_ARRAY_FIELD;
         I5 = r1 + $aacdec.SBR_Q_TEMP_PERSISTENT_ENVELOPES_ARRAY_FIELD;

         do save_persistent_envelopes_inner_loop;
            r2 = M[I0, MK1],
             r1 = M[I4, MK1];
            M[I1, MK1] = r2,
             M[I5, MK1] = r1;
         save_persistent_envelopes_inner_loop:

      g_and_q_temp_envelope_saved:

      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = r2 + r0;
      r2 = r2 + M1;
      r1 = r2 + $aacdec.SBR_G_TEMP_PERSISTENT_ENVELOPES_ARRAY_FIELD;
      M[r0 + $aacdec.SBR_G_TEMP_LIM_ENV_ADDR_ARRAY_FIELD] = r1;

      r1 = r2 + $aacdec.SBR_Q_TEMP_PERSISTENT_ENVELOPES_ARRAY_FIELD;
      M[r0 + $aacdec.SBR_Q_TEMP_LIM_ENV_ADDR_ARRAY_FIELD] = r1;

   M0 = M0 - 1;
   if NZ jump save_persistent_g_and_temp_envelopes_loop;

   rts;

.ENDMODULE;

#endif




