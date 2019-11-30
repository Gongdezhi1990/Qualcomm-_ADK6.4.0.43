// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCENC_WRITE_PADDING_BITS_INCLUDED
#define SBCENC_WRITE_PADDING_BITS_INCLUDED

#include "stack.h"

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcenc.write_padding_bits
//
// DESCRIPTION:
//    Write padding bits
//
// INPUTS:
//    - I0 = buffer pointer to write words to
//
// OUTPUTS:
//    - I0 = buffer pointer to write words to (updated)
//
// TRASHED REGISTERS:
//    r0-r4
//
// NOTES:
//    Write padding bits to frame. If the VM application has requested even
// length packets, i.e. 16 bit word aligned, this function will add sufficient
// bits to achieve this.
//
//     @verbatim
//     padding()
//     (
//        while ((bit_count mod wordLen)!=0)               Where wordLen=8 or 16
//        (
//        $padding_bit                                     1 UiMsbf
//        bit_count++
//        )
//     )
//     @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcenc.write_padding_bits;
   .CODESEGMENT SBCENC_WRITE_PADDING_BITS_PM;
   .DATASEGMENT DM;

   $sbcenc.write_padding_bits:

   // push rLink onto stack
   push rLink;

   // get aligned
   r0 = M[r9 + $sbc.mem.PUT_BITPOS_FIELD];
   // do we want to be byte or word aligned
   r3 = M[r9 + $sbc.mem.ENC_SETTING_FORCE_WORD_ALIGN_FIELD];

   r3 = r3 LSHIFT 3;
   r3 = r3 OR 7;
   // add zero padding bits
   r1 = 0;
   r0 = r0 AND r3;
   if NZ call $sbcenc.putbits;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
#endif
