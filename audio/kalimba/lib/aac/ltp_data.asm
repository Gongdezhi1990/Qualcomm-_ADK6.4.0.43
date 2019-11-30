// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.ltp_data
//
// DESCRIPTION:
//    Get the LTP (long term prediction) data
//
// INPUTS:
//    - r4 = pointer to ics data
//    - r5 = pointer to ltp data
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all
//
// *****************************************************************************
.MODULE $M.aacdec.ltp_data;
   .CODESEGMENT AACDEC_LTP_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.ltp_data:

   // push rLink onto stack
   push rLink;

   // allocate memory for LTP data
   r0 = $aacdec.ltp.STRUC_SIZE;
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;

   // store ltp_data_ptr
   M[r5] = r1;
   r5 = r1;

   // ltp_lag = getbits(11)
   r0 = 11;
   call $aacdec.getbits;
   M[r5 + $aacdec.ltp.LAG_FIELD] = r1;

   // ltp_coef = getbits(3)
   call $aacdec.get3bits;
   M[r5 + $aacdec.ltp.COEF_FIELD] = r1;

   // check we are not using EIGHT_SHORT_SEQUENCE
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z jump $aacdec.possible_corruption;

   // set r10 = min(max_sfb, LTP_MAX_SFB_LONG)
   r10 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
   r1 = r10 - $aacdec.LTP_MAX_SFB_LONG;
   if POS r10 = r10 - r1;

   // read all the long_used bits and store in the LTP structure
   I1 = r5 + $aacdec.ltp.LONG_USED_FIELD;
   do get_ltp_long_used_loop;
      call $aacdec.get1bit;
      M[I1, MK1] = r1;
   get_ltp_long_used_loop:

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
