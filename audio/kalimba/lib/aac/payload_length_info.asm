// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.payload_length_info
//
// DESCRIPTION:
//    Read the payload length info for an latm packet
//
// INPUTS:
//    - I0 = buffer pointer to read words from
//
// OUTPUTS:
//    - r4 = output value read
//    - I0 = buffer pointer to read words from (updated)
//
// TRASHED REGISTERS:
//    - r0-r3
//
// *****************************************************************************
.MODULE $M.aacdec.payload_length_info;
   .CODESEGMENT AACDEC_PAYLOAD_LENGTH_INFO_PM;
   .DATASEGMENT DM;

   $aacdec.payload_length_info:

   // push rLink onto stack
   push rLink;

   // Note: 'allStreamsSameTimeFraming' must be set
   //       'numProgram' and 'numLayer' must be 0
   // These checks are already assumed to have been done while parsing the
   // 'stream_mux_config' block

   //    latm.MuxSlotLengthBytes = 0;
   //    tmp = 255;
   //    while (tmp==255)
   //       tmp = getbits(8);
   //       latm.MuxSlotLengthBytes = latm.MuxSlotLengthBytes + tmp;
   //    end

   r4 = 0;
   byte_loop:

      call $aacdec.get1byte;
      r4 = r4 + r1;

      Null = r1 - 255;
   if Z jump byte_loop;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

