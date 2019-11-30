// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCDEC_SYNTHESIS_SUBBAND_FILTER_INCLUDED
#define SBCDEC_SYNTHESIS_SUBBAND_FILTER_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcdec.synthesis_subband_filter
//
// DESCRIPTION:
//    Synthesis subband filter
//
// INPUTS:
//    - I1 = cbuffer address for output samples
//    - L1 = length of cbuffer
//    - r5 = current channel 0 (left),  1 (right)
//
// OUTPUTS:
//    - I1 = updated
//
// TRASHED REGISTERS:
//    rMAC,  r0-r5,  r10,  DoLoop,  I0-I2,  I4,  I5,  M0-M3
//
// NOTES:
//
//   Equivalent Matlab code: M = nrof_subbands,   L = M*10
//
//      @verbatim
//      % get new subband samples
//      S = In(1:M);
//
//      % shift down by M*2 samples
//      V(M*2+1:L*2) = V(1:(L-M)*2);
//
//      % do matrixing
//      V(1:M*2) = (SynthMat'*S)';
//
//      % build vector U
//      U = zeros(1, L);
//      for i = 0:(L/M/2-1),
//         U(i*(M*2)+(1:M)) = V(i*(M*4)+(1:M));
//         U(i*(M*2)+M+(1:M)) = V(i*(M*4)+(M*3)+(1:M));
//      end
//
//      % window the data
//      W = U.*proto_filt*M;
//
//      % calculate M samples
//      Out = zeros(1, M);
//      for i=0:(L/M - 1),
//         Out(1:M) = Out(1:M) + W((1:M)+M*i);
//      end
//      @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcdec.synthesis_subband_filter;
   .CODESEGMENT SBCDEC_SYNTHESIS_SUBBAND_FILTER_PM;
   .DATASEGMENT DM;

   $sbcdec.synthesis_subband_filter:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcdec.SYNTHESIS_SUBBAND_FILTER_ASM.SYNTHESIS_SUBBAND_FILTER.PATCH_ID_0, r1)
#endif

   // M0 = nrof_subbands = M

   r1 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   Words2Addr (r1);
   M0 = r1;

   // r3 = next block increment size - 1;

   r2 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];

   r3 = r2 * r1 (int);
   r3 = r3 - ADDR_PER_WORD;

   // I2 = starting offset for current channel  (r5 = channel)
   r0 = r1 * r5 (int);
   // if mono but right channel selected still use single channel starting offset
   Null = r2 - 1;
   if Z r0 = 0;

   r4 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I2 = r0 + r4;
   // setup V ptr for current channel (r5 = channel)
   r0 = r9 + $sbc.mem.SYNTHESIS_VCH1PTR_FIELD;
   Words2Addr (r5);
   r0 = M[r5 + r0];


   I0 = r0;

   r4 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];


#ifdef BASE_REGISTER_MODE
   r0 = M[r9 + $sbc.mem.SYNTHESIS_VCH1_FIELD];
   r1 = M[r9 + $sbc.mem.SYNTHESIS_VCH2_FIELD];

   Null = r5;
   if NZ r0 = r1;
   push r0;
   pop B0;
#endif

#ifndef SBC_WBS_ONLY
   // See whether nrof_subands (M) = 4 or 8
   Null = M0 - (4*ADDR_PER_WORD);
   if Z jump M_equals_4;
#endif
      // set up length reg for V ring buffer
      L0 = (160*ADDR_PER_WORD);

      block_loop_M8:

         // TODO put outside the loop for optimisation
         r0 = M[r9 + $sbc.mem.WIN_COEFS_M8_FIELD];
         I4 = r0;
         r0 = M[r9 + $sbc.mem.SYNTHESIS_COEFS_M8_FIELD];
         I5 = r0; // I5 = synthesis matrix M=8


         // M1 = 1
         M1 = ADDR_PER_WORD;
         // M2 = -2M
         M2 = -(16*ADDR_PER_WORD);
         // M3 = 1-M
         M3 = -(7*ADDR_PER_WORD);


         // insert 2M new matrixed samples into V ring buffer
         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
         r10 = r10 + r10;
         // read 1st subband sample
         // and read 1st col coef from synthesis matrix
         r0 = M[I2, MK1],       r1 = M[I5, MK1];
         do matrix_loop_M8;

            // read 2nd subband sample
            // and read 2nd col coef from synthesis matrix
            rMAC = r0 * r1,         r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 3rd subband sample
            // and read 3rd col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 4th subband sample
            // and read 4th col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 5th subband sample
            // and read 5th col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 6th subband sample
            // and read 6th col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 7th subband sample
            // and read 7th col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 8th subband sample
            // and read 8th col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, M3],   r1 = M[I5, M1];

            // read 1st subband sample
            // and read 1st col (next row) coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // save matrix result in V ring buffer
            M[I0, MK1] = rMAC;

         matrix_loop_M8:

         // dummy read to set I0 to start of V ring buffer ie -M*2
         r0 = M[I0, M2];
         // move I2 onto start of next block/channel
         I2 = I2 + r3;
         // M*3
         M1 = (24*ADDR_PER_WORD);
         // (1 - M*9)
         M2 = -(71*ADDR_PER_WORD);
         // (1 - M*19)
         M3 = -(151*ADDR_PER_WORD);


         call output_code;


         // dummy read to set V pointer correctly: I0 = I0 - 6M;
         M1 = -(48*ADDR_PER_WORD);
         r0 = M[I0, M1];

         // loop around for every block
         r4 = r4 - 1;
      if NZ jump block_loop_M8;

