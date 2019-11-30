// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.getbits
//
// DESCRIPTION:
//    Get bits from input buffer
//
// INPUTS:
//    - r0 = number of bits to get from buffer
//    - I0 = buffer to read words from
//
// OUTPUTS:
//    - r0 = unaffected
//    - r1 = the data read from the buffer
//
// TRASHED REGISTERS:
//    r2, r3
//    r0 if getXbits or getXbytes called
//
// *****************************************************************************
.MODULE $M.aacdec.getbits;
   .CODESEGMENT AACDEC_GETBITS_PM;
   .DATASEGMENT DM;

   // save PM memory by having dedicated functions for particular bit/byte sizes
   $aacdec.get2bytes:
   r0 = 16;
   jump $aacdec.getbits;

   $aacdec.get1byte:
   r0 = 8;
   jump $aacdec.getbits;

   $aacdec.get6bits:
   r0 = 6;
   jump $aacdec.getbits;

   $aacdec.get5bits:
   r0 = 5;
   jump $aacdec.getbits;

   $aacdec.get4bits:
   r0 = 4;
   jump $aacdec.getbits;

   $aacdec.get3bits:
   r0 = 3;
   jump $aacdec.getbits;

   $aacdec.get2bits:
   r0 = 2;
   jump $aacdec.getbits;

   $aacdec.get1bit:
   r0 = 1;

   $aacdec.getbits:
   
#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($aacdec.GETBITS_ASM.GETBITS.GETBITS.PATCH_ID_0, r3)
#endif   

   r3 = M[r9 + $aac.mem.READ_BIT_COUNT];
   r3 = r3 + r0;
   M[r9 + $aac.mem.READ_BIT_COUNT] = r3;  // Update number of bits read
   r3 = M[r9 + $aac.mem.BITMASK_LOOKUP_FIELD];
   Words2Addr(r0);
   r3 = M[r3 + r0];  // form a bit mask (r3)
   Addr2Words(r0);
   r1 = M[r9 + $aac.mem.GET_BITPOS];
   r2 = r0 - r1;      // r2 = shift amount
   if GT jump anotherword;               // is another word from buffer needed?

      r1 = M[I0,0];                         // get current word from buffer (r1)
      r1 = r1 LSHIFT r2;                    // shift data to right
      r2 = NULL - r2;
      M[r9 + $aac.mem.GET_BITPOS] = r2;     // update get_bitpos
      r1 = r1 AND r3;                       // do the bit masking
      rts;

   anotherword:

      r1 = M[I0, MK1];                         // get current word from buffer (r1)
      r1 = r1 LSHIFT r2;                    // shift current word
      r1 = r1 AND r3,                       // mask out unwanted bits
       r3 = M[I0,0];                        // get another word from buffer (r3)
      r2 = r2 - 16;                         // calc new shift amount
      r3 = r3 LSHIFT r2;                    // and shift
      r2 = NULL - r2;
      M[r9 + $aac.mem.GET_BITPOS] = r2;//Null - r2;    // update get_bitpos
      r1 = r1 + r3;                         // combine the 2 parts
      rts;

.ENDMODULE;
