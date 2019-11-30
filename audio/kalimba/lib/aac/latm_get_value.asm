// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.latm_get_value
//
// DESCRIPTION:
//    Read an latm variable length value
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - r4 = output value read
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0-r3, r5
//
// *****************************************************************************
.MODULE $M.aacdec.latm_get_value;
   .CODESEGMENT AACDEC_LATM_GET_VALUE_PM;
   .DATASEGMENT DM;

   $aacdec.latm_get_value:

   // push rLink onto stack
   push rLink;


   // bytes_for_value = getbits(2);
   // value = 0;
   // for i=0:bytes_for_value,
   //   value = value*256;
   //   value = value + getbits(8);
   // end

   call $aacdec.get2bits;
   r5 = r1;

   r4 = 0;
   byte_loop:

      r4 = r4 LSHIFT 8;

      call $aacdec.get1byte;

      r4 = r4 + r1;

      r5 = r5 - 1;
   if POS jump byte_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

