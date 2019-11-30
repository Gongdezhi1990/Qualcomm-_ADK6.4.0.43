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
//    $aacdec.sbr_save_prev_data
//
// DESCRIPTION:
//    Save data from this frame that is required for next frame
//
// INPUTS:
//    - r5 channel
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r2, r10, I1, I2
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_save_prev_data;
   .CODESEGMENT AACDEC_SBR_SAVE_PREV_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_save_prev_data:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5);
   // SBR_bs_add_harmonic_flag_prev[ch] = SBR_bs_add_harmonic_flag[ch]
   r0 = r5 + r9;
   r0 = M[$aac.mem.SBR_bs_add_harmonic_flag + r0];
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r2 = r1 + r5;
   M[r2 + $aacdec.SBR_bs_add_harmonic_flag_prev] = r0;

   // SBR_bs_add_harmonic_prev[ch][0:48] = SBR_bs_add_harmonic[ch][0:48]
   r0 = r5 * 49 (int);

   r2 = r1 + r0;
   I1 = r2 + $aacdec.SBR_bs_add_harmonic_prev;
   r0 = r5 * 64 (int);
   r2 = r1 + r0;
   I2 = r2 + $aacdec.SBR_bs_add_harmonic;

   r10 = 48;
   do bs_add_harmonic_prev_loop;
      r0 = M[I2, MK1];
      M[I1, MK1] = r0;
   bs_add_harmonic_prev_loop:


   // SBR_freq_res_prev[ch] = SBR_bs_freq_res[ch][SBR_bs_num_env[ch]-1]
   r0 = r5 + r9;
   r0 = M[$aac.mem.SBR_bs_num_env + r0];
   r1 = r5 * 6 (int);
   Words2Addr(r0);
   r1 = r1 + r0;
   Addr2Words(r0);
   r2 = M[r9 + $aac.mem.SBR_info_ptr];
   r2 = r2 + r1;
   r1 = M[r2 + $aacdec.SBR_bs_freq_res - (1*ADDR_PER_WORD)];
   r2 = M[r9 + $aac.mem.SBR_info_ptr];
   r2 = r2 + r5;
   M[r2 + $aacdec.SBR_freq_res_prev] = r1;


   // if(SBR_l_A[ch] == SBR_bs_num_env[ch])
   r2 = 1;
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = r1 + r5;
   r1 = M[r1 + $aacdec.SBR_l_A];
   Null = r1 - r0;
   if NZ r2 = 0;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = r0 + r5;
   M[r0 + $aacdec.SBR_prevEnvIsShort] = r2;

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
