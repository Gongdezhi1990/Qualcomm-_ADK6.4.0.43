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
//    $aacdec.sbr_auto_correlation_opt
//
// DESCRIPTION:
//    - Calculates the covariance matrix elements used for calculation of
//       prediction coeffiecients
//
//                x
//    phi(i,j) = sum [ X_sbr(k, n - i + tHFAdj).X_sbr*(k, n - j + tHFAdj) ]
//               n=0
//
//    x = numTimeSlotsRate+5
//    for [i,j] = [0,1], [0,2], [1,1], [1,2], [2,2]
//
//
// INPUTS:
//    - r8 = k
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r8, r10, rMAC, I0-I7, M0-M3
//    - $aacdec.tmp
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_auto_correlation_opt;
   .CODESEGMENT AACDEC_SBR_AUTO_CORRELATION_OPT_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_auto_correlation_opt:

   // X(z) = X_sbr(k, z+tHFAdj)
   // w = numTimeSlotsRate+5
   //
   // phi(0,1) = X( 0:w  ).X*(-1:w-1)
   // phi(0,2) = X( 0:w  ).X*(-2:w-2)
   // phi(1,1) = X(-1:w-1).X*(-1:w-1)
   // phi(1,2) = X(-1:w-1).X*(-2:w-2)
   // phi(2,2) = X(-2:w-2).X*(-2:w-2)
   //
   // So, calculate phi(0,1), phi(0,2) and phi(1,1).
   // Then,
   // phi(1,2) = phi(0,1) - X(w  ).X*(w-1) + X(-1).X*(-2)
   // phi(2,2) = phi(1,1) - X(w-1).X*(w-1) + X(-2).X*(-2)
   //
   // Note: phi(1,1) and phi(2,2) are real only so no imaginary parts are calculated or stored

   Words2Addr(r8);
   r0 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
   r0 = r0 + r8;
   I2 = r0 + ((($aacdec.SBR_tHFAdj*ADDR_PER_WORD) - (2*ADDR_PER_WORD)) * $aacdec.X_SBR_WIDTH);
   r0 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
   r0 = r0 + r8;
   I6 = r0 + ((($aacdec.SBR_tHFAdj*ADDR_PER_WORD) - (2*ADDR_PER_WORD)) * $aacdec.X_SBR_WIDTH);

   M0 = 0;
   M1 = ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
   M2 = -($aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
   M3 = (2 * $aacdec.X_SBR_WIDTH*ADDR_PER_WORD);

   r8 = $aacdec.SBR_tHFGen - $aacdec.SBR_tHFAdj;
#ifdef AACDEC_ELD_ADDITIONS
   r1 = $aacdec.SBR_tHFGen_eld - $aacdec.SBR_tHFAdj;
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z r8 = r1;
#endif


   // calculate phi_11_r
   loop_1:
      r10 = M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted];
      I0 = I2;
      I4 = I6,
       r0 = M[I0, M1]; // dummy
      r1 = M[I4, M1], // dummy
       r2 = M[I0, M1];

      rMAC = 0;
      do calc_loop_1;
         rMAC = rMAC + r2 * r2,
          r3 = M[I4, M1];
         rMAC = rMAC + r3 * r3,
          r2 = M[I0, M1];
      calc_loop_1:


   // calculate phi_01_r
   loop_2:
      r10 = M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted];
      I0 = I2;
      I4 = I6,
       r0 = M[I0, M1]; // dummy
      r0 = M[I0, M1],
       r1 = M[I4, M1]; // dummy

      r6 = SIGNDET rMAC;
      r6 = r6 - 1;
      rMAC = rMAC ASHIFT r6,
       r2 = M[I0, M0],
       r1 = M[I4, M1];
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r] = rMAC;

      rMAC = 0;
      do calc_loop_2;
         rMAC = rMAC + r2 * r0,
          r0 = M[I0, M1],
          r3 = M[I4, M0];
         rMAC = rMAC + r3 * r1,
          r2 = M[I0, M0],
          r1 = M[I4, M1];
      calc_loop_2:


   // calculate phi_01_i
   loop_3:
      r5 = SIGNDET rMAC;
      r5 = r5 - 1;
      r0 = r5 - r6;
      if GE jump shift_2;
         r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r];
         r1 = r1 ASHIFT r0;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r] = r1;
         r6 = r5;
      shift_2:

      r10 = M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted];
      I0 = I2;
      I4 = I6,
       r0 = M[I0, M1]; // dummy
      r1 = M[I4, M3]; // dummy

      rMAC = rMAC ASHIFT r6,
       r0 = M[I0, M1],
       r3 = M[I4, M2];
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r] = rMAC;

      rMAC = 0;
      do calc_loop_3;
         rMAC = rMAC + r3 * r0,
          r2 = M[I0, M0],
          r1 = M[I4, M3];
         rMAC = rMAC - r2 * r1,
          r0 = M[I0, M1],
          r3 = M[I4, M2];
      calc_loop_3:


   // calculate phi_02_r
   loop_4:
      r5 = SIGNDET rMAC;
      r5 = r5 - 1;
      r0 = r5 - r6;
      if GE jump shift_3;
         r6 = r5;
         r1 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r];
         r1 = r1 ASHIFT r0;
         r5 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r];
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r] = r1;
         r5 = r5 ASHIFT r0;
         M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r] = r5;
      shift_3:

      r10 = M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted];
      I0 = I2;
      I4 = I6,
       r0 = M[I0, M3];

      rMAC = rMAC ASHIFT r6,
       r2 = M[I0, M2],
       r1 = M[I4, M3];
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i] = rMAC;

      rMAC = 0;
      do calc_loop_4;
         rMAC = rMAC + r2 * r0,
          r0 = M[I0, M3],
          r3 = M[I4, M2];
         rMAC = rMAC + r3 * r1,
          r2 = M[I0, M2],
          r1 = M[I4, M3];
      calc_loop_4:


   // calculate phi_02_i
   loop_5:
      r5 = SIGNDET rMAC;
      r5 = r5 - 1;
      r0 = r5 - r6;
      if GE jump shift_4;
         r6 = r5;
         I0 = r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r;
         r1 = M[I0, MK1];
         r1 = r1 ASHIFT r0,
          r5 = M[I0, -MK1];
         M[I0, MK2] = r1;
         r5 = r5 ASHIFT r0,
          r1 = M[I0, -MK1];
         M[I0, MK1] = r5;
         r1 = r1 ASHIFT r0;
         M[I0, 0] = r1;
      shift_4:

      r10 = M[r9 + $aac.mem.SBR_numTimeSlotsRate_adjusted];
      I0 = I2;
      I4 = I6;
      r1 = M[I4, M3]; // dummy

      M0 = -(2 * $aacdec.X_SBR_WIDTH*ADDR_PER_WORD);
      M1 = (3 * $aacdec.X_SBR_WIDTH*ADDR_PER_WORD);

      rMAC = rMAC ASHIFT r6,
       r0 = M[I0, M3],
       r3 = M[I4, M0];
      M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_02_r] = rMAC;

      rMAC = 0;
      do calc_loop_5;
         rMAC = rMAC + r3 * r0,
          r2 = M[I0, M2],
          r1 = M[I4, M1];
         rMAC = rMAC - r2 * r1,
          r0 = M[I0, M3],
          r3 = M[I4, M0];
      calc_loop_5:


   finished_looping:

   r5 = SIGNDET rMAC;
   r5 = r5 - 1;
   r3 = r5 - r6;
   if GE jump shift_5;
      r6 = r5;
      I0 = r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r;
      r1 = M[I0, MK1];
      r1 = r1 ASHIFT r3,
       r5 = M[I0, -MK1];
      M[I0, MK2] = r1;
      r5 = r5 ASHIFT r3,
       r1 = M[I0, -MK1];
      M[I0, MK2] = r5;
      r1 = r1 ASHIFT r3,
       r5 = M[I0, -MK1];
      M[I0, MK1] = r1;
      r5 = r5 ASHIFT r3;
      M[I0, 0] = r5;
   shift_5:
   rMAC = rMAC ASHIFT r6;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_02_i] = rMAC;

   M1 = ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD);

   r7 = r0;
   r10 = r2;
   r0 = M[I4, M1];
   r8 = r0;
   r5 = M[I4, M1];
   r2 = M[I2, M1],
    r3 = M[I6, M1];
   r0 = M[I2, M1],
    r1 = M[I6, M1];

   // phi(1,2) = phi(0,1) - X(w).X*(w-1) + X(-1).X*(-2)
   rMAC = r2 * r0;
   rMAC = rMAC + r3 * r1;
   rMAC = rMAC - r10 * r7;
   rMAC = rMAC - r5 * r8;
   rMAC = rMAC ASHIFT r6;
   r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_r];
   rMAC = rMAC + r4;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_12_r] = rMAC;

   rMAC = r2 * r1;
   rMAC = rMAC - r3 * r0;
   rMAC = rMAC - r5 * r7;
   rMAC = rMAC + r10 * r8;
   rMAC = rMAC ASHIFT r6;
   r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_01_i];
   rMAC = rMAC + r4;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_12_i] = rMAC;

   // phi(2,2) = phi(1,1) - X(w-1).X*(w-1) + X(-2).X*(-2)
   //  real only
   rMAC = r2 * r2;
   rMAC = rMAC + r3 * r3;
   rMAC = rMAC - r7 * r7;
   rMAC = rMAC - r8 * r8;
   rMAC = rMAC ASHIFT r6;
   r4 = M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_11_r];
   rMAC = rMAC + r4;
   M[r9 + $aac.mem.TMP + $aacdec.SBR_phi_22_r] = rMAC;

   rts;

.ENDMODULE;

#endif
