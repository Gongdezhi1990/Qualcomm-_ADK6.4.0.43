// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.ltp_reconstruction
//
// DESCRIPTION:
//    Subroutine to add the scale factor bands of X_est defined by ltp_long_used
//    to spec
//
// INPUTS:
//    - I4 = ptr to start of X_est
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all
//
// *****************************************************************************
.MODULE $M.aacdec.ltp_reconstruction;
   .CODESEGMENT AACDEC_LTP_RECONSTRUCTION_PM;
   .DATASEGMENT DM;

   $aacdec.ltp_reconstruction:

   // push rLink onto stack
   push rLink;

   // set r5 to the appropriate ltp pointer (depends on state of common_window)
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   r5 = M[r4 + $aacdec.ics.LTP_INFO_PTR_FIELD];

   // is this a common_window?
   r2 = M[r9 + $aac.mem.COMMON_WINDOW];
   if Z jump not_common_window;
      // if (current_channel == left) ics_data_ptr, else ics_data_ch2_ptr
      r0 = r9 + $aac.mem.ICS_LEFT;
      Null = r4 - r0;//&$aacdec.ics_left;
      if Z jump not_common_window;
         r4 = r9 + $aac.mem.ICS_LEFT;//&$aacdec.ics_left;
         r5 = M[r4 + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD];
   not_common_window:

   I3 = r5 + $aacdec.ltp.LONG_USED_FIELD;   // ltp_long_used

   r2 = M[r9 + $aac.mem.CURRENT_SPEC_PTR];
   I1 = r2;

   r2 = M[r4 + $aacdec.ics.MAX_SFB_FIELD];
   // if max_sfb = 0 we can exit now
   if Z jump $pop_rLink_and_rts;

   M0 = r2;   // max_sfb

   r2 = M[r4 + $aacdec.ics.SWB_OFFSET_PTR_FIELD];
   I2 = r2;

   // M0 = min(ics(ics_ch).max_sfb,LTP_MAX_SFB_LONG)
   r0 = M0 - $aacdec.LTP_MAX_SFB_LONG;
   if POS M0 = M0 - r0;

   r1 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR]; // ravneet to check this
   r8 = M[r1];

   for_all_sfb:

      r5 = M[I3, MK1];
      r2 = M[I2, MK1];   // sfb_start

      // is ltp_long_used(sfb) == 1?
      Null = Null + r5,
       r3 = M[I2, 0];
      if Z jump consider_next_sfb;
         r10 = r3 - r2;
         r3 = r3 - 1;     // sfb_end

         Words2Addr(r2);
         I0 = I4 + r2;
         I5 = I1 + r2;

         do add_xest_loop;
            r5 = M[I5,0],
             r0 = M[I0, MK1];

            r5 = r5 + r0;

            r8 = BLKSIGNDET r5,
             M[I5, MK1] = r5;  // spec(i) = spec(i) + X_est(i)
         add_xest_loop:

      consider_next_sfb:

      // M0 is the outer loop counter
      M0 = M0 - 1;
   if NZ jump for_all_sfb;

   r1 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
   M[r1] = r8;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
