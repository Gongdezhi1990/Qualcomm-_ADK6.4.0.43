// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCDEC_SILENCE_DECODER_INCLUDED
#define SBCDEC_SILENCE_DECODER_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcdec.silence_decoder
//
// DESCRIPTION:
//    Silence the decoder - clears any buffers so that no pops and squeeks upon
//    re-enabling output audio
//
// INPUTS:
//    r9 - data object pointer
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, DoLoop, I1
//
// *****************************************************************************
.MODULE $M.sbcdec.silence_decoder;
   .CODESEGMENT SBCDEC_SILENCE_DECODER_PM;
   .DATASEGMENT DM;

   $sbcdec.silence_decoder:
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbc.SILENCE_DECODER_ASM.SBCDEC.SILENCE_DECODER.PATCH_ID_0, r10)
#endif
   

   // initialise V ptrs to the start of V
   r0 = M[r9 + $sbc.mem.SYNTHESIS_VCH1_FIELD];
   M[r9 + $sbc.mem.SYNTHESIS_VCH1PTR_FIELD] = r0;


   // initialise vector V ch 1 to zero

   r0 = M[r9 + $sbc.mem.SYNTHESIS_VCH1_FIELD];
   I1 = r0;

   r10 = $sbcdec.SYNTHESIS_BUFFER_LENGTH;
   r0 = 0;
   do ssf_init_loop1;
      M[I1, MK1] = r0;
   ssf_init_loop1:

#ifndef SBC_WBS_ONLY
   r0 = M[r9 + $sbc.mem.SYNTHESIS_VCH2_FIELD];
   // When were doing mono eg. WBS we don't bother allocating a buffer so check it isn't NULL
   if Z jump ssf_init_loop2;
   M[r9 + $sbc.mem.SYNTHESIS_VCH2PTR_FIELD] = r0;

   // initialise vector V ch 2 to zero

   r0 = M[r9 + $sbc.mem.SYNTHESIS_VCH2_FIELD];
   I1 = r0;

   r10 = $sbcdec.SYNTHESIS_BUFFER_LENGTH;
   r0 = 0;
   do ssf_init_loop2;
      M[I1, MK1] = r0;
   ssf_init_loop2:
#endif
   rts;

.ENDMODULE;

#endif
