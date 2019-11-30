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
//    $aacdec.ps_hybrid_type_a_fir_filter
//
// DESCRIPTION:
//    - g[n]*exp(j*2pi/8*(q+0.5)*(n-6))
//
// INPUTS:
//    - r8  qmf subband number 'p'
//    - r7  ps_hybrid_sub_subband_index_offset
//
// OUTPUTS:
//    - r8 unchanged
//
// TRASHED REGISTERS:
//    - toupdate
//
// *****************************************************************************
.MODULE $M.aacdec.ps_hybrid_type_a_fir_filter;
   .CODESEGMENT AACDEC_PS_HYBRID_TYPE_A_FIR_FILTER_PM;
   .DATASEGMENT DM;

   $aacdec.ps_hybrid_type_a_fir_filter:

   // push rLink onto stack
   push rLink;

   // I7 -> temp_real[n=0][0]
   r0 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
   I7 = r0;

   r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
   Words2Addr(r0);
   // I2 -> real(X_SBR[ch=0][k=p][l=SBR_tHFAdj+6]) = real(fir_input[n=0])
   r1 = M[r9 + $aac.mem.SBR_X_curr_real_ptr];
   r1 = r1 + r0;
   I2 = r1 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);

   M0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;
   M2 = 5*ADDR_PER_WORD;
   M3 = 8*ADDR_PER_WORD;

   // I1 -> real(fir_input[0-(PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH-1)])
   r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
   r0 = r0 * (($aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1)*ADDR_PER_WORD) (int);
   I1 = r9 + $aac.mem.PS_time_history_real;
   I1 = I1 + r0;

   // I4 points to the circular buffer which maintains the delay-line (real component of the input time-samples)
   // of the FIR filter for each successive value of n the oldest time-sample in the buffer is replaced with
   // the new input time-sample entering the delay-line
   r2 = M[r9 + $aac.mem.PS_hybrid_type_a_fir_filter_input_buffer_ptr];
   I4 = r2;
#ifdef BASE_REGISTER_MODE
   push r2;
   pop B4;
#endif

   // initialise the real input circular-buffer
   r0 = M[I2,M0]; // r0 = real(fir_input[n=0])
   M[r2 + (12*ADDR_PER_WORD)] = r0;

   r10 = $aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1;
   L4 = $aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH*ADDR_PER_WORD;

   do ps_hybrid_type_a_fir_initialise_real_input_buffer_loop;
      r0 = M[I1, MK1];
      M[I4, MK1] = r0;
   ps_hybrid_type_a_fir_initialise_real_input_buffer_loop:

   r8 = 1;

   M1 = ADDR_PER_WORD;
   r0 = 1;

   // filter coefficients
   r2 = 0.00746082949812;/* g[0,12] */  r3 = 0.02270420949825; /* g[1,11] */ r4 = 0.04546865930473; /* g[2,10] */  r5 = 0.07266113929591; /* g[3,9] */
   r6 = 0.09885108575264; /* g[4,8] */  r7 = 0.11793710567217; /* g[5,7] */

   // first outer-loop iteration processed real(fir_input[]) and second iteration processes imag(fir_input[])

   ps_hybrid_type_a_fir_temp_outer_loop:


      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REAL_IMAG_FLAG] = r0;

      r10 = 0.125; /* g[6] */

      L0 = $aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH*ADDR_PER_WORD;

      r0 = M[r9 + $aac.mem.PS_hybrid_type_a_fir_filter_input_buffer_ptr];
      I4 = r0 + (4*ADDR_PER_WORD);
      I0 = r0 + (12*ADDR_PER_WORD);
#ifdef BASE_REGISTER_MODE
      push r0;
      pop B0;
