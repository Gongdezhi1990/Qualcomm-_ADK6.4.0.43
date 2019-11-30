// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.discard_dse
//
// DESCRIPTION:
//    Discard the data stream element
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-r4, r10
//
// *****************************************************************************
.MODULE $M.aacdec.discard_dse;
   .CODESEGMENT AACDEC_DISCARD_DSE_PM;
   .DATASEGMENT DM;

   $aacdec.discard_dse:

   // push rLink onto stack
   push rLink;

   // element_instance_tag = getbits(4);
   // dummy read this as not needed
   call $aacdec.get4bits;

   // data_byte_align_flag = getbits(1);
   call $aacdec.get1bit;
   r4 = r1;

   // cnt = getbits(8);
   call $aacdec.get1byte;
   r10 = r1;

   // if (cnt == 255)
   // {
   //    esc_count = getbits(8);
   //    cnt += esc_count;
   // }
   Null = r10 - 255;
   if NZ jump cnt_not_255;
      call $aacdec.get1byte;
      r10 = r10 + r1;
   cnt_not_255:

   // if (data_byte_align_flag)
   //    byte_alignment();
   Null = r4;
   if NZ call $aacdec.byte_align;

   // for i=0:cnt-1
   //  data_stream_byte = getbits(8);
   // dummy read all of these as not needed
   do data_stream_byte_loop;
      call $aacdec.get1byte;
   data_stream_byte_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
