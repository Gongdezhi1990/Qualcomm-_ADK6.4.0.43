// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBCENC_WRITE_SCALE_FACTORS_INCLUDED
#define SBCENC_WRITE_SCALE_FACTORS_INCLUDED

#include "stack.h"

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.write_scale_factors
//
// DESCRIPTION:
//    Write Scalefactors
//
// INPUTS:
//    - I0 = buffer pointer to write words to
//
// OUTPUTS:
//    - I0 = buffer pointer to write words to (updated)
//
// TRASHED REGISTERS:
//    r0-r5, r10, DoLoop, I1
//
// NOTES:
//  Write scalefactors
//
//     @verbatim
//     scale_factors()
//     (
//        for (ch=0;ch<nrof_channels;ch++)
//        (
//           for (sb=0;sb<nrof_subbands;sb++)
//           (
//           $scale_factor[ch][sb]                             4 UiMsbf
//           )
//        )
//     )
//     @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.write_scale_factors;
   .CODESEGMENT SBCENC_WRITE_SCALE_FACTORS_PM;
   .DATASEGMENT DM;

   $sbcenc.write_scale_factors:

   // push rLink onto stack
   push rLink;

#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($sbcenc.WRITE_SCALE_FACTORS_ASM.WRITE_SCALE_FACTORS.PATCH_ID_0, r4)
#endif

   r4 = M[r9 + $sbc.mem.NROF_CHANNELS_FIELD];
   r5 = M[r9 + $sbc.mem.NROF_SUBBANDS_FIELD];
   r0 = M[r9 + $sbc.mem.SCALE_FACTOR_FIELD];
   I1 = r0;
   r10 = r4 * r5 (int);
   r0 = 4;

   do loop;
      r1 = M[I1,MK1];
      call $sbcenc.putbits;         // write scalefactor
   loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
