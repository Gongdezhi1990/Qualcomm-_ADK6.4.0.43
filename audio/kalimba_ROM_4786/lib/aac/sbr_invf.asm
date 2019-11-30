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
//    $aacdec.sbr_invf
//
// DESCRIPTION:
//    Get inverse filtering information
//
// INPUTS:
//    - r5 channel
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r3, r10, I1
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_invf;
   .CODESEGMENT AACDEC_SBR_INVF_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_invf:

   // push rLink onto stack
   push rLink;

   r0 = r5 * (5*ADDR_PER_WORD) (int);
   r0 = r0 + r9;
   I1 = $aac.mem.SBR_bs_invf_mode + r0;

   r0 = M[r9 + $aac.mem.SBR_info_ptr];
   r10 = M[r0 + $aacdec.SBR_Nq];

   do invf_mode_loop;
      call $aacdec.get2bits;
      M[I1, MK1] = r1;
   invf_mode_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
