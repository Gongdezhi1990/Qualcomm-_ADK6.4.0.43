// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.silence_decoder
//
// DESCRIPTION:
//    Silence the decoder - clears any buffers so that no pops and squeeks upon
//    re-enabling output audio
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, DoLoop, I0, I4
//
// *****************************************************************************
.MODULE $M.aacdec.silence_decoder;
   .CODESEGMENT AACDEC_SILENCE_DECODER_PM;
   .DATASEGMENT DM;

   $aacdec.silence_decoder:

   // clear overlap add buffers
   r0 = M[r9 +  $aac.mem.OVERLAP_ADD_LEFT_PTR];
   I0 = r0; //&$aacdec.overlap_add_left;
   r0 = M[r9 +  $aac.mem.OVERLAP_ADD_RIGHT_PTR];
   I4 = r0;
   //I4 = &$aacdec.overlap_add_right;
   r10 = $aacdec.OVERLAP_ADD_LENGTH;
   r0 = 0;
   do oa_clear_loop;
      M[I0, MK1] = r0,
       M[I4, MK1] = r0;
   oa_clear_loop:

   rts;

.ENDMODULE;
