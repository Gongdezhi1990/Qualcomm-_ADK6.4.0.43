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
//    $aacdec.sbr_dtdf
//
// DESCRIPTION:
//    Get envelope and noise data delta coding information
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
.MODULE $M.aacdec.sbr_dtdf;
   .CODESEGMENT AACDEC_SBR_DTDF_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_dtdf:

   // push rLink onto stack
   push rLink;

   Words2Addr(r5);
   r10 = r5 + r9;
   r10 = M[$aac.mem.SBR_bs_num_env + r10];
   r0 = r5 * 5 (int);
   r1 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r1 = r1 + r0;
   I1 = r1 + $aacdec.SBR_bs_df_env;

   do df_env_loop;
      call $aacdec.get1bit;
      M[I1, MK1] = r1;
   df_env_loop:

   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   r0 = r0 + r5;
   r10 = M[r0 + $aacdec.SBR_bs_num_noise];

   r0 = r0 + r5;
   I1 = r0 + $aacdec.SBR_bs_df_noise;


   do df_noise_loop;
      call $aacdec.get1bit;
      M[I1, MK1] = r1;
   df_noise_loop:

   Addr2Words(r5);

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