#ifndef SBC_WBS_ONLY
      jump all_done;



   M_equals_4:
      // set up length reg for V ring buffer
      L0 = (80*ADDR_PER_WORD);

      block_loop_M4:

         // TODO put outside the loop for optimisation
         r0 = M[r9 + $sbc.mem.WIN_COEFS_M4_FIELD];
         I4 = r0; // I4 = window coefficients M=4
         r0 = M[r9 + $sbc.mem.SYNTHESIS_COEFS_M4_FIELD];
         I5 = r0;   // I5 = synthesis matrix M=4


         // M1 = 1
         M1 = ADDR_PER_WORD;
         // M2 = -2M
         M2 = -(8*ADDR_PER_WORD);
         // M3 = 1-M
         M3 = -(3*ADDR_PER_WORD);


         // insert 2M new matrixed samples into V ring buffer
         r10 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
         r10 = r10 + r10;
         // read 1st subband sample
         // and read 1st col coef from synthesis matrix
         r0 = M[I2, MK1],       r1 = M[I5, MK1];
         do matrix_loop_M4;

            // read 2nd subband sample
            // and read 2nd col coef from synthesis matrix
            rMAC = r0 * r1,         r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 3rd subband sample
            // and read 3rd col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // read 4th subband sample
            // and read 4th col coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, M3],   r1 = M[I5, M1];

            // read 1st subband sample
            // and read 1st col (next row) coef from synthesis matrix
            rMAC = rMAC + r0 * r1,  r0 = M[I2, MK1],    r1 = M[I5, MK1];

            // save matrix result in V ring buffer
            M[I0, MK1] = rMAC;

         matrix_loop_M4:


         // dummy read to set I0 to start of V ring buffer ie -M*2
         r0 = M[I0, M2];
         // move I2 onto start of next block/channel
         I2 = I2 + r3;
         // M*3
         M1 = (12*ADDR_PER_WORD);
         // (1 - M*9)
         M2 = -(35*ADDR_PER_WORD);
         // (1 - M*19)
         M3 = -(75*ADDR_PER_WORD);


         call output_code;


         // dummy read to set V pointer correctly: I0 = I0 - 6M;
         M1 = -(24*ADDR_PER_WORD);
         r0 = M[I0, M1];

         // loop around for every block
         r4 = r4 - 1;
      if NZ jump block_loop_M4;


   all_done:
#endif
   // Write updated V ptr back to memory
   r0 = I0;

   r4 = r9 + $sbc.mem.SYNTHESIS_VCH1PTR_FIELD;
   M[r5 + r4] = r0;

   L0 = 0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;



output_code:


   // calculate M output samples
   r10 = M0;
   Addr2Words (r10);

   // combine: building vector U,  windowing data,  and calculating output sample
   // get V(0+n)
   // and get wind_coef(0+n)
   r0 = M[I0, M1],         r1 = M[I4, M0];

   do output_loop;
      // get V(3M+n)
      // and get wind_coef(M+n)
      rMAC = r0 * r1,         r0 = M[I0, M0],   r1 = M[I4, M0];

      // get V(4M+n)
      // and get wind_coef(2M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M1],   r1 = M[I4, M0];

      // get V(7M+n)
      // and get wind_coef(3M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M0],   r1 = M[I4, M0];

      // get V(8M+n)
      // and get wind_coef(4M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M1],   r1 = M[I4, M0];

      // get V(11M+n)
      // and get wind_coef(5M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M0],   r1 = M[I4, M0];

      // get V(12M+n)
      // and get wind_coef(6M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M1],   r1 = M[I4, M0];

      // get V(15M+n)
      // and get wind_coef(7M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M0],   r1 = M[I4, M0];

      // get V(16M+n)
      // and get wind_coef(8M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M1],   r1 = M[I4, M0];

      // get V(19M+n)
      // and get wind_coef(9M+n)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M3],   r1 = M[I4, M2];

      // effectively n = n + 1

      // get V(0+n)           (next sample)
      // and get wind_coef(0+n)   (next sample)
      rMAC = rMAC + r0 * r1,  r0 = M[I0, M1],   r1 = M[I4, M0];

      // scale so that PCM data is between -1.0 and + 1.0 (24bit)
      r2 = rMAC * -4 (int) (sat);
      // write to cbuffer (24bit)
      M[I1, MK1] = r2;
   output_loop:

   rts;



.ENDMODULE;

#endif
