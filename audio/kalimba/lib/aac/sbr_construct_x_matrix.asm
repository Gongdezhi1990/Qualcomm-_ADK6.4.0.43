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
//    $aacdec.sbr_construct_x_matrix
//
// DESCRIPTION:
//    - sets kx_band, kx_prev, M_band and M_prev in sbr_info
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-3, r5-6
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_construct_x_matrix;
   .CODESEGMENT AACDEC_SBR_CONSTRUCT_X_MATRIX_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_construct_x_matrix:

   // push rLink onto stack
   push rLink;

   // r5 = kx
   // r6 = M
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r5 = M[r0 + $aacdec.SBR_kx];
   r6 = M[r0 + $aacdec.SBR_M];

   // if (headercount ~= 0)
   r0 = M[r0 + $aacdec.SBR_header_count];
   if Z jump use_old_values;

      // if(numTimeSlotsRate <= sbr.t_E(ch, 1))
      //    kx_band = sbr.kx_prev;
      //    M_band = sbr.M_prev;
      // else
      //    kx_band = sbr.Kx;
      //    M_band = sbr.M;
      // end;
      r0 = M[r9 + $aac.mem.CURRENT_CHANNEL];
      r0 = r0 * (6*ADDR_PER_WORD) (int);

      r0 = r0 + r9;
      r0 = M[$aac.mem.SBR_t_E + r0];
      r1 = M[r9 + $aac.mem.SBR_numTimeSlotsRate];
      Null = r0 - r1;
      if LT jump not_prev;
         r0 = M[r9 + $aac.mem.SBR_info_ptr];
         r2 = M[r0 + $aacdec.SBR_kx_prev];
         r3 = M[r0 + $aacdec.SBR_M_prev];
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         M[r0 + $aacdec.SBR_kx_band] = r2;
         M[r0 + $aacdec.SBR_M_band] = r3;
         jump out_main_if;

      not_prev:
         r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
         M[r0 + $aacdec.SBR_kx_band] = r5;
         M[r0 + $aacdec.SBR_M_band] = r6;


   out_main_if:
   use_old_values:

   // sbr.kx_prev = sbr.Kx;
   // sbr.M_prev = sbr.M;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r0 + $aacdec.SBR_kx_prev] = r5;
   M[r0 + $aacdec.SBR_M_prev] = r6;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
