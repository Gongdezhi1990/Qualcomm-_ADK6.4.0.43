// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY
#ifndef SBCDEC_READ_PADDING_BITS_INCLUDED
#define SBCDEC_READ_PADDING_BITS_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbcdec.read_padding_bits
//
// DESCRIPTION:
//    Read padding bits
//
// INPUTS:
//    - I0 = buffer to read words from
//
// OUTPUTS:
//    - I0 = buffer to read words from (updated)
//
// TRASHED REGISTERS:
//    r0-r3
//
// NOTES:
//  Remove padding bits from frame:
//
//     @verbatim
//     padding()
//     (
//        while ((bit_count mod 8)!=0)
//        (
//        $padding_bit                                         1 UiMsbf
//        bit_count++
//        )
//     )
//     @endverbatim
//
// *****************************************************************************
.MODULE $M.sbcdec.read_padding_bits;
   .CODESEGMENT SBCDEC_READ_PADDING_BITS_PM;
   .DATASEGMENT DM;

   $sbcdec.read_padding_bits:
   $sbcdec.byte_align:

   // push rLink onto stack
   push rLink;


   // get byte aligned

   r0 = M[r9 + $sbc.mem.GET_BITPOS_FIELD];

   r0 = r0 AND 7;
   call $sbcdec.getbits;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
#endif
