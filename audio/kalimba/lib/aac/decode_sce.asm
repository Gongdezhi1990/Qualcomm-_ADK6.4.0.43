// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.decode_sce
//
// DESCRIPTION:
//    Get single channel element information
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
.MODULE $M.aacdec.decode_sce;
   .CODESEGMENT AACDEC_DECODE_SCE_PM;
   .DATASEGMENT DM;

   $aacdec.decode_sce:

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

   // dummy = getbits(4);  //element_instance_tag
   call $aacdec.get4bits;

   // set current ics and current spec pointers
   r0 = r9 + $aac.mem.ICS_LEFT;
   M[r9 + $aac.mem.CURRENT_ICS_PTR] = r0;
   r0 = M[r9 + $aac.mem.BUF_LEFT_PTR];
   M[r9 + $aac.mem.CURRENT_SPEC_PTR] = r0;
   M[r9 + $aac.mem.CURRENT_CHANNEL] = Null;

   // -- reset the list of unpacked huffman tables --
   call $aacdec.huffman_reset_unpacked_list;


   // individual_channel_stream(0);
   M[r9 + $aac.mem.COMMON_WINDOW] = Null;
   call $aacdec.individual_channel_stream;


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
