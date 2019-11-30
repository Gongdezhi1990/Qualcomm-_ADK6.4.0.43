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
//    $aacdec.sbr_prediction_coeff
//
// DESCRIPTION:
//    - Calculates alpha_0(k) and alpha_1(k). Both are complex and are stored real
//       part then imaginary part.
//
//       d(k) = phi(2,2).phi(1,1) - 0.999999|phi(1,2)|^2
//
//                   | phi(0,1).phi(1,2) - phi(0,2).phi(1,1)
//                   | -------------------------------------       if d(k)<>0
//       alpha_1(k) =|                 d(k)
//                   |
//                   | 0                                           if d(k) =0
//
//                   | phi(0,1) + alpha_1(k).phi*(1,2)
//                   | -------------------------------             if phi(1,1)<>0
//       alpha_0(k) =|             phi(1,1)
//                   |
//                   | 0                                           if phi(1,1) =0
//
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - alpha_0(k) and alpha_1(k) stored as 1/16 of actual value
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I0, I4, DivResult, DivRemainder
//    - $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_prediction_coeff;
   .CODESEGMENT AACDEC_SBR_PREDICTION_COEFF_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_prediction_coeff:

   I0 = r9 + $aac.mem.TMP + $aacdec.SBR_alpha_0; // alpha_0
   I4 = r9 + $aac.mem.TMP + $aacdec.SBR_alpha_1; // alpha_1


   r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r];
   r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r];
   r6 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_12_r];
   r7 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_12_i];


   // d = phi(2,2).phi(1,1) - 0.999999|phi(1,2)|^2
   rMAC = r0 * r1;
   rMAC = rMAC - r6 * r6;
   rMAC = rMAC - r7 * r7;

   r4 = SIGNDET rMAC;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r] = r4;        // write scale over phi_22_r
   r8 = rMAC ASHIFT r4;
   // div = 1/d      calculated as 1/16 of actual value to stop overflow
   rMAC = 0.03125;                                     //=1/16 LSHIFT -1
   Div = rMAC/r8;

   Null = r8;
   if Z jump alpha_1_zero;
      // r8 = Real[phi(0,1).phi(1,2) - phi(0,2).phi(1,1)]
      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r];
      r5 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i];
      r2 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_02_r];

      rMAC = r0 * r6;
      rMAC = rMAC - r5 * r7;
      rMAC = rMAC - r2 * r1;
      r4 = SIGNDET rMAC;
      r8 = rMAC ASHIFT r4;
      r3 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r];
      r4 = r3 - r4;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r] = r4;      // write scale over phi_01_r


      // r10 = Imag[phi(0,1).phi(1,2) - phi(0,2).phi(1,1)]
      r3 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_02_i];

      rMAC = r5 * r6;
      rMAC = rMAC + r0 * r7;
      rMAC = rMAC - r3 * r1;
      r4 = SIGNDET rMAC;
      r10 = rMAC ASHIFT r4;
      r3 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r];
      r4 = r3 - r4;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i] = r4;      // write scale over phi_01_i

      jump writeback_alpha_1;

   alpha_1_zero:
      r8 = 0;
      r10 = 0;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r] = Null;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i] = Null;


   writeback_alpha_1:
      r2 = DivResult;

      r4 = SIGNDET r1;
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r] = r4;        // write scale over phi_22_r
      r1 = r1 ASHIFT r4;
      // div = 1/phi(1,1)            calculated as 1/16 of actual value to stop overflow
      rMAC = 0.03125;                               //=1/16 LSHIFT -1
      Div = rMAC/r1;

      // Real[alpha_1] = r8 /d
      // Imag[alpha_1] = r10/d
      r3 = r8 * r2 (frac);
      r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r];
      r3 = r3 ASHIFT r4;
      M[I4, MK1] = r3;
      r2 = r10 * r2 (frac);
      r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i];
      r2 = r2 ASHIFT r4;
      M[I4, -MK1] = r2;

      Null = r1;
      if Z jump alpha_0_zero;

         // r8 = Real[phi(0,1) + alpha_1.phi*(1,1)]
         // scale down phi(0,1) and then scale answer back up since alpha_1 is stored as 1/16th of actual value
         rMAC = r0 * (-0.0625);
         rMAC = rMAC - r3 * r6;
         rMAC = rMAC - r2 * r7;
         r4 = SIGNDET rMAC;
         r8 = rMAC ASHIFT r4;
         rMAC = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r];
         r4 = rMAC - r4;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r] = r4;      // write scale over phi_01_r

         // r10 = Imag[phi(0,1) + alpha_1.phi*(1,1)]
         // scale down phi(0,1) and then scale answer back up since alpha_1 is stored as 1/16th of actual value
         rMAC = r5 * (-0.0625);
         rMAC = rMAC - r2 * r6;
         rMAC = rMAC + r3 * r7;
         r4 = SIGNDET rMAC;
         r10 = rMAC ASHIFT r4;
         rMAC = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r];
         r4 = rMAC - r4;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i] = r4;      // write scale over phi_01_i

         jump writeback_alpha_0;

      alpha_0_zero:
         r8 = 0;
         r10 = 0;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r] = Null;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i] = Null;

   writeback_alpha_0:
      // Real[alpha_0] = r8 /phi(1,1)
      // Imag[alpha_0] = r10/phi(1,1)
      r6 = DivResult;
      rMAC = r8 * r6;
      r0 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r];
      r4 = r0 + 4;
      r0 = rMAC ASHIFT r4;
      M[I0, MK1] = r0;
      rMAC = r10 * r6;
      r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i];
      r4 = r1 + 4;
      r1 = rMAC ASHIFT r4;
      M[I0, -MK1] = r1;



   // if (|alpha_0|^2 >= 16 OR |alpha_1|^2 >= 16)
   //   alpha_0 = 0
   //   alpha_1 = 0
   rMAC = r0 * r0;
   rMAC = rMAC + r1 * r1;
   r2 = 0.0625;            // alphas stored 1/16 of actual answer, 0.0625 = 16*(1/16)^2
   Null = rMAC - r2;
   if GE jump zeros_out;

   r0 = M[I4, MK1];
   rMAC = r0 * r0;
   r0 = M[I4, -MK1];
   rMAC = rMAC + r0 * r0;
   Null = rMAC - r2;
   if LT rts;

   zeros_out:
      r0 = 0;
      M[I0, MK1] = r0;
      M[I0, MK1] = r0;
      M[I4, MK1] = r0;
      M[I4, MK1] = r0;
   rts;

.ENDMODULE;

#endif
