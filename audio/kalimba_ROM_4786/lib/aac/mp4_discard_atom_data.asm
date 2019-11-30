// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.mp4_discard_atom_data
//
// DESCRIPTION:
//    Discard data from an atom
//
// INPUTS:
//    - r5 = LS word amount to discard (3 bytes)
//    - r4 = MS word amount to discard (1 byte)
//    - r9 = input structure pointer
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0-5, r8
//
// *****************************************************************************
.MODULE $M.aacdec.mp4_discard_atom_data;
   .CODESEGMENT AACDEC_MP4_DISCARD_ATOM_DATA_PM;
   .DATASEGMENT DM;

   $aacdec.mp4_discard_atom_data:

   // push rLink onto stack
   push rLink;
   
#if defined(PATCH_LIBS)
   LIBS_SLOW_SW_ROM_PATCH_POINT($aacdec.MP4_DISCARD_ATOM_DATA_ASM.MP4_DISCARD_ATOM_DATA.MP4_DISCARD_ATOM_DATA.PATCH_ID_0, r8)
#endif
   

   // see if half way through discarding
   Null = M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA];
   if Z jump new_sub_atom_to_discard;

      // restore amount left to discard
      r4 = M[r9 + $aac.mem.MP4_DISCARD_AMOUNT_MS];
      r5 = M[r9 + $aac.mem.MP4_DISCARD_AMOUNT_LS];

   new_sub_atom_to_discard:

   Null = r4 AND 0xFFFF00;
   if NZ jump $aacdec.possible_corruption;
   Null = r5;
   if NZ jump non_zero_input;
      Null = r4;
      if Z jump escape;
   non_zero_input:


   r8 = M[r9 + $aac.mem.NUM_BYTES_AVAILABLE];

   // loop around discarding {r4:r5} bytes
   discard_loop:

      r8 = r8 - 1;
      if NEG jump out_of_data;

      // discard a byte
      call $aacdec.get1byte;

      // decrement count (r4:r5) by 1
      r5 = r5 - 1;
      r4 = r4 - Borrow;
   if NZ jump discard_loop;
   Null = r5;
   if NZ jump discard_loop;

   // update num_bytes_available store
   M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = r8;

   // successfully discarded all bytes required
   M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA] = Null;

   escape:
   // pop rLink from stack
   jump $pop_rLink_and_rts;


   out_of_data:
      // update num_bytes_available store
      M[r9 + $aac.mem.NUM_BYTES_AVAILABLE] = Null;
      r0 = 1;
      M[r9 + $aac.mem.FRAME_UNDERFLOW] = r0;

      // flag that we still need to discard the remainder next time
      M[r9 + $aac.mem.MP4_IN_DISCARD_ATOM_DATA] = r0;
      M[r9 + $aac.mem.MP4_DISCARD_AMOUNT_MS] = r4;
      M[r9 + $aac.mem.MP4_DISCARD_AMOUNT_LS] = r5;

      // pop rLink from stack
      jump $pop_rLink_and_rts;

.ENDMODULE;
