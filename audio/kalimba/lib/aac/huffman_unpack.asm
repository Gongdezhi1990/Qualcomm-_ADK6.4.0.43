// *****************************************************************************
// Copyright (c) 2005 - 2017 Qualcomm Technologies International, Ltd.
// %%version
//
// *****************************************************************************

#include "aac_library.h"
#include "core_library.h"

// *****************************************************************************
// MODULE:
//    $aacdec.huffman_reset_unpacked_list
//
// DESCRIPTION:
//    Reset the list of huffman tables that have been unpacked from flash.
//
// INPUTS:
//    - none
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r10, DoLoop, I1
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_reset_unpacked_list;
   .CODESEGMENT AACDEC_HUFFMAN_RESET_UNPACKED_LIST_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_reset_unpacked_list:

   I1 = r9 + $aac.mem.huffman_cb_table;
   r10 = 12;
   r0 = $aacdec.HUFFMAN_TABLE_NOT_UNPACKED;
   do clear_table;
      M[I1, MK1] = r0;
   clear_table:
   rts;

.ENDMODULE;





// *****************************************************************************
// MODULE:
//    $aacdec.huffman_unpack_individual_flash_table
//
// DESCRIPTION:
//    Unpack a huffman table from flash into the frame memory pool.
//
// INPUTS:
//    - r5 - table to unpack
//
// OUTPUTS:
//    - none
//
// TRASHED REGISTERS:
//    r0, r1, r2, r3, r8, r10, DoLoop, I1, I3
//
// *****************************************************************************
.MODULE $M.aacdec.huffman_unpack_individual_flash_table;
   .CODESEGMENT AACDEC_HUFFMAN_UNPACK_INDIVIDUAL_FLASH_TABLE_PM;
   .DATASEGMENT DM;

   $aacdec.huffman_unpack_individual_flash_table:

   // check if huffman table has already been unpacked
   // hence no need to unpack it again
   Words2Addr(r5);
   r0 = r9 + r5;
   r0 = M[$aac.mem.huffman_cb_table  + r0];
   Null = r0 - $aacdec.HUFFMAN_TABLE_NOT_UNPACKED;
   if NZ rts;

   // push rLink onto stack
   push rLink;

   r10 = r5 + r9;
   r10 = M[$aac.mem.huffman_table_sizes_div_2+ r10];
   r0 = r9 + r5;
   r0 = M[$aac.mem.huffman_packed_list + r0];

   // map the appropriate flash page into the flash window
   call $mem.ext_window_access_as_ram;
   I1 = r0;

   // allocate tmp memory for unpacked huffman codebook
   //  - these can be destroyed after all huffman data has been read
   r0 = r10 LSHIFT 1;
   call $aacdec.frame_mem_pool_allocate;
   if NEG jump $aacdec.possible_corruption;

   r0 = r9 + r5;
   M[$aac.mem.huffman_cb_table + r0] = r1;

   // copy out of flash the huffman codebook
   // they are packed as follows:
   //    addr 0:  (word 0) bits 23-8
   //    addr 1:  (word 0) bits 7-0    (word 1) bits 23-16
   //    addr 2:  (word 1) bits 15-0
   //    addr 3:  (word 2) bits 23-8
   //    addr 4:  (word 2) bits 7-0    (word 3) bits 23-16
   //    addr 5:  (word 3) bits 15-0
   //    ....
   //    etc
   I3 = r1;
   do flash_copy_loop;
      // read flash addr N
      r0 = M[I1, MK1];
      // read flash addr N+1
      r1 = M[I1, MK1];

      // -- reassemble 1st 24bit word --
      r0 = r0 LSHIFT 8;
      r2 = r1 LSHIFT -8;
      // mask off possible sign extension of flash reads
      r2 = r2 AND 0xFF;
      r2 = r2 OR r0,
       r0 = M[I1, MK1];       // read flash addr N+2

      // -- reassemble 2nd 24bit word --
      // mask off possible sign extension of flash reads
      r0 = r0 AND 0xFFFF;
      r1 = r1 AND 0xFF;
      r1 = r1 LSHIFT 16;
      r0 = r0 + r1,
       M[I3, MK1] = r2;       // store 1st 24bit word

      // store 2nd 24bit word
      M[I3, MK1] = r0;
   flash_copy_loop:


   // pop rLink from stack
   jump $pop_rLink_and_rts;

.ENDMODULE;
