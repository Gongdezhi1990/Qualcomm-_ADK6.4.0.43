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
//    $aacdec.sbr_hf_assembly_initialise_signal_gain_and_component_loop
//
// DESCRIPTION:
//    - initialise the loop which applies the signal (G) and noise (Q) filters as well
//    - as inserts the sinusoidal components (S) across the high-band in the current
//    - time-sample i
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
.MODULE $M.aacdec.sbr_hf_assembly_initialise_signal_gain_and_component_loop;
   .CODESEGMENT AACDEC_SBR_HF_ASSEMBLY_INITIALISE_SIGNAL_GAIN_AND_COMPONENT_LOOP_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_hf_assembly_initialise_signal_gain_and_component_loop:

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r6 = M[r0 + $aacdec.SBR_kx];
   Words2Addr(r6);

   // I1 -> real(X_sbr[ch][SBR_kx][i+SBR_tHFAdj])
   // I4 -> imag(X_sbr[ch][SBR_kx][i+SBR_tHFAdj])

   r0 = M1 + ($aacdec.SBR_tHFAdj);
   r0 = r0 * ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD) (int);
   r0 = r0 + r6;

   r1 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
   I1 = r1 + r0;
   r1 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
   I4 = r1 + r0;

   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_calc_gain_boost_data_per_envelope];
   r0 = r0 * r8 (int);
   Words2Addr(r0);
   // I3 -> SBR_S_M_boost_mantissa[l][0]
   r7 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = M[r7 + $aacdec.SBR_S_M_boost_mantissa_ptr];
   I3 = r0 + r1;

   // I7 -> SBR_G_filt_mantissa[0]
   r0 = M[r7 + $aacdec.SBR_G_filt_ptr];
   I7 = r0;
   // I2 -> SBR_Q_filt_mantissa[0]
   r0 = M[r7 + $aacdec.SBR_Q_filt_ptr];
   I2 = r0;

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r10 = M[r0 + $aacdec.SBR_M];

   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_noise_component_flag];
   M2 = r0;

   M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_save_ch] = r5;

   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_sine];
   Words2Addr(r0);
   r7 = M[r9 + $aac.mem.SBR_phi_re_sin_ptr];
   r6 = M[r7 + r0];
   r7 = M[r9 + $aac.mem.SBR_phi_im_sin_ptr];
   r5 = M[r7 + r0];
   Addr2Words(r0);
   // SBR_f_index_sine = bitand(SBR_f_index_sine + 1, 3);
   r0 = r0 + 1;
   r0 = r0 AND 3;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_sine] = r0;

   r7 = M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_noise];
   r7 = r7 + r10;
   r7 = r7 AND 0x1FF;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_f_index_noise] = r7;

   I5 = I0 + (512*ADDR_PER_WORD);

   /* Base registers are needed as the data could be anywhere in flash.  Previous code used Luck!!) */
   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_v_noise_start];
   push r0;
   pop B0;
   r0 = r0 + (512*ADDR_PER_WORD);
   push r0;
   pop B5;

   L0 = 512*ADDR_PER_WORD;
   L5 = 512*ADDR_PER_WORD;

   // if(bitand(SBR_kx, 1) == 1)
   //    rev = 1;
   // else
   //    rev = -1;
   r1 = M[r9 + $aac.mem.SBR_info_ptr];
   r0 = M[r1 + $aacdec.SBR_kx];
   Null = r0 AND 1;
   //if NZ r5 = -r5;
   if Z r5 = -r5;

   // r7 = no. bits to shift the calculated Sinusoidal component by to make it $aacdec.SBR_ANALYSIS_SHIFT_AMOUNT - bits
   // higher than the correct scale like X_sbr is inorder to keep $aacdec.SBR_ANALYSIS_SHIFT_AMOUNT fractional bits
   Words2Addr(r8);
   r0 = r1 + r8;
   r4 = M[r0 + $aacdec.SBR_S_M_BOOST_BLOCK_EXPONENT_ARRAY_FIELD];
   r7 = r4 + ($aacdec.SBR_ANALYSIS_SHIFT_AMOUNT - 23);
   Addr2Words(r8);

   M[r9 + $aac.mem.TMP + $aacdec.SBR_hf_assembly_save_l] = r8;
   r4 = M[r1 + $aacdec.SBR_G_FILT_BLOCK_EXPONENT_FIELD];

   // r8 = no. bits to shift the calculated Noise component by to make it $aacdec.SBR_ANALYSIS_SHIFT_AMOUNT - bits
   // higher than the correct scale like X_sbr is inorder to keep $aacdec.SBR_ANALYSIS_SHIFT_AMOUNT fractional bits
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r8 = M[r0 + $aacdec.SBR_Q_FILT_BLOCK_EXPONENT_FIELD];
   r8 = r8 + ($aacdec.SBR_ANALYSIS_SHIFT_AMOUNT-23 + (DAWTH-16));
   rts;

.ENDMODULE;

#endif