#endif

      // for n=0:PS_NUM_SAMPLES_PER_FRAME-1,
      L1 = $aacdec.PS_NUM_SAMPLES_PER_FRAME;

      ps_hybrid_type_a_fir_temp_inner_loop:

         r1 = M[I4, -MK1],    // r1 = fir_input[n-8]
          r0 = M[I0, -MK1];   // r0 = fir_input[n-0]

         // rMAC = fir_input[n-8]*c[4] + fir_input[n-0]*c[12]
         rMAC = r0 * r2,
          r0 = M[I0, -MK1];   // r0 = fir_input[n-1]
         rMAC = rMAC - r1 * r6,
          r1 = M[I4, -MK1];   // r1 = fir_input[n-9]

         // rMAC = fir_input[n-9]*c[3] + fir_input[n-1]*c[11]
         rMAC = r0 * r3,
          M[I7, MK1] = rMAC,
          r0 = M[I0, -MK1];   // r0 = fir_input[n-2]
         rMAC = rMAC - r1 * r5,
          r1 = M[I4, -MK1];   // r1 = fir_input[n-10]

         // rMAC = (fir_input[n-10] - fir_input[n-2])*c[10]
         rMAC = r1 * r4,
          M[I7, MK1] = rMAC,
          r1 = M[I0, -MK1];   // r1 = fir_input[n-3]
         rMAC = rMAC - r0 * r4,
          r0 = M[I4, -MK1];   // r0 = fir_input[n-11]
         rMAC = rMAC * r8 (int);

         // rMAC = fir_input[n-11]*c[1] + fir_input[n-3]*c[9]
         rMAC = r1 * r5,
          M[I7, MK1] = rMAC,
          r1 = M[I0, -MK1];   // r1 = fir_input[n-4]
         rMAC = rMAC - r0 * r3,
          r0 = M[I4,0];   // r0 = fir_input[n-12]

         // rMAC = fir_input[n-12]*c[0] + fir_input[n-4]*c[0]
         rMAC = r1 * r6,
          M[I7, MK1] = rMAC,
          r1 = M[I0, -MK1];   // r1 = fir_input[n-5]
         rMAC = rMAC - r0 * r2;

         // rMAC = fir_input[n-5] * c[7]
         rMAC = r1 * r7,
          M[I7, MK1] = rMAC,
          r1 = M[I0, -MK1];   // r1 = fir_input[n-6]

         // rMAC = fir_input[n-6] * c[6]
         rMAC = r1 * r10,
          M[I7, MK1] = rMAC,
          r1 = M[I0,0];   // r1 = fir_input[n-7]

         // rMAC = fir_input[n-7] * c[5]
         rMAC = r1 * r7,
          M[I7,M1] = rMAC,
          r0 = M[I2,M0];   // r0 = fir_input[n+1]

         M[I7, MK1] = rMAC;

         M[I4,M2] = r0, // replace oldest input sample in circular-buffer with fir_input[n+1]
          r0 = M[I0,M3];

         L1 = L1 - 1;
      if NZ jump ps_hybrid_type_a_fir_temp_inner_loop;


      Null = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REAL_IMAG_FLAG];
      if Z jump end_ps_hybrid_type_a_fir_temp_loop;

         // I7 -> temp_imag[n=0][0]
         r0 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
         I7 = r0 + (8 * 32 * ADDR_PER_WORD);

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
         Words2Addr(r0);
         // I2 -> imag(X_SBR[ch=0][k=p][l=SBR_tHFAdj+6]) = imag(fir_input[n=0])
         r1 = M[r9 + $aac.mem.SBR_X_curr_imag_ptr];
         r1 = r1 + r0;
         I2 = r1 + (($aacdec.SBR_tHFGen-$aacdec.SBR_tHFAdj)*$aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
         M0 = $aacdec.X_SBR_WIDTH*ADDR_PER_WORD;

         // I1 -> imag(fir_input[0-(PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH-1)])
         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_QMF_SUBBAND];
         r0 = r0 * (($aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1)*ADDR_PER_WORD) (int);
         I1 = r9 + &$aac.mem.PS_time_history_imag;
         I1 = I1 + r0;

         // I4 points to the circular buffer which maintains the delay-line (imag component of the input time-samples)
         // of the FIR filter for each successive value of n the oldest time-sample in the buffer is replaced with
         // the new input time-sample entering the delay-line
         r1 = M[r9 + $aac.mem.PS_hybrid_type_a_fir_filter_input_buffer_ptr];
         I4 = r1;

         // initialise the imaginary input circular-buffer
         r0 = M[I2,M0]; // r0 = imag(fir_input[n=0])
         M[r1 + (12*ADDR_PER_WORD)] = r0;

         r10 = $aacdec.PS_HYBRID_ANALYSIS_FIR_FILTER_LENGTH - 1;

         do ps_hybrid_type_a_fir_initialise_imag_input_buffer_loop;
            r0 = M[I1, MK1];
            M[I4, MK1] = r0;
         ps_hybrid_type_a_fir_initialise_imag_input_buffer_loop:

         r8 = -1;

         r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_REAL_IMAG_FLAG];
         r0 = r0 - 1;
      jump ps_hybrid_type_a_fir_temp_outer_loop;
   end_ps_hybrid_type_a_fir_temp_loop:


   L0 = 0;
   L4 = 0;
