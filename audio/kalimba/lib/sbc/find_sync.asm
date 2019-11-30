// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************
#ifndef SBC_WBS_ONLY

#include "sbc.h"

// *****************************************************************************

// output:  r6- get_bitpos of the location before the syncword or 99 indicating
//              sync word not found
//          I2- I0 of the location before the sync word or not used

// *****************************************************************************

.MODULE $M.sbcdec.find_sync;
   .CODESEGMENT SBCDEC_FIND_SYNC_PM;
   .DATASEGMENT DM;

   $sbcdec.find_sync:

   // push rLink onto stack
   push rLink;


   // get byte aligned - we should be anyway but for robustness we force it
   call $sbcdec.byte_align;

   // r10 as input
   DO findsyncloop;
      // backup the bitstream buffer pointer before reading next byte

      r6 = M[r9 + $sbc.mem.GET_BITPOS_FIELD];

      I2 = I0;

      call $sbcdec.get8bits;
      Null = r1 - $sbc.SYNC_WORD;
      if Z jump found_sync;
   findsyncloop:

   // all available data are searched and sync word not found
   r6 = $sbc.SBC_NOT_SYNC;
   jump $pop_rLink_and_rts;



   found_sync:

   // restore bitstream buffer pointer to just before the sync word
   I0 = I2;

   M[r9 + $sbc.mem.GET_BITPOS_FIELD] = r6;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif
