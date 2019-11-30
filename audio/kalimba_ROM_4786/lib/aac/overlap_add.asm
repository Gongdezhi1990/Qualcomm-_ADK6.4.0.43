// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.overlap_add
//
// DESCRIPTION:
//    Copy data to the overlap_add buffer for next frame
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    - r0, r1, r3, r4, r10, I1, I4
//
// *****************************************************************************
.MODULE $M.aacdec.overlap_add;
   .CODESEGMENT AACDEC_OVERLAP_ADD_PM;
   .DATASEGMENT DM;

   $aacdec.overlap_add:

   // push rLink onto stack
   push rLink;

   // select overlap_add_left or overlap_add_right
   r0 = M[r9 +  $aac.mem.OVERLAP_ADD_LEFT_PTR];
   I1 = r0;
   r0 = M[r9 + $aac.mem.OVERLAP_ADD_RIGHT_PTR];
   Null = M[r9 + $aac.mem.CURRENT_CHANNEL];
   if NZ I1 = r0;
   r0 = M[r9 + $aac.mem.TMP_MEM_POOL_PTR];
   I4 = r0 + (1023*ADDR_PER_WORD);

   // scale data up if not already done in tns
   r4 = M[r9 + $aac.mem.CURRENT_SPEC_BLKSIGNDET_PTR];
   r0 = 1;
   r3 = 2;
   Null = M[r4 + (1*ADDR_PER_WORD)];
   if NZ r3 = r0;
   M[r4 + (1*ADDR_PER_WORD)] = Null;

   // set number of elements to copy based on the window sequence type
   r10 = 255;
   r1 = 287;
   r4 = M[r9 + $aac.mem.CURRENT_ICS_PTR];
   r0 = M[r4 + $aacdec.ics.WINDOW_SEQUENCE_FIELD];
   Null = r0 - $aacdec.EIGHT_SHORT_SEQUENCE;
   if Z r10 = r1;
   I1 = I1 + MK1;
   Words2Addr(r10);
   I1 = I1 + r10;
   I1 = I1 + r10,
    r4 = M[I4, -MK1];

   Addr2Words(r10);
   // do the copy
   r4 = r4 * r3 (int),
    r0 = M[I4, -MK1];
   do overlap_add_loop2;
      r0 = r0 * r3 (int),
       r4 = M[I4, -MK1],
       M[I1, -MK1] = r4;
      r4 = r4 * r3 (int),
       r0 = M[I4, -MK1],
       M[I1, -MK1] = r0;
   overlap_add_loop2:
   r0 = r0 * r3 (int),
    M[I1, -MK1] = r4;
   M[I1, -MK1] = r0;

   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
