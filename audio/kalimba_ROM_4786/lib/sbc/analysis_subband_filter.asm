// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_ANALYSIS_SUBBAND_FILTER_INCLUDED
#define SBCENC_ANALYSIS_SUBBAND_FILTER_INCLUDED

#include "sbc.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.analysis_subband_filter
//
// DESCRIPTION:
//    Analysis subband filter
//
// INPUTS:
//    - I1 = cbuffer address for input samples
//    - L1 = length of cbuffer
//    - r5 = current channel 0 (left), 1 (right)
//    - r6 = sample increment to next block = 0 (true mono)
//                                         = nrof_subbands (stereo or mono conversion)
//
// OUTPUTS:
//    - I1 = updated pointer
//
// TRASHED REGISTERS:
//    rMAC, r0-r4, r10, I0-I6, M0-M3
//
// NOTES:
//    Equivalent Matlab code: M = nrof_subbands,  L = M*10
//
//      @verbatim
//      % shift down by M samples
//      X(M+1:L) = X(1:L-M);
//
//      % insert new data
//      X(1:M) = In(M:-1:1);
//
//      % window the data
//      Z = X .* proto_filt;
//
//      % partial calculation
//      Y = zeros(1,M*2);
//      for j = 0:(L/M/2-1),
//         Y = Y + Z((1:M*2)+(M*2)*j);
//      end
//
//      % do matrixing
//      Out = (AnalyMat * Y')';
//      @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.analysis_subband_filter;
   .CODESEGMENT SBCENC_ANALYSIS_SUBBAND_FILTER_PM;
   .DATASEGMENT DM;

   $sbcenc.analysis_subband_filter:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcenc.ANALYSIS_SUBBAND_FILTER_ASM.ANALYSIS_SUBBAND_FILTER.PATCH_ID_0, r3)
#endif


   r3 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];

   // M0 = M * 2
   r0 = r3 + r3;
   Words2Addr(r0);
   M0 = r0;
   M1 = ADDR_PER_WORD;
   // M2 = 1 - (M * 2)
   M2 = M1 - M0;
   r0 = r3 * 8 (int);
   Words2Addr(r0);
   // M1 = 1 - (M*8)
   M3 = M1 - r0;

   // I2 = starting offset for current channel  (r5 = channel)
   Words2Addr(r5);
   r0 = r3 * r5 (int);
   // if true mono then offset is always zero
   Null = r6;
   if Z r0 = 0;

   Words2Addr(r6);

   r4 = M[r9 + $sbc.mem.AUDIO_SAMPLE_FIELD];
   I2 = r0 + r4;


   // setup V ptr for current channel (r5 = channel)
   r4 = r9 + $sbc.mem.ANALYSIS_XCH1PTR_FIELD;
   r0 = M[r5 + r4];


   I6 = r0;

   r4 = M[r9 + $sbc.mem.NROF_BLOCKS_FIELD];


#ifdef BASE_REGISTER_MODE
   r0 = M[r9 + $sbc.mem.ANALYSIS_XCH1_FIELD];

   r2 = M[r9 + $sbc.mem.ANALYSIS_XCH2_FIELD];

   Null = r5;
   if NZ r0 = r2;
   push r0;
   pop B4;
#endif

   // See whether nrof_subands (M) = 4 or 8
#ifndef SBC_WBS_ONLY
   Null = r3 - 4;
   if Z jump M_equals_4;
