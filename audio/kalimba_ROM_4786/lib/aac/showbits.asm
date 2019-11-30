// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.showbits -
//
// DESCRIPTION:
//    Show bits from input buffer
//
// INPUTS:
//    - r0 = number of bits to show from buffer
//    - I0 = buffer to show words from
//
// OUTPUTS:
//    - r1 = the data shown from the buffer
//
// TRASHED REGISTERS:
//    r0, r2, r3
//
// *****************************************************************************
.MODULE $M.aacdec.showbits;
   .CODESEGMENT AACDEC_SHOWBITS_PM;
   .DATASEGMENT DM;

   $aacdec.showbits:
   r3 = M[r9 + $aac.mem.BITMASK_LOOKUP_FIELD];
   Words2Addr(r0);
   r3 = M[r3 + r0];  // form a bit mask (r3)
   Addr2Words(r0);
   r1 = M[r9 + $aac.mem.GET_BITPOS];
   r2 = r0 - r1;      // r2 = shift amount
   if GT jump anotherword;               // is another word from buffer needed?

      r1 = M[I0,0];                         // get current word from buffer (r1)
      r1 = r1 LSHIFT r2;                    // shift data to right
      r1 = r1 AND r3;                       // do the bit masking
      rts;

   anotherword:

      r1 = M[I0,MK1];                       // get current word from buffer (r1)
      r1 = r1 LSHIFT r2;                    // shift current word
      r1 = r1 AND r3,                       // mask out unwanted bits
       r3 = M[I0,-MK1];                     // get another word from buffer (r3)
      r2 = r2 - 16;                         // calc new shift amount
      r3 = r3 LSHIFT r2;                    // and shift
      r1 = r1 + r3;                         // combine the 2 parts

      rts;

.ENDMODULE;
