// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#ifndef SBC_CRC_CALC_INCLUDED
#define SBC_CRC_CALC_INCLUDED

#include "sbc.h"

// *****************************************************************************
// MODULE:
//    $sbc.crc_calc
//
// DESCRIPTION:
//    CRC Check, Updates $sbc.crc_checksum with the current checksum
//
// INPUTS:
//     - r0 = number of bits of data
//     - r1 = the data
//
// OUTPUTS:
//     - none
//
// TRASHED REGISTERS:
//    r2-r5, r10, DoLoop
//
// *****************************************************************************
.MODULE $M.sbc.crc_calc;
   .CODESEGMENT SBC_CRC_CALC_PM;
   .DATASEGMENT DM;

   $sbc.crc_calc:

#if defined(PATCH_LIBS)
   LIBS_PUSH_R0_SLOW_SW_ROM_PATCH_POINT($sbc.CRC_CALC_ASM.CRC_CALC.PATCH_ID_0, r2)
#endif

   // get current crc_checksum

   r2 = M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD];

   // preset the generator polynomial
   r5 = $sbc.CRC_GENPOLY;

   // shift data_in
   r3 = 8 - r0;
   // set up the do loop
   r10 = r0;

   // shift so MSB is in bit position 7
   r3 = r1 LSHIFT r3;

   do crc_loop;
      r4 = r3 XOR r2;
      // r2 = crc << 1
      r2 = r2 LSHIFT 1;
      // temp = XOR databit in with crc[7]
      r4 = r4 AND 0x80;
      // if (temp) crc = (crc << 1) XOR gen_poly
      // else  crc = (crc << 1)
      if NZ r2 = r2 XOR r5;


      // shift data_in ready for extracting MSB
      r3 = r3 LSHIFT 1;
   crc_loop:

   // save updated crc_checksum

   M[r9 + $sbc.mem.CRC_CHECKSUM_FIELD] = r2;

   rts;

.ENDMODULE;

#endif
