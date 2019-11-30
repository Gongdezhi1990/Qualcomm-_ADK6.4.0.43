// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.ics_info
//
// DESCRIPTION:
//    Get the ICS (individual channel stream) information data
//
// INPUTS:
//    - r4 = current ics structure
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume all
//
// *****************************************************************************
.MODULE $M.aacdec.ics_info;
   .CODESEGMENT AACDEC_ICS_INFO_PM;
   .DATASEGMENT DM;

   $aacdec.ics_info:

   // push rLink onto stack
   push rLink;
   
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.ICS_INFO_ASM.ICS_INFO.ICS_INFO.PATCH_ID_0, r3)
#endif
   

   // clear the ltp fields
   M[r4 + $aacdec.ics.LTP_INFO_PTR_FIELD] = Null;
   M[r4 + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD] = Null;
   M[r4 + $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD] = Null;


   // dummy = getbits(1);  //ics_reserved_bit
   call $aacdec.get1bit;

   // window_sequence = getbits(2);
   call $aacdec.get2bits;
   M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD] = r1;
   r5 = r1;

   // window_shape = getbits(1);
   call $aacdec.get1bit;
   M[r4 + $aacdec.ics.WINDOW_SHAPE_FIELD] = r1;


   // if (window_sequence == EIGHT_SHORT_SEQUENCE)
   Null = r5 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if NZ jump not_eight_short_sequence;

      // max_sfb = = getbits(4);
      call $aacdec.get4bits;
      M[r4 + $aacdec.ics.MAX_SFB_FIELD] = r1;

      // scale_factor_grouping = getbits(7);
      r0 = 7;
      call $aacdec.getbits;
      M[r4 + $aacdec.ics.SCALE_FACTOR_GROUPING_FIELD] = r1;

      // calc_sfb_and_wingroup();
      call $aacdec.calc_sfb_and_wingroup;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump possible_corruption;

      jump window_type_done;

   // else (window_sequence != EIGHT_SHORT_SEQUENCE)
   not_eight_short_sequence:

      // max_sfb = getbits(6);
      call $aacdec.get6bits;
      M[r4 + $aacdec.ics.MAX_SFB_FIELD] = r1;

      // calc_sfbs_and_wingroups();
      call $aacdec.calc_sfb_and_wingroup;
      Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
      if NZ jump possible_corruption;

      // predictor_data_present = getbits(1);
      call $aacdec.get1bit;
      M[r4 + $aacdec.ics.PREDICTOR_DATA_PRESENT_FIELD] = r1;

      // if (predictor_data_present == 1)
      if Z jump no_predictor_data_present;

         // Here we assume it's not object type  1 (ie. its LTP rather than main profile)

         // ltp_data_present = getbits(1);
         call $aacdec.get1bit;
         r5 = r4 + $aacdec.ics.LTP_INFO_PTR_FIELD;
         M[r4 + $aacdec.ics.LTP_INFO_PTR_FIELD] = r1;

         // if (ltp_data_present == 1) aacdec.ltp_data()
         #ifdef AACDEC_SBR_ADDITIONS
            if Z jump no_ltp_left;
               Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
               #ifdef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
                  if NZ call $error;         // Currently LTP & SBR together not supported
               #endif
               if Z call $aacdec.ltp_data;
            no_ltp_left:
         #else
            if NZ call $aacdec.ltp_data;
         #endif
         Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
         if NZ jump possible_corruption;


         // if (common_window == 1)
         Null = M[r9 + $aac.mem.COMMON_WINDOW];
         if Z jump no_common_window;
            // ltp_data_present = getbits(1);
            call $aacdec.get1bit;
            r5 = r4 + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD;
            M[r4 + $aacdec.ics.LTP_INFO_CH2_PTR_FIELD] = r1;

            // if (ltp_data_present == 1) aacdec.ltp_data()
            #ifdef AACDEC_SBR_ADDITIONS
               if Z jump no_ltp_right;
                  Null = M[r9 + $aac.mem.SBR_PRESENT_FIELD];
                  #ifdef AACDEC_CALL_ERROR_ON_POSSIBLE_CORRUPTION
                     if NZ call $error;         // Currently LTP & SBR together not supported
                  #endif
                  if Z call $aacdec.ltp_data;
               no_ltp_right:
            #else
               if NZ call $aacdec.ltp_data;
            #endif
            Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
            if NZ jump possible_corruption;

         no_common_window:
      no_predictor_data_present:

   window_type_done:

   // Flag that ics_info has been called successfully
   r0 = 1;
   M[r9 + $aac.mem.ICS_INFO_DONE_FIELD] = r0;

   possible_corruption:
   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