#endif
      // set up length reg for X ring buffer
      r0 = $sbcenc.ANALYSIS_BUFFER_LENGTH;
      Words2Addr(r0);
      L4 = r0;

      // set shift amount for scaling
      r2 = -4;

      block_loop_M8:

         // TODO put outside the loop for optimisation
         // I0 = window coeffiecients M=8
         r0 = M[r9 + $sbc.mem.WIN_COEFS_M8_FIELD];
         I0 = r0;

         r0 = M[r9 + $sbc.mem.ANALYSIS_COEFS_M8_FIELD];
         I5 = r0;// I5 = analysis matrix M=8

         // set V ptr
         I4 = I6;


         // insert M new samples into X ring buffer
         // and scale samples appropriately
         r0 = M[I1,MK1];
         r0 = r0 ASHIFT r2,      r1 = M[I1, MK1];
         r1 = r1 ASHIFT r2,   r0 = M[I1, MK1],    M[I4, -MK1] = r0;
         r0 = r0 ASHIFT r2,   r1 = M[I1, MK1],    M[I4, -MK1] = r1;
         r1 = r1 ASHIFT r2,   r0 = M[I1, MK1],    M[I4, -MK1] = r0;
         r0 = r0 ASHIFT r2,   r1 = M[I1, MK1],    M[I4, -MK1] = r1;
         r1 = r1 ASHIFT r2,   r0 = M[I1, MK1],    M[I4, -MK1] = r0;
         r0 = r0 ASHIFT r2,   r1 = M[I1, MK1],    M[I4, -MK1] = r1;
         r1 = r1 ASHIFT r2,   M[I4, -MK1] = r0;
         M[I4, -MK1] = r1;


         call share_code;


         do matrix_loop_M8;
            // do (matrix row) * (Y vector) sum of products
            rMAC = r0 * r1,         r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            // go back to start of Y vector
            rMAC = rMAC + r0 * r1,  r0 = M[I3, M2],   r1 = M[I5, M1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            // write to subband out.
            M[I2, MK1] = rMAC;
         matrix_loop_M8:

         // move I2 onto start of next block/channel
         I2 = I2 + r6;
         // loop around for every block
         r4 = r4 - 1;
      if NZ jump block_loop_M8;

#ifndef SBC_WBS_ONLY
      jump all_done;


   M_equals_4:

      // set up length reg for X ring buffer
      r0 = $sbcenc.ANALYSIS_BUFFER_LENGTH;
      Words2Addr(r0);
      r0 = r0 LSHIFT -1;   // half the size when M=4
      L4 = r0;

      // set shift amount for scaling
      r2 = -3;

      block_loop_M4:

         // TODO put outside the loop for optimisation
         r0 = M[r9 + $sbc.mem.WIN_COEFS_M4_FIELD];
         I0 = r0; // I0 = window coefficients M=4


         r0 = M[r9 + $sbc.mem.ANALYSIS_COEFS_M4_FIELD];
         I5 = r0;// I5 = analysis matrix M=4


         // set V ptr
         I4 = I6;

         // insert M new samples into X ring buffer
         // and scale samples appropriately
         r0 = M[I1, MK1];
         r0 = r0 ASHIFT r2,   r1 = M[I1, MK1];
         r1 = r1 ASHIFT r2,   r0 = M[I1, MK1],    M[I4, -MK1] = r0;
         r0 = r0 ASHIFT r2,   r1 = M[I1, MK1],    M[I4, -MK1] = r1;
         r1 = r1 ASHIFT r2,   M[I4, -MK1] = r0;
         M[I4, -MK1] = r1;


         call share_code;


         do matrix_loop_M4;
            // do (matrix row) * (Y vector) sum of products
            rMAC = r0 * r1,         r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            // go back to start of Y vector
            rMAC = rMAC + r0 * r1,  r0 = M[I3, M2],   r1 = M[I5, M1];
            rMAC = rMAC + r0 * r1,  r0 = M[I3, MK1],    r1 = M[I5, MK1];
            // write to subband out.
            M[I2, MK1] = rMAC;
         matrix_loop_M4:

         // move I2 onto start of next block/channel
         I2 = I2 + r6;

         // loop around for every block
         r4 = r4 - 1;
      if NZ jump block_loop_M4;

   all_done:


#endif
   // Write updated X ptr back to memory
   r0 = I6;

   r4 = r9 + $sbc.mem.ANALYSIS_XCH1PTR_FIELD;
   M[r5 + r4] = r0;


   L4 = 0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;


share_code:
         // save V ptr for the next block
         I6 = I4;
         // increment I4 (allowing ring buffer wrap around)
         r0 = M[I4, M1];
         // Window the data & do the partial calculation

         r0 = M[r9 + $sbc.mem.ANALYSIS_Y_FIELD];
         I3 = r0;


         // loop counter = M*2
         r0 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
         r0 = r0 LSHIFT 1;
         r10 = r0;

         // read 1st sample from X
         // and read 1st value from win_coef
         r0 = M[I4, M0],         r1 = M[I0, M0];
         do window_calc_loop_M4;
            rMAC = r0 * r1,         r0 = M[I4, M0],      r1 = M[I0, M0];
            rMAC = rMAC + r0 * r1,  r0 = M[I4, M0],      r1 = M[I0, M0];
            rMAC = rMAC + r0 * r1,  r0 = M[I4, M0],      r1 = M[I0, M0];
            rMAC = rMAC + r0 * r1,  r0 = M[I4, M3],      r1 = M[I0, M3];
            rMAC = rMAC + r0 * r1,  r0 = M[I4, M0],      r1 = M[I0, M0];
            // store partial Y
            M[I3, MK1] = rMAC;
         window_calc_loop_M4:


         r0 = M[r9 + $sbc.mem.ANALYSIS_Y_FIELD];
         I3 = r0;

         // loop counter = M
         r10 = r3;
         // read 1st sample from Y
         // and read 1st coef from analysis matrix
         r0 = M[I3, MK1],          r1 = M[I5, MK1];

         rts;



.ENDMODULE;

#endif // SBCENC_ANALYSIS_SUBBAND_FILTER_INCLUDED
