// *****************************************************************************
// Copyright (c) 2015 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

#ifdef AACDEC_ELD_ADDITIONS

// *****************************************************************************
// MODULE:
//    $aacdec.eld_decode_sce
//
// DESCRIPTION:
//    Common function to get single channel element information
//    or read lfe channel element information (ELD version)
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - assume everything
//
// *****************************************************************************
.MODULE $M.aacdec.eld_decode_sce;
   .CODESEGMENT AACDEC_DECODE_SCE_ELD_PM;
   .DATASEGMENT DM;

   $aacdec.eld_decode_sce:
   $aacdec.eld_decode_lfe_ce:

   // push rLink onto stack
   push rLink;

   // make sure that sample_rate is known
   // (ie. corrupt frames might get us here with out it being set)
   Null = M[r9 + $aac.mem.SF_INDEX_FIELD];
   if NEG jump $aacdec.possible_corruption;

   // make sure we haven't had too many SCEs
   r0 = M[r9 + $aac.mem.NUM_SCEs];
   r0 = r0 + 1;
   M[r9 + $aac.mem.NUM_SCEs] = r0;
   Null = r0 - $aacdec.MAX_NUM_SCES;
   if GT jump $aacdec.possible_corruption;

   // set current ics and current spec pointers
   r4 = r9 + $aac.mem.ICS_LEFT;
   M[r9 + $aac.mem.CURRENT_ICS_PTR] = r4;
   r0 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
   M[r9 + $aac.mem.CURRENT_CHANNEL] = Null;

   // calc_sfbs_and_wingroups();
   call $aacdec.calc_sfb_and_wingroup;
   Null = M[r9 + $aac.mem.POSSIBLE_FRAME_CORRUPTION];
   if NZ jump $aacdec.possible_corruption;

   // -- reset the list of unpacked huffman tables --
   call $aacdec.huffman_reset_unpacked_list;

   // individual_channel_stream_eld(0);
   M[r9 + $aac.mem.COMMON_WINDOW] = Null;
   call $aacdec.eld_individual_channel_stream;

   // Flag that ics_info has been called successfully
   r0 = 1;
   M[r9 + $aac.mem.ICS_INFO_DONE_FIELD] = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;

#endif //AACDEC_ELD_ADDITIONS
