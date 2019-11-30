// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCDEC_RESET_DECODER_INCLUDED
#define SBCDEC_RESET_DECODER_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $_sbcdec_reset_decode
//
// DESCRIPTION:
//    C callable version of the $sbcdec.reset_decoder function.
//    Reset variables for sbc decoding_c callable
//
// INPUTS:
//    - r0 = pointer to decoder data structure
//
// OUTPUTS:
//    - none
//
// *****************************************************************************
.MODULE $M.sbcdec._reset_decoder;
   .CODESEGMENT SBCDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

$_sbcdec_reset_decoder:
   pushm <r9, rLink>;

   r9 = r0;

   call $sbcdec.reset_decoder;

   popm <r9, rLink>;
   rts;
.ENDMODULE;


// *****************************************************************************
// MODULE:
//    $sbcdec.reset_decoder
//
// DESCRIPTION:
//    Reset variables for sbc decoding
//
// INPUTS:
//    - r9(R5 for legacy code) = pointer to decoder data structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, (r9 for legacy code),r10, DoLoop, I1
//
// *****************************************************************************
.MODULE $M.sbcdec.reset_decoder;
   .CODESEGMENT SBCDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

   $sbcdec.reset_decoder:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcdec.RESET_DECODER_ASM.RESET_DECODER.PATCH_ID_0, r10)
#endif
   r0 = 16;

#ifndef KYMERA
   // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.DECODER_DATA_OBJECT_FIELD];
#endif

   M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r0;


   // clear the filter buffers
   call $sbcdec.silence_decoder;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_sbc_decode_reset
//
// DESCRIPTION:
//    C - callable version of $sbcdec.reset_decoder.
//
// INPUTS:
//    - r0 = pointer to decoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, DoLoop, I1?
//
// *****************************************************************************
.MODULE $M.sbcdec.reset_decoder;
   .CODESEGMENT SBCDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

   $_sbc_decode_lib_reset:
   // Save the registers C expects us to preserve that get trashed
   pushm <r5, r9, rLink>;
   push I0;

   r9 = r0;

   call $sbcdec.reset_decoder;

   // restore preserved registers
   pop I0;
   popm <r5, r9, rLink>;

   rts;

.ENDMODULE;

#endif

