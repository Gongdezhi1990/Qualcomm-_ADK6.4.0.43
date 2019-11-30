// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// *****************************************************************************

#ifndef SBCENC_RESET_ENCODER_INCLUDED
#define SBCENC_RESET_ENCODER_INCLUDED

#include "sbc.h"


// *****************************************************************************
// MODULE:
//    $sbcenc.reset_encoder
//
// DESCRIPTION:
//    Reset variables for sbc encoding
//
// INPUTS:
//    - r9(R5 for legacy code) = pointer to encoder data structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, DoLoop, I0
//
// *****************************************************************************
.MODULE $M.sbcenc.reset_encoder;
   .CODESEGMENT SBCENC_RESET_ENCODER_PM;
   .DATASEGMENT DM;

   $sbcenc.reset_encoder:

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcenc.RESET_ENCODER_ASM.RESET_ENCODER.PATCH_ID_0, r10)
#endif

#ifndef KYMERA
   // -- Load memory structure pointer
   // This pointer should have been initialised externally
   r9 = M[r5 + $codec.ENCODER_DATA_OBJECT_FIELD];
#endif

   // init X ptrs to the start of X
   r0 = M[r9 + $sbc.mem.ANALYSIS_XCH1_FIELD];
   M[r9 + $sbc.mem.ANALYSIS_XCH1PTR_FIELD] = r0;

   // init vector X ch 1 to zero
   I0 = r0;

   r10 = $sbcenc.ANALYSIS_BUFFER_LENGTH;
   r0 = 0;
   do asf_init_loop1;
      M[I0, MK1] = r0;
   asf_init_loop1:

#ifndef SBC_WBS_ONLY
   // init vector X ch 2 to zero
   r0 = M[r9 + $sbc.mem.ANALYSIS_XCH2_FIELD];
   // When were doing mono eg. WBS we don't bother allocating a buffer so check it isn't NULL
   if Z jump asf_init_loop2;
   M[r9 + $sbc.mem.ANALYSIS_XCH2PTR_FIELD] = r0;

   I0 = r0;
   r10 = $sbcenc.ANALYSIS_BUFFER_LENGTH;
   r0 = 0;
   do asf_init_loop2;
      M[I0, MK1] = r0;
   asf_init_loop2:
#endif
   rts;

.ENDMODULE;

// *****************************************************************************
// MODULE:
//    $_sbc_encode_reset
//
// DESCRIPTION:
//    C - callable version of $sbcenc.reset_encoder.
//
// INPUTS:
//    - r0 = pointer to encoder structure
//      allocated
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, DoLoop, I1?
//
// *****************************************************************************
.MODULE $M.sbcenc.reset_encoder;
   .CODESEGMENT SBCDEC_RESET_DECODER_PM;
   .DATASEGMENT DM;

   $_sbc_encode_lib_reset:
   // Save the registers C expects us to preserve that get trashed
   pushm <r5, r9, rLink>;
   push I0;

   r9 = r0;

   call $sbcenc.reset_encoder;

   // restore preserved registers
   pop I0;
   popm <r5, r9, rLink>;

   rts;

.ENDMODULE;

#endif
