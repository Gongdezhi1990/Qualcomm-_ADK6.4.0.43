/****************************************************************************
 * Copyright (c) 2014 - 2017 Qualcomm Technologies International, Ltd.
****************************************************************************/
#include "stack.h"

#include "codec/codec_library.h"


#ifdef PATCH_LIBS
#include "patch_library.h"
#endif

// *****************************************************************************
// MODULE:
//    $_encoder_encode
//
// DESCRIPTION:
//    Common encoder C wrapper
//
// INPUTS:
//    - r0 = pointer to the codec's Encoder structure
//    - r1 = pointer to the entry function ( encode function or strip encode )
//
// OUTPUTS:
//    - r0 = Boolean indicating whether encoded output was produced.
//
// TRASHED REGISTERS:
//    C calling convention respected.
//
// NOTES:
//
// *****************************************************************************
.MODULE $M.encoder_encode;
   .CODESEGMENT PM_FLASH;

$_encoder_encode:
   // Preserve rLink and the registers C doesn't view as Scratch
   PUSH_ALL_C

   // The codec libraries expect the Encoder structure in r5
   r5 = r0;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($encode_cap.ENCODER_CAP_ASM.ENCODER_ENCODE.ENCODER_ENCODE.PATCH_ID_0, r4)   
#endif   
   
   
   
   r0 = 1;
   // Call the encoder repeatedly until it fills the output/runs out of input
   encode_loop:
   pushm <r0, r1>;

   // Call the codec library to do an encode
   call r1;

   popm <r0, r1>;

   r4 = M[r5 + $codec.ENCODER_MODE_FIELD];
   Null = r4 - $codec.SUCCESS;
   if EQ r0 = r4;
   if EQ jump encode_loop;

   // invert the value in r0 so that 1 represents decoded something.
   r0 = r0 XOR 1;
   // Restore the original state
   POP_ALL_C
   rts;

.ENDMODULE;