#ifdef BASE_REGISTER_MODE
   push Null;
   pop B4;
   push Null;
   pop B0;
#endif


   // assemble inv_fft_input

   // I0 -> real(ifft_input[n=0][0])
   r0 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
   I0 = r0 + ($aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS * $aacdec.PS_NUM_SAMPLES_PER_FRAME * 2 * ADDR_PER_WORD);
   // I2 -> real(ifft_input[n=0][2])
   I2 = I0 + (2*ADDR_PER_WORD);
   // I4 -> imag(ifft_input[n=0][0])
   I4 = I0 + $aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS*ADDR_PER_WORD;
   // I6 -> imag(ifft_input[n=0][2])
   I6 = I4 + (2*ADDR_PER_WORD);


   // I1 -> temp_real[n=0][0]
   I1 = r0;
   // I5 -> temp_imag[n=0][0]
   I5 = r0 + ($aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS * $aacdec.PS_NUM_SAMPLES_PER_FRAME * ADDR_PER_WORD);

   r10 = $aacdec.PS_NUM_SAMPLES_PER_FRAME;

   r4 = $aacdec.PS_COS_PI_OVER_FOUR;
   r5 = $aacdec.PS_COS_PI_OVER_EIGHT;
   r6 = $aacdec.PS_SIN_PI_OVER_EIGHT;
   r7 = -$aacdec.PS_COS_PI_OVER_FOUR;
   r8 = -$aacdec.PS_SIN_PI_OVER_EIGHT;

   // for n=0:PS_NUM_SAMPLES_PER_FRAME-1,

   do ps_hybrid_type_a_fir_assemble_inv_fft_input_loop;

      r0 = M[I1, MK1],  // r0 = temp_real[n][0]
       r1 = M[I5, MK1];  // r1 = temp_imag[n][0]

      rMAC = r1 * r4;
      rMAC = rMAC - r0 * r4;

      rMAC = r1 * r7,
       M[I2, MK1] = rMAC;  // real(ifft_input[n][2]) = (temp_imag[n][0] - temp_real[n][0]) * PS_COS_PI_OVER_FOUR
      rMAC = rMAC + r0 * r7;

      r0 = M[I1, MK1],  // r0 = temp_real[n][1]
       r1 = M[I5, MK1];  // r1 = temp_imag[n][1]

      rMAC = r1 * r5,
       M[I6, MK1] = rMAC;  // imag(ifft_input[n][2]) = (temp_imag[n][0] + temp_real[n][0]) * -PS_COS_PI_OVER_FOUR
      rMAC = rMAC - r0 * r6;

      rMAC = r1 * r8,
       M[I2, MK1] = rMAC;  // real(ifft_input[n][3]) = (temp_imag[n][1] * PS_COS_PI_OVER_EIGHT) - (temp_real[n][1] * PS_SIN_PI_OVER_EIGHT)
      rMAC = rMAC - r0 * r5;
      M[I6, MK1] = rMAC;   // imag(ifft_input[n][3]) = (temp_imag[n][1] * -PS_SIN_PI_OVER_EIGHT) + (temp_real[n][1] * -PS_COS_PI_OVER_EIGHT)

      r0 = M[I1, MK1],  // r0 = temp_real[n][2]
       r1 = M[I5, MK1];  // r1 = temp_imag[n][2]

      M[I2, MK1] = r1,  // real(ifft_input[n][4])
       M[I6, MK1] = r0; // imag(ifft_input[n][4])

      r0 = M[I1, MK1],  // r0 = temp_real[n][3]
       r1 = M[I5, MK1];  // r1 = temp_real[n][3]

      rMAC = r1 * r5;
      rMAC = rMAC + r0 * r6;

      rMAC = r1 * r6,
       M[I2, MK1] = rMAC;  // real(ifft_input[n][5]) = (temp_imag[n][3] * PS_COS_PI_OVER_EIGHT) + (temp_real[n][3] * PS_SIN_PI_OVER_EIGHT)
      rMAC = rMAC - r0 * r5;

      r0 = M[I1, MK1],  // r0 = temp_real[n][4]
       r1 = M[I5, MK1];  // r1 = temp_imag[n][4]

      rMAC = r1 * r4,
       M[I6, MK1] = rMAC;  // imag(ifft_input[n][5)) = (temp_imag[n][3] * PS_SIN_PI_OVER_EIGHT) - (temp_real[n][3] * PS_COS_PI_OVER_EIGHT)
      rMAC = rMAC + r0 * r4;

      rMAC = r1 * r4,
       M[I2, MK1] = rMAC;  // real(ifft_input[n][6]) = (temp_imag[n][4] + temp_real[n][4]) * PS_COS_PI_OVER_FOUR
      rMAC = rMAC - r0 * r4;

      r0 = M[I1, MK1],  // r0 = temp_real[n][5]
       r1 = M[I5, MK1];  // r1 = temp_imag[n][5]

      rMAC = r1 * r6,
       M[I6, MK1] = rMAC;  // imag(ifft_input[n][6]) = (temp_imag[n][4] - temp_real[n][4]) * PS_COS_PI_OVER_FOUR
      rMAC = rMAC + r0 * r5;

      rMAC = r1 * r5,
       M[I2, MK1] = rMAC;  // real(ifft_input[n][7]) = (temp_imag[n][5] * PS_SIN_PI_OVER_EIGHT) + (temp_real[n][5] * PS_COS_PI_OVER_EIGHT)
      rMAC = rMAC - r0 * r6;

      r0 = M[I1, MK1],  // r0 = temp_real[n][6]
       r1 = M[I5, MK1];  // r1 = temp_imag[n][6]

      M[I0, MK1] = r0,  // real(ifft_input[n][0]) = temp_real[n][6]
       M[I4, MK1] = r1; // imag(ifft_input[n][0]) = temp_imag[n][6]

      r0 = M[I1, MK1],
       r1 = M[I5, MK1];

      rMAC = r0 * r5,
       M[I6, MK1] = rMAC;  // imag(ifft_input[n][7]) = (temp_imag[n][5] * PS_COS_PI_OVER_EIGHT) - (temp_real[n][5] * PS_SIN_PI_OVER_EIGHT)
      rMAC = rMAC - r1 * r6;

      rMAC = r0 * r6,
       M[I0, MK1] = rMAC;  // imag(ifft_input[n][1]) = (temp_real[n][7] * PS_COS_PI_OVER_EIGHT) - (temp_imag[n][7] * PS_SIN_PI_OVER_EIGHT)

      rMAC = rMAC + r1 * r5;
       M[I4, MK1] = rMAC;  // imag(ifft_input[n][1]) = (temp_real[n][7] * PS_SIN_PI_OVER_EIGHT) + (temp_imag[n][7] * PS_COS_PI_OVER_EIGHT)

      I0 = I0 + (14*ADDR_PER_WORD);
      I4 = I4 + (14*ADDR_PER_WORD);

      I2 = I2 + (10*ADDR_PER_WORD);
      I6 = I6 + (10*ADDR_PER_WORD);

   ps_hybrid_type_a_fir_assemble_inv_fft_input_loop:



   I7 = r9 + $aac.mem.PS_hybrid_type_a_ifft_struc;
   r0 = $aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS;
   M[r9 + $aac.mem.PS_hybrid_type_a_ifft_struc + $fft.NUM_POINTS_FIELD] = r0;


   // for n=0:PS_NUM_SAMPLES_PER_FRAME-1,
   r0 = 0;


   ps_hybrid_type_a_fir_inverse_fft_loop:


      M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_IFFT_LOOP_INDICE] = r0;

      r0 = r0 * (2 * $aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS*ADDR_PER_WORD) (int);
      r1 = M[r9 + $aac.mem.FRAME_MEM_POOL_PTR];
      r1 = r1 + r0;
      r0 = r1 + ($aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS * $aacdec.PS_NUM_SAMPLES_PER_FRAME * 2 * ADDR_PER_WORD);
      r1 = r0 + ($aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS*ADDR_PER_WORD);

      M[r9 + $aac.mem.PS_hybrid_type_a_ifft_struc + $fft.REAL_ADDR_FIELD] = r0;
      M[r9 + $aac.mem.PS_hybrid_type_a_ifft_struc + $fft.IMAG_ADDR_FIELD] = r1;

      push r9;
      call $math.ifft;
      pop r9;

      M1 = 1*ADDR_PER_WORD;
      M2 = $aacdec.PS_NUM_SAMPLES_PER_FRAME*ADDR_PER_WORD;

      r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_IFFT_LOOP_INDICE];
      Words2Addr(r0);
      // I2 -> real(ps_X_hybrid[ch=0][k=0][n])
      r1 = M[r9 + $aac.mem.SBR_synth_temp_ptr];
      I2 = r1 + r0;
      // I5 -> imag(ps_X_hybrid[ch=0][k=0][n])

      r1 = r1 + r0;
      I5 = r1 + (320*ADDR_PER_WORD);

      r2 = $aacdec.PS_HYBRID_TYPE_A_NUM_SUB_SUBBANDS;

      r0 = M[r9 + $aac.mem.PS_hybrid_type_a_ifft_struc + $fft.REAL_ADDR_FIELD];
      r1 = M[r9 + $aac.mem.PS_hybrid_type_a_ifft_struc + $fft.IMAG_ADDR_FIELD];
      I1 = r0;
      I4 = r1;

      r0 = M[I1,M1],  // r0 = real(ps_X_hybrid[ch=0][k=0][n]) / 8
       r1 = M[I4,M1]; // r1 = imag(ps_X_hybrid[ch=0][k=0][n]) / 8

      r0 = r0 * r2 (int),
       r3 = M[I1,M1]; // r3 = real(ps_X_hybrid[ch=0][k=1][n]) / 8

      r1 = r1 * r2 (int),
       r4 = M[I4,M1]; // r4 = imag(ps_X_hybrid[ch=0][k=1][n]) / 8

      r3 = r3 * r2 (int),
       M[I2,M2] = r0,   // store real(ps_X_hybrid[ch=0][k=0][n])
       M[I5,M2] = r1;   // store imag(ps_X_hybrid[ch=0][k=0][n])

      r4 = r4 * r2 (int),
       M[I2,M2] = r3, // store real(ps_X_hybrid[ch=0][k=1][n])
       r1 = M[I4,M1];   // r1 = imag(ps_X_hybrid[ch=0][k=2][n]) / 8

      r1 = r1 * r2 (int),
       r0 = M[I1,M1],   // r0 = real(ps_X_hybrid[ch=0][k=2][n]) / 8
       M[I5,M2] = r4; // store imag(ps_X_hybrid[ch=0][k=1][n])

      r0 = r0 * r2 (int),
       r3 = M[I1,M1], // r3 = real(ps_X_hybrid[ch=0][k=3][n]) / 8
       r4 = M[I4,M1]; // r4 = imag(ps_X_hybrid[ch=0][k=3][n]) / 8

      r3 = r3 * r2 (int),
       r5 = M[I1,M1],   // r5 = real(ps_X_hybrid[ch=0][k=4][n]) / 8
       rMAC = M[I4,M1]; // rMAC = imag(ps_X_hybrid[ch=0][k=4][n]) / 8

      r5 = r5 * r2 (int);

      r3 = r3 + r5,  // r3 = real(ps_X_hybrid[ch=0][k=3][n]) +  real(ps_X_hybrid[ch=0][k=4][n])
       r5 = M[I1,M1];   // r5 = real(ps_X_hybrid[ch=0][k=5][n]) / 8

      r4 = r4 * r2 (int);
      rMAC = rMAC * r2 (int);

      r4 = r4 + rMAC,   // r4 = imag(ps_X_hybrid[ch=0][k=3][n]) + imag(ps_X_hybrid[ch=0][k=4][n])
       rMAC = M[I4,M1]; // rMAC = imag(ps_X_hybrid[ch=0][k=5][n]) / 8

      r5 = r5 * r2 (int);

      r0 = r0 + r5,  // r0 = real(ps_X_hybrid[ch=0][k=2][n]) + real(ps_X_hybrid[ch=0][k=5][n])
       r5 = M[I1,M1];   // r5 = real(ps_X_hybrid[ch=0][k=6][n]) / 8

      rMAC = rMAC * r2 (int);

      r1 = r1 + rMAC,   // r1 = imag(ps_X_hybrid[ch=0][k=2][n]) + imag(ps_X_hybrid[ch=0][k=5][n])
       M[I2,M2] = r0,   // store real(ps_X_hybrid[ch=0][k=2][n])
       rMAC = M[I4,M1]; // rMAC = imag(ps_X_hybrid[ch=0][k=6][n]) / 8

      r5 = r5 * r2 (int),
       M[I5,M2] = r1,   // store imag(ps_X_hybrid[ch=0][k=2][n])
       M[I2,M2] = r3;   // store real(ps_X_hybrid[ch=0][k=3][n])

      rMAC = rMAC * r2 (int),
       M[I2,M2] = r5,   // store real(ps_X_hybrid[ch=0][k=6][n])
       r1 = M[I4,M1];   // r1 = imag(ps_X_hybrid[ch=0][k=7][n]) / 8

      r1 = r1 * r2 (int),
       r0 = M[I1,M1];   // r0 = real(ps_X_hybrid[ch=0][k=7][n]) / 8
       M[I5,M2] = r4; // store imag(ps_X_hybrid[ch=0][k=3][n])

      r0 = r0 * r2 (int),
       M[I5,M2] = rMAC;   // store imag(ps_X_hybrid[ch=0][k=6][n])

      M[I2,M2] = r0,    // store real(ps_X_hybrid[ch=0][k=7][n])
       M[I5,M2] = r1;   // store imag(ps_X_hybrid[ch=0][k=7][n])


      r0 = M[r9 + $aac.mem.TMP + $aacdec.PS_HYBRID_TYPE_A_FIR_IFFT_LOOP_INDICE];
      r0 = r0 + 1;
      Null = r0 - $aacdec.PS_NUM_SAMPLES_PER_FRAME;
   if LT jump ps_hybrid_type_a_fir_inverse_fft_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
