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
//    $aacdec.sbr_header
//
// DESCRIPTION:
//    read in sbr header data fields
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_header;
   .CODESEGMENT AACDEC_SBR_HEADER_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_header:

   // push rLink onto stack
   push rLink;

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r1 = M[r0 + $aacdec.SBR_header_count];
   r1 = r1 + 1;
   M[r0 + $aacdec.SBR_header_count] = r1;

   call $aacdec.get1bit;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r0 + $aacdec.SBR_bs_amp_res] = r1;

   call $aacdec.get4bits;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r0 + $aacdec.SBR_bs_start_freq] = r1;

   call $aacdec.get4bits;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r0 + $aacdec.SBR_bs_stop_freq] = r1;

   call $aacdec.get3bits;
   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   M[r0 + $aacdec.SBR_bs_xover_band] = r1;

   // discard 2 bits
   call $aacdec.get2bits;

   call $aacdec.get1bit;
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r0 + $aacdec.SBR_bs_header_extra_1] = r1;

   call $aacdec.get1bit;
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   M[r0 + $aacdec.SBR_bs_header_extra_2] = r1;

   // if(SBR_bs_header_extra_1)
   Null = M[r0 + $aacdec.SBR_bs_header_extra_1];
   if Z jump not_bs_header_extra_1;
      // read bs_freq_scale
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_freq_scale] = r1;
      // read bs_alter_scale
      call $aacdec.get1bit;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_alter_scale] = r1;
      // read bs_noise_bands
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_noise_bands] = r1;
   not_bs_header_extra_1:


   // if(SBR_bs_header_extra_2)
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   Null = M[r0 + $aacdec.SBR_bs_header_extra_2];
   if Z jump not_bs_header_extra_2;
      // read bs_limiter_bands
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_limiter_bands] = r1;
      // read bs_limiter_gains
      call $aacdec.get2bits;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_limiter_gains] = r1;
      // read bs_interpol_freq
      call $aacdec.get1bit;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_interpol_freq] = r1;
      // read bs_smoothing_mode
      call $aacdec.get1bit;
      r0 = M[r9 + $aac.mem.SBR_info_ptr];
      M[r0 + $aacdec.SBR_bs_smoothing_mode] = r1;
   not_bs_header_extra_2:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
