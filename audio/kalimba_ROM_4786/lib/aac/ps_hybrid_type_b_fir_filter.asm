// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"
#include "fft.h"

#ifdef AACDEC_PS_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.ps_hybrid_type_b_fir_filter
//
// DESCRIPTION:
//    - g[n]*cos(2pi/8*q*(n-6))
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - toupdate
//
// *****************************************************************************
.MODULE $M.aacdec.ps_hybrid_type_b_fir_filter;
   .CODESEGMENT AACDEC_PS_HYBRID_TYPE_B_FIR_FILTER_PM;
   .DATASEGMENT DM;

   $aacdec.ps_hybrid_type_b_fir_filter:

   // Initialisation for filtering of the real input samples

   r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET];
   r0 = r0 * ($aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD) (int);
   // I0 -> real(ps_X_hybrid[ch=0][k=ps_hybrid_sub_subband_index_offset][n=0])
   r1 = M[r9 + $aac.mem.SBR_synth_temp_ptr];
   I0 = r1 + r0;
   // I1 -> real(ps_X_hybrid[ch=0][k=ps_hybrid_sub_subband_index_offset+1][n=0])
   I1 = I0 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD);

   r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
   Words2Addr(r8);
   // I2 -> real(X_SBR[ch=0][k=p][l=SBR_tHFAdj+6])
   r1 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   r1 = r1 + r8;
   I2 = r1 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);

   M0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;

   // r10 -> real(fir_input[0-(PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH-1)])
   r10 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
   r10 = r10 * ($aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1) (int);
   Words2Addr(r10);

   r10 = r10 + $aac.mem.PS_time_history_real;
   r10 = r10 + r9;

   M2 = 1;

   // first outer-loop iteration processed real(fir_input[]) and second iteration processes imag(fir_input[])

   ps_hybrid_type_b_fir_filter_outer_loop:

      r0 = M[r9 + $aac.mem.PS_hybrid_type_b_fir_filter_coefficients_ptr];
      I4 = r0;

      // calculate {real | imaginary} filter output for n=0

      r7 = M[r10 + (1*ADDR_PER_WORD)];   // r7 = fir_input[n-11]
      r6 = M[r10 + (3*ADDR_PER_WORD)];   // r6 = fir_input[n-9]
      r4 = M[r10 + (5*ADDR_PER_WORD)];   // r4 = fir_input[n-7]

      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_FOUR] = r6;
      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_THREE] = r4;

      r0 = M[r10 + (11*ADDR_PER_WORD)];  // r0 = fir_input[n-1]
      r1 = M[r10 + (9*ADDR_PER_WORD)];   // r1 = fir_input[n-3]
      r3 = M[r10 + (6*ADDR_PER_WORD)];   // r3 = fir_input[n-6]
      r2 = M[r10 + (7*ADDR_PER_WORD)];   // r2 = fir_input[n-5]
      r5 = M[I4, MK1];

      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_TWO] = r1;
      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_ONE] = r0;

      rMAC = r7 * r5,
       r5 = M[I4, MK1];
      rMAC = rMAC + r6 * r5,
       r5 = M[I4,0];
      rMAC = rMAC + r4 * r5,
       r5 = M[I4, -MK1];
      rMAC = rMAC + r2 * r5,
       r5 = M[I4, -MK1];
      rMAC = rMAC + r1 * r5,
       r5 = M[I4,0];
      rMAC = rMAC + r0 * r5;
      r8 = r3 * 0.5 (frac);
      r5 = r8 + rMAC;
      rMAC = r8 - rMAC;
      M[I0, MK1] = r5;
      M[I1, MK1] = rMAC,
       r5 = M[I4, MK1];

      // calculate {real | imaginary} filter output for n=1

      r7 = M[r10 + (1*ADDR_PER_WORD + ADDR_PER_WORD)];   // r7 = fir_input[n-11]
      rMAC = r7 * r5,
       r5 = M[I4, MK1];
      r7 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_FOUR];  // r7 = fir_input[n+1-11]

      r6 = M[r10 + (3*ADDR_PER_WORD + ADDR_PER_WORD)];   // r6 = fir_input[n-9]
      rMAC = rMAC + r6 * r5,
       r5 = M[I4,0];
      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_FOUR] = r6;
      r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_THREE];   // r6 = fir_input[n+1-9]

      r4 = r3;    // r4 = fir_input[n-7]
      rMAC = rMAC + r4 * r5,
       r5 = M[I4, -MK1];
      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_THREE] = r4;

      r3 = r2; // r3 = fir_input[n-6]
      r8 = r3 * 0.5 (frac);
      r4 = r3;  // r4 = fir_input[n+1-7]

      r2 = M[r10 + (7*ADDR_PER_WORD + ADDR_PER_WORD)];   // r2 = fir_input[n-5]
      rMAC = rMAC + r2 * r5,
       r5 = M[I4, -MK1];
      r3 = r2; // r3 = fir_input[n+1-6]
      r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_TWO];    // r2 = fir_input[n+1-5]

      r1 = M[r10 + (9*ADDR_PER_WORD + ADDR_PER_WORD)];   // r1 = fir_input[n-3]
      rMAC = rMAC + r1 * r5,
       r5 = M[I4,0];
      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_TWO] = r1;
      r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_ONE];    // r1 = fir_input[n+1-3]

      r0 = M[I2,M0];    // r0 = fir_input[n-1]
      rMAC = rMAC + r0 * r5;
      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_ONE] = r0;

      r5 = r8 + rMAC;
      rMAC = r8 - rMAC;
      M[I0, MK1] = r5;
      M[I1, MK1] = rMAC,
       r5 = M[I4, MK1];

      // for n=2:PS_NUM_SAMPLES_PER_FRAME-1

      r10 = $aacdec.PS_NUM_SAMPLES_PER_FRAME - 2;

      do ps_hybrid_type_b_fir_time_samples_loop;

         rMAC = r7 * r5,
          r5 = M[I4, MK1];
         r7 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_FOUR];    // r7 = fir_input[n+1-11]

         rMAC = rMAC + r6 * r5,
          r5 = M[I4,0];
         M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_FOUR] = r6;
         r6 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_THREE];   // r6 = fir_input[n+1-9]

         rMAC = rMAC + r4 * r5,
          r5 = M[I4, -MK1];
         M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_THREE] = r4;
         r4 = r3;    // r4 = fir_input[n+1-7]

         r8 = r3 * 0.5 (frac);
         r3 = r2;    // r3 = fir_input[n+1-6]

         rMAC = rMAC + r2 * r5,
          r5 = M[I4, -MK1];
         r2 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_TWO];    // r2 = fir_input[n+1-5]

         rMAC = rMAC + r1 * r5,
          r5 = M[I4,0];

         r0 = M[I2,M0];    // r0 = fir_input[n-1]
         M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_TWO] = r1;
         r1 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_ONE];    // r1 = fir_input[n+1-3]

         rMAC = rMAC + r0 * r5;
         M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REGISTER_ONE] = r0;

         r5 = r8 + rMAC;
         rMAC = r8 - rMAC;
         M[I0, MK1] = r5;
         M[I1, MK1] = rMAC,
          r5 = M[I4, MK1];

      ps_hybrid_type_b_fir_time_samples_loop:


      Null = M2;
      if Z jump exit_ps_hybrid_type_b_fir_filter;

         // Initialisation for filtering the imaginary input samples

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_SUB_SUBBAND_INDEX_OFFSET];
         r0 = r0 * ($aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD) (int);
         // I0 -> imag(ps_X_hybrid[ch=0][k=ps_hybrid_sub_subband_index_offset][n=0])

         r1 = M[r9 + $aac.mem.SBR_synth_temp_ptr];
         r1 = r1 + r0;
         I0 = (($aacdec.PS_NUM_HYBRID_SUB_SUBBANDS - 2) * $aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD) + r1;
         // I1 -> imag(ps_X_hybrid[ch=0][k=ps_hybrid_sub_subband_index_offset+1][n=0])
         I1 = I0 + ($aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD);

         r8 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
         Words2Addr(r8);
         // I2 -> imag(X_SBR[ch=0][k=p][l=SBR_tHFAdj+0])
         r1 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
         r1 = r1 + r8;
         I2 = r1 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
         M0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;

         r10 = r8 * ($aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1) (int);

         r10 = r10 + &$aac.mem.PS_time_history_imag;
         r10 = r10 + r9;

         M2 = M2 - 1;
   jump ps_hybrid_type_b_fir_filter_outer_loop;

   exit_ps_hybrid_type_b_fir_filter:
   rts;

.ENDMODULE;

#endif
