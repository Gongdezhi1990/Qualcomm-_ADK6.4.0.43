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
//    $aacdec.sbr_grid
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
.MODULE $M.aacdec.sbr_grid;
   .CODESEGMENT AACDEC_SBR_GRID_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_grid:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5); // In all code below r5 is a channel address offset

   // SBR_bs_frame_class
   call $aacdec.get2bits;
   r0 = r5 + r9;
   M[$aac.mem.SBR_bs_frame_class + r0] = r1;

   // switch(SBR_bs_frame_class)
   Null = r1 - $aacdec.SBR_FIXFIX;
   if NZ jump not_fixfix;
      // case FIXFIX
      call $aacdec.get2bits;
      r10 = 1 LSHIFT r1;
      // SBR_bs_num_env[ch] = min(r0, 5)
      r1 = r10 - 5;
      if GT r10 = r10 - r1;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_num_env + r0] = r10;
      r8 = r10;

      // read 1 bit
      call $aacdec.get1bit;

      r0 = r5 * 6 (int);
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r0;
      I1 = r2 + $aacdec.SBR_bs_freq_res;

      // for 1:SBR_bs_num_env[ch]

      do fixfix_bs_freq_res_loop;
         M[I1, MK1] = r1;
      fixfix_bs_freq_res_loop:

      // SBR_abs_bord_lead[ch] = 0
      r2 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r2 = r2 + r5;
      M[r2 + $aacdec.SBR_abs_bord_lead] = Null;
      // SBR_abs_bord_trail[ch] = SBR_numTimeSlots
      r1 = $aacdec.SBR_numTimeSlots;
      M[r2 + $aacdec.SBR_abs_bord_trail] = r1;

      jump end_switch_bs_frame_class;
   not_fixfix:
   Null = r1 - $aacdec.SBR_FIXVAR;
   if NZ jump not_fixvar;
      // case FIXVAR

      // SBR_abs_bord_trail[ch] = SBR_bs_var_bord_1[ch] ...
      //                          + SBR_numTimeSlots
      // read 2 bits
      call $aacdec.get2bits;
      r1 = r1 + $aacdec.SBR_numTimeSlots;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_abs_bord_trail] = r1;

      // SBR_bs_num_env[ch] = SBR_bs_num_rel_1[ch] + 1
      // read 2 bits
      call $aacdec.get2bits;
      r8 = r1 + 1;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_num_env + r0] = r8;

      r4 = r5 * 5 (int);
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r4;
      I1 = r0 + $aacdec.SBR_bs_rel_bord;
      I4 = r0 + $aacdec.SBR_bs_rel_bord_1;

      // SBR_bs_num_env[ch] - 1
      r10 = r1;

      do fixvar_rel_bord_loop;
         // read 2 bits
         call $aacdec.get2bits;
         r1 = r1 LSHIFT 1;
         r1 = r1 + 2;
         M[I1, MK1] = r1,
          M[I4, MK1] = r1;
      fixvar_rel_bord_loop:

      Words2Addr(r8);
      // ptr_bits = SBR_log2Table[SBR_bs_num_env[ch]]
      r1 = M[r9 + $aac.mem.SBR_log2Table_ptr];
      r1 = r1 + (1*ADDR_PER_WORD);
      r0 = M[r1 + r8];
      // SBR_bs_pointer[ch] = getbits(ptr_bits)
      call $aacdec.getbits;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_pointer + r0] = r1;

      r0 = r5 * 6 (int);
      r1 = M[r9 + $aac.mem.SBR_info_ptr];
      r1 = r1 + r0;
      r0 = r1 + $aacdec.SBR_bs_freq_res;

      I1 = r0 + r8;
      Addr2Words(r8);
      r10 = r8;
      I1 = I1 - MK1;

      do fixvar_bs_freq_res_loop;
         call $aacdec.get1bit;
         M[I1, -MK1] = r1;
      fixvar_bs_freq_res_loop:

      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_abs_bord_lead] = Null;

      jump end_switch_bs_frame_class;
   not_fixvar:
   Null = r1 - $aacdec.SBR_VARFIX;
   if NZ jump not_varfix;
      // case VARFIX

      // SBR_abs_bord_lead[ch] = getbits(2)
      // read 2 bits
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_abs_bord_lead] = r1;

      // SBR_bs_num_env[ch] = SBR_bs_num_rel_0[ch] + 1
      // read 2 bits
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_bs_num_rel_0] = r1;
      r8 = r1 + 1;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_num_env + r0] = r8;

      r2 = r5 * 5 (int);
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r2;
      I1 = r0 + $aacdec.SBR_bs_rel_bord_0;
      I4 = r0 + $aacdec.SBR_bs_rel_bord;

      // SBR_bs_num_env[ch] - 1
      r10 = r8 - 1;

      do varfix_bs_rel_bord_loop;
         // read 2 bits
         call $aacdec.get2bits;
         r1 = r1 LSHIFT 1;
         r1 = r1 + 2;
         M[I1, MK1] = r1,
          M[I4, MK1] = r1;
      varfix_bs_rel_bord_loop:

      // ptr_bits = SBR_log2Table[SBR_bs_num_env[ch]+1]
      Words2Addr(r8);
      r1 = M[r9 + $aac.mem.SBR_log2Table_ptr];
      r1 = r1 + (1*ADDR_PER_WORD);
      r0 = M[r1 + r8];
      Addr2Words(r8);
      // SBR_bs_pointer[ch] = getbits(ptr_bits)
      call $aacdec.getbits;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_pointer + r0] = r1;

      r10 = r8;
      r0 = r5 * 6 (int);
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r0;
      I1 = r2 + $aacdec.SBR_bs_freq_res;

      do varfix_bs_freq_res_loop;
         call $aacdec.get1bit;
         M[I1, MK1] = r1;
      varfix_bs_freq_res_loop:

      r1 = $aacdec.SBR_numTimeSlots;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_abs_bord_trail] = r1;

      jump end_switch_bs_frame_class;
   not_varfix:
   Null = r1 - $aacdec.SBR_VARVAR;
   if NZ jump end_switch_bs_frame_class;
      // case VARVAR

      // SBR_abs_bord_lead[ch] = getbits(2)
      // read 2 bits
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_abs_bord_lead] = r1;

      // SBR_abs_bord_trail[ch] = getbits(2) + SBR_numTimeSlots
      // read 2 bits
      call $aacdec.get2bits;
      r1 = r1 + $aacdec.SBR_numTimeSlots;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_abs_bord_trail] = r1;

      // SBR_bs_num_rel_0[ch]
      // read 2 bits
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_bs_num_rel_0] = r1;
      r4 = r1;
      // SBR_bs_num_rel_1[ch]
      // read 2 bits
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r5;
      M[r0 + $aacdec.SBR_bs_num_rel_1] = r1;
      r6 = r4 + r1;

      // SBR_bs_num_env[ch] = SBR_bs_num_rel_0[ch] ...
      //                       + SBR_bs_num_rel_1[ch] + 1
      r8 = r6 + 1;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_num_env + r0] = r8;

      r2 = r5 * 5 (int);
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r2;
      I1 = r0 + $aacdec.SBR_bs_rel_bord_0;

      // SBR_bs_num_rel_0[ch]
      r10 = r4;

      do varvar_bs_rel_bord_0_loop;
         // read 2 bits
         call $aacdec.get2bits;
         r1 = r1 LSHIFT 1;
         r1 = r1 + 2;
         M[I1, MK1] = r1;
      varvar_bs_rel_bord_0_loop:

      // SBR_bs_num_rel_1[ch]
      r10 = r6 - r4;
      r1 = r5 * 5 (int);
      r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
      r0 = r0 + r1;
      I1 = r0 + $aacdec.SBR_bs_rel_bord_1;

      do varvar_bs_rel_bord_1_loop;
         // read 2 bits
         call $aacdec.get2bits;
         r1 = r1 LSHIFT 1;
         r1 = r1 + 2;
         M[I1, MK1] = r1;
      varvar_bs_rel_bord_1_loop:

      // ptr_bits = SBR_log2Table[SBR_bs_num_env[ch]+1]
      Words2Addr(r8);
      r1 = M[r9 + $aac.mem.SBR_log2Table_ptr];
      r1 = r1 + (1*ADDR_PER_WORD);
      r0 = M[r1 + r8];
      // SBR_bs_pointer[ch] = getbits(ptr_bits)
      call $aacdec.getbits;
      r0 = r5 + r9;
      M[$aac.mem.SBR_bs_pointer + r0] = r1;

      Addr2Words(r8);
      r10 = r8;
      r0 = r5 * 6 (int);
      r2 = M[r9 + $aac.mem.SBR_info_ptr];
      r2 = r2 + r0;
      I1 = r2 + $aacdec.SBR_bs_freq_res;

      do varvar_bs_freq_res_loop;
         // read 1 bit
         call $aacdec.get1bit;
         M[I1, MK1] = r1;
      varvar_bs_freq_res_loop:

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

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
