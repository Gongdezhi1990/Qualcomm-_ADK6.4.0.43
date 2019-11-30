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
//    $aacdec.sbr_swap_channels
//
// DESCRIPTION:
//    Swap the left and right channels in the persistent part of X_sbr
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - I0, I1, I4, I5, r0-r3, r10
//
// *****************************************************************************
.MODULE $M.aacdec.sbr_swap_channels;
   .CODESEGMENT AACDEC_SBR_SWAP_CHANNELS_PM;
   .DATASEGMENT DM;

   $aacdec.sbr_swap_channels:

   // push rLink onto stack
   push rLink;

#ifdef AACDEC_ELD_ADDITIONS
   r0 = M[r9 + $aac.mem.AUDIO_OBJECT_TYPE_FIELD];
   Null = r0 - $aacdec.ER_AAC_ELD;
   if Z jump eld_version;
#endif


      // *** NON ELD MODE ***
      // ********************
      r10 = $aacdec.X_SBR_WIDTH/2;

      r0 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I0 = r0;
      r0 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r0;
      I1 = I0 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I5 = I4 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      call swap;

      r10 = $aacdec.X_SBR_WIDTH/2;
      I0 = I0 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I1 = I1 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I4 = I4 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I5 = I5 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      call swap;

      r10 = $aacdec.X_SBR_LEFTRIGHT_SIZE;
      I0 = I0 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      I4 = I4 + ($aacdec.X_SBR_WIDTH*ADDR_PER_WORD/2);
      r0 = M[r9 + $aac.mem.SBR_X_sbr_other_real_ptr];
      I1 = r0;
      r0 = M[r9 + $aac.mem.SBR_X_sbr_other_imag_ptr];
      I5 = r0;
      call swap;
      // pop rLink from stack
      jump $pop_rLink_and_rts;



#ifdef AACDEC_ELD_ADDITIONS
   eld_version:
      // *** ELD MODE ***
      // ****************
      r10 = $aacdec.X_SBR_WIDTH*2;
      r0 = M[r9 + $aac.mem.SBR_X_2env_real_ptr];
      I0 = r0;
      r0 = M[r9 + $aac.mem.SBR_X_2env_imag_ptr];
      I4 = r0;
      r0 = M[r9 + $aac.mem.SBR_X_sbr_other_real_ptr];
      I1 = r0;
      r0 = M[r9 + $aac.mem.SBR_X_sbr_other_imag_ptr];
      I5 = r0;
      call swap;
      // pop rLink from stack
      jump $pop_rLink_and_rts;

 #endif //AACDEC_ELD_ADDITIONS


   swap:
      do copy_loop;
         r0 = M[I0, 0],
          r2 = M[I4, 0];
         r1 = M[I1, 0],
          r3 = M[I5, 0];
         M[I0, MK1] = r1,
          M[I4, MK1] = r3;
         M[I1, MK1] = r0,
          M[I5, MK1] = r2;
      copy_loop:
      rts;

.ENDMODULE;

#endif
