// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_ELD_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.eld_sbr_grid
//
// DESCRIPTION:
//    Get information about how the current SBR frame is subdivided
//
// INPUTS:
//    - r5 ch
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r4, r6-r8, r10, I1, I4
//
// *****************************************************************************
.MODULE $M.aacdec.eld_sbr_grid;
   .CODESEGMENT AACDEC_LD_SBR_GRID_PM;
   .DATASEGMENT DM;

   $aacdec.eld_sbr_grid:

   push rLink;

   Words2Addr(r5); // In all code below r5 is a channel address offset
   // *****************************************************************************
   // extract bs_frame_class
   // *****************************************************************************
   call $aacdec.get1bit;
   r0 = r5 + r9;
   M[$aac.mem.SBR_bs_frame_class + r0] = r1;
   // *****************************************************************************
   // switch(bs_frame_class)
   // *****************************************************************************
   Null = r1 - $aacdec.SBR_FIXFIX;
   if NZ jump not_fixfix;

   case_fixfix:
      // *****************************************************************************
      // extract tmp
      // *****************************************************************************
      call $aacdec.get2bits;           // output is in r1
      r10 = 1 LSHIFT r1;
      r1 = r10 - 4;
      if GT r10 = r10 - r1;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_num_env + r0] = r10;
      r8 = r10;

      // *****************************************************************************
      // if (bs_num_env[ch] == 1)
      // *****************************************************************************
      Null = r10 - 1;
      if NZ jump bsampres_cal_not_reqd;

      // *****************************************************************************
      // extract bs_amp_res
      // *****************************************************************************
      call $aacdec.get1bit;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_amp_res] = r1;

   bsampres_cal_not_reqd:
      // *****************************************************************************
      // extract bs_freq_res
      // *****************************************************************************
      call $aacdec.get1bit;

      // *****************************************************************************
      // for (env = 1; env < bs_num_env[ch]; env++)
      // *****************************************************************************
      r0 = r5 * 6 (int);
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r0;
      I1 = r2 + $aacdec.SBR_bs_freq_res;
      do fixfix_bs_freq_res_loop;
         M[I1, MK1] = r1;
      fixfix_bs_freq_res_loop:

      // *****************************************************************************
      // SBR_abs_bord_lead[ch] = 0
      // *****************************************************************************
      r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r2 = r2 + r5;
      M[r2 + $aacdec.SBR_abs_bord_lead] = Null;

      // *****************************************************************************
      // SBR_abs_bord_trail[ch] = SBR_numTimeSlots
      // *****************************************************************************
      r1 = M[r9 + $aac.mem.ELD_sbr_numTimeSlots];
      M[r2 + $aacdec.SBR_abs_bord_trail] = r1;
      jump end_switch_bs_frame_class;

   not_fixfix:
      // *****************************************************************************
      // extract bs_transient_position
      // *****************************************************************************
      Null = r1 - $aacdec.SBR_LDTRAN;
      if Z call $aacdec.get4bits;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_transient_position + r0] = r1;


      r1 = r1 * (4*ADDR_PER_WORD)(int);               // [bs_transient_position][num_envelopes]
      r2 = &$aacdec.ld_envelopetable512;
      r3 = &$aacdec.ld_envelopetable480;
      Null = M[r9 + $aac.mem.ELD_frame_length_flag];
      if NZ r2 = r3;

      r0= r2 + r1;
      call $mem.ext_window_access_as_ram;
      r10 = M[r0];

      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_num_env + r0] = r10;
      r8 = r10;
      // *****************************************************************************
      // for (env = 0; env < bs_num_env[ch]; env++)
      // *****************************************************************************
      r0 = r5 * 6 (int);
      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r1 = r1 + r0;
      I1 = r1 + $aacdec.SBR_bs_freq_res;
      do ldtrans_bs_freq_res_loop;
         call $aacdec.get1bit;
         M[I1, MK1] = r1;
       ldtrans_bs_freq_res_loop:

   end_switch_bs_frame_class:
      r0 = 1;
      Null = r8 - 1;
      if GT r0 = r0 + r0;
      r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r1 = r1 + r5;
      M[r1 + $aacdec.SBR_bs_num_noise] = r0;

      Addr2Words(r5);

      call $aacdec.sbr_envelope_time_border_vector;
      call $aacdec.sbr_envelope_noise_border_vector;

      jump $pop_rLink_and_rts;

.ENDMODULE;

#endif


