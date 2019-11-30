// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef MATH_ADDRESS_BITREVERSE_INCLUDED
#define MATH_ADDRESS_BITREVERSE_INCLUDED

#include "math_library.h"

//******************************************************************************
// MODULE:
//    $math.address_bitreverse
//
// DESCRIPTION:
//    Software version of the BITREVERSE() preprocessor command.
//    Lower 15 bits of input address are bitreversed, bit15 is left the same.
// @verbatim
//    input:   XXXXXXXXY110010001000010
//                     ||             |
//                     || BITREVERSE  |
//                     V|             |
//    output:  00000000Y010000100010011
// @endverbatim
//
// INPUTS:
//    - r0 = address
//
// OUTPUTS:
//    - r1 = bitreversed address
//
// TRASHED REGISTERS:
//
//******************************************************************************
.MODULE $M.math.address_bitreverse;
   .CODESEGMENT MATH_ADDRESS_BITREVERSE_PM;
   .DATASEGMENT DM;

   $math.address_bitreverse:
      // >= -- KAL_ARCH3 --
      // Save address register.  Protect from interupts
      push r2;
      r2 = M[$BITREVERSE_VAL];
      // Compute Bit reversed address
      M[$BITREVERSE_VAL] = r0;
      r1 = M[$BITREVERSE_ADDR];
      // Restore address register
      M[$BITREVERSE_VAL] = r2; 
      pop r2;
      rts;

.ENDMODULE;

#endif // MATH_ADDRESS_BITREVERSE_INCLUDED
